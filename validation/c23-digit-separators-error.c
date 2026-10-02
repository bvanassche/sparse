int a = 1''0;
int b = 0x'1;
int b2 = 0b'1;
int c = 1'u;
int c2 = 123'u;
double d = 1.'0;
double e = 1'.';
double f = 1e'0;
double g = 1'e0;
double h = 1e+'0;
float i = 0x1p'0f;
float j = 0x1.0p1'f;
float k = 1.0'f;

/*
 * check-name: c23-digit-separators-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-digit-separators-error.c:1:10: error: Expected ; at end of declaration
c23-digit-separators-error.c:1:10: error: got ''
c23-digit-separators-error.c:2:9: error: digit separator after base indicator
c23-digit-separators-error.c:3:10: error: digit separator after base indicator
c23-digit-separators-error.c:4:9: error: digit separator outside digit sequence
c23-digit-separators-error.c:5:10: error: digit separator outside digit sequence
c23-digit-separators-error.c:6:12: error: digit separator adjacent to decimal point
c23-digit-separators-error.c:7:13: error: Expected ; at end of declaration
c23-digit-separators-error.c:7:13: error: got '.'
c23-digit-separators-error.c:8:12: error: digit separator adjacent to exponent
c23-digit-separators-error.c:9:12: error: digit separator adjacent to exponent
c23-digit-separators-error.c:10:12: error: digit separator adjacent to exponent
c23-digit-separators-error.c:11:11: error: digit separator adjacent to exponent
c23-digit-separators-error.c:12:11: error: digit separator outside digit sequence
c23-digit-separators-error.c:13:11: error: digit separator outside digit sequence
 * check-error-end
 */
