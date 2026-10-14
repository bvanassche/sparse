/*
 * Check that __attribute__((packed)) on enums selects the smallest
 * integer type that can hold all enumerator values.
 */

enum e_u8 {
	U8_0 = 0,
	U8_1 = 200,
} __attribute__((packed));

enum e_s8 {
	S8_0 = -10,
	S8_1 = 100,
} __attribute__((packed));

enum __attribute__((packed)) e_u16 {
	U16_0 = 0,
	U16_1 = 1000,
};

enum e_s16 {
	S16_0 = -1000,
	S16_1 = 50,
} __attribute__((__packed__));

enum e_u32 {
	U32_0 = 0,
	U32_1 = 70000,
} __attribute__((packed));

_Static_assert(sizeof(enum e_u8) == 1, "e_u8 size");
_Static_assert(_Alignof(enum e_u8) == 1, "e_u8 align");
_Static_assert(sizeof(enum e_s8) == 1, "e_s8 size");
_Static_assert(_Alignof(enum e_s8) == 1, "e_s8 align");

_Static_assert(sizeof(enum e_u16) == 2, "e_u16 size");
_Static_assert(_Alignof(enum e_u16) == 2, "e_u16 align");
_Static_assert(sizeof(enum e_s16) == 2, "e_s16 size");
_Static_assert(_Alignof(enum e_s16) == 2, "e_s16 align");

_Static_assert(sizeof(enum e_u32) == 4, "e_u32 size");

/* Subsequent tag lookups must not clear the packed flag */
static enum e_u8 var_u8;
static enum e_u16 var_u16;
_Static_assert(sizeof(var_u8) == 1, "var_u8 size");
_Static_assert(sizeof(var_u16) == 2, "var_u16 size");

enum __attribute__((aligned(2), packed)) e_aligned_packed {
	AP_0 = 0,
};
_Static_assert(sizeof(enum e_aligned_packed) == 1, "e_aligned_packed size");
_Static_assert(_Alignof(enum e_aligned_packed) == 2, "e_aligned_packed align");

static enum e_aligned_packed bad_enum_arr[2];
typedef char __attribute__((aligned(2))) aligned_char;
static aligned_char bad_char_arr[2];
static aligned_char *ok_char_ptr_arr[2];
__attribute__((aligned(16))) static char ok_aligned_arr[2];
static enum e_u8 ok_u8_arr[4];

enum __attribute__((packed, mode(__SI__))) e_packed_si {
	PSI_0 = 0,
};
_Static_assert(sizeof(enum e_packed_si) == 4, "mode overrides packed");

enum __attribute__((packed)) e_fixed_packed : int {
	EFP_0 = 0,
};
_Static_assert(sizeof(enum e_fixed_packed) == sizeof(int),
	       "fixed type overrides packed");

_Static_assert(sizeof(U8_0) == sizeof(int), "enumerator has int size");
_Static_assert(sizeof(S8_0) == sizeof(int), "enumerator has int size");

/*
 * check-name: packed enum support
 * check-command: sparse $file
 *
 * check-error-start
enum-packed.c:49:42: warning: Clang and GCC treat 'packed' and 'aligned' attributes on enum types differently
enum-packed.c:55:42: error: size of array element is not a multiple of its alignment
enum-packed.c:57:33: error: size of array element is not a multiple of its alignment
 * check-error-end
 */
