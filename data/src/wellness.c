#include <string.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <uriparser/Uri.h>
#include "dataapi.h"
#include "wellness.h"

const char* wellnessEndpoint = "data";

bool wellnessGet(ConnectionInfo* ci, char** pOut) {
    const char* err = "data api: invalid endpoint or GET not supported for this endpoint";
    int len = strlen(err);
    char* errOut = (char*)malloc(sizeof(char*) * (len + 1));
    memcpy(errOut, err, len);
    errOut[len] = '\x00';
    *pOut = errOut;
    return false;
}

bool wellnessPost(ConnectionInfo* ci) {
    char* req = ci->buf;
    char* afterLast = req;
    while (*afterLast) afterLast++;

    UriQueryListA *queryList, *currentKv;
    int itemCount;
    if (uriDissectQueryMallocA(&queryList, &itemCount, req, afterLast) != URI_SUCCESS) {
        LOG("malformed query string < %s >\n", req);
        return false;
    }

    currentKv = queryList;
    while (currentKv) {
        LOG("key: %s | value: %s\n", currentKv->key, currentKv->value);
        currentKv = currentKv->next;
    }

    uriFreeQueryListA(queryList);

    LOG("%s\n", req);
    return false;
}
