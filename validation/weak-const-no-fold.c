const int __attribute__((weak)) weak_const = 10;

int read_weak_const(void)
{
	return weak_const;
}

/*
 * check-name: weak-const-no-fold
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-contains: load\\.32.*\\[weak_const\\]
 * check-output-excludes: ret\\.32.*\\$10
 */
