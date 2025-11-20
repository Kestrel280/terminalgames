#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define LOG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#define PORT 10279
#define QUEUE_ERROR_RESPONSE() do { struct MHD_Response* __response = MHD_create_response_from_buffer(strlen(errstr), (void*)errstr, MHD_RESPMEM_PERSISTENT);\
                                MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, __response);\
                                MHD_destroy_response(__response); } while(0)
const char* _leaderboardDir = "/leaderboards";
const char* errstr = "internal server error occurred";


// GNU libmicrohttpd tutorial
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
//
// Helpful reference project for libmicrohttpd
// https://github.com/PedroFnseca/rest-api-C/blob/main/src/main.c

typedef struct _handlerParam HandlerParam;
struct _handlerParam {
    char* data;
};

// for use with MHD_get_connection_values: simply prints header pairs
enum MHD_Result printKey(void* cls, enum MHD_ValueKind kind, const char* key, const char* value) {
    printf("%s: %s\n", key, value);
    return MHD_YES;
}

enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {
    struct MHD_Response* response;

    printf("New '%s' request for '%s' using version '%s'\n", method, url, version);

    // MHD_get_connection_values() -> 3rd argument is a fn pointer; calls this fn for each key-value pair in the request
    //MHD_get_connection_values(connection, MHD_HEADER_KIND, &printKey, NULL);

    // GET request
    if (strcmp(method, "GET") == 0) {

        // check that they're accessing /leaderboards/; if not, respond with error
        if (strncmp(url, _leaderboardDir, strlen(_leaderboardDir)) != 0) {
            QUEUE_ERROR_RESPONSE();
            LOG("... improper access to leaderboards directory\n");
            return MHD_YES;
        }

        // open requested leaderboard file; if not found, respond with error
        int fd = open(url+1, O_RDONLY, NULL);
        if (fd == -1) {
            QUEUE_ERROR_RESPONSE();
            LOG("... file not found in leaderboards dir\n");
            return MHD_YES;
        }
        struct stat fdStat;
        fstat(fd, &fdStat);

        // respond with file contents
        response = MHD_create_response_from_fd_at_offset64((size_t)fdStat.st_size, fd, 0);
        MHD_add_response_header(response, "Content-Type", "text/csv");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // not a GET request; error (for now)
    QUEUE_ERROR_RESPONSE();
    return MHD_YES;
}


int main(int argc, char* argv[]) {
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                              &connectionCallback, NULL, MHD_OPTION_END);

    if (daemon == NULL) return 1;

    getchar(); // block; server (listen + response callback) is running in its own thread, so just keep the process alive

    MHD_stop_daemon(daemon);
    return 0;
}
