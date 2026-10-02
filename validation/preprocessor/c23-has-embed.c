#if defined(__has_embed)
int has_macro = 1;
#endif

#if __has_embed("options.h") == __STDC_EMBED_FOUND__
int found_options = 1;
#endif

#if __has_embed("options.h" limit(0)) == __STDC_EMBED_EMPTY__
int limit_zero_empty = 1;
#endif

#if __has_embed("options.h" __limit__(0)) == __STDC_EMBED_EMPTY__
int dunder_limit_zero_empty = 1;
#endif

#if __has_embed("options.h" limit(5)) == __STDC_EMBED_FOUND__
int limit_five_found = 1;
#endif

#if __has_embed("nonexistent_embed_12345.bin") == __STDC_EMBED_NOT_FOUND__
int not_found = 1;
#endif

#if __has_embed("options.h" prefix(1) suffix(2)) == __STDC_EMBED_FOUND__ && \
    __has_embed("options.h" __prefix__(1) __suffix__(2) __if_empty__(3)) == \
	__STDC_EMBED_FOUND__
int standard_params = 1;
#endif

#if __has_embed("options.h" unknown_vendor::param(1)) == \
	__STDC_EMBED_NOT_FOUND__
int unknown_param = 1;
#endif

#define HAS_EMB __has_embed
#if HAS_EMB("options.h") == __STDC_EMBED_FOUND__
int aliased_has_embed = 1;
#endif

#if __has_embed("/dev/null") == __STDC_EMBED_EMPTY__
int dev_null_empty = 1;
#endif

#if __has_embed("options.h" limit(1ULL << 63)) == __STDC_EMBED_FOUND__ && \
    __has_embed("options.h" limit(0xffffffffffffffffULL)) == \
	__STDC_EMBED_FOUND__
int large_unsigned_limit = 1;
#endif

/*
 * check-name: c23-has-embed
 * check-command: sparse -std=c23 -E -I.. $file
 *
 * check-output-start

int has_macro = 1;
int found_options = 1;
int limit_zero_empty = 1;
int dunder_limit_zero_empty = 1;
int limit_five_found = 1;
int not_found = 1;
int standard_params = 1;
int unknown_param = 1;
int aliased_has_embed = 1;
int dev_null_empty = 1;
int large_unsigned_limit = 1;
 * check-output-end
 */
