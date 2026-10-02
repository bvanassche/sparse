int outside = __has_embed("options.h");

#if __has_embed("options.h" limit(1) limit(2))
#endif

#embed "options.h" limit(1) __limit__(2)
#embed "options.h" vendor::param(1)

/*
 * check-name: c23-embed-error
 * check-command: sparse -std=c23 -Wno-decl -I.. $file
 *
 * check-error-start
preprocessor/c23-embed-error.c:1:15: error: '__has_embed' used outside of preprocessing directive
preprocessor/c23-embed-error.c:3:38: error: duplicate embed parameter 'limit'
preprocessor/c23-embed-error.c:6:29: error: duplicate embed parameter '__limit__'
preprocessor/c23-embed-error.c:7:20: error: unknown embed parameter 'vendor::param'
 * check-error-end
 */
