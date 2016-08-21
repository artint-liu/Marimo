// 全局头文件
#include <GrapX.H>

// 标准接口
#include "GrapX/GResource.H"
#include "GrapX/GXImage.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/MOSprite.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXKernel.H"

// 私有头文件
#include <clPathFile.h>
#include <clUtility.H>
#include <clStringSet.h>
#include <Smart/SmartStream.h>
#include <Smart/Stock.h>
#include <Canvas/MOSpriteImpl.H>

//using namespace std;
//////////////////////////////////////////////////////////////////////////
extern "C" GXBOOL GXDLLAPI gxSetRegn(GXLPREGN lprg, GXINT xLeft, GXINT yTop, GXINT xWidth, GXINT yHeight);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
MOSpriteImpl::MOSpriteImpl()
  : m_pImage(NULL)
{
}

MOSpriteImpl::~MOSpriteImpl()
{
  SAFE_RELEASE(m_pImage);
  m_aModules.clear();
  m_SpriteDict.clear();
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT MOSpriteImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT MOSpriteImpl::Release()
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

//int MOSpriteImpl::IntGetSpriteCount() const
//{
//  return (int)m_aModules.size();
//}

//////////////////////////////////////////////////////////////////////////


GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const
{
  //const GXREGN& regn = ;
  if(nIndex < (GXINT)m_aModules.size()) {
    pCanvas->DrawImage(m_pImage, x, y, &m_aModules[nIndex].regn);
  }
}

GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
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

GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const 
{

}

//GXVOID MOSpriteImpl::PaintModuleH(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth) const
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
//GXVOID MOSpriteImpl::PaintModuleV(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nHeight) const
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

//GXLONG MOSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
//{
//  if(nStartIdx + 3 > IntGetSpriteCount())
//  {
//    ASSERT(FALSE);
//    return -1;
//  }
//  GXLONG nMaxHeight = clMax(m_aModules[nStartIdx].regn.height, clMax(m_aModules[nStartIdx + 1].regn.height, m_aModules[nStartIdx + 2].regn.height));
//  float fVertScale = (float)nHeight / (float)nMaxHeight;
//  float fHorzScale = (float)nWidth / (float)(m_aModules[nStartIdx].regn.width + m_aModules[nStartIdx + 2].regn.width);
//  REGN rgTip0 = m_aModules[nStartIdx].regn;
//  REGN rgTip1 = m_aModules[nStartIdx + 2].regn;
//  REGN rgDest;
//
//  if(fHorzScale > fVertScale)
//  {
//    //REGN rgTip0 = m_pRect[nStartIdx];
//    //REGN rgDest;
//    //REGN rgTip1 = m_pRect[nStartIdx + 2];
//    float fScale0 = (float)nHeight / (float)m_aModules[nStartIdx    ].regn.height;
//    float fScale1 = (float)nHeight / (float)m_aModules[nStartIdx + 1].regn.height;
//    //float fScale2 = (float)nHeight / (float)m_aModules[nStartIdx + 2].regn.height;
//
//    rgTip0.width  = (GXLONG)(rgTip0.width  * fScale0 + 0.5f);
//    rgTip0.height = nHeight;
//    rgTip1.width  = (GXLONG)(rgTip1.width  * fScale1 + 0.5f);
//    rgTip1.height = nHeight;
//
//    rgTip0.left = x;
//    rgTip0.top  = y;
//    rgTip1.left = x + nWidth - rgTip1.width;
//    rgTip1.top  = y;
//
//    rgDest.left   = x + rgTip0.width;
//    rgDest.top    = y;
//    rgDest.width  = nWidth - rgTip0.width - rgTip1.width;
//    rgDest.height = nHeight;
//
//    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx    ], &rgTip0);
//    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx + 2], &rgTip1);
//    pCanvas->DrawImage(m_pImage, &rgDest, &m_aModules[nStartIdx + 1].regn);
//  }
//  else
//  {
//    //REGN rgTip0 = m_pRect[nStartIdx];
//    //REGN rgTip1 = m_pRect[nStartIdx + 2];
//    //ASSERT(FALSE);
//    rgTip0.left   = x;
//    rgTip0.top    = y;
//    rgTip0.width  = (GXLONG)((float)m_aModules[nStartIdx].regn.width * fHorzScale + 0.5f);
//    rgTip0.height = nHeight;
//    rgTip1.left   = x + rgTip0.width;
//    rgTip1.top    = y;
//    rgTip1.width  = nWidth - rgTip0.width;
//    rgTip1.height = nHeight;
//
//    rgDest.width = 0;
//  }
//  pCanvas->DrawImage(m_pImage, &rgTip0, &m_aModules[nStartIdx    ].regn);
//  pCanvas->DrawImage(m_pImage, &rgTip1, &m_aModules[nStartIdx + 2].regn);
//  return rgDest.width;  // 返回值是中间可缩放Sprite的宽度
//}
//
//GXLONG MOSpriteImpl::PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
//{
//  if(nStartIdx + 3 > IntGetSpriteCount())
//  {
//    ASSERT(FALSE);
//    return -1;
//  }
//  GXLONG nMaxWidth = clMax(m_aModules[nStartIdx].regn.width, clMax(m_aModules[nStartIdx + 1].regn.width, m_aModules[nStartIdx + 2].regn.width));
//  float fHorzScale = (float)nWidth / (float)nMaxWidth;
//  float fVertScale = (float)nHeight / (float)(m_aModules[nStartIdx].regn.height + m_aModules[nStartIdx + 2].regn.height);
//  REGN rgTip0 = m_aModules[nStartIdx].regn;
//  REGN rgTip1 = m_aModules[nStartIdx + 2].regn;
//  REGN rgDest;
//
//  if(fVertScale > fHorzScale)
//  {
//    //REGN rgTip0 = m_pRect[nStartIdx];
//    //REGN rgDest;
//    //REGN rgTip1 = m_pRect[nStartIdx + 2];
//    float fScale0 = (float)nWidth / (float)m_aModules[nStartIdx    ].regn.width;
//    float fScale1 = (float)nWidth / (float)m_aModules[nStartIdx + 1].regn.width;
//    //float fScale2 = (float)nWidth / (float)m_aModules[nStartIdx + 2].regn.width;
//
//    rgTip0.height  = (GXLONG)(rgTip0.height  * fScale0 + 0.5f);
//    rgTip0.width = nWidth;
//    rgTip1.height  = (GXLONG)(rgTip1.height  * fScale1 + 0.5f);
//    rgTip1.width = nWidth;
//
//    rgTip0.left = x;
//    rgTip0.top  = y;
//    rgTip1.top = y + nHeight - rgTip1.height;
//    rgTip1.left  = x;
//
//    rgDest.top   = y + rgTip0.height;
//    rgDest.left    = x;
//    rgDest.height  = nHeight - rgTip0.height - rgTip1.height;
//    rgDest.width = nWidth;
//
//    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx    ], &rgTip0);
//    //pCanvas->DrawImage(m_pImage, &m_pRect[nStartIdx + 2], &rgTip1);
//    pCanvas->DrawImage(m_pImage, &rgDest, &m_aModules[nStartIdx + 1].regn);
//  }
//  else
//  {
//    //REGN rgTip0 = m_pRect[nStartIdx];
//    //REGN rgTip1 = m_pRect[nStartIdx + 2];
//    //ASSERT(FALSE);
//    rgTip0.top     = y;
//    rgTip0.left    = x;
//    rgTip0.height  = (GXLONG)((float)m_aModules[nStartIdx].regn.height * fHorzScale + 0.5f);
//    rgTip0.width = nWidth;
//    rgTip1.top   = y + rgTip0.height;
//    rgTip1.left    = x;
//    rgTip1.height  = nHeight - rgTip0.height;
//    rgTip1.width = nWidth;
//
//    rgDest.height = 0;
//  }
//  pCanvas->DrawImage(m_pImage, &rgTip0, &m_aModules[nStartIdx    ].regn);
//  pCanvas->DrawImage(m_pImage, &rgTip1, &m_aModules[nStartIdx + 2].regn);
//  return rgDest.height;  // 返回值是中间可缩放Sprite的宽度
//}
//
//GXVOID MOSpriteImpl::PaintModule3x3(GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const
//{
//  // 0 1 2
//  // 3 4 5
//  // 6 7 8
//  REGN rcDest;
//  //rcDest.left = x;
//  //rcDest.top = y;
//  //rcDest.width = nWidth;
//  //rcDest.height = nHeight;
//  const GXINT width = rect->right - rect->left;
//  const GXINT height = rect->bottom - rect->top;
//  const GXINT top = rect->top;
//  //const GXINT bottom = rect->bottom;
//
//  /*左上角*/pCanvas->DrawImage(m_pImage, rect->left, rect->top, &m_aModules[nStartIdx].regn);
//  /*右上角*/pCanvas->DrawImage(m_pImage, rect->right - m_aModules[nStartIdx + 2].regn.width, rect->top, &m_aModules[nStartIdx + 2].regn);
//  /*左下角*/pCanvas->DrawImage(m_pImage, rect->left, rect->bottom - m_aModules[nStartIdx + 6].regn.height, &m_aModules[nStartIdx + 6].regn);
//  /*右下角*/pCanvas->DrawImage(m_pImage, rect->right - m_aModules[nStartIdx + 8].regn.width, rect->bottom - m_aModules[nStartIdx + 8].regn.height, &m_aModules[nStartIdx + 8].regn);
//
////*
//  rcDest.left   = rect->left + m_aModules[nStartIdx].regn.width;
//  rcDest.top    = top;
//  rcDest.width  = width - m_aModules[nStartIdx].regn.width - m_aModules[nStartIdx + 2].regn.width;
//  rcDest.height = m_aModules[nStartIdx + 1].regn.height;
//  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 1].regn);
//
//  rcDest.left   = rect->left + m_aModules[nStartIdx + 6].regn.width;
//  rcDest.top    = rect->top + height - m_aModules[nStartIdx + 7].regn.height;
//  rcDest.width  = width  - m_aModules[nStartIdx + 6].regn.width - m_aModules[nStartIdx + 8].regn.width;
//  rcDest.height = m_aModules[nStartIdx + 7].regn.height;
//  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 7].regn);
//
//  // 左边
//  rcDest.left   = rect->left;
//  rcDest.top    = rect->top + m_aModules[nStartIdx   ].regn.height;
//  rcDest.width  = m_aModules[nStartIdx + 3].regn.width;
//  rcDest.height = height - m_aModules[nStartIdx].regn.height - m_aModules[nStartIdx + 6].regn.height;
//  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 3].regn);
//
//  // 右边
//  rcDest.left   = rect->right - m_aModules[nStartIdx + 5].regn.width;
//  rcDest.top    = rect->top + m_aModules[nStartIdx + 2].regn.height;
//  rcDest.width  = m_aModules[nStartIdx + 5].regn.width;
//  rcDest.height = height - m_aModules[nStartIdx + 2].regn.height - m_aModules[nStartIdx + 8].regn.height;
//  pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 5].regn);
////*/
//  if(bDrawEdge == FALSE)
//  {
//    rcDest.left   = m_aModules[nStartIdx + 3].regn.width;
//    rcDest.top    = m_aModules[nStartIdx].regn.height;
//    rcDest.width  = width  - m_aModules[nStartIdx + 3].regn.width  - m_aModules[nStartIdx + 5].regn.width;
//    rcDest.height = height - m_aModules[nStartIdx].regn.height - m_aModules[nStartIdx + 8].regn.height;
//    pCanvas->DrawImage(m_pImage, &rcDest, &m_aModules[nStartIdx + 4].regn);
//  }
//}
//
//GXSIZE_T MOSpriteImpl::GetModuleCount() const
//{
//  return m_aModules.size();
//}
//
//












GXBOOL MOSpriteImpl::GetModule( GXINT nIndex, MODULE* pModule ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aModules.size() && pModule != NULL) {
    *pModule = m_aModules[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXBOOL MOSpriteImpl::GetFrame( GXINT nIndex, FRAME* pFrame ) const
{
  if(nIndex >= 0 && nIndex < (GXINT)m_aFrames.size() && pFrame != NULL) {
    *pFrame = m_aFrames[nIndex];
    return TRUE;
  }
  return FALSE;
}

GXVOID MOSpriteImpl::PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const 
{

}

GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const 
{

}

GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const 
{

}

GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const 
{

}

GXVOID MOSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const 
{

}

GXVOID MOSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const 
{

}

GXVOID MOSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const 
{

}

GXVOID MOSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const 
{

}

GXVOID MOSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const 
{

}

GXVOID MOSpriteImpl::PaintModule3x3(GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawCenter, GXLPCRECT rect) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const 
{

}

GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const 
{

}

GXINT MOSpriteImpl::Find(ID id) const 
{
  return -1;
}

GXINT MOSpriteImpl::Find(GXLPCSTR szName) const 
{
  return -1;
}

GXSIZE_T MOSpriteImpl::GetModuleCount() const 
{
  return 0;
}

GXSIZE_T MOSpriteImpl::GetFrameCount() const 
{
  return 0;
}

GXSIZE_T MOSpriteImpl::GetAnimationCount() const 
{
  return 0;
}

GXSIZE_T MOSpriteImpl::GetAnimFrameCount(GXINT nIndex) const 
{
  return 0;
}

GXSIZE_T MOSpriteImpl::GetFrameModule(GXINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const 
{
  return 0;
}

GXBOOL MOSpriteImpl::GetAnimation(GXINT nIndex, ANIMATION* pAnimation) const 
{
  return FALSE;
}

GXSIZE_T MOSpriteImpl::GetAnimFrame(GXINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const 
{
  return 0;
}

GXBOOL MOSpriteImpl::GetModuleRect(GXINT nIndex, GXLPRECT rcSprite) const 
{
  return FALSE;
}

GXBOOL MOSpriteImpl::GetModuleRegion(GXINT nIndex, GXLPREGN rgSprite) const 
{
  return FALSE;
}

GXBOOL MOSpriteImpl::GetFrameBounding(GXINT nIndex, GXLPRECT lprc) const 
{
  return FALSE;
}

GXBOOL MOSpriteImpl::GetAnimBounding(GXINT nIndex, GXLPRECT lprc) const 
{
  return FALSE;
}

MOSprite::Type MOSpriteImpl::GetBounding(ID id, GXLPRECT lprc) const 
{
  return Type_Error;
}

MOSprite::Type MOSpriteImpl::GetBounding(ID id, GXLPREGN lprg) const 
{
  return Type_Error;
}

GXSIZE_T MOSpriteImpl::GetImageCount() const 
{
  return 0;
}

GXBOOL MOSpriteImpl::GetImage(GXImage** pImage, GXINT index) const 
{
  return GX_FAIL;
}

clStringW MOSpriteImpl::GetImageFileW(GXINT index) const 
{
  return L"";
}

clStringA MOSpriteImpl::GetImageFileA(GXINT index) const 
{
  return "";
}

