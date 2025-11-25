#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include "leaderboard.h"
#include "utils.h"

const char* _templateInsertStmt = "INSERT into %s values(@name, @score, @time);";
extern sqlite3* db;

char* leaderboardGet(char* request) {
    char* out = (char*)malloc(sizeof(char) * 10);
    for (int i = 0; i < 5; i++) out[i] = "asdfg"[i];
    out[5] = '\x00';
    return out;
}

bool leaderboardPost(char* request) {
    json_object* jobj = json_tokener_parse(request);
    if (!jobj) { LOG("invalid json provided to leaderboardPost\n"); return false; }

    // extract game
    // this is the only value we'll need to "sanitize"/validate, since we'll be sprintf'ing it into our SQL statement (other values are prepared statement parameters)
    // upstream, server is will reject messages with more than MAX_POST_BODY_SIZE (server.h) bytes
    // so, don't need to worry about oversized entries here
    json_object* jgame;
    const char* game;
    if (!json_object_object_get_ex(jobj, "game", &jgame)) { LOG("no 'game' key in POST body\n"); json_object_put(jobj); return false; }
    game = json_object_get_string(jgame);
    for (const char* pc = &(game[0]); *pc != '\x00'; pc++) {
        if ((*pc < 'a') || (*pc > 'z')) {
            LOG("invalid 'game' specified in POST body\n");
            json_object_put(jobj);
            return false;
        }
    }

    // extract scoreentry
    json_object* jentry;
    if (!json_object_object_get_ex(jobj, "entry", &jentry)) { LOG("no 'entry' key in POST body\n"); json_object_put(jobj); return false; }

    // extract time, score, and name from scoreentry
    json_object *jname, *jtime, *jscore;
    const char* name;
    int64_t score, time;
    if (!json_object_object_get_ex(jentry, "name", &jname)) { LOG("no 'entry->name' key in POST body\n"); json_object_put(jobj); return false; }
    if (!json_object_object_get_ex(jentry, "score", &jscore)) { LOG("no 'entry->score' key in POST body\n"); json_object_put(jobj); return false; }
    if (!json_object_object_get_ex(jentry, "time", &jtime)) { LOG("no 'entry->time' key in POST body\n"); json_object_put(jobj); return false; }
    name = json_object_get_string(jname);
    score = json_object_get_int64(jscore);
    time = json_object_get_int64(jtime);
    if ((score <= 0) || (time <= 0)) { LOG("invalid score/time key in POST body\n"); json_object_put(jobj); return false; }

    //LOG("leaderboardPost extracted game='%s', name='%s', score=%ld, time=%ld\n", game, name, score, time);

    // construct SQL insert-into statement
    char _insertStmt[strlen(_templateInsertStmt) + strlen(game)];
    sprintf(_insertStmt, _templateInsertStmt, game);
    sqlite3_stmt* insertStmt;
    if (sqlite3_prepare_v2(db, _insertStmt, -1, &insertStmt, NULL) != SQLITE_OK) { // compile statement
        LOG("error compiling SQL insert-into statement\n");
        json_object_put(jobj);
        return false;
    }
    sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@name"), name, strlen(name), SQLITE_STATIC); // SQLITE_STATIC = i am responsible for memory of the string
    sqlite3_bind_int64(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@score"), score);
    sqlite3_bind_int64(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@time"), time);

    // execute SQL statement
    char* expandedStmt = sqlite3_expanded_sql(insertStmt); // must be freed by sqlite3_free
    LOG("\t executing SQL statement '%s'\n", expandedStmt);
    sqlite3_step(insertStmt);

    //clean up
    sqlite3_free(expandedStmt);
    sqlite3_finalize(insertStmt);
    json_object_put(jobj);
    return true;
}
