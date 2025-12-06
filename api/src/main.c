#include <systemd/sd-daemon.h>
#include <microhttpd.h>
#include <sqlite3.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"
#include "leaderboardapi.h"
#include "sdserver.h"

sqlite3* db = NULL;
struct MHD_Daemon* sdDaemon = NULL;

void handleShutdown() {
    const char* errmsg = "Leaderboard API shutdown\n";
    write(2, errmsg, strlen(errmsg));
    sqlite3_close(db); // nop if null
    //if (sdDaemon) sdServerShutdown(&sdDaemon);
    sd_notify(0, "STOPPING=1");
    exit(1);
    return;
}
void handleServerPanic(void* cls, const char* file, unsigned int line, const char* reason) {
    LOG("Leaderboard API encountered server panic:\n");
    LOG("\t\tfile = %s\n\t\tline = %u\n\t\treason = '%s'\n", file, line, reason);
    handleShutdown();
    return;
}

int main(int argc, char* argv[]) {
    // install signal handlers
    signal(SIGTERM, handleShutdown);

    // open leaderboard database
    sqlite3_open("leaderboards.db", &db);
    if (db == NULL) { LOG("failure opening database\n"); return 1; }

    // start running the actual server
    MHD_set_panic_func(handleServerPanic, NULL);
    sdDaemon = sdServerStart(leaderboardProcessRequest, PORT);
    if (sdDaemon == NULL) { LOG("failure starting sdServer\n"); return 1; }

    // inform systemd that we're good to go
    sd_notify(0, "READY=1");

    // server is running on its own thread, and we've installed signal handlers for 
    pthread_exit(NULL);
}
