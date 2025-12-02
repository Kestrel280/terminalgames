#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <time.h>
#define STRING_BASE_CAPACITY 8

extern const char* leaderboardGetUrl;
extern const char* leaderboardPostUrl;
extern const char* gameName;

// small helpers for curl callbacks
typedef struct _string {
    char* data;
    int capacity;
    int size;
} String;

typedef struct _outbound {
    const char* readPtr;
    int remaining;
} Outbound;

// callback for CURL GET request: may be invoked multiple times for a single request
size_t curlWriteCallback(void* curledData, size_t sz, size_t count, void* hStr);

// callback for CURL POST request: may be invoked multiple times for a single request
size_t curlReadCallback(void*, size_t sz, size_t count, void* hOb);

// helper to display error
void printError(const char* errStr, const char* reason);

// monolith entrypoint that sets up CURL request, executes, and displays results
void leaderboardDisplay(const char* gameName);

// monolith entrypoint to construct JSON message and CURL to leaderboard endpoint
void leaderboardSubmitScore(const char* gameName, const char* name, uint64_t score, time_t time);

#endif
