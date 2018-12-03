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

    GXBOOL RasterizerStateImpl::Activate(RasterizerStateImpl* pPrevState)
    {
      ASSERT(m_pGraphicsImpl->InlIsActiveRasterizerState(this));
      InlSetRasterizerState();
      return TRUE;
    }

    inline void RasterizerStateImpl::InlSetRasterizerState()
    {
      ID3D11DeviceContext* pd3dContext = m_pGraphicsImpl->D3DGetDeviceContext();
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
        RenderTarget.BlendOp                = (D3D11_BLEND_OP)Desc.BlendOp;
        RenderTarget.BlendOpAlpha           = (D3D11_BLEND_OP)Desc.BlendOpAlpha;
        RenderTarget.DestBlend              = (D3D11_BLEND)Desc.DestBlend;
        RenderTarget.DestBlendAlpha         = (D3D11_BLEND)Desc.DestBlendAlpha;
        RenderTarget.SrcBlend               = (D3D11_BLEND)Desc.SrcBlend;
        RenderTarget.SrcBlendAlpha          = (D3D11_BLEND)Desc.SrcBlendAlpha;
        RenderTarget.RenderTargetWriteMask  = (D3D11_BLEND)Desc.WriteMask;
      }

      ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

      if(GXFAILED(pd3dDevice->CreateBlendState(&m_BlendDesc, &m_pBlendState))) {
        CLOG_ERROR("BlendState: Error to create d3d11 blend state.\n");
        return FALSE;
      }
      return TRUE;
    }

    void BlendStateImpl::InlSetBlendState()
    {
      ID3D11DeviceContext* pd3dContext = m_pGraphicsImpl->D3DGetDeviceContext();
      GXColor crBlendFactor = m_BlendFactor;
      pd3dContext->OMSetBlendState(m_pBlendState, (float*)&crBlendFactor, 0xffffffff);
    }

    GXBOOL BlendStateImpl::Activate(BlendStateImpl* pPrevState)
    {
      ASSERT(m_pGraphicsImpl->InlIsActiveBlendState(this));
      InlSetBlendState();
      return TRUE;
    }

    GXDWORD BlendStateImpl::SetBlendFactor(GXDWORD dwBlendFactor)
    {
      const GXDWORD dwPrevFactor = m_BlendFactor;
      m_BlendFactor = dwBlendFactor;

      if(m_pGraphicsImpl->InlIsActiveBlendState(this))
      {
        InlSetBlendState();
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

    GXBOOL DepthStencilStateImpl::Activate(DepthStencilStateImpl* pPrevState)
    {
      ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
      pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
      return TRUE;
    }

    GXDWORD DepthStencilStateImpl::SetStencilRef(GXDWORD dwStencilRef)
    {
      const GXDWORD dwPrevStencilReft = m_StencilRef;
      m_StencilRef = dwStencilRef;
      if(m_pGraphicsImpl->InlIsActiveDepthStencilState(this))
      {
        ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
        pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
      }
      return dwPrevStencilReft;
    }

    //////////////////////////////////////////////////////////////////////////
#define DX11_IMPLEMENT

    SamplerStateImpl::~SamplerStateImpl()
    {
      for(UINT i = 0; i < SAMPLERCOUNT; i++) {
        SAFE_RELEASE(m_pSampler[i]);
      }
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

  
    SamplerStateImpl::SamplerStateImpl(GrapX::Graphics* pGraphics)
      : SamplerState   ()
      , m_pGraphicsImpl (static_cast<GraphicsImpl*>(pGraphics))
      //, m_dwChangeMask  (0xffff)
    {
      //ResetToDefault();
      //memset(m_pSampler, 0, sizeof(ID3D11SamplerState*) * SAMPLERCOUNT);
      InlSetZeroT(m_pSampler);
      InlSetZeroT(m_SamplerDesc);
    }

    GXBOOL SamplerStateImpl::InitializeStatic()
    {
      //IntSetSamplerToDefault(&s_DefaultSamplerState);
      return TRUE;
    }

    GXBOOL SamplerStateImpl::Initialize(SamplerStateImpl* pDefault)
    {
      ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      //ID3D11DeviceContext* const pd3dDeviceContext = m_pGraphicsImpl->D3DGetDeviceContext();
      if(pDefault == NULL)
      {
        D3D11_SAMPLER_DESC sampler_desc = { D3D11_FILTER_MIN_MAG_MIP_POINT };
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MipLODBias = 0.0f;
        sampler_desc.MaxAnisotropy = 0;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS;
        sampler_desc.BorderColor[0] = 0.0f;
        sampler_desc.BorderColor[1] = 0.0f;
        sampler_desc.BorderColor[2] = 0.0f;
        sampler_desc.BorderColor[3] = 0.0f;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        ID3D11SamplerState* pd3d11SamplerState = NULL;
        V(pd3dDevice->CreateSamplerState(&sampler_desc, &pd3d11SamplerState));
        for(UINT i = 0; i < SAMPLERCOUNT; i++)
        {
          m_pSampler[i] = pd3d11SamplerState;
          m_pSampler[i]->AddRef();
        }
        SAFE_RELEASE(pd3d11SamplerState);
      }
      else
      {
        for(UINT i = 0; i < SAMPLERCOUNT; i++)
        {
          m_pSampler[i] = pDefault->m_pSampler[i];
          m_pSampler[i]->AddRef();
        }
      }
      return TRUE;
    }


    GXBOOL SamplerStateImpl::Activate(SamplerStateImpl* pPrevSamplerState)
    {
      ASSERT(m_pGraphicsImpl->InlIsActiveSamplerState(this)); // 确定已经放置到Graphics上

      ID3D11DeviceContext* const pd3dDeviceContext = m_pGraphicsImpl->D3DGetDeviceContext();
      pd3dDeviceContext->PSSetSamplers(0, SAMPLERCOUNT, m_pSampler);
      return TRUE;
    }

    GXHRESULT SamplerStateImpl::SetState(GXUINT Sampler, const GXSAMPLERDESC* pSamplerDesc)
    {
      ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      GXSAMPLERDESC& SamplerDesc = m_SamplerDesc[Sampler];

      D3D11_SAMPLER_DESC SampDesc11;
      InlSetZeroT(SampDesc11);

      SamplerDesc = *pSamplerDesc;
      GXColor crBorder = SamplerDesc.BorderColor;

      SampDesc11.Filter = GrapXToDX11::FilterFrom((GXTextureFilterType)SamplerDesc.MagFilter,
        (GXTextureFilterType)SamplerDesc.MinFilter, (GXTextureFilterType)SamplerDesc.MipFilter);

      SampDesc11.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressU;
      SampDesc11.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressV;
      SampDesc11.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressW;
      //sampDesc.MipLODBias     = SamplerStage.dwMipmapLodBias; // FIXME: 类型不对
      //sampDesc.MaxAnisotropy  = SamplerDesc.MaxAnisotropy;
      SampDesc11.ComparisonFunc = D3D11_COMPARISON_NEVER;
      SampDesc11.BorderColor[0] = crBorder.r; // FIXME: 顺序没有验证
      SampDesc11.BorderColor[1] = crBorder.g;
      SampDesc11.BorderColor[2] = crBorder.b;
      SampDesc11.BorderColor[3] = crBorder.a;
      SampDesc11.MinLOD = 0;
      SampDesc11.MaxLOD = D3D11_FLOAT32_MAX;

      // TODO: 可以提取一个特征值,从一个池中查询
      SAFE_RELEASE(m_pSampler[Sampler]);
      HRESULT hr = pd3dDevice->CreateSamplerState(&SampDesc11, &m_pSampler[Sampler]);
      if(FAILED(hr)) {
        TRACE(">%s(%d), create sampler error.\n", __FILE__, __LINE__);
        CLBREAK;
      }

      // 如果当前 sampler 在设备上, 则立即提交
      if(SUCCEEDED(hr) && m_pGraphicsImpl->InlIsActiveSamplerState(this))
      {
        ID3D11DeviceContext* const pd3dDeviceContext = m_pGraphicsImpl->D3DGetDeviceContext();
        pd3dDeviceContext->PSSetSamplers(Sampler, 1, &m_pSampler[Sampler]);
      }
      //SamplerStage.dwMask &= (~0xffff);
      //RESET_FLAG(m_dwChangeMask, 1 << dwStage);

      return GX_OK;
    }

    GXHRESULT SamplerStateImpl::SetStateArray(GXUINT nStartSlot, const GXSAMPLERDESC* pSamplerDesc, int nCount)
    {
      CLBREAK;
      return GX_FAIL;
    }

    GXHRESULT SamplerStateImpl::ResetToDefault()
    {
      //CLBREAK;
      return GX_FAIL;
    }

  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)