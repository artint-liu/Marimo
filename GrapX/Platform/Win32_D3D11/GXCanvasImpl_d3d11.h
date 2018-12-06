#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_
#define _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_

#define CANVAS_SHARED_SHADER_REGCOUNT 64

class CKinematicGrid;

//extern GXRENDERSTATE s_OpaqueFinalBlend[];
//extern GXRENDERSTATE s_AlphaFinalBlend[];
//extern GXRENDERSTATE s_OpaquePreBlend[];
//extern GXRENDERSTATE s_AlphaPreBlend[];

extern "C" GXBOOL GXDLLAPI  gxSetRectEmpty  (GXLPRECT lprc);
namespace GrapX
{
  class EffectImpl;
  namespace D3D11
  {
    class GraphicsImpl;
    class TextureImpl;
    //class GRenderState;

#include "Canvas/GXCanvasCoreImpl.h"

    struct CANVAS_EFFECT_DESC
    {
      EffectImpl*   pEffectImpl;
      MOVarMatrix4  transform;
      MOVarFloat4   color_mul;
      MOVarFloat4   color_add;

      CANVAS_EFFECT_DESC();
      ~CANVAS_EFFECT_DESC();
      void InitEffect(Effect* _pEffect);

    private:
      void operator=(const CANVAS_EFFECT_DESC&){} // 禁用
    };

    struct RENDERSTATE
    {
      EffectImpl* pEffectImpl;

      RENDERSTATE() : pEffectImpl(NULL){}
      ~RENDERSTATE()
      { SAFE_RELEASE(pEffectImpl); }
    };

    class CanvasImpl : public CanvasCoreImpl
    {
      friend class GraphicsImpl;
    public:
      CanvasImpl(GraphicsImpl* pGraphics, GXBOOL bStatic);
      virtual ~CanvasImpl();
      GXBOOL  Initialize(RenderTarget* pTarget, const REGN* pRegn);

      GXINT   UpdateStencil    (GRegion* pClipRegion);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      GXULONG  GetRef();

      struct PRIMITIVE : public CANVAS_PRMI_VERT
      {
        inline void SetTexcoord(const GXFLOAT _u, const GXFLOAT _v) { u = _u; v = _v; }
        inline void Set(float _x, float _y, GXCOLORREF col)
        {
          x = _x, y = _y, z = 0, w = 1;
          u = v = 0, color = col;
        }
        inline void Set(float _x, float _y, float _z, GXCOLORREF col)
        {
          x = _x, y = _y, z = _z, w = 1;
          u = v = 0, color = col;
        }
      };


    public:
      Graphics*   GetGraphicsUnsafe   () const override;
      GXBOOL      SetTransform        (const float4x4* matTransform) override;
      GXBOOL      GetTransform        (float4x4* matTransform) const override;
      GXBOOL      SetViewportOrg      (GXINT x, GXINT y, GXLPPOINT lpPoint) override;
      GXBOOL      GetViewportOrg      (GXLPPOINT lpPoint) const override;
      GXBOOL      Flush               () override;
      GXBOOL      SetSamplerState     (GXUINT Sampler, GXSAMPLERDESC* pDesc) override;
      GXBOOL      SetEffect           (Effect* pEffect) override;
      GXDWORD     SetParametersInfo   (CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam) override;
      PenStyle    SetPenStyle         (PenStyle eStyle) override;

      GXBOOL      Clear               (GXCOLORREF crClear) override;

      GXBOOL      SetPixel            (GXINT xPos, GXINT yPos, GXCOLORREF crPixel) override;
      GXBOOL      DrawLine            (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine) override;

      inline GXBOOL       InlDrawRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crRect);
      inline GXBOOL       InlFillRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crFill);

      GXBOOL      DrawRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect) override;
      GXBOOL      DrawRectangle       (GXLPCRECT lprc, GXCOLORREF crRect) override;
      GXBOOL      DrawRectangle       (GXLPCREGN lprg, GXCOLORREF crRect) override;

      GXBOOL      FillRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill) override;
      GXBOOL      FillRectangle       (GXLPCRECT lprc, GXCOLORREF crFill) override;
      GXBOOL      FillRectangle       (GXLPCREGN lprg, GXCOLORREF crFill) override;

      GXBOOL      FillRegion          (GRegion* pRegion, GXCOLORREF crFill) override;

      GXBOOL      DrawUserPrimitive   (Texture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest) override;
      GXBOOL      DrawTexture         (Texture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation) override;

      GXINT       DrawText           (GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText) override;
      GXINT       DrawText           (GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText) override;
      GXBOOL      TextOut            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCSTR lpString, GXINT cbString, GXCOLORREF crText) override;
      GXBOOL      TextOut            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCWSTR lpString, GXINT cbString, GXCOLORREF crText) override;
      GXLONG      TabbedTextOut      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText) override;
      GXLONG      TabbedTextOut      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText) override;


      CompositingMode SetCompositingMode  (CompositingMode eMode) override;
      CompositingMode GetCompositingMode  () override;
      GXBOOL      SetRegion           (GRegion* pRegion, GXBOOL bAbsOrigin) override;
      GXBOOL      SetClipBox          (const GXLPRECT lpRect) override;
      GXINT       GetClipBox          (GXLPRECT lpRect) override;
      GXDWORD     GetStencilLevel     () override;

      GXBOOL      Scroll              (int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate) override;

    private:
      enum CanvasFunc
      {
        CF_NoOperation,  // 空的指令

        CF_DrawFirst,  // 绘图相关的
        CF_Points,
        CF_LineList,
        CF_Triangle,
        CF_Textured,
        CF_Clear,
        CF_ClearStencil,
        CF_DrawLast,

        CF_StateFirst,  // 状态相关的
        CF_SetViewportOrg,
        CF_SetRegion,
        CF_SetClipBox,
        CF_ResetClipBox,
        CF_SetTextClip,
        CF_ResetTextClip,
        CF_CompositingMode,
        CF_Effect,
        CF_ColorAdditive,
        CF_SetExtTexture,
        CF_SetSamplerState,
        CF_SetTransform,
        CF_StateLast,
      };

      struct CMDBASE
      {
        GXUINT     cbSize;
        CanvasFunc cmd;

        template<typename _Ty>
        _Ty* cast_to()
        {
          ASSERT(sizeof(_Ty) == cbSize);
          return static_cast<_Ty*>(this);
        }

        template<typename _Ty>
        const _Ty* cast_to() const
        {
          ASSERT(sizeof(_Ty) == cbSize);
          return static_cast<const _Ty*>(this);
        }

        virtual void OnMerge(){} // 合并两条命令时的清理操作
      };

      struct DRAWCALLBASE : public CMDBASE
      {
        GXUINT  uVertexCount;
        GXUINT  uIndexCount;
      };

      struct DRAWCALL_POINTS : public DRAWCALLBASE {};
      struct DRAWCALL_LINELIST : public DRAWCALLBASE {};
      struct DRAWCALL_TRIANGLELIST : public DRAWCALLBASE {};

      struct DRAWCALL_TEXTURE : public DRAWCALLBASE
      {
        Texture* pTexture;
        void OnMerge() override { CLBREAK; } // 纹理绘制不应该发生合并操作
      };

      //////////////////////////////////////////////////////////////////////////
      struct STATESWITCHING_TEXTUREEXT : public CMDBASE
      {
        GXUINT   stage;
        Texture* pTexture;

        void OnMerge() override { CLBREAK; } // 纹理切换不应该发生合并操作
      };

      struct STATESWITCHING_EFFECT : public CMDBASE
      {
        EffectImpl* pEffectImpl;
        void OnMerge() override { SAFE_RELEASE(pEffectImpl); }
      };

      struct STATESWITCHING_REGION : public CMDBASE
      {
        GRegion* pRegion;
        void OnMerge() override { SAFE_RELEASE(pRegion); }
      };

      struct STATESWITCHING_COMPOSITINGMODE : public CMDBASE
      {
        CompositingMode mode;
      };

      struct STATESWITCHING_TRANSFORM : public CMDBASE
      {
        float4x4 transform;
      };

      struct STATESWITCHING_ORIGIN : public CMDBASE
      {
        GXPOINT ptOrigin;
      };

      struct STATESWITCHING_SAMPLERSTATE : public CMDBASE
      {
        GXUINT sampler_slot;
        SamplerState* pSamplerState;
        void OnMerge() override { SAFE_RELEASE(pSamplerState); }
      };

      struct STATESWITCHING_CLEAR : public CMDBASE
      {
        GXBOOL      bEntire;
        GXDWORD     flags;
        GXCOLORREF  color;
      };

      struct STATESWITCHING_COLORADD : public CMDBASE
      {
        GXColor color;
      };

      struct STATESWITCHING_SETCLIPBOX : public CMDBASE
      {
        GXRECT rect;
      };

      struct STATESWITCHING_SETTEXTCLIP : public CMDBASE
      {
        GXRECT rect;
      };

      //////////////////////////////////////////////////////////////////////////      

      // CALLSTATE 是用来检测是否重复设置的, 这里面的值是在接口调用后立即改变的
      // 而对应类中的值是在Flush之后改变的。
      struct CALLSTATE
      {
        POINT           origin;
        GXRECT          rcClip;       // m_rcClip
        CompositingMode eCompMode;
        
        float4x4        matTransform;
        GXColor         color_mul;
        GXColor         color_add;

        RENDERSTATE     RenderState;  // 不增加引用
        CALLSTATE()
          : eCompMode       (CompositingMode_SourceCopy)
        {
          origin.x = origin.y = 0;
          gxSetRectEmpty(&rcClip);
        }
      };
    private:
      GXBOOL    CommitState        ();

      void      IntCommitEffectCB    ();

      inline GXBOOL IntCanFillPrimitive     (GXUINT uVertexCount, GXUINT uIndexCount);
      inline void _SetPrimitivePos      (GXUINT nIndex, const GXINT _x, const GXINT _y);

      template<typename _Ty>
      _Ty*     IntAppendCommandAlways   (CanvasFunc cmd); // 追加命令

      template<typename _Ty>
      _Ty*     IntAppendCommand         (CanvasFunc cmd); // 带合并的追加命令

      template<typename _Ty>
      GXUINT   IntAppendDrawCall        (_Ty** ppCmdBuffer, CanvasFunc cmd, GXUINT uVertexCount, GXUINT uIndexCount, Texture* pTextureReference);

      void     IntClear                 (const GXRECT* lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil);

      GXINT    TextOutDirect            (const INTMEASURESTRING* p, GXLPPOINT pptPosition);
      GXINT    LocalizeTabPos           (const INTMEASURESTRING* pMeasureStr, int nCurPos, int nDefaultTabWidth, int* pLastIndex);
      GXINT    MeasureStringWidth_SL    (const INTMEASURESTRING* pMeasureStr); // Single line
      GXINT    MeasureStringWidth_RN    (const INTMEASURESTRING* pMeasureStr, GXINT* pEnd); // 忘了，靠！
      GXINT    MeasureStringWidth_WB    (const INTMEASURESTRING* pMeasureStr, GXINT nWidthLimit, GXINT* pEnd); // Work Break
      GXINT    DrawText_SingleLine      (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
      GXINT    DrawText_Normal          (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
      GXINT    DrawText_WordBreak       (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);

      void    SetStencil                (GXDWORD dwStencil);
      void    IntUpdateClip             (const GXRECT& rcClip);

      BlendStateImpl* IntGetBlendStateUnsafe(CompositingMode mode) const;
    public:

    private:
      GXDWORD                 m_bStatic : 1;    // 标志是否是 GXGraphics 中的静态成员, 是的话表示在 GXGraphics 创建时已经创建好

      GXINT                   m_xAbsOrigin;     // 绝对原点位置，不受API影响
      GXINT                   m_yAbsOrigin;
      GXRECT                  m_rcAbsClip;      // Canvas 初始化的/最大的/系统的 裁剪区域, 其实这个Rect的left和top等于m_xAbsOrigin, m_yAbsOrigin

      GXDWORD                 m_dwStencil;
      GXDWORD                 m_dwTexVertColor; // 输出纹理图元时的顶点颜色，不直接参与渲染状态
      PenStyle                m_eStyle;

      GRegion*                m_pClipRegion;

      const GXUINT            s_uDefVertIndexSize = 128;

      // 预置的状态
      RasterizerStateImpl*    m_pRasterizerState;
      DepthStencilStateImpl*  m_pCanvasStencil[2]; // Canvas 用的Stencil开关,[0]关闭模板测试, [1]开启模板测试
      BlendStateImpl*         m_pBlendingState[2]; // Alpha合成方式的状态, 0: 最终合成, 1: 预先合成到纹理
      BlendStateImpl*         m_pOMOpaque;      // 不透明方式的状态
      BlendStateImpl*         m_pOMInvert;

      CANVAS_EFFECT_DESC      m_ClearEffect;     // Clear接口使用的Effect

      BlendStateImpl*         m_pOMNoColor;
      DepthStencilState*      m_pWriteStencil; // 写入模板参考值的模板状态

      // 预置纹理
      Texture*                m_pWhiteTex;

      Primitive*              m_pPrimitive;
      PRIMITIVE*              m_lpLockedVertex;
      GXUINT                  m_uVertCount;
      GXUINT                  m_uIndexCount;
      GXUINT                  m_uVertIndexSize;
      VIndex*                 m_lpLockedIndex;

      clstd::MemBuffer        m_Commands;
      CMDBASE*                m_pLastCommand;

      CALLSTATE               m_CallState;   // 记录命令队列最后的状态, 精简绘图函数的重复调用

      GXDWORD                 m_dwTexSlot;  // 用来判断是否设置了扩展纹理的标志, 减少循环之用
      TextureImpl*            m_aTextureStage[GX_MAX_TEXTURE_STAGE];    // 第一个应该总为0
      EffectImpl*             m_pDefaultEffectImpl; // 默认Effect

      // 用于Flush状态续接
      GXRECT                  m_rcClip;         // 对应 m_LastState.rcClip, 纹理的坐标空间, 在Flush阶段, 用来限制绘图区域
      CANVAS_EFFECT_DESC      m_CurrentEffect;
      float4x4                m_transform;
      float4                  m_color_mul;
      float4                  m_color_add;
    };

  } // namespace D3D11
} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
#endif // _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11