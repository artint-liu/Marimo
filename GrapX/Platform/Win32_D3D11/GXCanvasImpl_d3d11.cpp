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
      , m_xAbsOrigin        (0)
      , m_yAbsOrigin        (0)
      //, m_pOMOpaque         (NULL)
      //, m_pOMInvert         (NULL)
      , m_pPrimitive        (NULL)
      , m_lpLockedVertex    (NULL)
      , m_lpLockedIndex     (NULL)
      //, m_pOMNoColor        (NULL)
      //, m_pWriteStencil     (NULL)
      //, m_pWhiteTex         (NULL)
      , m_pLastCommand      (NULL)
      , m_uIndexCount       (NULL)
      , m_uVertCount        (0)
      , m_uVertIndexSize    (0)
      , m_dwStencil         (0)
      , m_pClipRegion       (NULL)
      , m_dwTexVertColor    (-1)
      //, m_dwTexSlot         (NULL)
      //, m_pRasterizerState  (NULL)
      //, m_pDefaultEffectImpl(NULL)
    {
      m_Commands.Reserve(1024);
      InlSetZeroT(m_rcClip);
      InlSetZeroT(m_rcAbsClip);
      //gxRtlZeroMemory(&m_aTextureStage, sizeof(m_aTextureStage));
      //gxRtlZeroMemory(&m_pBlendingState, sizeof(m_pBlendingState));
      //gxRtlZeroMemory(&m_pCanvasStencil, sizeof(m_pCanvasStencil));
      //m_DefaultEffect.pEffect = NULL;
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
        //m_CallState.RenderState.sEffect.InitEffect(m_pEffectImpl);

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

        //STATESWITCHING_COMPOSITINGMODE* pCompositingModeCmd = IntAppendCommand<STATESWITCHING_COMPOSITINGMODE>(CF_CompositingMode);
        //pCompositingModeCmd->mode = CompositingMode_SourceOver;
        m_dwTexVertColor = (GXDWORD)-1;
        //m_dwColorAdditive = 0;
        m_eStyle = PS_Solid;

        // 初始化空纹理时的替换纹理
        if(m_pWhiteTex == NULL) {
          m_pGraphics->CreateTexture(&m_pWhiteTex, "White8x8", 0, 0, Format_Unknown, GXResUsage::Default, 0, NULL, 0); // 查询
        }

        // 创建BlendState
        if(m_pBlendingState[0] == NULL || m_pBlendingState[1] == NULL)
        {
          GXBLENDDESC AlphaBlendState(TRUE);

          ASSERT(m_pBlendingState[0] == NULL);
          ASSERT(m_pBlendingState[1] == NULL);

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
        }

        if(m_pOMOpaque == NULL)
        {
          GXBLENDDESC OpaqueState;
          m_pGraphics->CreateBlendState((BlendState**)&m_pOMOpaque, &OpaqueState, 1);
        }

        if(m_pOMInvert == NULL)
        {
          GXBLENDDESC sInvertBlend(TRUE, GXBLEND_ONE, GXBLEND_ONE, GXBLENDOP_SUBTRACT);
          m_pGraphics->CreateBlendState((BlendState**)&m_pOMInvert, &sInvertBlend, 1);
        }

        // 初始化渲染模式
        m_CallState.eCompMode = CompositingMode_SourceOver;
        //m_CallState.dwColorAdditive = 0;

        SAFE_RELEASE(m_pBlendStateImpl);
        m_pBlendStateImpl = IntGetBlendStateUnsafe(m_CallState.eCompMode);
        m_pBlendStateImpl->AddRef();


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

        if(m_pWriteStencil == NULL)
        {
          GXDEPTHSTENCILDESC desc(TRUE, TRUE);
          desc.DepthFunc = GXCMP_ALWAYS;
          desc.FrontFace.StencilDepthFailOp = GXSTENCILOP_KEEP;
          desc.FrontFace.StencilFailOp      = GXSTENCILOP_KEEP;
          desc.FrontFace.StencilFunc        = GXCMP_ALWAYS;
          desc.FrontFace.StencilPassOp      = GXSTENCILOP_REPLACE;

          desc.BackFace.StencilDepthFailOp  = GXSTENCILOP_KEEP;
          desc.BackFace.StencilFailOp       = GXSTENCILOP_KEEP;
          desc.BackFace.StencilFunc         = GXCMP_ALWAYS;
          desc.BackFace.StencilPassOp       = GXSTENCILOP_REPLACE;
          m_pGraphics->CreateDepthStencilState((DepthStencilState**)&m_pWriteStencil, &desc);
        }

        if(m_pOMNoColor == NULL)
        {
          GXBLENDDESC sBlendDesc;
          sBlendDesc.WriteMask = 0;
          m_pGraphics->CreateBlendState((BlendState**)&m_pOMNoColor, &sBlendDesc, 1);
        }

        // 初始化寄存器常量
        GCAMERACONETXT gcc;
        gcc.dwMask = GCC_WORLD;
        m_pCamera->GetContext(&gcc);

        if(m_pDefaultEffectImpl == NULL)
        {
          m_pGraphics->IntGetEffect()->Clone((Effect**)&m_pDefaultEffectImpl);
          //m_pDefaultEffectImpl = static_cast<EffectImpl*>(m_pGraphics->IntGetEffect());
          //m_pDefaultEffectImpl->AddRef();
        }

        if(m_ClearEffect.pEffectImpl == NULL)
        {
          EffectImpl* pClearEffect;
          m_pDefaultEffectImpl->Clone((Effect**)&pClearEffect);
          m_ClearEffect.InitEffect(pClearEffect);
          m_ClearEffect.color_mul->set(0.0f);  // 使纹理颜色失效
          m_ClearEffect.color_add->set(1.0f);
          SAFE_RELEASE(pClearEffect);
        }

        m_ClearEffect.transform = gcc.matWorld;
        m_transform = m_CallState.matTransform = gcc.matWorld;
        m_CallState.color_mul = m_color_mul = 1.0f;
        m_CallState.color_add = m_color_add = 0.0f;

        SAFE_RELEASE(m_CallState.RenderState.pEffectImpl);
        m_CallState.RenderState.pEffectImpl = m_pDefaultEffectImpl;
        m_CallState.RenderState.pEffectImpl->AddRef();
        m_CurrentEffect.InitEffect(m_pDefaultEffectImpl);

        //ASSERT(m_dwTexSlot == NULL);
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
          IntClear(&rcClip, 1, dwFlags, 0xff0000ff, 0, 0); // 防止stencil值重复, 先清零
          IntClear(lpRects, nRectCount, dwFlags, 0xff00ff00, 0, m_dwStencil);

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
        m_pGraphics->InlSetCanvas(NULL);
      }

      // 最多两个引用
      ASSERT(m_uRefCount > 0 && m_uRefCount <= 3);
      m_uRefCount--;

      // Graphics 内置 Canvas 在一个引用计数时释放
      // 否则在引用计数为 0 时释放
      if((m_bStatic == TRUE && m_uRefCount == 1) ||
        m_uRefCount == 0)
      {
        //SAFE_RELEASE(m_pEffectImpl);
        SAFE_RELEASE(m_pTargetTex);
        //SAFE_RELEASE(m_pTargetImage);
        m_dwStencil = 0;

        SAFE_RELEASE(m_pClipRegion);
        //SAFE_RELEASE(m_DefaultEffect.pEffect);

        //if(m_dwTexSlot != NULL)
        //{
        //  ASSERT(m_aTextureStage[0] == NULL);
        //  for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && m_dwTexSlot != 0; i++)
        //  {
        //    if(m_aTextureStage[i] != NULL)
        //    {
        //      m_pGraphics->InlSetTexture(NULL, i);
        //      m_aTextureStage[i]->Release();
        //      m_aTextureStage[i] = NULL;
        //      RESETBIT(m_dwTexSlot, i);
        //    }
        //  }
        //}
        //ASSERT(m_dwTexSlot == NULL);

        m_uVertCount = 0;
        m_uIndexCount = 0;
      }

      if(m_uRefCount == 0)
      {
        //SAFE_RELEASE(m_pDefaultEffectImpl);
        //SAFE_RELEASE(m_pOMInvert);
        //SAFE_RELEASE(m_pOMOpaque);
        //SAFE_RELEASE(m_pRasterizerState);
        //SAFE_RELEASE(m_pBlendingState[0]);
        //SAFE_RELEASE(m_pBlendingState[1]);
        //SAFE_RELEASE(m_pCanvasStencil[0]);
        //SAFE_RELEASE(m_pCanvasStencil[1]);
        //SAFE_RELEASE(m_pOMNoColor);
        //SAFE_RELEASE(m_pWriteStencil);
        //SAFE_RELEASE(m_pWhiteTex);
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
    _Ty* CanvasImpl::IntAppendCommandAlways(CanvasFunc cmd)
    {
      size_t cbFinalSize = m_Commands.GetSize() + sizeof(_Ty);
      if(cbFinalSize > m_Commands.GetCapacity()) {
        cbFinalSize = sizeof(_Ty);
        Flush();
      }

      _Ty* pCmdPtr = new(m_Commands.GetEnd()) _Ty(); // 构造vtbl
      m_Commands.Resize(cbFinalSize, FALSE);
      pCmdPtr->cbSize = sizeof(_Ty);
      pCmdPtr->cmd = cmd;

      m_pLastCommand = pCmdPtr;
      return pCmdPtr;
    }

    template<typename _Ty>
    _Ty* CanvasImpl::IntAppendCommand(CanvasFunc cmd)
    {
      //if(cmd != CF_SetExtTexture) // 不支持合并的命令
      {
        if(m_pLastCommand && m_pLastCommand->cmd == cmd) {
          m_pLastCommand->OnMerge();
          return m_pLastCommand->cast_to<_Ty>();
        }
      }

      return IntAppendCommandAlways<_Ty>(cmd);
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
        if(cmd != CF_Textured || m_pLastCommand->cast_to<DRAWCALL_TEXTURE>()->pTexture == pTextureReference)
        {
          DRAWCALLBASE* pLastDrawCall = static_cast<DRAWCALLBASE*>(m_pLastCommand);
          const GXUINT uBaseIndex = pLastDrawCall->uVertexCount;
          pLastDrawCall->uVertexCount += uVertexCount;
          pLastDrawCall->uIndexCount  += uIndexCount;
          *ppCmdBuffer = pLastDrawCall->cast_to<_Ty>();
          return uBaseIndex;
        }
      }

      *ppCmdBuffer = IntAppendCommandAlways<_Ty>(cmd);
      (*ppCmdBuffer)->uVertexCount = uVertexCount;
      (*ppCmdBuffer)->uIndexCount = uIndexCount;
      if(cmd == CF_Textured) {
        (*ppCmdBuffer)->cast_to<DRAWCALL_TEXTURE>()->pTexture = pTextureReference;
        if(pTextureReference) {
          pTextureReference->AddRef();
        }
      }
      return 0;
    }

    void CanvasImpl::IntClear(const GXRECT* lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil)
    {
      // 确保全是预知状态
      ASSERT(m_pGraphics->IsActiveCanvas(this));
      ASSERT(m_pGraphics->m_pCurShader == m_CurrentEffect.pEffectImpl->GetShaderUnsafe());
      ASSERT(m_pGraphics->m_pCurPrimitive == m_pPrimitive);
      ASSERT(m_pGraphics->m_pCurBlendState == m_pBlendStateImpl);
      ASSERT(
        (m_pClipRegion == NULL && m_pGraphics->m_pCurDepthStencilState == m_pCanvasStencil[0]) ||
        (m_pClipRegion != NULL && m_pGraphics->m_pCurDepthStencilState == m_pCanvasStencil[1]) ||
        (m_pClipRegion != NULL && m_pGraphics->m_pCurDepthStencilState == m_pCanvasStencil[0]) ); // UpdateStencil 中间状态
      
      m_pGraphics->IntSetEffect(m_ClearEffect.pEffectImpl);
      m_pWriteStencil->SetStencilRef(dwStencil);
      m_pGraphics->InlSetDepthStencilState(m_pWriteStencil);
      m_pGraphics->InlSetBlendState(TEST_FLAG(dwFlags, GXCLEAR_TARGET) ? m_pOMOpaque : m_pOMNoColor);


      Primitive* pPrimitive = NULL;
      clstd::LocalBuffer<sizeof(PRIMITIVE) * 6 * 64> buf;
      buf.Resize(sizeof(PRIMITIVE) * 6 * nCount, FALSE);
      PRIMITIVE* pVert = reinterpret_cast<PRIMITIVE*>(buf.GetPtr());
      
      GXDWORD dwColor = COLORREF_TO_NATIVE(crClear);

      for(GXUINT i = 0; i < nCount; i++)
      {
        pVert[0].Set((float)lpRects[i].left, (float)lpRects[i].top, z, dwColor);
        pVert[1].Set((float)lpRects[i].right, (float)lpRects[i].top, z, dwColor);
        pVert[2].Set((float)lpRects[i].left, (float)lpRects[i].bottom, z, dwColor);
        pVert[3].Set((float)lpRects[i].left, (float)lpRects[i].bottom, z, dwColor);
        pVert[4].Set((float)lpRects[i].right, (float)lpRects[i].top, z, dwColor);
        pVert[5].Set((float)lpRects[i].right, (float)lpRects[i].bottom, z, dwColor);

        pVert += 6;
      }


      m_pGraphics->CreatePrimitive(&pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P4T2F_C1D), GXResUsage::Default, 6 * nCount, sizeof(PRIMITIVE), buf.GetPtr(), 0, 0, NULL);
      m_pGraphics->SetPrimitive(pPrimitive);

      m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST, 0, nCount * 2);

      // 状态恢复
      m_pGraphics->SetPrimitive(static_cast<Primitive*>(m_pPrimitive));
      m_pGraphics->IntSetEffect(m_CurrentEffect.pEffectImpl);
      m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[m_pClipRegion == NULL ? 0 : 1]);
      m_pGraphics->InlSetBlendState(m_pBlendStateImpl);

      SAFE_RELEASE(pPrimitive);
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

    BlendStateImpl* CanvasImpl::IntGetBlendStateUnsafe(CompositingMode mode) const
    {
      if(mode == CompositingMode_InvertTarget) {
        return m_pOMInvert;
      }
      else
      {
        // 判断是不是最终渲染目标
        if(mode == CompositingMode_SourceCopy) {
          return m_pOMOpaque;
        }
        else {
          return (m_pTargetTex == NULL) ? m_pBlendingState[0] : m_pBlendingState[1];
        }
      }
      CLBREAK;
      return NULL;
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
        CanvasCoreImpl::CommitState();

        m_pGraphics->SetPrimitive(m_pPrimitive);
        m_pGraphics->InlSetRasterizerState(m_pRasterizerState);
        m_pGraphics->InlSetSamplerState(0, m_pSamplerState);
        //m_pGraphics->InlSetDepthStencil(NULL);

        if(m_CurrentEffect.transform.IsValid()) { // TODO: 改为不判断形式
          m_CurrentEffect.transform = m_transform;
        }
        if(m_CurrentEffect.color_mul.IsValid()) { // TODO: 改为不判断形式
          m_CurrentEffect.color_mul = m_color_mul;
        }
        if(m_CurrentEffect.color_add.IsValid()) { // TODO: 改为不判断形式
          m_CurrentEffect.color_add = m_color_add;
        }

        m_pGraphics->IntSetEffect(m_CurrentEffect.pEffectImpl);
        UpdateStencil(m_pClipRegion);

        gxRectToRegn(&regn, &m_rcClip);
        m_pGraphics->SetSafeClip(&regn);  // TODO: 是不是应该把这个改为GXRECT
        m_pGraphics->SetViewport(NULL);

        m_pGraphics->InlSetTexture(m_pWhiteTex.ReinterpretCastPtr<TexBaseImpl>(), 0);

        //GXDWORD dwTexSlot = m_dwTexSlot;
        //if(dwTexSlot != NULL)
        //{
        //  for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && dwTexSlot != 0; i++)
        //  {
        //    if(m_aTextureStage[i] != NULL)
        //    {
        //      m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(m_aTextureStage[i]), i);
        //      RESETBIT(dwTexSlot, i);
        //    }
        //  }
        //}
      }
      return TRUE;
    }

    void CanvasImpl::IntCommitEffectCB()
    {
      static_cast<ShaderImpl*>(m_CurrentEffect.pEffectImpl->GetShaderUnsafe())->CommitConstantBuffer(m_CurrentEffect.pEffectImpl->GetDataPoolUnsafe());
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
      if(m_Commands.GetSize() == 0) {
        return FALSE;
      }

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
      GXBOOL bCommitEffectCB = TRUE;

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
          const DRAWCALL_LINELIST* pLineListCmd = pCmdPtr->cast_to<DRAWCALL_LINELIST>();
          if(bEmptyRect == FALSE) {

            if (bCommitEffectCB) {
              IntCommitEffectCB();
            }
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
          const DRAWCALL_POINTS* pPointsCmd = pCmdPtr->cast_to<DRAWCALL_POINTS>();
          if(bEmptyRect == FALSE) {
            if (bCommitEffectCB) {
              IntCommitEffectCB();
            }
            m_pGraphics->DrawPrimitive(GXPT_POINTLIST, nBaseVertex, pPointsCmd->uVertexCount);
          }
          nBaseVertex += pPointsCmd->uVertexCount;
        }
        break;
        case CF_Triangle:
        {
          TRACE_CMD("CF_Trangle\n");
          const DRAWCALL_TRIANGLELIST* pTriangleListCmd = pCmdPtr->cast_to<DRAWCALL_TRIANGLELIST>();
          if(bEmptyRect == FALSE) {
            if (bCommitEffectCB) {
              IntCommitEffectCB();
            }
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
          const DRAWCALL_TEXTURE* pTextureCmd = pCmdPtr->cast_to<DRAWCALL_TEXTURE>();

          // TODO: 需要一个Texture对象记录状态
          if(bEmptyRect == FALSE)
          {
            if (m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(pTextureCmd->pTexture), 0) == StateResult::Ok)
            {
              m_CurrentEffect.pEffectImpl->UpdateTexelSize(0, pTextureCmd->pTexture);
            }

            if (bCommitEffectCB) { // 依赖上面的TexelSize更新
              IntCommitEffectCB();
              bCommitEffectCB = FALSE;
            }
            m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST,
              nBaseVertex, 0, pTextureCmd->uVertexCount, nStartIndex, pTextureCmd->uIndexCount / 3);
          }

          nBaseVertex += pTextureCmd->uVertexCount;
          nStartIndex += pTextureCmd->uIndexCount;

          m_pGraphics->InlSetTexture(m_pWhiteTex.ReinterpretCastPtr<TexBaseImpl>(), 0);
          pTextureCmd->pTexture->Release();
        }
        break;

        case CF_SetSamplerState:
        {
          STATESWITCHING_SAMPLERSTATE* pSamplerStateCmd = const_cast<STATESWITCHING_SAMPLERSTATE*>(pCmdPtr->cast_to<STATESWITCHING_SAMPLERSTATE>());
          m_pGraphics->InlSetSamplerState(pSamplerStateCmd->sampler_slot, static_cast<SamplerStateImpl*>(pSamplerStateCmd->pSamplerState));
          SAFE_RELEASE(pSamplerStateCmd->pSamplerState);
        }
        break;

        case CF_Clear:
        {
          TRACE_CMD("CF_Clear\n");
          const STATESWITCHING_CLEAR* pClearCmd = pCmdPtr->cast_to<STATESWITCHING_CLEAR>();
          if(pClearCmd->bEntire)
          {
            const GXHRESULT hRet = m_pGraphics->Clear(NULL, 0, pClearCmd->flags, pClearCmd->color, 1.0f, m_dwStencil);
            ASSERT(GXSUCCEEDED(hRet));
          }
          else
          {
            if(m_pClipRegion == NULL)
            {
              GXRECT rect = m_rcClip;

              IntClear(&rect, 1, pClearCmd->flags, pClearCmd->color, 1.0f, m_dwStencil);
            }
            else
            {
              const GXUINT nRectCount = m_pClipRegion->GetRectCount();
              clstd::LocalBuffer<sizeof(GXRECT) * 128> _buf;
              _buf.Resize(sizeof(GXRECT) * nRectCount, FALSE);

              GXRECT* lpRects = (GXRECT*)_buf.GetPtr(); // _GlbLockStaticRects(nRectCount);
              m_pClipRegion->GetRects(lpRects, nRectCount);
              IntClear(lpRects, nRectCount, pClearCmd->flags, pClearCmd->color, 1.0f, m_dwStencil);
              //_GlbUnlockStaticRects(lpRects);
            }
          }
        }
        break;
        case CF_CompositingMode:
        {
          TRACE_CMD("CF_CompositingMode\n");
          const STATESWITCHING_COMPOSITINGMODE* pCompositingModeCmd = pCmdPtr->cast_to<STATESWITCHING_COMPOSITINGMODE>();

          BlendStateImpl* pBlendState = IntGetBlendStateUnsafe(pCompositingModeCmd->mode);
          m_pGraphics->InlSetBlendState(pBlendState);

          SAFE_RELEASE(m_pBlendStateImpl);
          m_pBlendStateImpl = pBlendState;
          m_pBlendStateImpl->AddRef();
        }
        break;

        case CF_Effect:
        {
          TRACE_CMD("CF_Effect\n");
          const STATESWITCHING_EFFECT* pEffectCmd = pCmdPtr->cast_to<STATESWITCHING_EFFECT>();

          m_CurrentEffect.InitEffect(pEffectCmd->pEffectImpl);
          if(m_CurrentEffect.transform.IsValid()) { // TODO: 改为不判断形式
            m_CurrentEffect.transform = m_transform;
          }
          if(m_CurrentEffect.color_mul.IsValid()) { // TODO: 改为不判断形式
            m_CurrentEffect.color_mul = m_color_mul;
          }
          if(m_CurrentEffect.color_add.IsValid()) { // TODO: 改为不判断形式
            m_CurrentEffect.color_add = m_color_add;
          }

          //m_pEffectImpl = pEffectCmd->pEffectImpl;
          m_pGraphics->IntSetEffect(pEffectCmd->pEffectImpl);
          pEffectCmd->pEffectImpl->Release();
        }
        break;

        case CF_ColorAdditive:
        {
          TRACE_CMD("CF_ColorAdditive\n");
          const STATESWITCHING_COLOR* pColorAddCmd = pCmdPtr->cast_to<STATESWITCHING_COLOR>();

          pColorAddCmd->color.ToFloat4(m_color_add);
          if(m_CurrentEffect.color_add.IsValid()) // TODO: 改为不判断形式
          {
            m_CurrentEffect.color_add = m_color_add;
            bCommitEffectCB = TRUE;
          }
        }
        break;

        case CF_ColorMultiply:
        {
          TRACE_CMD("CF_ColorMultiply\n");
          const STATESWITCHING_COLOR* pColorMulCmd = pCmdPtr->cast_to<STATESWITCHING_COLOR>();

          pColorMulCmd->color.ToFloat4(m_color_mul);
          if(m_CurrentEffect.color_mul.IsValid()) // TODO: 改为不判断形式
          {
            m_CurrentEffect.color_mul = m_color_mul;
            bCommitEffectCB = TRUE;
          }
        }
        break;

        case CF_SetViewportOrg:
          TRACE_CMD("CF_SetViewportOrg\n");
          break;

        case CF_SetClipBox:
        {
          TRACE_CMD("CF_SetClipBox\n");
          const STATESWITCHING_SETCLIPBOX* pClipBoxCmd = pCmdPtr->cast_to<STATESWITCHING_SETCLIPBOX>();
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
          const STATESWITCHING_SETTEXTCLIP* pTextClipCmd = pCmdPtr->cast_to<STATESWITCHING_SETTEXTCLIP>();
          GXRECT rcClip = pTextClipCmd->rect;
          GXREGN rgClip;
          gxRectToRegn(&rgClip, &rcClip);
          m_pGraphics->SetSafeClip(&rgClip);
        }
        break;
        case CF_SetRegion:
        {
          TRACE_CMD("CF_SetRegion\n");
          const STATESWITCHING_REGION* pRegionCmd = pCmdPtr->cast_to<STATESWITCHING_REGION>();
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

        //case CF_SetExtTexture:
        //{
        //  const STATESWITCHING_TEXTUREEXT* pTextureExtCmd = pCmdPtr->cast_to<STATESWITCHING_TEXTUREEXT>();

        //  const GXUINT uStage = pTextureExtCmd->stage;
        //  Texture* pTexture = pTextureExtCmd->pTexture;
        //  m_pGraphics->InlSetTexture(reinterpret_cast<TexBaseImpl*>(pTexture), uStage);
        //  SAFE_RELEASE(m_aTextureStage[uStage]);
        //  m_aTextureStage[uStage] = (TextureImpl*)pTexture;

        //  if(pTexture != NULL)
        //    SETBIT(m_dwTexSlot, uStage);
        //  else
        //    RESETBIT(m_dwTexSlot, uStage);
        //}
        //break;
        case CF_SetTransform:
        {
          const STATESWITCHING_TRANSFORM* pTransformCmd = pCmdPtr->cast_to<STATESWITCHING_TRANSFORM>();

          m_transform = pTransformCmd->transform;
         
          if(m_CurrentEffect.transform.IsValid()) { // TODO: 改为不判断形式
            m_CurrentEffect.transform = pTransformCmd->transform;
            bCommitEffectCB = TRUE;
            //IntCommitEffectCB(); // FIXME: 这里全部重新提交了CB
          }
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
      SamplerState* pSamplerState = NULL;

      if(GXSUCCEEDED(m_pGraphics->CreateSamplerState(&pSamplerState, pDesc)))
      {
        STATESWITCHING_SAMPLERSTATE* pSamplerStateCmd = IntAppendCommand<STATESWITCHING_SAMPLERSTATE>(CF_SetSamplerState);
        pSamplerStateCmd->sampler_slot = Sampler;
        pSamplerStateCmd->pSamplerState = pSamplerState;
        return TRUE;
      }
      return FALSE;
    }

    GXBOOL CanvasImpl::SetEffect(Effect* pEffect)
    {
      if(m_CallState.RenderState.pEffectImpl == pEffect) {
        return FALSE;
      }

      // 这个不增加引用计数
      EffectImpl* pEffectImpl = pEffect ? static_cast<EffectImpl*>(pEffect) : m_pDefaultEffectImpl;

      STATESWITCHING_EFFECT* pEffectCmd = IntAppendCommand<STATESWITCHING_EFFECT>(CF_Effect);
      pEffectCmd->pEffectImpl = pEffectImpl;

      SAFE_RELEASE(m_CallState.RenderState.pEffectImpl);
      m_CallState.RenderState.pEffectImpl = pEffectImpl;

      if(pEffectImpl != NULL) {
        pEffectImpl->AddRef(); // pEffectCmd->pEffectImpl
        pEffectImpl->AddRef(); // m_CallState.RenderState.pEffectImpl
      }

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
        dwRet = m_CallState.color_add.ARGB();
        if(dwRet != uParam)
        {
          m_CallState.color_add = uParam;

          STATESWITCHING_COLOR* pCommand = IntAppendCommand<STATESWITCHING_COLOR>(CF_ColorAdditive);
          pCommand->color = m_CallState.color_add;
        }
      }
      break;

      case CPI_SETCOLORMULTIPLY:
      {
        dwRet = 0;
        m_CallState.color_mul = *(float4*)pParam;

        STATESWITCHING_COLOR* pCommand = IntAppendCommand<STATESWITCHING_COLOR>(CF_ColorMultiply);
        pCommand->color = m_CallState.color_mul;
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

      //case CPI_SETEXTTEXTURE:
      //{
      //  if(uParam > 0 && uParam < GX_MAX_TEXTURE_STAGE)
      //  {
      //    STATESWITCHING_TEXTUREEXT* pCommand = IntAppendCommand<STATESWITCHING_TEXTUREEXT>(CF_SetExtTexture);
      //    pCommand->stage = uParam;
      //    pCommand->pTexture = reinterpret_cast<Texture*>(pParam);
      //    if(pCommand->pTexture) {
      //      pCommand->pTexture->AddRef();
      //    }
      //  }
      //  else {
      //    dwRet = 0;
      //  }
      //  dwRet = uParam;
      //}
      //break;
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

    GXBOOL CanvasImpl::Clear(GXCOLORREF crClear) // TODO: 改成浮点
    {
      STATESWITCHING_CLEAR* pCommand = IntAppendCommand<STATESWITCHING_CLEAR>(CF_Clear);
      pCommand->flags = GXCLEAR_TARGET;
      pCommand->color = crClear;

      if(m_CallState.rcClip.left == 0 &&
        m_CallState.rcClip.top == 0 &&
        m_CallState.rcClip.right == m_sExtent.cx &&
        m_CallState.rcClip.bottom == m_sExtent.cy)
      {
        pCommand->bEntire = TRUE;
      }
      else
      {
        pCommand->bEntire = FALSE;
      }

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
      if(lprcClip == NULL) {
        m_pGraphics->CreateRectRgn(&prgnClip, m_rcAbsClip.left, m_rcAbsClip.top, m_rcAbsClip.right, m_rcAbsClip.bottom);
      }
      else {
        m_pGraphics->CreateRectRgn(&prgnClip, lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);
      }

      ScrollTexDesc.pOperationTex = m_pTargetTex ? m_pTargetTex->GetColorTextureUnsafe(GXResUsage::Default) : NULL;
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

      m_lpLockedVertex[m_uVertCount].Set((float)(xPos + m_CallState.origin.x), (float)(yPos + m_CallState.origin.y), (GXDWORD)COLORREF_TO_NATIVE(crPixel));

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

      const GXDWORD dwColor = (GXDWORD)COLORREF_TO_NATIVE(crLine);
      m_lpLockedVertex[m_uVertCount + 0].Set(float(left + m_CallState.origin.x), float(top + m_CallState.origin.y), dwColor);
      m_lpLockedVertex[m_uVertCount + 1].Set(float(right + m_CallState.origin.x), float(bottom + m_CallState.origin.y), dwColor);

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

      const GXDWORD dwColor = COLORREF_TO_NATIVE(crRect);

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

    GXBOOL CanvasImpl::FillRegion(GRegion* pRegion, GXCOLORREF crFill)
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

    CANVAS_EFFECT_DESC::CANVAS_EFFECT_DESC()
      : pEffectImpl(NULL) {}

    CANVAS_EFFECT_DESC::~CANVAS_EFFECT_DESC()
    {
      SAFE_RELEASE(pEffectImpl);
    }

    void CANVAS_EFFECT_DESC::InitEffect(Effect* _pEffect)
    {
      SAFE_RELEASE(pEffectImpl);
      pEffectImpl = static_cast<EffectImpl*>(_pEffect);
      if(pEffectImpl)
      {
        pEffectImpl->AddRef();
        transform = pEffectImpl->GetUniform("matWVProj").CastToVariable<MOVarMatrix4>();
        color_mul = pEffectImpl->GetUniform("Color").CastToVariable<MOVarFloat4>();
        color_add = pEffectImpl->GetUniform("ColorAdd").CastToVariable<MOVarFloat4>();
      }
    }

  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)