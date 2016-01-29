// GrapVR
#ifndef _GRAPVR_SCENE_NODE_MESH_H_
#define _GRAPVR_SCENE_NODE_MESH_H_

class GVScene;
class GPrimitiveVI;

struct GVMESHDATA
{
  GXSIZE_T   nVertexCount;     // 顶点数量, 除索引列表外, 下面的元素必须都与这个数量一致
  GXSIZE_T   nIndexCount;      // 索引数量
  float3*     pVertices;        // 顶点, 这个必须有
  float3*     pNormals;         // 法线, 可选
  float3*     pTangents;        // 切线, 可选
  float3*     pBinormals;       // 副法线, 可选
  float2*     pTexcoord0;       // 第一层UV, 可选
  float2*     pTexcoord1;       // 第二层UV, 可选
  float3*     pBoneWeights;     // 骨骼权重, float3和DWORD任选其一, 内部将自动转换
  GXDWORD*    pBoneWeights32;   // 参考pBoneWeights, 同时存在时以此为准
  int*        pBoneIndices;     // 骨骼索引
  GXDWORD*    pBoneIndices32;   // 骨骼索引
  GXColor*    pColors;          // 顶点色, 这个不会被转换为pColors32
  GXColor32*  pColors32;        // 顶点色, 如果pColors也不为空,则以pColors32为准
  VIndex*     pIndices;         // 索引列表,需要与索引数量一致

  static void   Destroy   (GVMESHDATA* pMeshData);
  static GXBOOL Check     (const GVMESHDATA* pMeshData);
  static GXSIZE_T Build     (const GVMESHDATA* pMeshData, GXLPVERTEXELEMENT lpVertDelement); // 要给一个足够大的数组,建议64个
};

class GXDLL GVMesh : public GVNode
{
protected:
  GXMaterialInst*   m_pMtlInst;
  GPrimitiveVI*     m_pPrimitive;
  GXUINT            m_nPrimiCount;
  GXUINT            m_nVertCount;
  GXUINT            m_nStartIndex;
protected:
  void   Clear();
  GXBOOL InitializeAsObjFromFile(GXGraphics* pGraphics, GXLPCWSTR szFilename, const float4x4* pTransform);
  GXBOOL InitializeAsObjFromMemory(GXGraphics* pGraphics, clBufferBase* pBuffer, const float4x4* pTransform);
  GXBOOL IntCreatePrimitive(GXGraphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount);
  GXBOOL IntSetPrimitive(GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, GPrimitiveVI* pPrimitive);
  GXBOOL IntCreateMesh(GXGraphics* pGraphics, const GVMESHDATA* pMeshComponent);
  GXBOOL IntInitializeAsContainer(GXGraphics* pGraphics, GVNode** pNodesArray, int nNodeCount);
  //GXBOOL CloneMesh(GVMesh* pSource);

  //static GXHRESULT SavePrimitive(SmartRepository* pStorage, GPrimitiveVI* pPrimitive, int nStartIndex, int nNumPrimi);

  virtual GXHRESULT LoadFile (GXGraphics* pGraphics, clSmartRepository* pStorage);
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

  // 文件存取相关的
  virtual GXHRESULT SaveFile (clSmartRepository* pStorage);
  virtual GXHRESULT SetMaterialInstDirect     (GXMaterialInst* pMtlInst);

  virtual GXHRESULT GetMaterialInst         (GXMaterialInst** ppMtlInst);
  //virtual GXHRESULT SetMaterialInst         (GXMaterialInst* pMtlInst, GXDWORD dwFlags);
  virtual GXHRESULT GetMaterialInstFilenameW(clStringW* pstrFilename);
  //virtual GXHRESULT SetMaterialInstFromFileW(GXLPCWSTR szFilename, GXDWORD dwFlags);

  void ApplyTransform(); // 将变换应用到顶点

  static GXHRESULT CreateUserPrimitive    (GXGraphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount, GVMesh** ppMesh);
  static GXHRESULT CreateUserPrimitive    (GXGraphics* pGraphics, GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, GPrimitiveVI* pPrimitive, GVMesh** ppMesh);
  static GXHRESULT CreateMesh             (GXGraphics* pGraphics, const GVMESHDATA* pMeshComponent, GVMesh** ppMesh);
  static GXHRESULT CreateContainer        (GXGraphics* pGraphics, GVNode** pNodesArray, int nNodeCount, GVMesh** ppMesh); // 不增加pNodesArray中Node的引用计数
  //static GXHRESULT Clone                  (GVMesh** pNewMesh, GVMesh* pSourceMesh);
  static GXHRESULT LoadObjFromFileA       (GXGraphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh, const float4x4* pTransform = NULL);
  static GXHRESULT LoadObjFromFileW       (GXGraphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh, const float4x4* pTransform = NULL);
  static GXHRESULT LoadMeshFromFileA      (GXGraphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromFileW      (GXGraphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromRepository (GXGraphics* pGraphics, clSmartRepository* pStorage, GVMesh** ppMesh);
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
  GXBOOL GXDLL ClearVertexElement(GXLPVOID lpFirstElement, GXSIZE_T cbElement, GXUINT nStride, GXSIZE_T nVertCount);
  GXBOOL GXDLL CopyVertexElementFromStream(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID lpSourceStream, GXSIZE_T nVertCount); // lpSourceStream 的大小是 cbElementStride
  GXBOOL GXDLL SetVertexElement(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID pValue, GXSIZE_T nVertCount);
  GXBOOL GXDLL TransformPosition(const float4x4& mat, float3* pVertices, GXUINT nCount, GXUINT nStride = 0);
  GXBOOL GXDLL TransformVectors(const float4x4& mat, float3* pVertors, GXUINT nCount, GXUINT nStride = 0);

  // 翻转pVertices中的顶点顺序，如果pIndices不为NULL也会调整索引
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex* pIndices, GXSIZE_T nFaceCount);
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex32* pIndices, GXSIZE_T nFaceCount);

  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex* pIndices, int nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex32* pIndices, int nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex32>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex32* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);

  GXVOID GXDLL CalculateAABB(AABB& aabb, const float3* pVertices, int nVertCount, GXUINT nVertexStride = NULL);
  GXVOID GXDLL CalculateAABBFromIndices(AABB& aabb, GXLPCVOID pVertices, const VIndex* pIndex, GXSIZE_T nIndexCount, GXUINT nVertexStride = NULL);

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
