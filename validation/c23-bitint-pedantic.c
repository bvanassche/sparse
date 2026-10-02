static _BitInt(10) a;
static auto b = 42wb;

/*
 * check-name: C23 _BitInt pedantic
 * check-command: sparse -std=c17 -Wpedantic $file
 *
 * check-error-start
c23-bitint-pedantic.c:1:15: warning: '_BitInt' is a C23 feature
c23-bitint-pedantic.c:2:13: warning: 'auto' type inference is a C23 feature
c23-bitint-pedantic.c:2:17: warning: bit-precise integer suffix is a C23 feature
 * check-error-end
 */
