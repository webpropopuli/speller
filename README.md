# speller from CS50
This is my speller from CS50 modified to run under GCC on windows.

The timer library used (sys/resources.h) does not exist in Windows builds so I reworked it to use timerval structs and located the timersadd() and timersub() macros.

It was sort of ugly but that's deprecated C for you.

Anyhow, now I can do local testing with a working timer as I shoot for a top10 on the Leaderboard (currently mid-30's on the edX version)
