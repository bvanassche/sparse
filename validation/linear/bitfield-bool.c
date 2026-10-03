struct bfb {
	_Bool a:1;
	_Bool f:1;
	_Bool z:1;
};

_Bool get_z(struct bfb *p)
{
	return p->z;
}

void set_f_one(struct bfb *p)
{
	p->f = 1;
}

void set_f_two(struct bfb *p)
{
	p->f = 2;
}

void set_f_ptr(struct bfb *p, int *ptr)
{
	p->f = ptr;
}

int init_bfb(void)
{
	struct bfb s = { .a = 1, .z = 1 };
	return s.a + s.f + s.z;
}

/*
 * check-name: bitfield-bool-linearize
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: load\\.1
 * check-output-excludes: store\\.1
 * check-output-contains: load\\.8
 * check-output-contains: store\\.8
 * check-output-pattern(2): or\\.8.*\\$2
 * check-output-contains: ret\\.32.*\\$2
 */
