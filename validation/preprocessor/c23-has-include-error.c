int outside = __has_include(<stdio.h>);

#if __has_include("")
#endif

#if __has_include(<>)
#endif

/*
 * check-name: c23-has-include-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
preprocessor/c23-has-include-error.c:1:15: error: '__has_include' used outside of preprocessing directive
preprocessor/c23-has-include-error.c:3:19: error: empty filename
preprocessor/c23-has-include-error.c:6:5: error: empty filename
 * check-error-end
 */
