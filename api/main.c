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

// GNU libmicrohttpd tutorial
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
//
// Helpful reference project for libmicrohttpd
// https://github.com/PedroFnseca/rest-api-C/blob/main/src/main.c

#define PORT 10279
#define MAX_POST_BODY_SIZE 1023
#define LOG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#define QUEUE_ERROR_RESPONSE(ERRMSG) do { struct MHD_Response* __response = MHD_create_response_from_buffer(strlen(ERRMSG), (void*)ERRMSG, MHD_RESPMEM_PERSISTENT);\
                                MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, __response);\
                                MHD_destroy_response(__response);\
                                LOG(ERRMSG); } while(0)
const char* _leaderboardDir = "/leaderboards";
const char* _templateInsertStmt = "INSERT into %s values(@name, @score, @time);";

enum {
    CONNECTIONTYPE_UNUSED,
    CONNECTIONTYPE_GET,
    CONNECTIONTYPE_POST
};

sqlite3* db;

typedef struct _handlerParam HandlerParam;
struct _handlerParam { char* data; }; // TODO what is this used for? can it be chopped

typedef struct _connectionInfo ConnectionInfo;
struct _connectionInfo {
    int connectionType;
    int idx;
    char buf[MAX_POST_BODY_SIZE + 1];
};

// data is received incrementally through the body of a request
enum MHD_Result processIncrementalData(void* _ci, const char* data, size_t size) {
    ConnectionInfo* ci = (ConnectionInfo*) _ci;
    //LOG("pID: %s", data);
    if (ci->idx + size >= MAX_POST_BODY_SIZE) { // exceed max POST size. for now, wipe buf and reject all further data
        memset(ci->buf, 0, MAX_POST_BODY_SIZE);
        ci->idx = MAX_POST_BODY_SIZE;
        return MHD_NO; 
    }
    
    // simply append newly received data to buf
    memcpy(&(ci->buf[ci->idx]), data, size);
    ci->idx = ci->idx + size;
    return MHD_YES;
}

// callback function when a request is FULLY handled (response constructed + queued)
void completeRequest(void* cls, struct MHD_Connection* connection, void** req_cls, enum MHD_RequestTerminationCode toe) {
    LOG("completeRequest callback invoked\n");
    ConnectionInfo* ci = (ConnectionInfo*) *req_cls;
    free(ci);
}

void processRequest(ConnectionInfo* ci, struct MHD_Connection* connection) {
    ci->buf[ci->idx] = '\x00';
    struct MHD_Response* r = MHD_create_response_from_buffer_static(strlen(ci->buf), (void*)ci->buf);
    MHD_add_response_header(r, "content-type", "text/plain");
    MHD_queue_response(connection, MHD_HTTP_OK, r);
    MHD_destroy_response(r);
    LOG("processRequest called... queued an echo response:\n");
    LOG("%s\n", ci->buf);
}

enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {
    LOG("%s '%s' request for '%s' using version '%s'\n", *req_cls ? "Followup" : "New", method, url, version);

    // check that they're accessing /leaderboards/; if not, respond with error
    if (strncmp(url, _leaderboardDir, strlen(_leaderboardDir)) != 0) {
        QUEUE_ERROR_RESPONSE("improper attempt to access leaderboards API");
        return MHD_YES;
    }

    // is this a new connection? if so, create a ConnectionInfo for it; which will be freed by completeRequest()
    if (*req_cls == NULL) {
        ConnectionInfo* ci = (ConnectionInfo*)malloc(sizeof(ConnectionInfo));
        if (ci == NULL) { LOG("major error: failed to allocate memory for ConnectionInfo\n"); exit(1); }
        ci->idx = 0;

        if (strcmp(method, "GET") == 0) ci->connectionType = CONNECTIONTYPE_GET;
        else if (strcmp(method, "POST") == 0) ci->connectionType = CONNECTIONTYPE_POST;
        else return MHD_NO;

        *req_cls = ci;
        return MHD_YES;
    } 

    // not a new connection: if more data available, store incremental data received
    ConnectionInfo* ci = (ConnectionInfo*) *req_cls;
    if (*upload_data_size != 0) { 
        int ret = processIncrementalData(ci, upload_data, *upload_data_size);
        *upload_data_size = 0; // processed this data, reset the "counter"
        return ret;
    }

    // not a new connection, all data received: process and construct response, all done
    processRequest(ci, connection);
    return MHD_YES;
}

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
