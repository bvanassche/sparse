[[maybe_unused]] static int x;

/*
 * check-name: C23 [[]] standard attribute syntax pedantic warning
 * check-command: sparse -std=c11 -Wpedantic $file
 *
 * check-error-start
c23-attributes-pedantic.c:1:1: warning: C23 attribute syntax is a C23 extension
 * check-error-end
 */
