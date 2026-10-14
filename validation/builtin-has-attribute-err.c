static int x;

static int test_err(void)
{
	return __builtin_has_attribute(x, 123) +
	       __builtin_has_attribute(x, "aligned") +
	       __builtin_has_attribute(x, );
}

/*
 * check-name: builtin-has-attribute-err
 * check-command: sparse $file
 *
 * check-error-start
builtin-has-attribute-err.c:5:43: error: expected attribute name
builtin-has-attribute-err.c:6:43: error: expected attribute name
builtin-has-attribute-err.c:7:43: error: expected attribute name
 * check-error-end
 */
