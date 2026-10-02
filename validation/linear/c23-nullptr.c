typedef typeof(nullptr) nullptr_t;

static void *np_to_ptr(nullptr_t np)
{
	return np;
}

static bool np_is_null(nullptr_t np)
{
	return !np;
}

/*
 * check-name: linear-c23-nullptr
 * check-command: test-linearize -m64 -std=c23 $file
 *
 * check-output-ignore
 * check-output-contains: ret\\.64 *%arg1
 * check-output-contains: seteq\\.1
 */
