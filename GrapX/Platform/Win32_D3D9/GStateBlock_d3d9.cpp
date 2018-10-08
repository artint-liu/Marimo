#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.h>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"

// 私有头文件
#define _GXGRAPHICS_INLINE_RENDERSTATE_
#include "Canvas/GXResourceMgr.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"
#include "Platform/Win32_D3D9/GStateBlock_d3d9.h"
#include "User/GrapX.Hxx"
#include "GrapX/gxError.h"

namespace D3D9
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  static float s_fOne = 1.0f;

#define GETRSVALUE(TYPE)  m_aRenderStateValue[s_aEnumToIdx[TYPE]]


  //////////////////////////////////////////////////////////////////////////
  //GXINT  GRenderStateImpl::s_aEnumToIdx[LASTRENDERSTATEENUM];
  //GXDWORD  GRenderStateImpl::s_aRenderStateValue[RENDERSTATECOUNT];

  //// 这个排序影响到分组
  //GXRenderStateType  GRenderStateImpl::s_aRenderStateTypeList[RENDERSTATECOUNT + 1] = {
  //  //GXRS_ALPHABLENDENABLE,          GXRS_SRCBLEND,          GXRS_DESTBLEND,        GXRS_BLENDOP,
  //  //GXRS_SEPARATEALPHABLENDENABLE,  GXRS_SRCBLENDALPHA,     GXRS_DESTBLENDALPHA,   GXRS_BLENDOPALPHA,
  //  //GXRS_COLORWRITEENABLE, GXRS_COLORWRITEENABLE1,         GXRS_COLORWRITEENABLE2,         GXRS_COLORWRITEENABLE3,
  //  //GXRS_BLENDFACTOR,        

  //  //GXRS_ZENABLE,          GXRS_ZWRITEENABLE,        GXRS_ZFUNC,          
  //  GXRS_FILLMODE,                  GXRS_CULLMODE,

  //  GXRS_ANTIALIASEDLINEENABLE,     GXRS_SCISSORTESTENABLE,

  //  GXRS_SHADEMODE,        
  //  GXRS_LASTPIXEL,        
  //  GXRS_DITHERENABLE,              GXRS_SPECULARENABLE,

  //  //GXRS_STENCILENABLE,     GXRS_STENCILREF,          GXRS_STENCILMASK,        GXRS_STENCILWRITEMASK,      
  //  //GXRS_STENCILFAIL,       GXRS_STENCILZFAIL,        GXRS_STENCILPASS,        GXRS_STENCILFUNC,   
  //  //GXRS_CCW_STENCILFAIL,   GXRS_CCW_STENCILZFAIL,    GXRS_CCW_STENCILPASS,    GXRS_CCW_STENCILFUNC,      

  //  GXRS_TEXTUREFACTOR,             GXRS_CLIPPING,
  //  GXRS_LIGHTING,                  GXRS_AMBIENT,                 GXRS_COLORVERTEX,
  //  GXRS_LOCALVIEWER,               GXRS_NORMALIZENORMALS,        GXRS_DIFFUSEMATERIALSOURCE,   GXRS_SPECULARMATERIALSOURCE,
  //  GXRS_AMBIENTMATERIALSOURCE,     GXRS_EMISSIVEMATERIALSOURCE,  GXRS_VERTEXBLEND,             GXRS_CLIPPLANEENABLE,
  //  
  //  GXRS_POINTSIZE,                 GXRS_POINTSIZE_MIN,           GXRS_POINTSPRITEENABLE,       GXRS_POINTSCALEENABLE,
  //  GXRS_POINTSCALE_A,              GXRS_POINTSCALE_B,            GXRS_POINTSCALE_C,            GXRS_POINTSIZE_MAX,

  //  GXRS_MULTISAMPLEANTIALIAS,      GXRS_MULTISAMPLEMASK,         GXRS_PATCHEDGESTYLE,          GXRS_DEBUGMONITORTOKEN,         
  //  GXRS_INDEXEDVERTEXBLENDENABLE,  GXRS_TWEENFACTOR,        
  //  GXRS_POSITIONDEGREE,            GXRS_NORMALDEGREE,            GXRS_SLOPESCALEDEPTHBIAS,
  //  GXRS_MINTESSELLATIONLEVEL,      GXRS_MAXTESSELLATIONLEVEL,    GXRS_ADAPTIVETESS_X,
  //  GXRS_ADAPTIVETESS_Y,            GXRS_ADAPTIVETESS_Z,          GXRS_ADAPTIVETESS_W,          GXRS_ENABLEADAPTIVETESSELLATION,
  //  GXRS_TWOSIDEDSTENCILMODE,  
  //  GXRS_SRGBWRITEENABLE,           GXRS_DEPTHBIAS,        

  //  // Alpha Test
  //  GXRS_ALPHATESTENABLE,      GXRS_ALPHAREF,          GXRS_ALPHAFUNC,

  //  //// Fog
  //  //GXRS_FOGENABLE,        GXRS_FOGCOLOR,          GXRS_FOGSTART,          GXRS_FOGEND,
  //  //GXRS_FOGTABLEMODE,        GXRS_FOGVERTEXMODE,      GXRS_FOGDENSITY,        GXRS_RANGEFOGENABLE,      

  //  //// Wrap
  //  //GXRS_WRAP0,          GXRS_WRAP1,          GXRS_WRAP2,          GXRS_WRAP3,
  //  //GXRS_WRAP4,          GXRS_WRAP5,          GXRS_WRAP6,          GXRS_WRAP7,    
  //  //GXRS_WRAP8,          GXRS_WRAP9,          GXRS_WRAP10,          GXRS_WRAP11,      
  //  //GXRS_WRAP12,          GXRS_WRAP13,          GXRS_WRAP14,          GXRS_WRAP15,
  //  (GXRenderStateType)0
  //};

  //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GRasterizerStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GRasterizerStateImpl::Release()
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

  GRasterizerStateImpl::GRasterizerStateImpl(GXGraphicsImpl* pGraphicsImpl)
    : GRasterizerState()
    , m_pGraphicsImpl (pGraphicsImpl)
  {
    InlSetZeroT(m_RasterizerDesc);
  }

  GRasterizerStateImpl::~GRasterizerStateImpl()
  {
    m_pGraphicsImpl->UnregisterResource(this);
  }

  GXBOOL GRasterizerStateImpl::Initialize(GXRASTERIZERDESC* pDesc)
  {
    GXLPCSTR c_szPrefix = "Rasterizer State:";
    if(pDesc->cbSize != sizeof(GXRASTERIZERDESC)) {
      CLOG_ERROR("%s Desc size does not match.\n", c_szPrefix);
      return FALSE;
    }
    if(pDesc->FillMode != GXFILL_POINT && pDesc->FillMode != GXFILL_SOLID &&
      pDesc->FillMode != GXFILL_WIREFRAME)
    {
      CLOG_ERROR("%s Invalid FillMode enum value.\n", c_szPrefix);
      return FALSE;
    }
    if(pDesc->CullMode != GXCULL_NONE && pDesc->CullMode != GXCULL_CW &&
      pDesc->CullMode != GXCULL_CCW)
    {
      CLOG_ERROR("%s Invalid CullMode enum value.\n", c_szPrefix);
      return FALSE;
    }

    if(pDesc->ScissorEnable != FALSE && pDesc->ScissorEnable != TRUE)
    {
      CLOG_ERROR("%s \"ScissorEnable\" must be TRUE or FALSE.\n", c_szPrefix);
      return FALSE;
    }

    ASSERT(pDesc->FrontCounterClockwise == 0); // 不支持
    ASSERT(pDesc->DepthBias == 0); // 不支持
    ASSERT(pDesc->DepthBiasClamp == 0); // 不支持
    ASSERT(pDesc->SlopeScaledDepthBias == 0); // 不支持
    ASSERT(pDesc->DepthClipEnable == TRUE); // 不支持

    m_RasterizerDesc = *pDesc;
    return TRUE;
  }

  STATIC_ASSERT(GXCULL_NONE       == D3DCULL_NONE);
  STATIC_ASSERT(GXCULL_CW         == D3DCULL_CW);
  STATIC_ASSERT(GXCULL_CCW        == D3DCULL_CCW);

  STATIC_ASSERT(GXFILL_POINT      == D3DFILL_POINT);
  STATIC_ASSERT(GXFILL_WIREFRAME  == D3DFILL_WIREFRAME);
  STATIC_ASSERT(GXFILL_SOLID      == D3DFILL_SOLID);

  GXBOOL GRasterizerStateImpl::Activate(GRasterizerStateImpl* pPrevState)
  {
    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    if(pPrevState == NULL)
    {
      pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_RasterizerDesc.FillMode);
      pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_RasterizerDesc.CullMode);
      pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, m_RasterizerDesc.ScissorEnable);
    }
    else
    {
      if(m_RasterizerDesc.FillMode != pPrevState->m_RasterizerDesc.FillMode) {
        pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_RasterizerDesc.FillMode);
      }

      if(m_RasterizerDesc.CullMode != pPrevState->m_RasterizerDesc.CullMode) {
        pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_RasterizerDesc.CullMode);
      }

      if(m_RasterizerDesc.ScissorEnable != pPrevState->m_RasterizerDesc.ScissorEnable) {
        pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, m_RasterizerDesc.ScissorEnable);
      }
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GBlendStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GBlendStateImpl::Release()
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

  GBlendStateImpl::GBlendStateImpl(GXGraphicsImpl* pGraphicsImpl)
    :GBlendState()
    , m_pGraphicsImpl(pGraphicsImpl)
    , m_BlendFactor(-1)
  {
  }

  GBlendStateImpl::~GBlendStateImpl()
  {
    m_pGraphicsImpl->UnregisterResource(this);
  }

  GXBOOL GBlendStateImpl::Initialize(GXBLENDDESC* pDesc, GXUINT nNum)
  {
    // 状态合法性检查
    if(pDesc == NULL || nNum != 1) {
      CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, "BlendState");
      return FALSE;
    }
    
    if((pDesc->BlendEnable != 0 && pDesc->BlendEnable != 1) ||
      (pDesc->SeparateAlphaBlend != 0 && pDesc->SeparateAlphaBlend != 1)) {
      CLOG_ERROR("BlendState: BlendEnable/SeparateAlphaBlend 必须是 TRUE 或者 FALSE.\n");
      return FALSE;
    }

    if(pDesc->BlendOp < 1 || pDesc->BlendOp > 5 ||
      pDesc->BlendOpAlpha < 1 || pDesc->BlendOpAlpha > 5) {
        CLOG_ERROR("BlendState: BlendOp/BlendOpAlpha 超出了枚举范围.\n");
        return FALSE;
    }

    if(IsInvalidBlend(pDesc->DestBlend) || IsInvalidBlend(pDesc->SrcBlend) ||
      IsInvalidBlend(pDesc->DestBlendAlpha) || IsInvalidBlend(pDesc->SrcBlendAlpha)) {
        CLOG_ERROR("BlendState: Blend Type 超出了枚举范围.\n");
        return FALSE;
    }

    if(pDesc->WriteMask & (~0xf)) {
      CLOG_ERROR("BlendState: WriteMask 只使用了 4 bits, 但是参数中超过了这个值.\n");
      return FALSE;
    }

    m_BlendDesc = *pDesc;
    return TRUE;
  }

  GXBOOL GBlendStateImpl::Activate(GBlendStateImpl* pPrevState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveBlendState(this));
    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    if(pPrevState != NULL)
    {
      const GXBLENDDESC& PrevDesc = pPrevState->m_BlendDesc;
      if(m_BlendDesc.BlendEnable != PrevDesc.BlendEnable) {
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, m_BlendDesc.BlendEnable);
      }

      if(m_BlendDesc.BlendEnable) {
        if(PrevDesc.BlendEnable) {
          if(m_BlendDesc.BlendOp != PrevDesc.BlendOp) {
            pd3dDevice->SetRenderState(D3DRS_BLENDOP, m_BlendDesc.BlendOp);
          }
          if(m_BlendDesc.SrcBlend != PrevDesc.SrcBlend) {
            pd3dDevice->SetRenderState(D3DRS_SRCBLEND, m_BlendDesc.SrcBlend);
          }
          if(m_BlendDesc.DestBlend != PrevDesc.DestBlend) {
            pd3dDevice->SetRenderState(D3DRS_DESTBLEND, m_BlendDesc.DestBlend);
          }
        }
        else
        {
          pd3dDevice->SetRenderState(D3DRS_BLENDOP, m_BlendDesc.BlendOp);
          pd3dDevice->SetRenderState(D3DRS_SRCBLEND, m_BlendDesc.SrcBlend);
          pd3dDevice->SetRenderState(D3DRS_DESTBLEND, m_BlendDesc.DestBlend);
        }
      }

      if(m_BlendDesc.SeparateAlphaBlend != PrevDesc.SeparateAlphaBlend)
      {
        pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, m_BlendDesc.SeparateAlphaBlend);
      }
      
      if(m_BlendDesc.SeparateAlphaBlend == TRUE)
      {
        pd3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, m_BlendDesc.BlendOpAlpha);
        pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, m_BlendDesc.SrcBlendAlpha);
        pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, m_BlendDesc.DestBlendAlpha);
      }

      if(m_BlendDesc.WriteMask != PrevDesc.WriteMask) {
        pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, m_BlendDesc.WriteMask);
      }
      if(m_BlendFactor != pPrevState->m_BlendFactor) {
        pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR, m_BlendFactor);
      }
    }
    else
    {
      pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, m_BlendDesc.BlendEnable);
      if(m_BlendDesc.BlendEnable == TRUE)
      {
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, m_BlendDesc.SrcBlend);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, m_BlendDesc.DestBlend);
        pd3dDevice->SetRenderState(D3DRS_BLENDOP, m_BlendDesc.BlendOp);
      }

      pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, m_BlendDesc.SeparateAlphaBlend);
      if(m_BlendDesc.SeparateAlphaBlend == TRUE)
      {
        pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, m_BlendDesc.SrcBlendAlpha);
        pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, m_BlendDesc.DestBlendAlpha);
        pd3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, m_BlendDesc.BlendOpAlpha);
      }
      pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, m_BlendDesc.WriteMask);
      pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR, m_BlendFactor);
    }
    return TRUE;       
  }

  GXDWORD GBlendStateImpl::SetBlendFactor(GXDWORD dwBlendFactor)
  {
    const GXDWORD dwPrevFactor = m_BlendFactor;
    m_BlendFactor = dwBlendFactor;
    if(m_pGraphicsImpl->InlIsActiveBlendState(this))
    {
      LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR, m_BlendFactor);
    }
    return dwPrevFactor;
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GDepthStencilStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GDepthStencilStateImpl::Release()
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

  GDepthStencilStateImpl::GDepthStencilStateImpl(GXGraphicsImpl* pGraphicsImpl)
    : GDepthStencilState()
    , m_pGraphicsImpl   (pGraphicsImpl)
    , m_StencilRef      (0)
  {
  }

  GDepthStencilStateImpl::~GDepthStencilStateImpl()
  {
    m_pGraphicsImpl->UnregisterResource(this);
  }

  GXBOOL GDepthStencilStateImpl::Initialize(GXDEPTHSTENCILDESC* pDesc)
  {
    if(pDesc == NULL) {
      CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, "DepthStencilState");
      return FALSE;
    }

    if(pDesc->DepthEnable != 0 && pDesc->DepthEnable != 1 &&
      pDesc->StencilEnable != 0 && pDesc->StencilEnable != 1) {
      CLOG_ERROR("DepthStencilState: 非法的 DepthEnable 或者 StencilEnable.\n");
      return FALSE;
    }

    if(pDesc->DepthWriteMask != 0 && pDesc->DepthWriteMask != 1) {
      CLOG_ERROR("DepthStencilState: DepthWriteMask 只能是 0 或者 1.\n");
      return FALSE;
    }

    if(pDesc->DepthFunc < 1 || pDesc->DepthFunc > 8) {
      CLOG_ERROR("DepthStencilState: DepthFunc 超出了枚举范围.\n");
      return FALSE;
    }
    GXBOOL bval = CheckStencilOp(&pDesc->FrontFace, "FrontFace") && CheckStencilOp(&pDesc->BackFace, "BackFace");
    if(bval) {
      m_Desc = *pDesc;
    }
    return bval;
  }

  GXBOOL GDepthStencilStateImpl::CheckStencilOp(GXDEPTHSTENCILOP* pStencilOp, GXLPCSTR szPrefix)
  {
    if(pStencilOp->StencilFailOp < 1 || pStencilOp->StencilFailOp > 8) {
      CLOG_ERROR("DepthStencilState: %s.StencilFailOp 超出了枚举范围.\n", szPrefix);
      return FALSE;
    }
    if(pStencilOp->StencilDepthFailOp < 1 || pStencilOp->StencilDepthFailOp > 8) {
      CLOG_ERROR("DepthStencilState: %s.StencilDepthFailOp 超出了枚举范围.\n", szPrefix);
      return FALSE;
    }
    if(pStencilOp->StencilPassOp < 1 || pStencilOp->StencilPassOp > 8) {
      CLOG_ERROR("DepthStencilState: %s.StencilPassOp 超出了枚举范围.\n", szPrefix);
      return FALSE;
    }
    if(pStencilOp->StencilFunc < 1 || pStencilOp->StencilFunc > 8) {
      CLOG_ERROR("DepthStencilState: %s.StencilFunc 超出了枚举范围.\n", szPrefix);
      return FALSE;
    }
    return TRUE;
  }

  GXBOOL GDepthStencilStateImpl::Activate(GDepthStencilStateImpl* pPrevState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveDepthStencilState(this));
    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    GXDEPTHSTENCILDESC& PrevDesc = pPrevState->m_Desc;

    if(pPrevState != NULL)
    {
      if(m_Desc.DepthEnable != PrevDesc.DepthEnable) {
        pd3dDevice->SetRenderState(D3DRS_ZENABLE, m_Desc.DepthEnable);
      }
      if(m_Desc.DepthWriteMask != PrevDesc.DepthWriteMask) {
        pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, m_Desc.DepthWriteMask);
      }

      if(m_Desc.DepthEnable)
      {
        if(PrevDesc.DepthEnable) {
          if(m_Desc.DepthFunc != PrevDesc.DepthFunc) {
            pd3dDevice->SetRenderState(D3DRS_ZFUNC, m_Desc.DepthFunc);
          }
        }
        else {
          pd3dDevice->SetRenderState(D3DRS_ZFUNC, m_Desc.DepthFunc);
        }
      }

      if(m_Desc.StencilEnable != PrevDesc.StencilEnable) {
        pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, m_Desc.StencilEnable);
      }

      if(m_Desc.StencilEnable)
      {
        if(PrevDesc.StencilEnable)
        {
          if(m_StencilRef != pPrevState->m_StencilRef) {
            pd3dDevice->SetRenderState(D3DRS_STENCILREF, m_StencilRef);
          }

          if(m_Desc.FrontFace.StencilFailOp != PrevDesc.FrontFace.StencilFailOp) {
            pd3dDevice->SetRenderState(D3DRS_STENCILFAIL,  m_Desc.FrontFace.StencilFailOp);
          }
          if(m_Desc.FrontFace.StencilDepthFailOp != PrevDesc.FrontFace.StencilDepthFailOp) {
            pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, m_Desc.FrontFace.StencilDepthFailOp);
          }
          if(m_Desc.FrontFace.StencilPassOp != PrevDesc.FrontFace.StencilPassOp) {
            pd3dDevice->SetRenderState(D3DRS_STENCILPASS,  m_Desc.FrontFace.StencilPassOp);
          }
          if(m_Desc.FrontFace.StencilFunc != PrevDesc.FrontFace.StencilFunc) {
            pd3dDevice->SetRenderState(D3DRS_STENCILFUNC,  m_Desc.FrontFace.StencilFunc);
          }

          if(m_Desc.BackFace.StencilFailOp != PrevDesc.BackFace.StencilFailOp) {
            pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL,  m_Desc.BackFace.StencilFailOp);
          }
          if(m_Desc.BackFace.StencilDepthFailOp != PrevDesc.BackFace.StencilDepthFailOp) {
            pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, m_Desc.BackFace.StencilDepthFailOp);
          }
          if(m_Desc.BackFace.StencilPassOp != PrevDesc.BackFace.StencilPassOp) {
            pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS,  m_Desc.BackFace.StencilPassOp);
          }
          if(m_Desc.BackFace.StencilFunc != PrevDesc.BackFace.StencilFunc) {
            pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC,  m_Desc.BackFace.StencilFunc);
          }

          if(m_Desc.StencilReadMask != PrevDesc.StencilReadMask) {
            pd3dDevice->SetRenderState(D3DRS_STENCILMASK, m_Desc.StencilReadMask);
          }

          if(m_Desc.StencilWriteMask != PrevDesc.StencilWriteMask) {
            pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, m_Desc.StencilWriteMask);
          }
        }
        else
        {
          pd3dDevice->SetRenderState(D3DRS_STENCILREF, m_StencilRef);

          pd3dDevice->SetRenderState(D3DRS_STENCILFAIL,  m_Desc.FrontFace.StencilFailOp);
          pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, m_Desc.FrontFace.StencilDepthFailOp);
          pd3dDevice->SetRenderState(D3DRS_STENCILPASS,  m_Desc.FrontFace.StencilPassOp);
          pd3dDevice->SetRenderState(D3DRS_STENCILFUNC,  m_Desc.FrontFace.StencilFunc);

          pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL,  m_Desc.BackFace.StencilFailOp);
          pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, m_Desc.BackFace.StencilDepthFailOp);
          pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS,  m_Desc.BackFace.StencilPassOp);
          pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC,  m_Desc.BackFace.StencilFunc);

          pd3dDevice->SetRenderState(D3DRS_STENCILMASK, m_Desc.StencilReadMask);
          pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, m_Desc.StencilWriteMask);
        }
      }
    }
    else
    {
      pd3dDevice->SetRenderState(D3DRS_ZENABLE, m_Desc.DepthEnable);
      pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, m_Desc.DepthWriteMask);
      pd3dDevice->SetRenderState(D3DRS_ZFUNC, m_Desc.DepthFunc);
      pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, m_Desc.StencilEnable);
      pd3dDevice->SetRenderState(D3DRS_STENCILREF, m_StencilRef);

      pd3dDevice->SetRenderState(D3DRS_STENCILFAIL,  m_Desc.FrontFace.StencilFailOp);
      pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, m_Desc.FrontFace.StencilDepthFailOp);
      pd3dDevice->SetRenderState(D3DRS_STENCILPASS,  m_Desc.FrontFace.StencilPassOp);
      pd3dDevice->SetRenderState(D3DRS_STENCILFUNC,  m_Desc.FrontFace.StencilFunc);

      pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL,  m_Desc.BackFace.StencilFailOp);
      pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, m_Desc.BackFace.StencilDepthFailOp);
      pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS,  m_Desc.BackFace.StencilPassOp);
      pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC,  m_Desc.BackFace.StencilFunc);

      pd3dDevice->SetRenderState(D3DRS_STENCILMASK, m_Desc.StencilReadMask);
      pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, m_Desc.StencilWriteMask);
    }
    return TRUE;
  }

  GXDWORD GDepthStencilStateImpl::SetStencilRef(GXDWORD dwStencilRef)
  {
    const GXDWORD dwPrevStencilReft = m_StencilRef;
    m_StencilRef = dwStencilRef;

    if(m_pGraphicsImpl->InlIsActiveDepthStencilState(this))
    {
      LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      pd3dDevice->SetRenderState(D3DRS_STENCILREF, m_StencilRef);
    }
    return dwPrevStencilReft;
  }

  //////////////////////////////////////////////////////////////////////////
#include "Platform/CommonInline/GStateBlock.inl"

  GSamplerStateImpl::GSamplerStateImpl(GXGraphics* pGraphics)
    : GSamplerState   ()
    , m_pGraphicsImpl (static_cast<GXGraphicsImpl*>(pGraphics))
  {
    //ResetToDefault();
    InlSetZeroT(m_dwDefaultMask);
  }

  GSamplerStateImpl::~GSamplerStateImpl()
  {
  }

  //void IntSetSamplerToDefault(GXLPSAMPLERSTAGE lpSampStage)
  //{
  //  lpSampStage->dwMask = 0;
  //  lpSampStage->dwAddressU       = GXTADDRESS_WRAP;
  //  lpSampStage->dwAddressV       = GXTADDRESS_WRAP;
  //  lpSampStage->dwAddressW       = GXTADDRESS_WRAP;
  //  lpSampStage->dwBorderColor    = 0x00000000;
  //  lpSampStage->dwMagFilter      = GXTEXFILTER_POINT;
  //  lpSampStage->dwMinFilter      = GXTEXFILTER_POINT;
  //  lpSampStage->dwMipFilter      = GXTEXFILTER_NONE;
  //  lpSampStage->dwMipmapLodBias  = 0;
  //  lpSampStage->dwMaxMipLevel    = 0;
  //  lpSampStage->dwMaxAnisotropy  = 1;
  //  lpSampStage->dwSRGBTexture    = 0;
  //  lpSampStage->dwElementIndex   = 0;
  //  lpSampStage->dwDMapOffset     = 0;
  //}


  void GSamplerStateImpl::SetStageToDevice(GXUINT Stage, const GXSAMPLERDESC* pPrevSampDesc)
  {
    LPDIRECT3DDEVICE9 const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXSAMPLERDESC& ThisSampDesc = m_SamplerDesc[Stage];
    if(pPrevSampDesc != NULL)
    {
      if(pPrevSampDesc->AddressU != ThisSampDesc.AddressU) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, ThisSampDesc.AddressU);
      }

      if(pPrevSampDesc->AddressV != ThisSampDesc.AddressV) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, ThisSampDesc.AddressV);
      }

      if(pPrevSampDesc->AddressW != ThisSampDesc.AddressW) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSW, ThisSampDesc.AddressW);
      }

      if(pPrevSampDesc->BorderColor != ThisSampDesc.BorderColor) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_BORDERCOLOR, ThisSampDesc.BorderColor.ARGB());
      }

      if(pPrevSampDesc->MagFilter != ThisSampDesc.MagFilter) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MAGFILTER, ThisSampDesc.MagFilter);
      }

      if(pPrevSampDesc->MinFilter != ThisSampDesc.MinFilter) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MINFILTER, ThisSampDesc.MinFilter);
      }

      if(pPrevSampDesc->MipFilter != ThisSampDesc.MipFilter) {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MIPFILTER, ThisSampDesc.MipFilter);
      }
    }
    else
    {
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, ThisSampDesc.AddressU);
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, ThisSampDesc.AddressV);
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSW, ThisSampDesc.AddressW);
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_BORDERCOLOR, ThisSampDesc.BorderColor.ARGB());
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MAGFILTER, ThisSampDesc.MagFilter);
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MINFILTER, ThisSampDesc.MinFilter);
        pd3dDevice->SetSamplerState(Stage, D3DSAMP_MIPFILTER, ThisSampDesc.MipFilter);
    }
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, ThisSampDesc.MipmapLodBias);
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL,   ThisSampDesc.MaxMipLevel);
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, ThisSampDesc.MaxAnisotropy);
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_SRGBTEXTURE,   ThisSampDesc.SRGBTexture);
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_ELEMENTINDEX,  ThisSampDesc.ElementIndex);
    //pd3dDevice->SetSamplerState(Stage, D3DSAMP_DMAPOFFSET,    ThisSampDesc.DMapOffset);
  }

  GXBOOL GSamplerStateImpl::Initialize(GSamplerStateImpl* pDefault)
  {
    return TRUE;
  }

  GXBOOL GSamplerStateImpl::Activate(GSamplerStateImpl* pPrevSamplerState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveSamplerState(this)); // 确定已经放置到Graphics上

    if(pPrevSamplerState != NULL)
    {
      ASSERT(this != pPrevSamplerState); // 外面保证这个

      for(int i = 0; i < SAMPLERCOUNT; i++)
      {
        //GXSAMPLERSTAGE& PrevSamp = pPrevSamplerState->m_SamplerStage[i];
        //GXSAMPLERSTAGE& ThisSamp = m_SamplerStage[i];
        const GXSAMPLERDESC& PrevSampDesc = pPrevSamplerState->m_SamplerDesc[i];
        //GXSAMPLERDESC& ThisSampDesc = m_SamplerDesc[i];

        // 前后两个状态的默认值位
        const GXDWORD dwPrevMask = pPrevSamplerState->m_dwDefaultMask[i] & 0xffff;
        const GXDWORD dwThisMask = m_dwDefaultMask[i] & 0xffff;

        // 如果其中任何"是"非默认值, 则设置
        if((dwPrevMask | dwThisMask) != 0) {
          SetStageToDevice(i, &PrevSampDesc);
        }
      }
    }
    else {
      for(int i = 0; i < SAMPLERCOUNT; i++)
      {
        SetStageToDevice(i, NULL);
      }
    }

    return TRUE;
  }
  //GXBOOL GSamplerStateImpl::ResetToDefault()
  //{
  //  if(m_pGraphicsImpl->InlIsActiveSamplerState(this))
  //  {
  //    for(DWORD i = 0; i < SAMPLERCOUNT; i++) {
  //      // 如果已经是默认值就跳过
  //      if((m_SamplerStage[i].dwMask & 0xffff) == 0) {
  //        continue;
  //      }

  //      IntSetSamplerToDefault(&m_SamplerStage[i]);
  //      SetStageToDevice(i);
  //    }
  //  }
  //  else {
  //    for(DWORD i = 0; i < SAMPLERCOUNT; i++) {
  //      // 如果已经是默认值就跳过
  //      if((m_SamplerStage[i].dwMask & 0xffff) == 0) {
  //        continue;
  //      }

  //      IntSetSamplerToDefault(&m_SamplerStage[i]);
  //    }
  //  }
  //  return TRUE;
  //}
  void GSamplerStateImpl::IntUpdateStates(GXUINT Sampler, const GXSAMPLERDESC* pSamplerDesc, const GXSAMPLERDESC* pDefault)
  {
    GXDWORD& dwDefaultMask = m_dwDefaultMask[Sampler];
    dwDefaultMask = 0;

    // 更新默认状态掩码
    if(pSamplerDesc->AddressU != pDefault->AddressU) {
      SETBIT(dwDefaultMask, D3DSAMP_ADDRESSU);
    }
    if(pSamplerDesc->AddressV != pDefault->AddressV) {
      SETBIT(dwDefaultMask, D3DSAMP_ADDRESSV);
    }
    if(pSamplerDesc->AddressW != pDefault->AddressW) {
      SETBIT(dwDefaultMask, D3DSAMP_ADDRESSW);
    }
    if(pSamplerDesc->BorderColor != pDefault->BorderColor) {
      SETBIT(dwDefaultMask, D3DSAMP_BORDERCOLOR);
    }
    if(pSamplerDesc->MagFilter != pDefault->MagFilter) {
      SETBIT(dwDefaultMask, D3DSAMP_MAGFILTER);
    }
    if(pSamplerDesc->MinFilter != pDefault->MinFilter) {
      SETBIT(dwDefaultMask, D3DSAMP_MINFILTER);
    }
    if(pSamplerDesc->MipFilter != pDefault->MipFilter) {
      SETBIT(dwDefaultMask, D3DSAMP_MIPFILTER);
    }

    // 更新对应sampler的状态
    m_SamplerDesc[Sampler] = *pSamplerDesc; // TODO: 检查合法性
  }

  GXHRESULT GSamplerStateImpl::SetState(GXUINT nSamplerSlot, GXSAMPLERDESC* pSamplerDesc)
  {
    if(nSamplerSlot >= SAMPLERCOUNT) {
      CLOG_ERROR("%s : Bad sampler index.\n", __FUNCTION__);
      return GX_ERROR_OUTOFRANGE;
    }
    GXSAMPLERDESC DefaultDesc;
    GXSAMPLERDESC PrevDesc = m_SamplerDesc[nSamplerSlot];

    IntUpdateStates(nSamplerSlot, pSamplerDesc, &DefaultDesc);

    if(m_pGraphicsImpl->InlIsActiveSamplerState(this))
    {
      SetStageToDevice(nSamplerSlot, &PrevDesc);
    }
    return GX_OK;
  }
 
  GXHRESULT GSamplerStateImpl::SetStateArray(GXUINT nStartSlot, GXSAMPLERDESC* pSamplerDesc, int nCount)
  {
    if(nStartSlot >= SAMPLERCOUNT) {
      CLOG_ERROR("%s : Bad sampler index.\n", __FUNCTION__);
      return GX_ERROR_OUTOFRANGE;
    }
    if(nStartSlot + nCount > SAMPLERCOUNT)
    {
      CLOG_WARNING("%s : Out of slot range.\n", __FUNCTION__);
      nCount = SAMPLERCOUNT - nStartSlot;
    }

    GXSAMPLERDESC DefaultDesc;
    if(m_pGraphicsImpl->InlIsActiveSamplerState(this))
    {
      for(int i = 0; i < nCount; i++)
      {
        const GXUINT nSampler = nStartSlot + i;
        GXSAMPLERDESC PrevDesc = m_SamplerDesc[nSampler];

        IntUpdateStates(nSampler, &pSamplerDesc[i], &DefaultDesc);
        SetStageToDevice(nSampler, &PrevDesc);
      }
    }
    else
    {
      for(int i = 0; i < nCount; i++)
      {
        IntUpdateStates(nStartSlot + i, &pSamplerDesc[i], &DefaultDesc);
      }
    }
    return GX_OK;
  }
  
  GXHRESULT GSamplerStateImpl::ResetToDefault()
  {
    static GXSAMPLERDESC aSamplerDesc[SAMPLERCOUNT];
    return SetStateArray(0, aSamplerDesc, SAMPLERCOUNT);
  }
} // namespace D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

//////////////////////////////////////////////////////////////////////////

const static float s_fOne = 1.0f;
const static float s_f8192 = 8192.0f;
const static GXRENDERSTATE c_aDefaultRenderState[] = {

  // Rasterizer State
  {GXRS_FILLMODE, GXFILL_SOLID},
  {GXRS_CULLMODE, GXCULL_CCW},
  {GXRS_SCISSORTESTENABLE, FALSE},
  {GXRS_ANTIALIASEDLINEENABLE, FALSE},
  {GXRS_DEPTHBIAS, 0}, // D3D11 上没实现这个,看到这个注释确认一下,如果已经实现就删除这个注释
  {GXRS_SLOPESCALEDEPTHBIAS, 0},

  // Blend State
  //{GXRS_BLENDFACTOR, 0xffffffff},
  //{GXRS_ALPHABLENDENABLE, FALSE},
  //{GXRS_SRCBLEND, GXBLEND_ONE},
  //{GXRS_DESTBLEND, GXBLEND_ZERO},
  //{GXRS_BLENDOP, GXBLENDOP_ADD},
  //{GXRS_SEPARATEALPHABLENDENABLE, FALSE},
  //{GXRS_SRCBLENDALPHA, GXBLEND_ONE},
  //{GXRS_DESTBLENDALPHA, GXBLEND_ZERO},
  //{GXRS_BLENDOPALPHA, GXBLENDOP_ADD},
  //{GXRS_COLORWRITEENABLE, 0x0000000f},
  //{GXRS_COLORWRITEENABLE1, 0x0000000f},
  //{GXRS_COLORWRITEENABLE2, 0x0000000f},
  //{GXRS_COLORWRITEENABLE3, 0x0000000f},

  // Depth Stencil State
  //{GXRS_ZENABLE, FALSE},
  //{GXRS_ZFUNC, GXCMP_LESSEQUAL},
  //{GXRS_ZWRITEENABLE, TRUE},
  //{GXRS_STENCILENABLE, FALSE},
  //{GXRS_STENCILREF, 0},
  //{GXRS_STENCILFAIL, GXSTENCILOP_KEEP},
  //{GXRS_STENCILZFAIL, GXSTENCILOP_KEEP},
  //{GXRS_STENCILPASS, GXSTENCILOP_KEEP},  
  //{GXRS_STENCILFUNC, GXCMP_ALWAYS},
  //{GXRS_CCW_STENCILFAIL, GXSTENCILOP_KEEP},  
  //{GXRS_CCW_STENCILZFAIL, GXSTENCILOP_KEEP},
  //{GXRS_CCW_STENCILPASS, GXSTENCILOP_KEEP},
  //{GXRS_CCW_STENCILFUNC, GXCMP_ALWAYS},
  //{GXRS_STENCILMASK, 0xffffffff},
  //{GXRS_STENCILWRITEMASK, 0xffffffff},

  // 下面的没分类
  {GXRS_ALPHAREF, 0},
  {GXRS_ALPHATESTENABLE, FALSE},
  {GXRS_ALPHAFUNC, GXCMP_ALWAYS},

  {GXRS_LASTPIXEL, TRUE},
  {GXRS_DITHERENABLE, FALSE},      
  {GXRS_TEXTUREFACTOR, 0xffffffff},        
  {GXRS_LOCALVIEWER, TRUE},            
  {GXRS_TWOSIDEDSTENCILMODE, FALSE},
  {GXRS_SRGBWRITEENABLE, 0},
       
  {GXRS_MINTESSELLATIONLEVEL, *(DWORD*)&s_fOne}, 
  {GXRS_MAXTESSELLATIONLEVEL, *(DWORD*)&s_fOne},

  {GXRS_WRAP0, 0},                {GXRS_WRAP1, 0},                {GXRS_WRAP2, 0},
  {GXRS_WRAP3, 0},                {GXRS_WRAP4, 0},                {GXRS_WRAP5, 0},
  {GXRS_WRAP6, 0},                {GXRS_WRAP7, 0},                {GXRS_WRAP8, 0},
  {GXRS_WRAP9, 0},                {GXRS_WRAP10, 0},               {GXRS_WRAP11, 0},
  {GXRS_WRAP12, 0},               {GXRS_WRAP13, 0},               {GXRS_WRAP14, 0},
  {GXRS_WRAP15, 0},                

  {GXRS_SPECULARENABLE, FALSE},
  {GXRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1},
  {GXRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL},
  {GXRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2},
  {GXRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL},

  {GXRS_SHADEMODE, D3DSHADE_GOURAUD},
  {GXRS_POSITIONDEGREE, D3DDEGREE_CUBIC},
  {GXRS_NORMALDEGREE, D3DDEGREE_LINEAR},

  {GXRS_FOGSTART, 0}, {GXRS_FOGEND, 1}, {GXRS_FOGDENSITY, 1},

  {GXRS_POINTSCALE_A, *(DWORD*)&s_fOne}, 
  {GXRS_POINTSIZE, *(DWORD*)&s_fOne}, 
  {GXRS_POINTSIZE_MIN, *(DWORD*)&s_fOne}, 
  {GXRS_POINTSIZE_MAX, *(DWORD*)&s_f8192},
  {(GXRenderStateType)0,0},
};

GXLPCRENDERSTATE GXDLLAPI MOGetDefaultRenderState()
{
  return c_aDefaultRenderState;
}

STATIC_ASSERT(sizeof(GXRENDERSTATE) == 8);
//STATIC_ASSERT(sizeof(D3D9::GXSAMPLERSTAGE) == sizeof(GXDWORD) * 14);
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
