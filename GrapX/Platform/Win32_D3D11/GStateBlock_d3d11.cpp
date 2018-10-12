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
namespace D3D11
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  static float s_fOne = 1.0f;
  //const static FLOAT crBlendFactor[4] = {1,1,1,1};

#define GETRSVALUE(TYPE)  m_aRenderStateValue[s_aEnumToIdx[TYPE]]


  //////////////////////////////////////////////////////////////////////////
  //GXINT  GRenderState::s_aEnumToIdx[LASTRENDERSTATEENUM];
  //GXDWORD  GRenderState::s_aRenderStateValue[RENDERSTATECOUNT];

  //// 这个排序影响到分组
  //GXRenderStateType  GRenderState::s_aRenderStateTypeList[RENDERSTATECOUNT + 1] = {
  //  //GXRS_ALPHABLENDENABLE,      GXRS_SRCBLEND,          GXRS_DESTBLEND,        GXRS_BLENDOP,
  //  //GXRS_SEPARATEALPHABLENDENABLE,  GXRS_SRCBLENDALPHA,             GXRS_DESTBLENDALPHA,        GXRS_BLENDOPALPHA,

  //  //GXRS_ZENABLE,          GXRS_ZWRITEENABLE,        GXRS_ZFUNC,          
  //  GXRS_FILLMODE,        GXRS_CULLMODE,

  //  GXRS_ANTIALIASEDLINEENABLE,     GXRS_SCISSORTESTENABLE,

  //  GXRS_SHADEMODE,        
  //  GXRS_LASTPIXEL,        
  //  GXRS_DITHERENABLE,        GXRS_SPECULARENABLE,
  //  //GXRS_STENCILENABLE,             GXRS_STENCILFAIL,
  //  //GXRS_STENCILZFAIL,        GXRS_STENCILPASS,        GXRS_STENCILFUNC,        GXRS_STENCILREF,
  //  //GXRS_STENCILMASK,        GXRS_STENCILWRITEMASK,      
  //  GXRS_TEXTUREFACTOR,             GXRS_CLIPPING,
  //  GXRS_LIGHTING,          GXRS_AMBIENT,          GXRS_COLORVERTEX,
  //  GXRS_LOCALVIEWER,        GXRS_NORMALIZENORMALS,      GXRS_DIFFUSEMATERIALSOURCE,     GXRS_SPECULARMATERIALSOURCE,
  //  GXRS_AMBIENTMATERIALSOURCE,     GXRS_EMISSIVEMATERIALSOURCE,  GXRS_VERTEXBLEND,        GXRS_CLIPPLANEENABLE,
  //  GXRS_POINTSIZE,                 GXRS_POINTSIZE_MIN,             GXRS_POINTSPRITEENABLE,         GXRS_POINTSCALEENABLE,
  //  GXRS_POINTSCALE_A,        GXRS_POINTSCALE_B,        GXRS_POINTSCALE_C,        GXRS_MULTISAMPLEANTIALIAS,
  //  GXRS_MULTISAMPLEMASK,      GXRS_PATCHEDGESTYLE,      GXRS_DEBUGMONITORTOKEN,         GXRS_POINTSIZE_MAX,
  //  GXRS_INDEXEDVERTEXBLENDENABLE,  GXRS_COLORWRITEENABLE,      GXRS_TWEENFACTOR,        
  //  GXRS_POSITIONDEGREE,      GXRS_NORMALDEGREE,        GXRS_SLOPESCALEDEPTHBIAS,
  //  GXRS_MINTESSELLATIONLEVEL,    GXRS_MAXTESSELLATIONLEVEL,    GXRS_ADAPTIVETESS_X,
  //  GXRS_ADAPTIVETESS_Y,      GXRS_ADAPTIVETESS_Z,      GXRS_ADAPTIVETESS_W,            GXRS_ENABLEADAPTIVETESSELLATION,
  //  GXRS_TWOSIDEDSTENCILMODE,    
  //  //GXRS_CCW_STENCILFAIL,      GXRS_CCW_STENCILZFAIL,          GXRS_CCW_STENCILPASS,    GXRS_CCW_STENCILFUNC,      
  //  GXRS_COLORWRITEENABLE1,         GXRS_COLORWRITEENABLE2,         GXRS_COLORWRITEENABLE3,
  //  GXRS_BLENDFACTOR,        GXRS_SRGBWRITEENABLE,      GXRS_DEPTHBIAS,        

  //  // Alpha Test
  //  GXRS_ALPHATESTENABLE,      GXRS_ALPHAREF,          GXRS_ALPHAFUNC,

  //  // Fog
  //  GXRS_FOGENABLE,        GXRS_FOGCOLOR,          GXRS_FOGSTART,          GXRS_FOGEND,
  //  GXRS_FOGTABLEMODE,        GXRS_FOGVERTEXMODE,      GXRS_FOGDENSITY,        GXRS_RANGEFOGENABLE,      

  //  // Wrap
  //  GXRS_WRAP0,          GXRS_WRAP1,          GXRS_WRAP2,          GXRS_WRAP3,
  //  GXRS_WRAP4,          GXRS_WRAP5,          GXRS_WRAP6,          GXRS_WRAP7,    
  //  GXRS_WRAP8,          GXRS_WRAP9,          GXRS_WRAP10,          GXRS_WRAP11,      
  //  GXRS_WRAP12,          GXRS_WRAP13,          GXRS_WRAP14,          GXRS_WRAP15,
  //  (GXRenderStateType)0
  //};

  


  //GRenderState::GRenderState(GXGraphics* pGraphics)
  //  : m_pGraphicsImpl       ((GXGraphicsImpl*)pGraphics)
  //  //, m_bOnDevice           (FALSE)
  //  //, m_pBlendState         (NULL)
  //  //, m_pDepthStencilState  (NULL)
  //  , m_pRasterizerState    (NULL)
  //  //, m_dwStencilRef        (NULL)
  //{
  //  ASSERT(sizeof(GROUPMASK) == 4);
  //  ASSERT(sizeof(m_aChanged) == 16);
  //  memset(&m_aChanged, 0, sizeof(m_aChanged));
  //  ResetToDefault();

  //  m_RasterizerDesc.FillMode = D3D11_FILL_SOLID;
  //  m_RasterizerDesc.CullMode = D3D11_CULL_BACK;
  //  m_RasterizerDesc.FrontCounterClockwise = FALSE;
  //  m_RasterizerDesc.DepthBias = 0;
  //  m_RasterizerDesc.DepthBiasClamp = 0;
  //  m_RasterizerDesc.SlopeScaledDepthBias = 0;
  //  m_RasterizerDesc.DepthClipEnable = FALSE;
  //  m_RasterizerDesc.ScissorEnable = FALSE;
  //  m_RasterizerDesc.MultisampleEnable = FALSE;
  //  m_RasterizerDesc.AntialiasedLineEnable = FALSE;

  //  //////////////////////////////////////////////////////////////////////////
  //  //m_DepthStencilDesc.DepthEnable = FALSE;
  //  //m_DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  //  //m_DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  //  //m_DepthStencilDesc.StencilEnable = FALSE;
  //  //m_DepthStencilDesc.StencilReadMask = 0xf;
  //  //m_DepthStencilDesc.StencilWriteMask = 0xf;
  //  //m_DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  //  //m_DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  //  //m_DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  //  //////////////////////////////////////////////////////////////////////////
  //  //m_BlendState.AlphaToCoverageEnable = FALSE;
  //  //m_BlendState.IndependentBlendEnable = FALSE;
  //  //for(int i = 0; i < 8; i++)
  //  //{
  //  //  m_BlendState.RenderTarget[i].BlendEnable = FALSE;
  //  //  m_BlendState.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  //  //  m_BlendState.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  //  //  m_BlendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
  //  //  m_BlendState.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
  //  //  m_BlendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  //  //  m_BlendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  //  //  m_BlendState.RenderTarget[i].RenderTargetWriteMask = 0xf;
  //  //}

  //}

  //GXLRESULT GRenderState::AddRef()
  //{
  //  m_uRefCount++;
  //  return m_uRefCount;
  //}
  //GXLRESULT GRenderState::Release()
  //{
  //  m_uRefCount--;
  //  if(m_uRefCount == 0)
  //  {
  //    //SAFE_RELEASE(m_pBlendState);
  //    //SAFE_RELEASE(m_pDepthStencilState);
  //    SAFE_RELEASE(m_pRasterizerState);
  //    delete this;
  //    return GX_OK;
  //  }  
  //  //else if(m_uRefCount == 1)
  //  //{
  //  //  m_pGraphics->UnregisterResource(this);
  //  //}

  //  return m_uRefCount;
  //}

  //GXBOOL GRenderState::InitializeStatic()
  //{
  //  memset(s_aRenderStateValue, 0, RENDERSTATECOUNT * sizeof(GXDWORD));
  //  for(GXINT i = 0;; i++)
  //  {
  //    if(s_aRenderStateTypeList[i] == 0)
  //      break;
  //    s_aEnumToIdx[s_aRenderStateTypeList[i]] = i;
  //  }

  //  return TRUE;
  //}

  //GXBOOL GRenderState::Update(GRenderState* pPrevState)
  //{
  //  ASSERT(this != pPrevState);
  //  ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();

  //  if(pPrevState == NULL)
  //  {
  //    pImmediateContext->RSSetState(m_pRasterizerState);
  //    //pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_dwStencilRef);
  //    //pImmediateContext->OMSetBlendState(m_pBlendState, (FLOAT*)&m_BlendFactor, 0xffffffff);
  //  }
  //  else
  //  {

  //    if(m_pRasterizerState != pPrevState->m_pRasterizerState)
  //    {
  //      pImmediateContext->RSSetState(m_pRasterizerState);
  //    }

  //    //if(m_pDepthStencilState != pPrevState->m_pDepthStencilState || 
  //    //  m_dwStencilRef != pPrevState->m_dwStencilRef)
  //    //{
  //    //  pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_dwStencilRef);
  //    //}

  //    //if(m_pBlendState != pPrevState->m_pBlendState) {
  //    //  pImmediateContext->OMSetBlendState(m_pBlendState, (FLOAT*)&m_BlendFactor, 0xffffffff);
  //    //}
  //  }


  //  return TRUE;
  //}

  //GXBOOL GRenderState::ResetToDefault()
  //{
  //  GXLPCRENDERSTATE lpDefRenderState = MOGetDefaultRenderState();
  //  return SetBlock(lpDefRenderState);
  //}

  //GXBOOL GRenderState::Set(GXRenderStateType eType, GXDWORD dwValue)
  //{
  //  GXRENDERSTATE StateBlock[2] = {eType, dwValue, (GXRenderStateType)0, 0};
  //  return SetBlock(StateBlock);
  //  //const GXINT nStateIndex = s_aEnumToIdx[eType];
  //  //if(m_aRenderStateValue[nStateIndex] == dwValue)
  //  //  return FALSE;
  //  //m_aRenderStateValue[nStateIndex] = dwValue;
  //  //if(s_aRenderStateValue[nStateIndex] == dwValue)
  //  //  m_aChanged[nStateIndex >> 5].dw &= ((0xFFFFFFFE) << (nStateIndex & 31));
  //  //else
  //  //  m_aChanged[nStateIndex >> 5].dw |= (1 << (nStateIndex & 31));

  //  //if(m_bOnDevice == TRUE)
  //  //{
  //  //  m_pGraphics->D3DGetDevice()->SetRenderState(
  //  //    (D3DRENDERSTATETYPE)eType, dwValue);
  //  //}
  //  return TRUE;
  //}
















  STATIC_ASSERT(GXSTENCILOP_KEEP    == D3D11_STENCIL_OP_KEEP);
  STATIC_ASSERT(GXSTENCILOP_ZERO    == D3D11_STENCIL_OP_ZERO);
  STATIC_ASSERT(GXSTENCILOP_REPLACE == D3D11_STENCIL_OP_REPLACE);
  STATIC_ASSERT(GXSTENCILOP_INCRSAT == D3D11_STENCIL_OP_INCR_SAT);
  STATIC_ASSERT(GXSTENCILOP_DECRSAT == D3D11_STENCIL_OP_DECR_SAT);
  STATIC_ASSERT(GXSTENCILOP_INVERT  == D3D11_STENCIL_OP_INVERT);
  STATIC_ASSERT(GXSTENCILOP_INCR    == D3D11_STENCIL_OP_INCR);
  STATIC_ASSERT(GXSTENCILOP_DECR    == D3D11_STENCIL_OP_DECR);

  
  
  
  
  
  
  
  STATIC_ASSERT(GXCMP_NEVER        == D3D11_COMPARISON_NEVER);
  STATIC_ASSERT(GXCMP_LESS         == D3D11_COMPARISON_LESS);
  STATIC_ASSERT(GXCMP_EQUAL        == D3D11_COMPARISON_EQUAL);
  STATIC_ASSERT(GXCMP_LESSEQUAL    == D3D11_COMPARISON_LESS_EQUAL);
  STATIC_ASSERT(GXCMP_GREATER      == D3D11_COMPARISON_GREATER);
  STATIC_ASSERT(GXCMP_NOTEQUAL     == D3D11_COMPARISON_NOT_EQUAL);
  STATIC_ASSERT(GXCMP_GREATEREQUAL == D3D11_COMPARISON_GREATER_EQUAL);
  STATIC_ASSERT(GXCMP_ALWAYS       == D3D11_COMPARISON_ALWAYS);
  

  STATIC_ASSERT(GXBLEND_ZERO               == D3D11_BLEND_ZERO);
  STATIC_ASSERT(GXBLEND_ONE                == D3D11_BLEND_ONE);
  STATIC_ASSERT(GXBLEND_SRCCOLOR           == D3D11_BLEND_SRC_COLOR);
  STATIC_ASSERT(GXBLEND_INVSRCCOLOR        == D3D11_BLEND_INV_SRC_COLOR);
  STATIC_ASSERT(GXBLEND_SRCALPHA           == D3D11_BLEND_SRC_ALPHA);
  STATIC_ASSERT(GXBLEND_INVSRCALPHA        == D3D11_BLEND_INV_SRC_ALPHA);
  STATIC_ASSERT(GXBLEND_DESTALPHA          == D3D11_BLEND_DEST_ALPHA);
  STATIC_ASSERT(GXBLEND_INVDESTALPHA       == D3D11_BLEND_INV_DEST_ALPHA);
  STATIC_ASSERT(GXBLEND_DESTCOLOR          == D3D11_BLEND_DEST_COLOR);
  STATIC_ASSERT(GXBLEND_INVDESTCOLOR       == D3D11_BLEND_INV_DEST_COLOR);
  STATIC_ASSERT(GXBLEND_SRCALPHASAT        == D3D11_BLEND_SRC_ALPHA_SAT);
  STATIC_ASSERT(GXBLEND_BOTHSRCALPHA       == 12);
  STATIC_ASSERT(GXBLEND_BOTHINVSRCALPHA    == 13);
  STATIC_ASSERT(GXBLEND_BLENDFACTOR        == D3D11_BLEND_BLEND_FACTOR);
  STATIC_ASSERT(GXBLEND_INVBLENDFACTOR     == D3D11_BLEND_INV_BLEND_FACTOR);
  STATIC_ASSERT(GXBLEND_SRCCOLOR2          == D3D11_BLEND_SRC1_COLOR);
  STATIC_ASSERT(GXBLEND_INVSRCCOLOR2       == D3D11_BLEND_INV_SRC1_COLOR);

  STATIC_ASSERT(GXBLENDOP_ADD              == D3D11_BLEND_OP_ADD);
  STATIC_ASSERT(GXBLENDOP_SUBTRACT         == D3D11_BLEND_OP_SUBTRACT);
  STATIC_ASSERT(GXBLENDOP_REVSUBTRACT      == D3D11_BLEND_OP_REV_SUBTRACT);
  STATIC_ASSERT(GXBLENDOP_MIN              == D3D11_BLEND_OP_MIN);
  STATIC_ASSERT(GXBLENDOP_MAX              == D3D11_BLEND_OP_MAX);

  STATIC_ASSERT(GXFILL_WIREFRAME  == D3D11_FILL_WIREFRAME);
  STATIC_ASSERT(GXFILL_SOLID      == D3D11_FILL_SOLID);
  
  STATIC_ASSERT(GXCULL_NONE       == D3D11_CULL_NONE);
  STATIC_ASSERT(GXCULL_CW         == D3D11_CULL_FRONT);
  STATIC_ASSERT(GXCULL_CCW        == D3D11_CULL_BACK);



  //GXDWORD GRenderState::Get(GXRenderStateType eType)
  //{
  //  const GXINT nStateIndex = s_aEnumToIdx[eType];
  //  return m_aRenderStateValue[nStateIndex];
  //}

  //GXBOOL GRenderState::SetBlock(GXLPCRENDERSTATE lpBlock)
  //{
  //  int i = 0;
  //  UINT bUpdateBlendState = 0;
  //  UINT bUpdateDepthStencilState = 0;
  //  UINT bUpdateRasterizer = 0;

  //  while(lpBlock[i].dwType != (GXRenderStateType)0)
  //  {
  //    const GXRenderStateType eType = lpBlock[i].dwType;
  //    const GXDWORD dwValue = lpBlock[i].dwValue;
  //  //  const GXRenderStateType  eType    = lpBlock[i].dwType;
  //  //  const GXDWORD      dwValue    = lpBlock[i].dwValue;
  //  //  const GXINT        nStateIndex = s_aEnumToIdx[eType];
  //  //  i++;

  //  //  if(m_aRenderStateValue[nStateIndex] == dwValue)
  //  //    continue;

  //  //  m_aRenderStateValue[nStateIndex] = dwValue;
  //  //  if(s_aRenderStateValue[nStateIndex] == dwValue)
  //  //    m_aChanged[nStateIndex >> 5].dw &= ((0xFFFFFFFE) << (nStateIndex & 31));
  //  //  else
  //  //    m_aChanged[nStateIndex >> 5].dw |= (1 << (nStateIndex & 31));

  //  //  if(m_bOnDevice == TRUE)
  //  //    m_pGraphics->D3DGetDevice()->SetRenderState(
  //  //    (D3DRENDERSTATETYPE)eType, dwValue);
  //    //Set(lpBlock[i].dwType, lpBlock[i].dwValue);

  //    switch(eType)
  //    {
  //    //case GXRS_BLENDFACTOR:
  //    //  m_BlendFactor = dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_ALPHABLENDENABLE:
  //    //  m_BlendState.RenderTarget[0].BlendEnable = dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_DESTBLEND:
  //    //  m_BlendState.RenderTarget[0].DestBlend = (D3D11_BLEND)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_SRCBLEND:
  //    //  m_BlendState.RenderTarget[0].SrcBlend = (D3D11_BLEND)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_DESTBLENDALPHA:
  //    //  m_BlendState.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_SRCBLENDALPHA:
  //    //  m_BlendState.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_BLENDOP:
  //    //  m_BlendState.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_BLENDOPALPHA:
  //    //  m_BlendState.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_COLORWRITEENABLE:
  //    //  m_BlendState.RenderTarget[0].RenderTargetWriteMask = (UINT8)dwValue;
  //    //  bUpdateBlendState = TRUE;
  //    //  break;
  //    //case GXRS_ZENABLE:
  //    //  m_DepthStencilDesc.DepthEnable = dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_ZFUNC:
  //    //  m_DepthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_ZWRITEENABLE:
  //    //  if(dwValue)
  //    //    m_DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  //    //  else
  //    //    m_DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILENABLE:
  //    //  m_DepthStencilDesc.StencilEnable = dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILWRITEMASK:
  //    //  m_DepthStencilDesc.StencilWriteMask = (UINT8)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILMASK:
  //    //  m_DepthStencilDesc.StencilReadMask = (UINT8)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILFUNC:
  //    //  m_DepthStencilDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)dwValue;
  //    //  m_DepthStencilDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILPASS:
  //    //  m_DepthStencilDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)dwValue;
  //    //  m_DepthStencilDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILFAIL:
  //    //  m_DepthStencilDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)dwValue;
  //    //  m_DepthStencilDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILZFAIL:
  //    //  m_DepthStencilDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)dwValue;
  //    //  m_DepthStencilDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)dwValue;
  //    //  bUpdateDepthStencilState = TRUE;
  //    //  break;
  //    //case GXRS_STENCILREF:
  //    //  m_dwStencilRef = dwValue; // 这个不需要销毁DepthStencil对象
  //    //  break;
  //    case GXRS_FILLMODE:
  //      m_RasterizerDesc.FillMode = (D3D11_FILL_MODE)dwValue;
  //      bUpdateRasterizer = TRUE;
  //      break;
  //    case GXRS_CULLMODE:
  //      m_RasterizerDesc.CullMode = (D3D11_CULL_MODE)dwValue;
  //      bUpdateRasterizer = TRUE;
  //      break;
  //    case GXRS_DEPTHBIAS:
  //      //ASSERT(0); // TODO: 没看实现细节,用到了补上
  //      m_RasterizerDesc.DepthBias = 0;
  //      //CLOG_WARNING(__FUNCTION__" Warning, 没看实现细节,用到了补上.\n");
  //      break;
  //    case GXRS_SLOPESCALEDEPTHBIAS:
  //      m_RasterizerDesc.SlopeScaledDepthBias = *(float*)&dwValue;
  //      bUpdateRasterizer = TRUE;
  //      break;
  //    case GXRS_SCISSORTESTENABLE:
  //      m_RasterizerDesc.ScissorEnable = dwValue;
  //      bUpdateRasterizer = TRUE;
  //      break;
  //    case GXRS_ANTIALIASEDLINEENABLE:
  //      m_RasterizerDesc.AntialiasedLineEnable = dwValue;
  //      bUpdateRasterizer = TRUE;
  //      break;
  //    default:
  //      __asm nop
  //      break;
  //    }

  //    i++;
  //  }
  //  //if(bUpdateBlendState) {
  //  //  SAFE_RELEASE(m_pBlendState);
  //  //}
  //  //if(bUpdateDepthStencilState) {
  //  //  SAFE_RELEASE(m_pDepthStencilState);
  //  //}
  //  if(bUpdateRasterizer) {
  //    SAFE_RELEASE(m_pRasterizerState);
  //  }
  //  return TRUE;
  //}

  //GXBOOL GRenderState::IntCheckUpdate()
  //{
  //  ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
  //  ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();

  //  if(m_pRasterizerState == NULL)
  //  {
  //    pd3dDevice->CreateRasterizerState(&m_RasterizerDesc, &m_pRasterizerState);
  //    pImmediateContext->RSSetState(m_pRasterizerState);
  //  }

  //  //if(m_pDepthStencilState == NULL)
  //  //{
  //  //  pd3dDevice->CreateDepthStencilState(&m_DepthStencilDesc, &m_pDepthStencilState);
  //  //  pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_dwStencilRef);
  //  //}

  //  //if(m_pBlendState == NULL) {
  //  //  memcpy(&m_BlendState.RenderTarget[1], &m_BlendState.RenderTarget[0], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
  //  //  memcpy(&m_BlendState.RenderTarget[2], &m_BlendState.RenderTarget[0], sizeof(D3D11_RENDER_TARGET_BLEND_DESC) * 2);
  //  //  memcpy(&m_BlendState.RenderTarget[4], &m_BlendState.RenderTarget[0], sizeof(D3D11_RENDER_TARGET_BLEND_DESC) * 4);

  //  //  pd3dDevice->CreateBlendState(&m_BlendState, &m_pBlendState);

  //  //  pImmediateContext->OMSetBlendState(m_pBlendState, (FLOAT*)&m_BlendFactor, 0xffffffff);
  //  //}
  //  return TRUE;
  //}
  //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GRasterizerStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GRasterizerStateImpl::Release()
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

  GRasterizerStateImpl::GRasterizerStateImpl(GXGraphicsImpl* pGraphicsImpl)
    : GRasterizerState()
    , m_pGraphicsImpl (pGraphicsImpl)
    , m_pRasterizerState(NULL)
  {
    InlSetZeroT(m_RasterizerDesc);
  }

  GXBOOL GRasterizerStateImpl::Initialize(GXRASTERIZERDESC* pDesc)
  {   
    if(pDesc->cbSize != sizeof(GXRASTERIZERDESC)) {
      return FALSE;
    }

    //INT DepthBias;
    ASSERT(pDesc->DepthBias == 0); // 还不知道怎么转换

    m_RasterizerDesc.FillMode = (D3D11_FILL_MODE)pDesc->FillMode;
    m_RasterizerDesc.CullMode = (D3D11_CULL_MODE)pDesc->CullMode;
    m_RasterizerDesc.FrontCounterClockwise = pDesc->FrontCounterClockwise;
    m_RasterizerDesc.DepthBiasClamp        = pDesc->DepthBiasClamp;
    m_RasterizerDesc.SlopeScaledDepthBias  = pDesc->SlopeScaledDepthBias;
    m_RasterizerDesc.DepthClipEnable       = pDesc->DepthClipEnable;
    m_RasterizerDesc.ScissorEnable         = pDesc->ScissorEnable;

    ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    if(GXFAILED(pd3dDevice->CreateRasterizerState(&m_RasterizerDesc, &m_pRasterizerState))) {
      CLOG_ERROR("RasterizerState: Error to create d3d11 rasterizer state.\n");
      return FALSE;
    }
    return TRUE;
  }

  GXBOOL GRasterizerStateImpl::Activate(GRasterizerStateImpl* pPrevState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveRasterizerState(this));
    InlSetRasterizerState();
    return TRUE;
  }

  inline void GRasterizerStateImpl::InlSetRasterizerState()
  {
    ID3D11DeviceContext* pd3dContext = m_pGraphicsImpl->D3DGetDeviceContext();
    //GXColor crBlendFactor = m_BlendFactor;
    pd3dContext->RSSetState(m_pRasterizerState);
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GBlendStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GBlendStateImpl::Release()
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

  GBlendStateImpl::GBlendStateImpl(GXGraphicsImpl* pGraphicsImpl)
    : GBlendState     ()
    , m_pGraphicsImpl (pGraphicsImpl)
    , m_BlendFactor   (-1)
    , m_pBlendState   (NULL)
  {
    InlSetZeroT(m_BlendDesc);
  }

  GXBOOL GBlendStateImpl::Initialize(GXBLENDDESC* pDesc, GXUINT nNum)
  {
    // 状态合法性检查
    if(pDesc == NULL || nNum > 8 || nNum == 0) {
      CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, "BlendState");
      return FALSE;
    }

    m_BlendDesc.AlphaToCoverageEnable = FALSE;
    m_BlendDesc.IndependentBlendEnable = FALSE;
    for(GXUINT i = 0; i < nNum; i++)
    {
      GXBLENDDESC& Desc = pDesc[i < nNum ? i : nNum - 1];
      D3D11_RENDER_TARGET_BLEND_DESC& RenderTarget = m_BlendDesc.RenderTarget[i];

      RenderTarget.BlendEnable    = Desc.BlendEnable;
      RenderTarget.BlendOp        = (D3D11_BLEND_OP)Desc.BlendOp;
      RenderTarget.BlendOpAlpha   = (D3D11_BLEND_OP)Desc.BlendOpAlpha;
      RenderTarget.DestBlend      = (D3D11_BLEND)Desc.DestBlend;
      RenderTarget.DestBlendAlpha = (D3D11_BLEND)Desc.DestBlendAlpha;
      RenderTarget.SrcBlend       = (D3D11_BLEND)Desc.SrcBlend;
      RenderTarget.SrcBlendAlpha  = (D3D11_BLEND)Desc.SrcBlendAlpha;
      RenderTarget.RenderTargetWriteMask = (D3D11_BLEND)Desc.WriteMask;
    }

    ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    if(GXFAILED(pd3dDevice->CreateBlendState(&m_BlendDesc, &m_pBlendState))) {
      CLOG_ERROR("BlendState: Error to create d3d11 blend state.\n");
      return FALSE;
    }
    return TRUE;
  }

  void GBlendStateImpl::InlSetBlendState()
  {
    ID3D11DeviceContext* pd3dContext = m_pGraphicsImpl->D3DGetDeviceContext();
    GXColor crBlendFactor = m_BlendFactor;
    pd3dContext->OMSetBlendState(m_pBlendState, (float*)&crBlendFactor, 0xffffffff);
  }

  GXBOOL GBlendStateImpl::Activate(GBlendStateImpl* pPrevState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveBlendState(this));
    InlSetBlendState();
    return TRUE;
  }

  GXDWORD GBlendStateImpl::SetBlendFactor(GXDWORD dwBlendFactor)
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
  GXHRESULT GDepthStencilStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GDepthStencilStateImpl::Release()
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

  GDepthStencilStateImpl::GDepthStencilStateImpl(GXGraphicsImpl* pGraphicsImpl)
    : GDepthStencilState  ()
    , m_pGraphicsImpl     (pGraphicsImpl)
    , m_StencilRef        (0)
    , m_pDepthStencilState(NULL)
  {
    InlSetZeroT(m_DepthStencilDesc);
  }

  GXBOOL GDepthStencilStateImpl::Initialize(GXDEPTHSTENCILDESC* pDesc)
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
    m_DepthStencilDesc.FrontFace.StencilFunc= (D3D11_COMPARISON_FUNC)pDesc->FrontFace.StencilFunc;

    m_DepthStencilDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilFailOp;
    m_DepthStencilDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilDepthFailOp;
    m_DepthStencilDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)pDesc->BackFace.StencilPassOp;
    m_DepthStencilDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)pDesc->BackFace.StencilFunc;

    ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    HRESULT hval = pd3dDevice->CreateDepthStencilState(&m_DepthStencilDesc, &m_pDepthStencilState);
    return SUCCEEDED(hval);
  }

  GXBOOL GDepthStencilStateImpl::Activate(GDepthStencilStateImpl* pPrevState)
  {
    ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
    pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
    return TRUE;
  }

  GXDWORD GDepthStencilStateImpl::SetStencilRef(GXDWORD dwStencilRef)
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

  GSamplerStateImpl::~GSamplerStateImpl()
  {
    for(UINT i = 0; i < SAMPLERCOUNT; i++) {
      SAFE_RELEASE(m_pSampler[i]);
    }
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GSamplerStateImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GSamplerStateImpl::Release()
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

  //ID3D11SamplerState** GSamplerStateImpl::GetSamplers()
  //{
  //  //if(m_dwChangeMask != NULL)
  //  //{
  //  //  for(UINT i = 0; i < SAMPLERCOUNT; i++)
  //  //  {
  //  //    if(m_SamplerStage[i].dwMask != 0)
  //  //    {
  //  //      D3D11BuildSampler(i);
  //  //      ASSERT((m_SamplerStage[i].dwMask & 0xffff) == 0);
  //  //    }
  //  //  }
  //  //  ASSERT((m_dwChangeMask & 0xffff) == 0);
  //  //}

  //  // TODO: 重新实现!
  //  CLBREAK;
  //  return m_pSampler;
  //}

  //void GSamplerState::SetStageToDevice(DWORD dwStage)
  //{
  //  //LPDIRECT3DDEVICE9 const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
  //  //GXSAMPLERSTAGE& SamplerStage = m_SamplerStage[dwStage];

  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_ADDRESSU,      SamplerStage.dwAddressU);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_ADDRESSV,      SamplerStage.dwAddressV);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_ADDRESSW,      SamplerStage.dwAddressW);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_BORDERCOLOR,   SamplerStage.dwBorderColor);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MAGFILTER,     SamplerStage.dwMagFilter);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MINFILTER,     SamplerStage.dwMinFilter);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MIPFILTER,     SamplerStage.dwMipFilter);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MIPMAPLODBIAS, SamplerStage.dwMipmapLodBias);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MAXMIPLEVEL,   SamplerStage.dwMaxMipLevel);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_MAXANISOTROPY, SamplerStage.dwMaxAnisotropy);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_SRGBTEXTURE,   SamplerStage.dwSRGBTexture);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_ELEMENTINDEX,  SamplerStage.dwElementIndex);
  //  //pd3dDevice->SetSamplerState(dwStage, D3DSAMP_DMAPOFFSET,    SamplerStage.dwDMapOffset);
  //}

  //void GSamplerState::SetStateToDevice(DWORD dwStage, GXSamplerStateType eType)
  //{
  //  //LPDIRECT3DDEVICE9 const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
  //  //HRESULT hval = pd3dDevice->SetSamplerState(dwStage, (D3DSAMPLERSTATETYPE)eType, m_SamplerStage[dwStage].m[eType]);
  //  //ASSERT(SUCCEEDED(hval));
  //}

  GSamplerStateImpl::GSamplerStateImpl(::GXGraphics* pGraphics)
    : GSamplerState   ()
    , m_pGraphicsImpl (static_cast<GXGraphicsImpl*>(pGraphics))
    //, m_dwChangeMask  (0xffff)
  {
    //ResetToDefault();
    //memset(m_pSampler, 0, sizeof(ID3D11SamplerState*) * SAMPLERCOUNT);
    InlSetZeroT(m_pSampler);
    InlSetZeroT(m_SamplerDesc);
  }

  GXBOOL GSamplerStateImpl::InitializeStatic()
  {
    //IntSetSamplerToDefault(&s_DefaultSamplerState);
    return TRUE;
  }

  GXBOOL GSamplerStateImpl::Initialize(GSamplerStateImpl* pDefault)
  {
    ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    //ID3D11DeviceContext* const pd3dDeviceContext = m_pGraphicsImpl->D3DGetDeviceContext();
    if(pDefault == NULL)
    {
      D3D11_SAMPLER_DESC sampler_desc = { D3D11_FILTER_MIN_MAG_MIP_POINT };
      sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
      sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.MipLODBias     = 0.0f;
      sampler_desc.MaxAnisotropy  = 0;
      sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS;
      sampler_desc.BorderColor[0] = 0.0f;
      sampler_desc.BorderColor[1] = 0.0f;
      sampler_desc.BorderColor[2] = 0.0f;
      sampler_desc.BorderColor[3] = 0.0f;
      sampler_desc.MinLOD         = 0.0f;
      sampler_desc.MaxLOD         = D3D11_FLOAT32_MAX;
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

  //GXBOOL GSamplerStateImpl::ResetToDefault()
  //{
  //  //TODO: 这个应该根据当前的状态来设置mask, DX11实现比较特殊

  //  //if(m_pGraphicsImpl->IsActiveSamplerState(this))
  //  //{
  //  //  for(DWORD i = 0; i < SAMPLERCOUNT; i++) {
  //  //    // 如果已经是默认值就跳过
  //  //    if((m_SamplerStage[i].dwMask & 0xffff) == 0) {
  //  //      continue;
  //  //    }

  //  //    ::IntSetSamplerToDefault(&m_SamplerStage[i]);
  //  //    SetStageToDevice(i);
  //  //  }
  //  //}
  //  //else {
  //    for(DWORD i = 0; i < SAMPLERCOUNT; i++) {
  //  //    // 如果已经是默认值就跳过
  //  //    if((m_SamplerStage[i].dwMask & 0xffff) == 0) {
  //  //      continue;
  //  //    }

  //      IntSetSamplerToDefault(&m_SamplerStage[i]);
  //      m_SamplerStage[i].dwMask = 0xffff;  // 这个Mask含义和D3D9实现里的不同!
  //    }
  //  //}
  //  return TRUE;
  //}

  GXBOOL GSamplerStateImpl::Activate(GSamplerStateImpl* pPrevSamplerState)
  {
    ASSERT(m_pGraphicsImpl->InlIsActiveSamplerState(this)); // 确定已经放置到Graphics上

    ID3D11DeviceContext* const pd3dDeviceContext = m_pGraphicsImpl->D3DGetDeviceContext();
    pd3dDeviceContext->PSSetSamplers(0, SAMPLERCOUNT, m_pSampler);

    //if(pPrevSamplerState != NULL)
    //{
    //  ASSERT(this != pPrevSamplerState); // 外面保证这个

    //  for(int i = 0; i < SAMPLERCOUNT; i++)
    //  {
    //    GXSAMPLERSTAGE& PrevSamp = pPrevSamplerState->m_SamplerStage[i];
    //    GXSAMPLERSTAGE& ThisSamp = m_SamplerStage[i];

    //    // 如果相等则跳过
    //    if((PrevSamp.dwMask & 0xffff) != (ThisSamp.dwMask & 0xffff)) {
    //      SetStageToDevice(i);
    //    }
    //  }
    //}
    //else {
    //for(int i = 0; i < SAMPLERCOUNT; i++)
    //{
    //  SetStageToDevice(i);
    //}
    //}

    return TRUE;
  }

  //GXBOOL GSamplerStateImpl::Set(GXDWORD Sampler, GXSamplerStateType eType, GXDWORD dwValue)
  //{
  //  ASSERT(Sampler < SAMPLERCOUNT && eType >= 1 && eType <= 13);
  //  GXSAMPLERSTAGE& Samp = m_SamplerStage[Sampler];
  //  
  //  if(Samp.m[eType] != dwValue)
  //  {
  //    Samp.m[eType] = dwValue;
  //    SET_FLAG(Samp.dwMask, 1 << eType);
  //    SET_FLAG(m_dwChangeMask, 1 << Sampler);
  //    //if(m_pGraphicsImpl->IsActiveSamplerState(this)) {
  //    //  SetStateToDevice(Sampler, eType);
  //    //}
  //  }

  //  //// 如果是默认值则复位对应位
  //  //if(s_DefaultSamplerState.m[eType] == dwValue) {
  //  //  RESET_FLAG(Samp.dwMask, 1 << eType);
  //  //}
  //  //else {
  //  //  SET_FLAG(Samp.dwMask, 1 << eType);
  //  //}

  //  //if(m_pGraphicsImpl->IsActiveSamplerState(this)) {
  //  //  SetStateToDevice(Sampler, eType);
  //  //}
  //  return TRUE;
  //}

  //GXDWORD GSamplerStateImpl::Get(GXDWORD Sampler, GXSamplerStateType eType)
  //{
  //  ASSERT(Sampler < SAMPLERCOUNT);
  //  return m_SamplerStage[Sampler].m[eType];
  //}
  //
  GXHRESULT GSamplerStateImpl::SetState(GXUINT Sampler, GXSAMPLERDESC* pSamplerDesc)
  {
    ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();    
    GXSAMPLERDESC& SamplerDesc = m_SamplerDesc[Sampler];

    D3D11_SAMPLER_DESC SampDesc11;
    InlSetZeroT(SampDesc11);

    SamplerDesc = *pSamplerDesc;
    GXColor crBorder = SamplerDesc.BorderColor;

    SampDesc11.Filter = GrapXToDX11::FilterFrom((GXTextureFilterType)SamplerDesc.MagFilter, 
      (GXTextureFilterType)SamplerDesc.MinFilter, (GXTextureFilterType)SamplerDesc.MipFilter);

    SampDesc11.AddressU       = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressU;
    SampDesc11.AddressV       = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressV;
    SampDesc11.AddressW       = (D3D11_TEXTURE_ADDRESS_MODE)SamplerDesc.AddressW;
    //sampDesc.MipLODBias     = SamplerStage.dwMipmapLodBias; // FIXME: 类型不对
    //sampDesc.MaxAnisotropy  = SamplerDesc.MaxAnisotropy;
    SampDesc11.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SampDesc11.BorderColor[0] = crBorder.r; // FIXME: 顺序没有验证
    SampDesc11.BorderColor[1] = crBorder.g;
    SampDesc11.BorderColor[2] = crBorder.b;
    SampDesc11.BorderColor[3] = crBorder.a;
    SampDesc11.MinLOD         = 0;
    SampDesc11.MaxLOD         = D3D11_FLOAT32_MAX;

    // TODO: 可以提取一个特征值,从一个池中查询
    SAFE_RELEASE(m_pSampler[Sampler]);
    HRESULT hr = pd3dDevice->CreateSamplerState(&SampDesc11, &m_pSampler[Sampler]);
    if( FAILED( hr ) ) {
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

  GXHRESULT GSamplerStateImpl::SetStateArray(GXUINT nStartSlot, GXSAMPLERDESC* pSamplerDesc, int nCount)
  {
    CLBREAK;
    return GX_FAIL;
  }

  GXHRESULT GSamplerStateImpl::ResetToDefault()
  {
    //CLBREAK;
    return GX_FAIL;
  }

} // namespace D3D11
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)