# Common libraries

Libraries which may be used across multiple projects.

- libleaderboard: Used for fetching from (`leaderboardDisplay()`) and posting to (`leaderboardSubmitScore`) the leaderboards api (see `terminalgames/api`).

- libbitpack: BitPack structure: efficient storage of boolean data.

- libsdserver: Helper to manage a libmicrohttpd server more conveniently. Handles collecting a full HTTP request into a `ConnectionInfo` object which is passed to a user-specified callback.
