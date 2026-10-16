#define S16 "0123456789abcdef"
#define S64 S16 S16 S16 S16
#define S256 S64 S64 S64 S64
#define S1024 S256 S256 S256 S256
#define S4096 S1024 S1024 S1024 S1024
#define S16384 S4096 S4096 S4096 S4096
#define S65536 S16384 S16384 S16384 S16384

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

#define T4 a+a+a+a
#define T16 T4+T4+T4+T4
#define T64 T16+T16+T16+T16
#define T256 T64+T64+T64+T64
#define T1024 T256+T256+T256+T256
#define T4096 T1024+T1024+T1024+T1024
#define T16384 T4096+T4096+T4096+T4096
#define T65536 T16384+T16384+T16384+T16384

/* Concatenated string literal > 65535 bytes */
_Static_assert(sizeof(S65536 S16) == 65536 + 16 + 1, "concat > 64KB");

/* Stringified macro expansion with escaped quotes > 65535 bytes */
_Static_assert(sizeof(STR("q" T65536)) > 130000, "stringify > 64KB");

/*
 * check-name: long-string-expansion
 * check-command: sparse $file
 */
