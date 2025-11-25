# Overview

This application:
- Manages a sqlite database of high-score leaderboards for certain terminal games
- Runs a webserver which allows users to fetch data from the leaderboards, as well as post their scores to the leaderboards

It is meant to be deployed as an API endpoint for a personal website which is under development.

**(IN PROGRESS! Not yet functional)**

# API usage
## **Fetch** leaderboard information

There are currently 2 ways to fetch leaderboard information, by requesting GET requests to the respective endpoint:

- `<server>/leaderboards/game/<game>` will fetch the top 10 times for the specified game.
- `<server>/leaderboards/player/<playername>` will fetch all posted leaderboard times for all games matching the specified player's name.

The format of the response will be the same regardless of how the API is accessed: a JSON object with 'game: [entry1, entry2, ...]' pairs, where `entry` is itself JSON with a `name` value, `time` value, and `score` value.

Example:

`curl --request GET {server}/leaderboards/game/snake`

```
{
    snake: [
        {
            "name": "<snake rank 1 player name>",
            "time": Unix time,
            "score": <rank 1 score>
        },
        {
            "name": "<rank 2 player name>",
            "time": Unix time,
            "score": <rank 2 score">
        }
    ]
}
```

## **Post** a score to the leaderboard:

Posting scores to the leaderboards is done automatically by supported terminal games. At this time, there is no anti-cheating mechanism to prevent the submission of fake or cheated times.

To post a score to the leaderboard, issue a POST request to `<server>/leaderboards` with a JSON body like so:

```
{
    "game": "<game name>",
    "entry": {
        "name": "<player name>",
        "time": <UNIX epoch time in seconds>
        "score": <score>
    }
}
```

## Dependencies
    
- GNU **libmicrohttpd**, compiled with HTTPS support (requires libgnutls): https://www.gnu.org/software/libmicrohttpd/
- **sqlite3**
- **json-c**: https://github.com/json-c/json-c
