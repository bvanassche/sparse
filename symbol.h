#ifndef SYMBOL_H
#define SYMBOL_H
/*
 * Basic symbol and namespace definitions.
 *
 * Copyright (C) 2003 Transmeta Corp.
 *               2003 Linus Torvalds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "token.h"
#include "target.h"

/*
 * An identifier with semantic meaning is a "symbol".
 *
 * There's a 1:n relationship: each symbol is always
 * associated with one identifier, while each identifier
 * can have one or more semantic meanings due to C scope
 * rules.
 *
 * The progression is symbol -> token -> identifier. The
 * token contains the information on where the symbol was
 * declared.
 */
enum namespace {
	NS_NONE = 0,
	NS_MACRO = 1,
	NS_TYPEDEF = 2,
	NS_STRUCT = 4,  // Also used for unions and enums.
	NS_LABEL = 8,
	NS_SYMBOL = 16,
	NS_ITERATOR = 32,
	NS_PREPROCESSOR = 64,
	NS_UNDEF = 128,
	NS_KEYWORD = 256,
};

enum type {
	SYM_UNINITIALIZED,
	SYM_PREPROCESSOR,
	SYM_BASETYPE,
	SYM_NODE,
	SYM_PTR,
	SYM_FN,
	SYM_ARRAY,
	SYM_STRUCT,
	SYM_UNION,
	SYM_ENUM,
	SYM_TYPEOF,
	SYM_TYPEOF_UNQUAL,
	SYM_BITFIELD,
	SYM_LABEL,
	SYM_RESTRICT,
	SYM_FOULED,
	SYM_KEYWORD,
	SYM_BAD,
};

enum keyword {
	KW_SPECIFIER 	= 1 << 0,
	KW_MODIFIER	= 1 << 1,
	KW_QUALIFIER	= 1 << 2,
	KW_ATTRIBUTE	= 1 << 3,
	KW_BUILTIN	= 1 << 4,
	KW_ASM		= 1 << 5,
	KW_MODE		= 1 << 6,
	KW_STATIC	= 1 << 7,
     // KW UNUSED	= 1 << 8,
	KW_EXACT	= 1 << 9,
};

struct range {
	long long		neg;
	unsigned long long	pos;
};

struct context {
	struct expression *context;
	unsigned int in, out;
};

extern struct context *alloc_context(void);

DECLARE_PTR_LIST(context_list, struct context);

struct ctype {
	struct symbol *base_type;
	unsigned long modifiers;
	unsigned long alignment;
	struct context_list *contexts;
	struct ident *as;
};

struct decl_state {
	struct ctype ctype;
	struct ident **ident;
	struct symbol_op *mode;
	struct expression *cleanup;
	unsigned long f_modifiers;		// function attributes
	unsigned long ident_modifiers;		// declarator-id C23 attributes
	unsigned long storage_class;
	unsigned long typedef_mods;
	unsigned int explicit_alignment;
	unsigned int typedef_alignment;
	unsigned char prefer_abstract;
	unsigned char autotype;
	unsigned char forced;
	unsigned char packed;
	unsigned char has_aligned;
	unsigned char has_auto;
	unsigned char has_alignas;
	unsigned char is_constexpr;
	unsigned char designated_init;
	unsigned char transparent_union;
	unsigned char opaque_fixed_enum;
	long long bitint_width;
	struct position bitint_pos;
	struct position opaque_fixed_enum_pos;
};

struct pseudo;
struct entrypoint;
struct arg;

struct symbol_op {
	enum keyword type;
	int (*evaluate)(struct expression *);
	int (*expand)(struct expression *, int);
	int (*args)(struct expression *);
	struct pseudo *(*linearize)(struct entrypoint *, struct expression *);

	/* keywords */
	struct token *(*declarator)(struct token *token, struct symbol *, struct decl_state *ctx);
	struct token *(*statement)(struct token *token, struct statement *stmt);
	struct token *(*toplevel)(struct token *token, struct symbol_list **list);
	struct token *(*attribute)(struct token *token, struct symbol *attr, struct decl_state *ctx);
	struct symbol *(*to_mode)(struct symbol *);
	void (*asm_modifier)(struct token *token, unsigned long *mods, unsigned long mod);

	int test, set, class;
};


#define SYM_ATTR_WEAK		0
#define SYM_ATTR_NORMAL		1
#define SYM_ATTR_STRONG		2

struct symbol {
	enum type type:8;
	enum namespace namespace:9;
	unsigned char used:1, attr:2, enum_member:1, bound:1, defined:1;
	struct position pos;		/* Where this symbol was declared */
	struct position endpos;		/* Where this symbol ends*/
	struct ident *ident;		/* What identifier this symbol is associated with */
	struct symbol *next_id;		/* Next semantic symbol that shares this identifier */
	struct symbol	*replace;	/* What is this symbol shadowed by in copy-expression */
	struct scope	*scope;
	union {
		struct symbol	*same_symbol;
		struct symbol	*next_subobject;
	};

	struct symbol_op *op;

	union {
		struct /* NS_MACRO */ {
			struct token *expansion;
			struct token *arglist;
			struct scope *used_in;
			void (*expand_simple)(struct token *);
			bool (*expand)(struct token *, struct arg *args);
		};
		struct /* NS_PREPROCESSOR */ {
			int (*handler)(struct stream *, struct token **, struct token *);
			int normal;
		};
		struct /* NS_LABEL */ {
			struct scope *label_scope;
			struct position label_pos;
			unsigned long label_modifiers;
		};
		struct /* NS_SYMBOL */ {
			unsigned long	offset;
			int		bit_size;
			int bitint_width;
			unsigned int explicit_alignment;
			unsigned int bit_offset : 8;
			unsigned int bogus_linear : 1;
			unsigned int variadic : 1;
			unsigned int initialized : 1;
			unsigned int examined : 1;
			unsigned int expanding : 1;
			unsigned int evaluated : 1;
			unsigned int has_flex_array : 1;
			unsigned int string : 1;
			unsigned int designated_init : 1;
			unsigned int forced_arg : 1;
			unsigned int accessed : 1;
			unsigned int builtin : 1;
			unsigned int torename : 1;
			unsigned int packed : 1;
			unsigned int transparent_union : 1;
			unsigned int fixed_type : 1;
			unsigned int explicit_typedef : 1;
			unsigned int has_alignas : 1;
			unsigned int kr_args : 1;
			unsigned int underspecified : 1;
			unsigned int c23_auto : 1;
			unsigned int has_mode : 1;
			unsigned int has_aligned : 1;
			int		rank:3;	// arithmetic's rank
			struct expression *array_size;
			struct ctype ctype;
			struct symbol_list *arguments;
			struct statement *stmt;
			struct symbol_list *symbol_list;
			struct statement *inline_stmt;
			struct symbol_list *inline_symbol_list;
			struct expression *initializer;
			struct expression *cleanup;
			struct range range;
			struct entrypoint *ep;
			struct symbol *definition;
		};
	};
	union /* backend */ {
		struct basic_block *bb_target;	/* label */
		void *aux;			/* Auxiliary info, e.g. backend information */
		struct {
			char kind;		/* used by ctags & dissect */
			unsigned char visited:1;
			unsigned char inspected:1;
		};
	};
	pseudo_t pseudo;
};

/* Modifiers */
#define MOD_AUTO		0x00000001
#define MOD_REGISTER		0x00000002
#define MOD_STATIC		0x00000004
#define MOD_EXTERN		0x00000008
#define MOD_TOPLEVEL		0x00000010	// scoping..
#define MOD_TLS			0x00000020
#define MOD_ASM_GOTO		MOD_TLS		// never used together
#define MOD_INLINE		0x00000040

#define MOD_ASSIGNED		0x00000080
#define MOD_ADDRESSABLE		0x00000100

#define MOD_CONST		0x00000200
#define MOD_VOLATILE		0x00000400
#define MOD_RESTRICT		0x00000800
#define MOD_ATOMIC		0x00001000

#define MOD_SIGNED		0x00002000
#define MOD_UNSIGNED		0x00004000
#define MOD_EXPLICITLY_SIGNED	0x00008000

#define MOD_GNU_INLINE		0x00010000
#define MOD_USERTYPE		0x00020000
#define MOD_CONSTEXPR 0x00040000
#define MOD_BITINT 0x00080000
#define MOD_WEAK		0x00100000
#define MOD_ALIGNED		0x00200000

#define MOD_UNUSED		0x00400000
#define MOD_SAFE		0x00800000	// non-null/non-trapping pointer
#define MOD_PURE		0x01000000
#define MOD_BITWISE		0x02000000
#define MOD_NOCAST		0x04000000
#define MOD_NODEREF		0x08000000
#define MOD_NORETURN		0x10000000
#define MOD_EXT_VISIBLE		0x20000000
#define MOD_NONSTRING		0x40000000
#define MOD_FCONST		0x80000000UL


#define MOD_ACCESS	(MOD_ASSIGNED | MOD_ADDRESSABLE)
#define MOD_NONLOCAL	(MOD_EXTERN | MOD_TOPLEVEL)
#define MOD_STORAGE	(MOD_AUTO | MOD_REGISTER | MOD_STATIC | MOD_EXTERN | MOD_INLINE | MOD_TOPLEVEL)
#define MOD_ESIGNED	(MOD_SIGNED | MOD_EXPLICITLY_SIGNED)
#define MOD_SIGNEDNESS	(MOD_SIGNED | MOD_UNSIGNED | MOD_EXPLICITLY_SIGNED)
#define MOD_SPECIFIER (MOD_SIGNEDNESS | MOD_BITINT)
#define MOD_IGNORE	(MOD_STORAGE | MOD_ACCESS | MOD_USERTYPE | MOD_EXPLICITLY_SIGNED | MOD_EXT_VISIBLE | MOD_UNUSED | MOD_GNU_INLINE | MOD_WEAK | MOD_ALIGNED | MOD_NONSTRING | MOD_CONSTEXPR)
#define MOD_QUALIFIER	(MOD_CONST | MOD_VOLATILE | MOD_RESTRICT)
/* modifiers for function attributes */
#define MOD_FUN_ATTR	(MOD_PURE|MOD_NORETURN|MOD_FCONST)
#define MOD_PTRINHERIT	(MOD_QUALIFIER | MOD_ATOMIC | MOD_NODEREF | MOD_FUN_ATTR | MOD_NOCAST)
/* modifiers preserved by typeof() operator */
#define MOD_TYPEOF	(MOD_QUALIFIER | MOD_ATOMIC | MOD_NOCAST | MOD_SPECIFIER)
/* like cvr-qualifiers but 'reversed' (OK: source <= target) */
#define MOD_REV_QUAL	(MOD_PURE|MOD_NORETURN|MOD_FCONST)
/* do not warn when these are duplicated */
#define MOD_DUP_OK	(MOD_UNUSED|MOD_GNU_INLINE|MOD_WEAK|MOD_ALIGNED|MOD_USERTYPE|MOD_NONSTRING|MOD_FUN_ATTR)
/* must be part of the declared symbol, not its type */
#define MOD_DECLARE                                                         \
	(MOD_STORAGE | MOD_INLINE | MOD_TLS | MOD_GNU_INLINE | MOD_UNUSED | \
	 MOD_PURE | MOD_NORETURN | MOD_FCONST | MOD_EXT_VISIBLE |           \
	 MOD_CONSTEXPR | MOD_WEAK | MOD_ALIGNED | MOD_NONSTRING)

/* Current parsing/evaluation function */
extern struct symbol *current_fn;

/* Abstract types */
extern struct symbol	int_type,
			fp_type;

/* C types */
extern struct symbol	bool_ctype, void_ctype, type_ctype,
			char_ctype, schar_ctype, uchar_ctype,
			short_ctype, sshort_ctype, ushort_ctype,
			int_ctype, sint_ctype, uint_ctype,
			long_ctype, slong_ctype, ulong_ctype,
			llong_ctype, sllong_ctype, ullong_ctype,
			int128_ctype, sint128_ctype, uint128_ctype,
			float_ctype, double_ctype, ldouble_ctype,
			string_ctype, ptr_ctype, lazy_ptr_ctype,
			incomplete_ctype, label_ctype, bad_ctype,
			null_ctype, nullptr_ctype;
extern struct symbol	autotype_ctype;
extern struct symbol	schar_ptr_ctype, short_ptr_ctype;
extern struct symbol	int_ptr_ctype, uint_ptr_ctype;
extern struct symbol	long_ptr_ctype, ulong_ptr_ctype;
extern struct symbol	llong_ptr_ctype, ullong_ptr_ctype;
extern struct symbol	size_t_ptr_ctype, intmax_ptr_ctype, ptrdiff_ptr_ctype;
extern struct symbol	float32_ctype, float32x_ctype;
extern struct symbol	float64_ctype, float64x_ctype;
extern struct symbol	float128_ctype;
extern struct symbol	const_void_ctype, const_char_ctype;
extern struct symbol	const_ptr_ctype, const_string_ctype;
extern struct symbol	const_wchar_ctype, const_wstring_ctype;
extern struct symbol	volatile_void_ctype, volatile_ptr_ctype;
extern struct symbol	const_volatile_void_ctype, const_volatile_ptr_ctype;
extern struct symbol	volatile_bool_ctype, volatile_bool_ptr_ctype;

/* Special internal symbols */
extern struct symbol	zero_int;

#define __IDENT(n,str,res) \
	extern struct ident n
#include "ident-list.h"


extern struct symbol_list *translation_unit_used_list;

extern void access_symbol(struct symbol *);

extern const char * type_difference(struct ctype *c1, struct ctype *c2,
	unsigned long mod1, unsigned long mod2);

extern struct symbol *lookup_symbol(struct ident *, enum namespace);
extern struct symbol *create_symbol(int stream, const char *name, int type, int namespace);
extern void init_symbols(void);
extern void init_builtins(int stream);
extern void init_linearized_builtins(int stream);
extern void init_ctype(void);
extern struct symbol *alloc_symbol(struct position, int type);
extern void show_type(struct symbol *);
extern const char *modifier_name(unsigned long mod);
extern const char *modifier_string(unsigned long mod);
extern void show_symbol(struct symbol *);
extern int show_symbol_expr_init(struct symbol *sym);
extern void show_type_list(struct symbol *);
extern void show_symbol_list(struct symbol_list *);
extern void add_symbol(struct symbol_list **, struct symbol *);
extern void bind_symbol(struct symbol *, struct ident *, enum namespace);
extern void bind_symbol_with_scope(struct symbol *, struct ident *, enum namespace, struct scope *);

extern struct symbol *strip_array_qualifiers(struct symbol *);
extern struct symbol *examine_symbol_type(struct symbol *);
extern struct symbol *examine_pointer_target(struct symbol *);
extern struct symbol *get_enum_base_type(struct range range, int packed);
extern const char *show_as(struct ident *as);
extern const char *show_typename(struct symbol *sym);
extern const char *builtin_typename(struct symbol *sym);
extern const char *builtin_type_suffix(struct symbol *sym);
extern const char *builtin_ctypename(struct ctype *ctype);
extern const char* get_type_name(enum type type);

extern void debug_symbol(struct symbol *);
extern void merge_type(struct symbol *sym, struct symbol *base_type);
extern void check_declaration(struct symbol *sym);
extern void check_duplicates(struct symbol *sym);

static inline int valid_type(const struct symbol *ctype)
{
	return ctype && ctype != &bad_ctype;
}

static inline struct symbol *get_base_type(const struct symbol *sym)
{
	return examine_symbol_type(sym->ctype.base_type);
}

///
// test if type is an integer type
//
// @return: ``1`` for plain integer type, enums & bitfields
//	but ``0`` for bitwise types!
static inline int is_int_type(const struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	if (type->type == SYM_ENUM)
		type = type->ctype.base_type;
	return type->type == SYM_BITFIELD ||
	       type->ctype.base_type == &int_type;
}

static inline int is_enum_type(const struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return (type->type == SYM_ENUM);
}

static inline int is_signed_type(struct symbol *sym)
{
	if (sym->type == SYM_NODE)
		sym = sym->ctype.base_type;
	if (sym->type == SYM_PTR)
		return 0;
	return !(sym->ctype.modifiers & MOD_UNSIGNED);
}

static inline int is_bitint_type(const struct symbol *sym)
{
	while (sym && sym->type == SYM_NODE)
		sym = sym->ctype.base_type;
	return sym && (sym->ctype.modifiers & MOD_BITINT);
}

static inline int effective_int_width(const struct symbol *type)
{
	if (!type)
		return 0;
	while (type->type == SYM_NODE) {
		type = type->ctype.base_type;
		if (!type)
			return 0;
	}
	if ((type->ctype.modifiers & MOD_BITINT) && type->bitint_width > 0)
		return type->bitint_width;
	return type->bit_size;
}

extern struct symbol *get_bitint_type(int width, int is_signed);

static inline int is_type_type(struct symbol *type)
{
	return type == &type_ctype;
}

static inline int is_ptr_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->type == SYM_PTR || type->type == SYM_ARRAY || type->type == SYM_FN;
}

static inline int is_func_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->type == SYM_FN;
}

static inline struct symbol *cbase_type(struct symbol *sym)
{
	while (sym && sym->type == SYM_NODE)
		sym = sym->ctype.base_type;
	return sym;
}

static inline int is_array_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->type == SYM_ARRAY;
}

static inline int is_struct_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->type == SYM_STRUCT;
}

static inline int is_union_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->type == SYM_UNION;
}

static inline int is_float_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->ctype.base_type == &fp_type;
}

static inline int is_byte_type(struct symbol *type)
{
	return type->bit_size == bits_in_char && type->type != SYM_BITFIELD &&
	       !is_bitint_type(type);
}

static inline int is_wchar_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type == wchar_ctype;
}

static inline int is_void_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type == &void_ctype;
}

static inline int is_bool_type(struct symbol *type)
{
	while (type && (type->type == SYM_NODE || type->type == SYM_ENUM ||
			type->type == SYM_BITFIELD))
		type = type->ctype.base_type;
	return type == &bool_ctype;
}

static inline int is_nullptr_type(const struct symbol *type)
{
	if (type && type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type == &nullptr_ctype;
}

static inline int is_scalar_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	switch (type->type) {
	case SYM_ENUM:
	case SYM_BITFIELD:
	case SYM_PTR:
	case SYM_RESTRICT:	// OK, always integer types
	case SYM_FOULED:	// idem
		return 1;
	default:
		break;
	}
	if (type->ctype.base_type == &int_type)
		return 1;
	if (type->ctype.base_type == &fp_type)
		return 1;
	if (type == &nullptr_ctype)
		return 1;
	return 0;
}

/// return true for integer & pointer types
static inline bool is_integral_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	switch (type->type) {
	case SYM_ENUM:
	case SYM_PTR:
	case SYM_RESTRICT:	// OK, always integer types
	case SYM_FOULED:	// idem
		return 1;
	default:
		break;
	}
	if (type->ctype.base_type == &int_type)
		return 1;
	if (type == &nullptr_ctype)
		return 1;
	return 0;
}

static inline int is_function(struct symbol *type)
{
	return type && type->type == SYM_FN;
}

static inline int is_extern_inline(struct symbol *sym)
{
	return (sym->ctype.modifiers & MOD_EXTERN) &&
		(sym->ctype.modifiers & MOD_INLINE) &&
		is_function(sym->ctype.base_type);
}

static inline int has_flexible_array(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	return type->has_flex_array;
}

static inline int get_sym_type(struct symbol *type)
{
	if (type->type == SYM_NODE)
		type = type->ctype.base_type;
	if (type->type == SYM_ENUM)
		type = type->ctype.base_type;
	return type->type;
}

static inline long long extend_value(long long val, struct symbol *ctype)
{
	int is_signed = !(ctype->ctype.modifiers & MOD_UNSIGNED);
	unsigned size = effective_int_width(ctype);

	return bits_extend(val, size, is_signed);
}

static inline struct symbol *lookup_keyword(struct ident *ident, enum namespace ns)
{
	if (!ident->keyword)
		return NULL;
	return lookup_symbol(ident, ns);
}

#define is_restricted_type(type) (get_sym_type(type) == SYM_RESTRICT)
#define is_fouled_type(type) (get_sym_type(type) == SYM_FOULED)
#define is_bitfield_type(type)   (get_sym_type(type) == SYM_BITFIELD)

void create_fouled(struct symbol *type);
struct symbol *befoul(struct symbol *type);


extern struct ident bad_address_space;

static inline bool valid_as(struct ident *as)
{
	return as && as != &bad_address_space;
}

static inline void combine_address_space(struct position pos,
	struct ident **tas, struct ident *sas)
{
	struct ident *as;
	if (!sas)
		return;
	as = *tas;
	if (!as)
		*tas = sas;
	else if (as != sas) {
		*tas = &bad_address_space;
		sparse_error(pos, "multiple address spaces given");
	}
}

#endif /* SYMBOL_H */
