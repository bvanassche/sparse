/*
 * Cleanup expressions are copied after expansion. expand_compound()
 * collapses single-statement compound statements, so the statement of
 * a statement expression is not necessarily a compound statement anymore
 * when the cleanup expression is copied. Additionally, asm memory
 * operands have already been converted into addresses and must keep
 * their constraint flags when copied.
 */
struct s {
	int a;
	int counter;
};

static inline void release(struct s **p)
{
	struct s *v = *p;

	({ asm volatile("inc %0" : "+m" (v->counter)); });
	({ asm volatile("dec %0" : "=m" (v->a) : "m" (v->a)); });
}

int f(struct s *v)
{
	struct s *__attribute__((cleanup(release))) x = v;

	return 0;
}

/*
 * check-name: cleanup-stmt-expr-asm
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-start
f:
.L0:
	<entry-point>
	add.64      %r6 <- %arg1, $4
	asm         "inc %0"
		out: "+m" (%r6)
	asm         "dec %0"
		out: "=m" (%arg1)
		in: "m" (%arg1)
	# call      release, %r2
	ret.32      $0


 * check-output-end
 */
