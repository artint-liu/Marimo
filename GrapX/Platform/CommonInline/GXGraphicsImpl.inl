GXBOOL GraphicsImpl::InitCommon()
{
  ASSERT(m_pGraphicsLocker == NULL);

  m_pGraphicsLocker = new clstd::Locker;

  m_pShaderConstName = new Marimo::ShaderConstName(this);

  // Canvas 缓冲
  m_aCanvasPtrCache = new CanvasImpl*[s_uCanvasCacheCount];
  for(GXUINT i = 0; i < s_uCanvasCacheCount; i++)
    m_aCanvasPtrCache[i] = new CanvasImpl(this, TRUE);

  //GRenderState::InitializeStatic();
  //SamplerStateImpl::InitializeStatic();
  MaterialImpl::InitializeMtlStateDict();
  //IntCreateRenderState(&m_pCurRenderState);

  ASSERT(m_pCurRasterizerState == NULL);
  ASSERT(m_pDefaultBlendState == NULL);
  ASSERT(m_pDefaultDepthStencilState == NULL);
  ASSERT(m_pCurBlendState == NULL);
  ASSERT(m_pCurDepthStencilState == NULL);

  // Rasterizer State
  GXRASTERIZERDESC RasterizerDesc;
  if(GXFAILED(CreateRasterizerState((RasterizerState**)&m_pDefaultRasterizerState, &RasterizerDesc))) {
    CLOG_ERROR("%s : Create rasterizer state error.\n", __FUNCTION__);
  }
  m_pCurRasterizerState = m_pDefaultRasterizerState;
  m_pCurRasterizerState->AddRef();
  // --Rasterizer State

  // Blend State
  GXBLENDDESC BlendDesc;  // 自动构造默认参数
  if(GXFAILED(CreateBlendState((BlendState**)&m_pDefaultBlendState, &BlendDesc, 1))) {
    CLOG_ERROR("%s : Create default blend state error.\n", __FUNCTION__);
  }
  m_pCurBlendState = m_pDefaultBlendState;
  m_pCurBlendState->AddRef();
  // --Blend State

  GXDEPTHSTENCILDESC DepthStencilDesc(TRUE, FALSE);
  if(GXFAILED(CreateDepthStencilState((DepthStencilState**)&m_pDefaultDepthStencilState, &DepthStencilDesc))) {
    CLOG_ERROR("%s : Create depth stencil state error.\n", __FUNCTION__);
  }
  m_pCurDepthStencilState = m_pDefaultDepthStencilState;
  m_pCurDepthStencilState->AddRef();

  GXSamplerDesc sampler_desc;
  IntCreateSamplerState(&m_pDefaultSamplerState, sampler_desc);
  m_pCurSamplerState = m_pDefaultSamplerState;
  m_pCurSamplerState->AddRef();


  //CreateTexture(&m_pBackBufferTex, NULL, TEXSIZE_SAME, TEXSIZE_SAME, 1, GXFMT_A8R8G8B8, GXRU_TEX_RENDERTARGET);
  CreateRenderTarget(&m_pDefaultBackBuffer, NULL, GXSizeRatio::Same, GXSizeRatio::Same, GXFMT_A8R8G8B8, Format_D24S8);

  GXDWORD white[8 * 8];
  memset(white, 0xff, sizeof(white));
  CreateTexture(&m_pWhiteTexture8x8, "White8x8", 8, 8, Format_B8G8R8A8, GXResUsage::Default, 1, white, 0);
  //m_pBackBufferImg = CreateImageFromTexture(m_pBackBufferTex);

  //m_pShaderMgr = new GXShaderMgr(this);
  //m_pShaderMgr->Initialize();

  // 创建设备相关对象
  //OnDeviceEvent(DE_ResetDevice);
  INVOKE_RESET_DEVICE; // TODO: make D3D9 only!!
  return TRUE;
}

GXBOOL GraphicsImpl::ReleaseCommon()
{
  MaterialImpl::FinalizeMtlStateDict(); // 这个不应该在这儿, 如果多个GXGraphics第一个被释放的话会导致第二个材质出错.
  SAFE_RELEASE(m_pDefaultRasterizerState);
  SAFE_RELEASE(m_pDefaultBlendState);
  SAFE_RELEASE(m_pDefaultDepthStencilState);
  SAFE_RELEASE(m_pDefaultSamplerState);

  SAFE_RELEASE(m_pCurRasterizerState);
  SAFE_RELEASE(m_pCurBlendState);
  SAFE_RELEASE(m_pCurDepthStencilState);
  SAFE_RELEASE(m_pCurSamplerState);

  SAFE_DELETE(m_pShaderConstName);
  SAFE_RELEASE(m_pWhiteTexture8x8);

  SAFE_RELEASE(m_pBasicShader);
  SAFE_RELEASE(m_pBasicEffect);

  // 释放 Canvas 缓冲
  if(m_aCanvasPtrCache != NULL)
  {
    for(GXUINT i = 0; i < s_uCanvasCacheCount; i++)
      SAFE_RELEASE(m_aCanvasPtrCache[i]);
    delete[] m_aCanvasPtrCache;
    m_aCanvasPtrCache = NULL;  
  }

  return TRUE;
}

GXLPCWSTR GraphicsImpl::IntToAbsPathW(clStringW& strOutput, GXLPCWSTR szPath)
{
  ASSERT(m_strResourceDir.IsNotEmpty());
  if(clpathfile::IsRelative(szPath)) {
    clpathfile::CombinePath(strOutput, m_strResourceDir, szPath);
    return strOutput;
  }
  return szPath;
}

GXBOOL GraphicsImpl::ConvertToAbsolutePathW(clStringW& strFilename)
{
  ASSERT(m_strResourceDir.IsNotEmpty());
  //if(strFilename.IsEmpty() || IsFullPath(strFilename) == TRUE) {
  //  return FALSE;
  //}
  clpathfile::CombinePath(strFilename, m_strResourceDir, strFilename);
  return TRUE;
}

GXBOOL GraphicsImpl::ConvertToAbsolutePathA(clStringA& strFilename)
{
  if(strFilename.IsEmpty()) {
    return FALSE;
  }
  clStringW strFilenameW = strFilename;
  GXBOOL bval = ConvertToAbsolutePathW(strFilenameW);
  strFilename = strFilenameW;
  return bval;
}

GXBOOL GraphicsImpl::ConvertToRelativePathW(clStringW& strFilename)
{
  if(clpathfile::IsRelative(strFilename)) {
    return TRUE;
  }
  
  // 没有实现计算相对目录
  CLBREAK; 

  ASSERT(m_strResourceDir.IsNotEmpty());
  //if(strFilename.IsEmpty() || IsFullPath(strFilename) == TRUE) {
  //  return FALSE;
  //}
  clpathfile::CombinePath(strFilename, m_strResourceDir, strFilename);
  return TRUE;
}

GXBOOL GraphicsImpl::ConvertToRelativePathA(clStringA& strFilename)
{
  if(strFilename.IsEmpty()) {
    return FALSE;
  }
  clStringW strFilenameW = strFilename;
  GXBOOL bval = ConvertToRelativePathW(strFilenameW);
  strFilename = strFilenameW;
  return bval;
}

GXHRESULT GraphicsImpl::IntCreateRasterizerState(RasterizerStateImpl** ppRasterizerState, GXRASTERIZERDESC* pRazDesc)
{
  RasterizerStateImpl* pRasterizerState = new RasterizerStateImpl(this);
  if(! InlCheckNewAndIncReference(pRasterizerState)) {
    return GX_FAIL;
  }
  if( ! pRasterizerState->Initialize(pRazDesc))
  {
    pRasterizerState->Release();
    pRasterizerState = NULL;
    return GX_FAIL;
  }
  *ppRasterizerState = pRasterizerState;
  return GX_OK;
}

GXHRESULT GraphicsImpl::IntCreateBlendState(BlendStateImpl** ppBlendState, GXBLENDDESC* pState, GXUINT nNum)
{
  BlendStateImpl* pBlendState = new BlendStateImpl(this);
  if(pBlendState != NULL)
  {
    pBlendState->AddRef();
    if(pBlendState->Initialize(pState, nNum)) {
      *ppBlendState = pBlendState;
      return GX_OK;
    }
    SAFE_RELEASE(pBlendState);
  }
  return GX_FAIL;
}

GXHRESULT GraphicsImpl::IntCreateDepthStencilState(DepthStencilStateImpl** ppDepthStencilState, GXDEPTHSTENCILDESC* pState)
{
  DepthStencilStateImpl* pDepthStencilState = new DepthStencilStateImpl(this);
  if(pDepthStencilState != NULL)
  {
    pDepthStencilState->AddRef();
    if(pDepthStencilState->Initialize(pState)) {
      *ppDepthStencilState = pDepthStencilState;
      return GX_OK;
    }
    SAFE_RELEASE(pDepthStencilState);
  }
  return GX_FAIL;
}

GXHRESULT GraphicsImpl::CreateRasterizerState(RasterizerState** ppRasterizerState, GXRASTERIZERDESC* pRazDesc)
{
  GRESKETCH ResFeatDesc;
  GrapX::Internal::ResourceSketch::GenerateRasterizerState(&ResFeatDesc, pRazDesc);
  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL)
  {
    *ppRasterizerState = static_cast<RasterizerState*>(pResource);
    return pResource->AddRef();
  }
  GXHRESULT hval = IntCreateRasterizerState((RasterizerStateImpl**)ppRasterizerState, pRazDesc);
  if(GXSUCCEEDED(hval))
  {
    RegisterResource(*ppRasterizerState, &ResFeatDesc);
  }
  return hval;
}

GXHRESULT GraphicsImpl::CreateBlendState(BlendState** ppBlendState, GXBLENDDESC* pState, GXUINT nNum)
{
  GRESKETCH ResFeatDesc;
  GrapX::Internal::ResourceSketch::GenerateBlendState(&ResFeatDesc, pState);
  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL)
  {
    *ppBlendState = static_cast<BlendState*>(pResource);
    return pResource->AddRef();
  }
  GXHRESULT hval = IntCreateBlendState((BlendStateImpl**)ppBlendState, pState, nNum);
  if(GXSUCCEEDED(hval))
  {
    RegisterResource(*ppBlendState, &ResFeatDesc);
  }
  return hval;
}

GXHRESULT GraphicsImpl::CreateDepthStencilState(DepthStencilState** ppDepthStencilState, GXDEPTHSTENCILDESC* pState)
{
  GRESKETCH ResFeatDesc;
  GrapX::Internal::ResourceSketch::GenerateDepthStencilState(&ResFeatDesc, pState);
  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL)
  {
    *ppDepthStencilState = static_cast<DepthStencilState*>(pResource);
    return pResource->AddRef();
  }
  GXHRESULT hval = IntCreateDepthStencilState((DepthStencilStateImpl**)ppDepthStencilState, pState);
  if(GXSUCCEEDED(hval))
  {
    m_ResMgr.Register(&ResFeatDesc, *ppDepthStencilState);
  }
  return hval;
}

GXHRESULT GraphicsImpl::CreateSamplerState(SamplerState** ppSamplerState, const GXSAMPLERDESC* pDesc)
{
  return IntCreateSamplerState((SamplerStateImpl**)ppSamplerState, pDesc);
}

GXHRESULT GraphicsImpl::CreatePrimitive(
  Primitive** ppPrimitive, GXLPCSTR szName, GXLPCVERTEXELEMENT pVertexDecl, GXResUsage eResUsage,
  GXUINT uVertexCount, GXUINT uVertexStride, GXLPCVOID pVertInitData,
  GXUINT uIndexCount, GXUINT uIndexSize, GXLPCVOID pIndexInitData)
{
  // uIndexCount = 0 时，没有设置索引数量，与索引有关的参数将被忽略
  // uVertexStride = 0, 按照pVertexDecl设置顶点结构的大小

  GRESKETCH rs = { RCC_NamedPrimitive };

  // 按命名查找资源
  if(szName != NULL && szName[0] != '\0')
  {
    rs.strResourceName.Append(_CLTEXT("<Primitive>")).Append(szName);
    GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppPrimitive), &rs);
    if(GXSUCCEEDED(hr)) {
      return hr;
    }
  }

  // 参数检查
  if(eResUsage == GXResUsage::Default &&
    (pVertInitData == NULL || (uIndexCount > 0 && pIndexInitData == NULL))) {
    CLOG_ERROR("CreatePrimitive: GXResUsage_Default创建的Primitive必须指定初始化数据，因为在之后不能再改变");
    return GX_FAIL;
  }
  else if(uIndexCount > 0 && uIndexSize != 2 && uIndexSize != 4) {
    CLOG_ERROR("CreatePrimitive: 如果需要创建索引，uIndexSize只能是2或者4");
    return GX_FAIL;
  }
  else if(uVertexStride > 0 && uVertexStride < MOGetDeclVertexSize(pVertexDecl)) {
    CLOG_ERROR("CreatePrimitive: uVertexStridez设置的步长要小于顶点声明的实际步长");
    return GX_FAIL;
  }

  // 创建
  if(uIndexCount == 0)
  {
    GPrimitiveVertexOnlyImpl* pPrimitiveVertex =
      new GPrimitiveVertexOnlyImpl(this, eResUsage, uVertexCount, uVertexStride);

    if(InlIsFailedToNewObject(pPrimitiveVertex)) {
      return GX_FAIL;
    }

    if(_CL_NOT_(pPrimitiveVertex->InitPrimitive(pVertexDecl, pVertInitData)))
    {
      SAFE_RELEASE(pPrimitiveVertex);
      return GX_FAIL;
    }
    *ppPrimitive = pPrimitiveVertex;
  }
  else
  {
    GPrimitiveVertexIndexImpl* pPrimitiveVertexIndex =
      new GPrimitiveVertexIndexImpl(this, eResUsage, uVertexCount, uVertexStride, uIndexCount, uIndexSize);

    if(InlIsFailedToNewObject(pPrimitiveVertexIndex)) {
      return GX_FAIL;
    }

    if(_CL_NOT_(pPrimitiveVertexIndex->InitPrimitive(pVertexDecl, pVertInitData, pIndexInitData)))
    {
      SAFE_RELEASE(pPrimitiveVertexIndex);
      return GX_FAIL;         
    }
    *ppPrimitive = pPrimitiveVertexIndex;
  }

  return RegisterResource(*ppPrimitive, rs.dwCategoryId ? &rs : NULL);
}

//GXHRESULT GXGraphicsImpl::CreatePrimitiveV(
//  GPrimitiveV**       ppPrimitive, 
//  GXLPCSTR            szName, 
//  LPCGXVERTEXELEMENT  pVertexDecl, 
//  GXDWORD             ResUsage, 
//  GXUINT              uVertexCount, 
//  GXUINT              uVertexStride, 
//  GXLPVOID            pVertInitData)
//{
//  GPrimitiveVertexOnlyImpl* pPrimitiveImpl = NULL;
//
//  pPrimitiveImpl = new GPrimitiveVertexOnlyImpl(this);
//  if(pPrimitiveImpl != NULL)
//  {
//    pPrimitiveImpl->AddRef();
//    RegisterResource(pPrimitiveImpl, NULL);
//    if(pPrimitiveImpl->InitPrimitive(pVertInitData, uVertexCount, 
//      uVertexStride != 0 ? uVertexStride : MOGetDeclVertexSize(pVertexDecl), 
//      pVertexDecl, ResUsage) != FALSE)
//    {
//      *ppPrimitive = pPrimitiveImpl;
//      return GX_OK;
//    }
//    SAFE_RELEASE(pPrimitiveImpl);
//  }
//
//  // new 失败
//  CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
//  *ppPrimitive = pPrimitiveImpl;
//  return GX_FAIL;
//}
//
//GXHRESULT GXGraphicsImpl::CreatePrimitiveVI(
//  GPrimitiveVI**      ppPrimitive, 
//  GXLPCSTR            szName, 
//  LPCGXVERTEXELEMENT  pVertexDecl, 
//  GXDWORD             ResUsage, 
//  GXUINT              uIndexCount, 
//  GXUINT              uVertexCount, 
//  GXUINT              uVertexStride, 
//  GXLPCVOID           pIdxInitData, 
//  GXLPCVOID           pVertInitData)
//{
//  GPrimitiveVertexIndexImpl* pPrimitiveImpl = NULL;
//
//  if((pIdxInitData == NULL && pVertInitData != NULL) ||
//    (pIdxInitData != NULL && pVertInitData == NULL)) {
//      CLOG_ERROR("CreatePrimitiveVI: 顶点与数组初始化数据必须同时提供,或全同时为空.\n");
//      return GX_FAIL;
//  }
//
//  pPrimitiveImpl = new GPrimitiveVertexIndexImpl(this);
//
//  if(pPrimitiveImpl != NULL)
//  {
//    pPrimitiveImpl->AddRef();
//    RegisterResource(pPrimitiveImpl, NULL);
//
//    if(pPrimitiveImpl->InitPrimitive(pVertInitData, uVertexCount, 
//      uVertexStride != 0 ? uVertexStride : MOGetDeclVertexSize(pVertexDecl),
//      pIdxInitData, uIndexCount, pVertexDecl, ResUsage) != FALSE)
//    {
//      *ppPrimitive = pPrimitiveImpl;
//      return GX_OK;
//    }
//    SAFE_RELEASE(pPrimitiveImpl);
//  }
//
//  // new 失败
//  CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
//  *ppPrimitive = pPrimitiveImpl;
//  return GX_FAIL;
//}
//////////////////////////////////////////////////////////////////////////
//GXImage* GXGraphicsImpl::CreateImage(
//  GXLONG nWidth, GXLONG nHeight, GXFormat eFormat, 
//  GXBOOL bRenderable, const GXLPVOID lpBits)
//{
//  GXImageImpl* pImage = NULL;
//  pImage = new GXImageImpl(this, nWidth, nHeight, eFormat);
//  pImage->AddRef();
//  RegisterResource(pImage, NULL);
//  if(pImage->Initialize(bRenderable, NULL, lpBits) == FALSE)
//  {
//    SAFE_RELEASE(pImage);
//    return NULL;
//  }
//  return pImage;
//}

//////////////////////////////////////////////////////////////////////////
GXHRESULT GraphicsImpl::CreateEffect(Effect** ppEffect, Shader* pShader)
{
  if(pShader == NULL) {
    return GX_FAIL;
  }
  
  EffectImpl* pEffect = new EffectImpl(this, pShader);
  if(InlIsFailedToNewObject(pEffect)) {
    return GX_ERROR_OUROFMEMORY;
  }

  if(_CL_NOT_(pEffect->InitEffect())) {
    SAFE_RELEASE(pEffect);
    return GX_FAIL;
  }

  static_cast<ShaderImpl*>(pShader)->BuildCBTable(pEffect->GetDataPoolUnsafe());

  RegisterResource(pEffect, NULL);
  *ppEffect = pEffect;
  return GX_OK;
}

#if 0
GXHRESULT GraphicsImpl::CreateEffect(Effect** ppEffect, Shader* pShader)
{
  if(pShader == NULL)
    return NULL;
  GXEffectImpl* pNewEffect = new GXEffectImpl(this);
  RegisterResource(pNewEffect, NULL);
  pNewEffect->AddRef();
  pNewEffect->SetShaderRef((GShaderImpl*)pShader);
  *ppEffect = pNewEffect;
  return GX_OK;
}
#endif // 0

GXHRESULT GraphicsImpl::CreateMaterial(Material** ppMtlInst, Shader* pShader)
{
  if(pShader == NULL) {
    return GX_FAIL;
  }
  Graphics* pGraphics = pShader->GetGraphicsUnsafe();
  MaterialImpl* pNewMaterial = new MaterialImpl(pGraphics, pShader);
  if(InlIsFailedToNewObject(pNewMaterial)) {
    return GX_ERROR_OUROFMEMORY;
  }

  if(_CL_NOT_(pNewMaterial->InitMaterial()))
  {
    SAFE_RELEASE(pNewMaterial);
    return GX_FAIL;
  }

  *ppMtlInst = pNewMaterial;
  return GX_OK;
}

GXHRESULT GraphicsImpl::CreateMaterialFromFile(Material** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType)
{
  Shader* pShader = NULL;
  GRESKETCH ResFeatDesc;
  
  //
  // Scoped Locker
  //
  clstd::ScopedLocker lock(m_pGraphicsLocker);

  if(eLoadType == MLT_REFERENCE)
  {
    // 只有szShaderDesc完全一致时才能引用, 它不对文件名和宏进行排序(规范化)
    GrapX::Internal::ResourceSketch::GenerateMaterialDescW(&ResFeatDesc, szShaderDesc);
    GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
    if(pResource != NULL)
    {
      pResource->AddRef();
      *ppMtlInst = static_cast<Material*>(pResource);
      return GX_OK;
    }
  }
  return GX_FAIL; // 临时
#if 0
  MTLFILEPARAMDESC MtlParam;
  GXHRESULT hval = IntCreateSdrPltDescW(&pShader, szShaderDesc, InlGetPlatformStringA(), &MtlParam);
  if(GXSUCCEEDED(hval))
  {
    hval = CreateMaterial(ppMtlInst, pShader);
    if(GXFAILED(hval))
    {
      ASSERT(0);
    }
    else {
      if( ! MtlParam.aUniforms.empty() )
      {
        (*ppMtlInst)->SetParameters(Material::PT_UNIFORMS, (GXDEFINITION*)&MtlParam.aUniforms.front(), (int)MtlParam.aUniforms.size());
      }
      if( ! MtlParam.aStates.empty() )
      {
        (*ppMtlInst)->SetParameters(Material::PT_RENDERSTATE, (GXDEFINITION*)&MtlParam.aStates.front(), (int)MtlParam.aStates.size());
      }
      if( ! MtlParam.aBindPool.empty() )
      {
        for(clStringArrayA::iterator it = MtlParam.aBindPool.begin();
          it != MtlParam.aBindPool.end(); ++it)
        {
          clStringA& strDataPoolBody = *it;
          size_t pos = strDataPoolBody.Find('.', 0);
          if(pos == clStringA::npos) {
            (*ppMtlInst)->BindDataByName(strDataPoolBody, NULL);
          }
          else {
            (*ppMtlInst)->BindDataByName(strDataPoolBody.Left(pos), strDataPoolBody.Right(strDataPoolBody.GetLength() - pos - 1));
          }
        }
      }

      // 创建成功就注册
      if(eLoadType == MLT_REFERENCE) {
        m_ResMgr.Register(&ResFeatDesc, *ppMtlInst);
      }
      else {
        m_ResMgr.RegisterUnfeatured(*ppMtlInst);
      }

    }
    SAFE_RELEASE(pShader);
    return hval;
  }
  else {
    //ASSERT(0);
  }
  return hval;
#endif
}

//GXHRESULT GXGraphicsImpl::CreateMaterialFromFile(GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType)
//{
//  clStringW str = szShaderDesc;
//  return CreateMaterialFromFile(ppMtlInst, str, eLoadType);
//}


//////////////////////////////////////////////////////////////////////////
GXFont* GraphicsImpl::CreateFontIndirect(const GXLPLOGFONTW lpLogFont)
{
  GXLOGFONTA LogFontA;
  LogFontA.lfHeight         = lpLogFont->lfHeight;
  LogFontA.lfWidth          = lpLogFont->lfWidth;
  LogFontA.lfEscapement     = lpLogFont->lfEscapement;
  LogFontA.lfOrientation    = lpLogFont->lfOrientation;
  LogFontA.lfWeight         = lpLogFont->lfWeight;
  LogFontA.lfItalic         = lpLogFont->lfItalic;
  LogFontA.lfUnderline      = lpLogFont->lfUnderline;
  LogFontA.lfStrikeOut      = lpLogFont->lfStrikeOut;
  LogFontA.lfCharSet        = lpLogFont->lfCharSet;
  LogFontA.lfOutPrecision   = lpLogFont->lfOutPrecision;
  LogFontA.lfClipPrecision  = lpLogFont->lfClipPrecision;
  LogFontA.lfQuality        = lpLogFont->lfQuality;
  LogFontA.lfPitchAndFamily = lpLogFont->lfPitchAndFamily;
  gxWideCharToMultiByte(GXCP_ACP, 0, lpLogFont->lfFaceName, -1, LogFontA.lfFaceName, GXLF_FACESIZE, 0, 0);
  return CreateFontIndirect(&LogFontA);
}

GXFont* GraphicsImpl::CreateFontIndirect(const GXLPLOGFONTA lpLogFont)
{
  GRESKETCH ResFeatDesc;
  GrapX::Internal::ResourceSketch::GenerateFontA(&ResFeatDesc, lpLogFont);
  m_pLogger->OutputFormatA("Load font from file: %s", lpLogFont->lfFaceName);

  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL) {
    GXHRESULT hval = pResource->AddRef();
    m_pLogger->OutputFormatA("...Succeeded(%d).\n", hval);
    return (GXFont*)pResource;
  }

  _GFTFont* pFont = new _GFTFont(this, lpLogFont);
  //pFont->AddRef();

  if(pFont->CreateFont(lpLogFont->lfWidth, lpLogFont->lfHeight, lpLogFont->lfFaceName) == FALSE)
  {
    SAFE_RELEASE(pFont);
    CLOG_ERROR("GXGraphics::CreateFontIndirectA : Failed to create font(\"%s\").\n", lpLogFont->lfFaceName);
    m_pLogger->OutputFormatW(_CLTEXT("...Failed.\n"));
    return NULL;
  }

  RegisterResource(pFont, &ResFeatDesc);
  m_pLogger->OutputFormatA("...Succeeded.\n");
  return pFont;
}

GXFont* GraphicsImpl::CreateFont(const GXULONG nWidth, const GXULONG nHeight, GXLPCWSTR pFileName)
{
  GXLOGFONTW LogFont;
  memset(&LogFont, 0, sizeof(GXLOGFONTW));
  LogFont.lfWidth = nWidth;
  LogFont.lfHeight = nHeight;
  GXSTRCPYN(LogFont.lfFaceName, pFileName, GXLF_FACESIZE);
  return CreateFontIndirect(&LogFont);
}

GXFont* GraphicsImpl::CreateFont(const GXULONG nWidth, const GXULONG nHeight, GXLPCSTR pFileName)
{
  GXLOGFONTA LogFont;
  memset(&LogFont, 0, sizeof(GXLOGFONTA));
  LogFont.lfWidth = nWidth;
  LogFont.lfHeight = nHeight;
  GXSTRCPYN(LogFont.lfFaceName, pFileName, GXLF_FACESIZE);
  return CreateFontIndirect(&LogFont);
}
//////////////////////////////////////////////////////////////////////////

#include "Canvas/GXCanvas3DImpl.h"
#include "Canvas/GXCanvas3DImpl.inl"
GXHRESULT GraphicsImpl::CreateCanvas3D(Canvas3D** ppCanvas3D, RenderTarget* pTarget, LPCREGN lpRegn, float fNear, float fFar)
{
  GXREGN    regn = 0;
  GXHRESULT hval = GX_OK;
  GXLPCSTR  c_szError_NoSameSize = __FUNCTION__": Image and DepthStencil-texture must be in same size.\n";

  // 获得 Image 的尺寸, 如果是 NULL 则取后台缓冲的尺寸
  if(pTarget) {
    GXSIZE sDimension;
    pTarget->GetDimension(&sDimension);
    regn.width  = sDimension.cx;
    regn.height = sDimension.cy;

    //GXUINT nDepthWidth, nDepthHeight;
    //pDepthStencil->GetDimension(&nDepthWidth, &nDepthHeight);

    //if(regn.width != nDepthWidth || regn.height != nDepthHeight) {
    //  CLOG_ERROR(c_szError_NoSameSize);
    //  return GX_FAIL;
    //}
  }
  else// if(pImage == NULL && pDepthStencil == NULL) 
  {
    GXGRAPHICSDEVICE_DESC Desc;
    GetDesc(&Desc);
    regn.width  = Desc.BackBufferWidth;
    regn.height = Desc.BackBufferHeight;
    pTarget = m_pDefaultBackBuffer;
  }
  //else
  //{
  //  //(pImage == NULL && pDepthStencil != NULL) ||
  //  //(pImage != NULL && pDepthStencil == NULL)
  //  CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, __FUNCTION__);
  //  return GX_FAIL;
  //}

  if(lpRegn != NULL) {
    regn = *lpRegn;
  }

  Canvas3DImpl* pCanvas3D = new Canvas3DImpl(this);
  if( ! InlCheckNewAndIncReference(pCanvas3D)) {
    return GX_FAIL;
  }

  GXVIEWPORT Viewport(&regn, fNear, fFar);
  if( ! pCanvas3D->Initialize(pTarget, &Viewport))
  {
    pCanvas3D->Release();
    pCanvas3D = NULL;
    hval = GX_FAIL;
  }
  *ppCanvas3D = pCanvas3D;
  return hval;
}

//////////////////////////////////////////////////////////////////////////
#if 0
GXHRESULT GraphicsImpl::IntCreateSdrPltDescW(GShader** ppShader, GXLPCWSTR szShaderDesc, GXLPCSTR szPlatformSect, MTLFILEPARAMDESC* pMtlParam)
{
  GXHRESULT       hval = GX_FAIL;

  MOSHADER_ELEMENT_SOURCE ses;
  hval = GShader::Load(szShaderDesc, m_strResourceDir, szPlatformSect, &ses, pMtlParam);
  if(GXFAILED(hval)) {
    return hval;
  }

  IntAttachComposerSdrDesc(&ses);

  // 根据分量元素查找资源
  if( ! (ses.strVS.IsEmpty() || ses.strPS.IsEmpty()))
  {
    hval = IntCreateSdrFromElement(ppShader, &ses);
    if(GXSUCCEEDED(hval)) {
      static_cast<GShaderImpl*>(*ppShader)->m_strProfileDesc = szShaderDesc;
    }
  }
  return hval;
}
#endif // 0

GXHRESULT GraphicsImpl::IntCreateSamplerState(SamplerStateImpl** ppSamplerState, const GXSAMPLERDESC* pDesc)
{
  GRESKETCH ResFeatDesc;

  // 资源特征提取
  GrapX::Internal::ResourceSketch::GenerateSamplerState(&ResFeatDesc, pDesc);

  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL) {
    GXHRESULT hr = pResource->AddRef();
    //m_pLogger->OutputFormatA("...Succeeded(%d).\n", hval);
    *ppSamplerState = static_cast<SamplerStateImpl*>(pResource);
    return hr;
  }

  SamplerStateImpl* pSampleStateImpl = new SamplerStateImpl(this);
  if(InlCheckNewAndIncReference(pSampleStateImpl)) {
    if(pSampleStateImpl->Initialize(pDesc)) {
      *ppSamplerState = pSampleStateImpl;
      RegisterResource(pSampleStateImpl, &ResFeatDesc);
      return GX_OK;
    }
  }

  SAFE_RELEASE(pSampleStateImpl);
  return GX_FAIL;
}

void GraphicsImpl::IntAttachComposerSdrDesc(MOSHADER_ELEMENT_SOURCE* pSdrElementSrc)
{
  if(pSdrElementSrc->bExtComposer)
  {
    if(pSdrElementSrc->strPSComposer.IsEmpty()) {
      pSdrElementSrc->strPSComposer = m_ShaderExtElement.strPSComposer;
    }

    if(pSdrElementSrc->strVSComposer.IsEmpty()) {
      pSdrElementSrc->strVSComposer = m_ShaderExtElement.strVSComposer;
    }
  }
}

#if 0
GXHRESULT GraphicsImpl::IntCreateSdrFromElement(GShader** ppShader, MOSHADER_ELEMENT_SOURCE* pSdrElementSrc)
{
  GRESKETCH ResFeatDesc;

  // 资源特征提取
  GrapX::Internal::ResourceSketch::GenerateShaderElementA(&ResFeatDesc, pSdrElementSrc, NULL);

  m_pLogger->OutputFormatA("Load shader from file: %s(vs), %s(ps)", pSdrElementSrc->strVS, pSdrElementSrc->strPS);

  // 资源查找
  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL) {
    GXHRESULT hval = pResource->AddRef();
    m_pLogger->OutputFormatA("...Succeeded(%d).\n", hval);
    *ppShader = (GShader*)pResource;
    return GX_OK;
  }

  // 创建 增加引用计数
  GShaderImpl* pShader = new GShaderImpl(this);
  if(InlIsFailedToNewObject(pShader)) {
    return GX_ERROR_OUROFMEMORY;
  }

  // 初始化 验证
  GXHRESULT hval = pShader->LoadFromFile(pSdrElementSrc);
  if(GXFAILED(hval))
  {
    SAFE_RELEASE(pShader);
    m_pLogger->OutputFormatW(_CLTEXT("...Failed.\n"));
    return hval;
  }

  RegisterResource(pShader, &ResFeatDesc);
  pShader->PutInResourceMgr();

  *ppShader = pShader;
  m_pLogger->OutputFormatA("...Succeeded.\n");
  return GX_OK;
}
#endif // 0

GXHRESULT GraphicsImpl::CreateShaderFromSource(Shader** ppShader, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount)
{
  ShaderImpl* pShader = new ShaderImpl(this);
  if(InlIsFailedToNewObject(pShader))
  {
    return GX_ERROR_OUROFMEMORY;
  }

  if(_CL_NOT_(pShader->InitShader(m_strResourceDir, pShaderDescs, nCount))) {
    SAFE_RELEASE(pShader);
    return GX_FAIL;
  }

  RegisterResource(pShader, NULL);
  *ppShader = pShader;
  return GX_OK;
}

//GXHRESULT GraphicsImpl::CreateShaderFromSource(GShader** ppShader, GXLPCSTR szShaderSource, size_t nSourceLen, GXDEFINITION* pMacroDefinition)
//{
//  GShaderImpl* pShader = new GShaderImpl(this);
//  if(InlIsFailedToNewObject(pShader)) {
//    return GX_ERROR_OUROFMEMORY;
//  }
//
//  clBuffer sVertexBuffer;
//  clBuffer sPixelBuffer;
//  LPD3DXINCLUDE pInclude = NULL;
//  GXHRESULT hr = GX_OK;
//
//  if(nSourceLen == 0)
//  {
//    nSourceLen = GXSTRLEN(szShaderSource);
//  }
//
//  hr = pShader->CompileShader(&sVertexBuffer, szShaderSource, nSourceLen,
//    pInclude, pMacroDefinition, GShaderImpl::CompiledVertexShder);
//
//  if(GXSUCCEEDED(hr))
//  {
//    hr = pShader->CompileShader(&sPixelBuffer, szShaderSource, nSourceLen,
//      pInclude, pMacroDefinition, GShaderImpl::CompiledPixelShder);
//  }
//
//  if(GXSUCCEEDED(hr))
//  {
//    hr = pShader->LoadFromMemory(&sVertexBuffer, &sPixelBuffer);
//  }
//
//  if(GXSUCCEEDED(hr))
//  {
//    RegisterResource(pShader, NULL);
//    *ppShader = pShader; // 构造时已经加一
//  }
//  else
//  {
//    SAFE_RELEASE(pShader);
//  }
//
//  return hr;
//}

GXHRESULT GraphicsImpl::CreateShaderFromFile(Shader** ppShader, GXLPCWSTR szShaderDesc)
{
  //CLBREAK;
  //return IntCreateSdrPltDescW(ppShader, szShaderDesc, InlGetPlatformStringA(), NULL);
  return GX_FAIL;
}

GXHRESULT GraphicsImpl::CreateShaderFromFile(Shader** ppShader, GXLPCSTR szShaderDesc)
{
  clStringW str = szShaderDesc;
  return CreateShaderFromFile(ppShader, str);
  //CLBREAK;
  //clStringW strDesc = szShaderDesc;
  //return IntCreateSdrPltDescW(ppShader, strDesc, InlGetPlatformStringA(), NULL);
}

//GXHRESULT GraphicsImpl::CreateShaderStub(GShaderStub** ppShaderStub)
//{
//  GShaderStubImpl* pShaderStub = new GShaderStubImpl(this);
//  pShaderStub->AddRef();
//  *ppShaderStub = pShaderStub;
//  return GX_OK;
//}

GXHRESULT GraphicsImpl::CreateVertexDeclaration(GVertexDeclaration** ppVertexDecl, LPCGXVERTEXELEMENT lpVertexElement)
{
  // 资源特征提取
  GRESKETCH ResFeatDesc;
  GrapX::Internal::ResourceSketch::GenerateVertexDecl(&ResFeatDesc, lpVertexElement);

  //
  // locker
  //
  clstd::ScopedSafeLocker locker(m_pGraphicsLocker);

  GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
  if(pResource != NULL) {
    GXHRESULT hval = pResource->AddRef();
    *ppVertexDecl = static_cast<GVertexDeclaration*>(pResource);
    return hval;
  }

  VertexDeclImpl* pVertexDeclImpl = new VertexDeclImpl(this);
  if(InlIsFailedToNewObject(pVertexDeclImpl)) {
    return GX_FAIL;
  }

  GXHRESULT hval = pVertexDeclImpl->Initialize(lpVertexElement);

  if(GXFAILED(hval))
  {
    SAFE_RELEASE(pVertexDeclImpl);
    return GX_FAIL;
  }

  *ppVertexDecl = pVertexDeclImpl;
  RegisterResource(pVertexDeclImpl, &ResFeatDesc);
  return hval;
}

//GXHRESULT GXGraphicsImpl::CreateCanvas3D(GXCanvas3D** ppCanvas3D, GXImage* pImage, GXFormat eDepthStencil, LPCREGN lpRegn, float fNear, float fFar)
//{
//  // 如果 pImage 为 NULL, 则忽略 eDepthStencil 参数
//  GXINT nWidth = 0, nHeight = 0;
//  if(pImage != NULL) {
//    pImage->GetDimension(&nWidth, &nHeight);
//  }
//  else {
//    //GXGRAPHICSDEVICE_DESC Desc;
//    //GetDesc(&Desc);
//    //nWidth = Desc.BackBufferWidth;
//    //nHeight = Desc.BackBufferHeight;
//  }
//
//  GTexture* pDepthStencil = NULL;
//  if(pImage == NULL || GXSUCCEEDED(CreateTexture(&pDepthStencil, NULL, nWidth, nHeight, 1, eDepthStencil, GXRU_DEFAULT)))
//  {
//    GXHRESULT hval = CreateCanvas3D(ppCanvas3D, pImage, pDepthStencil, lpRegn, fNear, fFar);
//    SAFE_RELEASE(pDepthStencil);
//    return hval;
//  }
//  return GX_FAIL;
//}
//////////////////////////////////////////////////////////////////////////
GXDWORD GraphicsImpl::GetCaps(GXGrapCapsCategory eCate)
{
  switch(eCate)
  {
  case GXGRAPCAPS_TEXTURE:
#if defined(_CL_SYSTEM_WINDOWS)
#ifdef GLES2_GRAPHICS_IMPL
    return NULL;
#elif defined(D3D9_GRAPHICS_IMPL)
    return GXTEXTURECAPS_NONPOW2;
#endif // GLES2_GRAPHICS_IMPL
#elif defined(_CL_SYSTEM_IOS)
    return NULL;
#endif // defined(_WIN32) || defined(_WINDOWS)
    break;
  default:
    ASSERT(0);
    return NULL;
  }
  return NULL;
}

Canvas* GraphicsImpl::LockCanvas(RenderTarget* pTarget, const LPREGN lpRegn, GXDWORD dwFlags)
{
  // 允许 lpRegn 是空的, 比如Edit控件会因为调整大小而创建DC.此时需要设备的Font信息.
  if( ! TEST_FLAG(m_dwFlags, F_ACTIVATE))
  {
    ASSERT(0);
    return NULL;
  }

#ifdef D3D9_GRAPHICS_IMPL
  ASSERT(m_dwThreadId == NULL || m_dwThreadId == GetCurrentThreadId());
#endif // #ifdef D3D9_GRAPHICS_IMPL

  CanvasImpl* pCanvas = (CanvasImpl*)AllocCanvas();
  if(pCanvas->Initialize(pTarget, lpRegn) == FALSE)
  {
    SAFE_RELEASE(pCanvas);
    return NULL;
  }
  ASSERT(m_dwBackBufferStencil != 0);
  pCanvas->SetStencil(m_dwBackBufferStencil);
  IncreaseStencil(&m_dwBackBufferStencil);
  return pCanvas;
}

//GXHRESULT GXGraphicsImpl::SetPrimitive(GPrimitive* pPrimitive, GXUINT uStreamSource/* = 0*/)
//{
//  if(pPrimitive->GetType() == RESTYPE_PRIMITIVE)
//    return SetPrimitiveV((GPrimitiveV*)pPrimitive);
//  else
//    return SetPrimitiveVI((GPrimitiveVI*)pPrimitive);
//}

GXHRESULT GraphicsImpl::SetRasterizerState(RasterizerState* pRasterizerState)
{
  if(pRasterizerState == NULL) {
    pRasterizerState = m_pDefaultRasterizerState;
  }

  if(InlSetRasterizerState(static_cast<RasterizerStateImpl*>(pRasterizerState))) {
    return GX_OK;
  }
  return GX_FAIL;
}

GXHRESULT GraphicsImpl::SetBlendState(BlendState* pBlendState)
{
  if(pBlendState == NULL) {
    pBlendState = m_pDefaultBlendState;
  }

  if(InlSetBlendState(static_cast<BlendStateImpl*>(pBlendState))) {
    return GX_OK;
  }
  return GX_FAIL;
}

GXHRESULT GraphicsImpl::SetDepthStencilState(DepthStencilState* pDepthStencilState)
{
  if(pDepthStencilState == NULL) {
    pDepthStencilState = m_pDefaultDepthStencilState;
  }

  if(InlSetDepthStencilState(static_cast<DepthStencilStateImpl*>(pDepthStencilState))) {
    return GX_OK;
  }
  return GX_FAIL;

}

GXHRESULT GraphicsImpl::SetSamplerState(GXUINT slot, SamplerState* pSamplerState)
{
  if(pSamplerState == NULL) {
    pSamplerState = m_pDefaultSamplerState;
  }

  if(InlSetSamplerState(slot, static_cast<SamplerStateImpl*>(pSamplerState))) {
    return GX_OK;
  }
  return GX_FAIL;
}

GXBOOL GraphicsImpl::SwitchConsole()
{
  Enter();
  GXBOOL bret = TEST_FLAG(m_dwFlags, F_SHOWCONSOLE);
  SWITCH_FLAG(m_dwFlags, F_SHOWCONSOLE);
  //m_pConsole->Show(!bret);
  Leave();
  return bret;
}

//////////////////////////////////////////////////////////////////////////
GXHRESULT GraphicsImpl::RegisterResource(GResource* pResource, LPCRESKETCH pSketch)
{
  clstd::ScopedSafeLocker locker(m_pGraphicsLocker);

  GXHRESULT hval = GX_OK;
  if(pSketch == NULL) {
    hval = m_ResMgr.RegisterUnfeatured(pResource);
  }
  else {
    hval = m_ResMgr.Register(pSketch, pResource);
  }
  return hval;
}

//GXHRESULT GXGraphicsImpl::RegisterResourceEx(LPCRESKETCH pDesc, GResource* pResource)
//{
//  GXHRESULT hval = m_ResMgr.Register(pDesc, pResource);
//  return hval;
//}

GXHRESULT GraphicsImpl::UnregisterResource(GResource* pResource)
{
  clstd::ScopedSafeLocker locker(m_pGraphicsLocker);

  return m_ResMgr.Unregister(pResource);
}
//////////////////////////////////////////////////////////////////////////

//GXBOOL GXGraphicsImpl::OldRegisterResource(GResource* pResource)
//{
//  if(FindResource(pResource) != m_aResource.end())
//    return FALSE;
//  m_aResource.push_back(pResource);
//  pResource->AddRef();
//  return TRUE;
//}
//
//GXUINT GXGraphicsImpl::OldUnregisterResource(GResource* pResource)
//{
//  GResourceArray::iterator& it = FindResource(pResource);
//  if(it == m_aResource.end()) {
//    TRACE("Released by wrong resource manager.\n");
//    ASSERT(0);
//    return FALSE;
//  }
//
//  m_aResource.erase(it);
//  return (GXUINT)pResource->Release();
//}

GXHRESULT GraphicsImpl::BroadcastScriptCommand(GRESCRIPTDESC* pDesc)
{
  pDesc->bBroadcast = TRUE;
  pDesc->dwTime = gxGetTickCount();
  return m_ResMgr.BroadcastScriptCommand(pDesc);
}

GXHRESULT GraphicsImpl::BroadcastCategoryCommand(GXDWORD dwCategoryID, GRESCRIPTDESC* pDesc)
{
  pDesc->bBroadcast = TRUE;
  pDesc->dwTime = gxGetTickCount();
  return m_ResMgr.BroadcastCategoryMessage(dwCategoryID, pDesc);
}

Canvas* GraphicsImpl::AllocCanvas()
{
  for(GXUINT i = 0; i < s_uCanvasCacheCount; i++)
  {
    if(m_aCanvasPtrCache[i]->GetRef() == 1)
    {
      m_aCanvasPtrCache[i]->AddRef();
      return m_aCanvasPtrCache[i];
    }
  }
  return new CanvasImpl(this, FALSE);
}

//GXGraphicsImpl::GResourceArray::iterator
//  GXGraphicsImpl::FindResource(GResource* pResource)
//{
//  for(GResourceArray::iterator it = m_aResource.begin();
//    it != m_aResource.end(); ++it)
//  {
//    if(*it == pResource)
//      return it;
//  }
//  return m_aResource.end();
//}

void GraphicsImpl::IncreaseStencil(GXDWORD* pdwStencil)
{
  ++(*pdwStencil);

  // 这个算法保证 pdwStencil 不会等于0
  *pdwStencil = ((*pdwStencil) & 0xff) + ((*pdwStencil) >> 8);
}

//GXImage* GXGraphicsImpl::CreateImageFromFile(GXLPCWSTR lpwszFilename)
//{
//  GTexture* pTexture;
//  if(GXSUCCEEDED(CreateTextureFromFileExW(
//    &pTexture, lpwszFilename, GX_DEFAULT_NONPOW2, GX_DEFAULT_NONPOW2, 1, GXFMT_UNKNOWN, 
//    GXRU_DEFAULT, GXFILTER_POINT, GX_DEFAULT, 0, NULL)) )
//  {
//    GXImage* pImage = CreateImageFromTexture(pTexture);
//    SAFE_RELEASE(pTexture);
//    return pImage;
//  }
//  return NULL;
//}
//
//GXImage* GXGraphicsImpl::CreateImageFromTexture(GTexture* pTexture)
//{
//  GXImageImpl* pImage = NULL;
//  pImage = new GXImageImpl(this);
//  pImage->AddRef();
//  // TODO: 这里是不是该加锁?
//  RegisterResource(pImage, NULL);
//  if(pImage->Initialize(pTexture) == FALSE)
//  {
//    SAFE_RELEASE(pImage);
//    return NULL;
//  }
//  return pImage;
//}

//////////////////////////////////////////////////////////////////////////

  //GXImage* GXGraphicsImpl::GetBackBufferImg()
  //{
  //  return m_pBackBufferImg;
  //}

  //GTexture* GXGraphicsImpl::GetBackBufferTex()
  //{
  //  return m_pBackBufferTex;
  //}

  GXHRESULT GraphicsImpl::GetBackBuffer(RenderTarget** ppTarget)
  {
    *ppTarget = m_pDefaultBackBuffer;
    return m_pDefaultBackBuffer->AddRef();
  }

  Texture* GraphicsImpl::GetDeviceOriginTex()
  {
    return m_pDeviceOriginTex;
  }

  GXBOOL GraphicsImpl::ScrollTexture(const SCROLLTEXTUREDESC* lpstd)
  {
    GXBOOL bresult = FALSE;
    // 该函数不对偏移量为(0,0)的情况进行检测
    ASSERT(lpstd->dx != 0 || lpstd->dy != 0);
    Enter();

    // 确定滚动区是否为一个简单矩形
    GXBOOL bSimpleRect = FALSE;

    Texture* const pTarget  = lpstd->pOperationTex == NULL 
      ? m_pDeviceOriginTex
      : lpstd->pOperationTex;
    Texture* const pTempTex = lpstd->pTempTex == NULL 
      ? m_pTempBuffer->GetColorTextureUnsafe(GXResUsage::Default)
      : lpstd->pTempTex;

    GXRECT rcTarget(0);
    //GXUINT nBackWidth, nBackHeight;
    GXSIZE sTargetDimension;
    GXSIZE sTempDimension;


    pTarget->GetDimension(&sTargetDimension);
    rcTarget.right = sTargetDimension.cx;
    rcTarget.bottom = sTargetDimension.cy;
    pTempTex->GetDimension(&sTempDimension);

    GRegion* prgnSrc      = NULL;
    GRegion* prgnClip      = NULL;
    GRegion* prgnInvMoveClip  = NULL;

//#ifdef _DEBUG
//    // 检测后台临时缓冲要大于操作用的缓冲纹理
//    ASSERT((GXUINT)rcTarget.right <= nBackWidth && (GXUINT)rcTarget.bottom <= nBackHeight);  
//#endif

    // 裁剪区是一个Simple Rect
    if(lpstd->lprgnClip != NULL && lpstd->lprgnClip->GetRectCount() == 1)
    {
      GXRECT rcClip;
      lpstd->lprgnClip->GetBounding(&rcClip);
      gxIntersectRect(&rcTarget, &rcTarget, &rcClip);
      bSimpleRect = TRUE;
    }

    if(bSimpleRect)
    {
      GXRECT rcSrc, rcDst;
      GXRECT rcInvMoveTar = rcTarget;  // rcTarget 向相反移动的区域

      gxOffsetRect(&rcInvMoveTar, -lpstd->dx, -lpstd->dy);

      // 确定源矩形范围
      if(lpstd->lprcScroll != NULL)
        gxIntersectRect(&rcSrc, &rcTarget, lpstd->lprcScroll);
      else
        rcSrc = rcTarget;

      if( ! gxIntersectRect(&rcSrc, &rcSrc, &rcInvMoveTar)) {
        goto FUNC_RET_0;
      }

      // 目标矩形范围
      rcDst = rcSrc;
      gxOffsetRect(&rcDst, lpstd->dx, lpstd->dy);

      //pTempTex->StretchRect(pTarget, &rcDst, &rcSrc, GXTEXFILTER_POINT);
      //pTarget->StretchRect(pTempTex, &rcDst, &rcDst, GXTEXFILTER_POINT);
      CLBREAK; // 上面两条没实现
      if(lpstd->lpprgnUpdate != NULL || lpstd->lprcUpdate != NULL)
      {
        CreateRectRgn(&prgnSrc, rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom);
        CreateRectRgn(&prgnClip, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
        prgnInvMoveClip = prgnClip->Clone();
        prgnInvMoveClip->Offset(-lpstd->dx, -lpstd->dy);
      }
    }
    else
    {
      CreateRectRgn(&prgnClip, 0, 0, rcTarget.right, rcTarget.bottom);

      if(lpstd->lprcScroll != NULL)
        CreateRectRgn(&prgnSrc, 
        lpstd->lprcScroll->left, lpstd->lprcScroll->top, 
        lpstd->lprcScroll->right, lpstd->lprcScroll->bottom);

      if(lpstd->lprgnClip != NULL)
        prgnClip->Intersect(lpstd->lprgnClip);

      prgnInvMoveClip = prgnClip->Clone();
      prgnInvMoveClip->Offset(-lpstd->dx, -lpstd->dy);

      if(prgnSrc != NULL)
      {
        prgnSrc->Intersect(prgnClip);
        prgnSrc->Intersect(prgnInvMoveClip);
      }
      else
      {
        prgnSrc = prgnClip;
        prgnClip = NULL;
        prgnSrc->Intersect(prgnInvMoveClip);
      }
      const GXUINT nRectCount = prgnSrc->GetRectCount();
      clstd::LocalBuffer<sizeof(GXRECT) * 128> _buf;
      _buf.Resize(sizeof(GXRECT) * nRectCount, FALSE);

      GXRECT* lpRects = (GXRECT*)_buf.GetPtr(); // _GlbLockStaticRects(nRectCount);
      prgnSrc->GetRects(lpRects, nRectCount);
      //vector<GXRECT> aRects;
      //prgnSrc->GetData(aRects);
      /*
      //for(vector<GXRECT>::iterator it = aRects.begin(); it != aRects.end(); ++it)
      // 这个有Bug啊
      for(GXUINT i = 0; i < nRectCount; i++)
      {
      GXRECT& rect = lpRects[i];
      pTempTex->StretchRect(pTarget, &rect, &rect, D3DTEXF_POINT);

      GXRECT rcDest;
      rcDest.left    = rect.left + lpstd->dx;
      rcDest.top    = rect.top + lpstd->dy;
      rcDest.right  = rect.right + lpstd->dx;
      rcDest.bottom  = rect.bottom + lpstd->dy;
      pTarget->StretchRect(pTempTex, &rcDest, &rect, D3DTEXF_POINT);
      }
      /*/
      //for(vector<GXRECT>::iterator it = aRects.begin(); it != aRects.end(); ++it)
      for(GXUINT i = 0; i < nRectCount; i++)
      {
        GXRECT& rect = lpRects[i];
        //pTempTex->StretchRect(pTarget, &rect, &rect, GXTEXFILTER_POINT);
        CLBREAK; // 上面1条没实现
      }
      //for(vector<GXRECT>::iterator it = aRects.begin(); it != aRects.end(); ++it)
      for(GXUINT i = 0; i < nRectCount; i++)
      {
        GXRECT& rect = lpRects[i];
        GXRECT rcDest;
        rcDest.left   = rect.left + lpstd->dx;
        rcDest.top    = rect.top + lpstd->dy;
        rcDest.right  = rect.right + lpstd->dx;
        rcDest.bottom = rect.bottom + lpstd->dy;
        //pTarget->StretchRect(pTempTex, &rcDest, &rect, GXTEXFILTER_POINT);
        CLBREAK; // 上面1条没实现
      }
      //*/
      //_GlbUnlockStaticRects(lpRects);
    }
    //clipRgn & (updateRgn + offset(updateRgn-clipRgn)) - destRgn
    if(lpstd->lpprgnUpdate != NULL || lpstd->lprcUpdate != NULL)
    {
      GRegion* prgnDst = prgnSrc->Clone();
      prgnDst->Offset(lpstd->dx, lpstd->dy);

      GRegion* prgnUpdate = NULL;
      CreateRectRgn(&prgnUpdate, 
        lpstd->lprcScroll->left, lpstd->lprcScroll->top, 
        lpstd->lprcScroll->right, lpstd->lprcScroll->bottom);

      GRegion* prgnUpdateB = prgnUpdate->CreateSubtract(prgnClip);
      prgnUpdateB->Offset(lpstd->dx, lpstd->dy);

      prgnUpdate->Union(prgnUpdateB);
      prgnUpdate->Intersect(prgnClip);
      prgnUpdate->Subtract(prgnDst);

      if(lpstd->lprcUpdate != NULL)
        prgnUpdate->GetBounding(lpstd->lprcUpdate);

      if(lpstd->lpprgnUpdate != NULL)
        *lpstd->lpprgnUpdate = prgnUpdate;
      else
        SAFE_RELEASE(prgnUpdate);
      SAFE_RELEASE(prgnUpdateB);
      SAFE_RELEASE(prgnDst);
    }

    SAFE_RELEASE(prgnSrc);
    SAFE_RELEASE(prgnClip);
    SAFE_RELEASE(prgnInvMoveClip);
#ifdef D3D9_GRAPHICS_IMPL
    IntCheckRTTexture(static_cast<TextureImpl*>(pTarget));
#endif // #ifdef D3D9_GRAPHICS_IMPL
    bresult = TRUE;
FUNC_RET_0:
    Leave();
    return bresult;
  }
//////////////////////////////////////////////////////////////////////////
  Effect* GraphicsImpl::IntGetEffect()
  {
    return m_pBasicEffect;
  }

  void GraphicsImpl::Enter()
  {
    //TRACE("\n\n"__FUNCTION__":%d\n", m_pGraphicsLocker->m_nLockCount);
    m_pGraphicsLocker->Lock();
  }

  void GraphicsImpl::Leave()
  {
    m_pGraphicsLocker->Unlock();
    //TRACE(__FUNCTION__":%d\n", m_pGraphicsLocker->m_nLockCount);
  }

  GXBOOL GraphicsImpl::GetRenderStatistics(RENDER_STATISTICS* pStat)
  {
    *pStat = m_sStatistics;
    return TRUE;
  }