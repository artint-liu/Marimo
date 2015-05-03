class GXCanvasImpl : public GXCanvasCoreImpl
{
  friend class GXGraphicsImpl;
public:
  GXCanvasImpl(GXGraphicsImpl* pGraphics, GXBOOL bStatic);
  virtual ~GXCanvasImpl();
  GXBOOL  Initialize(GTexture* pTexture, GXCONST REGN* pRegn);
  GXBOOL  Initialize(GXImage* pImage, GXCONST REGN* pRegn);

  GXINT   UpdateStencil    (GRegion* pClipRegion);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXULONG  GetRef();

  struct PRIMITIVE : public CANVAS_PRMI_VERT
  {
    inline void SetTexcoord(const GXFLOAT _u, const GXFLOAT _v) { u = _u; v = _v; }
  };


public:
  virtual GXGraphics* GetGraphicsUnsafe   () GXCONST;
  virtual GXBOOL      SetTransform        (const float4x4* matTransform);
  virtual GXBOOL      GetTransform        (float4x4* matTransform) GXCONST;
  virtual GXBOOL      SetViewportOrg      (GXINT x, GXINT y, GXLPPOINT lpPoint);
  virtual GXBOOL      GetViewportOrg      (GXLPPOINT lpPoint) GXCONST;
  virtual GXVOID      EnableAlphaBlend    (GXBOOL bEnable);
  virtual GXBOOL      Flush               ();
  virtual GXBOOL      SetSamplerState     (GXUINT Sampler, GXSAMPLERDESC* pDesc);
  virtual GXBOOL      SetRenderState      (GXRenderStateType eType, GXDWORD dwValue);
  virtual GXBOOL      SetRenderStateBlock (GXLPCRENDERSTATE lpBlock);
  virtual GXBOOL      SetEffect           (GXEffect* pEffect);
  virtual GXBOOL      SetEffectConst      (GXLPCSTR lpName, void* pData, int nPackCount);
  virtual GXDWORD     SetParametersInfo   (CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam);
  virtual PenStyle    SetPenStyle         (PenStyle eStyle);

  virtual GXBOOL      Clear               (GXCOLORREF crClear);

  virtual GXBOOL      SetPixel            (GXINT xPos, GXINT yPos, GXCOLORREF crPixel);
  virtual GXBOOL      DrawLine            (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine);
  
  inline GXBOOL       InlDrawRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crRect);
  inline GXBOOL       InlFillRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crFill);

  virtual GXBOOL      DrawRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect);
  virtual GXBOOL      DrawRectangle       (GXLPCRECT lprc, GXCOLORREF crRect);
  virtual GXBOOL      DrawRectangle       (GXLPCREGN lprg, GXCOLORREF crRect);

  virtual GXBOOL      FillRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill);
  virtual GXBOOL      FillRectangle       (GXLPCRECT lprc, GXCOLORREF crFill);
  virtual GXBOOL      FillRectangle       (GXLPCREGN lprg, GXCOLORREF crFill);
  virtual GXBOOL      InvertRect          (GXINT x, GXINT y, GXINT w, GXINT h);

  virtual GXBOOL      ColorFillRegion     (GRegion* pRegion, GXCOLORREF crFill);

  virtual GXBOOL      DrawUserPrimitive   (GTexture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount);
  virtual GXBOOL      DrawTexture         (GTexture*pTexture, const GXREGN *rcDest);
  virtual GXBOOL      DrawTexture         (GTexture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc);
  virtual GXBOOL      DrawTexture         (GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc);
  virtual GXBOOL      DrawTexture         (GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation);

  virtual GXBOOL      DrawImage           (GXImage* pImage, const GXREGN *rgDest);
  virtual GXBOOL      DrawImage           (GXImage* pImage, GXINT xPos, GXINT yPos, const GXREGN *rgSrc);
  virtual GXBOOL      DrawImage           (GXImage* pImage, const GXREGN *rgDest, const GXREGN *rgSrc);
  virtual GXBOOL      DrawImage           (GXImage*pImage, const GXREGN* rgDest, const GXREGN* rgSrc, RotateType eRotation);

  virtual GXINT       DrawTextA           (GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText);
  virtual GXINT       DrawTextW           (GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText);
  virtual GXBOOL      TextOutA            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart,GXLPCSTR lpString,GXINT cbString, GXCOLORREF crText);
  virtual GXBOOL      TextOutW            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart,GXLPCWSTR lpString,GXINT cbString, GXCOLORREF crText);
  virtual GXLONG      TabbedTextOutA      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText);
  virtual GXLONG      TabbedTextOutW      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText);


  virtual GXINT       SetCompositingMode  (CompositingMode eMode);
  virtual GXBOOL      SetRegion           (GRegion* pRegion, GXBOOL bAbsOrigin);
  virtual GXBOOL      SetClipBox          (const GXLPRECT lpRect);
  virtual GXINT       GetClipBox          (GXLPRECT lpRect);
  virtual GXDWORD     GetStencilLevel     ();
  virtual GXBOOL      GetUniformData      (CANVASUNIFORM* pCanvasUniform);

  virtual GXBOOL      Scroll              (int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate);

  virtual GXBOOL      SetEffectUniformByName1f (const GXCHAR* pName, const float fValue);
  virtual GXBOOL      SetEffectUniformByName2f (const GXCHAR* pName, const float2* vValue);
  virtual GXBOOL      SetEffectUniformByName3f (const GXCHAR* pName, const float3* fValue);
  virtual GXBOOL      SetEffectUniformByName4f (const GXCHAR* pName, const float4* fValue);
  virtual GXBOOL      SetEffectUniformByName4x4(const GXCHAR* pName, const float4x4* pValue);

  virtual GXBOOL      SetEffectUniform1f       (const GXINT nIndex, const float fValue);
  virtual GXBOOL      SetEffectUniform2f       (const GXINT nIndex, const float2* vValue);
  virtual GXBOOL      SetEffectUniform3f       (const GXINT nIndex, const float3* fValue);
  virtual GXBOOL      SetEffectUniform4f       (const GXINT nIndex, const float4* fValue);

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
    //CF_Scroll,
    CF_DrawLast,

    CF_StateFirst,  // 状态相关的
    CF_RenderState,
    CF_SetViewportOrg,
    CF_SetRegion,
    CF_SetClipBox,
    CF_ResetClipBox,
    CF_SetTextClip,
    CF_ResetTextClip,
    CF_CompositingMode,
    CF_Effect,
    CF_ColorAdditive,
    CF_SetUniform1f,
    CF_SetUniform2f,
    CF_SetUniform3f,
    CF_SetUniform4f,
    CF_SetUniform4x4f,
    //CF_SetPixelSizeInv,
    CF_SetExtTexture,
    CF_SetEffectConst,
    CF_SetSamplerState,
    CF_SetTransform,
    CF_StateLast,
  };
  struct BATCH
  {
    CanvasFunc  eFunc;
    GXUINT      Handle;
    union{
      struct // Path, Pixel, Triangle
      {
        GXUINT  uVertexCount;
        GXUINT  uIndexCount;
      };
      struct  // Canvas' origin
      {
        GXINT   x;
        GXINT   y;
        GXINT   z;
        GXINT   w;
      }PosI;
      struct
      {
        float x;
        float y;
        float z;
        float w;
      }PosF;
      struct  // Render state
      {
        GXUINT   nRenderStateCode;
        GXDWORD  dwStateValue;
      };
      struct // qita (这是拼音!)
      {
        GXDWORD   dwFlag;
        GXWPARAM  wParam;
        GXLPARAM  lParam;
      }comm;
    };
    inline void Set(CanvasFunc _eFunc, GXUINT _uVertexCount, GXUINT _uIndexCount, GXLPARAM _lParam);
    inline void Set2(CanvasFunc _eFunc, GXINT x, GXINT y);
    inline void SetFloat4(CanvasFunc _eFunc, float x, float y, float z, float w);
    inline void SetRenderState(GXUINT nCode, GXDWORD dwValue);
  };

  // LASTSTATE 是用来检测是否重复设置的, 这里面的值是在接口设置后立即改变的
  // 而对应类中的值是在Flush之后改变的。
  struct LASTSTATE
  {
    GXINT           xOrigin;
    GXINT           yOrigin;
    GXRECT          rcClip;       // m_rcClip
    CompositingMode eCompMode;
    GXDWORD         dwColorAdditive;
    GXEffectImpl*   pEffectImpl;  // 不增加引用
    float4x4        matTransform;
    LASTSTATE() 
      : xOrigin         (0)
      , yOrigin         (0)
      , eCompMode       (CM_SourceCopy)
      , dwColorAdditive (NULL)
      , pEffectImpl     (NULL)
    {
      gxSetRectEmpty(&rcClip);
    }
  };
private:
  GXBOOL    CommitState        ();

  inline GXBOOL _CanFillBatch       (GXUINT uVertCount, GXUINT uIndexCount);
  inline void _SetPrimitivePos      (GXUINT nIndex, const GXINT _x, const GXINT _y);
  GXUINT    PrepareBatch            (CanvasFunc eFunc, GXUINT uVertCount, GXUINT uIndexCount, GXLPARAM lParam);

  GXINT    TextOutDirect            (const INTMEASURESTRING* p, GXLPPOINT pptPosition);
  GXINT    LocalizeTabPos           (const INTMEASURESTRING* pMeasureStr, int nCurPos, int nDefaultTabWidth, int* pLastIndex);
  GXINT    MeasureStringWidth_SL    (const INTMEASURESTRING* pMeasureStr); // Single line
  GXINT    MeasureStringWidth_RN    (const INTMEASURESTRING* pMeasureStr, GXINT* pEnd); // 忘了，靠！
  GXINT    MeasureStringWidth_WB    (const INTMEASURESTRING* pMeasureStr, GXINT nWidthLimit, GXINT* pEnd); // Work Break
  GXINT    DrawText_SingleLine      (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
  GXINT    DrawText_Normal          (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
  GXINT    DrawText_WordBreak       (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);

  void    SetStencil        (GXDWORD dwStencil);
public:
  inline GXCONST GXCANVASCOMMCONST& GetCommonConst() GXCONST;
  inline clBuffer&                  GetUniformBuffer();
  inline void                       GetConstBuffer(clBuffer** ppVertexBuffer, clBuffer** ppPixelBuffer);

private:
  GXDWORD       m_bStatic : 1;    // 标志是否是 GXGraphics 中的静态成员, 是的话表示在 GXGraphics 创建时已经创建好

  GXINT         m_xAbsOrigin;     // 绝对原点位置，不受API影响
  GXINT         m_yAbsOrigin;
  GXRECT        m_rcAbsClip;      // Canvas 初始化的/最大的/系统的 裁剪区域, 其实这个Rect的left和top等于m_xAbsOrigin, m_yAbsOrigin

  GXINT         m_xOrigin;        // 原点位置，可以通过函数设置  // [貌似这些值可以省略]
  GXINT         m_yOrigin;        // [貌似这些值可以省略]
  GXRECT        m_rcClip;         // 对应 m_LastState.rcClip, 纹理的坐标空间

  GXDWORD       m_dwStencil;
  GXImage*      m_pTargetImage;
  GXDWORD       m_dwTexVertColor; // 输出纹理图元时的顶点颜色
  GXDWORD       m_dwColorAdditive;
  PenStyle      m_eStyle;

  GRegion*      m_pClipRegion;

  const GXUINT  s_uDefVertIndexSize;
  const GXUINT  s_uDefBatchSize;

  GRasterizerStateImpl*   m_pRasterizerState;
  GBlendStateImpl*        m_pBlendingState[2];// Alpha合成方式的状态, 0: 最终合成, 1: 预先合成到纹理
  GBlendStateImpl*        m_pOpaqueState[2];  // 不透明方式的状态
  GDepthStencilStateImpl* m_pCanvasStencil[2];  // Canvas 用的Stencil开关,[0]关闭模板测试, [1]开启模板测试

  GPrimitiveVI* m_pPrimitive;
  PRIMITIVE*    m_lpLockedVertex;
  GXUINT        m_uVertCount;
  GXUINT        m_uIndexCount;
  GXUINT        m_uVertIndexSize;
  GXWORD*       m_lpLockedIndex;
  GTexture*     m_pWhiteTex;


  BATCH*      m_aBatch;
  GXUINT      m_uBatchCount;  // 计数 这个必须小于m_uBatchSize(不能等于)
  GXUINT      m_uBatchSize;   // 尺寸

  LASTSTATE    m_LastState;

  GXDWORD      m_dwTexSlot;  // 用来判断是否设置了扩展纹理的标志, 减少循环之用
  GTextureImpl*  m_aTextureStage[GX_MAX_TEXTURE_STAGE];    // 第一个应该总为0

  GXCANVASCOMMCONST  m_CanvasCommConst;
  clBuffer      m_UniformBuffer;  // 如果这个用了就不用下面两个
  clBuffer      m_VertexConstBuffer;
  clBuffer      m_PixelConstBuffer;
};

//class GXCanvas3DCoreImpl : public GXCanvas3D
//{

//};
GXCONST GXCANVASCOMMCONST& GXCanvasImpl::GetCommonConst() GXCONST
{
  return m_CanvasCommConst;
}
clBuffer& GXCanvasImpl::GetUniformBuffer()
{
  return m_UniformBuffer;
}
void GXCanvasImpl::GetConstBuffer(clBuffer** ppVertexBuffer, clBuffer** ppPixelBuffer)
{
  *ppVertexBuffer = &m_VertexConstBuffer;
  *ppPixelBuffer = &m_PixelConstBuffer;
}
