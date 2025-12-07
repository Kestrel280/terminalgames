#ifndef LEADERBOARDAPI_H
#define LEADERBOARDAPI_H

#include <stdbool.h>
#include "sdserver.h"

#define PORT 10279
#define NAME_MAX_LENGTH 12

extern const char* leaderboardEndpoint;

// ordering of the columns in leaderboard database tables. consistent across all game tables
enum {
    COLUMN_GAME,
    COLUMN_NAME,
    COLUMN_SCORE,
    COLUMN_TIME
};

extern const char* GAMES[];

// callback passed to sdServer: upon receiving a full request, pass it to this function
// this function will dispatch it to leaderboard-specific functionality
void leaderboardProcessRequest(ConnectionInfo* ci, struct MHD_Connection* connection);

// get from leaderboard.
// 'request' is a json-formatted string specifying which game to fetch, and TODO filters for user, # of scores to report, etc
// returns a malloc'd json-formatted string containing answer, which MUST BE FREED BY CALLER
bool leaderboardApiGet(ConnectionInfo* ci, char** pOut);

// post to leaderboard.
// 'request' is a json-formatted string specifying which game to post to, along with score information
// returns true if successfully posted, false if not
bool leaderboardApiPost(ConnectionInfo* ci);

#endif
