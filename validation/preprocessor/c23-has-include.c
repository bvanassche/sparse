#if defined(__has_include)
int has_macro = 1;
#endif

#if __has_include(<stdio.h>)
int has_stdio = 1;
#else
int has_stdio = 0;
#endif

#if __has_include("options.h")
int has_options = 1;
#endif

#if __has_include(<nonexistent_header_12345.h>)
int has_nonexistent = 1;
#else
int has_nonexistent = 0;
#endif

#define HEADER <stdio.h>
#if __has_include(HEADER)
int has_macro_header = 1;
#endif

#define HAS_INC __has_include
#if HAS_INC(<stdio.h>) && HAS_INC("options.h")
int has_aliased_op = 1;
#endif

#if __has_include("this_is_an_extremely_long_path_name_that_tests_bounds_checking_1234567890_abcdefghijklmnopqrstuvwxyz.h")
int has_long = 1;
#else
int has_long = 0;
#endif

/*
 * check-name: c23-has-include
 * check-command: sparse -std=c23 -E -I.. $file
 *
 * check-output-start

int has_macro = 1;
int has_stdio = 1;
int has_options = 1;
int has_nonexistent = 0;
int has_macro_header = 1;
int has_aliased_op = 1;
int has_long = 0;
 * check-output-end
 */
