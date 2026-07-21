#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include "utils.h"
#include "dataapi.h"
#include "sdserver.h"
#include "wellness.h"

#define EXPAND_AND_LOG_SQL_STATEMENT(stmt) do { char* __expStr__ = sqlite3_expanded_sql(stmt); LOG("\t expanded SQL statement: '%s'\n", __expStr__); sqlite3_free(__expStr__); } while(0)
static inline bool SAME_STRING(const char* s1, const char* s2) { return (strcmp(s1, s2) == 0); }
extern sqlite3* db;

void dataProcessRequest(ConnectionInfo* ci, struct MHD_Connection* connection) {
    ci->buf[ci->idx] = '\x00';
    LOG("responding to request <%s>\n", ci->buf);

    char* rtext;
    struct MHD_Response* r;
    if (ci->resourceChainSize == 0) { QUEUE_ERROR_RESPONSE("no API endpoint here"); return; }

    if (strcmp(ci->resourceChain[0], wellnessEndpoint) == 0) {
        switch (ci->connectionType) {
            case CONNECTION_TYPE_GET: {
                bool success = wellnessGet(ci, &rtext);
                r = MHD_create_response_from_buffer_with_free_callback(strlen(rtext), rtext, &free);
                MHD_add_response_header(r, "content-type", success ? "application/json" : "text/plain");
                break;
            }
            case CONNECTION_TYPE_POST: {
                rtext = wellnessPost(ci) ? "success" : "failure";
                r = MHD_create_response_from_buffer(strlen(rtext), rtext, MHD_RESPMEM_PERSISTENT);
                MHD_add_response_header(r, "content-type", "text/plain");
                break;
            }
            default: QUEUE_ERROR_RESPONSE("http method not supported"); return;
        }
    } else {
        QUEUE_ERROR_RESPONSE("no API endpoint here");
        return;
    }
    LOG("\t responding with <%s>\n", rtext);
    MHD_queue_response(connection, MHD_HTTP_OK, r);
    MHD_destroy_response(r);
}
