#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include "server.h"
#include "leaderboard.h"

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

void completeRequest(void* cls, struct MHD_Connection* connection, void** req_cls, enum MHD_RequestTerminationCode toe) {
    //LOG("completeRequest callback invoked\n");
    ConnectionInfo* ci = (ConnectionInfo*) *req_cls;
    for (int i = 0; i < ci->resourceChainSize; i++) free(ci->resourceChain[i]);
    free(ci->resourceChain);
    free(ci);
}

void processRequest(ConnectionInfo* ci, struct MHD_Connection* connection) {
    ci->buf[ci->idx] = '\x00';
    LOG("responding to request <%s>\n", ci->buf);

    char* rtext;
    struct MHD_Response* r;
    if (ci->resourceChainSize == 0) { QUEUE_ERROR_RESPONSE("no API endpoint here"); return; }

    if (strcmp(ci->resourceChain[0], leaderboardEndpoint) == 0) {
        switch (ci->connectionType) {
            case CONNECTION_TYPE_GET: {
                bool success = leaderboardGet(ci, &rtext);
                r = MHD_create_response_from_buffer_with_free_callback(strlen(rtext), rtext, &free);
                MHD_add_response_header(r, "content-type", success ? "application/json" : "text/plain");
                break;
            }
            case CONNECTION_TYPE_POST: {
                rtext = leaderboardPost(ci) ? "successfully posted to leaderboard" : "failed to post to leaderboard: check API spec";
                r = MHD_create_response_from_buffer(strlen(rtext), rtext, MHD_RESPMEM_PERSISTENT);
                MHD_add_response_header(r, "content-type", "text/plain");
                break;
            }
            default: QUEUE_ERROR_RESPONSE("http method not supported"); return;
        }
    } else {
        QUEUE_ERROR_RESPONSE("no API endpoint here"); return;
    }
    LOG("\t responding with <%s>\n", rtext);
    MHD_queue_response(connection, MHD_HTTP_OK, r);
    MHD_destroy_response(r);
}

enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {
    //LOG("%s '%s' request for '%s' using version '%s'\n", *req_cls ? "Followup" : "New", method, url, version);

    // check that they're accessing /leaderboards...; if not, respond with error
    /*if (strncmp(url, urlEndpoint, strlen(urlEndpoint)) != 0) {
        QUEUE_ERROR_RESPONSE("improper attempt to access leaderboards API");
        return MHD_YES;
    }
    */

    // is this a new connection? if so, create a ConnectionInfo for it; which will be freed by completeRequest()
    if (*req_cls == NULL) {
        ConnectionInfo* ci = (ConnectionInfo*)malloc(sizeof(ConnectionInfo));
        if (ci == NULL) { LOG("major error: failed to allocate memory for ConnectionInfo\n"); exit(1); }
        ci->idx = 0;

        // -- extract subresources: e.g. "/leaderboards/game/snake" -> ["leaderboards", "game", "snake"] TODO clean this up
        // first, create writable copy of url
        int numResources = 0;
        int urlLen = strlen(url);
        char *urlCpy = (char*)malloc(sizeof(char) * (urlLen + 1)), *token, *savePtr;
        memcpy(urlCpy, url, urlLen);
        urlCpy[urlLen] = '\x00';

        // do a first pass through the url to determine how many subpaths there are
        for (char* str = urlCpy; ; numResources++, str = NULL) { token = strtok_r(str, "/", &savePtr); if (token == NULL) break; }
        ci->resourceChainSize = numResources;
        ci->resourceChain = (char**)malloc(sizeof(char*) * (numResources + 1));

        // now do another pass to actually save each subpath
        memcpy(urlCpy, url, urlLen);
        urlCpy[urlLen] = '\x00';
        int _i = 0;
        for (char* str = urlCpy; ; _i++, str = NULL) {
            token = strtok_r(str, "/", &savePtr);
            if (token == NULL) break; 
            else {
                ci->resourceChain[_i] = malloc(sizeof(char) * (strlen(token) + 1));
                memcpy(ci->resourceChain[_i], token, strlen(token));
                ci->resourceChain[_i][strlen(token)] = '\x00';
            }
        }

        // -- subresource collection clean up
        free(urlCpy);
        /*
        LOG("numResources = %d: ", numResources);
        for (int i = 0; i < ci->resourceChainSize; i++) LOG("%s . ", ci->resourceChain[i]);
        LOG("\n");
        */

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
