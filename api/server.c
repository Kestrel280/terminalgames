#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include "server.h"

extern const char* urlEndpoint;

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
    if (strncmp(url, urlEndpoint, strlen(urlEndpoint)) != 0) {
        QUEUE_ERROR_RESPONSE("improper attempt to access leaderboards API");
        return MHD_YES;
    }

    // is this a new connection? if so, create a ConnectionInfo for it; which will be freed by completeRequest()
    if (*req_cls == NULL) {
        ConnectionInfo* ci = (ConnectionInfo*)malloc(sizeof(ConnectionInfo));
        if (ci == NULL) { LOG("major error: failed to allocate memory for ConnectionInfo\n"); exit(1); }
        ci->idx = 0;

        if (strcmp(method, "GET") == 0) ci->connectionType = CONNECTION_TYPE_GET;
        else if (strcmp(method, "POST") == 0) ci->connectionType = CONNECTION_TYPE_POST;
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
