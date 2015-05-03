// GrapVR Module Geometry
#ifndef _GRAPVR_SCENE_MODULE_GEOMETRY_H_
#define _GRAPVR_SCENE_MODULE_GEOMETRY_H_

class GVScene;
class GPrimitiveVI;

class GXDLL GVGeometry : public GVMesh
{
public:
  enum GEOTYPE
  {
    GT_UNDEFINED,
    GT_AABB,
    GT_BOX,
    GT_AXIS,
    GT_AXIS_MESH,
    GT_QUADPLANE,
    GT_MESH,
    GT_TORUS,
    GT_CONE,
    GT_OBJMODEL,
  };
protected:
  GXPrimitiveType   m_eType;
  GEOTYPE           m_eGeoType;
protected:
  GXBOOL InitializeAsAABB(GXGraphics* pGraphics, GXCOLOR clr);
  GXBOOL InitializeAsAxis(GXGraphics* pGraphics, const float3& vPos, float fExtent, int nLevel);
  GXBOOL InitializeAsQuadPlane(GXGraphics* pGraphics, const float3& vPos, const float3& vDirection, const float3& vUp, const float2& vExtent, GXUINT xSeg, GXUINT ySeg, GXDWORD dwVertexFlags);
  GXBOOL CreatePrimitive(GXGraphics* pGraphics, GXPrimitiveType eType, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, GXWORD* pIndices, int nIdxCount);
  GXBOOL SetPrimitive(GXPrimitiveType eType, int nPrimCount, int nStartIndex, GPrimitiveVI* pPrimitive);
public:
  GVGeometry();
  GVGeometry(GXGraphics* pGraphics, GEOTYPE eType);
  GVGeometry(GXGraphics* pGraphics, GEOTYPE eType, const float3& vMin, const float3& vMax);
  virtual ~GVGeometry();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual void    GetRenderDesc   (GVRenderType eType, GVRENDERDESC* pRenderDesc);
  virtual GXBOOL  RayTrace       (const Ray& ray, NODERAYTRACE* pRayTrace);

  //GXHRESULT SetMaterialInst(GXMaterialInst* pMtlInst, GXDWORD dwFlags);

  static GXHRESULT CreateBox            (GXGraphics* pGraphics, CFloat3& vCenter, CFloat3& vExtent, GXCOLOR clr, GVGeometry** ppGeometry, GXDWORD dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));
  static GXHRESULT CreateAABB           (GXGraphics* pGraphics, const float3& vMin, const float3& vMax, GXCOLOR clr, GVGeometry** ppGeometry);
  static GXHRESULT CreateAxis           (GXGraphics* pGraphics, const float3& vPos, float fExtent, int nLevel, GVGeometry** ppGeometry);
  static GXHRESULT CreateQuadPlane      (
    GXGraphics*   pGraphics, 
    const float3& vPos, 
    const float3& vDirection, 
    const float3& vUp, 
    const float2& vExtent, 
    GXUINT        xSeg, 
    GXUINT        ySeg, 
    GVGeometry**  ppGeometry,
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateCylinder(
    GXGraphics*   pGraphics, 
    float         fOffset, 
    float         fRadius, 
    float         fHeight, 
    int           nHeightSeg, 
    int           nCapSeg, 
    int           nSides, 
    GXBOOL        bSmooth, 
    GXColor32     color, 
    GVGeometry**  ppGeometry, 
    float4x4*     pTransform = NULL, 
    GXDWORD       dwTBS = 0x7,
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateCylinder(
    GXGraphics*   pGraphics, 
    float         fOffset, 
    float         fRadius, 
    float         fHeight, 
    int           nHeightSeg, 
    int           nCapSeg, 
    int           nSides, 
    GXBOOL        bSmooth, 
    GXColor32     color, 
    const float3& vDir,     // ��Ҫ��һ��
    GVGeometry**  ppGeometry, 
    GXDWORD       dwTBS = 0x7,
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateCone(
    GXGraphics*   pGraphics, 
    float         fOffset, 
    float         fRadius1, 
    float         fRadius2, 
    float         fHeight, 
    int           nHeightSeg, 
    int           nCapSeg, 
    int           nSides, 
    GXBOOL        bSmooth, 
    GXColor32     color, 
    GVGeometry**  ppGeometry, 
    float4x4*     pTransform = NULL, 
    GXDWORD       dwTBS = 0x7,  // TODO: ������DWORDӦ�ú�Ϊһ��Flags
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateTorus(
    GXGraphics*   pGraphics,
    float         fRadius1,   // ����뾶, Բ���൱��һ��Ť���Բ����
    float         fRadius2,   // ����뾶,�������⾶��ֵ��һ��
    int           nSegment,   // �ػ���Ķ���
    int           nSides,     // ���������
    GXColor32     color, 
    GVGeometry**  ppGeometry,
    float4x4*     pTransform = NULL,
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateSphere(
    GXGraphics*   pGraphics,
    float         fOffset,
    float         fRadius,
    int           nSegments,
    int           nSides,
    float         fHemisphere,
    GXColor32     color, 
    GVGeometry**  ppGeometry,
    float4x4*     pTransform = NULL,
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateCapsule(
    GXGraphics*   pGraphics,
    float         fOffset,
    float         fRadius,
    float         fHeight,
    int           nSides,
    int           nHeightSeg,
    int           nParallelSeg, // ���ά�ȷֶ�, ����ֶ����������������ģ�����10����ÿ�������Լ���ֳ�5��
    GXBOOL        bSmooth, 
    GXColor32     color, 
    GVGeometry**  ppGeometry, 
    float4x4*     pTransform = NULL, 
    GXDWORD       dwTBS = 0x7,  // TODO: ������DWORDӦ�ú�Ϊһ��Flags
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateConvex(
    GXGraphics*   pGraphics,
    Plane*        pPlanes,
    int           nNumPlanes,
    GXColor32     color, 
    GVGeometry**  ppGeometry, 
    float4x4*     pTransform = NULL, 
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));
};

class GXDLL GVGeometryBox : public GVGeometry
{
protected:
public:
  static GXHRESULT Create(GXGraphics* pGraphics, CFloat3& vCenter, CFloat3& vExtent, GXCOLOR clr, GVGeometry** ppGeometry, GXDWORD dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));
};

#endif // _GRAPVR_SCENE_MODULE_GEOMETRY_H_
