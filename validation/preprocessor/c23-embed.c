static const unsigned char data[] = {
#embed "options.h" limit(4)
};

static const unsigned char with_params[] = {
#embed "options.h" limit(2) prefix(10, 20, ) suffix(, 30, 40)
};

static const unsigned char dunder_params[] = {
#embed "options.h" __limit__(2) __prefix__(10, ) __suffix__(, 20)
};

static const unsigned char empty_buf[] = {
#embed "options.h" limit(0) if_empty(99)
};

static const unsigned char dev_null_buf[] = {
#embed "/dev/null" __if_empty__(77)
};

static const unsigned char zero_buf[] = {
#embed "/dev/zero" limit(4)
};

static const unsigned char nested_has_embed[] = {
#embed "options.h" limit(__has_embed("options.h"))
};

static_assert(sizeof(data) == 4, "size data");
static_assert(sizeof(with_params) == 6, "size with_params");
static_assert(sizeof(dunder_params) == 4, "size dunder_params");
static_assert(sizeof(empty_buf) == 1, "size empty_buf");
static_assert(sizeof(dev_null_buf) == 1, "size dev_null_buf");
static_assert(sizeof(zero_buf) == 4, "size zero_buf");
static_assert(sizeof(nested_has_embed) == 1, "size nested_has_embed");

static constexpr unsigned char p0 =
#embed "options.h" limit(1) prefix(10 + ) suffix( + 5)
	;
static_assert(p0 == 50, "p0");

static constexpr unsigned char z0 =
#embed "/dev/zero" limit(1)
	;
static_assert(z0 == 0, "z0");

static constexpr unsigned char b0 =
#embed "options.h" limit(1)
	;
static_assert(b0 == 35, "b0");

#define EMBED_HDR "options.h"
static const unsigned char macro_hdr_params[] = {
#embed EMBED_HDR limit(2) if_empty(__COUNTER__) prefix(11, )
};
static_assert(__COUNTER__ == 0, "if_empty not expanded when non-empty");
static_assert(sizeof(macro_hdr_params) == 3, "size macro_hdr_params");

static constexpr unsigned char macro_hdr_p0 =
#embed EMBED_HDR limit(1) if_empty(__COUNTER__) prefix(10 + )
	;
static_assert(macro_hdr_p0 == 45, "macro_hdr_p0");

static const unsigned char large_limit_buf[] = {
#embed "options.h" limit(1ULL << 63)
};
static_assert(sizeof(large_limit_buf) > 4, "size large_limit_buf");

/*
 * check-name: c23-embed
 * check-command: sparse -std=c23 -Wno-decl -I.. $file
 */
