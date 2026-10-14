struct fam {
	int count;
	char arr[];
};

static void test_cbr_args(struct fam *f)
{
	(void)__builtin_counted_by_ref();
	(void)__builtin_counted_by_ref(f->arr, 1);
}

/*
 * check-name: builtin-counted-by-ref-err
 * check-command: sparse $file
 *
 * check-error-start
builtin-counted-by-ref-err.c:8:39: error: not enough arguments for __builtin_counted_by_ref
builtin-counted-by-ref-err.c:9:39: error: too many arguments for __builtin_counted_by_ref
 * check-error-end
 */
