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
      float   x, y, z;      // λ��
      GXBYTE  b, g, r, a;   // ��ɫ
    };
  };
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
      float   x, y, z;      // λ��
      float   u, v;         // ��������
      float   r, g, b, a;   // ��ɫ
    };
  };
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
      float   x, y, z, w;   // λ��
      float   u, v;         // ��������
      float   r, g, b, a;   // ��ɫ
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
      float   x, y, z;      // λ��
      float   u, v;         // ��������
      GXBYTE  b, g, r, a;   // ��ɫ
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
      float   x, y, z, w;   // λ��
      float   u, v;         // ��������
      GXBYTE  b, g, r, a;   // ��ɫ
    };
  };
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
      float   x, y, z;   // λ��
      float   u, v;         // ��������
      float   nx, ny, nz;
      GXBYTE  b, g, r, a;   // ��ɫ
    };
  };
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
extern "C" int  GXDLL MOVIndex32To16(const VIndex32* pIndices32, int nIndexCount, VIndex* pIndices16);  // ����ֵ��ת������������,��������벻һ��,��˵�����λ�õ���������������Ǵ���65535

#endif // _GRAPHICS_X_VERTEX_DECL_H_