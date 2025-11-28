#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "sqlite_wrapper.h"
#include "log.h"

static sqlite3 *db = NULL;
static pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

int db_open(const char *path) {
    if (!path) return -1;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        LOGE("sqlite open failed: %s", sqlite3_errmsg(db));
        return -1;
    }
    char *err = NULL;
    const char *schema = "CREATE TABLE IF NOT EXISTS config(key TEXT PRIMARY KEY, value TEXT);";
    if (sqlite3_exec(db, schema, 0, 0, &err) != SQLITE_OK) {
        LOGE("sqlite create table failed: %s", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

int db_set(const char *k, const char *v) {
    if (!db || !k) return -1;
    pthread_mutex_lock(&db_lock);
    sqlite3_stmt *st = NULL;
    sqlite3_prepare_v2(db, "REPLACE INTO config(key,value) VALUES(?,?)", -1, &st, NULL);
    sqlite3_bind_text(st, 1, k, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 2, v ? v : "", -1, SQLITE_STATIC);
    sqlite3_step(st);
    sqlite3_finalize(st);
    pthread_mutex_unlock(&db_lock);
    return 0;
}

char *db_get(const char *k) {
    if (!db || !k) return NULL;
    pthread_mutex_lock(&db_lock);
    sqlite3_stmt *st = NULL;
    char *res = NULL;
    sqlite3_prepare_v2(db, "SELECT value FROM config WHERE key=?", -1, &st, NULL);
    sqlite3_bind_text(st, 1, k, -1, SQLITE_STATIC);
    if (sqlite3_step(st) == SQLITE_ROW) {
        const unsigned char *txt = sqlite3_column_text(st, 0);
        if (txt) res = strdup((const char*)txt);
    }
    sqlite3_finalize(st);
    pthread_mutex_unlock(&db_lock);
    return res;
}
