#define FOO 1

#if 1
int taken = 1;
#elifdef FOO
int skipped1 = 1;
#elifndef BAR
int skipped2 = 1;
#elifdef 123
int skipped3 = 1;
#endif

#if 0
#if 1
#elifdef FOO
#elifdef 123
#endif
#endif

/*
 * check-name: c23-elifdef-error
 * check-command: sparse -std=c17 -Wpedantic -E $file
 *
 * check-output-start

int taken = 1;
 * check-output-end
 *
 * check-error-start
preprocessor/c23-elifdef-error.c:5:2: warning: ISO C does not support #elifdef before C23
preprocessor/c23-elifdef-error.c:7:2: warning: ISO C does not support #elifndef before C23
preprocessor/c23-elifdef-error.c:9:2: warning: ISO C does not support #elifdef before C23
preprocessor/c23-elifdef-error.c:9:2: error: expected preprocessor identifier
 * check-error-end
 */
