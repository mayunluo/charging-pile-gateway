#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "io_service.h"
#include "log.h"
#include "thread_pool.h"

#define MAX_EVENTS 64
#define READ_BUF 2048

int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_tcp_listener(int port, int backlog) {
    int sock = -1;
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    int opt = 1; setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(sock); return -1; }
    if (listen(sock, backlog) < 0) { close(sock); return -1; }
    make_socket_nonblocking(sock);
    return sock;
}

struct client_info {
    int fd;
};

static void handle_client_read(void *arg) {
    struct client_info *ci = (struct client_info*)arg;
    int fd = ci->fd;
    char buf[READ_BUF];
    while (1) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r > 0) {
            // simple echo handling example: log and echo back
            LOGD("recv %zd bytes from fd=%d", r, fd);
            ssize_t w = write(fd, buf, r);
            if (w < 0) { LOGW("write failed fd=%d", fd); }
        } else if (r == 0) {
            LOGI("peer closed fd=%d", fd);
            close(fd);
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            LOGE("read error fd=%d err=%s", fd, strerror(errno));
            close(fd);
            break;
        }
    }
    free(ci);
}

int start_io_loop(int listen_fd, thread_pool_t *pool) {
    int epfd = epoll_create1(0);
    if (epfd < 0) { LOGE("epoll_create1 failed"); return -1; }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) { LOGE("epoll_ctl add listen_fd failed"); close(epfd); return -1; }

    struct epoll_event events[MAX_EVENTS];
    for (;;) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, 1000);
        if (n < 0) {
            if (errno == EINTR) continue;
            LOGE("epoll_wait error: %s", strerror(errno));
            break;
        }
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == listen_fd) {
                // accept loop
                while (1) {
                    int cfd = accept(listen_fd, NULL, NULL);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        LOGW("accept error: %s", strerror(errno));
                        break;
                    }
                    make_socket_nonblocking(cfd);
                    struct epoll_event cev;
                    cev.events = EPOLLIN | EPOLLET;
                    cev.data.fd = cfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev);
                    LOGI("accepted fd=%d", cfd);
                }
            } else {
                // ready for read; hand off to thread pool
                int fd = events[i].data.fd;
                struct client_info *ci = malloc(sizeof(*ci));
                ci->fd = fd;
                thread_pool_submit(pool, handle_client_read, ci);
            }
        }
    }

    close(epfd);
    return 0;
}
