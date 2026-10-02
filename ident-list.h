
#define IDENT(n) __IDENT(n## _ident, #n, 0)
#define IDENT_RESERVED(n) __IDENT(n## _ident, #n, 1)

/* Basic C reserved words.. */
IDENT_RESERVED(sizeof);
IDENT_RESERVED(if);
IDENT_RESERVED(else);
IDENT_RESERVED(return);
IDENT_RESERVED(switch);
IDENT_RESERVED(case);
IDENT_RESERVED(default);
IDENT_RESERVED(break);
IDENT_RESERVED(continue);
IDENT_RESERVED(for);
IDENT_RESERVED(while);
IDENT_RESERVED(do);
IDENT_RESERVED(goto);

/* C typenames. They get marked as reserved when initialized */
IDENT(struct);
IDENT(union);
IDENT(enum);
IDENT(__attribute); IDENT(__attribute__);
IDENT(volatile); IDENT(__volatile); IDENT(__volatile__);
IDENT(double);

/* C storage classes. They get marked as reserved when initialized */
IDENT(static);

/* C99 keywords */
IDENT(restrict); IDENT(__restrict); IDENT(__restrict__);
IDENT(_Bool);
IDENT_RESERVED(_Complex);
IDENT_RESERVED(_Imaginary);

/* C11 keywords */
IDENT(_Alignas);
IDENT_RESERVED(_Alignof);
IDENT(_Atomic);
IDENT_RESERVED(_Generic);
IDENT(_Noreturn);
IDENT_RESERVED(_Static_assert);
IDENT(_Thread_local);

/* C23 keywords */
IDENT(alignas);
IDENT(alignof);
IDENT(bool);
IDENT(constexpr);
IDENT(false);
IDENT(nullptr);
IDENT(static_assert);
IDENT(thread_local);
IDENT(true);

/* Special case for L'\t' */
IDENT(L);

/* Extended gcc identifiers */
IDENT(asm); IDENT_RESERVED(__asm); IDENT_RESERVED(__asm__);
IDENT_RESERVED(__alignof);
IDENT_RESERVED(__alignof__);
IDENT_RESERVED(__sizeof_ptr__);
IDENT_RESERVED(__builtin_types_compatible_p);
IDENT_RESERVED(__builtin_offsetof);
IDENT_RESERVED(__label__);
IDENT(gnu);
IDENT(__gnu__);

/* Preprocessor idents.  Direct use of __IDENT avoids mentioning the keyword
 * itself by name, preventing these tokens from expanding when compiling
 * sparse. */
IDENT(defined);
IDENT(once);
IDENT(c_alignas);
IDENT(c_alignof);
IDENT(c_generic_selections);
IDENT(c_static_assert);
__IDENT(pragma_ident, "__pragma__", 0);
__IDENT(__VA_ARGS___ident, "__VA_ARGS__", 0);
__IDENT(__func___ident, "__func__", 0);
__IDENT(__FUNCTION___ident, "__FUNCTION__", 0);
__IDENT(__PRETTY_FUNCTION___ident, "__PRETTY_FUNCTION__", 0);
__IDENT(__has_include_ident, "__has_include", 0);
__IDENT(__has_include_next_ident, "__has_include_next", 0);
__IDENT(__has_embed_ident, "__has_embed", 0);
IDENT(limit);
IDENT(prefix);
IDENT(suffix);
IDENT(if_empty);
__IDENT(__limit___ident, "__limit__", 0);
__IDENT(__prefix___ident, "__prefix__", 0);
__IDENT(__suffix___ident, "__suffix__", 0);
__IDENT(__if_empty___ident, "__if_empty__", 0);

/* Sparse commands */
IDENT_RESERVED(__context__);
IDENT_RESERVED(__range__);

/* Magic function names we recognize */
IDENT(memset); IDENT(memcpy);
IDENT(copy_to_user); IDENT(copy_from_user);
IDENT(main);

/* used by the symbolic checker */
IDENT(__assume);
IDENT(__assert);
IDENT(__assert_eq);
IDENT(__assert_const);

#undef __IDENT
#undef IDENT
#undef IDENT_RESERVED
