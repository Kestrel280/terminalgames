Server which maintains a database of *game leaderboards* and exposes an endpoint to fetch leaderboard data or post scores to the leaderboards.

(IN PROGRESS! Not yet functional)

## API usage
### **Fetch** leaderboard information

Fetching leaderboard information is done by issuing a GET request to `<server>/leaderboards`. The body of the request should be a JSON object with a "game / <gamename>" key/value pair. The request will return a JSON object containing the top 10 leaderboard times.

Example:

```
curl
    --header "Content-Type: application/json"
    --request GET
    --data '{"game": "snake"}
    {server}/leaderboards

{
    [
        {
            "name": "<rank 1 player name>",
            "time": UNIX epoch time in seconds,
            "score": <rank 1 score>
        },
        {
            "name": "<rank 2 player name>",
            "time": UNIX epoch time in seconds,
            "score": <rank 2 score">
        }
    ]
}

```

### **Post** a score to the leaderboard:

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
- **json-c**: https://github.com/json-c/json-c
