typedef int aligned16 __attribute__((aligned(16)));

/* 1. Post-brace packed struct with explicitly aligned member */
struct S1 {
	char a;
	int b __attribute__((aligned(8)));
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S1, b) == 8, "S1 offset b");
_Static_assert(_Alignof(struct S1) == 8, "S1 align");
_Static_assert(sizeof(struct S1) == 16, "S1 size");

/* 2. Pre-tag packed struct with explicitly aligned member */
struct __attribute__((packed)) S2 {
	char a;
	int b __attribute__((aligned(8)));
};
_Static_assert(__builtin_offsetof(struct S2, b) == 8, "S2 offset b");
_Static_assert(_Alignof(struct S2) == 8, "S2 align");
_Static_assert(sizeof(struct S2) == 16, "S2 size");

/* 3. Struct with both packed and aligned(8) on the struct itself */
struct S3 {
	char a;
	int b;
} __attribute__((packed, aligned(8)));
_Static_assert(__builtin_offsetof(struct S3, b) == 1, "S3 offset b");
_Static_assert(_Alignof(struct S3) == 8, "S3 align");
_Static_assert(sizeof(struct S3) == 8, "S3 size");

/* 4. Nested S1 inside packed S4 without explicit aligned on inner */
struct S4 {
	char a;
	struct S1 inner;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S4, inner) == 1, "S4 offset inner");
_Static_assert(_Alignof(struct S4) == 1, "S4 align");
_Static_assert(sizeof(struct S4) == 17, "S4 size");

/* 5. Nested S3 inside packed S5 without explicit aligned on inner */
struct S5 {
	char a;
	struct S3 inner;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S5, inner) == 1, "S5 offset inner");
_Static_assert(_Alignof(struct S5) == 1, "S5 align");
_Static_assert(sizeof(struct S5) == 9, "S5 size");

/* 6. Nested S1 inside packed S6 with smaller explicit aligned(4) on inner */
struct S6 {
	char a;
	struct S1 inner __attribute__((aligned(4)));
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S6, inner) == 4, "S6 offset inner");
_Static_assert(_Alignof(struct S6) == 4, "S6 align");
_Static_assert(sizeof(struct S6) == 20, "S6 size");

/* 7. Unpacked struct with member carrying both packed and aligned(8) */
struct S7 {
	char a;
	int b __attribute__((packed, aligned(8)));
};
_Static_assert(__builtin_offsetof(struct S7, b) == 8, "S7 offset b");
_Static_assert(_Alignof(struct S7) == 8, "S7 align");
_Static_assert(sizeof(struct S7) == 16, "S7 size");

/* 8. _Alignas(8) and _Alignas(0) inside packed structs */
struct S8 {
	char a;
	_Alignas(8) int b;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S8, b) == 8, "S8 offset b");
_Static_assert(_Alignof(struct S8) == 8, "S8 align");
_Static_assert(sizeof(struct S8) == 16, "S8 size");

struct S9 {
	char a;
	_Alignas(0) int b;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct S9, b) == 1, "S9 offset b");
_Static_assert(_Alignof(struct S9) == 1, "S9 align");
_Static_assert(sizeof(struct S9) == 5, "S9 size");

/* 9. Aligned typedef inside packed struct / packed field / pointer */
struct ST1 {
	char a;
	aligned16 b;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct ST1, b) == 1, "ST1 offset b");
_Static_assert(_Alignof(struct ST1) == 1, "ST1 align");
_Static_assert(sizeof(struct ST1) == 5, "ST1 size");

struct ST2 {
	char a;
	aligned16 b __attribute__((packed));
};
_Static_assert(__builtin_offsetof(struct ST2, b) == 1, "ST2 offset b");
_Static_assert(_Alignof(struct ST2) == 1, "ST2 align");
_Static_assert(sizeof(struct ST2) == 5, "ST2 size");

struct ST3 {
	char a;
	aligned16 b __attribute__((aligned(8)));
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct ST3, b) == 8, "ST3 offset b");
_Static_assert(_Alignof(struct ST3) == 8, "ST3 align");
_Static_assert(sizeof(struct ST3) == 16, "ST3 size");

struct ST4 {
	char a;
	aligned16 *p;
} __attribute__((packed));
_Static_assert(__builtin_offsetof(struct ST4, p) == 1, "ST4 offset p");
_Static_assert(_Alignof(struct ST4) == 1, "ST4 align");
_Static_assert(sizeof(struct ST4) == 1 + sizeof(void *), "ST4 size");

/* 10. Packed union with explicitly aligned member */
union U1 {
	char a;
	int b __attribute__((aligned(8)));
} __attribute__((packed));
_Static_assert(_Alignof(union U1) == 8, "U1 align");
_Static_assert(sizeof(union U1) == 8, "U1 size");

/* 11. Packed struct with explicitly aligned bitfield */
struct BF1 {
	unsigned char a : 1;
	unsigned int b : 3 __attribute__((aligned(8)));
} __attribute__((packed));
_Static_assert(_Alignof(struct BF1) == 8, "BF1 align");
_Static_assert(sizeof(struct BF1) == 16, "BF1 size");

/*
 * check-name: struct-packed-aligned-layout
 * check-command: sparse $file
 */
