#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_EFFECT_D3D11_
#define _GXGRAPHICS_INLINE_SHADER_D3D11_
#define _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D11_
#define _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_
#define _GXGRAPHICS_INLINE_SET_BLEND_STATE_
#define _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_
#define _GXGRAPHICS_INLINE_SET_RENDER_STATE_
#define _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/GRegion.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXRenderTarget.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GShaderImpl_D3D11.h"
#include "Platform/Win32_D3D11/GStateBlock_D3D11.h"
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Canvas/GXEffectImpl.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "Platform/Win32_D3D11/GTextureImpl_D3D11.h"
#include "Platform/Win32_D3D11/GXRenderTargetImpl_D3D11.h"

// 私有头文件
#include <clUtility.h>
#include <GrapX/VertexDecl.h>
#include "Platform/Win32_D3D11/GXCanvasImpl_D3D11.h"
#include "GrapX/GXUser.h"
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>
#include <GrapX/GCamera.h>

#ifdef ENABLE_GRAPHICS_API_DX11
#define TRACE_CMD

// 根据RenderTarget格式改成对应转换函数
#define COLORREF_TO_NATIVE(CLR) ((CLR & 0xff00ff00) | ((CLR & 0xff0000) >> 16) | ((CLR & 0xff) << 16))
#define NATIVE_TO_COLORREF(CLR) COLORREF_TO_NATIVE(CLR)


//////////////////////////////////////////////////////////////////////////
namespace GrapX
{
  namespace D3D11
  {
#include "Canvas/GXCanvas_Text.inl"
#define D3D11_CANVAS_IMPL
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Canvas/GXCanvasCoreImpl.inl"


//////////////////////////////////////////////////////////////////////////
    CanvasImpl::CanvasImpl(GraphicsImpl* pGraphics, GXBOOL bStatic)
      : CanvasCoreImpl    (pGraphics, 2, RESTYPE_CANVAS2D)
      , m_bStatic           (bStatic)
      , m_pTargetImage      (NULL)
      , m_xAbsOrigin        (0)
      , m_yAbsOrigin        (0)
      , m_pInvertState      (NULL)
      , m_pPrimitive        (NULL)
      , m_lpLockedVertex    (NULL)
      , m_lpLockedIndex     (NULL)
      , m_pWhiteTex         (NULL)
      , m_pLastCommand      (NULL)
      , m_uIndexCount       (NULL)
      , m_uVertCount        (0)
      , m_uVertIndexSize    (0)
      , m_dwStencil         (0)
      , m_pClipRegion       (NULL)
      , m_dwTexVertColor    (-1)
      , m_dwColorAdditive   (0)
      , m_dwTexSlot         (NULL)
      , m_pRasterizerState  (NULL)
    {
      m_Commands.Reserve(1024);
      InlSetZeroT(m_rcClip);
      InlSetZeroT(m_rcAbsClip);
      gxRtlZeroMemory(&m_aTextureStage, sizeof(m_aTextureStage));
      gxRtlZeroMemory(&m_pBlendingState, sizeof(m_pBlendingState));
      gxRtlZeroMemory(&m_pOpaqueState, sizeof(m_pOpaqueState));
      gxRtlZeroMemory(&m_pCanvasStencil, sizeof(m_pCanvasStencil));
      m_CommonEffect.pEffect = NULL;
    }

    CanvasImpl::~CanvasImpl()
    {
    }

    GXBOOL CanvasImpl::Initialize(RenderTarget* pTarget, const REGN* pRegn)
    {
      ASSERT(m_Commands.GetSize() == 0);

      if(CanvasCoreImpl::Initialize(pTarget) == TRUE)
      {
        if(pRegn != NULL)
        {
          GXRECT rcTexture;

          m_xAbsOrigin = m_CallState.origin.x = m_rcClip.left = pRegn->left;
          m_yAbsOrigin = m_CallState.origin.y = m_rcClip.top = pRegn->top;
          m_rcClip.right = pRegn->left + pRegn->width;
          m_rcClip.bottom = pRegn->top + pRegn->height;

          gxSetRect(&rcTexture, 0, 0, m_sExtent.cx, m_sExtent.cy);
          gxIntersectRect(&m_rcClip, &rcTexture, &m_rcClip);
        }
        else
        {
          m_xAbsOrigin = m_CallState.origin.x = m_rcClip.left = 0;
          m_yAbsOrigin = m_CallState.origin.y = m_rcClip.top = 0;
          m_rcClip.right = m_sExtent.cx;
          m_rcClip.bottom = m_sExtent.cy;
        }

        m_rcAbsClip = m_rcClip;
        m_CallState.rcClip = m_rcClip;
        m_CallState.RenderState.pEffect = m_pEffectImpl;

        m_uVertIndexSize = s_uDefVertIndexSize;

        if(m_pPrimitive == NULL) {
          m_pGraphics->CreatePrimitive(&m_pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P4T2F_C1D),
            GXResUsage::Write,
            m_uVertIndexSize, sizeof(CANVAS_PRMI_VERT), NULL,
            m_uVertIndexSize, 2, NULL);
        }

        if(m_pCamera == NULL) {
          m_pCamera = GCamera_ScreenAligned::Create((CanvasCore*)(Canvas*)this);
        }

        // 初始化渲染模式
        m_CallState.eCompMode = CompositingMode_SourceOver;
        m_CallState.dwColorAdditive = 0;
        STATESWITCHING_COMPOSITINGMODE* pCompositingModeCmd = IntAppendCommand<STATESWITCHING_COMPOSITINGMODE>(CF_CompositingMode);
        pCompositingModeCmd->mode = CompositingMode_SourceOver;
        m_dwTexVertColor = (GXDWORD)-1;
        m_dwColorAdditive = 0;
        m_eStyle = PS_Solid;

        // 初始化空纹理时的替换纹理
        if(m_pWhiteTex == NULL) {
          m_pGraphics->CreateTexture(&m_pWhiteTex, "White8x8", 0, 0, Format_Unknown, GXResUsage::Default, 0, NULL, 0); // 查询
        }

        // 创建BlendState
        if(m_pBlendingState[0] == NULL || m_pBlendingState[1] == NULL ||
          m_pOpaqueState[0] == NULL || m_pOpaqueState[1] == NULL)
        {
          GXBLENDDESC AlphaBlendState(TRUE);
          GXBLENDDESC OpaqueState;

          ASSERT(m_pBlendingState[0] == NULL);
          ASSERT(m_pBlendingState[1] == NULL);
          ASSERT(m_pOpaqueState[0] == NULL);
          ASSERT(m_pOpaqueState[1] == NULL);

          // ---
          GXRASTERIZERDESC RasterizerDesc;
          RasterizerDesc.ScissorEnable = TRUE;
          m_pGraphics->CreateRasterizerState((RasterizerState**)&m_pRasterizerState, &RasterizerDesc);

          // ---
          m_pGraphics->CreateBlendState((BlendState**)&m_pBlendingState[0], &AlphaBlendState, 1);

          // ---
          AlphaBlendState.SeparateAlphaBlend = TRUE;
          AlphaBlendState.BlendOpAlpha = GXBLENDOP_ADD;
          AlphaBlendState.SrcBlendAlpha = GXBLEND_INVDESTALPHA;
          AlphaBlendState.DestBlendAlpha = GXBLEND_ONE;
          m_pGraphics->CreateBlendState((BlendState**)&m_pBlendingState[1], &AlphaBlendState, 1);

          // ---
          m_pGraphics->CreateBlendState((BlendState**)&m_pOpaqueState[0], &OpaqueState, 1);

          // ---
          OpaqueState.BlendEnable = TRUE;
          m_pGraphics->CreateBlendState((BlendState**)&m_pOpaqueState[1], &OpaqueState, 1);
        }

        if(m_pInvertState == NULL)
        {
          GXBLENDDESC sInvertBlend(TRUE, GXBLEND_ONE, GXBLEND_ONE, GXBLENDOP_SUBTRACT);
          m_pGraphics->CreateBlendState((BlendState**)&m_pInvertState, &sInvertBlend, 1);
        }

        if(m_pCanvasStencil[0] == NULL || m_pCanvasStencil[1] == NULL)
        {
          ASSERT(m_pCanvasStencil[0] == NULL && m_pCanvasStencil[1] == NULL);
          GXDEPTHSTENCILDESC DepthStencil(FALSE, FALSE);
          m_pGraphics->CreateDepthStencilState((DepthStencilState**)&m_pCanvasStencil[0], &DepthStencil);

          DepthStencil.StencilEnable = TRUE;
          DepthStencil.StencilReadMask = 0xff;
          DepthStencil.StencilWriteMask = 0xff;
          DepthStencil.FrontFace.StencilFunc = GXCMP_EQUAL;
          m_pGraphics->CreateDepthStencilState((DepthStencilState**)&m_pCanvasStencil[1], &DepthStencil);
        }

        // 初始化寄存器常量
        GCAMERACONETXT gcc;
        gcc.dwMask = GCC_WORLD;
        m_pCamera->GetContext(&gcc);
        m_CallState.matTransform = gcc.matWorld;

        ASSERT(m_CommonEffect.pEffect == NULL);
        m_CommonEffect.pEffect = m_pGraphics->IntGetEffect();
        m_CommonEffect.pEffect->AddRef();

        m_CommonEffect.transform = m_pEffectImpl->GetUniform("matWVProj").CastTo<MOVarMatrix4>();
        m_CommonEffect.color     = m_pEffectImpl->GetUniform("Color").CastTo<MOVarFloat4>();
        m_CommonEffect.color_add = m_pEffectImpl->GetUniform("ColorAdd").CastTo<MOVarFloat4>();

        m_CommonEffect.transform = gcc.matWorld;
        m_CommonEffect.color->set(1.0f);
        m_CommonEffect.color_add->set(0,0,0,1);

        ASSERT(m_dwTexSlot == NULL);
        return TRUE;
      }
      return FALSE;
    }

    GXINT CanvasImpl::UpdateStencil(GRegion* pClipRegion)
    {
      if(pClipRegion != m_pClipRegion)
      {
        SAFE_RELEASE(m_pClipRegion);
        m_pClipRegion = pClipRegion;
      }

      RGNCOMPLEX eCompx = RC_SIMPLE;
      GXRECT rcClip;
      GXREGN rgClip;

      if(m_pClipRegion == NULL)
      {
        rcClip = m_rcAbsClip;
        m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
      }
      else
      {
        eCompx = m_pClipRegion->GetComplexity();
        switch(eCompx)
        {
        case RC_ERROR:
          rcClip = m_rcAbsClip;
          m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
          break;
        case RC_NULL:
          m_pClipRegion->GetBounding(&rcClip);
          break;
        case RC_SIMPLE:
          m_pClipRegion->GetBounding(&rcClip);
          m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
          break;
        case RC_COMPLEX:
        {
          const GXUINT nRectCount = m_pClipRegion->GetRectCount();
          clstd::LocalBuffer<sizeof(GXRECT) * 128> _buf;
          _buf.Resize(sizeof(GXRECT) * nRectCount, FALSE);
          GXRECT* lpRects = (GXRECT*)_buf.GetPtr(); // _GlbLockStaticRects(nRectCount);

          m_pClipRegion->GetRects(lpRects, nRectCount);
          m_pClipRegion->GetBounding(&rcClip);

          // 如果不先设置Clear在区域之外的会失败
          gxRectToRegn(&rgClip, &rcClip);
          m_pGraphics->SetSafeClip(&rgClip);

          // TODO: 考虑是否在以后用快速Region求补来填充空白区域呢?
          // 如下填充实现了: 在一个矩形区域内,需要绘制图形的部分模板值>1,其他部分模板值为0
          const GXDWORD dwFlags = GXCLEAR_STENCIL;
          //const GXDWORD dwFlags = GXCLEAR_STENCIL|GXCLEAR_TARGET;
          m_pGraphics->Clear(&rcClip, 1, dwFlags, 0xff0000ff, 0, 0);
          m_pGraphics->Clear(lpRects, nRectCount, dwFlags, 0xff00ff00, 0, m_dwStencil);

          m_pCanvasStencil[1]->SetStencilRef(m_dwStencil);
          m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[1]);
        }
        IntUpdateClip(rcClip);
        return (GXINT)eCompx;
        }
      }
      gxRectToRegn(&rgClip, &rcClip);
      m_pGraphics->SetSafeClip(&rgClip);
      IntUpdateClip(rcClip);
      return (GXINT)eCompx;
    }

    //#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT CanvasImpl::Release()
    {
      if(m_Commands.GetSize() > 0) {
        Flush();
      }

      // 最多两个引用
      ASSERT(m_uRefCount > 0 && m_uRefCount <= 3);
      m_uRefCount--;

      // Graphics 内置 Canvas 在一个引用计数时释放
      // 否则在引用计数为 0 时释放
      if((m_bStatic == TRUE && m_uRefCount == 1) ||
        m_uRefCount == 0)
      {
        SAFE_RELEASE(m_pEffectImpl);
        SAFE_RELEASE(m_pTargetTex);
        SAFE_RELEASE(m_pTargetImage);
        m_dwStencil = 0;

        SAFE_RELEASE(m_pClipRegion);
        SAFE_RELEASE(m_CommonEffect.pEffect);

        if(m_dwTexSlot != NULL)
        {
          ASSERT(m_aTextureStage[0] == NULL);
          for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && m_dwTexSlot != 0; i++)
          {
            if(m_aTextureStage[i] != NULL)
            {
              m_pGraphics->InlSetTexture(NULL, i);
              m_aTextureStage[i]->Release();
              m_aTextureStage[i] = NULL;
              RESETBIT(m_dwTexSlot, i);
            }
          }
        }
        ASSERT(m_dwTexSlot == NULL);

        m_uVertCount = 0;
        m_uIndexCount = 0;
      }

      if(m_uRefCount == 0)
      {
        SAFE_RELEASE(m_pInvertState);
        SAFE_RELEASE(m_pOpaqueState[0]);
        SAFE_RELEASE(m_pOpaqueState[1]);
        SAFE_RELEASE(m_pRasterizerState);
        SAFE_RELEASE(m_pBlendingState[0]);
        SAFE_RELEASE(m_pBlendingState[1]);
        SAFE_RELEASE(m_pCanvasStencil[0]);
        SAFE_RELEASE(m_pCanvasStencil[1]);
        SAFE_RELEASE(m_pWhiteTex);
        SAFE_RELEASE(m_pPrimitive);
        delete this;
        return GX_OK;
      }
      return m_uRefCount;
    }
    //#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXULONG CanvasImpl::GetRef()
    {
      ASSERT(m_uRefCount > 0 && m_uRefCount <= 3);
      return m_uRefCount;
    }

    template<typename _Ty>
    _Ty* CanvasImpl::IntAppendCommand(CanvasFunc cmd)
    {
      if(m_pLastCommand && m_pLastCommand->cmd == cmd) {
        return static_cast<_Ty*>(m_pLastCommand);
      }

      size_t cbFinalSize = m_Commands.GetSize() + sizeof(_Ty);
      if(cbFinalSize > m_Commands.GetCapacity()) {
        cbFinalSize = sizeof(_Ty);
        Flush();
      }

      _Ty* pCmdPtr = reinterpret_cast<_Ty*>(m_Commands.GetEnd());
      m_Commands.Resize(cbFinalSize, FALSE);
      pCmdPtr->cbSize = sizeof(_Ty);
      pCmdPtr->cmd = cmd;

      m_pLastCommand = pCmdPtr;
      return pCmdPtr;
    }

    template<typename _Ty>
    GXUINT CanvasImpl::IntAppendDrawCall(_Ty** ppCmdBuffer, CanvasFunc cmd, GXUINT uVertexCount, GXUINT uIndexCount, Texture* pTextureReference)
    {
      // TODO: 只处理了增量超出的刷新，没有处理填入的顶点索引数超过缓冲区大小的情况
      if(_CL_NOT_(IntCanFillPrimitive(uVertexCount, uIndexCount))) {
        Flush();
      }

      if(m_pLastCommand && m_pLastCommand->cmd == cmd)
      {
        if(cmd != CF_Textured || static_cast<DRAWCALL_TEXTURE*>(m_pLastCommand)->pTexture == pTextureReference)
        {
          DRAWCALLBASE* pLastDrawCall = static_cast<DRAWCALLBASE*>(m_pLastCommand);
          const GXUINT uBaseIndex = pLastDrawCall->uVertexCount;
          pLastDrawCall->uVertexCount += uVertexCount;
          pLastDrawCall->uIndexCount  += uIndexCount;
          *ppCmdBuffer = static_cast<_Ty*>(pLastDrawCall);
          return uBaseIndex;
        }
      }

      *ppCmdBuffer = IntAppendCommand<_Ty>(cmd);
      (*ppCmdBuffer)->uVertexCount = uVertexCount;
      (*ppCmdBuffer)->uIndexCount = uIndexCount;
      if(cmd == CF_Textured) {
        reinterpret_cast<DRAWCALL_TEXTURE*>(*ppCmdBuffer)->pTexture = pTextureReference;
        if(pTextureReference) {
          pTextureReference->AddRef();
        }
      }
      return 0;
    }

    GXBOOL CanvasImpl::IntCanFillPrimitive(GXUINT uVertexCount, GXUINT uIndexCount)
    {
      return ((m_uVertCount + uVertexCount) < m_uVertIndexSize &&
        (m_uIndexCount + uIndexCount) < m_uVertIndexSize);
    }

    void CanvasImpl::SetStencil(GXDWORD dwStencil)
    {
      m_dwStencil = dwStencil;
    }

    void CanvasImpl::IntUpdateClip(const GXRECT& rcClip)
    {
      ASSERT(&m_rcClip != &rcClip);
      m_rcClip = rcClip;
    }

    void CanvasImpl::_SetPrimitivePos(GXUINT nIndex, const GXINT _x, const GXINT _y)
    {
      PRIMITIVE* pVertex = m_lpLockedVertex + nIndex;
      pVertex->x = (float)(m_CallState.origin.x + _x);
      pVertex->y = (float)(m_CallState.origin.y + _y);
    }

    GXBOOL CanvasImpl::CommitState()
    {
      REGN regn;

      // 如果返回0, 说明Graphics使用的就是当前的 Canvas, 所以不用初始化这些东东
      if(m_pGraphics->InlSetCanvas(this) > 0)
      {
        m_pGraphics->SetPrimitive(m_pPrimitive);
        m_pGraphics->InlSetRasterizerState(m_pRasterizerState);
        m_pGraphics->InlSetSamplerState(m_pSamplerState);
        m_pGraphics->InlSetDepthStencil(NULL);

        m_pGraphics->InlSetEffect((EffectImpl*)m_pEffectImpl);
        UpdateStencil(m_pClipRegion);

        gxRectToRegn(&regn, &m_rcClip);
        m_pGraphics->SetSafeClip(&regn);  // TODO: 是不是应该把这个改为GXRECT
        m_pGraphics->SetViewport(NULL);

        float4 vColorAdditive = m_dwColorAdditive;
        m_CommonEffect.color_add = vColorAdditive;

        static_cast<ShaderImpl*>(m_pEffectImpl->GetShaderUnsafe())->CommitConstantBuffer(m_pEffectImpl->GetDataPoolUnsafe()); // TODO: 局部更新


        m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(m_pWhiteTex), 0);

        GXDWORD dwTexSlot = m_dwTexSlot;
        if(dwTexSlot != NULL)
        {
          for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && dwTexSlot != 0; i++)
          {
            if(m_aTextureStage[i] != NULL)
            {
              m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(m_aTextureStage[i]), i);
              RESETBIT(dwTexSlot, i);
            }
          }
        }
      }
      return TRUE;
    }

    Graphics* CanvasImpl::GetGraphicsUnsafe() const
    {
      return m_pGraphics;
    }

    GXBOOL CanvasImpl::SetTransform(const float4x4* matTransform)
    {
      if(matTransform == NULL) {
        return FALSE;
      }

      STATESWITCHING_TRANSFORM* pTransformCmd = IntAppendCommand<STATESWITCHING_TRANSFORM>(CF_SetTransform);
      pTransformCmd->transform = *matTransform;

      m_CallState.matTransform = *matTransform;
      return TRUE;
    }

    GXBOOL CanvasImpl::GetTransform(float4x4* matTransform) const
    {
      *matTransform = m_CallState.matTransform;
      return TRUE;
    }

    GXBOOL CanvasImpl::SetViewportOrg(GXINT x, GXINT y, GXLPPOINT lpPoint)
    {
      // !!! 没测试过
      STATESWITCHING_ORIGIN* pOriginCmd = IntAppendCommand<STATESWITCHING_ORIGIN>(CF_SetViewportOrg);

      if(lpPoint)
      {
        lpPoint->x = m_xAbsOrigin - m_CallState.origin.x;
        lpPoint->y = m_yAbsOrigin - m_CallState.origin.y;
      }

      pOriginCmd->ptOrigin.x = m_CallState.origin.x = m_xAbsOrigin - x;
      pOriginCmd->ptOrigin.y = m_CallState.origin.y = m_yAbsOrigin - y;

      return TRUE;
    }

    GXBOOL CanvasImpl::GetViewportOrg(GXLPPOINT lpPoint) const
    {
      if(lpPoint == NULL)
        return FALSE;

      lpPoint->x = m_xAbsOrigin - m_CallState.origin.x;
      lpPoint->y = m_yAbsOrigin - m_CallState.origin.y;

      return TRUE;
    }

    GXBOOL CanvasImpl::Flush()
    {
      if(m_Commands.GetSize() == 0)
        return FALSE;

      GXBOOL bEmptyRect = gxIsRectEmpty(&m_rcClip);

      if(m_lpLockedVertex != NULL)
      {
        m_pPrimitive->UnmapVertexBuffer(m_lpLockedVertex);
        m_lpLockedVertex = NULL;
      }

      if(m_lpLockedIndex)
      {
        m_pPrimitive->UnmapIndexBuffer(m_lpLockedIndex);
        m_lpLockedIndex = NULL;
      }

      m_pGraphics->Enter();
      CommitState();  // TODO: 如果 bEmptyRect 为 TRUE, 则不提交状态

      GXUINT nBaseVertex = 0;
      GXUINT nStartIndex = 0;

      const CMDBASE* pCmdPtr = reinterpret_cast<CMDBASE*>(m_Commands.GetPtr());
      const CMDBASE* pCmdEnd = reinterpret_cast<CMDBASE*>(m_Commands.GetEnd());

      for(; pCmdPtr != pCmdEnd; pCmdPtr = reinterpret_cast<CMDBASE*>(reinterpret_cast<size_t>(pCmdPtr) + pCmdPtr->cbSize))
      {
        const CanvasFunc cmd = pCmdPtr->cmd;

        if(bEmptyRect && (cmd > CF_DrawFirst && cmd < CF_DrawLast)) {
          continue;
        }

        switch(cmd)
        {
        case CF_LineList:
        {
          TRACE_CMD("CF_LineList\n");
          const DRAWCALL_LINELIST* pLineListCmd = static_cast<const DRAWCALL_LINELIST*>(pCmdPtr);
          ASSERT(sizeof(DRAWCALL_LINELIST) == pCmdPtr->cbSize);
          if(bEmptyRect == FALSE) {

            m_pGraphics->DrawPrimitive(GXPT_LINELIST,
              nBaseVertex, 0, pLineListCmd->uVertexCount, nStartIndex, pLineListCmd->uIndexCount / 2);
          }

          nBaseVertex += pLineListCmd->uVertexCount;
          nStartIndex += pLineListCmd->uIndexCount;
        }
        break;

        case CF_Points:
        {
          TRACE_CMD("CF_Points\n");
          const DRAWCALL_POINTS* pPointsCmd = static_cast<const DRAWCALL_POINTS*>(pCmdPtr);
          ASSERT(sizeof(DRAWCALL_POINTS) == pCmdPtr->cbSize);
          if(bEmptyRect == FALSE) {
            m_pGraphics->DrawPrimitive(GXPT_POINTLIST, nBaseVertex, pPointsCmd->uVertexCount);
          }
          nBaseVertex += pPointsCmd->uVertexCount;
        }
        break;
        case CF_Triangle:
        {
          TRACE_CMD("CF_Trangle\n");
          const DRAWCALL_TRIANGLELIST* pTriangleListCmd = static_cast<const DRAWCALL_TRIANGLELIST*>(pCmdPtr);
          ASSERT(sizeof(DRAWCALL_TRIANGLELIST) == pCmdPtr->cbSize);
          if(bEmptyRect == FALSE) {
            m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST,
              nBaseVertex, 0, pTriangleListCmd->uVertexCount, nStartIndex, pTriangleListCmd->uIndexCount / 3);
          }
          nBaseVertex += pTriangleListCmd->uVertexCount;
          nStartIndex += pTriangleListCmd->uIndexCount;
        }
        break;

        case CF_Textured:
        {
          TRACE_CMD("CF_Textured\n");
          const DRAWCALL_TEXTURE* pTextureCmd = static_cast<const DRAWCALL_TEXTURE*>(pCmdPtr);
          ASSERT(sizeof(DRAWCALL_TEXTURE) == pCmdPtr->cbSize);

          if(bEmptyRect == FALSE)
          {
            m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(pTextureCmd->pTexture), 0);
            m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST,
              nBaseVertex, 0, pTextureCmd->uVertexCount, nStartIndex, pTextureCmd->uIndexCount / 3);
          }

          nBaseVertex += pTextureCmd->uVertexCount;
          nStartIndex += pTextureCmd->uIndexCount;

          m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(m_pWhiteTex), 0);
          pTextureCmd->pTexture->Release();
        }
        break;

        case CF_SetSamplerState:
        {
          const STATESWITCHING_SAMPLERSTATE* pSamplerStateCmd = static_cast<const STATESWITCHING_SAMPLERSTATE*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_SAMPLERSTATE) == pCmdPtr->cbSize);
          m_pSamplerState->SetState(pSamplerStateCmd->sampler_slot, &pSamplerStateCmd->desc);
        }
        break;

        case CF_Clear:
        {
          TRACE_CMD("CF_Clear\n");
          const STATESWITCHING_CLEAR* pClearCmd = static_cast<const STATESWITCHING_CLEAR*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_CLEAR) == pCmdPtr->cbSize);
          if(m_pClipRegion == NULL)
          {
            GXRECT rect;
            rect.left = m_rcClip.left;
            rect.top = m_rcClip.top;
            rect.right = m_rcClip.right;
            rect.bottom = m_rcClip.bottom;
            const GXHRESULT hRet = // Debug
              m_pGraphics->Clear(&rect, 1, pClearCmd->flags, (GXCOLOR)pClearCmd->color, 1.0f, m_dwStencil);
            ASSERT(GXSUCCEEDED(hRet));
          }
          else
          {
            const GXUINT nRectCount = m_pClipRegion->GetRectCount();
            clstd::LocalBuffer<sizeof(GXRECT) * 128> _buf;
            _buf.Resize(sizeof(GXRECT) * nRectCount, FALSE);

            GXRECT* lpRects = (GXRECT*)_buf.GetPtr(); // _GlbLockStaticRects(nRectCount);
            m_pClipRegion->GetRects(lpRects, nRectCount);
            m_pGraphics->Clear(lpRects, nRectCount, pClearCmd->flags, (GXCOLOR)pClearCmd->color, 1.0f, m_dwStencil);
            //_GlbUnlockStaticRects(lpRects);
          }
        }
        break;
        case CF_CompositingMode:
        {
          TRACE_CMD("CF_CompositingMode\n");
          const STATESWITCHING_COMPOSITINGMODE* pCompositingModeCmd
            = static_cast<const STATESWITCHING_COMPOSITINGMODE*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_COMPOSITINGMODE) == pCmdPtr->cbSize);

          BlendStateImpl* pBlendState = NULL;

          if(pCompositingModeCmd->mode == CompositingMode_InvertTarget)
          {
            pBlendState = m_pInvertState;
          }
          else
          {
            // 判断是不是最终渲染目标
            if(m_pTargetTex == NULL)
            {
              if(pCompositingModeCmd->mode == CompositingMode_SourceCopy)
                pBlendState = m_pOpaqueState[0];
              else
                pBlendState = m_pBlendingState[0];
            }
            else
            {
              if(pCompositingModeCmd->mode == CompositingMode_SourceCopy)
                pBlendState = m_pOpaqueState[1];
              else
                pBlendState = m_pBlendingState[1];
            }
          }
          m_pGraphics->InlSetBlendState(pBlendState);
        }
        break;

        case CF_Effect:
        {
          TRACE_CMD("CF_Effect\n");
          const STATESWITCHING_EFFECT* pEffectCmd = static_cast<const STATESWITCHING_EFFECT*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_EFFECT) == pCmdPtr->cbSize);
          if(m_pEffectImpl == pEffectCmd->pEffectImpl) {
            break;
          }
          SAFE_RELEASE(m_pEffectImpl);
          m_pEffectImpl = pEffectCmd->pEffectImpl;
          m_pGraphics->InlSetEffect(m_pEffectImpl);
        }
        break;

        case CF_ColorAdditive:
        {
          TRACE_CMD("CF_ColorAdditive\n");
          const STATESWITCHING_COLORADD* pColorAddCmd = static_cast<const STATESWITCHING_COLORADD*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_COLORADD) == pCmdPtr->cbSize);
          m_dwColorAdditive = (GXDWORD)pColorAddCmd->color;
          float4 v4 = m_dwColorAdditive;
          m_CommonEffect.color_add = v4;

          // TODO: 这里全部重新提交了CB，可以改为只提交$Globals          
          static_cast<ShaderImpl*>(m_pEffectImpl->GetShaderUnsafe())->CommitConstantBuffer(m_pEffectImpl->GetDataPoolUnsafe());
        }
        break;

        case CF_SetViewportOrg:
          TRACE_CMD("CF_SetViewportOrg\n");
          break;

        case CF_SetClipBox:
        {
          TRACE_CMD("CF_SetClipBox\n");
          const STATESWITCHING_SETCLIPBOX* pClipBoxCmd = static_cast<const STATESWITCHING_SETCLIPBOX*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_SETCLIPBOX) == pCmdPtr->cbSize);
          GXREGN rgClip;
          GXRECT rcClip = pClipBoxCmd->rect;
          SAFE_RELEASE(m_pClipRegion);

          m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);

          gxRectToRegn(&rgClip, &rcClip);
          m_pGraphics->SetSafeClip(&rgClip);

          if(gxIsRectEmpty(&rcClip) == TRUE)
            bEmptyRect = TRUE;

          IntUpdateClip(rcClip);
        }
        break;
        case CF_ResetClipBox:
          TRACE_CMD("CF_ResetClipBox\n");
          UpdateStencil(NULL);
          break;

        case CF_ResetTextClip:
        {
          GXREGN rgClip;
          if(m_pClipRegion != NULL)
          {
            GXRECT rcReset;
            m_pClipRegion->GetBounding(&rcReset);
            gxRectToRegn(&rgClip, &rcReset);
          }
          else
            gxRectToRegn(&rgClip, &m_rcAbsClip);
          m_pGraphics->SetSafeClip(&rgClip);
        }
        break;
        case CF_SetTextClip:
        {
          const STATESWITCHING_SETTEXTCLIP* pTextClipCmd = static_cast<const STATESWITCHING_SETTEXTCLIP*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_SETTEXTCLIP) == pCmdPtr->cbSize);
          GXRECT rcClip = pTextClipCmd->rect;
          GXREGN rgClip;
          gxRectToRegn(&rgClip, &rcClip);
          m_pGraphics->SetSafeClip(&rgClip);
        }
        break;
        case CF_SetRegion:
        {
          TRACE_CMD("CF_SetRegion\n");
          const STATESWITCHING_REGION* pRegionCmd = static_cast<const STATESWITCHING_REGION*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_REGION) == pCmdPtr->cbSize);
          if(UpdateStencil(pRegionCmd->pRegion) == RC_NULL) {
            bEmptyRect = TRUE;
          }
          else {
            bEmptyRect = FALSE;
          }
        }
        break;
        case CF_NoOperation:
          TRACE_CMD("CF_NoOperation\n");
          // 啥也没有!!
          break;
        case CF_SetExtTexture:
        {
          const STATESWITCHING_TEXTUREEXT* pTextureExtCmd = static_cast<const STATESWITCHING_TEXTUREEXT*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_TEXTUREEXT) == pCmdPtr->cbSize);

          const GXUINT uStage = pTextureExtCmd->stage;
          Texture* pTexture = pTextureExtCmd->pTexture;
          m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(pTexture), uStage);
          SAFE_RELEASE(m_aTextureStage[uStage]);
          m_aTextureStage[uStage] = (TextureImpl*)pTexture;

          if(pTexture != NULL)
            SETBIT(m_dwTexSlot, uStage);
          else
            RESETBIT(m_dwTexSlot, uStage);
        }
        break;
        case CF_SetTransform:
        {
          const STATESWITCHING_TRANSFORM* pTransformCmd = static_cast<const STATESWITCHING_TRANSFORM*>(pCmdPtr);
          ASSERT(sizeof(STATESWITCHING_TRANSFORM) == pCmdPtr->cbSize);
          m_CommonEffect.transform = pTransformCmd->transform;
          static_cast<ShaderImpl*>(m_pEffectImpl->GetShaderUnsafe())->CommitConstantBuffer(m_pEffectImpl->GetDataPoolUnsafe());

        }
        break;
        default:
          CLBREAK;
        }
      }
      m_uVertCount = 0;
      m_uIndexCount = 0;
      m_pLastCommand = NULL;
      m_Commands.Resize(0, FALSE);

      // 每次提交后两个clip应该是一致的，如果不一致说明中间的计算算法有差异
      ASSERT(gxEqualRect(&m_CallState.rcClip, &m_rcClip));

      m_pGraphics->Leave();
      return TRUE;
    }

    GXBOOL CanvasImpl::SetSamplerState(GXUINT Sampler, GXSAMPLERDESC* pDesc)
    {
      STATESWITCHING_SAMPLERSTATE* pSamplerStateCmd = IntAppendCommand<STATESWITCHING_SAMPLERSTATE>(CF_SetSamplerState);
      pSamplerStateCmd->sampler_slot = Sampler;
      pSamplerStateCmd->desc = *pDesc;

      return TRUE;
    }

    GXBOOL CanvasImpl::SetEffect(Effect* pEffect)
    {
      if(m_CallState.RenderState.pEffect == pEffect) {
        return FALSE;
      }

      STATESWITCHING_EFFECT* pEffectCmd = IntAppendCommand<STATESWITCHING_EFFECT>(CF_Effect);

      if(pEffect != NULL) {
        pEffect->AddRef();
      }
      pEffectCmd->pEffectImpl = static_cast<EffectImpl*>(pEffect);
      m_CallState.RenderState.pEffect = (EffectImpl*)pEffect;

      return TRUE;
    }

    GXDWORD CanvasImpl::SetParametersInfo(CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam)
    {
      GXDWORD dwRet = NULL;

      ASSERT(sizeof(uParam) >= sizeof(GXDWORD));

      switch(eAction)
      {
      case CPI_SETTEXTURECOLOR:
      {
        dwRet = m_dwTexVertColor;
        m_dwTexVertColor = (GXDWORD)COLORREF_TO_NATIVE(uParam);
      }
      break;
      case CPI_SETCOLORADDITIVE:
      {
        dwRet = NATIVE_TO_COLORREF(m_CallState.dwColorAdditive);
        m_CallState.dwColorAdditive = (GXDWORD)COLORREF_TO_NATIVE(uParam);

        // 检测是否是新的叠加颜色
        if(m_CallState.dwColorAdditive != dwRet)
        {
          STATESWITCHING_COLORADD* pCommand = IntAppendCommand<STATESWITCHING_COLORADD>(CF_ColorAdditive);
          pCommand->color = m_CallState.dwColorAdditive;
        }
      }
      break;
      case CPI_SETTEXTCLIP:
      {
        GXLPRECT lpRect = (GXLPRECT)pParam;
        // 这个函数里面将 RECT的left 和 right, top 和 bottom 压缩储存

        if(lpRect == NULL)  // 复位模式
        {
          CMDBASE* pCommand = IntAppendCommand<CMDBASE>(CF_ResetTextClip);
        }
        else  // 用户设置
        {
          STATESWITCHING_SETTEXTCLIP* pCommand = IntAppendCommand<STATESWITCHING_SETTEXTCLIP>(CF_SetTextClip);
          // 转换为 RenderTarget 空间的坐标
          GXRECT rcUserClip = *lpRect;
          gxOffsetRect(&rcUserClip, m_CallState.origin.x, m_CallState.origin.y);

          // 与系统区域裁剪
          //gxIntersectRect(&rcUserClip, &m_rcAbsClip, &rcUserClip);
          gxIntersectRect(&rcUserClip, &m_CallState.rcClip, &rcUserClip);

          pCommand->rect = rcUserClip;
        }
        return TRUE;
      }
      break;

      case CPI_SETEXTTEXTURE:
      {
        if(uParam > 0 && uParam < GX_MAX_TEXTURE_STAGE)
        {
          STATESWITCHING_TEXTUREEXT* pCommand = IntAppendCommand<STATESWITCHING_TEXTUREEXT>(CF_SetExtTexture);
          pCommand->stage = uParam;
          pCommand->pTexture = reinterpret_cast<Texture*>(pParam);
          if(pCommand->pTexture) {
            pCommand->pTexture->AddRef();
          }
        }
        else {
          dwRet = 0;
        }
        dwRet = uParam;
      }
      break;
      }
      return dwRet;
    }

    PenStyle CanvasImpl::SetPenStyle(PenStyle eStyle)
    {
      if(eStyle < 0 || eStyle > PS_DashDotDot) {
        return m_eStyle;
      }

      PenStyle ePrevStyle = m_eStyle;
      m_eStyle = eStyle;
      return ePrevStyle;
    }

    GXBOOL CanvasImpl::Clear(GXCOLORREF crClear)
    {
      STATESWITCHING_CLEAR* pCommand = IntAppendCommand<STATESWITCHING_CLEAR>(CF_Clear);

      pCommand->flags |= GXCLEAR_TARGET;
      pCommand->color = crClear;

      return TRUE;
    }

    CompositingMode CanvasImpl::SetCompositingMode(CompositingMode eMode)
    {
      if(m_CallState.eCompMode == eMode) {
        return eMode;
      }

      STATESWITCHING_COMPOSITINGMODE* pCommand = IntAppendCommand<STATESWITCHING_COMPOSITINGMODE>(CF_CompositingMode);
      pCommand->mode = eMode;

      CompositingMode ePrevCompMode = m_CallState.eCompMode;
      m_CallState.eCompMode = eMode;
      return ePrevCompMode;
    }

    CompositingMode CanvasImpl::GetCompositingMode()
    {
      return m_CallState.eCompMode;
    }

    GXBOOL CanvasImpl::SetRegion(GRegion* pRegion, GXBOOL bAbsOrigin)
    {
      GRegion* pSurfaceRegion = NULL;
      STATESWITCHING_REGION* pRegionCmd = IntAppendCommand<STATESWITCHING_REGION>(CF_SetRegion);

      if(pRegion != NULL)
      {
        // TODO: 测试如果区域小于屏幕区,就不用这个屏幕区的Region裁剪
        m_pGraphics->CreateRectRgn(&pSurfaceRegion, 0, 0, m_sExtent.cx, m_sExtent.cy);

        if(bAbsOrigin == TRUE) {
          pSurfaceRegion->Intersect(pRegion);
        }
        else
        {
          GRegion* pAbsRegion = pRegion->Clone();
          pAbsRegion->Offset(m_CallState.origin.x, m_CallState.origin.y);
          pSurfaceRegion->Intersect(pAbsRegion);
          SAFE_RELEASE(pAbsRegion);
        }
        pSurfaceRegion->GetBounding(&m_CallState.rcClip);
      }
      else {
        m_CallState.rcClip = m_rcAbsClip;
      }

      pRegionCmd->pRegion = pSurfaceRegion;
      return TRUE;
    }

    GXBOOL CanvasImpl::SetClipBox(const GXLPRECT lpRect)
    {
      if(lpRect == NULL)  // 复位模式
      {
        CMDBASE* pCommand = IntAppendCommand<CMDBASE>(CF_ResetClipBox);
        m_CallState.rcClip = m_rcAbsClip;
      }
      else  // 用户设置
      {
        STATESWITCHING_SETCLIPBOX* pCommand = IntAppendCommand<STATESWITCHING_SETCLIPBOX>(CF_SetClipBox);
        // 转换为 RenderTarget 空间的坐标
        GXRECT rcUserClip = *lpRect;
        gxOffsetRect(&rcUserClip, m_CallState.origin.x, m_CallState.origin.y);

        // 与系统区域裁剪
        gxIntersectRect(&m_CallState.rcClip, &m_rcAbsClip, &rcUserClip);

        pCommand->rect = m_CallState.rcClip;
      }
      return TRUE;
    }

    GXINT CanvasImpl::GetClipBox(GXLPRECT lpRect)
    {
      if(lpRect != NULL)
      {
        *lpRect = m_CallState.rcClip;
        gxOffsetRect(lpRect, -m_xAbsOrigin, -m_yAbsOrigin);
      }
      return RC_SIMPLE;
    }

    GXDWORD CanvasImpl::GetStencilLevel()
    {
      return m_dwStencil;
    }

    GXBOOL CanvasImpl::Scroll(int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate)
    {
      Flush();

      SCROLLTEXTUREDESC ScrollTexDesc;
      GRegion* prgnClip;
      m_pGraphics->CreateRectRgn(&prgnClip, lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);

      ScrollTexDesc.pOperationTex = m_pTargetTex->GetColorTextureUnsafe(GXResUsage::Default);
      ScrollTexDesc.pTempTex = NULL;
      ScrollTexDesc.dx = dx;
      ScrollTexDesc.dy = dy;
      ScrollTexDesc.lprcScroll = lprcScroll;
      ScrollTexDesc.lprgnClip = prgnClip;
      ScrollTexDesc.lpprgnUpdate = lpprgnUpdate;
      ScrollTexDesc.lprcUpdate = lprcUpdate;
      m_pGraphics->ScrollTexture(&ScrollTexDesc);
      SAFE_RELEASE(prgnClip);
      return TRUE;
    }

    //#define CHECK_LOCK if(m_lpLockedVertex == NULL)  \
    //  m_pPrimitive->Lock(0, 0, 0, 0, (void**)&m_lpLockedVertex, &m_lpLockedIndex, GXLOCK_DISCARD);
#define CHECK_LOCK \
  if(m_lpLockedVertex == NULL) { m_lpLockedVertex = static_cast<PRIMITIVE*>(m_pPrimitive->MapVertexBuffer(GXResMap::Write)); } \
  if(m_lpLockedIndex == NULL) { m_lpLockedIndex = static_cast<VIndex*>(m_pPrimitive->MapIndexBuffer(GXResMap::Write)); }

#define SET_INDEX_BUFFER(_OFFSET, _INDEX)  m_lpLockedIndex[m_uIndexCount + _OFFSET] = uBaseIndex + _INDEX

    GXBOOL CanvasImpl::SetPixel(GXINT xPos, GXINT yPos, GXCOLORREF crPixel)
    {
      DRAWCALL_POINTS* pDrawCallCmd = NULL;
      IntAppendDrawCall<DRAWCALL_POINTS>(&pDrawCallCmd, CF_Points, 1, 0, NULL);
      CHECK_LOCK;

      m_lpLockedVertex[m_uVertCount].z = 0;
      m_lpLockedVertex[m_uVertCount].w = 1;
      m_lpLockedVertex[m_uVertCount].u = 0;
      m_lpLockedVertex[m_uVertCount].v = 0;
      m_lpLockedVertex[m_uVertCount].color = (GXDWORD)COLORREF_TO_NATIVE(crPixel);

      _SetPrimitivePos(m_uVertCount, xPos, yPos);

      m_uVertCount++;
      //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
      return TRUE;
    }

    GXBOOL CanvasImpl::DrawLine(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine)
    {
      //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

      DRAWCALL_LINELIST* pDrawCallCmd = NULL;
      GXUINT uBaseIndex = IntAppendDrawCall<DRAWCALL_LINELIST>(&pDrawCallCmd, CF_LineList, 2, 2, NULL);
      CHECK_LOCK;

      for(int i = 0; i < 2; i++)
      {
        m_lpLockedVertex[m_uVertCount + i].z = 0;
        m_lpLockedVertex[m_uVertCount + i].w = 1;
        m_lpLockedVertex[m_uVertCount + i].u = 0;
        m_lpLockedVertex[m_uVertCount + i].v = 0;
        m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)COLORREF_TO_NATIVE(crLine);
      }

      _SetPrimitivePos(m_uVertCount, left, top);
      _SetPrimitivePos(m_uVertCount + 1, right, bottom);

      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);

      m_uVertCount += 2;
      m_uIndexCount += 2;

      //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////

    GXBOOL CanvasImpl::InlDrawRectangle(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crRect)
    {
      DRAWCALL_LINELIST* pDrawCallCmd = NULL;
      GXUINT uBaseIndex = IntAppendDrawCall<DRAWCALL_LINELIST>(&pDrawCallCmd, CF_LineList, 4, 8, NULL);
      CHECK_LOCK;

      GXDWORD dwColor = COLORREF_TO_NATIVE(crRect);

      float x1 = float(left   + m_CallState.origin.x);
      float y1 = float(top    + m_CallState.origin.y);
      float x2 = float(right  + m_CallState.origin.x - 1);
      float y2 = float(bottom + m_CallState.origin.y - 1);

      m_lpLockedVertex[m_uVertCount + 0].Set(x1, y1, dwColor);
      m_lpLockedVertex[m_uVertCount + 1].Set(x2, y1, dwColor);
      m_lpLockedVertex[m_uVertCount + 2].Set(x2, y2, dwColor);
      m_lpLockedVertex[m_uVertCount + 3].Set(x1, y2, dwColor);


      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);
      SET_INDEX_BUFFER(2, 1);
      SET_INDEX_BUFFER(3, 2);
      SET_INDEX_BUFFER(4, 2);
      SET_INDEX_BUFFER(5, 3);
      SET_INDEX_BUFFER(6, 3);
      SET_INDEX_BUFFER(7, 0);

      m_uVertCount += 4;
      m_uIndexCount += 8;

      //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
      return TRUE;
    }

    GXBOOL CanvasImpl::DrawRectangle(GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect)
    {
      return InlDrawRectangle(x, y, x + w, y + h, crRect);
    }

    GXBOOL CanvasImpl::DrawRectangle(GXLPCRECT lprc, GXCOLORREF crRect)
    {
      return InlDrawRectangle(lprc->left, lprc->top, lprc->right, lprc->bottom, crRect);
    }

    GXBOOL CanvasImpl::DrawRectangle(GXLPCREGN lprg, GXCOLORREF crRect)
    {
      return InlDrawRectangle(lprg->left, lprg->top, lprg->left + lprg->width, lprg->top + lprg->height, crRect);
    }

    //////////////////////////////////////////////////////////////////////////
    GXBOOL CanvasImpl::InlFillRectangle(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crFill)
    {
      //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

      DRAWCALL_TRIANGLELIST* pDrawCallCmd = NULL;
      GXUINT uBaseIndex = IntAppendDrawCall<DRAWCALL_TRIANGLELIST>(&pDrawCallCmd, CF_Triangle, 4, 6, NULL);
      CHECK_LOCK;

      GXDWORD dwColor = COLORREF_TO_NATIVE(crFill);
      float x1 = float(left   + m_CallState.origin.x);
      float y1 = float(top    + m_CallState.origin.y);
      float x2 = float(right  + m_CallState.origin.x);
      float y2 = float(bottom + m_CallState.origin.y);

      m_lpLockedVertex[m_uVertCount + 0].Set(x1, y1, dwColor);
      m_lpLockedVertex[m_uVertCount + 1].Set(x2, y1, dwColor);
      m_lpLockedVertex[m_uVertCount + 2].Set(x2, y2, dwColor);
      m_lpLockedVertex[m_uVertCount + 3].Set(x1, y2, dwColor);

      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);
      SET_INDEX_BUFFER(2, 3);
      SET_INDEX_BUFFER(3, 3);
      SET_INDEX_BUFFER(4, 1);
      SET_INDEX_BUFFER(5, 2);

      m_uVertCount += 4;
      m_uIndexCount += 6;

      //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
      return TRUE;
    }

    GXBOOL CanvasImpl::FillRectangle(GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill)
    {
      return InlFillRectangle(x, y, x + w, y + h, crFill);
    }

    GXBOOL CanvasImpl::FillRectangle(GXLPCRECT lprc, GXCOLORREF crFill)
    {
      return InlFillRectangle(lprc->left, lprc->top, lprc->right, lprc->bottom, crFill);
    }

    GXBOOL CanvasImpl::FillRectangle(GXLPCREGN lprg, GXCOLORREF crFill)
    {
      return InlFillRectangle(lprg->left, lprg->top, lprg->left + lprg->width, lprg->top + lprg->height, crFill);
    }

    //////////////////////////////////////////////////////////////////////////

    GXBOOL CanvasImpl::ColorFillRegion(GRegion* pRegion, GXCOLORREF crFill)
    {
      if(pRegion == NULL)
        return FALSE;
      int nCount = pRegion->GetRectCount();
      clstd::LocalBuffer<sizeof(GXRECT) * 128> _buf;
      _buf.Resize(sizeof(GXRECT) * nCount, FALSE);

      GXRECT* pRects = (GXRECT*)_buf.GetPtr(); // _GlbLockStaticRects(nCount);
      pRegion->GetRects(pRects, nCount);

      for(int i = 0; i < nCount; i++)
      {
        FillRectangle(pRects[i].left, pRects[i].top,
          pRects[i].right - pRects[i].left,
          pRects[i].bottom - pRects[i].top, crFill);
      }
      //_GlbUnlockStaticRects(pRects);
      return TRUE;
    }
    GXBOOL CanvasImpl::DrawUserPrimitive(Texture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount)
    {
      if(m_uVertIndexSize < uVertCount || m_uVertIndexSize < uIdxCount)
        return FALSE;

      DRAWCALL_TEXTURE* pDrawTextureCmd = NULL;
      GXUINT uBaseIndex = IntAppendDrawCall<DRAWCALL_TEXTURE>(&pDrawTextureCmd, CF_Textured, uVertCount, uIdxCount, pTexture);
      CHECK_LOCK;

      //memcpy(m_lpLockedVertex + m_uVertCount, lpVertices, uVertCount * sizeof(PRIMITIVE));
      PRIMITIVE* pVertex = m_lpLockedVertex + m_uVertCount;
      for(GXUINT i = 0; i < uVertCount; i++, pVertex++)
      {
        *pVertex = *((PRIMITIVE*)lpVertices + i);
        pVertex->x += (GXFLOAT)m_CallState.origin.x;
        pVertex->y += (GXFLOAT)m_CallState.origin.y;
      }

      for(GXUINT i = 0; i < uIdxCount; i++) {
        m_lpLockedIndex[m_uIndexCount + i] = uBaseIndex + pIndices[i];
      }

      m_uVertCount += uVertCount;
      m_uIndexCount += uIdxCount;

      return TRUE;
    }
    GXBOOL CanvasImpl::DrawTexture(Texture*pTexture, const GXREGN *rcDest)
    {
      if(pTexture == NULL) {
        return FALSE;
      }
      DRAWCALL_TEXTURE* pDrawCallCmd = NULL;
      GXUINT uBaseIndex = IntAppendDrawCall<DRAWCALL_TEXTURE>(&pDrawCallCmd, CF_Textured, 4, 6, pTexture);
      CHECK_LOCK;

      for(int i = 0; i < 4; i++)
      {
        m_lpLockedVertex[m_uVertCount + i].z = 0;
        m_lpLockedVertex[m_uVertCount + i].w = 1;
        m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)m_dwTexVertColor;
      }
#ifdef GLES2_CANVAS_IMPL
      m_lpLockedVertex[m_uVertCount].SetTexcoord(0, 1);
      m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1, 1);
      m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1, 0);
      m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0, 0);
#elif defined(D3D9_CANVAS_IMPL)
      GXUINT nTexWidth;
      GXUINT nTexHeight;
      pTexture->GetDimension(&nTexWidth, &nTexHeight);

      const GXFLOAT fHalfTexelKernelX = 0.5f / (GXFLOAT)nTexWidth;
      const GXFLOAT fHalfTexelKernelY = 0.5f / (GXFLOAT)nTexHeight;

      m_lpLockedVertex[m_uVertCount].SetTexcoord(0 + fHalfTexelKernelX, 0 + fHalfTexelKernelY);
      m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1 + fHalfTexelKernelX, 0 + fHalfTexelKernelY);
      m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1 + fHalfTexelKernelX, 1 + fHalfTexelKernelY);
      m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0 + fHalfTexelKernelX, 1 + fHalfTexelKernelY);
#elif defined(D3D11_CANVAS_IMPL)
      m_lpLockedVertex[m_uVertCount].SetTexcoord(0, 0);
      m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1, 0);
      m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1, 1);
      m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0, 1);
#else
#error 需要定义inl的环境
#endif

      _SetPrimitivePos(m_uVertCount + 0, rcDest->left, rcDest->top);
      _SetPrimitivePos(m_uVertCount + 1, rcDest->left + rcDest->width, rcDest->top);
      _SetPrimitivePos(m_uVertCount + 2, rcDest->left + rcDest->width, rcDest->top + rcDest->height);
      _SetPrimitivePos(m_uVertCount + 3, rcDest->left, rcDest->top + rcDest->height);

      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);
      SET_INDEX_BUFFER(2, 3);
      SET_INDEX_BUFFER(3, 3);
      SET_INDEX_BUFFER(4, 1);
      SET_INDEX_BUFFER(5, 2);

      m_uVertCount += 4;
      m_uIndexCount += 6;
      return TRUE;
    }

    GXBOOL CanvasImpl::DrawTexture(Texture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc)
    {
#include "Platform/CommonInline/GXCanvasImpl_DrawTexture.inl"

      const GXUINT uWidth = rcSrc->width;
      const GXUINT uHeight = rcSrc->height;

      _SetPrimitivePos(m_uVertCount + 0, xPos, yPos);
      _SetPrimitivePos(m_uVertCount + 1, xPos + uWidth, yPos);
      _SetPrimitivePos(m_uVertCount + 2, xPos + uWidth, yPos + uHeight);
      _SetPrimitivePos(m_uVertCount + 3, xPos, yPos + uHeight);

      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);
      SET_INDEX_BUFFER(2, 3);
      SET_INDEX_BUFFER(3, 3);
      SET_INDEX_BUFFER(4, 1);
      SET_INDEX_BUFFER(5, 2);

      m_uVertCount += 4;
      m_uIndexCount += 6;
      return TRUE;
    }

    GXBOOL CanvasImpl::DrawTexture(Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc)
    {
#include "Platform/CommonInline/GXCanvasImpl_DrawTexture.inl"

      // 0---1
      // | / |
      // 3---2

      const GXUINT uDestRight = (rcDest->left + rcDest->width);
      const GXUINT uDestBottom = (rcDest->top + rcDest->height);

      _SetPrimitivePos(m_uVertCount + 0, rcDest->left, rcDest->top);
      _SetPrimitivePos(m_uVertCount + 1, uDestRight, rcDest->top);
      _SetPrimitivePos(m_uVertCount + 2, uDestRight, uDestBottom);
      _SetPrimitivePos(m_uVertCount + 3, rcDest->left, uDestBottom);

      SET_INDEX_BUFFER(0, 0);
      SET_INDEX_BUFFER(1, 1);
      SET_INDEX_BUFFER(2, 3);
      SET_INDEX_BUFFER(3, 3);
      SET_INDEX_BUFFER(4, 1);
      SET_INDEX_BUFFER(5, 2);

      m_uVertCount += 4;
      m_uIndexCount += 6;

      return TRUE;
    }

    //
    // GXCanvasImpl::DrawTexture 根据定义做了特殊优化，不能随便修改
    // 不可以修改这个定义顺序，不可以随便添加定义，小心出错！
    STATIC_ASSERT(Rotate_None == 0);
    STATIC_ASSERT(Rotate_CW90 == 1);
    STATIC_ASSERT(Rotate_180 == 2);
    STATIC_ASSERT(Rotate_CCW90 == 3);
    STATIC_ASSERT(Rotate_FlipHorizontal == 4);
    STATIC_ASSERT(Rotate_CW90_Flip == 5);
    STATIC_ASSERT(Rotate_180_Flip == 6);
    STATIC_ASSERT(Rotate_CCW90_Flip == 7);

    GXBOOL CanvasImpl::DrawTexture(Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation)
    {
#include "Platform/CommonInline/GXCanvasImpl_DrawTexture.inl"

      // 0---1
      // | / |
      // 3---2

      const GXUINT uDestRight = (rcDest->left + rcDest->width);
      const GXUINT uDestBottom = (rcDest->top + rcDest->height);
      const GXPOINT aPos[] = {
        {rcDest->left, rcDest->top},
        {static_cast<GXLONG>(uDestRight),   rcDest->top},
        {static_cast<GXLONG>(uDestRight),   static_cast<GXLONG>(uDestBottom)},
        {rcDest->left, static_cast<GXLONG>(uDestBottom)}, };

      if(eRotation > Rotate_CCW90_Flip) {
        eRotation = Rotate_None;
      }

      if(eRotation < Rotate_FlipHorizontal) {
        int i = (int)eRotation;
        _SetPrimitivePos(m_uVertCount + 0, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
        _SetPrimitivePos(m_uVertCount + 1, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
        _SetPrimitivePos(m_uVertCount + 2, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
        _SetPrimitivePos(m_uVertCount + 3, aPos[i].x, aPos[i].y);

        SET_INDEX_BUFFER(0, 0);
        SET_INDEX_BUFFER(1, 1);
        SET_INDEX_BUFFER(2, 3);
        SET_INDEX_BUFFER(3, 3);
        SET_INDEX_BUFFER(4, 1);
        SET_INDEX_BUFFER(5, 2);
      }
      else {
        int i = ((int)eRotation - Rotate_FlipHorizontal + 1) & 3;
        _SetPrimitivePos(m_uVertCount + 0, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
        _SetPrimitivePos(m_uVertCount + 1, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
        _SetPrimitivePos(m_uVertCount + 2, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
        _SetPrimitivePos(m_uVertCount + 3, aPos[i].x, aPos[i].y);

        SET_INDEX_BUFFER(0, 0);
        SET_INDEX_BUFFER(1, 3);
        SET_INDEX_BUFFER(2, 1);
        SET_INDEX_BUFFER(3, 3);
        SET_INDEX_BUFFER(4, 2);
        SET_INDEX_BUFFER(5, 1);
      }

      m_uVertCount += 4;
      m_uIndexCount += 6;

      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)