[[nodiscard] static int unclosed;
[[gnu::]] static int missing_scoped;

static void test_if_attr_decl(int c)
{
	if (c)
		[[maybe_unused]] int z = 3;
}

/*
 * check-name: c23-attributes-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-attributes-error.c:1:14: error: Expected ] at end of attribute specifier
c23-attributes-error.c:1:14: error: got static
c23-attributes-error.c:2:8: error: expected attribute name after '::'
c23-attributes-error.c:7:17: error: declaration is not a statement
 * check-error-end
 */
