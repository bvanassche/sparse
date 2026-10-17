static inline void f(void)
{
	__builtin_constant_p(0);
}

void foo(void)
{
	0 ? 0 : f();
}

void bar(void)
{
	1 ? f() : 0;
}

/*
 * check-name: cond-err-expand.c
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: call.* __builtin_constant_p
 */
