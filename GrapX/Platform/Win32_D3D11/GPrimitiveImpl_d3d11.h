#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPHICS_X_PRIMITIVE_H_
#define _GRAPHICS_X_PRIMITIVE_H_

// ��������
class GXGraphics;

namespace D3D11
{
  class GXGraphicsImpl;
  class GVertexDeclImpl;

  class GPrimImpl
  {
  protected:
    GXGraphicsImpl*         m_pGraphicsImpl;
    ID3D11Buffer*           m_pD3D11VertexBuffer;
    GXUINT                  m_uElementSize;
    GXBYTE*                 m_pVertices;  // �ڴ��еı��� GXRU_FREQXXX û�����ֵ

    GVertexDeclImpl*        m_pVertexDecl;
    GXDWORD                 m_dwResUsage;

    // ��ǰ��,׷�������������������
    GXUINT                  m_uElementCount;
    //GXLPVOID                m_pLockedVertex;
    D3D11_MAPPED_SUBRESOURCE* m_pVertMappedRes;
  protected:
    GPrimImpl(GXGraphics* pGraphics);
    GXBOOL IntCreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl);
    HRESULT IntCreateBuffer(ID3D11Buffer** ppD3D11Buffer, GXUINT nSize, GXUINT nBindFlags, GXLPCVOID pInitData);

    virtual LPCGXVERTEXELEMENT GetVertexDeclUnsafe();
    //virtual GXUINT    GetVerticesCount  ();
    //virtual GXUINT    GetVertexStride   ();

    //GXBOOL RestoreVertices();
  };

  //
  // Primitive - Vertex 
  //
  class GPrimitiveVImpl : public GPrimitiveV, GPrimImpl
  {
    friend class GXGraphicsImpl;
  protected:
    GPrimitiveVImpl(GXGraphics* pGraphics);
    virtual ~GPrimitiveVImpl();
  public:

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef();
    virtual GXHRESULT   Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT   Invoke          (GRESCRIPTDESC* pDesc);


    GXBOOL  InitPrimitive(GXLPCVOID pVertInitData, GXUINT uElementCount, GXUINT uElementSize, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage);

    // ȷ���Ƿ������LostDeviceʱ����, Ĭ��ΪTRUE
    GXBOOL  EnableDiscard(GXBOOL bDiscard);
    GXBOOL  IsDiscardable();

    GXLPVOID      Lock      (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE));
    GXBOOL        Unlock    ();
    //virtual Type  GetType   ();

    GXLPVOID  GetVerticesBuffer () override;
    GXUINT    GetVerticesCount  () override;
    GXUINT    GetVertexStride   () override;
    GXUINT    GetIndicesCount   () override;

    GXBOOL    UpdateResouce     (ResEnum eRes) override;
    GXHRESULT GetVertexDeclaration(GVertexDeclaration** ppDeclaration) override;
    GXGraphics* GetGraphicsUnsafe () override;
    GXINT       GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;

    //private:
  };

  //
  // Primitive - Vertex & Index
  //
  class GPrimitiveVIImpl : public GPrimitiveVI, GPrimImpl
  {
    friend class GXGraphicsImpl;
  protected:
    GPrimitiveVIImpl(GXGraphics* pGraphics);
    virtual ~GPrimitiveVIImpl();

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef();
    virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);


    GXBOOL  InitPrimitive(GXLPCVOID pVertInitData, GXUINT uVertexCount, GXUINT uVertexSize, GXLPCVOID pIndexInitData, GXUINT uIndexCount, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage);

    // ȷ���Ƿ������LostDeviceʱ����, Ĭ��ΪTRUE
    GXBOOL  EnableDiscard(GXBOOL bDiscard);
    GXBOOL  IsDiscardable();

    GXBOOL        Lock  (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
      GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
      GXLPVOID* ppVertexData, GXWORD** ppIndexData,
      GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE));
    GXBOOL        Unlock  ();
    //virtual Type  GetType ();

    GXLPVOID    GetVerticesBuffer () override;
    GXUINT      GetVerticesCount  () override;
    GXUINT      GetVertexStride   () override;
    GXLPVOID    GetIndicesBuffer  () override;
    GXUINT      GetIndicesCount   () override;

    GXBOOL      UpdateResouce     (ResEnum eRes) override;
    GXHRESULT   GetVertexDeclaration(GVertexDeclaration** ppDeclaration) override;
    GXGraphics* GetGraphicsUnsafe () override;
    GXINT       GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;

  private:
    ID3D11Buffer*             m_pD3D11IndexBuffer;

    // GXRU_FREQXXX ����
    GXBYTE*                   m_pIndices;   // �ڴ��б������������

    GXUINT                    m_uIndexCount;

    //GXWORD*                   m_pLockedIndex;
    D3D11_MAPPED_SUBRESOURCE* m_pIndexMappedRes;
  };
} // namespace D3D11

#endif // _GRAPHICS_X_PRIMITIVE_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11