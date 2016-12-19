#ifndef _GRAPX_BASE_TYPES_H_
#define _GRAPX_BASE_TYPES_H_

#if defined(GXDLL_API_EXPORTS)
#define GXDLL       __declspec(dllexport)
#elif defined(GXDLL_API_IMPORTS)
#define GXDLL       __declspec(dllimport)
#elif defined(GXDLL_API_INDEPENDENT)
#define GXDLL
#else
#define GXDLL       __declspec(dllimport)
#endif

#ifdef _WIN32
#define _GXW64                 __w64
#define GXCALLBACK             __stdcall
# ifdef __clang__
#   define GXAPI
# else
#   define GXAPI                  __stdcall
# endif
#elif defined(_CL_SYSTEM_IOS)
#define _GXW64
#define GXCALLBACK
#define GXAPI
#else
#define _GXW64
#define GXCALLBACK             
#define GXAPI                  __stdcall
#endif

#define GXDLLAPI               GXDLL GXAPI


//#define TRUE                 1
//#define FALSE                0
//#define NULL                 0
//#define GXCONST                const
#define GXPURE                 = 0


typedef char            GXCHAR;
typedef unsigned char   GXUCHAR;
typedef short           GXSHORT;
typedef unsigned short  GXUSHORT;
typedef int             GXINT;
typedef unsigned int    GXUINT;
typedef long            GXLONG;
typedef unsigned long   GXULONG;
typedef float           GXFLOAT;
typedef void            GXVOID;
typedef void*           GXLPVOID;
typedef const void*     GXLPCVOID;

typedef i8              GXINT8;
typedef u8              GXUINT8;
typedef i16             GXINT16;
typedef u16             GXUINT16;
typedef i64             GXINT64;
typedef u64             GXUINT64;

typedef CLBYTE          GXBYTE;
typedef CLWORD          GXWORD;
typedef CLDWORD         GXDWORD;
typedef CLQWORD         GXQWORD;

typedef i8              GXBOOL8;
typedef i16             GXBOOL16;
typedef int             GXBOOL;

typedef wch             GXWCHAR;
typedef size_t          GXSIZE_T;

typedef unsigned long   GXCOLORREF;
typedef unsigned long   GXCOLOR;

typedef char              *GXLPCHAR, *LPGXCHAR;
typedef GXINT*            GXLPINT;
typedef GXUINT*           GXLPUINT;
typedef GXLONG*           GXLPLONG;
typedef GXBYTE*           GXLPBYTE;
typedef GXSHORT*          GXLPSHORT;
typedef GXDWORD*          GXLPDWORD;
typedef GXBOOL*           GXLPBOOL;

typedef const GXINT*      GXLPCINT;
typedef const GXUINT*     GXLPCUINT;
typedef const GXLONG*     GXLPCLONG;
typedef const GXBYTE*     GXLPCBYTE;
typedef const GXSHORT*    GXLPCSHORT;
typedef const GXDWORD*    GXLPCDWORD;

typedef i64               GXLONGLONG;

// 衍生类型定义
#if defined(_CL_ARCH_X86) || defined(_CL_ARCH_ARM)
typedef _GXW64 GXUINT   GXUINT_PTR;
typedef _GXW64 GXINT    GXINT_PTR;
typedef _GXW64 GXLONG   GXLONG_PTR;
typedef _GXW64 GXULONG  GXULONG_PTR;
typedef _GXW64 GXDWORD  GXDWORD_PTR;

#elif defined(_CL_ARCH_X64) || defined(_CL_ARCH_ARM64)
typedef u64             GXUINT_PTR;
typedef i64             GXINT_PTR;
typedef i64             GXLONG_PTR;
typedef u64             GXULONG_PTR;
typedef u64             GXDWORD_PTR;

//#elif _CL_ARCH_ARM
#else
#error must define instruction platform.
#endif // #ifdef _X86

typedef GXUINT_PTR      GXWPARAM;
typedef GXLONG_PTR      GXLPARAM;
typedef GXLONG_PTR      GXLRESULT;
typedef GXLONG          GXHRESULT;

typedef GXWCHAR         *GXLPWSTR, *LPGXWSTR, *GXLPWCHAR;
typedef GXCHAR          *GXLPSTR, *LPGXSTR, *GXLPCHAR;
typedef const GXWCHAR   *GXLPCWSTR, *LPGXCWSTR;
typedef const GXCHAR    *GXLPCSTR, *LPGXCSTR;
//typedef void            *HGXRGN;
//typedef void            *HGXMENU,*GXHMENU;

#ifdef _UNICODE
typedef GXLPCWSTR GXLPCTSTR;
typedef GXLPWSTR  GXLPTSTR;
typedef GXWCHAR   GXTCHAR;
#else
typedef GXLPCSTR  GXLPCTSTR;
typedef GXLPSTR   GXLPTSTR;
typedef GXCHAR    GXTCHAR;
#endif // _UNICODE

typedef clvector<GXINT>  GXIntArray;
typedef clvector<GXUINT> GXUIntArray;
typedef cllist<GXINT>    GXIntList;
typedef cllist<GXUINT>   GXUIntList;

class GXColor32
{
public:
  union
  {
    GXDWORD color;
    GXDWORD ARGB;
    struct{
      GXBYTE b;
      GXBYTE g;
      GXBYTE r;
      GXBYTE a;
    };
  };
  GXColor32() CLTRIVIAL_DEFAULT;
  GXColor32(int _r, int _g, int _b) : b(_b), g(_g), r(_r), a(255) {}
  GXColor32(int _a, int _r, int _g, int _b) : b(_b), g(_g), r(_r), a(_a) {}
  GXColor32(const GXColor32& clr) : color(clr.color){}
  GXColor32(GXDWORD clr) : color(clr){}

  //operator const GXDWORD() { return color; }
  //operator GXDWORD() { return color; }
  operator const GXCOLORREF() { return color; }
  operator GXCOLORREF() { return color; }
  b32 operator !=(GXDWORD dwVal) const { return color != dwVal; }
  GXCOLORREF operator |(GXCOLORREF clr) const { return color | clr; }
  GXCOLORREF operator &(GXCOLORREF clr) const { return color & clr; }
};

//class GXColor
//{
//
//};

typedef clstd::COLOR_RGBA_F GXColor;
//////////////////////////////////////////////////////////////////////////
#endif // _GRAPX_BASE_TYPES_H_