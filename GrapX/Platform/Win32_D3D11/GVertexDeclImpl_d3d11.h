#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_
#define _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_

namespace GrapX
{
  namespace D3D11
  {
    class GraphicsImpl;
    class VertexDeclImpl : public GrapX::GVertexDeclaration
    {
      friend class GraphicsImpl;
    private:
      typedef GrapXToDX11::GXD3D11InputElementDescArray DescArray;

      GraphicsImpl*         m_pGraphics;
      clStringA             m_strSketchName;  // ÌØÕ÷Ãû
      DescArray             m_aDescs;
      UINT                  m_NumDescs;
      GXUINT                m_nStride;
      LPGXVERTEXELEMENT     m_pVertexElement;

    protected:
      VertexDeclImpl(GraphicsImpl* pGraphics);
      virtual ~VertexDeclImpl();

    public:
      GXHRESULT Initialize(LPCGXVERTEXELEMENT lpVertexElement, const clStringA& strSketchName);
      GXHRESULT Activate();

      const clStringA& GetSketchName() const;
      const GrapXToDX11::GXD3D11InputElementDescArray& GetVertexLayoutDescArray() const;

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT           AddRef            () override;
      GXHRESULT           Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT           Invoke            (GRESCRIPTDESC* pDesc) override { return GX_OK; }

      GXUINT              GetStride         () override;
      GXINT               GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;
      GXLPCVERTEXELEMENT  GetVertexElement  () override;
    };
  } // namespace D3D11
}

#endif // #ifndef _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
