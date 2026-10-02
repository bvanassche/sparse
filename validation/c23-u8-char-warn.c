static unsigned char w1 = u8'\x100';
static const char *w2 = u8"test";

/*
 * check-name: c23-u8-char-warn
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-u8-char-warn.c:1:27: warning: hex escape sequence out of range
c23-u8-char-warn.c:2:25: warning: incorrect type in initializer (different signedness)
c23-u8-char-warn.c:2:25:    expected char const *static [toplevel] w2
c23-u8-char-warn.c:2:25:    got unsigned char *
 * check-error-end
 */
