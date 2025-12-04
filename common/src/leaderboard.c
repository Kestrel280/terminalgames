#include <curl/curl.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <json-c/json_tokener.h>
#include "../include/leaderboard.h"
#include <time.h>


// TODO move this to somewhere central
#define LOG(...) do {fprintf(stderr, __FILE__); fprintf(stderr, __VA_ARGS__); } while (0)

/* 
 * logic here is simple enough that it's not factored out to be reusable --
 * just setting up a CURL request, executing, parsing JSON, and displaying.
 * functions are monolithic and there's not much error checking because the stakes are so low.
*/

static int width, height;
static const char* pressAnyButtonStr = "Press any key to return to menu";

size_t curlWriteCallback(void* data, size_t sz, size_t count, void* hStr) {
    String* pStr = (String*)hStr;
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

// callback invoked when invoking a POST request with curl
size_t curlReadCallback(void* buf, size_t sz, size_t count, void* hOb) {
    Outbound* pOb = (Outbound*)hOb;
    size_t amountToCopy = pOb->remaining > (sz * count) ? sz * count : pOb->remaining;
    memcpy(buf, pOb->readPtr, amountToCopy);
    pOb->readPtr += amountToCopy;
    pOb->remaining -= amountToCopy;
    return amountToCopy;
}

void printError(const char* errStr, const char* reason) {
    int tmp = is_nodelay(stdscr);
    nodelay(stdscr, false);
    clear();
    mvprintw(height / 2, (width - strlen(errStr)) / 2, "%s", errStr);
    mvprintw(height / 2 + 1, (width - (strlen(reason))) / 2, "%s", reason);
    mvprintw(height / 2 + 2, (width - (strlen(pressAnyButtonStr))) / 2, "%s", pressAnyButtonStr);
    refresh();
    getch();
    nodelay(stdscr, tmp);
}

void leaderboardDisplay(const char* gameName) {
    static const char* errStr = "Could not fetch leaderboard:";

    getmaxyx(stdscr, height, width);
    clear();

    CURL* curl;
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    // res = 1; // force failure, for testing
    if (res) { printError(errStr, "Failed to initialize CURL environment"); return; }

    curl = curl_easy_init();
    if (!curl) { printError(errStr, "Failed to initialize CURL object"); curl_global_cleanup(); return; }

    String string;
    string.data = (char*)malloc(sizeof(char) * STRING_BASE_CAPACITY);
    string.capacity = STRING_BASE_CAPACITY;
    string.size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, leaderboardGetUrl);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &string);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) { printError(errStr, "Failed to fetch leaderboard data; server may be offline"); curl_global_cleanup(); return; }

    json_object* jobj = json_tokener_parse(string.data);
    if (jobj == NULL) { printError(errStr, "Failed to parse leaderboard data; server may have returned error"); curl_global_cleanup(); return; }

    json_object* jarr;
    json_object_object_get_ex(jobj, gameName, &jarr);
    if (jarr == NULL) { printError(errStr, "No entries in leaderboard data!"); curl_global_cleanup(); return; }

    size_t jsize = json_object_array_length(jarr);

    mvprintw(2, (width - 45) / 2, "%12s %12s   %20s", "NAME", "SCORE", "TIME");
    int i = 0;
    for (; i < jsize; i++) {
        json_object *jname, *jscore, *jtime;
        json_object* jel = json_object_array_get_idx(jarr, i);
        json_object_object_get_ex(jel, "name", &jname);
        json_object_object_get_ex(jel, "score", &jscore);
        json_object_object_get_ex(jel, "time", &jtime);
        const char* name = json_object_get_string(jname);
        uint64_t score = json_object_get_int64(jscore);
        time_t t = (time_t)json_object_get_int64(jtime);
        struct tm* ti;
        ti = localtime(&t);
        char ft[18]; // formatted time
        strftime(ft, 18, "%R %d %b %Y", ti);
        mvprintw(4 + i, (width - 45) / 2 - 4, "%2d.", i + 1);
        mvprintw(4 + i, (width - 45) / 2, "%12s %12lu   %20s", name, score, ft);
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

void leaderboardSubmitScore(const char* gameName, uint64_t score) {
    // get current time and prompt for name
    time_t t = time(NULL);

    // operate in stdscr space since game window might be too small
    getmaxyx(stdscr, height, width);

    char enterNamePrompt[100];
    sprintf(enterNamePrompt, "ENTER NAME (MAX %d):", NAME_MAX_LENGTH);

    char playerName[NAME_MAX_LENGTH + 1];
    playerName[0] = '\x00';

    echo();
    curs_set(1);
    do {
        erase();
        sprintf(playerName, "SCORE: %ld", score);
        mvprintw(2, (width - strlen(playerName)) / 2, "%s", playerName);
        mvprintw(4, (width - strlen(enterNamePrompt)) / 2, "%s", enterNamePrompt);
        mvaddch(5, (width - NAME_MAX_LENGTH) / 2 - 1, '<');
        mvaddch(5, (width + NAME_MAX_LENGTH) / 2, '>');
        mvgetnstr(5, (width - NAME_MAX_LENGTH) / 2, playerName, NAME_MAX_LENGTH);
        if (strlen(playerName) > 0) break;
        erase();
        mvprintw(height / 2, (width - strlen("Cannot submit time")) / 2, "Cannot submit time");
        mvprintw(height / 2 + 1, (width - strlen("Name cannot be empty")) / 2, "Name cannot be empty");
        mvprintw(height / 2 + 1, (width - strlen("Press any key to try again")) / 2, "Press any key to try again");
        refresh();
        getch();
    } while (1);

    curs_set(0); // TODO this makes the assumption that the game wants cursor and echo off. should store previous states and restore them
    noecho();
    playerName[NAME_MAX_LENGTH] = '\x00';

    /* post to leaderboard */

    static const char* errStr = "Submission failed";
    json_object* jobj = json_object_new_object();
    if (!jobj) { printError(errStr, "Memory error with json-c"); return; }

    json_object* jgame = json_object_new_string(gameName);
    if (!jgame) { json_object_put(jobj); printError(errStr, "Memory error with json-c"); return; }
    json_object_object_add(jobj, "game", jgame);

    json_object* jentry = json_object_new_object();
    json_object* jname = json_object_new_string(playerName);
    json_object* jscore = json_object_new_int64(score);
    json_object* jtime = json_object_new_int64(t);

    if (!(jentry && jname && jscore && jtime)) { printError(errStr, "Memory error with json-c"); return; }
    json_object_object_add(jentry, "name", jname);
    json_object_object_add(jentry, "score", jscore);
    json_object_object_add(jentry, "time", jtime);
    json_object_object_add(jobj, "entry", jentry);

    const char* data = json_object_to_json_string(jobj);
    Outbound ob;
    ob.readPtr = data;
    ob.remaining = strlen(data);

    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res) { printError(errStr, "Failed to initialize CURL environment"); curl_global_cleanup(); json_object_put(jobj); return; }
    CURL* curl = curl_easy_init();
    if(!curl) { printError(errStr, "Failed to create CURL object"); curl_global_cleanup(); json_object_put(jobj); return; }

    curl_easy_setopt(curl, CURLOPT_URL, leaderboardPostUrl);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, curlReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ob);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)ob.remaining);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) { LOG("POST error code: %d\n", res); printError(errStr, "Unable to POST"); curl_easy_cleanup(curl); curl_global_cleanup(); json_object_put(jobj); return; }
    long rcode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);
    if (rcode != 200L) { char buf[128]; sprintf(buf, "Unable to POST: error %ld (is the API running?)", rcode); printError(errStr, buf); curl_easy_cleanup(curl); curl_global_cleanup(); json_object_put(jobj); return; }

    printError("CONGRATULATIONS!", "YOUR SCORE WAS SUBMITTED");

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    json_object_put(jobj);
    return;
}
