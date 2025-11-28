#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "thread_pool.h"
#include "io_service.h"
#include "ipc.h"
#include "sqlite_wrapper.h"
#include "common/config.h"
#include "net/mqtt_client.h"
#include "web/http_server.h"

static volatile int running = 1;

static void sigint_handler(int sig) {
    LOGW("received signal %d, shutting down...", sig);
    running = 0;
    stop_io_loop();        // 让 epoll loop 退出
}

/*******************************************************************
 * Gateway main entry
 *******************************************************************/
int main(int argc, char **argv)
{
    /* 1. Catch kill signals */
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    /* 2. Load config */
    if (config_load("/etc/gateway/config.json") != 0) {
        LOGW("config not found, using default config");
        config_load_default();
    }

    LOGI("Gateway starting, version: %s", GATEWAY_VERSION);

    /* 3. Open SQLite */
    if (db_open(DEFAULT_DB_PATH) != 0) {
        LOGW("failed to open sqlite db (%s), fallback to in-memory DB",
             DEFAULT_DB_PATH);
        db_open(":memory:");
    }
    db_set("version", GATEWAY_VERSION);

    /* 4. Create thread pool (用于业务/CPU 任务，不阻塞 IO) */
    int worker_num = config_get_int("thread_pool_size", 4);
    thread_pool_t *pool = thread_pool_create(worker_num);

    /* 5. Create TCP listener (业务 TCP 端口) */
    int listen_port = config_get_int("listen_port", 7000);
    int listen_fd = create_tcp_listener(listen_port, 64);
    if (listen_fd < 0) {
        LOGE("Failed to create TCP listener on port %d", listen_port);
        goto exit_cleanup;
    }
    LOGI("TCP listener started on port %d", listen_port);

    /* 6. Create Unix domain socket (进程内 IPC) */
    int unix_fd = create_unix_server(UNIX_SOCKET_PATH);
    if (unix_fd < 0) {
        LOGW("Cannot create unix domain socket: %s", UNIX_SOCKET_PATH);
    } else {
        LOGI("Unix socket ready: %s", UNIX_SOCKET_PATH);
    }

    /* 7. Initialize MQTT (异步回调模式) */
    if (mqtt_init(config_get_str("mqtt.host", "localhost"),
                  config_get_int("mqtt.port", 1883),
                  config_get_str("mqtt.client_id", "gateway-001")) != 0) {
        LOGW("MQTT init failed, continue without MQTT.");
    } else {
        LOGI("MQTT initialized");
    }

    /* 8. Start HTTP CGI Server */
    const char *docroot = config_get_str("web.root", "/usr/share/gateway/www");
    if (http_server_start(config_get_int("web.port", 8000), docroot) != 0) {
        LOGW("HTTP server start failed");
    } else {
        LOGI("HTTP server running on port %d", config_get_int("web.port", 8000));
    }

    /* 9. Prepare epoll + IO loop */
    if (io_service_init() != 0) {
        LOGE("io_service_init failed");
        goto exit_cleanup;
    }

    /* 注册 fd 到 epoll（网络 IO） */
    io_service_add_fd(listen_fd, IO_READ, tcp_listener_callback, pool);
    if (unix_fd > 0)
        io_service_add_fd(unix_fd, IO_READ, unix_socket_callback, NULL);

    LOGI("Entering IO loop...");
    start_io_loop();    // 阻塞，直到 stop_io_loop() 调用

    LOGI("IO loop quit, cleaning up...");

    /*******************************************************************
     * Cleanup
     *******************************************************************/
exit_cleanup:

    http_server_stop();
    mqtt_deinit();

    if (listen_fd > 0) close(listen_fd);
    if (unix_fd > 0) close(unix_fd);

    thread_pool_destroy(pool);
    db_close();

    LOGI("Gateway exited normally.");

    return 0;
}
