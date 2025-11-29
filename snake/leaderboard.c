#include <curl/curl.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <json-c/json_tokener.h>
#include "leaderboard.h"

/* 
 * logic here is simple enough that it's not factored out to be reusable --
 * just setting up a CURL request, executing, parsing JSON, and displaying.
 * functions are monolithic and there's not much error checking because the stakes are so low.
*/

const char* leaderboardGetUrl = "https://samdowney.dev/leaderboards/game/snake";

static int width, height;
static const char* errStr = "Could not fetch leaderboard:";
static const char* pressAnyButtonStr = "Press any button to return to menu";

size_t curlWriteCallback(void* data, size_t __sz__always__1__, size_t count, void* ppOut) {
    String* pStr = (String*)ppOut;
    if (pStr->size + count >= pStr->capacity) {
        pStr->data = realloc(pStr->data, pStr->size + count + 1);
        pStr->capacity = pStr->size + count + 1;
    }
    memcpy(&(pStr->data[pStr->size]), data, count);
    pStr->size += count;
    pStr->data[pStr->size] = '\x00';
    // fprintf(stderr, "write callback with %d bytes, pStr->size now %d / capacity now %d\n", count, pStr->size, pStr->capacity);
    return count;
}

void printError(const char* reason) {
    mvprintw(height / 2, (width - strlen(errStr)) / 2, "%s", errStr);
    mvprintw(height / 2 + 1, (width - (strlen(reason))) / 2, "%s", reason);
    mvprintw(height / 2 + 2, (width - (strlen(pressAnyButtonStr))) / 2, "%s", pressAnyButtonStr);
    getch();
    refresh();
}

void leaderboardDisplay() {

    getmaxyx(stdscr, height, width);
    clear();

    CURL* curl;
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    // res = 1; // force failure, for testing
    if (res) { printError("Failed to initialize CURL environment"); return; }

    curl = curl_easy_init();
    if (!curl) { printError("Failed to initialize CURL object"); curl_global_cleanup(); return; }

    String string;
    string.data = (char*)malloc(sizeof(char) * STRING_BASE_CAPACITY);
    string.capacity = STRING_BASE_CAPACITY;
    string.size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, leaderboardGetUrl);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &string);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) { printError("Failed to fetch leaderboard data; server may be offline"); curl_global_cleanup(); return; }

    json_object* jobj = json_tokener_parse(string.data);
    if (jobj == NULL) { printError("Failed to parse leaderboard data; server may have returned error"); curl_global_cleanup(); return; }

    json_object* jarr = json_object_object_get(jobj, "snake");
    if (jarr == NULL) { printError("No entries in leaderboard data!"); curl_global_cleanup(); return; }

    size_t jsize = json_object_array_length(jarr);

    mvprintw(2, (width - 28) / 2, "%12s %8s %8s", "NAME", "SCORE", "TIME");
    int i = 0;
    for (; i < jsize; i++) {
        json_object* jel = json_object_array_get_idx(jarr, i);
        json_object* jname = json_object_object_get(jel, "name");
        json_object* jscore = json_object_object_get(jel, "score");
        json_object* jtime = json_object_object_get(jel, "time");
        const char* name = json_object_get_string(jname);
        int score = json_object_get_int(jscore);
        int time = json_object_get_int(jtime);
        mvprintw(4 + i, (width - 28) / 2 - 4, "%2d.", i + 1);
        mvprintw(4 + i, (width - 28) / 2, "%12s %8d %8d", name, score, time);
    }

    mvprintw(6 + i, (width - strlen(pressAnyButtonStr)) / 2, "%s", pressAnyButtonStr);

    //printw("%s", string.data);
    getch();

    free(string.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    json_object_put(jobj);

    return;
}
