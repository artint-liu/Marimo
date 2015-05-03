// GrapVR
#ifndef _GRAPVR_SCENE_NODE_MESH_H_
#define _GRAPVR_SCENE_NODE_MESH_H_

class GVScene;
class GPrimitiveVI;

struct GVMESHDATA
{
  int         nVertexCount;     // ��������, �������б���, �����Ԫ�ر��붼���������һ��
  int         nIndexCount;      // ��������
  float3*     pVertices;        // ����, ���������
  float3*     pNormals;         // ����, ��ѡ
  float3*     pTangents;        // ����, ��ѡ
  float3*     pBinormals;       // ������, ��ѡ
  float2*     pTexcoord0;       // ��һ��UV, ��ѡ
  float2*     pTexcoord1;       // �ڶ���UV, ��ѡ
  float3*     pBoneWeights;     // ����Ȩ��, float3��DWORD��ѡ��һ, �ڲ����Զ�ת��
  GXDWORD*    pBoneWeights32;   // �ο�pBoneWeights, ͬʱ����ʱ�Դ�Ϊ׼
  int*        pBoneIndices;     // ��������
  GXDWORD*    pBoneIndices32;   // ��������
  GXColor*    pColors;          // ����ɫ, ������ᱻת��ΪpColors32
  GXColor32*  pColors32;        // ����ɫ, ���pColorsҲ��Ϊ��,����pColors32Ϊ׼
  VIndex*     pIndices;         // �����б�,��Ҫ����������һ��

  static void   Destroy   (GVMESHDATA* pMeshData);
  static GXBOOL Check     (const GVMESHDATA* pMeshData);
  static GXUINT Build     (const GVMESHDATA* pMeshData, GXLPVERTEXELEMENT lpVertDelement); // Ҫ��һ���㹻�������,����64��
};

class GXDLL GVMesh : public GVNode
{
protected:
  GXMaterialInst*   m_pMtlInst;
  GPrimitiveVI*     m_pPrimitive;
  int               m_nPrimiCount;
  int               m_nVertCount;
  int               m_nStartIndex;
protected:
  void   Clear();
  GXBOOL InitializeAsObjFromFile(GXGraphics* pGraphics, GXLPCWSTR szFilename, const float4x4* pTransform);
  GXBOOL InitializeAsObjFromMemory(GXGraphics* pGraphics, clBufferBase* pBuffer, const float4x4* pTransform);
  GXBOOL IntCreatePrimitive(GXGraphics* pGraphics, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, VIndex* pIndices, int nIdxCount);
  GXBOOL IntSetPrimitive(int nPrimCount, int nStartIndex, GPrimitiveVI* pPrimitive);
  GXBOOL IntCreateMesh(GXGraphics* pGraphics, const GVMESHDATA* pMeshComponent);
  GXBOOL IntInitializeAsContainer(GXGraphics* pGraphics, GVNode** pNodesArray, int nNodeCount);
  //GXBOOL CloneMesh(GVMesh* pSource);

  //static GXHRESULT SavePrimitive(SmartRepository* pStorage, GPrimitiveVI* pPrimitive, int nStartIndex, int nNumPrimi);

  virtual GXHRESULT LoadFile (GXGraphics* pGraphics, SmartRepository* pStorage);
public:
  GVMesh(GXGraphics* pGraphics);
  GVMesh(GXGraphics* pGraphics, GXDWORD dwClassCode);
  //GVMesh(GXGraphics* pGraphics, const float3& vMin, const float3& vMax);
  virtual ~GVMesh();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //virtual GXBOOL  Update          (const GVSCENEUPDATE& sContext);
  virtual void    GetRenderDesc   (GVRenderType eType, GVRENDERDESC* pRenderDesc);
  virtual GXVOID  CalculateAABB   ();
  virtual GXBOOL  RayTrace        (const Ray& ray, NODERAYTRACE* pRayTrace);
  //virtual float   RayTrace        (const Ray& ray, float3* pHit);
  virtual GXHRESULT Clone( GVNode** ppClonedNode/*, GXBOOL bRecursive*/ );

  // �ļ���ȡ��ص�
  virtual GXHRESULT SaveFile (SmartRepository* pStorage);
  virtual GXHRESULT SetMaterialInstDirect     (GXMaterialInst* pMtlInst);

  virtual GXHRESULT GetMaterialInst         (GXMaterialInst** ppMtlInst);
  //virtual GXHRESULT SetMaterialInst         (GXMaterialInst* pMtlInst, GXDWORD dwFlags);
  virtual GXHRESULT GetMaterialInstFilenameW(clStringW* pstrFilename);
  //virtual GXHRESULT SetMaterialInstFromFileW(GXLPCWSTR szFilename, GXDWORD dwFlags);

  void ApplyTransform(); // ���任Ӧ�õ�����

  static GXHRESULT CreateUserPrimitive    (GXGraphics* pGraphics, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, VIndex* pIndices, int nIdxCount, GVMesh** ppMesh);
  static GXHRESULT CreateUserPrimitive    (GXGraphics* pGraphics, int nPrimCount, int nStartIndex, GPrimitiveVI* pPrimitive, GVMesh** ppMesh);
  static GXHRESULT CreateMesh             (GXGraphics* pGraphics, const GVMESHDATA* pMeshComponent, GVMesh** ppMesh);
  static GXHRESULT CreateContainer        (GXGraphics* pGraphics, GVNode** pNodesArray, int nNodeCount, GVMesh** ppMesh); // ������pNodesArray��Node�����ü���
  //static GXHRESULT Clone                  (GVMesh** pNewMesh, GVMesh* pSourceMesh);
  static GXHRESULT LoadObjFromFileA       (GXGraphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh, const float4x4* pTransform = NULL);
  static GXHRESULT LoadObjFromFileW       (GXGraphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh, const float4x4* pTransform = NULL);
  static GXHRESULT LoadMeshFromFileA      (GXGraphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromFileW      (GXGraphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromRepository (GXGraphics* pGraphics, SmartRepository* pStorage, GVMesh** ppMesh);
};

namespace mesh
{
  template<class _Ty>
  inline _Ty& ElementFromStream(GXLPVOID lpStream, GXUINT nStreamStride, int nIndex)
  {
    return *(_Ty*)((GXLPBYTE)lpStream + nStreamStride * nIndex);
  }

  template<class _Ty>
  inline const _Ty& ElementFromStream(GXLPCVOID lpStream, GXUINT nStreamStride, int nIndex)
  {
    return *(const _Ty*)((GXLPCBYTE)lpStream + nStreamStride * nIndex);
  }

  GXBOOL GXDLL ValidateIndex(VIndex* pIndices, int nIndexCount, int nVertexCount);
  GXBOOL GXDLL ClearVertexElement(GXLPVOID lpFirstElement, GXUINT cbElement, GXUINT nStride, GXUINT nVertCount);
  GXBOOL GXDLL CopyVertexElementFromStream(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID lpSourceStream, GXUINT nVertCount); // lpSourceStream �Ĵ�С�� cbElementStride
  GXBOOL GXDLL SetVertexElement(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID pValue, GXUINT nVertCount);
  GXBOOL GXDLL TransformPosition(const float4x4& mat, float3* pVertices, GXUINT nCount, GXUINT nStride = 0);
  GXBOOL GXDLL TransformVectors(const float4x4& mat, float3* pVertors, GXUINT nCount, GXUINT nStride = 0);

  // ��תpVertices�еĶ���˳�����pIndices��ΪNULLҲ���������
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXUINT nBegin, GXUINT nCount, VIndex* pIndices, GXUINT nFaceCount);
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXUINT nBegin, GXUINT nCount, VIndex32* pIndices, GXUINT nFaceCount);

  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex* pIndices, int nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex32* pIndices, int nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex32>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex32* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);

  GXVOID GXDLL CalculateAABB(AABB& aabb, const float3* pVertices, int nVertCount, GXUINT nVertexStride = NULL);
  GXVOID GXDLL CalculateAABBFromIndices(AABB& aabb, GXLPCVOID pVertices, const VIndex* pIndex, int nIndexCount, GXUINT nVertexStride = NULL);

  GXBOOL CalculateTBs(
    float4*       pTangents, 
    float4*       pBinormals, 
    const float3* pVertices, 
    const float3* pNormals, 
    const float2* pTexcoord, 
    int           nVertCount, 
    const VIndex* pIndeices,
    int           nFaceCount,
    GXUINT        nTangentStride = NULL,
    GXUINT        nBinormalStride = NULL,
    GXUINT        nVertexStride = NULL,
    GXUINT        nNormalStride = NULL,
    GXUINT        nTexcoordStride = NULL);
}

#endif // #ifndef _GRAPVR_SCENE_NODE_MESH_H_