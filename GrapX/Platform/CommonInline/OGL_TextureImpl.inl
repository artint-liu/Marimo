//GXLRESULT TextureImpl::OnDeviceEvent(DeviceEvent eEvent)
//{
//  if(m_Format == GXFMT_UNKNOWN)
//    return GX_FAIL;
//
//  GXFormatCategory eFmtCate = GetGraphicsFormatCategory(m_Format);
//  switch(eEvent)
//  {
//  case DE_LostDevice:
//    {
//      //if(m_Pool == GXPOOL_DEFAULT)
//      //{
//      ASSERT(m_uTexture != 0);
//      switch(eFmtCate)
//      {
//      case GXFMTCATE_COLOR:
//        GLVERIFY(glDeleteTextures(1, &m_uTexture));
//        m_uTexture = 0;
//        //if(m_uFrameBuffer != 0)
//        //{
//        //  GLVERIFY(glDeleteFramebuffers(1, &m_uFrameBuffer));
//        //}
//        break;
//      case GXFMTCATE_DEPTHSTENCIL:
//        if(m_uDepthRenderBuffer != 0)
//        {
//          GLVERIFY(glDeleteRenderbuffers(1, &m_uDepthRenderBuffer));
//          m_uDepthRenderBuffer = 0;
//        }
//        if(m_uStencilRenderBuffer != 0 && IS_DEPTHSTENCIL_NOT_IN_SAME_FBO)
//        {
//          GLVERIFY(glDeleteRenderbuffers(1, &m_uStencilRenderBuffer));
//          m_uStencilRenderBuffer = 0;
//        }
//        break;
//      }
//
//
//      //}
//    }
//    break;
//  case DE_ResetDevice:
//    {
//      ASSERT(m_uTexture == 0/* && m_uFrameBuffer == 0*/);
//      switch(eFmtCate)
//      {
//      case GXFMTCATE_COLOR:
//        GLVERIFY(glGenTextures(1, &m_uTexture));
//        //if(TEST_FLAG(m_dwUsage, GXUSAGE_RENDERTARGET))
//        //{
//        //  GLVERIFY(glGenFramebuffers(1, &m_uFrameBuffer));
//        //}
//        break;
//      case GXFMTCATE_DEPTHSTENCIL:
//        m_glDepthFormat = GrapXToOpenGL::DepthFormat(m_Format);
//        m_glStencilFormat = GrapXToOpenGL::StencilFormat(m_Format);
//
//        if( m_glDepthFormat != 0 )
//        {
//          GLVERIFY(glGenRenderbuffers(1, &m_uDepthRenderBuffer));
//        }
//        if( m_glStencilFormat != 0 )
//        {
//          if(IS_DEPTHSTENCIL_NOT_IN_SAME_FBO)
//          {
//            GLVERIFY(glGenRenderbuffers(1, &m_uStencilRenderBuffer));
//          }
//          else
//            m_uStencilRenderBuffer = m_uDepthRenderBuffer;
//        }
//        break;
//      }
//
//      GLint level  = 0;  // 对基层纹理进行操作
//      const GXFormatCategory eFmtCate = GetGraphicsFormatCategory(m_Format);
//
//      //GXUINT nWidth, nHeight;
//      //GetDimension(&nWidth, &nHeight);
//      GXGRAPHICSDEVICE_DESC GrapDeviceDesc;
//      m_pGraphics->GetDesc(&GrapDeviceDesc);
//      GXDWORD dwFlags = TEST_FLAG(m_pGraphics->GetCaps(GXGRAPCAPS_TEXTURE), GXTEXTURECAPS_NONPOW2)
//        ? NULL : TEXTURERATIO_POW2;
//
//      if(m_nWidthRatio < 0)
//        m_nWidth = TextureRatioToDimension((GXINT)m_nWidthRatio, GrapDeviceDesc.BackBufferWidth, dwFlags);
//
//      if(m_nHeightRatio < 0)
//        m_nHeight = TextureRatioToDimension((GXINT)m_nHeightRatio, GrapDeviceDesc.BackBufferHeight, dwFlags);
//
//
//      if(eFmtCate == GXFMTCATE_COLOR)
//      {
//        if(m_uTexture == 0)
//          return GX_ERROR_HANDLE;
//
//        GrapXToOpenGL::PixelFormatFrom(m_Format, &m_glInternalFormat, &m_glFormat, &m_glType);
//
//        GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
//        GLASSERT(glTexImage2D(GL_TEXTURE_2D, level, m_glInternalFormat, m_nWidth, m_nHeight, 0, m_glFormat, m_glType, (GLvoid*)m_pLockedData));
//        // 采样器
//        GLVERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
//        GLVERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
//        GLVERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//        GLVERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//        m_pLockedData = NULL;
//
//        //if(TEST_FLAG(m_dwUsage, GXUSAGE_RENDERTARGET))
//        //{
//        //  GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, m_uFrameBuffer));
//        //  GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, NULL));
//        //}
//      }
//      else if(eFmtCate == GXFMTCATE_DEPTHSTENCIL)
//      {
//        //m_glDepthFormat = GrapXToOpenGL::DepthFormat(m_Format);
//        //m_glStencilFormat = GrapXToOpenGL::StencilFormat(m_Format);
//        ASSERT(m_glDepthFormat != 0 && m_glStencilFormat != 0);
//
//        if(m_uDepthRenderBuffer != 0)
//        {
//          GLVERIFY(glBindRenderbuffer(GL_RENDERBUFFER, m_uDepthRenderBuffer));
//          GLVERIFY(glRenderbufferStorage(GL_RENDERBUFFER, m_glDepthFormat, m_nWidth, m_nHeight));
//        }
//        if(m_uStencilRenderBuffer != 0 && IS_DEPTHSTENCIL_NOT_IN_SAME_FBO)
//        {
//          GLVERIFY(glBindRenderbuffer(GL_RENDERBUFFER, m_uStencilRenderBuffer));
//          GLVERIFY(glRenderbufferStorage(GL_RENDERBUFFER, m_glStencilFormat, m_nWidth, m_nHeight));
//        }
//      }
//    }
//    break;
//  case DE_ResizeDevice:
//    if(((GXINT)m_nWidth) < 0 || ((GXINT)m_nHeight) < 0)
//    {
//      OnDeviceEvent(DE_LostDevice);
//      OnDeviceEvent(DE_ResetDevice);
//    }
//    break;
//  default:
//    ASSERT(0);
//    return GX_FAIL;
//  }
//  return GX_OK;
//}

//GXLRESULT TextureImpl::AddRef()
//{
//  return gxInterlockedIncrement(&m_nRefCount);
//  //m_uRefCount++;
//  //return m_uRefCount;
//}
//
//GXLRESULT GTextureImpl::Release()
//{
//  m_uRefCount--;
//  if(m_uRefCount == 0)
//  {
//    //OnDeviceEvent(DE_LostDevice);
//    delete this;
//    return GX_OK;
//  }
//  else if(m_uRefCount == 1)
//  {
//    return m_pGraphics->OldUnregisterResource(this);
//  }
//
//  return m_uRefCount;
//}
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXLRESULT GTextureImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXLRESULT TextureImpl::Release()
{
  clstd::ScopeLocker sl(m_pGraphics->GetLocker());
  m_uRefCount--;
  if(m_uRefCount == 0)
  {
    //OnDeviceEvent(DE_LostDevice);
    INVOKE_LOST_DEVICE;

    // m_uTexture 为空说明创建失败,此时还没有注册
    if(m_uTexture != NULL)
    {
      m_pGraphics->UnregisterResource(this);
    }
    delete this;
    return GX_OK;
  }
  return m_uRefCount;
}

GXBOOL TextureImpl::SaveToFile(GXLPCWSTR pszFileName, GXLPCWSTR pszDestFormat)
{
  //GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
  //GXUINT nWidth, nHeight;
  //GetDimension(&nWidth, &nHeight);
  //GXBYTE* pData = new GXBYTE[nWidth * nHeight * GetBytesOfGraphicsFormat(m_Format)];
  //GLVERIFY(glGetTexImage(GL_TEXTURE_2D, 0, m_glInternalFormat, GL_UNSIGNED_BYTE, pData));
  //SAFE_DELETE(pData);

  //if(pszFileName == NULL)
  //  return FALSE;
  //D3DXIMAGE_FILEFORMAT d3diff = D3DXIFF_DDS;
  //if(IS_PTR(pszDestFormat))
  //{
  //  if(GXSTRCMPI(pszDestFormat, L"BMP") == 0)
  //    d3diff = D3DXIFF_BMP;
  //  else if(GXSTRCMPI(pszDestFormat, L"JPG") == 0)
  //    d3diff = D3DXIFF_JPG;
  //  else if(GXSTRCMPI(pszDestFormat, L"TGA") == 0)
  //    d3diff = D3DXIFF_TGA;
  //  else if(GXSTRCMPI(pszDestFormat, L"PNG") == 0)
  //    d3diff = D3DXIFF_PNG;
  //  else if(GXSTRCMPI(pszDestFormat, L"DDS") == 0)
  //    d3diff = D3DXIFF_DDS;
  //  else if(GXSTRCMPI(pszDestFormat, L"PPM") == 0)
  //    d3diff = D3DXIFF_PPM;
  //  else if(GXSTRCMPI(pszDestFormat, L"DIB") == 0)
  //    d3diff = D3DXIFF_DIB;
  //  else if(GXSTRCMPI(pszDestFormat, L"HDR") == 0)
  //    d3diff = D3DXIFF_HDR;
  //  else if(GXSTRCMPI(pszDestFormat, L"PFM") == 0)
  //    d3diff = D3DXIFF_PFM;

  //}
  //else if((GXUINT_PTR)pszDestFormat <= 8)
  //  d3diff = (D3DXIMAGE_FILEFORMAT)(GXUINT_PTR)pszDestFormat;
  //return GXSUCCEEDED(D3DXSaveTextureToFileW(pszFileName, d3diff, m_pTexture, NULL));
  return GX_OK;
}

TextureImpl::TextureImpl(Graphics* pGraphics)
  : Texture             ()
  , m_pGraphics         ((GraphicsImpl*)pGraphics)
  , m_pLockedData       (NULL)
  , m_uTexture          (0)
  , m_Format            (GXFMT_UNKNOWN)
  , m_nWidth            (0)
  , m_nHeight           (0)
  , m_nMipLevels        (0)
  , m_dwResUsage        (GXRU_DEFAULT)
  , m_glFormat          (0)
  , m_glType            (0)
  , m_glInternalFormat  (0)
  , m_nWidthRatio       (0)
  , m_nHeightRatio      (0)
{
  gxSetRect(&m_rcLocked, 0, 0, 0, 0);
}

GXBOOL GXDLLAPI GXSaveTextureToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat, Texture* pTexture)
{
  //if(pszFileName == NULL || pTexture == NULL)
  //  return FALSE;

  //D3DXIMAGE_FILEFORMAT d3diff = D3DXIFF_DDS;
  //if(IS_PTR(pszDestFormat))
  //{
  //  if(GXSTRCMPI(pszDestFormat, L"BMP") == 0)
  //    d3diff = D3DXIFF_BMP;
  //  else if(GXSTRCMPI(pszDestFormat, L"JPG") == 0)
  //    d3diff = D3DXIFF_JPG;
  //  else if(GXSTRCMPI(pszDestFormat, L"TGA") == 0)
  //    d3diff = D3DXIFF_TGA;
  //  else if(GXSTRCMPI(pszDestFormat, L"PNG") == 0)
  //    d3diff = D3DXIFF_PNG;
  //  else if(GXSTRCMPI(pszDestFormat, L"DDS") == 0)
  //    d3diff = D3DXIFF_DDS;
  //  else if(GXSTRCMPI(pszDestFormat, L"PPM") == 0)
  //    d3diff = D3DXIFF_PPM;
  //  else if(GXSTRCMPI(pszDestFormat, L"DIB") == 0)
  //    d3diff = D3DXIFF_DIB;
  //  else if(GXSTRCMPI(pszDestFormat, L"HDR") == 0)
  //    d3diff = D3DXIFF_HDR;
  //  else if(GXSTRCMPI(pszDestFormat, L"PFM") == 0)
  //    d3diff = D3DXIFF_PFM;

  //}
  //else if((GXUINT_PTR)pszDestFormat <= 8)
  //  d3diff = (D3DXIMAGE_FILEFORMAT)(GXUINT_PTR)pszDestFormat;
  //return GXSUCCEEDED(D3DXSaveTextureToFileW(pszFileName, d3diff, ((GTextureImpl*)pTexture)->D3DTexture(), NULL));    
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL TextureImpl::Clear(const GXLPRECT lpRect, GXCOLOR dwColor)
{
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXVOID TextureImpl::GenerateMipMaps()
{
  ASSERT(0);
}

GXBOOL TextureImpl::CopyRect(Texture* pSrc, const GXLPCRECT lpRect)
{
  GXRECT rect = {0, 0};
  TextureImpl* pSrcImpl = (TextureImpl*)pSrc;
  GXUINT uDestHeight = GetHeight();
  GXUINT uSrcWidth, uSrcHeight;

  pSrc->GetDimension(&uSrcWidth, &uSrcHeight);

  if(lpRect != NULL)
    rect = *lpRect;
  else
  {
    rect.right  = (GXLONG)uSrcWidth;
    rect.bottom = (GXLONG)uSrcHeight;
  }

  if(pSrc == NULL)
  {
    GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
    GLVERIFY(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, 
      rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0));
    return TRUE;
  }
  else
  {
    //GLuint uFrameBuffer = m_pGraphics->GetFrameBuffer();
    // ! 其实这里面的 Height 就相当于 GXGraphics 的 m_uTargetHeight
    GXUINT uSrcRectHeight = rect.bottom - rect.top;

    GLuint uPrevTexture = m_pGraphics->IntSetTargetTexture(pSrcImpl->m_uTexture);
    //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, uFrameBuffer));
    //GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pSrcImpl->m_uTexture, 0));
    GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));
    GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0));

    GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));

    GLVERIFY(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, rect.left, uDestHeight - rect.bottom, 
      rect.left, uSrcHeight - rect.bottom, rect.right - rect.left, (GLsizei)uSrcRectHeight));

    m_pGraphics->IntSetTargetTexture(uPrevTexture);
    //GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
    //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    //GLVERIFY(GLBindTexture(GL_TEXTURE_2D, 0));
  }
  return TRUE;
}

GXBOOL TextureImpl::StretchRect(Texture* pSrc, const GXLPCRECT lpDestRect, const GXLPCRECT lpSrcRect, GXTextureFilterType eFilter)
{
  GXRECT rcDst = {0, 0};
  GXRECT rcSrc = {0, 0};
  TextureImpl* pSrcImpl = (TextureImpl*)pSrc;

  GXUINT uSrcHeight  = pSrc->GetHeight();
  GXUINT uDestHeight = GetHeight();

  if(lpDestRect != NULL)
    rcDst = *lpDestRect;
  else
  {
    rcDst.right = GetWidth();
    rcDst.bottom = uDestHeight;
  }

  if(lpSrcRect != NULL)
    rcSrc = *lpSrcRect;
  else
  {
    rcSrc.right = pSrc->GetWidth();
    rcSrc.bottom = uSrcHeight;
  }

  //GLuint uFrameBuffer = m_pGraphics->GetFrameBuffer();
  GLuint uTexture = 0;
  //GLVERIFY(glGenTextures(1, &uTexture));

  // 针对没有缩放情况的处理
  if(rcDst.right - rcDst.left == rcSrc.right - rcSrc.left && 
    rcDst.bottom - rcDst.top == rcSrc.bottom - rcSrc.top )
  {
    GLuint uPrevTexture = m_pGraphics->IntSetTargetTexture(pSrcImpl->m_uTexture);
    //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, uFrameBuffer));
    //GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pSrcImpl->m_uTexture, 0));
    GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));
    GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0));

    GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));

    GLVERIFY(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, rcDst.left, uDestHeight - rcDst.bottom, 
      rcSrc.left, uSrcHeight - rcSrc.bottom, rcSrc.right - rcSrc.left, (GLsizei)rcSrc.bottom - rcSrc.top));

    //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, NULL));
    if(uPrevTexture != pSrcImpl->m_uTexture)
      m_pGraphics->IntSetTargetTexture(uPrevTexture);
    return TRUE;
  }
  TRACE(">error: TextureImpl::StretchRect: 没实现缩放拷贝.\n");
  GLuint uPrevTexture = m_pGraphics->IntSetTargetTexture(pSrcImpl->m_uTexture);
  //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, uFrameBuffer));
  //GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, 
  //  GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pSrcImpl->m_uTexture, 0));

  //GLVERIFY(GLBindTexture(GL_TEXTURE_2D, uTexture));
  GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
  //GLVERIFY(glTexImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, rcDst.right - rcDst.left, rcDst.bottom - rcDst.top, 0, m_glFormat, m_glType, NULL));
  //GLVERIFY(glTexImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, 100, 100, 0, m_glFormat, m_glType, NULL));

  //GLVERIFY(glCopyTexImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, rcSrc.left, uSrcHeight - rcSrc.bottom, 
  //  rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, 0));

  //GLVERIFY(glBlitFramebufferANGLE(rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom,
  //  rcDst.left, rcDst.top, rcDst.right, rcDst.bottom, GL_COLOR_BUFFER_BIT, GL_NEAREST));

  //GLVERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, 
  //  GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uTexture, 0));

  //GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
  //GLVERIFY(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rcDst.left, rcDst.top, 
  //  /*0, 0, */rcDst.right - rcDst.left, rcDst.bottom - rcDst.top));
  m_pGraphics->IntSetTargetTexture(uPrevTexture);
  //GLVERIFY(glBindFramebuffer(GL_FRAMEBUFFER, NULL));
  //GLVERIFY(GLBindTexture(GL_TEXTURE_2D, NULL));
  //GLVERIFY(glDeleteTextures(1, &uTexture));

  return TRUE;
}

GXBOOL TextureImpl::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)
{
  lpLockRect->pBits = NULL;
  lpLockRect->Pitch = NULL;
  ASSERT(m_pLockedData == NULL);
  ASSERT(m_uTexture > 0);
  if(m_pLockedData == NULL)
  {
    GXUINT nWidth, nHeight;
    GetDimension(&nWidth, &nHeight);
    if(lpRect == NULL)
    {
      gxSetRect(&m_rcLocked, 0, 0, nWidth, nHeight);
    }
    else
    {
      m_rcLocked = *lpRect;
      ASSERT(lpRect->left >= 0 && lpRect->top >= 0 &&
        lpRect->right <= (GXLONG)nWidth && lpRect->bottom <= (GXLONG)nHeight);
      ASSERT(lpRect->left < lpRect->right && lpRect->top < lpRect->bottom);
    }
    const size_t nByteOfPixel = GetBytesOfGraphicsFormat(m_Format);
    const GXINT nPitch = ALIGN_4(m_rcLocked.right - m_rcLocked.left) * nByteOfPixel;
    m_pLockedData = new GXBYTE[nPitch * (m_rcLocked.bottom - m_rcLocked.top) * 2];

    lpLockRect->pBits = m_pLockedData;
    lpLockRect->Pitch = nPitch;
    return TRUE;
  }
  return FALSE;
}

GXBOOL TextureImpl::UnlockRect()
{
  ASSERT(m_pLockedData != NULL);
  if(m_pLockedData != NULL)
  {
    ASSERT(m_uTexture > 0);
    GLVERIFY(GLBindTexture(GL_TEXTURE_2D, m_uTexture));
    GXUINT nHeight = GetHeight();
    const size_t nByteOfPixel = GetBytesOfGraphicsFormat(m_Format);
    const GXINT nPitch = ALIGN_4(m_rcLocked.right - m_rcLocked.left) * nByteOfPixel;

    size_t nFlipOffset = nPitch * (m_rcLocked.bottom - m_rcLocked.top);
    GXBYTE* pDst = (GXBYTE*)((size_t)m_pLockedData + nFlipOffset);
    GXBYTE* pSrc = pDst - nPitch;
    for(GXLONG i = m_rcLocked.bottom - m_rcLocked.top; i > 0; i--)
    {
      memcpy(pDst, pSrc, nPitch);
      pDst += nPitch;
      pSrc -= nPitch;
    }

    GLASSERT(glTexSubImage2D(GL_TEXTURE_2D, 0,
      m_rcLocked.left, 
      //m_rcLocked.top,
      nHeight - m_rcLocked.bottom,
      m_rcLocked.right - m_rcLocked.left, 
      m_rcLocked.bottom - m_rcLocked.top,
      m_glFormat, m_glType, (GLvoid*)((size_t)m_pLockedData + nFlipOffset)));
    //m_glFormat, m_glType, (GLvoid*)m_pLockedData));
    //GLASSERT(glTexSubImage2D(GL_TEXTURE_2D, 0,
    //  0, 0,
    //  16, 128,
    //  GL_ALPHA, GL_UNSIGNED_BYTE, (GLvoid*)m_pLockedData));
    //GLASSERT(glTexImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, m_rcLocked.right - m_rcLocked.left, 
    //  m_rcLocked.bottom - m_rcLocked.top, 0, m_glFormat, m_glType, (GLvoid*)m_pLockedData));

    delete m_pLockedData;
    m_pLockedData = NULL;
    return TRUE;
  }
  return FALSE;
}

Graphics*  TextureImpl::GetGraphicsUnsafe()
{
  return m_pGraphics;
}

GXBOOL TextureImpl::GetRatio(GXINT* pWidthRatio, GXINT* pHeightRatio)
{
  if(pWidthRatio != NULL)
    *pWidthRatio = (GXINT)m_nWidthRatio;
  if(pHeightRatio != NULL)
    *pHeightRatio = (GXINT)m_nHeightRatio;
  return TRUE;
}

GXUINT TextureImpl::GetWidth()
{
  return m_nWidth;
}
GXUINT TextureImpl::GetHeight()
{
  return m_nHeight;
}
GXBOOL TextureImpl::GetDimension(GXUINT* pWidth, GXUINT* pHeight)
{
  if(pWidth != NULL || pHeight != NULL)
  {
    if(pWidth != NULL)
    {
      *pWidth = m_nWidth;
    }
    if(pHeight != NULL)
    {
      *pHeight = m_nHeight;
    }

    return TRUE;
  }
  return FALSE;
}
GXDWORD TextureImpl::GetUsage()
{
  return m_dwResUsage;
}
GXFormat TextureImpl::GetFormat()
{
  return m_Format;
}
//GXPool TextureImpl::GetPool()
//{
//  return m_Pool;
//}

GXBOOL TextureImpl::GetDesc(GXBITMAP*lpBitmap)
{
  if(lpBitmap == NULL)
    return FALSE;

  lpBitmap->bmType      = 0;
  lpBitmap->bmPlanes    = 1;
  lpBitmap->bmBitsPixel = GetBytesOfGraphicsFormat(m_Format);
  lpBitmap->bmBits      = NULL;

  GetDimension((GXUINT*)&lpBitmap->bmWidth, (GXUINT*)&lpBitmap->bmHeight);
  lpBitmap->bmWidthBytes = lpBitmap->bmBitsPixel * lpBitmap->bmWidth;

  return TRUE;
}

GXHRESULT TextureImpl::Create(
  const GXSIZE* TexSize, const GXSIZE* ImageSize, 
  GXUINT MipLevels, GXFormat Format, 
  GXDWORD ResUsage, GXDWORD MipFilter, GXLPVOID lpBits)
{
  m_Format       = Format;
  m_dwResUsage   = ResUsage;
  m_nMipLevels   = MipLevels;
  m_nWidth       = TexSize->cx;
  m_nHeight      = TexSize->cy;
  if(ImageSize == NULL)
  {
    // TexSize 可能是比率
    m_nWidthRatio  = (GXSHORT)TexSize->cx;
    m_nHeightRatio = (GXSHORT)TexSize->cy;
  }
  else
  {
    m_nWidthRatio  = (GXSHORT)ImageSize->cx;
    m_nHeightRatio = (GXSHORT)ImageSize->cy;

    // TexSize 是确定的值
    ASSERT(TexSize->cx > 0 && TexSize->cy > 0);
  }
  ASSERT(MipLevels == 1);  // 暂时不支持多重纹理

  // 传递初始化图象数据
  m_pLockedData = (GXBYTE*)lpBits;
  //OnDeviceEvent(DE_ResetDevice);


  return GX_OK;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//GXLRESULT TextureOffscreenPlainSur::OnDeviceEvent(DeviceEvent eEvent)
//{
//  switch(eEvent)
//  {
//  case DE_ResetDevice:
//    return GX_FAIL;
//
//  case DE_LostDevice:
//    return TextureImpl::OnDeviceEvent(eEvent);
//  case DE_ResizeDevice:
//    break;
//  }
//  return GX_OK;
//}
GXBOOL TextureOffscreenPlainSur::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)
{
  return FALSE;
}
GXBOOL TextureOffscreenPlainSur::UnlockRect()
{
  return FALSE;
}


GXBOOL TextureOffscreenPlainSur::CopyRect(Texture* pSrc)
{
  return FALSE;
}
TextureOffscreenPlainSur::TextureOffscreenPlainSur(Graphics* pGraphics)
  : TextureImpl(pGraphics)
{
}
TextureOffscreenPlainSur::~TextureOffscreenPlainSur()
{
}

Texture_OriginFrameBuffer::Texture_OriginFrameBuffer(Graphics* pGraphics)
  : TextureImpl(pGraphics)
{
}

//GXLRESULT Texture_OriginFrameBuffer::OnDeviceEvent(DeviceEvent eEvent)
//{
//  switch(eEvent)
//  {
//  case DE_ResizeDevice:
//  case DE_ResetDevice:
//    GXGRAPHICSDEVICE_DESC gdd;
//    m_pGraphics->GetDesc(&gdd);
//    m_Format = gdd.BackBufferFormat;
//    m_nWidth = gdd.BackBufferWidth;
//    m_nHeight = gdd.BackBufferHeight;
//    return GX_OK;
//  case DE_LostDevice:
//    return GX_OK;
//  }
//  return GX_OK;
//}

GXUINT Texture_OriginFrameBuffer::GetWidth()
{
  return m_nWidth;
}

GXUINT Texture_OriginFrameBuffer::GetHeight()
{
  return m_nHeight;
}

GXBOOL Texture_OriginFrameBuffer::GetDimension(GXUINT* pWidth, GXUINT* pHeight)
{
  if(pWidth != NULL)
    *pWidth = m_nWidth;
  if(pHeight != NULL)
    *pHeight = m_nHeight;
  return TRUE;
}
