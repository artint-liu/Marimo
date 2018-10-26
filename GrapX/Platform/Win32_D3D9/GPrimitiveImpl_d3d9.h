#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPHICS_X_PRIMITIVE_H_
#define _GRAPHICS_X_PRIMITIVE_H_

// 类型声明
class GXGraphics;

namespace D3D9
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
    GXGraphicsImpl*         m_pGraphicsImpl;
    IDirect3DVertexBuffer9* m_pD3D9VertexBuffer;
    GXResUsage              m_eResUsage;
    
    const GXUINT            m_uVertexCount;
    GXUINT                  m_uVertexSize;
    GXLPVOID                m_pLockedVertex;
    GXBYTE*                 m_pVertexBuffer;

    GVertexDeclImpl*        m_pVertexDecl;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT AddRef        () override;
    GXHRESULT Release       () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE


  protected:
    GPrimitiveVertexOnlyImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride);
    virtual ~GPrimitiveVertexOnlyImpl();

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) override;

    GXBOOL    CreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl);
    GXBOOL    RestoreVertices   ();

    GXBOOL  InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData);

    // 确定是否可以在LostDevice时丢弃, 默认为TRUE
    //GXBOOL  EnableDiscard(GXBOOL bDiscard);
    //GXBOOL  IsDiscardable();



    GXUINT      GetVertexCount        () override;
    GXUINT      GetVertexStride       () override;
    GXLPVOID    MapVertexBuffer       (GXResMap eMap) override;
    GXBOOL      UnmapVertexBuffer     (GXLPVOID lpMappedBuffer) override;

    GXUINT      GetIndexCount         () override;
    GXUINT      GetIndexStride        () override;
    GXLPVOID    MapIndexBuffer        (GXResMap eMap) override;
    GXBOOL      UnmapIndexBuffer      (GXLPVOID lpMappedBuffer) override;

    GXHRESULT   GetVertexDeclaration  (GVertexDeclaration** ppDeclaration) override;
    GXGraphics* GetGraphicsUnsafe     () override;
    GXINT       GetElementOffset      (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;
  };

  //
  // Primitive - Vertex & Index
  //
  class GPrimitiveVertexIndexImpl : public GPrimitiveVertexOnlyImpl
  {
    friend class GXGraphicsImpl;

  protected:
    IDirect3DIndexBuffer9*    m_pD3D9IndexBuffer;
    const GXUINT              m_uIndexCount;
    const GXUINT              m_uIndexSize;
    GXLPVOID                  m_pLockedIndex;
    GXBYTE*                   m_pIndexBuffer;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        ();
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  protected:
    GPrimitiveVertexIndexImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride, GXUINT nIndexCount, GXUINT nIndexStride);
    virtual ~GPrimitiveVertexIndexImpl();


    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);

    GXBOOL  InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData, GXLPCVOID pIndexInitData);


    GXUINT      GetIndexCount         () override;
    GXUINT      GetIndexStride        () override;
    GXLPVOID    MapIndexBuffer        (GXResMap eMap) override;
    GXBOOL      UnmapIndexBuffer      (GXLPVOID lpMappedBuffer) override;


    GXGraphics* GetGraphicsUnsafe     () override;
    GXINT       GetElementOffset      (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;
  protected:
    GXBOOL      RestoreIndices        ();

  };
} // namespace D3D9

#endif // _GRAPHICS_X_PRIMITIVE_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)