#ifndef _GRAPHICS_X_VERTEX_DECL_H_
#define _GRAPHICS_X_VERTEX_DECL_H_

struct GXVERTEX_P3F_C1D
{
  union {
    struct {
      float3    pos;
      GXColor32 color;
    };
    struct {
      float   x, y, z;      // 位置
      GXBYTE  b, g, r, a;   // 颜色
    };
  };

  GXVERTEX_P3F_C1D() CLTRIVIAL_DEFAULT;
};

struct GXVERTEX_P3T2C4F
{
  union{
    struct {
      float3        pos;
      float2        texcoord;
      GXColor       color;
    };
    struct {
      float   x, y, z;      // 位置
      float   u, v;         // 纹理坐标
      float   r, g, b, a;   // 颜色
    };
  };

  GXVERTEX_P3T2C4F() CLTRIVIAL_DEFAULT;
};

struct GXVERTEX_P4T2C4F
{
  union {
    struct {
      float4  pos;
      float2  texcoord;
      GXColor color;
    };
    struct {
      float   x, y, z, w;   // 位置
      float   u, v;         // 纹理坐标
      float   r, g, b, a;   // 颜色
    };
  };
};

struct GXVERTEX_P3T2F_C1D
{
  union {
    struct {
      float3    pos;
      float2    texcoord;
      GXColor32 color;
    };
    struct {
      float   x, y, z;      // 位置
      float   u, v;         // 纹理坐标
      GXBYTE  b, g, r, a;   // 颜色
    };
  };
};

struct GXVERTEX_P4T2F_C1D
{
  union {
    struct {
      float4    pos;
      float2    texcoord;
      GXColor32 color;
    };
    struct {
      float   x, y, z, w;   // 位置
      float   u, v;         // 纹理坐标
      GXBYTE  b, g, r, a;   // 颜色
    };
  };
  GXVERTEX_P4T2F_C1D() CLTRIVIAL_DEFAULT;

  GXVERTEX_P4T2F_C1D& operator=(const GXVERTEX_P4T2F_C1D& v)
  {
    pos = v.pos;
    texcoord = v.texcoord;
    color = v.color;
    return *this;
  }
};

struct GXVERTEX_P3T2N3F_C1D
{
  union {
    struct {
      float3    pos;
      float2    texcoord;
      float3    normal;
      GXColor32 color;
    };
    struct {
      float   x, y, z;   // 位置
      float   u, v;         // 纹理坐标
      float   nx, ny, nz;
      GXBYTE  b, g, r, a;   // 颜色
    };
  };

  GXVERTEX_P3T2N3F_C1D() CLTRIVIAL_DEFAULT;
  GXVERTEX_P3T2N3F_C1D(const GXVERTEX_P3T2N3F_C1D& v)
    : pos(v.pos)
    , texcoord(v.texcoord)
    , normal(v.normal)
    , color(v.color)
  {
  }


  //GXVERTEX_P3T2N3F_C1D& operator=(const GXVERTEX_P3T2N3F_C1D& v)
  //{
  //  pos = v.pos;
  //  texcoord = v.texcoord;
  //  normal = v.normal;
  //  color = v.color;
  //  return *this;
  //}
};

STATIC_ASSERT(sizeof(GXVERTEX_P3T2N3F_C1D) == 36);

struct GXVERTEX_P3T2N3F
{
  float3  pos;
  float2  texcoord;
  float3  normal;
};

typedef GXVERTEX_P3T2C4F  POS_TEXCOORD_COLOR;
typedef GXVERTEX_P3T2C4F* LPPOS_TEXCOORD_COLOR;
//typedef GXVERTEX_P4T2C4F  POST_TEXCOORD_COLOR;
//typedef GXVERTEX_P4T2C4F* LPPOST_TEXCOORD_COLOR;
typedef GXVERTEX_P4T2F_C1D  CANVAS_PRMI_VERT;
typedef GXVERTEX_P4T2F_C1D* LPCANVAS_PRMI_VERT;
typedef GXWORD              VIndex;
typedef GXUINT              VIndex32;
//extern GXVERTEXELEMENT GXVERTDECL_P3T2C4F[];
//extern GXVERTEXELEMENT GXVERTDECL_P4T2C4F[];

struct INDEXEDTRIANGLE
{
  union {
    struct {
      VIndex a, b, c;
    };
    VIndex m[3];
  };
};

STATIC_ASSERT(sizeof(INDEXEDTRIANGLE) == sizeof(VIndex) * 3);

typedef clvector<VIndex>          IndicesArray;
typedef clvector<VIndex32>        Indices32Array;
typedef clvector<INDEXEDTRIANGLE> IndexedTrianglesArray;

extern "C" void GXDLL MOVIndex16To32(const VIndex* pIndices16, int nIndexCount, VIndex32* pIndices32);
extern "C" int  GXDLL MOVIndex32To16(const VIndex32* pIndices32, int nIndexCount, VIndex* pIndices16);  // 返回值是转换的索引数量,如果与输入不一致,则说明这个位置的索引有问题可能是大于65535

#endif // _GRAPHICS_X_VERTEX_DECL_H_