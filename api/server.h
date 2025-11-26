#ifndef SERVER_H
#define SERVER_H
#include "utils.h"

#define MAX_POST_BODY_SIZE 1023
#define PORT 10279
#define QUEUE_ERROR_RESPONSE(ERRMSG) do { struct MHD_Response* __response = MHD_create_response_from_buffer(strlen(ERRMSG), (void*)ERRMSG, MHD_RESPMEM_PERSISTENT);\
                                MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, __response);\
                                MHD_destroy_response(__response);\
                                LOG(ERRMSG); } while(0)

// GNU libmicrohttpd references
// https://git.gnunet.org/libmicrohttpd.git/plain/src/include/microhttpd.h
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
// https://github.com/Karlson2k/libmicrohttpd/blob/master/doc/examples/simplepost.c
// https://github.com/PedroFnseca/rest-api-C/blob/main/src/main.c

typedef enum {
    CONNECTION_TYPE_UNUSED,
    CONNECTION_TYPE_GET,
    CONNECTION_TYPE_POST
} ConnectionType;

typedef struct _connectionInfo ConnectionInfo;
struct _connectionInfo {
    char** resourceChain; // /leaderboards/game/snake -> ["leaderboards", "game", "snake"]
    int resourceChainSize;
    int idx;
    char buf[MAX_POST_BODY_SIZE + 1];
    ConnectionType connectionType;
};

// main connection callback
// invoked at several points:
//      1. upon initial connection, after headers are processed by MHD
//      2. 0 or more times with incremental upload_data updates; this function is responsible for handling it and updating *upload_data_size with the # of UNPROCESSED bytes
//      3. upon full request body received
enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls);

// data is received incrementally through the body of a request
enum MHD_Result processIncrementalData(void* _ci, const char* data, size_t size);

// once a request has been fully received, the ConnectionInfo is passed to this function to process the request and generate/queue a response
void processRequest(ConnectionInfo* ci, struct MHD_Connection* connection);

// callback function when a request is FULLY handled (response constructed + queued)
void completeRequest(void* cls, struct MHD_Connection* connection, void** req_cls, enum MHD_RequestTerminationCode toe);


#endif
