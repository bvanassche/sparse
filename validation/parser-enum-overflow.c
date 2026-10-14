enum {
	E_ULLMAX = ~0ULL,
	E_ULL_OVERFLOW,
};

/*
 * check-name: parser-enum-overflow
 * check-command: sparse $file
 *
 * check-error-start
parser-enum-overflow.c:3:9: error: enumerator value for 'E_ULL_OVERFLOW' overflows
 * check-error-end
 */
