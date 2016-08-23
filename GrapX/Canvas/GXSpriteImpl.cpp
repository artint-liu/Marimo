// 全局头文件
#include <GrapX.H>

// 标准接口
#include "GrapX/GResource.H"
#include "GrapX/GXImage.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXSprite.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXKernel.H"

// 私有头文件
#include <clPathFile.h>
#include <clUtility.H>
#include <clStringSet.h>
#include <Smart/SmartStream.h>
#include <Smart/Stock.h>
#include <Canvas/GXSpriteImpl.H>

//using namespace std;
//////////////////////////////////////////////////////////////////////////
extern "C" GXBOOL GXDLLAPI gxSetRegn(GXLPREGN lprg, GXINT xLeft, GXINT yTop, GXINT xWidth, GXINT yHeight);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
GXSpriteImpl::GXSpriteImpl()
  : m_pImage(NULL)
{
  //AddRef();
}

GXSpriteImpl::~GXSpriteImpl()
{
  SAFE_RELEASE(m_pImage);
  //SAFE_DELETE_ARRAY(m_pRect);
  m_aModules.clear();
  m_SpriteDict.clear();
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXSpriteImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GXSpriteImpl::Release()
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

int GXSpriteImpl::IntGetSpriteCount() const
{
  return (int)m_aModules.size();
}

//////////////////////////////////////////////////////////////////////////


GXVOID GXSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const
{
  //const GXREGN& regn = ;
  if(nIndex < (GXINT)m_aModules.size()) {
    pCanvas->DrawImage(m_pImage, x, y, &m_aModules[nIndex].regn);
  }
}

GXVOID GXSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
{
  REGN rcDest;
  rcDest.left = x;
  rcDest.top = y;
  rcDest.width = nWidth;
  rcDest.height = nHeight;
  if(nIndex < (GXINT)m_aModules.size()) {
    pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nIndex].regn);
  }
}

//GXVOID GXSpriteImpl::PaintModuleH(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth) const
//{
//  REGN rcDest;
//  rcDest.left = x;
//  rcDest.top = y;
//  rcDest.width = nWidth;
//  rcDest.height = m_aModules[nIndex].regn.height;
//
//  if(nIndex < (GXINT)m_aModules.size()) {
//    pCanvas->DrawImage(m_pImage, &rcDest, m_aModules[nIndex].regn);
//  }
//}
//
//GXVOID GXSpriteImpl::PaintModuleV(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nHeight) const
//{
//  REGN rcDest;
//  rcDest.left = x;
//  rcDest.top = y;
//  rcDest.width = m_aModules[nIndex].regn.width;
//  rcDest.height = nHeight;
//
//  if(nIndex < (GXINT)m_aModules.size()) {
//    pCanvas->DrawImage(m_pImage, &rcDest, m_aModules[nIndex].regn);
//  }
//}

GXLONG GXSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
{
  if(nStartIdx + 3 > IntGetSpriteCount())
  {
    ASSERT(FALSE);
    return -1;
  }
  GXLONG nMaxHeight = clMax(m_aModules[nStartIdx].regn.height, clMax(m_aModules[nStartIdx + 1].regn.height, m_aModules[nStartIdx + 2].regn.height));
  float fVertScale = (float)nHeight / (float)nMaxHeight;
  float fHorzScale = (float)nWidth / (float)(m_aModules[nStartIdx].regn.width + m_aModules[nStartIdx + 2].regn.width);
  REGN rgTip0 = m_aModules[nStartIdx].regn;
  REGN rgTip1 = m_aModules[nStartIdx + 2].regn;
  REGN rgDest;

  if(fHorzScale > fVertScale)
  {
    //REGN rgTip0 = m_pRect[nStartIdx];
    //REGN rgDest;
    //REGN rgTip1 = m_pRect[nStartIdx + 2];
    float fScale0 = (float)nHeight / (float)m_aModules[nStartIdx    ].regn.height;
    float fScale1 = (float)nHeight / (float)m_aModules[nStartIdx + 1].regn.height;
    //float fScale2 = (float)nHeight / (float)m_aModules[nStartIdx + 2].regn.height;

    rgTip0.width  = (GXLONG)(rgTip0.width  * fScale0 + 0.5f);
    rgTip0.height = nHeight;
    rgTip1.width  = (GXLONG)(rgTip1.width  * fScale1 + 0.5f);
    rgTip1.height = nHeight;

    rgTip0.left = x;
    rgTip0.top  = y;
    rgTip1.left = x + nWidth - rgTip1.width;
    rgTip1.top  = y;

    rgDest.left   = x + rgTip0.width;
    rgDest.top    = y;
    rgDest.width  = nWidth - rgTip0.width - rgTip1.width;
    rgDest.height = nHeight;

    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx    ], &rgTip0);
    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx + 2], &rgTip1);
    pCanvas->DrawImage(m_pImage, &rgDest, &m_aModules[nStartIdx + 1].regn);
  }
  else
  {
    //REGN rgTip0 = m_pRect[nStartIdx];
    //REGN rgTip1 = m_pRect[nStartIdx + 2];
    //ASSERT(FALSE);
    rgTip0.left   = x;
    rgTip0.top    = y;
    rgTip0.width  = (GXLONG)((float)m_aModules[nStartIdx].regn.width * fHorzScale + 0.5f);
    rgTip0.height = nHeight;
    rgTip1.left   = x + rgTip0.width;
    rgTip1.top    = y;
    rgTip1.width  = nWidth - rgTip0.width;
    rgTip1.height = nHeight;

    rgDest.width = 0;
  }
  pCanvas->DrawImage(m_pImage, &rgTip0, &m_aModules[nStartIdx    ].regn);
  pCanvas->DrawImage(m_pImage, &rgTip1, &m_aModules[nStartIdx + 2].regn);
  return rgDest.width;  // 返回值是中间可缩放Sprite的宽度
}

GXLONG GXSpriteImpl::PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
{
  if(nStartIdx + 3 > IntGetSpriteCount())
  {
    ASSERT(FALSE);
    return -1;
  }
  GXLONG nMaxWidth = clMax(m_aModules[nStartIdx].regn.width, clMax(m_aModules[nStartIdx + 1].regn.width, m_aModules[nStartIdx + 2].regn.width));
  float fHorzScale = (float)nWidth / (float)nMaxWidth;
  float fVertScale = (float)nHeight / (float)(m_aModules[nStartIdx].regn.height + m_aModules[nStartIdx + 2].regn.height);
  REGN rgTip0 = m_aModules[nStartIdx].regn;
  REGN rgTip1 = m_aModules[nStartIdx + 2].regn;
  REGN rgDest;

  if(fVertScale > fHorzScale)
  {
    //REGN rgTip0 = m_pRect[nStartIdx];
    //REGN rgDest;
    //REGN rgTip1 = m_pRect[nStartIdx + 2];
    float fScale0 = (float)nWidth / (float)m_aModules[nStartIdx    ].regn.width;
    float fScale1 = (float)nWidth / (float)m_aModules[nStartIdx + 1].regn.width;
    //float fScale2 = (float)nWidth / (float)m_aModules[nStartIdx + 2].regn.width;

    rgTip0.height  = (GXLONG)(rgTip0.height  * fScale0 + 0.5f);
    rgTip0.width = nWidth;
    rgTip1.height  = (GXLONG)(rgTip1.height  * fScale1 + 0.5f);
    rgTip1.width = nWidth;

    rgTip0.left = x;
    rgTip0.top  = y;
    rgTip1.top = y + nHeight - rgTip1.height;
    rgTip1.left  = x;

    rgDest.top   = y + rgTip0.height;
    rgDest.left    = x;
    rgDest.height  = nHeight - rgTip0.height - rgTip1.height;
    rgDest.width = nWidth;

    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx    ], &rgTip0);
    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx + 2], &rgTip1);
    pCanvas->DrawImage(m_pImage, &rgDest, &m_aModules[nStartIdx + 1].regn);
  }
  else
  {
    //REGN rgTip0 = m_pRect[nStartIdx];
    //REGN rgTip1 = m_pRect[nStartIdx + 2];
    //ASSERT(FALSE);
    rgTip0.top     = y;
    rgTip0.left    = x;
    rgTip0.height  = (GXLONG)((float)m_aModules[nStartIdx].regn.height * fHorzScale + 0.5f);
    rgTip0.width = nWidth;
    rgTip1.top   = y + rgTip0.height;
    rgTip1.left    = x;
    rgTip1.height  = nHeight - rgTip0.height;
    rgTip1.width = nWidth;

    rgDest.height = 0;
  }
  pCanvas->DrawImage(m_pImage, &rgTip0, &m_aModules[nStartIdx    ].regn);
  pCanvas->DrawImage(m_pImage, &rgTip1, &m_aModules[nStartIdx + 2].regn);
  return rgDest.height;  // 返回值是中间可缩放Sprite的宽度
}

GXVOID GXSpriteImpl::PaintModule3x3(GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const
{
  // 0 1 2
  // 3 4 5
  // 6 7 8
  REGN rcDest;
  //rcDest.left = x;
  //rcDest.top = y;
  //rcDest.width = nWidth;
  //rcDest.height = nHeight;
  const GXINT width = rect->right - rect->left;
  const GXINT height = rect->bottom - rect->top;
  const GXINT top = rect->top;
  //const GXINT bottom = rect->bottom;

  /*左上角*/pCanvas->DrawImage(m_pImage, rect->left, rect->top, &m_aModules[nStartIdx].regn);
  /*右上角*/pCanvas->DrawImage(m_pImage, rect->right - m_aModules[nStartIdx + 2].regn.width, rect->top, &m_aModules[nStartIdx + 2].regn);
  /*左下角*/pCanvas->DrawImage(m_pImage, rect->left, rect->bottom - m_aModules[nStartIdx + 6].regn.height, &m_aModules[nStartIdx + 6].regn);
  /*右下角*/pCanvas->DrawImage(m_pImage, rect->right - m_aModules[nStartIdx + 8].regn.width, rect->bottom - m_aModules[nStartIdx + 8].regn.height, &m_aModules[nStartIdx + 8].regn);

//*
  rcDest.left   = rect->left + m_aModules[nStartIdx].regn.width;
  rcDest.top    = top;
  rcDest.width  = width - m_aModules[nStartIdx].regn.width - m_aModules[nStartIdx + 2].regn.width;
  rcDest.height = m_aModules[nStartIdx + 1].regn.height;
  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 1].regn);

  rcDest.left   = rect->left + m_aModules[nStartIdx + 6].regn.width;
  rcDest.top    = rect->top + height - m_aModules[nStartIdx + 7].regn.height;
  rcDest.width  = width  - m_aModules[nStartIdx + 6].regn.width - m_aModules[nStartIdx + 8].regn.width;
  rcDest.height = m_aModules[nStartIdx + 7].regn.height;
  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 7].regn);

  // 左边
  rcDest.left   = rect->left;
  rcDest.top    = rect->top + m_aModules[nStartIdx   ].regn.height;
  rcDest.width  = m_aModules[nStartIdx + 3].regn.width;
  rcDest.height = height - m_aModules[nStartIdx].regn.height - m_aModules[nStartIdx + 6].regn.height;
  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 3].regn);

  // 右边
  rcDest.left   = rect->right - m_aModules[nStartIdx + 5].regn.width;
  rcDest.top    = rect->top + m_aModules[nStartIdx + 2].regn.height;
  rcDest.width  = m_aModules[nStartIdx + 5].regn.width;
  rcDest.height = height - m_aModules[nStartIdx + 2].regn.height - m_aModules[nStartIdx + 8].regn.height;
  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 5].regn);
//*/
  if(bDrawEdge == FALSE)
  {
    rcDest.left   = m_aModules[nStartIdx + 3].regn.width;
    rcDest.top    = m_aModules[nStartIdx].regn.height;
    rcDest.width  = width  - m_aModules[nStartIdx + 3].regn.width  - m_aModules[nStartIdx + 5].regn.width;
    rcDest.height = height - m_aModules[nStartIdx].regn.height - m_aModules[nStartIdx + 8].regn.height;
    pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 4].regn);
  }
}

GXSIZE_T GXSpriteImpl::GetModuleCount() const
{
  return m_aModules.size();
}

GXBOOL GXSpriteImpl::GetNameA(IndexType eType, GXUINT nIndex, clStringA* pstrName) const
{
  switch(eType)
  {
  case GXSprite::IndexType_Module:
    if(nIndex >= m_aModules.size()) {
      return FALSE;
    }

    if(m_aModules[nIndex].name) {
      *pstrName = m_aModules[nIndex].name;
    }
    else {
      pstrName->Clear();
    }
    break;

  case GXSprite::IndexType_Frame:
  case GXSprite::IndexType_Animation:
  case GXSprite::IndexType_Unified:
    CLBREAK;
    break;
  }
  return pstrName->IsEmpty();
}

GXBOOL GXSpriteImpl::GetModuleRect(GXINT nIndex, GXRECT *rcSprite) const
{
  rcSprite->left   = m_aModules[nIndex].regn.left;
  rcSprite->top    = m_aModules[nIndex].regn.top;
  rcSprite->right  = m_aModules[nIndex].regn.width + m_aModules[nIndex].regn.left;
  rcSprite->bottom = m_aModules[nIndex].regn.height + m_aModules[nIndex].regn.top;
  return TRUE;
}

GXBOOL GXSpriteImpl::GetModuleRegion(GXINT nIndex, REGN *rgSprite) const
{
  *rgSprite  = m_aModules[nIndex].regn;
  return TRUE;
}

GXHRESULT GXSpriteImpl::GetImage(GXImage** ppImage)
{
  if(m_pImage == NULL) {
    return GX_FAIL;
  }

  *ppImage = m_pImage;
  return m_pImage->AddRef();
}

int GXSpriteImpl::FindByNameA(GXLPCSTR szName) const
{
  //clStringA strName = szName; // 改为直接的hash函数
  NameDict::const_iterator it = m_SpriteDict.find(szName);
  if(it == m_SpriteDict.end()) {
    return -1;
  }
  return it->second;
}

int GXSpriteImpl::FindByNameW(GXLPCWSTR szName) const
{
  clStringA strName = szName;
  return FindByNameA(strName);
}

//GXHRESULT GXSpriteImpl::SaveW(GXLPCWSTR szFilename) const
//{
//  clFile file;
//  if( ! file.CreateAlwaysW(szFilename)) {
//    CLOG_ERROR("Can't create file.\r\n");
//    return GX_FAIL;
//  }
//
//  if(m_strImageFile.IsEmpty()) {
//    CLOG_ERROR("Texture filename is empty.\r\n");
//    return GX_FAIL;
//  }
//
//  file.WritefA("smart{version = \"0.0.1.0\";}\r\n");
//  file.WritefA("Image {\r\n"); // <Image>
//
//  clStringW strRelativePath;
//  clpathfile::RelativePathToW(strRelativePath, szFilename, FALSE, m_strImageFile, FALSE);
//  clStringA strFilenameA(strRelativePath);
//  file.WritefA("  File=\"%s\";\r\n", strFilenameA);
//  file.WritefA("  Module {\r\n"); // <Module>
//
//  for(SpriteArray::const_iterator it = m_aModules.begin();
//    it != m_aModules.end(); ++it) {
//      file.WritefA("    rect {\r\n"); // <rect>
//      file.WritefA("      left=%d; top=%d;\r\n", it->regn.left, it->regn.top);
//      file.WritefA("      right=%d; bottom=%d;\r\n", it->regn.left + it->regn.width, it->regn.top + it->regn.height);
//      if(it->name.IsNotEmpty()) {
//        file.WritefA("      name=\"%s\"", it->name);
//      }
//      file.WritefA("    };\r\n"); // </rect>
//  }
//
//  file.WritefA("  };\r\n"); // </Module>
//  file.WritefA("};\r\n"); // </Image>
//  return GX_OK;
//}

clStringW GXSpriteImpl::GetImageFileW() const
{
  return m_strImageFile;
}

clStringA GXSpriteImpl::GetImageFileA() const
{
  return clStringA(m_strImageFile);
}

GXSIZE_T GXSpriteImpl::GetFrameCount() const
{
  return m_aFrames.size();
}

GXSIZE_T GXSpriteImpl::GetAnimationCount() const
{
  return m_aAnimations.size();
}

GXBOOL GXSpriteImpl::GetFrameBounding( GXINT nIndex, GXRECT* lprc ) const
{
  CLBREAK;
  return FALSE;
}

GXBOOL GXSpriteImpl::GetAnimBounding( GXINT nIndex, GXRECT* lprc ) const
{
  CLBREAK;
  return FALSE;
}

GXVOID GXSpriteImpl::PaintFrame( GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y ) const
{
  CLBREAK;
}

GXVOID GXSpriteImpl::PaintAnimationFrame( GXCanvas *pCanvas, GXINT idxAnim, GXINT idxFrame, GXINT x, GXINT y ) const
{
  CLBREAK;
}

GXVOID GXSpriteImpl::PaintSprite( GXCanvas *pCanvas, GXINT nUnifiedIndex, GXINT nMinorIndex, GXINT x, GXINT y, float xScale, float yScale ) const
{
  if(nUnifiedIndex < 0) {
    return;
  }
  else if(nUnifiedIndex < (GXINT)m_aModules.size()) {
    const GXREGN& regn = m_aModules[nUnifiedIndex].regn;
    PaintModule(pCanvas, nUnifiedIndex, x, y, (GXINT)(regn.width * xScale), (GXINT)(regn.height * yScale));
  }
}

GXBOOL GXSpriteImpl::GetSpriteBounding( GXINT nIndex, GXRECT* lprc ) const
{
  if(nIndex < 0) {
    return FALSE;
  }
  else if(nIndex < (GXINT)m_aModules.size()) {
    const GXREGN& regn = m_aModules[nIndex].regn;
    lprc->left   = 0;
    lprc->top    = 0;
    lprc->right  = regn.width;
    lprc->bottom = regn.height;
  }
  return TRUE;
}

GXBOOL GXSpriteImpl::GetSpriteBounding( GXINT nIndex, GXREGN* lprg ) const
{
  if(nIndex < 0) {
    return FALSE;
  }
  else if(nIndex < (GXINT)m_aModules.size()) {
    const GXREGN& regn = m_aModules[nIndex].regn;
    lprg->left   = 0;
    lprg->top    = 0;
    lprg->width  = regn.width;
    lprg->height = regn.height;
  }
  return TRUE;
}

GXBOOL GXSpriteImpl::Initialize(GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc)
{
  m_strImageFile = pDesc->szImageFile;
  m_pImage = pGraphics->CreateImageFromFile(m_strImageFile);

  if( ! m_pImage) {
    CLOG_ERRORW(L"GXSprite : Can not create sprite image(\"%s\").\r\n", m_strImageFile);
    return FALSE;
  }

  if(pDesc->nNumOfModules == 0 || pDesc->aModules == NULL) {
    CLOG_ERROR("%s : Sprite module is empty.\r\n", __FUNCTION__);
    return FALSE;
  }

  //
  // Module 列表
  //
  GXSprite::MODULE sModule;
  for(GXUINT i = 0; i < pDesc->nNumOfModules; i++)
  {
    sModule = pDesc->aModules[i];
    if(sModule.name) {
      sModule.name = m_NameSet.add(sModule.name);
      m_SpriteDict[sModule.name] = i;
    }
    m_aModules.push_back(sModule);
  }


  //
  // Frame 列表
  //
  GXSprite::FRAME sFrame;
  for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
  {
    sFrame = pDesc->aFrames[i];
    if(sFrame.name) {
      sFrame.name = m_NameSet.add(sFrame.name);
      m_SpriteDict[sFrame.name] = (int)(m_aModules.size() + i);
    }
    m_aFrames.push_back(sFrame);
  }

  for(GXUINT i = 0; i < pDesc->nNumOfFrameModules; i++)
  {
    m_aFrameModules.push_back(pDesc->aFrameModules[i]);    
  }

  //
  // Animation 列表
  //
  GXSprite::ANIMATION sAnimation;
  for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
  {
    sAnimation = pDesc->aAnimations[i];
    if(sAnimation.name) {
      sAnimation.name = m_NameSet.add(sAnimation.name);
      m_SpriteDict[sAnimation.name] = (int)(m_aModules.size() + m_aFrames.size() + i);
    }
    m_aAnimations.push_back(sAnimation);
  }

  for(GXUINT i = 0; i < pDesc->nNumOfAnimFrames; i++)
  {
    m_aAnimFrames.push_back(pDesc->aAnimFrames[i]);    
  }
  return TRUE;
}

GXBOOL GXSpriteImpl::Initialize( GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount )
{
  m_pImage = pGraphics->CreateImageFromFile(szTextureFile);
  m_strImageFile = szTextureFile;

  GXSpriteImpl::MODULE sSprite;
  sSprite.name = NULL;

  for(GXSIZE_T i = 0; i < nCount; i++)
  {
    sSprite.regn = arrayRegion[i];
    m_aModules.push_back(sSprite);
  }

  return TRUE;
}

GXBOOL GXSpriteImpl::GetModule( GXINT nIndex, MODULE* pModule ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aModules.size() && pModule != NULL) {
    *pModule = m_aModules[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXBOOL GXSpriteImpl::GetFrame( GXINT nIndex, FRAME* pFrame ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aFrames.size() && pFrame != NULL) {
    *pFrame = m_aFrames[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXUINT GXSpriteImpl::GetFrameModule( GXINT nIndex, FRAME_MODULE* pFrameModule, int nCount ) const
{
  if(nIndex < 0 || nIndex >= (GXINT)m_aFrames.size()) {
    return 0;
  }

  if(pFrameModule == NULL || nCount <= 0) {
    return m_aFrames[nIndex].count;
  }

  const GXUINT nNumOfCopy = clMin(nCount, m_aFrames[nIndex].count);
  memcpy(pFrameModule, &m_aFrameModules[m_aFrames[nIndex].start], sizeof(FRAME_MODULE) * nNumOfCopy);
  return nNumOfCopy;
}

GXBOOL GXSpriteImpl::GetAnimation( GXINT nIndex, ANIMATION* pAnimation ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aAnimations.size() && pAnimation != NULL) {
    *pAnimation = m_aAnimations[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXUINT GXSpriteImpl::GetAnimFrame( GXINT nIndex, ANIM_FRAME* pAnimFrame, int nCount ) const
{
  if(nIndex < 0 || nIndex >= (GXINT)m_aAnimations.size()) {
    return 0;
  }

  if(pAnimFrame == NULL || nCount <= 0) {
    return m_aAnimations[nIndex].count;
  }

  const GXUINT nNumOfCopy = clMin(nCount, m_aAnimations[nIndex].count);
  memcpy(pAnimFrame, &m_aAnimFrames[m_aAnimations[nIndex].start], sizeof(ANIM_FRAME) * nNumOfCopy);
  return nNumOfCopy;
}

//GXVOID GXSpriteImpl::PaintByUniformIndex( GXCanvas *pCanvas, GXINT nIndex, GXINT nMinorIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight ) const
//{
//  CLBREAK;
//}

