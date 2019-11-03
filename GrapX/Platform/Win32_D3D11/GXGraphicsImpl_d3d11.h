#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GX_GRAPHICS_H_
#define _GX_GRAPHICS_H_

#define COLORREF_TO_NATIVE(CLR) ((CLR & 0xff00ff00) | ((CLR & 0xff0000) >> 16) | ((CLR & 0xff) << 16))
#define NATIVE_TO_COLORREF(CLR) COLORREF_TO_NATIVE(CLR)

#define MAX_TEXTURE_STAGE  D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
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
class CanvasCore;
class GTexture;
class GXConsole;
class ILogger;

namespace Marimo {
  class ShaderConstName;
} // namespace Marimo

namespace GrapX
{
  class SamplerState;
  class EffectImpl;

  namespace D3D11
  {
    class TextureImpl;
    class CanvasCoreImpl;
    //class GRenderStateImpl;
    class RasterizerStateImpl;
    class BlendStateImpl;
    class DepthStencilStateImpl;
    class SamplerStateImpl;
    class Canvas3DImpl;
    template<class InterfaceT> class TextureBaseImplT;
    typedef TextureBaseImplT<TextureBase> TexBaseImpl;
    class TextureImpl_SwapBuffer;

    class VertexDeclImpl;
    class RenderTargetImpl;
    class RenderTargetImpl_BackBuffer;

#define GRAPHICS_CREATION_FLAG_DEBUG  0x00000001

    struct GRAPHICS_CREATION_DESC
    {
      HWND          hWnd;
      GXBOOL        bWaitForVSync;
      GXLPCWSTR     szRootDir;
      ILogger*      pLogger;
      GXDWORD       dwCreateFlags; // GRAPHICS_CREATION_FLAG_
      //GXAPP_DESC*   pAppDesc;
    };

    enum class StateResult : int
    {
      Failed = -1,
      Ok = 0,
      Same = 1,
    };

    template<
      class BlendStateTempl,
      class SamplerStateTempl,
      class RasterizerStateTempl,
      class DepthStencilStateTempl>
    struct DEVICECONTEXT_TEMPL
    {
      BlendStateTempl*         pBlendState = NULL;
      SamplerStateTempl*       pSamplerState = NULL;
      RasterizerStateTempl*    pRasterizerState = NULL;
      DepthStencilStateTempl*  pDepthStencilState = NULL;

      inline GXBOOL InlIsActiveSamplerState(SamplerStateTempl* _pSamplerState)
      {
        return this->pSamplerState == _pSamplerState;
      }

      inline GXBOOL InlIsActiveRasterizerState(RasterizerStateTempl* _pRasterizerState)
      {
        return this->pRasterizerState == _pRasterizerState;
      }

      inline GXBOOL InlIsActiveBlendState(BlendStateTempl* _pBlendState)
      {
        return this->pBlendState == _pBlendState;
      }

      inline GXBOOL InlIsActiveDepthStencilState(DepthStencilStateTempl* _pDepthStencilState)
      {
        return this->pDepthStencilState == _pDepthStencilState;
      }

      //template<class _TDrive, class _TStateObject>
      //inline GXBOOL InlSetStateT(GXUINT slot, _TStateObject*& pCurState, _TStateObject* pState)
      //{
      //  ASSERT(pState);
      //  if(pCurState == pState) {
      //    return TRUE;
      //  }
      //  _TStateObject* pPrevState = pCurState;
      //  pCurState = pState;
      //  if(pCurState->Activate(static_cast<_TDrive*>(this), slot, pPrevState)) // slot为了模板兼容，并不是所有对象都使用这个
      //  {
      //    SAFE_RELEASE(pPrevState);
      //    pCurState->AddRef();
      //    return TRUE;
      //  }

      //  // 如果失败就换回来
      //  pCurState = pPrevState;
      //  return FALSE;
      //}
    }; // struct DEVICECONTEXT_TEMPL

    struct DEVICECONTEXT : public DEVICECONTEXT_TEMPL<BlendStateImpl, SamplerStateImpl, RasterizerStateImpl, DepthStencilStateImpl>
    {
    private:
    public:
      ID3D11DeviceContext*    pContext = NULL;
      RenderTargetImpl*       pRenderTarget = NULL;
      Shader*                 pShader = NULL;
      TexBaseImpl*            pTextures[MAX_TEXTURE_STAGE] = { NULL };
      CanvasCore*             pCanvasCore = NULL;
      
      //BlendStateImpl*         pBlendState = NULL;
      //SamplerStateImpl*       pSamplerState = NULL;
      //RasterizerStateImpl*    pRasterizerState = NULL;
      //DepthStencilStateImpl*  pDepthStencilState = NULL;

      D3D_PRIMITIVE_TOPOLOGY  eTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
      VertexDeclImpl*         pVertexDecl = NULL;
      Primitive*              pPrimitive = NULL;

      // State Object - 状态对象
      ID3D11RenderTargetView* pD3D11RenderTargetView = NULL;
      ID3D11DepthStencilView* pD3D11DepthStencilView = NULL;

    public:
      inline ID3D11DeviceContext* D3DGetDeviceContext ()
      {
        return pContext;
      }
    };

    class GraphicsImpl : public GraphicsBaseImpl
    {
      //friend class GPrimImpl;
      friend class GPrimitiveVertexOnlyImpl;
      friend class GPrimitiveVertexIndexImpl;
      friend class VertexDeclImpl;
      //friend class GRenderState;
      friend class SamplerState;
      friend class CanvasCoreImpl;
      friend class CanvasImpl;
      friend class Canvas3DImpl;
      friend class ShaderImpl;
    public:
      enum
      {
        F_CREATEDEVICE = 0x00000001,  // 对象自己创建的设备
        F_DEVICEHASLOST = 0x00000002,  // 设备已经丢失
        F_ACTIVATE = 0x00000004,
        F_SHOWCONSOLE = 0x00000008,
        F_WAITFORVSYNC = 0x00000010,
      };
      typedef GrapX::Internal::GXResourceMgr  GXResourceMgr;
    public:
      static Graphics* Create(const GRAPHICS_CREATION_DESC* pDesc);

      // 初始化成员
      virtual HRESULT Initialize(const GRAPHICS_CREATION_DESC* pDesc);

#include "Platform/CommonInline/GXGraphicsImpl_ClassDecl.inl"
      //GXHRESULT CreateOffscreenPlainSurface      (GTexture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage);

    public:
      Effect*                     IntGetEffect        ();
      void                        IntGetDimension     (GXUINT& nWidth, GXUINT& nHeight);  // 
      inline ID3D11Device*        D3DGetDevice        ();
      inline ID3D11DeviceContext* D3DGetDeviceContext ();
      DEVICECONTEXT*              GetCurrentContext   ();
      //GXBOOL                      D3DGetSwapChainDesc  (DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
      inline clstd::Locker*       GetLocker           ();
    private:
      GXHRESULT         Test              ();

      inline void       InlUpdateTopology     (GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount);

      //GXBOOL      IntCheckSizeOfTargetAndDepth();
      GXVOID      BuildInputLayout();
      void IntEnumAdapter();

    private:
      const GXUINT            s_uCanvasCacheCount;
      HWND                    m_hWnd;
      D3D_DRIVER_TYPE         m_driverType;
      D3D_FEATURE_LEVEL       m_featureLevel;
      ID3D11Device*           m_pd3dDevice;
      ID3D11DeviceContext*    m_pImmediateContext;
      IDXGISwapChain*         m_pSwapChain;
      DXGI_SWAP_CHAIN_DESC    m_SwapChainDesc;

      RenderTarget*           m_pTempBuffer;
      RenderTargetImpl_BackBuffer* m_pBackBufferRenderTarget;
      Shader*                 m_pBasicShader;
      Effect*                 m_pBasicEffect;


      ID3D11InputLayout*      m_pVertexLayout;
      GXDWORD                 m_dwBackBufferStencil;  // m_pDeviceOriginTex 使用的模板 [1, 255]

      GXINT                   m_nGraphicsCount;
      DWORD                   m_idCreationThread = 0;   // 创建时所在线程
      DWORD                   m_dwThreadId; // 用来检测Begin()和End()是否调用自同一个线程
    };

    inline ID3D11Device* GraphicsImpl::D3DGetDevice()
    {
      return m_pd3dDevice;
    }

    inline ID3D11DeviceContext* GraphicsImpl::D3DGetDeviceContext()
    {
      return m_pImmediateContext;
    }

    inline clstd::Locker* GraphicsImpl::GetLocker()
    {
      return m_pGraphicsLocker;
    }

    inline GXLPCSTR GraphicsImpl::InlGetPlatformStringA() const
    {
      return "d3d11";
    }


  } // namespace D3D11
} // namespace GrapX

#endif // _GX_GRAPHICS_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11