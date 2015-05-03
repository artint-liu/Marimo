#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPHICS_X_PRIMITIVE_H_
#define _GRAPHICS_X_PRIMITIVE_H_

// ��������
class GXGraphics;

namespace D3D9
{
  class GXGraphicsImpl;
  class GVertexDeclImpl;

  class GPrimImpl
  {
  protected:
    GXGraphicsImpl*         m_pGraphicsImpl;
    IDirect3DVertexBuffer9* m_pVertexBuffer;
    GXUINT                  m_uElementSize;
    GXBYTE*                 m_pVertices;  // �ڴ��еı��� GXRU_FREQXXX û�����ֵ

    GVertexDeclImpl*        m_pVertexDecl;
    GXDWORD                 m_dwResUsage;

    // ��ǰ��,׷�������������������
    GXUINT                  m_uElementCount;
    GXLPVOID                m_pLockedVertex;
  protected:
    GPrimImpl(GXGraphics* pGraphics);
    GXBOOL    CreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl);
    GXBOOL    RestoreVertices   ();
  public:
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

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        ();
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);


    GXBOOL  InitPrimitive(GXLPCVOID pVertInitData, GXUINT uElementCount, GXUINT uElementSize, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage);

    // ȷ���Ƿ������LostDeviceʱ����, Ĭ��ΪTRUE
    GXBOOL  EnableDiscard(GXBOOL bDiscard);
    GXBOOL  IsDiscardable();

    GXLPVOID      Lock      (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE));
    GXBOOL        Unlock    ();

    virtual GXLPVOID    GetVerticesBuffer     ();
    virtual GXUINT      GetVerticesCount      ();
    virtual GXUINT      GetVertexStride       ();
    virtual GXBOOL      UpdateResouce         (ResEnum eRes);
    virtual GXHRESULT   GetVertexDeclaration  (GVertexDeclaration** ppDeclaration);
    virtual GXGraphics* GetGraphicsUnsafe     ();
    virtual GXINT       GetElementOffset      (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc);
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

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        ();
    virtual GXHRESULT Release       ();
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
    virtual GXUINT    GetIndexCount  ();

    virtual GXLPVOID    GetVerticesBuffer     ();
    virtual GXUINT      GetVerticesCount      ();
    virtual GXUINT      GetVertexStride       ();
    virtual GXLPVOID    GetIndicesBuffer      ();
    virtual GXBOOL      UpdateResouce         (ResEnum eRes);
    virtual GXHRESULT   GetVertexDeclaration  (GVertexDeclaration** ppDeclaration);
    virtual GXGraphics* GetGraphicsUnsafe     ();
    virtual GXINT       GetElementOffset      (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc);
  protected:
    GXBOOL    RestoreIndices   ();
  private:
    IDirect3DIndexBuffer9*    m_pIndexBuffer;

    // GXRU_FREQXXX ����
    GXBYTE*                   m_pIndices;   // �ڴ��б������������

    GXUINT                    m_uIndexCount;

    GXWORD*                   m_pLockedIndex;
  };
} // namespace D3D9

#endif // _GRAPHICS_X_PRIMITIVE_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)