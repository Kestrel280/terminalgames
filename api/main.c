#include <microhttpd.h>
#include <stdio.h>
#include <sqlite3.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include "utils.h"
#include "server.h"

const char* urlEndpoint = "/leaderboards";
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
