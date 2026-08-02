#include <string.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <uriparser/Uri.h>
#include "dataapi.h"
#include "wellness.h"

#define EXPAND_AND_LOG_SQL_STATEMENT(stmt) do { char* __expStr__ = sqlite3_expanded_sql(stmt); LOG("\texpanded SQL Statement: '%s'\n", __expStr__); sqlite3_free(__expStr__); } while (0);
static inline bool SAME_STRING(const char* s1, const char* s2) { return (strcmp(s1, s2) == 0); }

extern sqlite3* db;
const char* wellnessEndpoint = "data";
const char* _insertStmt = "INSERT INTO wellness VALUES(@user, @date, @measure_name, @measure_value);";
const char* _getUserPwdStmt = "SELECT password FROM users WHERE user=@user;";

bool wellnessGet(ConnectionInfo* ci, char** pOut) {
    const char* err = "data api: invalid endpoint or GET not supported for this endpoint";
    int len = strlen(err);
    char* errOut = (char*)malloc(sizeof(char*) * (len + 1));
    memcpy(errOut, err, len);
    errOut[len] = '\x00';
    *pOut = errOut;
    return false;
}

bool wellnessPost(ConnectionInfo* ci) {
    const char* user = MHD_lookup_connection_value(ci->mhd_connection, MHD_HEADER_KIND, "username");
    const char* pwd = MHD_lookup_connection_value(ci->mhd_connection, MHD_HEADER_KIND, "password");
    const char* date = MHD_lookup_connection_value(ci->mhd_connection, MHD_HEADER_KIND, "entrydate");

    char* req = ci->buf;
    char* afterLast = req;
    while (*afterLast) afterLast++;

    // Verify user/password/date were provided
    if (!pwd || !user || !date) {
        LOG("no user and/or password and/or date provided in header, rejecting\n");
        return false;
    }
    LOG("user: %s | password hash: %s | date: %s\n", user, pwd, date);

    // Prepare SQL statement to fetch stored password for user
    sqlite3_stmt* getUserPwdStmt;
    if (sqlite3_prepare_v2(db, _getUserPwdStmt, -1, &getUserPwdStmt, NULL) != SQLITE_OK) {
        LOG("error compiling SQL get-user-password statement\n");
        return false;
    }
    sqlite3_bind_text(getUserPwdStmt, sqlite3_bind_parameter_index(getUserPwdStmt, "@user"), user, strlen(user), SQLITE_STATIC);
    EXPAND_AND_LOG_SQL_STATEMENT(getUserPwdStmt);

    // Execute SQL query for user password and check for match
    if (sqlite3_step(getUserPwdStmt) != SQLITE_ROW) {
        LOG("no user '%s' on record\n", user);
        return false;
    }
    const char* storedPassword = (const char*)sqlite3_column_text(getUserPwdStmt, 0);
    if (!SAME_STRING(pwd, storedPassword)) {
        LOG("wrong password for user '%s'\n", user);
        return false;
    }
    
    // Prepare SQL statement for inserting values
    sqlite3_stmt* insertStmt;
    if (sqlite3_prepare_v2(db, _insertStmt, -1, &insertStmt, NULL) != SQLITE_OK) {
        LOG("error compiling SQL insert statement\n");
        return false;
    }
    // Parse query string
    UriQueryListA *queryList, *currentKv;
    int itemCount;
    if (uriDissectQueryMallocA(&queryList, &itemCount, req, afterLast) != URI_SUCCESS) {
        LOG("malformed query string < %s >\n", req);
        return false;
    }

    // Convert date to RFC3339 datetime
    char datetime[64] = { 0 };
    snprintf(datetime, 63, "%sT00:00:00Z", date);

    // For each kv in query string, insert into wellness db
    currentKv = queryList;
    while (currentKv) {
        if (strlen(currentKv->value) == 0) { currentKv = currentKv->next; continue; }
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@user"), user, strlen(user), SQLITE_STATIC);
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@date"), datetime, strlen(datetime), SQLITE_STATIC);
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@measure_name"), currentKv->key, strlen(currentKv->key), SQLITE_STATIC);
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@measure_value"), currentKv->value, strlen(currentKv->value), SQLITE_STATIC);
        EXPAND_AND_LOG_SQL_STATEMENT(insertStmt);
        sqlite3_step(insertStmt);
        sqlite3_reset(insertStmt);
        currentKv = currentKv->next;
    }
    uriFreeQueryListA(queryList);
    
    LOG("successfully inserted for user '%s': '%s'\n", user, req);
    return true;
}
