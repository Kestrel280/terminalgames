#include <systemd/sd-daemon.h>
#include <microhttpd.h>
#include <stdio.h>
#include <sqlite3.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include "utils.h"
#include "server.h"

const char* urlEndpoint = "/leaderboards";
sqlite3* db;

void handleServerPanic(void* cls, const char* file, unsigned int line, const char* reason) {
    stopServer();
    return;
}

int main(int argc, char* argv[]) {

    sqlite3_open("leaderboards.db", &db);
    if (db == NULL) {
        LOG("failure opening database\n");
        return -1;
    }

    MHD_set_panic_func(handleServerPanic, NULL);
    struct MHD_Daemon* daemon;
    sdStartServer(processRequest, PORT);
    sd_notify(0, "READY=1");

    if (daemon == NULL) return 1;

    getchar(); // block; server (listen + response callback) is running in its own thread, so just keep the process alive

    sd_notify(0, "STOPPING=1");
    sqlite3_close(db);
    MHD_stop_daemon(daemon);
    return 0;
}
