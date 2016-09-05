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
  m_NameDict.clear();
  m_IDDict.clear();
  //m_SpriteDict.clear();
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

GXVOID GXSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const
{
  if(nIndex < (GXINT)m_aModules.size()) {
    pCanvas->DrawImage(m_pImage, lpRegn, &m_aModules[nIndex].regn);
  }
}

GXVOID GXSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const
{
  REGN rcDest;
  rcDest.left   = x;
  rcDest.top    = y;
  rcDest.width  = right - x;
  rcDest.height = height - y;
  ASSERT(rcDest.width >= 0 && rcDest.height >= 0);

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

void GXSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
{
  if(nStartIdx + 3 > IntGetSpriteCount())
  {
    CLBREAK;
    //return -1;
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
  //return rgDest.width;  // 返回值是中间可缩放Sprite的宽度
}

void GXSpriteImpl::PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
{
  if(nStartIdx + 3 > IntGetSpriteCount())
  {
    CLBREAK;
    //return -1;
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
  //return rgDest.height;  // 返回值是中间可缩放Sprite的宽度
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

//GXBOOL GXSpriteImpl::GetNameA(IndexType eType, GXUINT nIndex, clStringA* pstrName) const
//{
//  switch(eType)
//  {
//  case GXSprite::IndexType_Module:
//    if(nIndex >= m_aModules.size()) {
//      return FALSE;
//    }
//
//    if(m_aModules[nIndex].name) {
//      *pstrName = m_aModules[nIndex].name;
//    }
//    else {
//      pstrName->Clear();
//    }
//    break;
//
//  case GXSprite::IndexType_Frame:
//  case GXSprite::IndexType_Animation:
//  case GXSprite::IndexType_Unified:
//    CLBREAK;
//    break;
//  }
//  return pstrName->IsEmpty();
//}

GXBOOL GXSpriteImpl::GetModuleRect(GXUINT nIndex, GXRECT *rcSprite) const
{
  rcSprite->left   = m_aModules[nIndex].regn.left;
  rcSprite->top    = m_aModules[nIndex].regn.top;
  rcSprite->right  = m_aModules[nIndex].regn.width + m_aModules[nIndex].regn.left;
  rcSprite->bottom = m_aModules[nIndex].regn.height + m_aModules[nIndex].regn.top;
  return TRUE;
}

GXBOOL GXSpriteImpl::GetModuleRegion(GXUINT nIndex, REGN *rgSprite) const
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

//int GXSpriteImpl::FindByNameA(GXLPCSTR szName) const
//{
//  //clStringA strName = szName; // 改为直接的hash函数
//  NameDict::const_iterator it = m_SpriteDict.find(szName);
//  if(it == m_SpriteDict.end()) {
//    return -1;
//  }
//  return it->second;
//}
//
//int GXSpriteImpl::FindByNameW(GXLPCWSTR szName) const
//{
//  clStringA strName = szName;
//  return FindByNameA(strName);
//}

const GXSpriteImpl::IDATTR* GXSpriteImpl::IntFind(ID id) const
{
  auto it = m_IDDict.find(id);
  if(it == m_IDDict.end()) {
    return NULL;
  }
  return &it->second;
}

const GXSpriteImpl::IDATTR* GXSpriteImpl::IntFind(GXLPCSTR szName) const
{
  auto it = m_NameDict.find(szName);
  if(it == m_NameDict.end()) {
    return NULL;
  }
  return &it->second;
}

GXINT GXSpriteImpl::AttrToIndex(const IDATTR* pAttr) const
{
  switch(pAttr->type)
  {
  case GXSprite::Type_Empty:
    return pAttr->index;
  case GXSprite::Type_Module:
    return (GXINT)(pAttr->pModel - &m_aModules.front());
  case GXSprite::Type_Frame:
    return (GXINT)(pAttr->pFrame - &m_aFrames.front());
  case GXSprite::Type_Animation:
    return (GXINT)(pAttr->pAnination- &m_aAnimations.front());
  }
  return -1;
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

GXSIZE_T GXSpriteImpl::GetFrameModuleCount(GXUINT nFrameIndex) const
{
  if(nFrameIndex >= (GXUINT)m_aFrames.size()) {
    return 0;
  }
  return m_aFrames[nFrameIndex].count;
}

GXSIZE_T GXSpriteImpl::GetAnimationCount() const
{
  return m_aAnimations.size();
}

GXSIZE_T GXSpriteImpl::GetAnimFrameCount(GXUINT nAnimIndex) const
{
  if(nAnimIndex >= (GXUINT)m_aAnimations.size()) {
    return 0;
  }
  return m_aAnimations[nAnimIndex].count;
}

GXBOOL GXSpriteImpl::GetFrameBounding( GXUINT nIndex, GXRECT* lprc ) const
{
  CLBREAK;
  return FALSE;
}

GXBOOL GXSpriteImpl::GetAnimBounding( GXUINT nIndex, GXRECT* lprc ) const
{
  CLBREAK;
  return FALSE;
}

GXVOID GXSpriteImpl::PaintFrame( GXCanvas *pCanvas, GXUINT nIndex, GXINT x, GXINT y ) const
{
  CLBREAK;
}

GXVOID GXSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXUINT nIndex, GXLPCREGN lpRegn) const
{
  CLBREAK;
}

GXVOID GXSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXUINT nIndex, GXLPCRECT lpRect) const
{
  CLBREAK;
}


//GXVOID GXSpriteImpl::PaintAnimationFrame( GXCanvas *pCanvas, GXINT idxAnim, GXINT idxFrame, GXINT x, GXINT y ) const
//{
//  CLBREAK;
//}

GXVOID GXSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXINT x, GXINT y) const
{
}

GXVOID GXSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCREGN lpRegn) const
{
}

GXVOID GXSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCRECT lpRect) const
{
}

GXVOID GXSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const
{
}

GXVOID GXSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const
{
}

GXVOID GXSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCRECT lpRect) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const
{
}

GXVOID GXSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const
{
}

//////////////////////////////////////////////////////////////////////////

GXINT GXSpriteImpl::Find(ID id, GXOUT Type* pType) const
{
  auto pAttr = IntFind(id);
  if(pType) {
    *pType = pAttr->type;
  }
  return AttrToIndex(pAttr);
}

GXINT GXSpriteImpl::Find(GXLPCSTR szName, GXOUT Type* pType) const
{
  auto pAttr = IntFind(szName);
  if(pType) {
    *pType = pAttr->type;
  }
  return AttrToIndex(pAttr);
}

GXINT GXSpriteImpl::Find(GXLPCWSTR szName, GXOUT Type* pType) const
{
  return Find(clStringA(szName), pType);
}

GXLPCSTR GXSpriteImpl::FindName(ID id) const
{
  auto pAttr = IntFind(id);
  switch(pAttr->type)
  {
  case GXSprite::Type_Module:
    return pAttr->pModel->name;
  case GXSprite::Type_Frame:
    return pAttr->pFrame->name;
  case GXSprite::Type_Animation:
    return pAttr->pAnination->name;
  }
  return NULL;
}

GXSprite::ID GXSpriteImpl::FindID(GXLPCSTR szName) const
{
  auto pAttr = IntFind(szName);
  switch(pAttr->type)
  {
  case GXSprite::Type_Module:
    return pAttr->pModel->id;
  case GXSprite::Type_Frame:
    return pAttr->pFrame->id;
  case GXSprite::Type_Animation:
    return pAttr->pAnination->id;
  }
  return 0;
}

GXSprite::ID GXSpriteImpl::FindID(GXLPCWSTR szName) const
{
  return FindID(clStringA(szName));
}

//////////////////////////////////////////////////////////////////////////

GXINT GXSpriteImpl::PackIndex(Type type, GXUINT index) const
{
  switch(type)
  {
  case GXSprite::Type_Module:
    return index;
  case GXSprite::Type_Frame:
    return (GXINT)m_aModules.size() + (GXINT)index;
  case GXSprite::Type_Animation:
    return (GXINT)m_aModules.size() + (GXINT)m_aFrames.size() + (GXINT)index;
  }
  return -1;
}

GXINT GXSpriteImpl::UnpackIndex(GXUINT nUniqueIndex, Type* pType) const
{
  const GXUINT nModuleCount = m_aModules.size();
  const GXUINT nModuleFrameCount = nModuleCount + m_aFrames.size();
  const GXUINT nModuleFrameAnimCount = nModuleFrameCount + m_aAnimations.size();

  if(nUniqueIndex < nModuleCount) {
    if(pType) {
      *pType = GXSprite::Type_Module;
    }
    return nUniqueIndex;
  }
  else if(nUniqueIndex < nModuleFrameCount) {
    if(pType) {
      *pType = GXSprite::Type_Frame;
    }
    return nUniqueIndex - nModuleCount;
  }
  else if(nUniqueIndex < nModuleFrameAnimCount) {
    if(pType) {
      *pType = GXSprite::Type_Animation;
    }
    return nUniqueIndex - nModuleFrameCount;
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////

//GXVOID GXSpriteImpl::PaintSprite( GXCanvas *pCanvas, GXINT nUnifiedIndex, GXINT nMinorIndex, GXINT x, GXINT y, float xScale, float yScale ) const
//{
//  if(nUnifiedIndex < 0) {
//    return;
//  }
//  else if(nUnifiedIndex < (GXINT)m_aModules.size()) {
//    const GXREGN& regn = m_aModules[nUnifiedIndex].regn;
//    PaintModule(pCanvas, nUnifiedIndex, x, y, (GXINT)(regn.width * xScale), (GXINT)(regn.height * yScale));
//  }
//}

//////////////////////////////////////////////////////////////////////////

void GXSpriteImpl::IntGetBounding(const IDATTR* pAttr, GXREGN* lprg) const
{
  switch(pAttr->type)
  {
  case GXSprite::Type_Module:
    ASSERT(pAttr->pModel >= &m_aModules.front() &&
      pAttr->pModel <= &m_aModules.back());

    *lprg = pAttr->pModel->regn;
    break;
  case GXSprite::Type_Frame:
    {
      ASSERT(pAttr->pFrame>= &m_aFrames.front() &&
        pAttr->pFrame <= &m_aFrames.back());

      const FRAME& f = *pAttr->pFrame;
      GXREGN rg = m_aModules[f.start].regn;
      for(GXUINT i = 1; i < f.count; i++)
      {
        gxUnionRegn(&rg, &rg, &m_aModules[i].regn);
      }
      *lprg = rg;
    }
    break;
  case GXSprite::Type_Animation:
    ASSERT(pAttr->pAnination>= &m_aAnimations.front() &&
      pAttr->pAnination <= &m_aAnimations.back());
    CLBREAK; // 没实现
    break;
  }
}

template<typename _TID>
GXSprite::Type GXSpriteImpl::GetBoundingT(_TID id, GXLPRECT lprc) const
{
  auto pAttr = IntFind(id);
  if( ! pAttr) {
    return GXSprite::Type_Empty;
  }
  GXREGN regn;
  IntGetBounding(pAttr, &regn);
  gxRegnToRect(lprc, &regn);
  return pAttr->type;
}

template<typename _TID>
GXSprite::Type GXSpriteImpl::GetBoundingT(_TID id, GXLPREGN lprg) const
{
  auto pAttr = IntFind(id);
  if( ! pAttr) {
    return GXSprite::Type_Empty;
  }

  IntGetBounding(pAttr, lprg);
  return pAttr->type;
}


GXSprite::Type GXSpriteImpl::GetBounding(ID id, GXRECT* lprc) const
{
  return GetBoundingT(id, lprc);
}

GXSprite::Type GXSpriteImpl::GetBounding(ID id, GXREGN* lprg) const
{
  return GetBoundingT(id, lprg);
}

GXSprite::Type GXSpriteImpl::GetBounding(GXLPCSTR szName, GXLPRECT lprc) const
{
  return GetBoundingT(szName, lprc);
}

GXSprite::Type GXSpriteImpl::GetBounding(GXLPCSTR szName, GXLPREGN lprg) const
{
  return GetBoundingT(szName, lprg);
}

GXSprite::Type GXSpriteImpl::GetBounding(GXLPCWSTR szName, GXLPRECT lprc) const
{
  return GetBoundingT(clStringA(szName), lprc);
}

GXSprite::Type GXSpriteImpl::GetBounding(GXLPCWSTR szName, GXLPREGN lprg) const
{
  return GetBoundingT(clStringA(szName), lprg);
}

//////////////////////////////////////////////////////////////////////////

template<class _TArray, class _TDesc>
void GXSpriteImpl::Add(_TArray& aArray, GXSprite::Type type, _TDesc& desc)
{
  IDATTR attr;
  attr.type = type;
  attr.index = (GXUINT)aArray.size();

  if(desc.name) {
    desc.name = m_NameSet.add(desc.name);
    m_NameDict.insert(clmake_pair(desc.name, attr));
  }

  if(desc.id) {
    m_IDDict.insert(clmake_pair(desc.id, attr));
  }
  aArray.push_back(desc);
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
    //IDATTR attr = {GXSprite::Type_Module, m_aModules.size()};
    //if(sModule.name) {
    //  sModule.name = m_NameSet.add(sModule.name);
    //  //m_SpriteDict[sModule.name] = i;
    //  m_NameDict.insert(clmake_pair(sModule.name, attr));
    //}
    //if(sModule.id) {
    //  m_IDDict.insert(clmake_pair(sModule.id, attr));
    //}
    //m_aModules.push_back(sModule);
    Add(m_aModules, GXSprite::Type_Module, sModule);
  }


  //
  // Frame 列表
  //
  GXSprite::FRAME sFrame;
  for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
  {
    sFrame = pDesc->aFrames[i];
    //if(sFrame.name) {
    //  sFrame.name = m_NameSet.add(sFrame.name);
    //  m_SpriteDict[sFrame.name] = (int)(m_aModules.size() + i);
    //}
    //m_aFrames.push_back(sFrame);
    Add(m_aFrames, GXSprite::Type_Frame, sFrame);
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
    //if(sAnimation.name) {
    //  sAnimation.name = m_NameSet.add(sAnimation.name);
    //  m_SpriteDict[sAnimation.name] = (int)(m_aModules.size() + m_aFrames.size() + i);
    //}
    //m_aAnimations.push_back(sAnimation);
    Add(m_aAnimations, GXSprite::Type_Animation, sAnimation);
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

GXBOOL GXSpriteImpl::GetModule( GXUINT nIndex, MODULE* pModule ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aModules.size() && pModule != NULL) {
    *pModule = m_aModules[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXBOOL GXSpriteImpl::GetFrame( GXUINT nIndex, FRAME* pFrame ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aFrames.size() && pFrame != NULL) {
    *pFrame = m_aFrames[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXUINT GXSpriteImpl::GetFrameModule( GXUINT nIndex, FRAME_MODULE* pFrameModule, GXSIZE_T nCount ) const
{
  if(nIndex < 0 || nIndex >= (GXINT)m_aFrames.size()) {
    return 0;
  }

  if(pFrameModule == NULL || nCount <= 0) {
    return m_aFrames[nIndex].count;
  }

  const GXUINT nNumOfCopy = clMin((GXUINT)nCount, m_aFrames[nIndex].count);
  memcpy(pFrameModule, &m_aFrameModules[m_aFrames[nIndex].start], sizeof(FRAME_MODULE) * nNumOfCopy);
  return nNumOfCopy;
}

GXBOOL GXSpriteImpl::GetAnimation( GXUINT nIndex, ANIMATION* pAnimation ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aAnimations.size() && pAnimation != NULL) {
    *pAnimation = m_aAnimations[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXUINT GXSpriteImpl::GetAnimFrame( GXUINT nIndex, ANIM_FRAME* pAnimFrame, GXSIZE_T nCount ) const
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

