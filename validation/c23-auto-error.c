auto no_init;
auto arr[2] = { 0, 1 };
auto foo(void)
{
}
auto x = 1, y = 2;
auto self = self;
auto auto dup = 1;
auto self_expr = self_expr + 1;
auto self_sz = sizeof(self_sz);
struct bad_s {
	auto m;
};
void bad_param(auto p);
typedef auto bad_td;
auto;
auto int bad_file_storage = 1;
extern int already_in_scope;
auto already_in_scope = 1;

void block_errors(void)
{
	int shadow = 5;
	{
		auto shadow = shadow + 1;
	}
	int bad_sz = sizeof(auto);
	int bad_cast = (auto)1;
	_Atomic(auto) bad_atomic = 1;
	auto braced = { 1 };
	auto empty = {};
	(void)bad_sz;
	(void)bad_cast;
	(void)bad_atomic;
	(void)braced;
	(void)empty;
}

/*
 * check-name: C23 auto error cases
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-auto-error.c:1:6: error: auto without initializer
c23-auto-error.c:2:6: error: auto on non-identifier
c23-auto-error.c:3:6: error: 'foo()' has auto return type
c23-auto-error.c:6:6: error: auto on declaration list
c23-auto-error.c:8:11: error: duplicate auto
c23-auto-error.c:12:14: error: struct or union member 'm' cannot be declared auto
c23-auto-error.c:14:16: error: parameter 'p' cannot be declared auto
c23-auto-error.c:15:14: error: auto cannot be used with typedef
c23-auto-error.c:16:5: error: auto in empty declaration
c23-auto-error.c:17:10: error: file-scope declaration of 'bad_file_storage' specifies 'auto'
c23-auto-error.c:19:6: error: underspecified declaration of 'already_in_scope', which is already declared in this scope
c23-auto-error.c:27:29: error: 'auto' in type name
c23-auto-error.c:28:25: error: 'auto' in type name
c23-auto-error.c:29:16: error: Trying to use reserved word 'auto' as identifier
c23-auto-error.c:29:23: error: Expected ; at end of declaration
c23-auto-error.c:29:23: error: got bad_atomic
c23-auto-error.c:30:14: error: auto requires a scalar initializer expression
c23-auto-error.c:31:14: error: auto requires a scalar initializer expression
c23-auto-error.c:2:15: error: invalid initializer
c23-auto-error.c:7:13: error: underspecified symbol 'self' used in its own initializer
c23-auto-error.c:9:18: error: underspecified symbol 'self_expr' used in its own initializer
c23-auto-error.c:10:23: error: underspecified symbol 'self_sz' used in its own initializer
c23-auto-error.c:10:16: error: cannot size expression
c23-auto-error.c:25:31: error: underspecified symbol 'shadow' used in its own initializer
c23-auto-error.c:27:22: error: cannot size expression
c23-auto-error.c:34:15: error: undefined identifier 'bad_atomic'
 * check-error-end
 */
