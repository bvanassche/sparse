constexpr int no_init;
constexpr int foo(void)
{
	return 0;
}
typedef constexpr int my_int;
extern constexpr int ext = 1;
register constexpr int reg = 1;
void bar(constexpr int p)
{
}
struct s {
	constexpr int m;
};
constexpr constexpr int double_ce = 1;
int non_const = 5;
constexpr int bad_init = non_const;
thread_local constexpr int tls = 1;
constexpr struct empty_s {
	int a;
};
constexpr volatile int cv = 1;
constexpr _Atomic int ca = 1;
constexpr int *restrict cr = nullptr;
constexpr int *bad_ptr = &non_const;
constexpr const char *bad_str = "hi";
constexpr int bad_flt_ice = 1.0;
constexpr unsigned int neg_to_u = -1;
constexpr signed char over_sc = 200;
constexpr bool bad_bool = 2;
constexpr float inexact_int = 16777217;
constexpr float inexact_dbl = 0.1;
extern int already_in_scope;
constexpr int already_in_scope = 1;

struct pwrap {
	int *p;
	unsigned char u;
};
constexpr struct pwrap bad_nested = { &non_const, 300 };

void assign_and_vla_test(int n)
{
	constexpr int c = 10;
	c = 20;

	constexpr int *cp = nullptr;
	cp = nullptr;

	constexpr int arr[2] = { 1, 2 };
	arr[0] = 3;

	constexpr int (*vla)[n] = nullptr;
	(void)vla;
}

/*
 * check-name: C23 constexpr error cases
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-constexpr-error.c:1:15: error: constexpr variable 'no_init' without initializer
c23-constexpr-error.c:2:15: error: function 'foo' cannot be declared constexpr
c23-constexpr-error.c:6:23: error: constexpr cannot be used with typedef
c23-constexpr-error.c:7:22: error: constexpr cannot be used with extern
c23-constexpr-error.c:8:24: error: constexpr register variable 'reg' at file scope
c23-constexpr-error.c:9:10: error: parameter 'p' cannot be declared constexpr
c23-constexpr-error.c:13:23: error: struct or union member 'm' cannot be declared constexpr
c23-constexpr-error.c:15:21: error: duplicate constexpr
c23-constexpr-error.c:18:28: error: constexpr cannot be used with thread_local
c23-constexpr-error.c:21:2: error: constexpr in empty declaration
c23-constexpr-error.c:34:15: error: underspecified declaration of 'already_in_scope', which is already declared in this scope
c23-constexpr-error.c:17:26: error: non-constant initializer for constexpr object
c23-constexpr-error.c:22:24: error: constexpr variable 'cv' cannot have volatile, atomic, or restrict type
c23-constexpr-error.c:23:23: error: constexpr variable 'ca' cannot have volatile, atomic, or restrict type
c23-constexpr-error.c:24:15: error: constexpr variable 'cr' cannot have volatile, atomic, or restrict type
c23-constexpr-error.c:25:27: error: constexpr pointer initializer is not null
c23-constexpr-error.c:26:33: error: constexpr pointer initializer is not null
c23-constexpr-error.c:27:29: error: constexpr integer initializer is not an integer constant expression
c23-constexpr-error.c:28:35: error: constexpr initializer not representable in target type
c23-constexpr-error.c:29:33: error: constexpr initializer not representable in target type
c23-constexpr-error.c:30:27: error: constexpr initializer not representable in target type
c23-constexpr-error.c:31:31: error: constexpr initializer not representable in target type
c23-constexpr-error.c:32:31: error: constexpr initializer not representable in target type
c23-constexpr-error.c:40:40: error: constexpr pointer initializer is not null
c23-constexpr-error.c:40:51: error: constexpr initializer not representable in target type
c23-constexpr-error.c:45:9: error: assignment to const expression
c23-constexpr-error.c:48:9: error: assignment to const expression
c23-constexpr-error.c:51:12: error: assignment to const expression
c23-constexpr-error.c:53:23: error: constexpr variable 'vla' cannot have variably modified type
 * check-error-end
 */
