### simple c coroutine library

coroutine in 200 lines, learned from libtask, only support Linux x86-64. io read/write supported.

```
examples:

// an http echo server
gcc -std=c99 -g co.S co.c http_echo.c

// -DCO_DEBUG: enable debug logging
gcc -DCO_DEBUG -std=c99 -g co.S co.c main.c
```
