enum E1 : char { A = 300 };
enum E2 : unsigned char { B = -1 };
enum E3 : unsigned char { C = 255, D };
enum E4 : short;
enum E4 : int;
enum E5;
enum E5 : short;
enum E6 : short;
enum E6 { F };
enum E7 : float;
enum E8 : bool { B0 = 0, B1, B2 };
enum E9 : unsigned long long { UMAX = ~0ULL, UOVER };
enum E10 : int bad_var;
typedef enum E11 : short e11_t;
void bad_enum_param(enum E12 : int p);
int bad_enum_sizeof = sizeof(enum E13 : int);
enum E14 : char;
enum E14 : signed char;

/*
 * check-name: C23 enum with fixed underlying type errors
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-enum-type-error.c:1:18: error: enumerator value outside the range of underlying type
c23-enum-type-error.c:2:27: error: enumerator value outside the range of underlying type
c23-enum-type-error.c:3:36: error: enumerator value outside the range of underlying type
c23-enum-type-error.c:5:14: error: 'enum' underlying type incompatible with previous declaration
c23-enum-type-error.c:7:16: error: 'enum' declared both with and without fixed underlying type
c23-enum-type-error.c:9:9: error: 'enum' declared with but defined without fixed underlying type
c23-enum-type-error.c:10:16: error: invalid 'enum' underlying type
c23-enum-type-error.c:11:30: error: enumerator value outside the range of underlying type
c23-enum-type-error.c:12:46: error: enumerator value outside the range of underlying type
c23-enum-type-error.c:13:10: error: 'enum' underlying type may not be specified here
c23-enum-type-error.c:14:18: error: 'enum' underlying type may not be specified here
c23-enum-type-error.c:15:30: error: 'enum' underlying type may not be specified here
c23-enum-type-error.c:16:39: error: 'enum' underlying type may not be specified here
c23-enum-type-error.c:18:23: error: 'enum' underlying type incompatible with previous declaration
 * check-error-end
 */
