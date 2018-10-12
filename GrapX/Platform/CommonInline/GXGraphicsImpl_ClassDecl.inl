public:
  // 构造函数, 设置成员变量的默认值
  GXGraphicsImpl();

  virtual GXHRESULT AddRef() override;
  // 释放所有资源并自删除
  virtual GXHRESULT Release() override;

  // 设备事件
  virtual GXHRESULT Invoke              (GRESCRIPTDESC* pDesc) override;
  virtual void      GetPlatformID       (GXPlaformIdentity* pIdentity) override;

  // 激活函数,之后的所有D3D操作必须由Graphics接口实现
  virtual GXBOOL    Activate            (GXBOOL bActive) override;
  virtual GXHRESULT Begin               () override;
  virtual GXHRESULT End                 () override;
  virtual GXHRESULT Present             () override;
  virtual GXHRESULT Resize              (int nWidth, int nHeight) override;

  //virtual GXBOOL    OldRegisterResource    (GResource* pResource);
  //virtual GXUINT    OldUnregisterResource  (GResource* pResource);

  virtual GXHRESULT RegisterResource    (GResource* pResource, LPCRESKETCH pSketch) override;
  //virtual GXHRESULT RegisterResourceEx  (LPCRESKETCH pDesc, GResource* pResource);
  virtual GXHRESULT UnregisterResource  (GResource* pResource) override;

  virtual GXHRESULT SetPrimitive        (GPrimitive* pPrimitive, GXUINT uStreamSource = 0) override;
  virtual GXHRESULT SetPrimitiveV       (GPrimitiveV* pPrimitive, GXUINT uStreamSource = 0) override;
  virtual GXHRESULT SetPrimitiveVI      (GPrimitiveVI* pPrimitive, GXUINT uStreamSource = 0) override;

  // 理论上GXGraphics没有 Set** 类函数, SetTexture 例外, 因为 SetTexture 同时肩负这清空指定设备纹理的任务
  virtual GXHRESULT SetTexture            (GTextureBase* pTexture, GXUINT uStage = 0) override;
  virtual GXHRESULT SetRasterizerState    (GRasterizerState* pRasterizerState) override;
  virtual GXHRESULT SetBlendState         (GBlendState* pBlendState) override;
  virtual GXHRESULT SetDepthStencilState  (GDepthStencilState* pDepthStencilState) override;
  virtual GXHRESULT SetSamplerState       (GSamplerState* pSamplerState) override;

  virtual GXHRESULT Clear               (const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil) override;
  virtual GXHRESULT DrawPrimitive       (const GXPrimitiveType eType, const GXUINT StartVertex, const GXUINT PrimitiveCount) override;
  virtual GXHRESULT DrawPrimitive       (const GXPrimitiveType eType, const GXINT BaseVertexIndex, const GXUINT MinIndex, const GXUINT NumVertices, const GXUINT StartIndex, const GXUINT PrimitiveCount) override;

  //////////////////////////////////////////////////////////////////////////
  // 低级函数
  // GTexture

  // CreateTexture 支持 TEXSIZE_* 族的参数作为纹理的宽或高.

  // 2D Texture
  virtual GXHRESULT CreateTexture            (GTexture** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage) override;
  virtual GXHRESULT CreateTextureFromFileW   (GTexture** ppTexture, GXLPCWSTR pSrcFile) override;
  virtual GXHRESULT CreateTextureFromFileExW (GTexture** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter = GXFILTER_NONE, GXDWORD MipFilter = GXFILTER_NONE, GXCOLORREF ColorKey = 0, GXOUT LPGXIMAGEINFOX pSrcInfo = NULL) override;

  // Volume Texture
  virtual GXHRESULT CreateTexture3D           (GTexture3D** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage) override;
  virtual GXHRESULT CreateTexture3DFromFileW  (GTexture3D** ppTexture, GXLPCWSTR pSrcFile) override;
  virtual GXHRESULT CreateTexture3DFromFileExW(GTexture3D** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo) override;

  // Cube Texture
  virtual GXHRESULT CreateTextureCube           (GTextureCube** ppTexture, GXLPCSTR szName, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage) override;
  virtual GXHRESULT CreateTextureCubeFromFileW  (GTextureCube** ppTexture, GXLPCWSTR pSrcFile) override;
  virtual GXHRESULT CreateTextureCubeFromFileExW(GTextureCube** ppTexture, GXLPCWSTR pSrcFile, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo) override;


  // GStateBlock_Old
  // GRenderState

  //GXHRESULT   IntCreateRenderState        (GRenderStateImpl** pRenderState);
  GXHRESULT   IntCreateRasterizerState    (GRasterizerStateImpl** ppRasterizerState, GXRASTERIZERDESC* pRazDesc);
  GXHRESULT   IntCreateBlendState         (GBlendStateImpl** ppBlendState, GXBLENDDESC* pState, GXUINT nNum);
  GXHRESULT   IntCreateDepthStencilState  (GDepthStencilStateImpl** ppDepthStencilState, GXDEPTHSTENCILDESC* pState);
  GXHRESULT   IntCreateSamplerState       (GSamplerStateImpl** pSamplerState, GSamplerStateImpl* pDefault);

  // GPrimitive
  virtual GXHRESULT   CreatePrimitiveV  (GPrimitiveV** ppPrimitive, GXLPCSTR szName, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage, GXUINT uVertexCount, GXUINT uVertexStride, GXLPVOID pVertInitData) override;
  virtual GXHRESULT   CreatePrimitiveVI (GPrimitiveVI** ppPrimitive, GXLPCSTR szName, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage, GXUINT uIndexCount, GXUINT uVertexCount, GXUINT uVertexStride, GXLPCVOID pIdxInitData, GXLPCVOID pVertInitData) override;

  // GShader
  virtual GXHRESULT   CreateShaderFromSource  (GShader** ppShader, GXLPCSTR szShaderSource, size_t nSourceLen, GXDEFINITION* pMacroDefinition) override;
  virtual GXHRESULT   CreateShaderFromFileW   (GShader** ppShader, GXLPCWSTR szShaderDesc) override;
  virtual GXHRESULT   CreateShaderFromFileA   (GShader** ppShader, GXLPCSTR szShaderDesc) override;
  virtual GXHRESULT   CreateShaderStub        (GShaderStub** ppShaderStub) override;

  // GRegion
  //virtual GXHRESULT   CreateRectRgn           (GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom) override;
  //virtual GXHRESULT   CreateRectRgnIndirect   (GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount) override;
  //virtual GXHRESULT   CreateRoundRectRgn      (GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse) override;

  virtual GXHRESULT   CreateVertexDeclaration (GVertexDeclaration** ppVertexDecl, LPCGXVERTEXELEMENT lpVertexElement) override;
  //////////////////////////////////////////////////////////////////////////
  // 高级函数

  // GXCanvas
  virtual GXCanvas*   LockCanvas              (GXImage* pImage, const LPREGN lpRegn, GXDWORD dwFlags) override;

  virtual GXHRESULT   CreateCanvas3D          (GXCanvas3D** ppCanvas3D, GXImage* pImage, GXFormat eDepthStencil, LPCREGN lpRegn, float fNear, float fFar) override;
  virtual GXHRESULT   CreateCanvas3D          (GXCanvas3D** ppCanvas3D, GXImage* pImage, GTexture* pDepthStencil, LPCREGN lpRegn, float fNear, float fFar) override;
  virtual GXHRESULT   CreateEffect            (GXEffect** ppEffect, GShader* pShader) override;
  virtual GXHRESULT   CreateMaterial          (GXMaterialInst** ppMtlInst, GShader* pShader) override;
  virtual GXHRESULT   CreateMaterialFromFileW (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType) override;
  virtual GXHRESULT   CreateMaterialFromFileA (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType) override;

  virtual GXHRESULT   CreateRasterizerState   (GRasterizerState** ppRasterizerState, GXRASTERIZERDESC* pRazDesc) override;
  virtual GXHRESULT   CreateBlendState        (GBlendState** ppBlendState, GXBLENDDESC* pState, GXUINT nNum) override;
  virtual GXHRESULT   CreateDepthStencilState (GDepthStencilState** ppDepthStencilState, GXDEPTHSTENCILDESC* pState) override;
  virtual GXHRESULT   CreateSamplerState      (GSamplerState** ppSamplerState) override;

  // GXImage
  // TODO: 即使创建失败,也会返回一个默认图片
  // CreateImage 支持 TEXSIZE_* 族的参数指定Image的宽或者高
  virtual GXImage*    CreateImage             (GXLONG nWidth, GXLONG nHeight, GXFormat eFormat, GXBOOL bRenderable, const GXLPVOID lpBits) override;
  virtual GXImage*    CreateImageFromFile     (GXLPCWSTR lpwszFilename) override;
  virtual GXImage*    CreateImageFromTexture  (GTexture* pTexture) override;

  // GXFTFont
  virtual GXFont*     CreateFontIndirectW     (const GXLPLOGFONTW lpLogFont) override;
  virtual GXFont*     CreateFontIndirectA     (const GXLPLOGFONTA lpLogFont) override;
  virtual GXFont*     CreateFontW             (const GXULONG nWidth, const GXULONG nHeight, GXLPCWSTR pFileName) override;
  virtual GXFont*     CreateFontA             (const GXULONG nWidth, const GXULONG nHeight, GXLPCSTR pFileName) override;

  virtual GXImage*    GetBackBufferImg        () override;
  virtual GTexture*   GetBackBufferTex        () override;
  virtual GTexture*   GetDeviceOriginTex      () override;
  virtual GXBOOL      ScrollTexture           (const SCROLLTEXTUREDESC* lpScrollTexDesc) override;

  virtual GXBOOL      GetDesc                 (GXGRAPHICSDEVICE_DESC* pDesc) override;
  virtual GXBOOL      GetRenderStatistics     (RENDER_STATISTICS* pStat) override;
  virtual GXBOOL      ConvertToAbsolutePathW  (clStringW& strFilename) override;
  virtual GXBOOL      ConvertToAbsolutePathA  (clStringA& strFilename) override;
  virtual GXBOOL      ConvertToRelativePathW  (clStringW& strFilename) override;
  virtual GXBOOL      ConvertToRelativePathA  (clStringA& strFilename) override;
  virtual GXDWORD     GetCaps                 (GXGrapCapsCategory eCate) override;
  virtual void        Enter                   () override;
  virtual void        Leave                   () override;
  virtual GXBOOL      SwitchConsole           () override;

private:
  //typedef clvector<GResource*>          GResourceArray;
  //typedef clvector<GXDefinition>      ParamArray;
  //typedef GResourceArray::iterator      GResIter;

  GXBOOL    InitCommon                ();
  GXBOOL    ReleaseCommon             ();
  void      IntAttachComposerSdrDesc  (MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
  GXHRESULT IntCreateSdrFromElement   (GShader** ppShader, MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
  GXHRESULT IntCreateSdrPltDescW      (GShader** ppShader, GXLPCWSTR szShaderDesc, GXLPCSTR szPlatformSect, MTLFILEPARAMDESC* pMtlParam);
  GXLPCWSTR IntToAbsPathW             (clStringW& strOutput, GXLPCWSTR szPath); // 当 szPath 已经是绝对路径时 strOutput 不会被设置
  GXBOOL    SetSafeClip               (const GXREGN* pRegn);
  GXCanvas* AllocCanvas               ();
  //GResIter  FindResource              (GResource* pResource);
  GXHRESULT BroadcastScriptCommand    (GRESCRIPTDESC* pDesc);
  GXHRESULT BroadcastCategoryCommand  (GXDWORD dwCategoryID, GRESCRIPTDESC* pDesc);
  GXBOOL    SetViewport               (const GXVIEWPORT* pViewport);
  GXVOID    SetShaderComposerPathW    (GXLPCWSTR szPath);

  inline GXLPCSTR   InlGetPlatformStringA () const;
  inline GXBOOL     InlSetTexture         (GTexBaseImpl* pTexture, GXUINT uStage);
  inline GXBOOL     InlSetRenderTarget    (GTexture* pTexture, GXDWORD uRenderTargetIndex);
  inline GXBOOL     InlSetDepthStencil    (GTexture* pTexture);

  inline GXHRESULT  InlSetCanvas            (GXCanvasCore* pCanvasCore);
  //inline GXBOOL     InlSetRenderState       (GRenderStateImpl* pRenderState);
  template<class _TState>
  inline GXBOOL     InlSetStateT            (_TState*& pCurState, _TState* pState);
  inline GXBOOL     InlSetRasterizerState   (GRasterizerStateImpl* pRasterizerState);
  inline GXBOOL     InlSetBlendState        (GBlendStateImpl* pBlendState);
  inline GXBOOL     InlSetDepthStencilState (GDepthStencilStateImpl* pDepthStencilState);
  inline GXBOOL     InlSetSamplerState      (GSamplerStateImpl* pSamplerState);
  inline GXBOOL     InlSetShader            (GShader* pShader);
  inline GXBOOL     InlSetEffect            (GXEffectImpl* pEffect);
  inline GXHRESULT  InlSetVertexDecl        (GVertexDeclImpl* pVertexDecl);
  inline GXBOOL     IsActiveCanvas          (GXCanvasCore* pCanvasCore);
  //inline GXBOOL     IsActiveRenderState     (GRenderStateImpl* pRenderState);
  static GXVOID     IncreaseStencil         (GXDWORD* pdwStencil);

public:
  inline GXBOOL     InlIsActiveRasterizerState    (GRasterizerStateImpl* pRasterizerState);
  inline GXBOOL     InlIsActiveBlendState         (GBlendStateImpl* pBlendState);
  inline GXBOOL     InlIsActiveDepthStencilState  (GDepthStencilStateImpl* pDepthStencilState);
  inline GXBOOL     InlIsActiveSamplerState       (GSamplerStateImpl* pSamplerState);
  inline Marimo::ShaderConstName* InlGetShaderConstantNameObj();

private: // 用于管理的对象
  GXCanvasImpl**          m_aCanvasPtrCache;
  //GResourceArray          m_aResource;
  GXResourceMgr           m_ResMgr;
  //GAllocator*             m_pRgnAllocator;

private:  // 状态对象
  GRasterizerStateImpl*   m_pDefaultRasterizerState;
  GBlendStateImpl*        m_pDefaultBlendState;
  GDepthStencilStateImpl* m_pDefaultDepthStencilState;
  GSamplerStateImpl*      m_pDefaultSamplerState;

  GPrimitive*             m_pCurPrimitive;
  GTexBaseImpl*           m_pCurTexture[MAX_TEXTURE_STAGE];
  GTextureImpl*           m_pCurRenderTarget;
  GTextureImpl*           m_pCurDepthStencil;
  GShader*                m_pCurShader;
  GXCanvasCore*           m_pCurCanvasCore;
  //GRenderStateImpl*       m_pCurRenderState;
  GRasterizerStateImpl*   m_pCurRasterizerState;
  GBlendStateImpl*        m_pCurBlendState;
  GDepthStencilStateImpl* m_pCurDepthStencilState;
  GSamplerStateImpl*      m_pCurSamplerState;
  GVertexDeclImpl*        m_pCurVertexDecl;

private:  // 对象的(几个平台共用的)存储
  GXDWORD                 m_dwFlags;
  GXPlaformIdentity       m_pIdentity;
  //GXConsole*              m_pConsole;
  ILogger*                m_pLogger;
  clStringW               m_strResourceDir;
  clstd::Locker*          m_pGraphicsLocker;

  clStringW               m_strShaderComposerPath;
  MOSHADER_ELEMENT_SOURCE m_ShaderExtElement;
  Marimo::ShaderConstName*m_pShaderConstName;
  RENDER_STATISTICS       m_sStatistics;
