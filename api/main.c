#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#define PORT 10279

// GNU libmicrohttpd tutorial
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
//
// Helpful reference project for libmicrohttpd
// https://github.com/PedroFnseca/rest-api-C/blob/main/src/main.c

enum MHD_Result connectionCallback(void* cls, struct MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* version, const char* upload_data,
                        size_t* upload_data_size, void** req_cls) {
    const char* __dummyResponse = "<html><body>dummy response</body></html>";
    struct MHD_Response* response;
    int ret;

    response = MHD_create_response_from_buffer(strlen(__dummyResponse), (void*)__dummyResponse, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    //MHD_destroy_response(response);
    return ret;
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
