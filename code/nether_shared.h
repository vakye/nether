
#pragma once

// NOTE(vak): Keywords

#define local   static // NOTE(vak): Function/variable is only visible within the current translation unit
#define persist static // NOTE(vak): Variable retains its value even after the function exits

// NOTE(vak): Macros

#define CTAssert(Expression) _Static_assert(Expression, "Compile-time assertion failed")
#define Assert(Expression) if (!(Expression)) *(int*)0 = 0

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

// NOTE(vak): Integer types

typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef s64 ssize;
typedef u64 usize;

// NOTE(vak): Floating point types

typedef float  f32;
typedef double f64;

// NOTE(vak): Boolean types

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

// NOTE(vak): Constants

#define true  (1)
#define false (0)

#define S8Min  (( s8)(-128ll))
#define S16Min ((s16)(-32768ll))
#define S32Min ((s32)(-2147483648ll))
#define S64Min ((s64)(-9223372036854775808ll))

#define S8Max  (( s8)(+127ll))
#define S16Max ((s16)(+32767ll))
#define S32Max ((s32)(+2147483647ll))
#define S64Max ((s64)(+9223372036854775807ll))

#define U8Max  (( u8)(255ull))
#define U16Max ((u16)(65535ull))
#define U32Max ((u32)(4294967295ull))
#define U64Max ((u64)(18446744073709551615ull))

// NOTE(vak): String

typedef struct
{
    char* Data;
    usize Size;
} string;

#define Str(Literal)        (string){Literal, sizeof(Literal) - 1}
#define StrData(Data, Size) (string){Data, Size}
