#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

int http_server_start(int port, const char *document_root);
void http_server_stop(void);

#endif
