struct bfb {
	_Bool a:1;
	_Bool f:1;
	_Bool z:1;
};


struct bfb foo(struct bfb s)
{
	return s;
}

/*
 * check-name: bitfield-bool-layout
 * check-description: Verify that consecutive _Bool bitfields pack into a
 *	single 1-byte (8-bit) storage unit rather than allocating a separate
 *	byte per bitfield (24 bits).
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: ret\\.24
 * check-output-contains: ret\\.8
 */
