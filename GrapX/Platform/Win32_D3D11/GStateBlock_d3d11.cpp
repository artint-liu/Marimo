#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.h>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// 私有头文件
#include "Platform/Win32_D3D11/GStateBlock_D3D11.h"
#define _GXGRAPHICS_INLINE_RENDERSTATE_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "User/GrapX.Hxx"
#include "GrapX/gxError.h"
#include "grapX/GXCanvas.h"

#ifdef ENABLE_GRAPHICS_API_DX11
namespace GrapX
{
  namespace D3D11
  {
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

    static float s_fOne = 1.0f;
    //const static FLOAT crBlendFactor[4] = {1,1,1,1};

#define GETRSVALUE(TYPE)  m_aRenderStateValue[s_aEnumToIdx[TYPE]]

  //////////////////////////////////////////////////////////////////////////

  //  //////////////////////////////////////////////////////////////////////////




    STATIC_ASSERT(GXSTENCILOP_KEEP == D3D11_STENCIL_OP_KEEP);
    STATIC_ASSERT(GXSTENCILOP_ZERO == D3D11_STENCIL_OP_ZERO);
    STATIC_ASSERT(GXSTENCILOP_REPLACE == D3D11_STENCIL_OP_REPLACE);
    STATIC_ASSERT(GXSTENCILOP_INCRSAT == D3D11_STENCIL_OP_INCR_SAT);
    STATIC_ASSERT(GXSTENCILOP_DECRSAT == D3D11_STENCIL_OP_DECR_SAT);
    STATIC_ASSERT(GXSTENCILOP_INVERT == D3D11_STENCIL_OP_INVERT);
    STATIC_ASSERT(GXSTENCILOP_INCR == D3D11_STENCIL_OP_INCR);
    STATIC_ASSERT(GXSTENCILOP_DECR == D3D11_STENCIL_OP_DECR);








    STATIC_ASSERT(GXCMP_NEVER == D3D11_COMPARISON_NEVER);
    STATIC_ASSERT(GXCMP_LESS == D3D11_COMPARISON_LESS);
    STATIC_ASSERT(GXCMP_EQUAL == D3D11_COMPARISON_EQUAL);
    STATIC_ASSERT(GXCMP_LESSEQUAL == D3D11_COMPARISON_LESS_EQUAL);
    STATIC_ASSERT(GXCMP_GREATER == D3D11_COMPARISON_GREATER);
    STATIC_ASSERT(GXCMP_NOTEQUAL == D3D11_COMPARISON_NOT_EQUAL);
    STATIC_ASSERT(GXCMP_GREATEREQUAL == D3D11_COMPARISON_GREATER_EQUAL);
    STATIC_ASSERT(GXCMP_ALWAYS == D3D11_COMPARISON_ALWAYS);


    STATIC_ASSERT(GXBLEND_ZERO == D3D11_BLEND_ZERO);
    STATIC_ASSERT(GXBLEND_ONE == D3D11_BLEND_ONE);
    STATIC_ASSERT(GXBLEND_SRCCOLOR == D3D11_BLEND_SRC_COLOR);
    STATIC_ASSERT(GXBLEND_INVSRCCOLOR == D3D11_BLEND_INV_SRC_COLOR);
    STATIC_ASSERT(GXBLEND_SRCALPHA == D3D11_BLEND_SRC_ALPHA);
    STATIC_ASSERT(GXBLEND_INVSRCALPHA == D3D11_BLEND_INV_SRC_ALPHA);
    STATIC_ASSERT(GXBLEND_DESTALPHA == D3D11_BLEND_DEST_ALPHA);
    STATIC_ASSERT(GXBLEND_INVDESTALPHA == D3D11_BLEND_INV_DEST_ALPHA);
    STATIC_ASSERT(GXBLEND_DESTCOLOR == D3D11_BLEND_DEST_COLOR);
    STATIC_ASSERT(GXBLEND_INVDESTCOLOR == D3D11_BLEND_INV_DEST_COLOR);
    STATIC_ASSERT(GXBLEND_SRCALPHASAT == D3D11_BLEND_SRC_ALPHA_SAT);
    STATIC_ASSERT(GXBLEND_BOTHSRCALPHA == 12);
    STATIC_ASSERT(GXBLEND_BOTHINVSRCALPHA == 13);
    STATIC_ASSERT(GXBLEND_BLENDFACTOR == D3D11_BLEND_BLEND_FACTOR);
    STATIC_ASSERT(GXBLEND_INVBLENDFACTOR == D3D11_BLEND_INV_BLEND_FACTOR);
    STATIC_ASSERT(GXBLEND_SRCCOLOR2 == D3D11_BLEND_SRC1_COLOR);
    STATIC_ASSERT(GXBLEND_INVSRCCOLOR2 == D3D11_BLEND_INV_SRC1_COLOR);

    STATIC_ASSERT(GXBLENDOP_ADD == D3D11_BLEND_OP_ADD);
    STATIC_ASSERT(GXBLENDOP_SUBTRACT == D3D11_BLEND_OP_SUBTRACT);
    STATIC_ASSERT(GXBLENDOP_REVSUBTRACT == D3D11_BLEND_OP_REV_SUBTRACT);
    STATIC_ASSERT(GXBLENDOP_MIN == D3D11_BLEND_OP_MIN);
    STATIC_ASSERT(GXBLENDOP_MAX == D3D11_BLEND_OP_MAX);

    STATIC_ASSERT(GXFILL_WIREFRAME == D3D11_FILL_WIREFRAME);
    STATIC_ASSERT(GXFILL_SOLID == D3D11_FILL_SOLID);

    STATIC_ASSERT(GXCULL_NONE == D3D11_CULL_NONE);
    STATIC_ASSERT(GXCULL_CW == D3D11_CULL_FRONT);
    STATIC_ASSERT(GXCULL_CCW == D3D11_CULL_BACK);



    
    //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT RasterizerStateImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT RasterizerStateImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        SAFE_RELEASE(m_pRasterizerState);
        m_pGraphicsImpl->UnregisterResource(this);
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    RasterizerStateImpl::RasterizerStateImpl(GraphicsImpl* pGraphicsImpl)
      : RasterizerState()
      , m_pGraphicsImpl (pGraphicsImpl)
      , m_pRasterizerState(NULL)
    {
      InlSetZeroT(m_RasterizerDesc);
    }

    GXBOOL RasterizerStateImpl::Initialize(GXRASTERIZERDESC* pDesc)
    {
      if(pDesc->cbSize != sizeof(GXRASTERIZERDESC)) {
        return FALSE;
      }

      //INT DepthBias;
      ASSERT(pDesc->DepthBias == 0); // 还不知道怎么转换

      m_RasterizerDesc.FillMode = (D3D11_FILL_MODE)pDesc->FillMode;
      m_RasterizerDesc.CullMode = (D3D11_CULL_MODE)pDesc->CullMode;
      m_RasterizerDesc.FrontCounterClockwise = pDesc->FrontCounterClockwise;
      m_RasterizerDesc.DepthBiasClamp = pDesc->DepthBiasClamp;
      m_RasterizerDesc.SlopeScaledDepthBias = pDesc->SlopeScaledDepthBias;
      m_RasterizerDesc.DepthClipEnable = pDesc->DepthClipEnable;
      m_RasterizerDesc.ScissorEnable = pDesc->ScissorEnable;

      ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

      if(GXFAILED(pd3dDevice->CreateRasterizerState(&m_RasterizerDesc, &m_pRasterizerState))) {
        CLOG_ERROR("RasterizerState: Error to create d3d11 rasterizer state.\n");
        return FALSE;
      }
      return TRUE;
    }

    GXBOOL RasterizerStateImpl::Activate(DEVICECONTEXT* pContext, GXUINT slot, RasterizerStateImpl* pPrevState)
    {
      ASSERT(slot == 0);
      ASSERT(m_pGraphicsImpl->InlIsActiveRasterizerState(this));
      InlSetRasterizerState(pContext);
      return TRUE;
    }

    inline void RasterizerStateImpl::InlSetRasterizerState(DEVICECONTEXT* pContext)
    {
      ID3D11DeviceContext* pd3dContext = pContext->D3DGetDeviceContext();
      //GXColor crBlendFactor = m_BlendFactor;
      pd3dContext->RSSetState(m_pRasterizerState);
    }
    //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT BlendStateImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT BlendStateImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        SAFE_RELEASE(m_pBlendState);
        m_pGraphicsImpl->UnregisterResource(this);
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    D3D11_BLEND BlendStateImpl::ColorBlendToAlphaBlend(GXBlend blend)
    {
      switch (blend)
      {
      case GXBLEND_SRCCOLOR:          return D3D11_BLEND_SRC_ALPHA;
      case GXBLEND_INVSRCCOLOR:           return D3D11_BLEND_INV_SRC_ALPHA;
      case GXBLEND_DESTCOLOR: return D3D11_BLEND_DEST_ALPHA;
      case GXBLEND_INVDESTCOLOR: return D3D11_BLEND_INV_DEST_ALPHA;
      case GXBLEND_SRCCOLOR2: return D3D11_BLEND_SRC1_ALPHA;
      case GXBLEND_INVSRCCOLOR2: return D3D11_BLEND_INV_SRC1_ALPHA;
      default:
        return (D3D11_BLEND)blend;
      }
      return (D3D11_BLEND)blend;
    }

    BlendStateImpl::BlendStateImpl(GraphicsImpl* pGraphicsImpl)
      : BlendState     ()
      , m_pGraphicsImpl (pGraphicsImpl)
      , m_BlendFactor   (-1)
      , m_pBlendState   (NULL)
    {
      InlSetZeroT(m_BlendDesc);
    }

    GXBOOL BlendStateImpl::Initialize(GXBLENDDESC* pDesc, GXUINT nNum)
    {
      // 状态合法性检查
      if(pDesc == NULL || nNum > countof(m_BlendDesc.RenderTarget) || nNum == 0) {
        CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, "BlendState");
        return FALSE;
      }

      m_BlendDesc.AlphaToCoverageEnable = FALSE;
      m_BlendDesc.IndependentBlendEnable = FALSE;
      for(GXUINT i = 0; i < nNum; i++)
      {
        GXBLENDDESC& Desc = pDesc[i < nNum ? i : nNum - 1];
        D3D11_RENDER_TARGET_BLEND_DESC& RenderTarget = m_BlendDesc.RenderTarget[i];

        RenderTarget.BlendEnable            = Desc.BlendEnable;
        RenderTarget.SrcBlend               = (D3D11_BLEND)Desc.SrcBlend;
        RenderTarget.DestBlend              = (D3D11_BLEND)Desc.DestBlend;
        RenderTarget.BlendOp                = (D3D11_BLEND_OP)Desc.BlendOp;
        if(Desc.SeparateAlphaBlend)
        {
          RenderTarget.SrcBlendAlpha  = (D3D11_BLEND)Desc.SrcBlendAlpha;
          RenderTarget.DestBlendAlpha = (D3D11_BLEND)Desc.DestBlendAlpha;
          RenderTarget.BlendOpAlpha   = (D3D11_BLEND_OP)Desc.BlendOpAlpha;
        }
        else
        {
          RenderTarget.SrcBlendAlpha  = ColorBlendToAlphaBlend(Desc.SrcBlend);
          RenderTarget.DestBlendAlpha = ColorBlendToAlphaBlend(Desc.DestBlend);
          RenderTarget.BlendOpAlpha   = (D3D11_BLEND_OP)Desc.BlendOp;
        }
        RenderTarget.RenderTargetWriteMask  = (D3D11_BLEND)Desc.WriteMask;
      }

      ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

      if(GXFAILED(pd3dDevice->CreateBlendState(&m_BlendDesc, &m_pBlendState))) {
        CLOG_ERROR("BlendState: Error to create d3d11 blend state.\n");
        return FALSE;
      }
      return TRUE;
    }

    void BlendStateImpl::InlSetBlendState(ID3D11DeviceContext* pd3dContext)
    {
      //ID3D11DeviceContext* pd3dContext = pContext->D3DGetDeviceContext();
      GXColor crBlendFactor = m_BlendFactor;
      pd3dContext->OMSetBlendState(m_pBlendState, (float*)&crBlendFactor, 0xffffffff);
    }

    GXBOOL BlendStateImpl::Activate(DEVICECONTEXT* pContext, GXUINT slot, BlendStateImpl* pPrevState)
    {
      ASSERT(slot == 0);
      ASSERT(m_pGraphicsImpl->InlIsActiveBlendState(this));
      InlSetBlendState(pContext->D3DGetDeviceContext());
      return TRUE;
    }

    GXDWORD BlendStateImpl::SetBlendFactor(CanvasCore* pCanvasCore, GXDWORD dwBlendFactor)
    {
      const GXDWORD dwPrevFactor = m_BlendFactor;
      m_BlendFactor = dwBlendFactor;

      if(m_pGraphicsImpl->InlIsActiveBlendState(this))
      {
        InlSetBlendState(static_cast<GraphicsImpl*>(pCanvasCore->GetGraphicsUnsafe())->D3DGetDeviceContext());
      }
      return dwPrevFactor;
    }
    //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT DepthStencilStateImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT DepthStencilStateImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        SAFE_RELEASE(m_pDepthStencilState);
        m_pGraphicsImpl->UnregisterResource(this);
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    DepthStencilStateImpl::DepthStencilStateImpl(GraphicsImpl* pGraphicsImpl)
      : DepthStencilState  ()
      , m_pGraphicsImpl     (pGraphicsImpl)
      , m_StencilRef        (0)
      , m_pDepthStencilState(NULL)
    {
      InlSetZeroT(m_DepthStencilDesc);
    }

    GXBOOL DepthStencilStateImpl::Initialize(GXDEPTHSTENCILDESC* pDesc)
    {
      m_DepthStencilDesc.DepthEnable = pDesc->DepthEnable;
      m_DepthStencilDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)pDesc->DepthWriteMask;
      m_DepthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)pDesc->DepthFunc;
      m_DepthStencilDesc.StencilEnable = pDesc->StencilEnable;
      m_DepthStencilDesc.StencilReadMask = pDesc->StencilReadMask;
      m_DepthStencilDesc.StencilWriteMask = pDesc->StencilWriteMask;

      m_DepthStencilDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)pDesc->FrontFace.StencilFailOp;
      m_DepthStencilDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)pDesc->FrontFace.StencilDepthFailOp;
      m_DepthStencilDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)pDesc->FrontFace.StencilPassOp;
      m_DepthStencilDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)pDesc->FrontFace.StencilFunc;

      m_DepthStencilDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilFailOp;
      m_DepthStencilDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilDepthFailOp;
      m_DepthStencilDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilPassOp;
      m_DepthStencilDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)pDesc->BackFace.StencilFunc;

      ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      HRESULT hval = pd3dDevice->CreateDepthStencilState(&m_DepthStencilDesc, &m_pDepthStencilState);
      return SUCCEEDED(hval);
    }

    GXBOOL DepthStencilStateImpl::Activate(DEVICECONTEXT* pContext, GXUINT slot, DepthStencilStateImpl* pPrevState)
    {
      ASSERT(slot == 0);
      ID3D11DeviceContext* const pImmediateContext = pContext->D3DGetDeviceContext();
      pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
      return TRUE;
    }

    GXDWORD DepthStencilStateImpl::SetStencilRef(CanvasCore* pCanvasCore, GXDWORD dwStencilRef)
    {
      //const GXDWORD dwPrevStencilReft = m_StencilRef;
      //m_StencilRef = dwStencilRef;
      //if(m_pGraphicsImpl->InlIsActiveDepthStencilState(this))
      //{
      //  ID3D11DeviceContext* const pImmediateContext = static_cast<GraphicsImpl*>(pCanvasCore->GetGraphicsUnsafe())->D3DGetDeviceContext();
      //  pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
      //}
      //return dwPrevStencilReft;
      ID3D11DeviceContext* const pd3dContext = static_cast<GraphicsImpl*>(pCanvasCore->GetGraphicsUnsafe())->D3DGetDeviceContext();
      return SetStencilRef(pd3dContext, dwStencilRef);
    }

    GXDWORD DepthStencilStateImpl::SetStencilRef(ID3D11DeviceContext* pd3dContext, GXDWORD dwStencilRef)
    {
      const GXDWORD dwPrevStencilReft = m_StencilRef;
      m_StencilRef = dwStencilRef;
      if (m_pGraphicsImpl->InlIsActiveDepthStencilState(this))
      {
        //ID3D11DeviceContext* const pImmediateContext = static_cast<GraphicsImpl*>(pCanvasCore->GetGraphicsUnsafe())->D3DGetDeviceContext();
        pd3dContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
      }
      return dwPrevStencilReft;
    }

    //////////////////////////////////////////////////////////////////////////
#define DX11_IMPLEMENT

    SamplerStateImpl::~SamplerStateImpl()
    {
      if(m_pD3D11SamplerState) {
        m_pGraphicsImpl->UnregisterResource(this);
      }
      SAFE_RELEASE(m_pD3D11SamplerState);
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT SamplerStateImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT SamplerStateImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  
    SamplerStateImpl::SamplerStateImpl(Graphics* pGraphics)
      : SamplerState   ()
      , m_pGraphicsImpl (static_cast<GraphicsImpl*>(pGraphics))
      , m_pD3D11SamplerState(NULL)
    {
      InlSetZeroT(m_SamplerDesc);
    }

    GXBOOL SamplerStateImpl::Initialize(const GXSAMPLERDESC* pDesc)
    {
      ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      D3D11_SAMPLER_DESC D3D11Desc;
      InterfaceDescToD3D11Desc(D3D11Desc, *pDesc);
      m_SamplerDesc = *pDesc;
      HRESULT hr = pd3dDevice->CreateSamplerState(&D3D11Desc, &m_pD3D11SamplerState);
      return SUCCEEDED(hr);
    }


    GXBOOL SamplerStateImpl::Activate(DEVICECONTEXT* pContext, GXUINT slot, SamplerStateImpl* pPrevSamplerState)
    {
      ASSERT(m_pGraphicsImpl->InlIsActiveSamplerState(this)); // 确定已经放置到Graphics上

      ID3D11DeviceContext* const pd3dDeviceContext = pContext->D3DGetDeviceContext();
      pd3dDeviceContext->PSSetSamplers(slot, 1, &m_pD3D11SamplerState);
      return TRUE;
    }

    GXHRESULT SamplerStateImpl::GetDesc(GXSAMPLERDESC* pSamplerDesc)
    {
      *pSamplerDesc = m_SamplerDesc;
      return GX_OK;
    }

    void SamplerStateImpl::InterfaceDescToD3D11Desc(D3D11_SAMPLER_DESC& D3D11Desc, const GXSAMPLERDESC& desc)
    {
      InlSetZeroT(D3D11Desc);

      D3D11Desc.Filter = GrapXToDX11::FilterFrom(desc.MagFilter, desc.MinFilter, desc.MipFilter);

      D3D11Desc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressU;
      D3D11Desc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressV;
      D3D11Desc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressW;
      //sampDesc.MipLODBias     = SamplerStage.dwMipmapLodBias; // FIXME: 类型不对
      //sampDesc.MaxAnisotropy  = SamplerDesc.MaxAnisotropy;
      D3D11Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
      D3D11Desc.BorderColor[0] = desc.BorderColor.r; // FIXME: 顺序没有验证
      D3D11Desc.BorderColor[1] = desc.BorderColor.g;
      D3D11Desc.BorderColor[2] = desc.BorderColor.b;
      D3D11Desc.BorderColor[3] = desc.BorderColor.a;
      D3D11Desc.MinLOD = 0;
      D3D11Desc.MaxLOD = D3D11_FLOAT32_MAX;
    }

    void SamplerStateImpl::D3D11DescToInterfaceDesc(GXSAMPLERDESC& desc, const D3D11_SAMPLER_DESC& D3D11Desc)
    {
      STATIC_ASSERT(static_cast<int>(D3D11_FILTER_TYPE_POINT) + 1 == static_cast<int>(GXTEXFILTER_POINT));
      STATIC_ASSERT(static_cast<int>(D3D11_FILTER_TYPE_LINEAR) + 1 == static_cast<int>(GXTEXFILTER_LINEAR));

      STATIC_ASSERT(GXTADDRESS_WRAP       == D3D11_TEXTURE_ADDRESS_WRAP);
      STATIC_ASSERT(GXTADDRESS_MIRROR     == D3D11_TEXTURE_ADDRESS_MIRROR);
      STATIC_ASSERT(GXTADDRESS_CLAMP      == D3D11_TEXTURE_ADDRESS_CLAMP);
      STATIC_ASSERT(GXTADDRESS_BORDER     == D3D11_TEXTURE_ADDRESS_BORDER);
      STATIC_ASSERT(GXTADDRESS_MIRRORONCE == D3D11_TEXTURE_ADDRESS_MIRROR_ONCE);

      desc.MinFilter = static_cast<GXTextureFilterType>(D3D11_DECODE_MIN_FILTER(D3D11Desc.Filter) + 1);
      desc.MagFilter = static_cast<GXTextureFilterType>(D3D11_DECODE_MAG_FILTER(D3D11Desc.Filter) + 1);
      desc.MipFilter = static_cast<GXTextureFilterType>(D3D11_DECODE_MIP_FILTER(D3D11Desc.Filter) + 1);

      desc.AddressU = static_cast<GXTextureAddress>(D3D11Desc.AddressU);
      desc.AddressV = static_cast<GXTextureAddress>(D3D11Desc.AddressV);
      desc.AddressW = static_cast<GXTextureAddress>(D3D11Desc.AddressW);

      desc.BorderColor.r = D3D11Desc.BorderColor[0];
      desc.BorderColor.g = D3D11Desc.BorderColor[1];
      desc.BorderColor.b = D3D11Desc.BorderColor[2];
      desc.BorderColor.a = D3D11Desc.BorderColor[3];
    }

    //GXHRESULT SamplerStateImpl::ResetToDefault()
    //{
    //  //CLBREAK;
    //  return GX_FAIL;
    //}

  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)