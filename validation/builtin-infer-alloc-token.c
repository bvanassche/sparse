typedef struct { __SIZE_TYPE__ v; } kmalloc_token_t;

_Static_assert(__builtin_infer_alloc_token(1) == 0, "infer_alloc_token ICE");
_Static_assert(__builtin_infer_alloc_token(1, 2) == 0, "infer_alloc_token 2-arg ICE");
_Static_assert(__builtin_types_compatible_p(typeof(__builtin_infer_alloc_token(1)),
					    __SIZE_TYPE__), "size_t return type");

static const __SIZE_TYPE__ static_tok = __builtin_infer_alloc_token(64);

static __SIZE_TYPE__ test_infer(int size, int flags)
{
	__SIZE_TYPE__ tok = __builtin_infer_alloc_token(size, flags);
	return tok + static_tok;
}

static __SIZE_TYPE__ test_infer_single(int size)
{
	return __builtin_infer_alloc_token(size);
}

static kmalloc_token_t test_struct(int size)
{
	return (kmalloc_token_t){ .v = __builtin_infer_alloc_token(size) };
}

/*
 * check-name: builtin-infer-alloc-token
 */
