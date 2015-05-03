// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GRegion.H"
#include "Include/GTexture.H"
#include "Include/GXGraphics.H"
#include "Include/GXImage.H"
#include "Include/GXKernel.H"

// ƽ̨���
// ˽��ͷ�ļ�
#include "GXImageImpl.H"
#include "clstd/clUtility.H"

GXImageImpl::GXImageImpl(GXGraphics* pGraphics)
: GXImage    ()
, m_pGraphics  (pGraphics)
, m_pNativeTex  (NULL)
, m_pHelperTex  (NULL)
, m_nWidth    (0)
, m_nHeight    (0)
, m_eFormat    (GXFMT_UNKNOWN)
, m_dwFlags    (NULL)
{
}

GXImageImpl::GXImageImpl(GXGraphics* pGraphics, GXLONG nWidth, GXLONG nHeight, GXFormat eFormat)
: GXImage()
, m_pGraphics  (pGraphics)
, m_pNativeTex(NULL)
, m_pHelperTex(NULL)
, m_nWidth(nWidth)
, m_nHeight(nHeight)
, m_eFormat(eFormat)
, m_dwFlags(NULL)
{
}

GXImageImpl::~GXImageImpl()
{
  GXGraphics* pGraphics = m_pNativeTex->GetGraphicsUnsafe();
  SAFE_RELEASE(m_pNativeTex);
  SAFE_RELEASE(m_pHelperTex);
  pGraphics->UnregisterResource(this);
}

GXBOOL GXImageImpl::Initialize(GTexture*pTexture)
{
  m_pNativeTex = pTexture;
  m_pNativeTex->AddRef();
  m_eFormat = pTexture->GetFormat();
  //pTexture->GetRatio((GXINT*)&m_nWidth, (GXINT*)&m_nHeight);
  UpdateDimension();
  ASSERT(m_nWidth > 0 && m_nHeight > 0);

  UpdateFlags();

  return TRUE;
}
GXBOOL GXImageImpl::Initialize(GXBOOL bRenderable, LPGXIMAGEINFOX pSrcFile/*ò��û����*/, GXLPVOID lpBits)
{
  if(m_pNativeTex == NULL)
  {
    GXDWORD dwUsage = bRenderable ? GXRU_TEX_RENDERTARGET|GXRU_FREQUENTLYREAD : NULL;
    m_pGraphics->CreateTexture(&m_pNativeTex, NULL, m_nWidth, m_nHeight, 1, m_eFormat, dwUsage);

    // �����������Ļ����ֵ, �ͷ���Ϊʵ�ʳߴ�
    //if(m_nWidth < 0 || m_nHeight < 0)
    //  m_pNativeTex->GetDimension((GXUINT*)&m_nWidth, (GXUINT*)&m_nHeight);
    UpdateDimension();


    GXUINT nHelpWidth = 0, nHelpHeight = 0;
    if(bRenderable != FALSE || lpBits != NULL)
    {
      m_pNativeTex->GetDimension(&nHelpWidth, &nHelpHeight);
      m_pGraphics->CreateTexture(&m_pHelperTex, NULL, nHelpWidth, nHelpHeight, 1, m_eFormat, GXRU_SYSTEMMEM);
      ASSERT(m_pHelperTex != NULL);
    }

    if(lpBits != NULL)
    {
      GTexture::LOCKEDRECT lr;
      if(m_eFormat == GXFMT_A8R8G8B8)
      {
        if(m_pHelperTex->LockRect(&lr, NULL, 0)) {
          memcpy(lr.pBits, lpBits, nHelpWidth * nHelpHeight * 4);  // TODO: Ӧ�ü���һ���ߴ�ת��
          m_pHelperTex->UnlockRect();
        }
        SetHelperState(HS_Fetch, NULL);
      }
      else
        ASSERT(m_eFormat);
    }
  }
  UpdateFlags();
  return TRUE;
}
void GXImageImpl::UpdateDimension()
{
  GXGRAPHICSDEVICE_DESC GrapDeviceDesc;
  m_pGraphics->GetDesc(&GrapDeviceDesc);
  GXINT nWidthRatio, nHeightRatio;
  m_pNativeTex->GetRatio(&nWidthRatio, &nHeightRatio);

  if(nWidthRatio < 0)
    m_nWidth = TextureRatioToDimension(nWidthRatio, GrapDeviceDesc.BackBufferWidth, NULL);
  else
    m_nWidth = nWidthRatio;

  if(nHeightRatio < 0)
    m_nHeight = TextureRatioToDimension(nHeightRatio, GrapDeviceDesc.BackBufferHeight, NULL);
  else
    m_nHeight = nHeightRatio;
}
void GXImageImpl::UpdateFlags()
{
  if(m_nWidth != m_pNativeTex->GetWidth() || m_nHeight != m_pNativeTex->GetHeight())
    SET_FLAG(m_dwFlags, GXIMAGEFLAG_NOTSAMEWITHTEXTURE);
}
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXImageImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GXImageImpl::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  ASSERT((nRefCount & 0x80000000) == 0);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  //else if(m_uRefCount == 1)
  //{
  //  return m_pNativeTex->GetGraphicsUnsafe()->OldUnregisterResource(this);
  //}
  //else
  //{
  //  // ���������ȫ�ͷ�,����º�̨����
  //  SetHelperState(HS_Hold, NULL);
  //}
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT GXImageImpl::Invoke(GRESCRIPTDESC* pDesc)
{
  INVOKE_DESC_CHECK(pDesc);
  //if(m_pNativeTex->m_pOwner == this)
  //  m_pNativeTex->Invoke(pDesc);
  //if(m_pNativeTex->m_pOwner == this)
  //  m_pHelperTex->Invoke(pDesc);

  switch(pDesc->dwCmdCode)
  {
  case RC_ResetDevice:
    ////ASSERT(0); // ���ﻹ����!! ���ǹ��˺ܾú���û�������ﲻ��?
    //m_pNativeTex->GetDimension((GXUINT*)&m_nWidth, (GXUINT*)&m_nHeight);
    //if(m_pHelperTex != NULL)
    //{
    //  GXUINT nHelperWidth, nHelperHeight;
    //  m_pHelperTex->GetDimension(&nHelperWidth, &nHelperHeight);

    //  GXRECT rect = {0, 0, clMin((GXUINT)m_nWidth, nHelperWidth), clMin((GXUINT)m_nHeight, nHelperHeight)};
    //  SetHelperState(HS_Fetch, (GXLPARAM)&rect);

    //  if(nHelperWidth != m_nWidth || nHelperHeight != m_nHeight)
    //  {
    //    SAFE_RELEASE(m_pHelperTex);
    //    m_pNativeTex->GetGraphicsUnsafe()->CreateTexture
    //      (&m_pHelperTex, NULL, m_nWidth, m_nHeight, 1, m_eFormat, GXRU_SYSTEMMEM);
    //    SetHelperState(HS_Hold, NULL);
    //  }
    //}
    break;
  }
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////
GXImage* GXImageImpl::Clone() const
{
  GXImage* pImage = NULL;
  ASSERT(0);
  return NULL;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXImageImpl::GetDesc(GXBITMAP*lpBitmap) const
{
  // TODO: ��������Ҫ�������������ֵ
  return m_pNativeTex->GetDesc(lpBitmap);
}

GXINT GXImageImpl::GetWidth() const
{
  return m_nWidth;
}

GXINT GXImageImpl::GetHeight() const
{
  return m_nHeight;
}
void GXImageImpl::GetDimension(GXINT* pWidth, GXINT* pHeight) const
{
  if(pWidth != NULL)
    *pWidth = m_nWidth;
  if(pHeight != NULL)
    *pHeight = m_nHeight;
}
GXHRESULT GXImageImpl::SetHelperState(HelperState eState, GXLPARAM lParam)
{
  //switch(eState)
  //{
  //case HS_Create:
  //  if(m_pHelperTex == NULL)
  //  {
  //    return m_pNativeTex->GetGraphicsUnsafe()->CreateTexture
  //      (&m_pHelperTex, NULL, m_nWidth, m_nHeight, 1, m_eFormat, GXRU_SYSTEMMEM);
  //  }
  //  break;
  //case HS_Release:
  //  if(m_pHelperTex != NULL)
  //  {
  //    GXHRESULT hr = m_pHelperTex->Release();
  //    m_pHelperTex = NULL;
  //    return hr;
  //  }
  //  break;
  //case HS_Hold:
  //  if(m_pHelperTex != NULL)
  //  {
  //    m_pHelperTex->CopyRect(m_pNativeTex, (GXLPCRECT)lParam);
  //    return GX_OK;
  //  }
  //  break;
  //case HS_Fetch:
  //  if(m_pHelperTex != NULL)
  //  {
  //    m_pNativeTex->CopyRect(m_pHelperTex, (GXLPCRECT)lParam);
  //    return GX_OK;
  //  }
  //  break;
  //}
  return GX_FAIL;
}

GXBOOL GXImageImpl::Scroll(int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate)
{
  SCROLLTEXTUREDESC ScrollTexDesc;
  GXGraphics* pGraphics = m_pNativeTex->GetGraphicsUnsafe();

  ScrollTexDesc.pOperationTex = m_pNativeTex;
  ScrollTexDesc.pTempTex      = NULL;
  ScrollTexDesc.dx            = dx;
  ScrollTexDesc.dy            = dy;
  ScrollTexDesc.lprcScroll    = lprcScroll;
  ScrollTexDesc.lprgnClip     = lprgnClip;
  ScrollTexDesc.lpprgnUpdate  = lpprgnUpdate;
  ScrollTexDesc.lprcUpdate    = NULL;

  pGraphics->ScrollTexture(&ScrollTexDesc);

  SetHelperState(HS_Hold, NULL);
  return TRUE;
}

GXHRESULT GXImageImpl::GetTexture(GTexture** ppTexture) const
{
  GXHRESULT hr = m_pNativeTex->AddRef();
  *ppTexture = m_pNativeTex;
  return hr;
}

GTexture* GXImageImpl::GetTextureUnsafe()
{
  return m_pNativeTex;
}

GXBOOL GXImageImpl::SaveToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat)
{
  return m_pNativeTex->SaveToFileW(szFileName, szDestFormat);
}

GXBOOL GXImageImpl::BitBltRegion(GXImage* pSource, int xDest, int yDest, GRegion* lprgnSource)
{
  int nCount = lprgnSource->GetRectCount();
  GXRECT* lpRects = _GlbLockStaticRects(nCount);
  GXRECT rcDest;

  lprgnSource->GetRects(lpRects, nCount);
  for(int i = 0; i < nCount; i++)
  {
    rcDest.left   = lpRects[i].left + xDest;
    rcDest.top    = lpRects[i].top + yDest;
    rcDest.right  = lpRects[i].right + xDest;
    rcDest.bottom = lpRects[i].bottom + yDest;

    m_pNativeTex->StretchRect(pSource->GetTextureUnsafe(), &rcDest, &lpRects[i], GXTEXFILTER_POINT);
  }

  SetHelperState(HS_Hold, NULL);

  _GlbUnlockStaticRects(lpRects);
  return TRUE;
}