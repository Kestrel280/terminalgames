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

#define LOG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#define PORT 10279
#define QUEUE_ERROR_RESPONSE(ERRMSG) do { struct MHD_Response* __response = MHD_create_response_from_buffer(strlen(ERRMSG), (void*)ERRMSG, MHD_RESPMEM_PERSISTENT);\
                                MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, __response);\
                                MHD_destroy_response(__response); } while(0)
const char* _leaderboardDir = "/leaderboards";
const char* _templateInsertStmt = "INSERT into %s values(@name, @score, @time);";

// GNU libmicrohttpd tutorial
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
//
// Helpful reference project for libmicrohttpd
// https://github.com/PedroFnseca/rest-api-C/blob/main/src/main.c

sqlite3* db;

typedef struct _handlerParam HandlerParam;
struct _handlerParam {
    char* data;
};

// for use with MHD_get_connection_values: simply prints header pairs
enum MHD_Result printKey(void* cls, enum MHD_ValueKind kind, const char* key, const char* value) {
    LOG("%s: %s\n", key, value);
    return MHD_YES;
}

enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {
    struct MHD_Response* response;

    LOG("New '%s' request for '%s' using version '%s'\n", method, url, version);

    // MHD_get_connection_values() -> 3rd argument is a fn pointer; calls this fn for each key-value pair in the request
    //MHD_get_connection_values(connection, MHD_HEADER_KIND, &printKey, NULL);

    // check that they're accessing /leaderboards/; if not, respond with error
    // normally, traffic can only reach this server via the nginx reverse proxy...
    //  but accessing it directly via port would allow arbitrary url payload, so we need to safeguard against that
    if (strncmp(url, _leaderboardDir, strlen(_leaderboardDir)) != 0) {
        QUEUE_ERROR_RESPONSE("improper attempt to access leaderboards API");
        LOG("... improper access to leaderboards directory\n");
        return MHD_YES;
    }

    // get game they're trying to access and construct uri to leaderboard file: "{_leaderboardDir}/{game}.csv"
    const char* game = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "game");
    if (game == NULL) {
        QUEUE_ERROR_RESPONSE("no game header specified in leaderboards API request");
        LOG("--ERROR-- no 'game' header specified in request to API\n");
        return MHD_YES;
    }

    if (strcmp(method, "GET") == 0) { // --- GET ---
        // TODO construct get request and respond with top n scores
        QUEUE_ERROR_RESPONSE("GET not yet implemented for leaderboards");
        return MHD_YES;
    } else if (strcmp(method, "POST") == 0) { // --- POST ---
        // fetch scoreEntry header TODO switch to json?
        const char* _lbEntry = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "scoreEntry");
        if (_lbEntry == NULL) {
            QUEUE_ERROR_RESPONSE("no scoreEntry header specified in leaderboards API request");
            LOG("user tried to post score, but did not provide a scoreEntry header\n");
            return MHD_YES;
        }

        // parse scoreEntry header
        char lbEntry[strlen(_lbEntry)];
        strcpy(lbEntry, _lbEntry);
        char* score = strtok(lbEntry, ","); // TODO sanitize
        char* name = strtok(NULL, ",");
        char* time = strtok(NULL, ",");
        if (time == NULL) {
            QUEUE_ERROR_RESPONSE("invalid scoreEntry header in leaderboards API request");
            LOG("malformed scoreEntry header\n");
            return MHD_YES;
        }

        // construct and execute sql statement
        char _insertStmt[strlen(_templateInsertStmt) + strlen(game)];
        sprintf(_insertStmt, _templateInsertStmt, game);
        sqlite3_stmt* insertStmt;
        if (sqlite3_prepare_v2(db, _insertStmt, -1, &insertStmt, NULL) != SQLITE_OK) { // compile statement
            QUEUE_ERROR_RESPONSE("internal database error");
            LOG("failure compiling SQL statements: error codes\n");
            return MHD_YES;
        }
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@name"), name, strlen(name), SQLITE_STATIC); // SQLITE_STATIC = i am responsible for memory of the string
        sqlite3_bind_int(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@score"), atoi(score));
        sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@time"), time, strlen(time), SQLITE_STATIC);
        sqlite3_step(insertStmt); // execute SQL statement
        sqlite3_finalize(insertStmt);

        LOG("user '%s' posted score '%s' at time '%s'\n", name, score, time);
        response = MHD_create_response_from_buffer(strlen("successfully posted score"), (void*)"successfully posted score", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // not a GET request; error (for now)
    QUEUE_ERROR_RESPONSE("unknown error accessing leaderboards API");
    return MHD_YES;
}

int main(int argc, char* argv[]) {
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                              &connectionCallback, NULL, MHD_OPTION_END);

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
