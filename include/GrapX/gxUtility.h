#ifndef _GRAPX_UTILITY_H_
#define _GRAPX_UTILITY_H_

struct GVRENDERDESC;
struct GVMESHDATA;
namespace clstd
{
  class Image;
  class MemBuffer;
} // namespace clstd

namespace GrapX
{

  namespace RepoUtility
  {
    GXHRESULT GXDLL SavePrimitive(clSmartRepository* pStorage, GXLPCSTR szDomain, GrapX::Primitive* pPrimitive, int nStartIndex, int nNumPrimi);
    GXHRESULT GXDLL LoadPrimitive(clSmartRepository* pStorage, GXLPCSTR szDomain, GXVERTEXELEMENT* pVertElement, clBuffer* pVertices, clBuffer* pIndices, GXSIZE_T& nStartIndex, GXSIZE_T& nNumPrimi);
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

    //typedef clvector<PRIMARYMESH> PrimaryMeshsArray;
    class GXDLL PrimaryMeshsArray
    {
    protected:
      typedef clvector<PRIMARYMESH> PrimaryMeshsArray_t;
      PrimaryMeshsArray_t* m_pMeshs;

    public:
      typedef PrimaryMeshsArray_t::iterator iterator;

      //static PrimaryMeshsArray* Create();
      //static void Destroy(PrimaryMeshsArray* pMesh);
      PrimaryMeshsArray();
      virtual ~PrimaryMeshsArray();

      void push_back(const PRIMARYMESH& m);
      GXBOOL empty() const;
      size_t size() const;
      iterator begin() const;
      iterator end() const;

      PRIMARYMESH& operator[](int index);
      const PRIMARYMESH& operator[](int index) const;
    };
    GXBOOL GXDLL LoadFromFileA(GXLPCSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
    GXBOOL GXDLL LoadFromFileW(GXLPCWSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
    GXBOOL GXDLL LoadFromMemory(const clBufferBase* pBuffer, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs);
    GXBOOL GXDLL SavePrimitive(clBuffer* pBuffer, GXLPCSTR szName, GVRENDERDESC* pDesc, GXINOUT u32& nVertBase);
  } // namespace ObjMeshUtility

  namespace PrimitiveUtility
  {
    // TODO: 将要实现
    // ConvertToplogyType();
    // 不处理索引数据
    GXBOOL GXDLL ConvertMeshDataToStream(GXOUT clstd::MemBuffer* pBuffer, GXOUT GXVERTEXELEMENT* pVertElement, const GVMESHDATA* pMeshComponent); // GXVERTEXELEMENT 要预留足够大，建议64个
    GXBOOL GXDLL ConvertStreamToMeshData(GXOUT clstd::MemBuffer* pBuffer, GXOUT GVMESHDATA* pMeshComponent, GXLPCVOID pStream, GXUINT nVertexCount, const GXVERTEXELEMENT* pVertElement);

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
    GXBOOL GXDLL SetUnifiedDiffuse(GrapX::Primitive* pPrimitive, GXColor32 crDiffuse, GXBOOL bManualUpdate);
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
    GXDLL clstd::Image* CreateImage(Texture* pTexture);
  } // namespace TextureUtility
} // namespace GrapX

#endif // #ifndef _GRAPX_UTILITY_H_