static void test_atomic_const_writes(const int *cptr, int *ptr,
				     int *exp, int *out,
				     const _Bool *cflag)
{
	int val = 1;

	__atomic_store(cptr, &val, 0);
	__atomic_store_n(cptr, val, 0);
	__atomic_load(ptr, cptr, 0);
	__atomic_exchange(cptr, &val, out, 0);
	__atomic_exchange(ptr, &val, cptr, 0);
	(void)__atomic_exchange_n(cptr, val, 0);
	(void)__atomic_compare_exchange(cptr, exp, &val, 0, 0, 0);
	(void)__atomic_compare_exchange(ptr, cptr, &val, 0, 0, 0);
	(void)__atomic_compare_exchange_n(cptr, exp, val, 0, 0, 0);
	(void)__atomic_compare_exchange_n(ptr, cptr, val, 0, 0, 0);
	(void)__atomic_fetch_add(cptr, 1, 0);
	(void)__atomic_test_and_set(cflag, 0);
	__atomic_clear(cflag, 0);
}

/*
 * check-name: atomic-builtins-const
 * check-command: sparse $file
 *
 * check-error-start
atomic-builtins-const.c:7:24: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:7:24:    expected int volatile *
atomic-builtins-const.c:7:24:    got int const *cptr
atomic-builtins-const.c:8:26: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:8:26:    expected int volatile *
atomic-builtins-const.c:8:26:    got int const *cptr
atomic-builtins-const.c:9:28: warning: incorrect type in argument 2 (different modifiers)
atomic-builtins-const.c:9:28:    expected int *
atomic-builtins-const.c:9:28:    got int const *cptr
atomic-builtins-const.c:10:27: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:10:27:    expected int volatile *
atomic-builtins-const.c:10:27:    got int const *cptr
atomic-builtins-const.c:11:38: warning: incorrect type in argument 3 (different modifiers)
atomic-builtins-const.c:11:38:    expected int *
atomic-builtins-const.c:11:38:    got int const *cptr
atomic-builtins-const.c:12:35: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:12:35:    expected int volatile *
atomic-builtins-const.c:12:35:    got int const *cptr
atomic-builtins-const.c:13:41: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:13:41:    expected int volatile *
atomic-builtins-const.c:13:41:    got int const *cptr
atomic-builtins-const.c:14:46: warning: incorrect type in argument 2 (different modifiers)
atomic-builtins-const.c:14:46:    expected int *
atomic-builtins-const.c:14:46:    got int const *cptr
atomic-builtins-const.c:15:43: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:15:43:    expected int volatile *
atomic-builtins-const.c:15:43:    got int const *cptr
atomic-builtins-const.c:16:48: warning: incorrect type in argument 2 (different modifiers)
atomic-builtins-const.c:16:48:    expected int *
atomic-builtins-const.c:16:48:    got int const *cptr
atomic-builtins-const.c:17:34: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:17:34:    expected int volatile *
atomic-builtins-const.c:17:34:    got int const *cptr
atomic-builtins-const.c:18:37: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:18:37:    expected void volatile *
atomic-builtins-const.c:18:37:    got bool const *cflag
atomic-builtins-const.c:19:24: warning: incorrect type in argument 1 (different modifiers)
atomic-builtins-const.c:19:24:    expected bool volatile *
atomic-builtins-const.c:19:24:    got bool const *cflag
 * check-error-end
 */
