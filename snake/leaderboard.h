#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#define STRING_BASE_CAPACITY 8
extern const char* leaderboardGetUrl;

// small helper
typedef struct _string {
    char* data;
    int capacity;
    int size;
} String;

// callback for CURL request: may be invoked multiple times for a single request
size_t curlWriteCallback(void* curledData, size_t sz /* always 1 */, size_t count, void* ppOut);

// helper to display error
void printError(const char* reason);

// monolith entrypoint that sets up CURL request, executes, and displays results
void leaderboardDisplay();

#endif
