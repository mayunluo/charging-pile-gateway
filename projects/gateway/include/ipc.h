#ifndef IPC_H
#define IPC_H
int create_unix_server(const char *path);
int unix_client_connect(const char *path);
#endif
