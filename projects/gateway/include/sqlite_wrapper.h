#ifndef SQLITE_WRAPPER_H
#define SQLITE_WRAPPER_H
int db_open(const char *path);
int db_set(const char *k, const char *v);
char *db_get(const char *k);
#endif
