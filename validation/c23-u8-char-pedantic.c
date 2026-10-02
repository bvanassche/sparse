static int x = u8'a';

/*
 * check-name: c23-u8-char-pedantic
 * check-command: sparse -std=c11 -Wpedantic $file
 *
 * check-error-start
c23-u8-char-pedantic.c:1:16: warning: UTF-8 character constants are a C23 feature
 * check-error-end
 */
