#ifndef _GRAPX_UTILITY_H_
#define _GRAPX_UTILITY_H_

struct GVRENDERDESC;
namespace clstd
{
  class Image;
} // namespace clstd

namespace RepoUtility
{
  GXHRESULT GXDLL SavePrimitive(SmartRepository* pStorage, GXLPCSTR szDomain, GPrimitiveVI* pPrimitive, int nStartIndex, int nNumPrimi);
  GXHRESULT GXDLL LoadPrimitive(SmartRepository* pStorage, GXLPCSTR szDomain, GXVERTEXELEMENT* pVertElement, clBuffer* pVertices, clBuffer* pIndices, size_t& nStartIndex, size_t& nNumPrimi);
}; // namespace RepoUtility

namespace ObjMeshUtility
{
  typedef GXVERTEX_P3T2N3F PrimaryVertex;
  typedef clvector<PrimaryVertex> PrimaryVerticesArray;
  struct PRIMARYMESH
  {
    clStringA     Name;
    PrimaryVerticesArray  aVertices;
    IndexedTrianglesArray aFaces;
  };

  typedef clvector<PRIMARYMESH> PrimaryMeshsArray;
  GXBOOL GXDLL LoadFromFileA(GXLPCSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
  GXBOOL GXDLL LoadFromFileW(GXLPCWSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
  GXBOOL GXDLL LoadFromMemory(const clBufferBase* pBuffer, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
  GXBOOL GXDLL SavePrimitive(clBuffer* pBuffer, GXLPCSTR szName, GVRENDERDESC* pDesc, GXINOUT u32& nVertBase);
} // namespace ObjMeshUtility

namespace PrimitiveUtility
{
  // TODO: 将要实现
  // ConvertToplogyType();
  //ResolveToMeshData(GVMESHDATA*);

  //************************************
  // 设置Primitive中的Diffuse值，如果存在的话。
  // Method:    SetDiffuse
  // FullName:  PrimitiveUtility::SetDiffuse
  // Returns:   GXBOOL
  // Qualifier:
  // Parameter: GPrimitive* pPrimitive
  // Parameter: GXColor32 crDiffuse
  // Parameter: GXBOOL bManualUpdate 是否手动调用UpdateResource
  //************************************
  GXBOOL GXDLL SetUnifiedDiffuse(GPrimitive* pPrimitive, GXColor32 crDiffuse, GXBOOL bManualUpdate);
}

namespace PrimitiveIndicesUtility
{
  // 四边形顶点阵列索引填充, clvector必须提前调用reserve预先分配内存!
  // xSeg, ySeg是分段数，如果网格是m*n个顶点的话，xSeg = m - 1, ySeg = n - 1
  void GXDLL FillIndicesAsQuadVertexArrayCW16 (int xSeg, int ySeg, int nBaseIndex, clvector<u32>& aIndices, int nPitch = 0);
  void GXDLL FillIndicesAsQuadVertexArrayCW32 (int xSeg, int ySeg, int nBaseIndex, clvector<u16>& aIndices, int nPitch = 0);
  void GXDLL FillIndicesAsQuadVertexArrayCCW16(int xSeg, int ySeg, int nBaseIndex, clvector<u16>& aIndices, int nPitch = 0);
  void GXDLL FillIndicesAsQuadVertexArrayCCW32(int xSeg, int ySeg, int nBaseIndex, clvector<u32>& aIndices, int nPitch = 0);

  // 循环顶点阵列索引填充
  void GXDLL FillIndicesAsCycleVertexArray(int nHeightSeg, int nSides, int nBaseIndex, clvector<GXWORD>& aIndices);

} // namespace PrimitiveIndicesUtility

namespace TextureUtility
{
  GXBOOL GXDLL TextureFormatToClstdImageFormat(char* fmt, int* nDepth, GXFormat eFmt); // fmt至少应该4个字节
  GXDLL clstd::Image* CreateImage(GTexture* pTexture);
} // namespace TextureUtility

#endif // #ifndef _GRAPX_UTILITY_H_