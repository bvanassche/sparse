#define LOG(fmt, ...) printf(fmt __VA_OPT__(, __VA_ARGS__))
LOG("empty")
LOG("empty", )
LOG("one", 1)
LOG("two", 1, 2)

#define M(...) __VA_OPT__(1) 2 __VA_OPT__(3)
M()
M(a)

#define N(...) __VA_OPT__(foo(bar(baz)))
N()
N(1)

#define G(a, b, ...) a b __VA_OPT__(c)
G(1, 2)
G(1, 2, )
G(1, 2, 3)

#define EMPTY
#define VA_ORDER1(...) __VA_ARGS__ __VA_OPT__(opt)
#define VA_ORDER2(...) __VA_OPT__(opt) __VA_ARGS__
VA_ORDER1()
VA_ORDER2()
VA_ORDER1(EMPTY)
VA_ORDER2(EMPTY)

#define K 42
#define X hello
#define S(a, ...) #__VA_OPT__(a K __VA_ARGS__)
S(X)
S(X, Y)
S(X, K)

#define PASTE_L(x, ...) x ## __VA_OPT__(_suffix)
#define PASTE_R(y, ...) __VA_OPT__(prefix_) ## y
PASTE_L(foo)
PASTE_L(foo, 1)
PASTE_R(bar)
PASTE_R(bar, 1)

#define FN(v) ((v) + 1)
#define CALL(...) __VA_OPT__(FN)(10)
CALL()
CALL(1)

/*
 * check-name: c23-va-opt
 * check-command: sparse -std=c23 -E $file
 *
 * check-output-start

printf("empty" )
printf("empty" )
printf("one" , 1)
printf("two" , 1, 2)
2
1 2 3
foo(bar(baz))
1 2
1 2
1 2 c
""
"hello K Y"
"hello K 42"
foo
foo_suffix
bar
prefix_bar
(10)
((10) + 1)
 * check-output-end
 */
