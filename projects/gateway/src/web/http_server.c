#define _POSIX_C_SOURCE 200809L
#include "http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../log/log.h"
#include "../storage/sqlite_wrapper.h"
#include "../net/mqtt_client.h"

/* CivetWeb headers (make sure civetweb.c/.h are in thirdparty and compile) */
#include "civetweb.h"

static struct mg_context *ctx = NULL;
static int http_port = 0;
static char doc_root[256];

static int api_config_get(struct mg_connection *conn, void *cbdata) {
    const struct mg_request_info *ri = mg_get_request_info(conn);
    LOGD("HTTP %s %s", ri->request_method, ri->uri);

    /* Example: return some config from sqlite */
    char *v = db_get("version");
    if (!v) v = strdup("unknown");
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"
              "{\"version\":\"%s\"}",
              v);
    free(v);
    return 200;
}

static int api_config_post(struct mg_connection *conn, void *cbdata) {
    /* read body */
    long long len = mg_read(conn, NULL, 0); /* not portable, read below */
    /* safer: read content-length header */
    const char *cl = mg_get_header(conn, "Content-Length");
    int content_len = cl ? atoi(cl) : 0;
    char *buf = NULL;
    if (content_len > 0) {
        buf = malloc(content_len + 1);
        int r = mg_read(conn, buf, content_len);
        if (r > 0) buf[r] = 0; else { free(buf); buf = NULL; }
    }
    if (buf) {
        /* very small example: expect {"key":"...", "value":"..."} */
        /* naive parse: find "key" and "value" substrings */
        char key[128] = {0}, val[256] = {0};
        sscanf(buf, "{\"key\":\"%127[^\"]\",\"value\":\"%255[^\"]\"}", key, val);
        if (key[0]) {
            db_set(key, val);
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"ok\":1}");
        } else {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\n\r\n{\"ok\":0}");
        }
        free(buf);
    } else {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\n\r\n{\"ok\":0}");
    }
    return 200;
}

/* simple handler to publish telemetry via MQTT (for testing) */
static int api_pubtele(struct mg_connection *conn, void *cbdata) {
    const char *qv = mg_get_var(conn, "voltage");
    double voltage = qv ? atof(qv) : 230.0;
    const char *cv = mg_get_var(conn, "current");
    double current = cv ? atof(cv) : 10.0;
    const char *ev = mg_get_var(conn, "energy");
    double energy = ev ? atof(ev) : 0.0;

    mqtt_publish_telemetry(voltage, current, energy);

    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"published\":1}");
    return 200;
}

int http_server_start(int port, const char *document_root) {
    if (ctx) return 0;
    http_port = port;
    if (document_root) strncpy(doc_root, document_root, sizeof(doc_root)-1);
    const char *options[] = {
        "document_root", doc_root[0] ? doc_root : "./web/admin",
        "listening_ports", 0, /* filled below */
        0
    };
    char portbuf[16]; snprintf(portbuf, sizeof(portbuf), "%d", port);
    const char *opts[6];
    opts[0] = "document_root"; opts[1] = doc_root[0] ? doc_root : "./web/admin";
    opts[2] = "listening_ports"; opts[3] = portbuf;
    opts[4] = NULL;

    LOGI("Starting CivetWeb on port %d docroot=%s", port, opts[1]);
    ctx = mg_start(NULL, NULL, opts);
    if (!ctx) { LOGE("mg_start failed"); return -1; }

    /* register URIs */
    mg_set_request_handler(ctx, "/api/config", api_config_get, NULL);    /* GET */
    mg_set_request_handler(ctx, "/api/config_set", api_config_post, NULL); /* POST */
    mg_set_request_handler(ctx, "/api/pubtele", api_pubtele, NULL);

    return 0;
}

void http_server_stop(void) {
    if (!ctx) return;
    mg_stop(ctx);
    ctx = NULL;
}
