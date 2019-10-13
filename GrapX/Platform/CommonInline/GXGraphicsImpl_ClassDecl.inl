public:
  // 构造函数, 设置成员变量的默认值
  GraphicsImpl();

  virtual GXHRESULT AddRef() override;
  // 释放所有资源并自删除
  virtual GXHRESULT Release() override;

  // 设备事件
  virtual GXHRESULT Invoke              (GRESCRIPTDESC* pDesc) override;
  virtual GXPlatformIdentity GetPlatformID       () const override;

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

  virtual GXHRESULT SetPrimitive        (GrapX::Primitive* pPrimitive, GXUINT uStreamSource = 0) override;
  //virtual GXHRESULT SetPrimitiveV       (GPrimitiveV* pPrimitive, GXUINT uStreamSource = 0) override;
  //virtual GXHRESULT SetPrimitiveVI      (GPrimitiveVI* pPrimitive, GXUINT uStreamSource = 0) override;

  // 理论上GXGraphics没有 Set** 类函数, SetTexture 例外, 因为 SetTexture 同时肩负这清空指定设备纹理的任务
  virtual GXBOOL    SetTexture            (TextureBase* pTexture, GXUINT uStage = 0) override;
  virtual GXHRESULT SetRasterizerState    (RasterizerState* pRasterizerState) override;
  virtual GXHRESULT SetBlendState         (BlendState* pBlendState) override;
  virtual GXHRESULT SetDepthStencilState  (DepthStencilState* pDepthStencilState) override;
  virtual GXHRESULT SetSamplerState       (GXUINT slot, SamplerState* pSamplerState) override;
  virtual GXBOOL    SetSamplerState       (GXUINT nStartSlot, GXUINT nSamplerCount, SamplerState** pSamplerStateArray) override;


  virtual GXHRESULT Clear               (const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, const GXColor& crClear, GXFLOAT z, GXDWORD dwStencil) override;
  virtual GXHRESULT DrawPrimitive       (const GXPrimitiveType eType, const GXUINT StartVertex, const GXUINT PrimitiveCount) override;
  virtual GXHRESULT DrawPrimitive       (const GXPrimitiveType eType, const GXINT BaseVertexIndex, const GXUINT MinIndex, const GXUINT NumVertices, const GXUINT StartIndex, const GXUINT PrimitiveCount) override;

  //////////////////////////////////////////////////////////////////////////
  // 低级函数
  // Texture

  // CreateTexture 支持 TEXSIZE_* 族的参数作为纹理的宽或高.

  // 2D Texture
  GXHRESULT CreateTexture(Texture** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXFormat Format, GXResUsage eResUsage, GXUINT MipLevels, GXLPCVOID pInitData, GXUINT nPitch) override;
  
  GXHRESULT CreateTexture(Texture** ppTexture, GXLPCSTR szName, GXResUsage eUsage, Texture* pSourceTexture) override;


  GXHRESULT CreateTextureFromMemory  (Texture** ppTexture, GXLPCWSTR szName, clstd::Buffer* pBuffer, GXUINT MipLevels, GXResUsage eUsage) override;
  GXHRESULT CreateTextureFromFile    (Texture** ppTexture, GXLPCWSTR szFilePath, GXUINT MipLevels, GXResUsage eUsage) override;
  //virtual GXHRESULT CreateTextureFromFileEx  (Texture** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter = GXFILTER_NONE, GXDWORD MipFilter = GXFILTER_NONE, GXCOLORREF ColorKey = 0, GXOUT LPGXIMAGEINFOX pSrcInfo = NULL) override;

  // Volume Texture
  virtual GXHRESULT CreateTexture3D           (Texture3D** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage) override;
  virtual GXHRESULT CreateTexture3DFromFile   (Texture3D** ppTexture, GXLPCWSTR pSrcFile) override;
  //virtual GXHRESULT CreateTexture3DFromFileEx (Texture3D** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo) override;

  // Cube Texture
  GXBOOL CreateTextureCube(TextureCube** ppTexture, GXLPCSTR szName, GXUINT Size, GXFormat Format, GXResUsage ResUsage, GXUINT MipLevels, GXLPCVOID pInitData, GXUINT nPitch) override;
  GXBOOL CreateTextureCubeFromMemory(TextureCube** ppTexture, GXLPCWSTR szName, clstd::Buffer* pBuffer, GXUINT MipLevels, GXResUsage eUsage) override;
  GXBOOL CreateTextureCubeFromFile(TextureCube** ppTexture, GXLPCWSTR pSrcFile, GXUINT MipLevels, GXResUsage eUsage) override;
  //virtual GXHRESULT CreateTextureCubeFromFileEx (TextureCube** ppTexture, GXLPCWSTR pSrcFile, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo) override;


  // GStateBlock_Old
  // GRenderState

  GXHRESULT   IntCreateRasterizerState    (RasterizerStateImpl** ppRasterizerState, GXRASTERIZERDESC* pRazDesc);
  GXHRESULT   IntCreateBlendState         (BlendStateImpl** ppBlendState, GXBLENDDESC* pState, GXUINT nNum);
  GXHRESULT   IntCreateDepthStencilState  (DepthStencilStateImpl** ppDepthStencilState, GXDEPTHSTENCILDESC* pState);
  GXHRESULT   IntCreateSamplerState       (SamplerStateImpl** pSamplerState, const GXSAMPLERDESC* pDesc);

  // GPrimitive
  GXHRESULT CreatePrimitive               (GrapX::Primitive** pPrimitive, GXLPCSTR szName, GXLPCVERTEXELEMENT pVertexDecl, GXResUsage eResUsage,
                                          GXUINT uVertexCount, GXUINT uVertexStride, GXLPCVOID pVertInitData,
                                          GXUINT uIndexCount, GXUINT uIndexSize, GXLPCVOID pIndexInitData) override;

  // GShader
  GXHRESULT   CreateShaderFromSource  (Shader** ppShader, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount) override;
  //GXHRESULT   CreateShaderFromSource  (GShader** ppShader, GXLPCSTR szShaderSource, size_t nSourceLen, GXDEFINITION* pMacroDefinition) override;
  GXHRESULT   CreateShaderFromFile    (Shader** ppShader, GXLPCWSTR szShaderDesc) override;
  GXHRESULT   CreateShaderFromFile    (Shader** ppShader, GXLPCSTR szShaderDesc) override;
  //GXHRESULT   CreateShaderStub        (GShaderStub** ppShaderStub) override;

  virtual GXHRESULT   CreateVertexDeclaration (GVertexDeclaration** ppVertexDecl, LPCGXVERTEXELEMENT lpVertexElement) override;
  //////////////////////////////////////////////////////////////////////////
  // 高级函数

  // GXCanvas
  virtual Canvas*   LockCanvas              (RenderTarget* pTarget, const LPREGN lpRegn, GXDWORD dwFlags) override;

  GXBOOL      CreateCanvas3D          (Canvas3D** ppCanvas3D, RenderTarget** pTargetArray, GXUINT nCount, LPCREGN lpRegn, float fNear, float fFar) override;
  GXBOOL      CreateCanvas3D          (Canvas3D** ppCanvas3D, RenderTarget* pTarget, LPCREGN lpRegn, float fNear, float fFar) override;
  //virtual GXHRESULT   CreateCanvas3D          (GXCanvas3D** ppCanvas3D, RenderTarget* pTarget, LPCREGN lpRegn, float fNear, float fFar) override;
  GXHRESULT   CreateEffect            (Effect** ppEffect, Shader* pShader) override;
  GXHRESULT   CreateMaterial          (Material** ppMtlInst, Shader* pShader) override;
  GXHRESULT   CreateMaterialFromFile  (Material** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType) override;
  //virtual GXHRESULT   CreateMaterialFromFile  (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType) override;

  GXHRESULT   CreateRasterizerState   (RasterizerState** ppRasterizerState, GXRASTERIZERDESC* pRazDesc) override;
  GXHRESULT   CreateBlendState        (BlendState** ppBlendState, GXBLENDDESC* pState, GXUINT nNum) override;
  GXHRESULT   CreateDepthStencilState (DepthStencilState** ppDepthStencilState, GXDEPTHSTENCILDESC* pState) override;
  GXHRESULT   CreateSamplerState      (SamplerState** ppSamplerState, const GXSAMPLERDESC* pDesc) override;

  // GXImage
  // TODO: 即使创建失败,也会返回一个默认图片
  // CreateImage 支持 TEXSIZE_* 族的参数指定Image的宽或者高
  //virtual GXImage*    CreateImage             (GXLONG nWidth, GXLONG nHeight, GXFormat eFormat, GXBOOL bRenderable, const GXLPVOID lpBits) override;
  //virtual GXImage*    CreateImageFromFile     (GXLPCWSTR lpwszFilename) override;
  //virtual GXImage*    CreateImageFromTexture  (Texture* pTexture) override;

  GXHRESULT CreateRenderTarget(RenderTarget** ppRenderTarget, GXLPCWSTR szName, GXINT nWidth, GXINT nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat) override;

  GXHRESULT CreateRenderTarget(RenderTarget** ppRenderTarget, GXLPCWSTR szName, GXSizeRatio nWidth, GXINT nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat) override
  {
    return CreateRenderTarget(ppRenderTarget, szName, static_cast<GXINT>(nWidth), nHeight, eColorFormat, eDepthStencilFormat);
  }

  GXHRESULT CreateRenderTarget(RenderTarget** ppRenderTarget, GXLPCWSTR szName, GXINT nWidth, GXSizeRatio nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat) override
  {
    return CreateRenderTarget(ppRenderTarget, szName, nWidth, static_cast<GXINT>(nHeight), eColorFormat, eDepthStencilFormat);
  }
  
  GXHRESULT CreateRenderTarget(RenderTarget** ppRenderTarget, GXLPCWSTR szName, GXSizeRatio nWidth, GXSizeRatio nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat) override
  {
    return CreateRenderTarget(ppRenderTarget, szName, static_cast<GXINT>(nWidth), static_cast<GXINT>(nHeight), eColorFormat, eDepthStencilFormat);
  }


  // GXFTFont
  virtual GXFont*     CreateFontIndirect      (const GXLPLOGFONTW lpLogFont) override;
  virtual GXFont*     CreateFontIndirect      (const GXLPLOGFONTA lpLogFont) override;
  virtual GXFont*     CreateFont              (const GXULONG nWidth, const GXULONG nHeight, GXLPCWSTR pFileName) override;
  virtual GXFont*     CreateFont              (const GXULONG nWidth, const GXULONG nHeight, GXLPCSTR pFileName) override;

  //virtual GXImage*    GetBackBufferImg        () override;
  //virtual GTexture*   GetBackBufferTex        () override;
  GXHRESULT   GetBackBuffer           (RenderTarget** ppTarget) override;
  Texture*    GetDeviceOriginTex      () override;
  GXBOOL      ScrollTexture           (const SCROLLTEXTUREDESC* lpScrollTexDesc) override;

  GXBOOL      GetDesc                 (GXGRAPHICSDEVICE_DESC* pDesc) override;
  GXBOOL      GetRenderStatistics     (RENDER_STATISTICS* pStat) override;
  GXBOOL      ConvertToAbsolutePathW  (clStringW& strFilename) override;
  GXBOOL      ConvertToAbsolutePathA  (clStringA& strFilename) override;
  GXBOOL      ConvertToRelativePathW  (clStringW& strFilename) override;
  GXBOOL      ConvertToRelativePathA  (clStringA& strFilename) override;
  GXDWORD     GetCaps                 (GXGrapCapsCategory eCate) override;
  void        Enter                   () override;
  void        Leave                   () override;
  GXBOOL      SwitchConsole           () override;

private:
  //typedef clvector<GResource*>          GResourceArray;
  //typedef clvector<GXDefinition>      ParamArray;
  //typedef GResourceArray::iterator      GResIter;

  GXBOOL    InitCommon                ();
  GXBOOL    ReleaseCommon             ();
  void      IntAttachComposerSdrDesc  (MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
  GXHRESULT IntCreateSdrFromElement   (Shader** ppShader, MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
  GXHRESULT IntCreateSdrPltDescW      (Shader** ppShader, GXLPCWSTR szShaderDesc, GXLPCSTR szPlatformSect, MTLFILEPARAMDESC* pMtlParam);
  GXLPCWSTR IntToAbsPathW             (clStringW& strOutput, GXLPCWSTR szPath); // 当 szPath 已经是绝对路径时 strOutput 不会被设置
  GXBOOL    SetSafeClip               (const GXREGN* pRegn);
  Canvas* AllocCanvas               ();
  //GResIter  FindResource              (GResource* pResource);
  GXHRESULT BroadcastScriptCommand    (GRESCRIPTDESC* pDesc);
  GXHRESULT BroadcastCategoryCommand  (GXDWORD dwCategoryID, GRESCRIPTDESC* pDesc);
  GXBOOL    SetViewport               (const GXVIEWPORT* pViewport);
  GXVOID    SetShaderComposerPathW    (GXLPCWSTR szPath);

  inline GXLPCSTR     InlGetPlatformStringA () const;
  inline StateResult  InlSetTexture         (TexBaseImpl* pTexture, GXUINT uStage);
  GXBOOL     InlSetRenderTarget    (RenderTarget* pTarget, GXUINT uRenderTar);
  GXBOOL     InlSetRenderTarget    (Canvas3DImpl* pCanvas3D);
  //inline GXBOOL     InlSetDepthStencil    (Texture* pTexture);

  GXHRESULT  InlSetCanvas            (CanvasCore* pCanvasCore);
  GXBOOL     IntSetEffect            (EffectImpl* pEffect);
  //inline GXBOOL     InlSetRenderState       (GRenderStateImpl* pRenderState);
  template<class _TState>
  inline GXBOOL     InlSetStateT            (GXUINT slot, _TState*& pCurState, _TState* pState);
  inline GXBOOL     InlSetRasterizerState   (RasterizerStateImpl* pRasterizerState);
  inline GXBOOL     InlSetBlendState        (BlendStateImpl* pBlendState);
  inline GXBOOL     InlSetDepthStencilState (DepthStencilStateImpl* pDepthStencilState);
  inline GXBOOL     InlSetSamplerState      (GXUINT slot, SamplerStateImpl* pSamplerState);
  inline GXBOOL     InlSetShader            (Shader* pShader);
  inline GXHRESULT  InlSetVertexDecl        (VertexDeclImpl* pVertexDecl);
  inline GXBOOL     IsActiveCanvas          (CanvasCore* pCanvasCore);
  //inline GXBOOL     IsActiveRenderState     (GRenderStateImpl* pRenderState);
  static GXVOID     IncreaseStencil         (GXDWORD* pdwStencil);

public:
  inline GXBOOL     InlIsActiveRasterizerState    (RasterizerStateImpl* pRasterizerState);
  inline GXBOOL     InlIsActiveBlendState         (BlendStateImpl* pBlendState);
  inline GXBOOL     InlIsActiveDepthStencilState  (DepthStencilStateImpl* pDepthStencilState);
  inline GXBOOL     InlIsActiveSamplerState       (SamplerStateImpl* pSamplerState);
  inline Marimo::ShaderConstName* InlGetShaderConstantNameObj()
  {
    return m_pShaderConstName;
  }

private: // 用于管理的对象
  CanvasImpl**          m_aCanvasPtrCache;
  //GResourceArray          m_aResource;
  GXResourceMgr           m_ResMgr;
  //GAllocator*             m_pRgnAllocator;

private:  // 状态对象
  RasterizerStateImpl*    m_pDefaultRasterizerState;
  BlendStateImpl*         m_pDefaultBlendState;
  DepthStencilStateImpl*  m_pDefaultDepthStencilState;
  SamplerStateImpl*       m_pDefaultSamplerState;

  Primitive*              m_pCurPrimitive;
  TexBaseImpl*            m_pCurTexture[MAX_TEXTURE_STAGE];
  RenderTargetImpl*       m_pCurRenderTarget;
  //GTextureImpl*           m_pCurDepthStencil;
  Shader*                 m_pCurShader;
  CanvasCore*             m_pCurCanvasCore;
  //GRenderStateImpl*       m_pCurRenderState;
  RasterizerStateImpl*    m_pCurRasterizerState;
  BlendStateImpl*         m_pCurBlendState;
  DepthStencilStateImpl*  m_pCurDepthStencilState;
  SamplerStateImpl*       m_pCurSamplerState;
  VertexDeclImpl*         m_pCurVertexDecl;

  Texture*                m_pWhiteTexture8x8;

private:  // 对象的(几个平台共用的)存储
  GXDWORD                 m_dwFlags;
  GXPlatformIdentity       m_pIdentity;
  //GXConsole*              m_pConsole;
  ILogger*                m_pLogger;
  clStringW               m_strResourceDir;
  clstd::Locker*          m_pGraphicsLocker;

  clStringW               m_strShaderComposerPath;
  MOSHADER_ELEMENT_SOURCE m_ShaderExtElement;
  Marimo::ShaderConstName*m_pShaderConstName;
  RENDER_STATISTICS       m_sStatistics;
