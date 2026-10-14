struct B;
struct A {
	struct B *b;
};
struct B {
	struct A *a;
};

/*
 * check-name: c2xml-recursive-types
 * check-command: c2xml $file
 * check-output-ignore
 * check-output-contains: <symbol type="struct".*ident="A"
 * check-output-contains: <symbol type="struct".*ident="B"
 */
