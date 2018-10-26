#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPHICS_X_PRIMITIVE_H_
#define _GRAPHICS_X_PRIMITIVE_H_

// 类型声明
class GXGraphics;

namespace D3D11
{
  class GXGraphicsImpl;
  class GVertexDeclImpl;


  //
  // Primitive - Vertex 
  //
  class GPrimitiveVertexOnlyImpl : public GPrimitive
  {
    friend class GXGraphicsImpl;
  protected:
    GXGraphicsImpl*           m_pGraphicsImpl;
    ID3D11Buffer*             m_pD3D11VertexBuffer;
    const GXUINT              m_uVertexCount;
    const GXUINT              m_uVertexStride;
    D3D11_MAPPED_SUBRESOURCE  m_sVertexMapped;
    GXLPBYTE                  m_pVertexBuffer;

    GVertexDeclImpl*          m_pVertexDecl;
    GXResUsage                m_eUsage;

  protected:
    GPrimitiveVertexOnlyImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVerteCount, GXUINT nVertextStride);
    virtual ~GPrimitiveVertexOnlyImpl();

    GXLPVOID      IntMapBuffer      (GXResMap eMap, ID3D11Buffer* pD3D11Buffer, D3D11_MAPPED_SUBRESOURCE& rMappedDesc, GXLPBYTE pMemBuffer);
    GXBOOL        IntUnmapBuffer    (GXLPVOID lpMappedBuffer, ID3D11Buffer* pD3D11Buffer, D3D11_MAPPED_SUBRESOURCE& rMappedDesc, GXLPBYTE pMemBuffer);

  public:

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef();
    virtual GXHRESULT   Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXGraphics* GetGraphicsUnsafe () override;

    GXHRESULT   Invoke          (GRESCRIPTDESC* pDesc) override;

    GXBOOL IntCreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl);
    GXBOOL IntCreateBuffer(ID3D11Buffer** ppD3D11Buffer, GXUINT nSize, GXUINT nBindFlags, GXLPCVOID pInitData);

    LPCGXVERTEXELEMENT GetVertexDeclUnsafe();


    GXBOOL  InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData);

    // 确定是否可以在LostDevice时丢弃, 默认为TRUE
    GXBOOL  EnableDiscard(GXBOOL bDiscard);
    GXBOOL  IsDiscardable();

    GXLPVOID      MapVertexBuffer      (GXResMap eMap) override;
    GXBOOL        UnmapVertexBuffer    (GXLPVOID lpMappedBuffer) override;
    //virtual Type  GetType   ();

    //GXLPVOID  GetVerticesBuffer () override;
    GXUINT    GetVertexCount  () override;
    GXUINT    GetVertexStride () override;


    GXUINT    GetIndexCount     () override;
    GXUINT    GetIndexStride    () override;
    GXLPVOID  MapIndexBuffer   (GXResMap eMap) override;
    GXBOOL    UnmapIndexBuffer (GXLPVOID lpMappedBuffer) override;


    //GXBOOL    UpdateResouce     (ResEnum eRes) override;
    GXHRESULT GetVertexDeclaration(GVertexDeclaration** ppDeclaration) override;
    GXINT       GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;

    //private:
  };

  //
  // Primitive - Vertex & Index
  //
  class GPrimitiveVertexIndexImpl : public GPrimitiveVertexOnlyImpl
  {
    friend class GXGraphicsImpl;
  protected:
    GPrimitiveVertexIndexImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride, GXUINT nIndexCount, GXUINT nIndexStride);
    virtual ~GPrimitiveVertexIndexImpl();

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef();
    virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);


    GXBOOL  InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData, GXLPCVOID pIndexInitData);

    // 确定是否可以在LostDevice时丢弃, 默认为TRUE
    GXBOOL  EnableDiscard(GXBOOL bDiscard);
    GXBOOL  IsDiscardable();

    //GXBOOL        Lock  (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
    //  GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
    //  GXLPVOID* ppVertexData, GXWORD** ppIndexData,
    //  GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE));
    //GXBOOL        Unlock  ();
    //virtual Type  GetType ();


    //GXLPVOID    GetVerticesBuffer () override;
    GXUINT    GetVertexCount    () override;
    GXUINT    GetVertexStride   () override;
    //GXLPVOID  MapVertexBuffer      (GXResMap eMap) override;
    //GXBOOL    UnmapVertexBuffer    (GXLPVOID lpMappedBuffer) override;
    //GXLPVOID    GetIndexBuffer    () override;

    GXUINT    GetIndexCount     () override;
    GXUINT    GetIndexStride    () override;
    GXLPVOID  MapIndexBuffer   (GXResMap eMap) override;
    GXBOOL    UnmapIndexBuffer (GXLPVOID lpMappedBuffer) override;


    //GXBOOL      UpdateResouce     (ResEnum eRes) override;
    GXHRESULT   GetVertexDeclaration(GVertexDeclaration** ppDeclaration) override;
    GXGraphics* GetGraphicsUnsafe () override;
    GXINT       GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;

  private:
    ID3D11Buffer*             m_pD3D11IndexBuffer;

    // GXRU_FREQXXX 才有
    //GXBYTE*                   m_pIndices;   // 内存中保存的索引缓冲

    const GXUINT              m_uIndexCount;
    const GXUINT              m_uIndexStride;
    D3D11_MAPPED_SUBRESOURCE  m_sIndexMapped;
    GXLPBYTE                  m_pIndexBuffer;

    //GXWORD*                   m_pLockedIndex;
  };
} // namespace D3D11

#endif // _GRAPHICS_X_PRIMITIVE_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11