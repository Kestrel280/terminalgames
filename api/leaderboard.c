#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include "leaderboard.h"
#include "utils.h"

#define EXPAND_AND_LOG_SQL_STATEMENT(stmt) do { char* __expStr__ = sqlite3_expanded_sql(stmt); LOG("\t expanded SQL statement: '%s'\n", __expStr__); sqlite3_free(__expStr__); } while(0)

// games which can be queried
const char* GAMES[] = { "snake" , "anotherfakegame" };

static inline bool SAME_STRING(const char* s1, const char* s2) { return (strcmp(s1, s2) == 0); }

const char* _templateInsertStmt = "INSERT into %s values('%s', @name, @score, @time);";
const char* _templateGetPlayerStmt = "SELECT * FROM %s WHERE name=@name";
const char* _templateGetGameStmt = "SELECT * FROM %s ORDER BY score DESC, time ASC";
extern sqlite3* db;

char* leaderboardGet(ConnectionInfo* ci) {
    char* out;

    // prepare error output in case we need it
    const char* err = "invalid endpoint or GET not supported for this endpoint";
    int len = strlen(err);
    char* errOut = (char*)malloc(sizeof(char*) * (len + 1));
    memcpy(errOut, err, len);
    errOut[len] = '\x00';
    
    //char* request = ci->buf;
    if (ci->resourceChainSize != 3 || (!SAME_STRING(ci->resourceChain[1], "player") && !SAME_STRING(ci->resourceChain[1], "game"))) return errOut;

    sqlite3_stmt* getStmt;

    // requesting highscores for a given player
    // TODO this is also pretty messy. and lots of redundant calls to strlen
    if (SAME_STRING(ci->resourceChain[1], "player")) {
        // determine the size of our query statement: each game needs a SELECT statement, and each game after the first needs a "UNION " prefix
        char* playerName = ci->resourceChain[2];
        int reqlen = 0;
        for (int i = 0; i < (sizeof(GAMES) / sizeof(char*)); i++) {
            reqlen += strlen(_templateGetPlayerStmt) - strlen("%s") + strlen(GAMES[i]);
            if (i > 0) reqlen += strlen(" UNION ");
        }
        reqlen += 2; // extra char for ; and null terminator
        
        // allocate space for the query statement
        char* _getStmt = (char*)malloc(sizeof(char) * (reqlen + 1));

        // add each individual game query to the query statement
        for (int i = 0, pc = 0; i < (sizeof(GAMES) / sizeof(char*)); i++) {
            int len = strlen(_templateGetPlayerStmt) - strlen("%s") + strlen(GAMES[i]);
            if (i > 0) len += strlen(" UNION ");
            char buf[len];
            if (i > 0) memcpy(buf, " UNION ", strlen(" UNION "));
            sprintf(i > 0 ? (buf + strlen(" UNION ")) : buf, _templateGetPlayerStmt, GAMES[i]);
            for (int j = 0; j < len; j++, pc++) _getStmt[pc] = buf[j];
        }

        _getStmt[reqlen - 2] = ';';
        _getStmt[reqlen - 1] = '\x00';

        
        LOG("_getStmt (reqlen = %d): %s\n", reqlen, _getStmt);

        if (sqlite3_prepare_v2(db, _getStmt, -1, &getStmt, NULL) != SQLITE_OK) {
            LOG("error compiling SQL get-player statement\n");
            free(_getStmt);
            return errOut;
        }
        sqlite3_bind_text(getStmt, sqlite3_bind_parameter_index(getStmt, "@name"), playerName, strlen(playerName), SQLITE_STATIC);
        EXPAND_AND_LOG_SQL_STATEMENT(getStmt);
        
        json_object* jobj = json_object_new_object();

        // parse results into a JSON object
        while (sqlite3_step(getStmt) == SQLITE_ROW) {
            int score = sqlite3_column_int(getStmt, COLUMN_SCORE);
            int time = sqlite3_column_int(getStmt, COLUMN_TIME);
            const char* game = (const char*)sqlite3_column_text(getStmt, COLUMN_GAME); // TODO sqlite returns UTF-8; we are coercing into ASCII because game names are guaranteed to be ASCII (hopefully)
            LOG("\t\t player %s has score %d at time %d in game %s\n", playerName, score, time, game);

            json_object* jgameobj = json_object_object_get(jobj, game);
            if (jgameobj == NULL) { // create an array of entries for this game
                jgameobj = json_object_new_array();
                json_object_object_add(jobj, game, jgameobj);
            }

            json_object* jentry = json_object_new_object();
            json_object* jentryName = json_object_new_string(playerName);
            json_object* jentryScore = json_object_new_int(score);
            json_object* jentryTime = json_object_new_int(time);
            json_object_object_add(jentry, "name", jentryName);
            json_object_object_add(jentry, "score", jentryScore);
            json_object_object_add(jentry, "time", jentryTime);
            json_object_array_add(jgameobj, jentry);
        };

        size_t sz;
        const char* _str = json_object_to_json_string_length(jobj, JSON_C_TO_STRING_SPACED, &sz);
        out = (char*)malloc(sizeof(char) * (sz + 1));
        memcpy(out, _str, sz);
        out[sz] = '\x00';

        //clean up
        free(_getStmt);
        json_object_put(jobj);
        sqlite3_finalize(getStmt);
    }
    else if (SAME_STRING(ci->resourceChain[1], "game")) {
        char* gameName = ci->resourceChain[2];
        int reqlen = strlen(_templateGetGameStmt) + strlen(gameName) + 2; // 2 extra characters for ; and null terminator
        char* _getStmt = (char*)malloc(sizeof(char) * reqlen);
        sprintf(_getStmt, _templateGetGameStmt, gameName);

        if (sqlite3_prepare_v2(db, _getStmt, -1, &getStmt, NULL) != SQLITE_OK) {
            LOG("error compiling SQL get-game statement\n");
            free(_getStmt);
            return errOut;
        }
        EXPAND_AND_LOG_SQL_STATEMENT(getStmt);

        json_object* jobj = json_object_new_object();
        json_object* jgameobj = json_object_new_array();
        json_object_object_add(jobj, gameName, jgameobj);

        for (int i = 0; (i < 10) && (sqlite3_step(getStmt) == SQLITE_ROW); i++) {
            int score = sqlite3_column_int(getStmt, COLUMN_SCORE);
            int time = sqlite3_column_int(getStmt, COLUMN_TIME);
            const char* playerName = (const char*)sqlite3_column_text(getStmt, COLUMN_NAME);
            LOG("\t\t player %s has score %d at time %d in game %s\n", playerName, score, time, gameName);

            json_object* jentry = json_object_new_object();
            json_object* jentryName = json_object_new_string(playerName);
            json_object* jentryScore = json_object_new_int(score);
            json_object* jentryTime = json_object_new_int(time);
            json_object_object_add(jentry, "name", jentryName);
            json_object_object_add(jentry, "score", jentryScore);
            json_object_object_add(jentry, "time", jentryTime);
            json_object_array_add(jgameobj, jentry);
        }

        size_t sz;
        const char* _str = json_object_to_json_string_length(jobj, JSON_C_TO_STRING_SPACED, &sz);
        out = (char*)malloc(sizeof(char) * (sz + 1));
        memcpy(out, _str, sz);
        out[sz] = '\x00';

        //clean up
        free(_getStmt);
        json_object_put(jobj);
        sqlite3_finalize(getStmt);
    }
    else {
        LOG("unreachable code in leaderboardGet()?");
        exit(1);
    }

    free(errOut);
    return out;
}

bool leaderboardPost(ConnectionInfo* ci) {
    if (ci->resourceChainSize > 1) return false;

    char* request = ci->buf;
    json_object* jobj = json_tokener_parse(request);
    if (!jobj) { LOG("invalid json provided to leaderboardPost\n"); return false; }

    // extract game
    // this is the only value we'll need to "sanitize"/validate, since we'll be sprintf'ing it into our SQL statement (other values are prepared statement parameters)
    // upstream, server will reject messages with more than MAX_POST_BODY_SIZE (server.h) bytes...
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
    char _insertStmt[strlen(_templateInsertStmt) + (2 * strlen(game))];
    sprintf(_insertStmt, _templateInsertStmt, game, game);
    LOG("_insertStmt: %s\n", _insertStmt);
    sqlite3_stmt* insertStmt;
    if (sqlite3_prepare_v2(db, _insertStmt, -1, &insertStmt, NULL) != SQLITE_OK) { // compile statement
        LOG("error compiling SQL insert-into statement\n");
        json_object_put(jobj);
        return false;
    }
    sqlite3_bind_text(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@name"), name, strlen(name) > 12 ? NAME_MAX_LENGTH : strlen(name), SQLITE_STATIC); // enforce max name length
    sqlite3_bind_int64(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@score"), score);
    sqlite3_bind_int64(insertStmt, sqlite3_bind_parameter_index(insertStmt, "@time"), time);

    // execute SQL statement
    EXPAND_AND_LOG_SQL_STATEMENT(insertStmt);
    sqlite3_step(insertStmt);

    //clean up
    sqlite3_finalize(insertStmt);
    json_object_put(jobj);
    return true;
}
