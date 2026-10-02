static unsigned char e1 = u8'';
static unsigned char e2 = u8'ab';
static unsigned char e3 = u8'\u00e9';
static const void *e4 = u8"a" L"b";

/*
 * check-name: c23-u8-char-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-u8-char-error.c:1:27: error: empty character constant
c23-u8-char-error.c:2:27: error: multi-character UTF-8 character constant
c23-u8-char-error.c:3:27: error: multi-character UTF-8 character constant
c23-u8-char-error.c:4:25: error: unsupported non-standard concatenation of string literals
 * check-error-end
 */
