typedef unsigned int __attribute__((bitwise)) be32;
enum E { E_ZERO, E_ONE, E_TWO };

static be32 test_atomic_restricted(be32 *ptr, be32 val, be32 *expected)
{
	__atomic_store_n(ptr, val, 0);
	be32 old = __atomic_exchange_n(ptr, val, 0);
	_Bool ok = __atomic_compare_exchange_n(ptr, expected, val, 0, 0, 0);
	(void)ok;
	return old;
}

static enum E test_atomic_enum(enum E *ptr, enum E val)
{
	__atomic_store_n(ptr, val, 0);
	return __atomic_load_n(ptr, 0);
}

static int test_atomic_const_reads(int *ptr, const int *cptr,
				   volatile int *vptr,
				   const volatile int *cvptr,
				   int *expected, int *out,
				   volatile _Bool *vflag)
{
	int v = __atomic_load_n(cptr, 0);
	v += __atomic_load_n(cvptr, 0);

	__atomic_load(cptr, out, 0);
	__atomic_load(cvptr, out, 0);
	__atomic_store(ptr, cptr, 0);
	__atomic_exchange(ptr, cptr, out, 0);
	(void)__atomic_compare_exchange(ptr, expected, cptr, 0, 0, 0);

	__atomic_store_n(vptr, v, 0);
	v += __atomic_fetch_add(vptr, 1, 0);
	(void)__atomic_test_and_set(vflag, 0);
	__atomic_clear(vflag, 0);

	v += __atomic_always_lock_free(sizeof(*cvptr), cvptr);
	v += __atomic_is_lock_free(sizeof(*cvptr), cvptr);
	return v;
}

/*
 * check-name: atomic-builtins-types
 * check-command: sparse -Wbitwise $file
 */
