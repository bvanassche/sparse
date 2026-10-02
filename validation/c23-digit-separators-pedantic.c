int a = 1'000;
double b = 1'0.5;

/*
 * check-name: c23-digit-separators-pedantic
 * check-command: sparse -std=c17 -Wpedantic -Wno-decl $file
 *
 * check-error-start
c23-digit-separators-pedantic.c:1:9: warning: ISO C does not support digit separators before C23
c23-digit-separators-pedantic.c:2:12: warning: ISO C does not support digit separators before C23
 * check-error-end
 */
