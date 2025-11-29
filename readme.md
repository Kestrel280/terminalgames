Each subfolder (excluding `/api`) contains a small, terminal based game written in C and using ncurses to render "graphics" to the terminal.

## Purpose
The intent behind these games is simple programming practice: projects are supposed to be small and completeable in 1-3 days. Each game deliberately has a minimal set of features, to avoid spending too much time on any one thing (I want to focus more on reps than on polish!). Each game has its own readme with more information. Projects are chosen to target specific things that I want to practice or learn.

## API
The `/api` folder contains a (WIP) webserver application used for maintaining a highscores leaderboard for the games. This is mostly an exercise in backend web development, and is a larger, ongoing project running in parallel to the terminal games. The application uses GNU `libmicrohttpd` for running the webserver and `sqlite` for managing the highscores database. See `/api/readme.md` for more information.

## Building & Dependencies
To build a project, invoke `make` from the project's directory. GCC and ncurses are required. To build on Windows, install `make`, `gcc-core`, and `ncurses-devel` using cygwin.

## Notes
- [ncurses leaks memory by design](https://invisible-island.net/ncurses/ncurses.faq.html#config_leaks).
