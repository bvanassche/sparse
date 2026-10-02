#if defined(__has_c_attribute)
int has_macro = 1;
#endif

#if __has_c_attribute(nodiscard) == 202003L
int has_nodiscard = __has_c_attribute(nodiscard);
#endif

#if __has_c_attribute(__maybe_unused__) == 202106L
int has_maybe_unused = __has_c_attribute(__maybe_unused__);
#endif

#if __has_c_attribute(deprecated) == 201904L
int has_deprecated = __has_c_attribute(deprecated);
#endif

#if __has_c_attribute(fallthrough) == 201910L
int has_fallthrough = __has_c_attribute(fallthrough);
#endif

#if __has_c_attribute(noreturn) == 202202L && \
    __has_c_attribute(_Noreturn) == 202202L
int has_noreturn = __has_c_attribute(noreturn);
#endif

#if __has_c_attribute(reproducible) == 202207L && \
    __has_c_attribute(unsequenced) == 202207L
long has_reproducible = __has_c_attribute(reproducible);
#endif

#if !__has_c_attribute(unknown_attr_12345)
int no_unknown = 1;
#endif

#if __has_c_attribute(gnu::aligned)
int has_gnu_aligned = __has_c_attribute(gnu::aligned);
#endif

#if !__has_c_attribute(nodiscard::invalid)
int no_scoped_std = 1;
#endif

/*
 * check-name: c23-has-c-attribute
 * check-command: sparse -std=c23 -E $file
 *
 * check-output-start

int has_macro = 1;
int has_nodiscard = 202003L;
int has_maybe_unused = 202106L;
int has_deprecated = 201904L;
int has_fallthrough = 201910L;
int has_noreturn = 202202L;
long has_reproducible = 202207L;
int no_unknown = 1;
int has_gnu_aligned = 1;
int no_scoped_std = 1;
 * check-output-end
 */
