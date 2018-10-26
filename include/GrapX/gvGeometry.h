// GrapVR Module Geometry
#ifndef _GRAPVR_SCENE_MODULE_GEOMETRY_H_
#define _GRAPVR_SCENE_MODULE_GEOMETRY_H_

class GVScene;
//class GPrimitiveVI;

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
  GXBOOL SetPrimitive(GXPrimitiveType eType, int nPrimCount, int nStartIndex, GPrimitive* pPrimitive);
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
    const float3& vDir,     // 需要归一化
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
    GXDWORD       dwTBS = 0x7,  // TODO: 这两个DWORD应该合为一个Flags
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateTorus(
    GXGraphics*   pGraphics,
    float         fRadius1,   // 总体半径, 圆环相当于一个扭弯的圆柱体
    float         fRadius2,   // 环体半径,就是内外径差值的一半
    int           nSegment,   // 沿环体的段数
    int           nSides,     // 环体的面数
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
    int           nParallelSeg, // 球的维度分段, 这个分段是相对于完整球体的，例如10，则每个半球大约被分成5段
    GXBOOL        bSmooth, 
    GXColor32     color, 
    GVGeometry**  ppGeometry, 
    float4x4*     pTransform = NULL, 
    GXDWORD       dwTBS = 0x7,  // TODO: 这两个DWORD应该合为一个Flags
    GXDWORD       dwVertexFlags = (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD));

  static GXHRESULT CreateConvex(
    GXGraphics*   pGraphics,
    GVNode::Plane*pPlanes,
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
