#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include "sdserver.h"

struct MHD_Daemon* sdServerStart(serverRequestProcessor reqProc, int port) {
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL,
                              &sdServerConnectionCallback, reqProc,
                              MHD_OPTION_NOTIFY_COMPLETED, sdServerCompleteRequest, NULL,
                              MHD_OPTION_END);
    return daemon;
}

int sdServerShutdown(struct MHD_Daemon* daemon) {
    MHD_stop_daemon(daemon);
    return 0;
}

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

void sdServerCompleteRequest(void* cls, struct MHD_Connection* connection, void** req_cls, enum MHD_RequestTerminationCode toe) {
    //LOG("completeRequest callback invoked\n");
    ConnectionInfo* ci = (ConnectionInfo*) *req_cls;
    for (int i = 0; i < ci->resourceChainSize; i++) free(ci->resourceChain[i]);
    free(ci->resourceChain);
    free(ci);
}

enum MHD_Result sdServerConnectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {

    // unpack the server request processor function pointer
    serverRequestProcessor sRP = cls;

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
    // TODO invoke callback
    sRP(ci, connection);
    return MHD_YES;
}
