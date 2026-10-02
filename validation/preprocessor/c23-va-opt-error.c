#define BAD0 __VA_OPT__(1)
#define BAD1(x) __VA_OPT__(x)
#define BAD2(...) __VA_OPT__
#define BAD3(...) __VA_OPT__(1
#define BAD4(...) __VA_OPT__(__VA_OPT__(1))
#define BAD5(...) __VA_OPT__(##x)
#define BAD6(...) __VA_OPT__(x##)
#define BAD7(__VA_OPT__) 1
#define __VA_OPT__ 1

/*
 * check-name: c23-va-opt-error
 * check-command: sparse -std=c23 -E $file
 *
 * check-output-start


 * check-output-end
 *
 * check-error-start
preprocessor/c23-va-opt-error.c:1:14: error: __VA_OPT__ can only appear in the expansion of a variadic macro
preprocessor/c23-va-opt-error.c:2:17: error: __VA_OPT__ can only appear in the expansion of a variadic macro
preprocessor/c23-va-opt-error.c:3:19: error: expected '(' after __VA_OPT__
preprocessor/c23-va-opt-error.c:4:19: error: unterminated __VA_OPT__
preprocessor/c23-va-opt-error.c:5:30: error: __VA_OPT__ cannot appear within __VA_OPT__
preprocessor/c23-va-opt-error.c:6:30: error: '##' cannot appear at the beginning of __VA_OPT__
preprocessor/c23-va-opt-error.c:7:31: error: '##' cannot appear at the end of __VA_OPT__
preprocessor/c23-va-opt-error.c:8:14: error: __VA_OPT__ cannot appear in a macro parameter list
preprocessor/c23-va-opt-error.c:9:9: error: "__VA_OPT__" cannot be used as a macro name
 * check-error-end
 */
