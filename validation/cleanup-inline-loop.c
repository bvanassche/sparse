#define __cleanup(f) __attribute__((cleanup(f)))

static inline void cleanup_fn(int *p)
{
	int tmp = *p;
	while (tmp > 0) {
		tmp--;
		if (tmp == 0)
			goto done;
	}
done:
	*p = tmp;
}

static int test_multiple_exits(int cond)
{
	int x __cleanup(cleanup_fn) = 2;
	if (cond == 1)
		return 1;
	if (cond == 2)
		return 2;
	return 0;
}

/*
 * check-name: inline cleanup with loop across multiple exits
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: VOID
 */
