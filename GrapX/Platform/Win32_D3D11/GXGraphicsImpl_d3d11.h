#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GX_GRAPHICS_H_
#define _GX_GRAPHICS_H_

#define MAX_TEXTURE_STAGE  16
//////////////////////////////////////////////////////////////////////////
//
//GXGraphics
//{
//  GStateBlock_Old
//  管理/创建 GTexture
//  创建 GXImage
//  {
//    引用 GTexture
//  }
//  创建 GXCanvas
//}
class GXCanvasCore;
class GXEffectImpl;
class GTexture;
class GXConsole;
class ILogger;
class GXRenderTargetImpl;

namespace Marimo {
  class ShaderConstName;
} // namespace Marimo

namespace D3D11
{
  class GTextureImpl;
  class GXCanvasCoreImpl;
  class GVertexDeclImpl;
  //class GRenderStateImpl;
  class GRasterizerStateImpl;
  class GBlendStateImpl;
  class GDepthStencilStateImpl;
  class GSamplerStateImpl;

  template<class InterfaceT> class GTexureBaseImplT;
  typedef GTexureBaseImplT<GTextureBase> GTexBaseImpl;

  struct GRAPHICS_CREATION_DESC
  {
    HWND          hWnd;
    GXBOOL        bWaitForVSync;
    GXLPCWSTR     szRootDir;
    ILogger*      pLogger;
  };

  class GXGraphicsImpl : public GrapX::GraphicsBaseImpl
  {
    //friend class GPrimImpl;
    friend class GPrimitiveVertexOnlyImpl;
    friend class GPrimitiveVertexIndexImpl;
    friend class GVertexDeclImpl;
    //friend class GRenderState;
    friend class GSamplerState;
    friend class GXCanvasImpl;
    friend class GXCanvas3DImpl;
    friend class GShaderImpl;
  public:
    enum
    {
      F_CREATEDEVICE  = 0x00000001,  // 对象自己创建的设备
      F_DEVICEHASLOST = 0x00000002,  // 设备已经丢失
      F_ACTIVATE      = 0x00000004,  
      F_SHOWCONSOLE   = 0x00000008,  
    };
    typedef GrapXInternal::GXResourceMgr  GXResourceMgr;
  public:
    static GXGraphics* Create(const GRAPHICS_CREATION_DESC* pDesc);

    // 初始化成员
    virtual HRESULT Initialize(const GRAPHICS_CREATION_DESC* pDesc);

#include "Platform/CommonInline/GXGraphicsImpl_ClassDecl.inl"
    //GXHRESULT CreateOffscreenPlainSurface      (GTexture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage);

  public:
    GXEffect*                   IntGetEffect        ();
    void                        IntGetDimension     (GXUINT& nWidth, GXUINT& nHeight);  // 
    inline ID3D11Device*        D3DGetDevice        ();
    inline ID3D11DeviceContext* D3DGetDeviceContext ();
    GXBOOL                      D3DGetSwapChainDesc  (DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
    inline clstd::Locker*       GetLocker           ();
  private:
    GXHRESULT         Test              ();

    inline void       InlUpdateTopology     (GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount);

    //GXBOOL      IntCheckSizeOfTargetAndDepth();
    GXVOID      BuildInputLayout();

  private:
    const GXUINT            s_uCanvasCacheCount;
    HWND                    m_hWnd;
    D3D_DRIVER_TYPE         m_driverType;
    D3D_FEATURE_LEVEL       m_featureLevel;
    ID3D11Device*           m_pd3dDevice;
    ID3D11DeviceContext*    m_pImmediateContext;
    IDXGISwapChain*         m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11Texture2D*        m_pDepthStencil;
    ID3D11DepthStencilView* m_pDepthStencilView;
    DXGI_SWAP_CHAIN_DESC    m_SwapChainDesc;

    GXRenderTarget*         m_pTempBuffer;
    GXRenderTarget*         m_pDefaultBackBuffer;
    //GTexture*               m_pBackBufferTex;    // 内部使用的纹理缓冲
    //GXImage*                m_pBackBufferImg;    // 引用 m_pBackBufferTex
    //GShader*                m_pSimpleShader;
    GShader*                m_pBaseShader;
    GXEffect*               m_pBaseEffect;

    // State define - 状态定义
    D3D_PRIMITIVE_TOPOLOGY  m_eCurTopology;

    // State Object - 状态对象
    ID3D11RenderTargetView* m_pCurRenderTargetView;
    ID3D11DepthStencilView* m_pCurDepthStencilView;
    ID3D11InputLayout*      m_pVertexLayout;
    GTextureImpl*           m_pDeviceOriginTex;
    GXDWORD                 m_dwBackBufferStencil;  // m_pDeviceOriginTex 使用的模板 [1, 255]

    GXINT                   m_nGraphicsCount;
    //clstd::Locker*          m_pGraphicsLocker;
    DWORD                   m_dwThreadId; // 用来检测Begin()和End()是否调用自同一个线程
  };

  inline ID3D11Device* GXGraphicsImpl::D3DGetDevice()
  {
    return m_pd3dDevice; 
  }
  inline ID3D11DeviceContext* GXGraphicsImpl::D3DGetDeviceContext()
  {
    return m_pImmediateContext;
  }
  inline clstd::Locker* GXGraphicsImpl::GetLocker()
  {
    return m_pGraphicsLocker;
  }

  inline GXLPCSTR GXGraphicsImpl::InlGetPlatformStringA() const
  {
    return "d3d11";
  }

} // namespace D3D11

#endif // _GX_GRAPHICS_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11