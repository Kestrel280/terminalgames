#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <stdbool.h>
#include "server.h"

// get from leaderboard.
// 'request' is a json-formatted string specifying which game to fetch, and TODO filters for user, # of scores to report, etc
// returns a malloc'd json-formatted string containing answer, which MUST BE FREED BY CALLER
char* leaderboardGet(ConnectionInfo* ci);

// post to leaderboard.
// 'request' is a json-formatted string specifying which game to post to, along with score information
// returns true if successfully posted, false if not
bool leaderboardPost(ConnectionInfo* ci);

#endif
