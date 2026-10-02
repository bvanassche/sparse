static_assert(0);
static_assert(0, "must fail");

typedef alignas(8) int bad_typedef;

struct bf {
	alignas(4) unsigned int x : 2;
};

static void bad_param(alignas(8) int p)
{
	(void)p;
}

static void bad_fn_locals(void)
{
	thread_local int local_tl = 0;
	register alignas(8) int reg_a = 0;
	int sz = sizeof(alignas(8) int);
	(void)local_tl;
	(void)reg_a;
	(void)sz;
}

static int (alignas);
static int (alignof);
static int (static_assert);
static int (thread_local);

/*
 * check-name: c23-keywords-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-keywords-error.c:1:15: error: static assertion failed
c23-keywords-error.c:2:15: error: static assertion failed: "must fail"
c23-keywords-error.c:4:24: error: alignment specified for typedef 'bad_typedef'
c23-keywords-error.c:7:35: error: alignment specified for bitfield 'x'
c23-keywords-error.c:10:23: error: alignment specified for parameter 'p'
c23-keywords-error.c:17:26: error: thread-local variable 'local_tl' at block scope must be 'static' or 'extern'
c23-keywords-error.c:18:33: error: alignment specified for 'register' object 'reg_a'
c23-keywords-error.c:19:25: error: alignment specified for type name
c23-keywords-error.c:25:12: error: Trying to use reserved word 'alignas' as identifier
c23-keywords-error.c:26:12: error: Trying to use reserved word 'alignof' as identifier
c23-keywords-error.c:27:12: error: Trying to use reserved word 'static_assert' as identifier
c23-keywords-error.c:28:12: error: Trying to use reserved word 'thread_local' as identifier
 * check-error-end
 */
