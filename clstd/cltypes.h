#ifndef _CLTYPES_H_

#ifndef _CL_STD_CODE_
#error Must be include "clstd.h" first.
#endif // _CL_STD_CODE_

#define _CLTYPES_H_

#ifndef NULL
# if _MSC_VER >= 1600
#   define NULL nullptr
# else
#   define NULL 0L
# endif
#endif // NULL

#if defined(_WINDOWS) || defined(_WIN32)
typedef __int8              s8, i8;
typedef __int16             s16, i16;
typedef __int32             s32, i32;
typedef __int64             s64, i64;
//typedef __int128      s128, i128;

typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32    u32;
typedef unsigned __int64    u64;
//typedef unsigned __int128  u128;

typedef signed char         b8;    // boolean
typedef signed __int32      b32;   // boolean

typedef size_t              clsize;
#else

typedef signed char         s8, i8;
typedef signed short        s16, i16;
typedef signed long         s32, i32;
typedef signed long long    s64, i64;
//typedef __int128      s128, i128;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef uint32_t            u32;
typedef unsigned long long  u64;
//typedef unsigned __int128  u128;

typedef s8                  b8;    // b32
typedef s32                 b32;   // b32

typedef size_t              clsize;
#endif

#if defined(_X86)
typedef _w64 i32            i32_ptr;
typedef _w64 s32            s32_ptr;
typedef _w64 u32            u32_ptr;
#elif defined(_X64)
typedef i64                 i32_ptr;
typedef s64                 s32_ptr;
typedef u64                 u32_ptr;
#elif defined(_ANDROID)
typedef i32                 i32_ptr;
typedef s32                 s32_ptr;
typedef u32                 u32_ptr;
#endif // #if defined(_X86)

typedef char                ch;
typedef char                ach;
#if defined(_WIN32) || defined(_WINDOWS)
typedef wchar_t             wch;
#elif defined(_IOS)
typedef wchar_t             wch;
#elif defined(_ANDROID)
typedef wchar_t             wch;
#endif

//#ifdef _UNICODE
//typedef wch                 tch;
//#else
//typedef ch                  tch;
//#endif // _UNICODE

//namespace clstd
//{
//  namespace regular
//  {
//    typedef u8               BYTE;
//    typedef u16              WORD;
//    typedef u32              DWORD;
//    typedef u64              QWORD;
//    typedef b32              BOOL;
//    typedef u32_ptr          SIZE_T;
//    typedef void*            LPVOID;
//    typedef const void*      LPCVOID;
//    typedef i16              SHORT;
//    typedef i32              INT;
//    typedef i32              LONG;
//    typedef void*            HANDLE;
//    typedef ch*              LPSTR;
//    typedef const ch*        LPCSTR;
//    typedef wch*             LPWSTR;
//    typedef const wch*       LPCWSTR;
//    typedef tch*             LPTSTR;
//    typedef const tch*       LPCTSTR;
//    typedef float            FLOAT;
//  } // namespace regular
//} // namespace clstd

typedef u8                CLBYTE;
typedef u8*               CLLPBYTE;
typedef unsigned short    CLWORD;
typedef unsigned long     CLDWORD;
typedef u32               CLUINT;
typedef u32*              CLLPUINT;
typedef u32               CLULONG;
typedef u32*              CLLPULONG;
typedef u64               CLQWORD;
typedef void*             CLLPVOID;
typedef const void*       CLLPCVOID;
typedef ch                CLCHAR;
typedef ch*               CLLPSTR;
typedef const ch*         CLLPCSTR;
typedef wch               CLWCHAR;
typedef wch*              CLLPWSTR;
typedef const wch*        CLLPCWSTR;
//typedef tch*              CLLPTSTR;
//typedef const tch*        CLLPCTSTR;
typedef int               CLINT;
typedef long              CLLONG;
typedef float             CLFLOAT;
typedef void              CLVOID;

typedef u16               CLUSHORT;

#if defined(_WIN32) || defined(_WINDOWS)
typedef _w64 CLDWORD      CLDWORD_PTR;
typedef _w64 CLINT        CLINT_PTR;
typedef _w64 CLLONG       CLLONG_PTR;
typedef _w64 CLUINT       CLUINT_PTR;
typedef _w64 CLULONG      CLULONG_PTR;
#else
typedef CLDWORD           CLDWORD_PTR;
typedef CLINT             CLINT_PTR;
typedef CLLONG            CLLONG_PTR;
typedef CLUINT            CLUINT_PTR;
typedef CLULONG           CLULONG_PTR;
#endif


#ifndef BYTE_MAX
#define BYTE_MAX 0xff
#endif

#ifndef SCHAR_MIN
#define SCHAR_MIN   (-128)
#endif

#ifndef SCHAR_MAX
#define SCHAR_MAX     127
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX     0xff
#endif

#ifndef SHRT_MIN
#define SHRT_MIN    (-32768)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX      32767
#endif
#ifndef USHRT_MAX
#define USHRT_MAX     0xffff
#endif
#ifndef INT_MIN
#define INT_MIN     (-2147483647 - 1)
#endif
#ifndef INT_MAX
#define INT_MAX       2147483647  // 0x7FFFFFFF
#endif
#ifndef UINT_MAX
#define UINT_MAX      0xffffffff
#endif
#ifndef LONG_MIN
#define LONG_MIN    (-2147483647L - 1)
#endif
#ifndef LONG_MAX
#define LONG_MAX      2147483647L
#endif
#ifndef ULONG_MAX
#define ULONG_MAX     0xffffffffUL
#endif
#ifndef LLONG_MAX
#define LLONG_MAX     9223372036854775807i64
#endif
#ifndef LLONG_MIN
#define LLONG_MIN   (-9223372036854775807i64 - 1)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX    0xffffffffffffffffui64
#endif

#ifndef FLT_MAX
#define FLT_MAX         3.402823466e+38F        /* max value */
#endif 
#ifndef FLT_MAX_10_EXP
#define FLT_MAX_10_EXP  38                      /* max decimal exponent */
#endif
#ifndef FLT_MAX_EXP
#define FLT_MAX_EXP     128                     /* max binary exponent */
#endif
#ifndef FLT_MIN
#define FLT_MIN         1.175494351e-38F        /* min positive value */
#endif

#if defined(_WINDOWS) || defined(_WIN32)
//#include <string>
//#include <xstring>
#include <vector>
#include <hash_map>
#include <hash_set>
#include <map>
#include <stack>
#include <queue>
#include <list>
#include <set>
//typedef std::string string;
//typedef std::wstring wstring;
#define clvector    std::vector
#define clhash_map  stdext::hash_map
#define clhash_set  stdext::hash_set
#define clmap       std::map
#define clstack     std::stack
#define clset       std::set
#define clqueue     std::queue
#define cllist      std::list
#define clist       std::list

#elif _IOS
#include <string>
#include <vector>
#include <hash_map.h>
#include <hash_set.h>
#include <map>
#include <stack>
#include <list>
typedef std::string string;
typedef std::basic_string<wch> wstring;
#define clvector    std::vector
#define clhash_map  hash_map
#define clhash_set  hash_set
#define clset       std::set
#define clmap       std::map
#define clstack     std::stack
#define clqueue     std::queue
#define cllist      std::list
#define clist       std::list
#else
#include <string>
#include <vector>
#include <hash_map>
#include <hash_set>
#include <map>
#include <stack>
#include <list>
typedef std::string string;
typedef std::wstring wstring;
#define clvector    std::vector
#define clhash_map  std::hash_map
#define clhash_set  std::hash_set
#define clmap       std::map
#define clset       std::set
#define clstack     std::stack
#define clqueue     std::queue
#define cllist      std::list
#define clist       std::list
#endif 

#else
#pragma message(__FILE__ ": warning : Duplicate included this file.")
#endif // _CLTYPES_H_