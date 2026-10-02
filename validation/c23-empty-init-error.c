static int inc_arr[] = {};

struct inc_struct;

static void bad_init(void)
{
	struct inc_struct x = {};
	(void)x;
}

/*
 * check-name: c23-empty-init-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-empty-init-error.c:1:12: error: empty initializer for incomplete array
c23-empty-init-error.c:7:31: error: initializer for incomplete type
 * check-error-end
 */
