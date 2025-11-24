#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include "utils.h"
#include "server.h"

const char* urlEndpoint = "/leaderboards";
const char* _templateInsertStmt = "INSERT into %s values(@name, @score, @time);";
sqlite3* db;

int main(int argc, char* argv[]) {
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                              &connectionCallback, NULL, // specify connection callback, and pass no args
                              MHD_OPTION_NOTIFY_COMPLETED, completeRequest, NULL,
                              MHD_OPTION_END);

    if (daemon == NULL) return 1;

    sqlite3_open("leaderboards.db", &db);
    if (db == NULL) {
        LOG("failure opening database\n");
        MHD_stop_daemon(daemon);
        return -1;
    }

    getchar(); // block; server (listen + response callback) is running in its own thread, so just keep the process alive

    sqlite3_close(db);
    MHD_stop_daemon(daemon);
    return 0;
}

/*void sqlPost() {
    // construct and execute sql statement
    char _insertStmt[strlen(_templateInsertStmt) + strlen(game)];
    sprintf(_insertStmt, _templateInsertStmt, game);
    sqlite3_stmt* insertStmt;
    if (sqlite3_prepare_v2(db, _insertStmt, -1, &insertStmt, NULL) != SQLITE_OK) { // compile statement
        QUEUE_ERROR_RESPONSE("internal database error (compiling SQL statement)");
        return MHD_YES;
    }
    sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@name"), name, strlen(name), SQLITE_STATIC); // SQLITE_STATIC = i am responsible for memory of the string
    sqlite3_bind_int(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@score"), atoi(score));
    sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@time"), time, strlen(time), SQLITE_STATIC);
    sqlite3_step(insertStmt); // execute SQL statement
    sqlite3_finalize(insertStmt);
}*/
