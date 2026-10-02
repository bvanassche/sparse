#define FOO 1

#if 0
int a = 0;
#elifdef FOO
int a = 1;
#else
int a = 2;
#endif

#if 0
int b = 0;
#elifdef BAR
int b = 1;
#elifndef BAR
int b = 2;
#else
int b = 3;
#endif

#if 1
int c = 1;
#elifdef FOO
int c = 2;
#endif

#if 1
int d = 1;
#elifndef BAR
int d = 2;
#endif

/*
 * check-name: c23-elifdef
 * check-command: sparse -E $file
 *
 * check-output-start

int a = 1;
int b = 2;
int c = 1;
int d = 1;
 * check-output-end
 */
