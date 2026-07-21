#ifndef LEADERBOARDAPI_H
#define LEADERBOARDAPI_H

#include <stdbool.h>
#include "sdserver.h"

#define PORT 10379
extern const char* wellnessEndpoint;

// callback passed to sdServer: upon receiving a full request, pass it to this function
// this function will dispatch it to leaderboard-specific functionality
void dataProcessRequest(ConnectionInfo* ci, struct MHD_Connection* connection);

#endif
