// GrapVR
#ifndef _GRAPVR_SCENE_NODE_MESH_H_
#define _GRAPVR_SCENE_NODE_MESH_H_

class GVScene;
//class Primitive;
//class GPrimitiveVI;

//enum class

#define MESHDATA_FLAG_UV2_FLOAT   0x00000001
#define MESHDATA_FLAG_UV2_FLOAT2  0x00000000
#define MESHDATA_FLAG_UV2_FLOAT3  0x00000002
#define MESHDATA_FLAG_UV2_FLOAT4  0x00000003
#define MESHDATA_FLAG_UV3_FLOAT   (MESHDATA_FLAG_UV2_FLOAT  << 2)
#define MESHDATA_FLAG_UV3_FLOAT2  (MESHDATA_FLAG_UV2_FLOAT2 << 2)
#define MESHDATA_FLAG_UV3_FLOAT3  (MESHDATA_FLAG_UV2_FLOAT3 << 2)
#define MESHDATA_FLAG_UV3_FLOAT4  (MESHDATA_FLAG_UV2_FLOAT4 << 2)
#define MESHDATA_FLAG_UV4_FLOAT   (MESHDATA_FLAG_UV2_FLOAT  << 4)
#define MESHDATA_FLAG_UV4_FLOAT2  (MESHDATA_FLAG_UV2_FLOAT2 << 4)
#define MESHDATA_FLAG_UV4_FLOAT3  (MESHDATA_FLAG_UV2_FLOAT3 << 4)
#define MESHDATA_FLAG_UV4_FLOAT4  (MESHDATA_FLAG_UV2_FLOAT4 << 4)
#define MESHDATA_FLAG_UV5_FLOAT   (MESHDATA_FLAG_UV2_FLOAT  << 6)
#define MESHDATA_FLAG_UV5_FLOAT2  (MESHDATA_FLAG_UV2_FLOAT2 << 6)
#define MESHDATA_FLAG_UV5_FLOAT3  (MESHDATA_FLAG_UV2_FLOAT3 << 6)
#define MESHDATA_FLAG_UV5_FLOAT4  (MESHDATA_FLAG_UV2_FLOAT4 << 6)
#define MESHDATA_FLAG_UV6_FLOAT   (MESHDATA_FLAG_UV2_FLOAT  << 8)
#define MESHDATA_FLAG_UV6_FLOAT2  (MESHDATA_FLAG_UV2_FLOAT2 << 8)
#define MESHDATA_FLAG_UV6_FLOAT3  (MESHDATA_FLAG_UV2_FLOAT3 << 8)
#define MESHDATA_FLAG_UV6_FLOAT4  (MESHDATA_FLAG_UV2_FLOAT4 << 8)
#define MESHDATA_FLAG_UV7_FLOAT   (MESHDATA_FLAG_UV2_FLOAT  << 10)
#define MESHDATA_FLAG_UV7_FLOAT2  (MESHDATA_FLAG_UV2_FLOAT2 << 10)
#define MESHDATA_FLAG_UV7_FLOAT3  (MESHDATA_FLAG_UV2_FLOAT3 << 10)
#define MESHDATA_FLAG_UV7_FLOAT4  (MESHDATA_FLAG_UV2_FLOAT4 << 10)
#define MESHDATA_FLAG_INDICES_32 0x10000

#define MESHDATA_FLAG_UV2_MASK  0x00000003
#define MESHDATA_FLAG_UV3_MASK  (MESHDATA_FLAG_UV2_MASK << 2)
#define MESHDATA_FLAG_UV4_MASK  (MESHDATA_FLAG_UV2_MASK << 4)
#define MESHDATA_FLAG_UV5_MASK  (MESHDATA_FLAG_UV2_MASK << 6)
#define MESHDATA_FLAG_UV6_MASK  (MESHDATA_FLAG_UV2_MASK << 8)
#define MESHDATA_FLAG_UV7_MASK  (MESHDATA_FLAG_UV2_MASK << 10)

struct GVMESHDATA
{
  GXDWORD     dwFlags;          // MESHDATA_FLAG
  GXSIZE_T    nVertexCount;     // 顶点数量, 除索引列表外, 下面的元素必须都与这个数量一致
  GXSIZE_T    nIndexCount;      // 索引数量
  float3*     pVertices;        // 顶点, 这个必须有
  float3*     pNormals;         // 法线, 可选
  float4*     pTangents;        // 切线, 可选
  float3*     pBinormals;       // 副法线, 可选
  float2*     pTexcoord0;       // 第一层UV, 可选
  float2*     pTexcoord1;       // 第二层UV, 可选
  GXLPCVOID   pTexcoord2;       // 第三层UV, GXLPCVOID类型可以储存float，float2，float3，float4各种类型
  GXLPCVOID   pTexcoord3;
  GXLPCVOID   pTexcoord4;
  GXLPCVOID   pTexcoord5;
  GXLPCVOID   pTexcoord6;
  GXLPCVOID   pTexcoord7;
  float3*     pBoneWeights;     // 骨骼权重, float3和DWORD任选其一, 内部将自动转换
  GXDWORD*    pBoneWeights32;   // 参考pBoneWeights, 同时存在时以此为准
  int*        pBoneIndices;     // 骨骼索引
  GXDWORD*    pBoneIndices32;   // 骨骼索引
  GXColor*    pColors;          // 顶点色, 这个不会被转换为pColors32
  GXColor32*  pColors32;        // 顶点色, 如果pColors也不为空,则以pColors32为准
  union {
    VIndex*     pIndices;         // 索引列表,需要与索引数量一致
    VIndex32*   pIndices32;       // 32位索引列表
  };
  GXResUsage  usage;            // 资源创建方式，编辑器raytrace拾取要使用read方式

  static void     Destroy   (GVMESHDATA* pMeshData);
  static GXBOOL   Check     (const GVMESHDATA* pMeshData);
  static GXSIZE_T Build     (const GVMESHDATA* pMeshData, GXLPVERTEXELEMENT lpVertDelement); // 要给一个足够大的数组,建议64个
};

class GXDLL GVMesh : public GVNode
{
  //typedef clvector<GrapX::ObjectT<GrapX::Material>>  MaterialArray;
protected:
  MaterialArray     m_MtlInsts;
  GrapX::Primitive* m_pPrimitive;
  GXUINT            m_nPrimiCount;
  GXUINT            m_nVertCount;
  GXUINT            m_nStartIndex;

  GVRENDERDESC2     m_Renderer;

protected:
  void   Clear();
  GXBOOL InitializeAsObjFromFile(GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GXResUsage usage, const float4x4* pTransform);
  GXBOOL InitializeAsObjFromMemory(GrapX::Graphics* pGraphics, clBufferBase* pBuffer, GXResUsage usage, const float4x4* pTransform);
  template<typename _VIndexT>
  GXBOOL IntCreatePrimitiveT(GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, _VIndexT* pIndices, GXSIZE_T nIdxCount, GXResUsage usage);
  GXBOOL IntCreatePrimitive(GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount, GXResUsage usage);
  GXBOOL IntCreatePrimitive(GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex32* pIndices, GXSIZE_T nIdxCount, GXResUsage usage);
  GXBOOL IntSetPrimitive(GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, GrapX::Primitive* pPrimitive);
  GXBOOL IntCreateMesh(GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshComponent);
  GXBOOL IntInitializeAsContainer(GrapX::Graphics* pGraphics, GVNode** pNodesArray, int nNodeCount);
  //GXBOOL CloneMesh(GVMesh* pSource);

  //static GXHRESULT SavePrimitive(SmartRepository* pStorage, GPrimitiveVI* pPrimitive, int nStartIndex, int nNumPrimi);

  virtual GXHRESULT LoadFile (GrapX::Graphics* pGraphics, clSmartRepository* pStorage, GXResUsage usage);
public:
  GVMesh(GrapX::Graphics* pGraphics);
  GVMesh(GrapX::Graphics* pGraphics, GXDWORD dwClassCode);
  //GVMesh(GXGraphics* pGraphics, const float3& vMin, const float3& vMax);
  virtual ~GVMesh();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //virtual GXBOOL  Update          (const GVSCENEUPDATE& sContext);
  virtual void    GetRenderDesc   (int nRenderCate, GVRENDERDESC* pRenderDesc);
  GVRENDERDESC2*  GetRenderDesc         (int nRenderCate) override;
  virtual GXVOID  CalculateAABB   ();
  virtual GXBOOL  RayTrace        (const Ray& ray, NODERAYTRACE* pRayTrace);
  //virtual float   RayTrace        (const Ray& ray, float3* pHit);
  virtual GXHRESULT Clone( GVNode** ppClonedNode/*, GXBOOL bRecursive*/ );

  // 文件存取相关的
  virtual GXHRESULT SaveFile (clSmartRepository* pStorage);
  virtual GXBOOL SetMaterial    (GrapX::Material* pMtlInst, int nRenderCate) override;

  virtual GXBOOL GetMaterial          (int nRenderCate, GrapX::Material** ppMtlInst) override;
  //virtual GXHRESULT SetMaterialInst         (GXMaterialInst* pMtlInst, GXDWORD dwFlags);
  virtual GXBOOL GetMaterialFilename(int nRenderCate, clStringW* pstrFilename) override;
  //virtual GXHRESULT SetMaterialInstFromFileW(GXLPCWSTR szFilename, GXDWORD dwFlags);

  void ApplyTransform(); // 将变换应用到顶点

  static GXHRESULT CreateUserPrimitive    (GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount, GXResUsage usage, GVMesh** ppMesh);
  static GXHRESULT CreateUserPrimitive    (GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, GrapX::Primitive* pPrimitive, GVMesh** ppMesh);
  static GXHRESULT CreateMesh             (GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshComponent, GVMesh** ppMesh);
  static GXHRESULT CreateContainer        (GrapX::Graphics* pGraphics, GVNode** pNodesArray, int nNodeCount, GVMesh** ppMesh); // 不增加pNodesArray中Node的引用计数
  //static GXHRESULT Clone                  (GVMesh** pNewMesh, GVMesh* pSourceMesh);
  static GXHRESULT LoadObjFromFileA       (GrapX::Graphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh, GXResUsage usage = GXResUsage::Default, const float4x4* pTransform = NULL);
  static GXHRESULT LoadObjFromFileW       (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh, GXResUsage usage = GXResUsage::Default, const float4x4* pTransform = NULL);
  static GXHRESULT LoadMeshFromFileA      (GrapX::Graphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromFileW      (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh);
  static GXHRESULT LoadMeshFromRepository (GrapX::Graphics* pGraphics, clSmartRepository* pStorage, GXResUsage usage, GVMesh** ppMesh);
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
  GXBOOL GXDLL CopyVertexElementFromStream(GXLPVOID lpDestStream, GXUINT nDestStride, GXLPCVOID lpSrcStream, GXUINT nSrcStride, GXUINT cbElementStride, GXSIZE_T nVertCount);
  GXBOOL GXDLL SetVertexElement(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID pValue, GXSIZE_T nVertCount);
  GXBOOL GXDLL TransformPosition(const float4x4& mat, float3* pVertices, GXUINT nCount, GXUINT nStride = 0);
  GXBOOL GXDLL TransformVectors(const float4x4& mat, float3* pVertors, GXUINT nCount, GXUINT nStride = 0);

  // 翻转pVertices中的顶点顺序，如果pIndices不为NULL也会调整索引
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex* pIndices, GXSIZE_T nFaceCount);
  GXBOOL GXDLL ReverseVerticesArray(float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex32* pIndices, GXSIZE_T nFaceCount);

  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex* pIndices, GXSIZE_T nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex32* pIndices, GXSIZE_T nFaceCount);
  GXBOOL GXDLL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex32>& aIndices);
  GXBOOL GXDLL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, const VIndex32* pIndices, int nFaceCount, GXUINT nNormalStride = NULL, GXUINT nVertexStride = NULL);

  GXVOID GXDLL CalculateAABB(GVNode::AABB& aabb, const float3* pVertices, int nVertCount, GXUINT nVertexStride = NULL);
  GXVOID GXDLL CalculateAABBFromIndices(GVNode::AABB& aabb, GXLPCVOID pVertices, const VIndex* pIndex, GXSIZE_T nIndexCount, GXUINT nVertexStride = NULL);

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
