struct s {
	int a;
};

static struct s s = {};

/*
 * check-name: c23 empty initializer pedantic warning
 * check-command: sparse -std=c11 -Wpedantic $file
 *
 * check-error-start
c23-empty-init-pedantic.c:5:21: warning: ISO C forbids empty initializer braces before C23
 * check-error-end
 */
