#ifndef IO_SERVICE_H
#define IO_SERVICE_H
#include "thread_pool.h"
int start_io_loop(int listen_fd, thread_pool_t *pool);
int create_tcp_listener(int port, int backlog);
int make_socket_nonblocking(int fd);
#endif
