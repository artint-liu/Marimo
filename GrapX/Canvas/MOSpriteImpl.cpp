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
namespace Marimo
{
  MOSpriteImpl::MOSpriteImpl()
  {
  }

  MOSpriteImpl::~MOSpriteImpl()
  {
    std::for_each(m_ImageArray.begin(), m_ImageArray.end(), [](GXImage*& pImage) {
      SAFE_RELEASE(pImage);
    });
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MOSpriteImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT MOSpriteImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if (nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //int MOSpriteImpl::IntGetSpriteCount() const
  //{
  //  return (int)m_loader.aModules.size();
  //}

  //////////////////////////////////////////////////////////////////////////


  GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const
  {
    //const GXREGN& regn = ;
    if (nIndex < (GXINT)m_loader.aModules.size()) {
      const MODULE& m = m_loader.aModules[nIndex];
      pCanvas->DrawImage(m_ImageArray[GXHIWORD(m.id)], x, y, &m.regn);
    }
  }

  GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
  {
    REGN rcDest;
    rcDest.left = x;
    rcDest.top = y;
    rcDest.width = nWidth;
    rcDest.height = nHeight;
    if (nIndex < (GXINT)m_loader.aModules.size()) {
      const MODULE& m = m_loader.aModules[nIndex];
      pCanvas->DrawImage(m_ImageArray[GXHIWORD(m.id)], &rcDest, &m.regn);
    }
  }

  GXVOID MOSpriteImpl::PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  //GXVOID MOSpriteImpl::PaintModuleH(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth) const
  //{
  //  REGN rcDest;
  //  rcDest.left = x;
  //  rcDest.top = y;
  //  rcDest.width = nWidth;
  //  rcDest.height = m_loader.aModules[nIndex].regn.height;
  //
  //  if(nIndex < (GXINT)m_loader.aModules.size()) {
  //    pCanvas->DrawImage(m_pImage, &rcDest, m_loader.aModules[nIndex].regn);
  //  }
  //}
  //
  //GXVOID MOSpriteImpl::PaintModuleV(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nHeight) const
  //{
  //  REGN rcDest;
  //  rcDest.left = x;
  //  rcDest.top = y;
  //  rcDest.width = m_loader.aModules[nIndex].regn.width;
  //  rcDest.height = nHeight;
  //
  //  if(nIndex < (GXINT)m_loader.aModules.size()) {
  //    pCanvas->DrawImage(m_pImage, &rcDest, m_loader.aModules[nIndex].regn);
  //  }
  //}

  //GXLONG MOSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
  //{
  //  if(nStartIdx + 3 > IntGetSpriteCount())
  //  {
  //    ASSERT(FALSE);
  //    return -1;
  //  }
  //  GXLONG nMaxHeight = clMax(m_loader.aModules[nStartIdx].regn.height, clMax(m_loader.aModules[nStartIdx + 1].regn.height, m_loader.aModules[nStartIdx + 2].regn.height));
  //  float fVertScale = (float)nHeight / (float)nMaxHeight;
  //  float fHorzScale = (float)nWidth / (float)(m_loader.aModules[nStartIdx].regn.width + m_loader.aModules[nStartIdx + 2].regn.width);
  //  REGN rgTip0 = m_loader.aModules[nStartIdx].regn;
  //  REGN rgTip1 = m_loader.aModules[nStartIdx + 2].regn;
  //  REGN rgDest;
  //
  //  if(fHorzScale > fVertScale)
  //  {
  //    //REGN rgTip0 = m_pRect[nStartIdx];
  //    //REGN rgDest;
  //    //REGN rgTip1 = m_pRect[nStartIdx + 2];
  //    float fScale0 = (float)nHeight / (float)m_loader.aModules[nStartIdx    ].regn.height;
  //    float fScale1 = (float)nHeight / (float)m_loader.aModules[nStartIdx + 1].regn.height;
  //    //float fScale2 = (float)nHeight / (float)m_loader.aModules[nStartIdx + 2].regn.height;
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
  //    pCanvas->DrawImage(m_pImage, &rgDest, &m_loader.aModules[nStartIdx + 1].regn);
  //  }
  //  else
  //  {
  //    //REGN rgTip0 = m_pRect[nStartIdx];
  //    //REGN rgTip1 = m_pRect[nStartIdx + 2];
  //    //ASSERT(FALSE);
  //    rgTip0.left   = x;
  //    rgTip0.top    = y;
  //    rgTip0.width  = (GXLONG)((float)m_loader.aModules[nStartIdx].regn.width * fHorzScale + 0.5f);
  //    rgTip0.height = nHeight;
  //    rgTip1.left   = x + rgTip0.width;
  //    rgTip1.top    = y;
  //    rgTip1.width  = nWidth - rgTip0.width;
  //    rgTip1.height = nHeight;
  //
  //    rgDest.width = 0;
  //  }
  //  pCanvas->DrawImage(m_pImage, &rgTip0, &m_loader.aModules[nStartIdx    ].regn);
  //  pCanvas->DrawImage(m_pImage, &rgTip1, &m_loader.aModules[nStartIdx + 2].regn);
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
  //  GXLONG nMaxWidth = clMax(m_loader.aModules[nStartIdx].regn.width, clMax(m_loader.aModules[nStartIdx + 1].regn.width, m_loader.aModules[nStartIdx + 2].regn.width));
  //  float fHorzScale = (float)nWidth / (float)nMaxWidth;
  //  float fVertScale = (float)nHeight / (float)(m_loader.aModules[nStartIdx].regn.height + m_loader.aModules[nStartIdx + 2].regn.height);
  //  REGN rgTip0 = m_loader.aModules[nStartIdx].regn;
  //  REGN rgTip1 = m_loader.aModules[nStartIdx + 2].regn;
  //  REGN rgDest;
  //
  //  if(fVertScale > fHorzScale)
  //  {
  //    //REGN rgTip0 = m_pRect[nStartIdx];
  //    //REGN rgDest;
  //    //REGN rgTip1 = m_pRect[nStartIdx + 2];
  //    float fScale0 = (float)nWidth / (float)m_loader.aModules[nStartIdx    ].regn.width;
  //    float fScale1 = (float)nWidth / (float)m_loader.aModules[nStartIdx + 1].regn.width;
  //    //float fScale2 = (float)nWidth / (float)m_loader.aModules[nStartIdx + 2].regn.width;
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
  //    pCanvas->DrawImage(m_pImage, &rgDest, &m_loader.aModules[nStartIdx + 1].regn);
  //  }
  //  else
  //  {
  //    //REGN rgTip0 = m_pRect[nStartIdx];
  //    //REGN rgTip1 = m_pRect[nStartIdx + 2];
  //    //ASSERT(FALSE);
  //    rgTip0.top     = y;
  //    rgTip0.left    = x;
  //    rgTip0.height  = (GXLONG)((float)m_loader.aModules[nStartIdx].regn.height * fHorzScale + 0.5f);
  //    rgTip0.width = nWidth;
  //    rgTip1.top   = y + rgTip0.height;
  //    rgTip1.left    = x;
  //    rgTip1.height  = nHeight - rgTip0.height;
  //    rgTip1.width = nWidth;
  //
  //    rgDest.height = 0;
  //  }
  //  pCanvas->DrawImage(m_pImage, &rgTip0, &m_loader.aModules[nStartIdx    ].regn);
  //  pCanvas->DrawImage(m_pImage, &rgTip1, &m_loader.aModules[nStartIdx + 2].regn);
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
  //  /*左上角*/pCanvas->DrawImage(m_pImage, rect->left, rect->top, &m_loader.aModules[nStartIdx].regn);
  //  /*右上角*/pCanvas->DrawImage(m_pImage, rect->right - m_loader.aModules[nStartIdx + 2].regn.width, rect->top, &m_loader.aModules[nStartIdx + 2].regn);
  //  /*左下角*/pCanvas->DrawImage(m_pImage, rect->left, rect->bottom - m_loader.aModules[nStartIdx + 6].regn.height, &m_loader.aModules[nStartIdx + 6].regn);
  //  /*右下角*/pCanvas->DrawImage(m_pImage, rect->right - m_loader.aModules[nStartIdx + 8].regn.width, rect->bottom - m_loader.aModules[nStartIdx + 8].regn.height, &m_loader.aModules[nStartIdx + 8].regn);
  //
  ////*
  //  rcDest.left   = rect->left + m_loader.aModules[nStartIdx].regn.width;
  //  rcDest.top    = top;
  //  rcDest.width  = width - m_loader.aModules[nStartIdx].regn.width - m_loader.aModules[nStartIdx + 2].regn.width;
  //  rcDest.height = m_loader.aModules[nStartIdx + 1].regn.height;
  //  pCanvas->DrawImage(m_pImage, &rcDest, &m_loader.aModules[nStartIdx + 1].regn);
  //
  //  rcDest.left   = rect->left + m_loader.aModules[nStartIdx + 6].regn.width;
  //  rcDest.top    = rect->top + height - m_loader.aModules[nStartIdx + 7].regn.height;
  //  rcDest.width  = width  - m_loader.aModules[nStartIdx + 6].regn.width - m_loader.aModules[nStartIdx + 8].regn.width;
  //  rcDest.height = m_loader.aModules[nStartIdx + 7].regn.height;
  //  pCanvas->DrawImage(m_pImage, &rcDest, &m_loader.aModules[nStartIdx + 7].regn);
  //
  //  // 左边
  //  rcDest.left   = rect->left;
  //  rcDest.top    = rect->top + m_loader.aModules[nStartIdx   ].regn.height;
  //  rcDest.width  = m_loader.aModules[nStartIdx + 3].regn.width;
  //  rcDest.height = height - m_loader.aModules[nStartIdx].regn.height - m_loader.aModules[nStartIdx + 6].regn.height;
  //  pCanvas->DrawImage(m_pImage, &rcDest, &m_loader.aModules[nStartIdx + 3].regn);
  //
  //  // 右边
  //  rcDest.left   = rect->right - m_loader.aModules[nStartIdx + 5].regn.width;
  //  rcDest.top    = rect->top + m_loader.aModules[nStartIdx + 2].regn.height;
  //  rcDest.width  = m_loader.aModules[nStartIdx + 5].regn.width;
  //  rcDest.height = height - m_loader.aModules[nStartIdx + 2].regn.height - m_loader.aModules[nStartIdx + 8].regn.height;
  //  pCanvas->DrawImage(m_pImage, &rcDest, &m_loader.aModules[nStartIdx + 5].regn);
  ////*/
  //  if(bDrawEdge == FALSE)
  //  {
  //    rcDest.left   = m_loader.aModules[nStartIdx + 3].regn.width;
  //    rcDest.top    = m_loader.aModules[nStartIdx].regn.height;
  //    rcDest.width  = width  - m_loader.aModules[nStartIdx + 3].regn.width  - m_loader.aModules[nStartIdx + 5].regn.width;
  //    rcDest.height = height - m_loader.aModules[nStartIdx].regn.height - m_loader.aModules[nStartIdx + 8].regn.height;
  //    pCanvas->DrawImage(m_pImage, &rcDest, &m_loader.aModules[nStartIdx + 4].regn);
  //  }
  //}
  //
  //GXSIZE_T MOSpriteImpl::GetModuleCount() const
  //{
  //  return m_loader.aModules.size();
  //}
  //
  //












  GXBOOL MOSpriteImpl::GetModule(GXUINT nIndex, MODULE* pModule) const
  {
    if (nIndex < (GXINT)m_loader.aModules.size() && pModule != NULL) {
      *pModule = m_loader.aModules[nIndex];
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL MOSpriteImpl::GetFrame(GXUINT nIndex, FRAME* pFrame) const
  {
    if (nIndex < (GXINT)m_loader.aFrames.size() && pFrame != NULL) {
      *pFrame = m_loader.aFrames[nIndex];
      return TRUE;
    }
    return FALSE;
  }

  GXVOID MOSpriteImpl::PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::PaintModule3x3(GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawCenter, GXLPCRECT rect) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const
  {
    CLBREAK;
  }

  GXVOID MOSpriteImpl::Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const
  {
    CLBREAK;
  }

  GXINT MOSpriteImpl::Find(ID id) const
  {
    CLBREAK;
    return -1;
  }

  GXINT MOSpriteImpl::Find(GXLPCSTR szName) const
  {
    CLBREAK;
    return -1;
  }

  GXSIZE_T MOSpriteImpl::GetModuleCount() const
  {
    return m_loader.aModules.size();
  }

  GXSIZE_T MOSpriteImpl::GetFrameCount() const
  {
    return m_loader.aFrames.size();
  }

  GXSIZE_T MOSpriteImpl::GetAnimationCount() const
  {
    return m_loader.aAnims.size();
  }

  GXSIZE_T MOSpriteImpl::GetAnimFrameCount(GXUINT nIndex) const
  {
    if(nIndex >= (GXUINT)m_loader.aAnims.size()) {
      return 0;
    }
    const ANIMATION& a = m_loader.aAnims[nIndex];
    return a.end - a.begin;
  }

  GXSIZE_T MOSpriteImpl::GetFrameModule(GXUINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const
  {
    if(nIndex >= (GXUINT)m_loader.aFrames.size()) {
      return 0;
    }

    const FRAME& f = m_loader.aFrames[nIndex];
    nCount = clMin(nCount, f.end - f.begin);

    for(GXSIZE_T i = 0; i < nCount; i++)
    {
      FRAME_UNIT& dst = pFrameModule[i];
      const FRAME_UNIT& src = m_loader.aFrameUnits[f.begin + i];
      dst.nModuleIdx = src.nModuleIdx;
      dst.regn       = src.regn;
      dst.rotate     = src.rotate;
    }
    return nCount;
    //return m_loader.aFrameUnits.size();
  }

  GXBOOL MOSpriteImpl::GetAnimation(GXUINT nIndex, ANIMATION* pAnimation) const
  {
    if(nIndex >= (GXUINT)m_loader.aAnims.size()) {
      return FALSE;
    }
    const ANIMATION& a = m_loader.aAnims[nIndex];
    pAnimation->id    = a.id;
    pAnimation->name  = a.name;
    pAnimation->rate  = a.rate;
    pAnimation->begin = a.begin;
    pAnimation->end   = a.end;
    return TRUE;
  }

  GXSIZE_T MOSpriteImpl::GetAnimFrame(GXUINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const
  {
    if(nIndex >= m_loader.aAnims.size()) {
      return 0;
    }
    
    const ANIMATION& a = m_loader.aAnims[nIndex];
    if(pAnimFrame == NULL || nCount == 0) {
      return a.end - a.begin;
    }

    nCount = clMin(nCount, a.end - a.begin);
    for(GXSIZE_T i = 0; i < nCount; i++) {
      pAnimFrame[i] = m_loader.aAnimUnits[a.begin + i];
    }
    return nCount;
  }

  GXBOOL MOSpriteImpl::GetModuleRect(GXUINT nIndex, GXLPRECT rcSprite) const
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL MOSpriteImpl::GetModuleRegion(GXUINT nIndex, GXLPREGN rgSprite) const
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL MOSpriteImpl::GetFrameBounding(GXUINT nIndex, GXLPRECT lprc) const
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL MOSpriteImpl::GetAnimBounding(GXUINT nIndex, GXLPRECT lprc) const
  {
    CLBREAK;
    return FALSE;
  }

  MOSprite::Type MOSpriteImpl::GetBounding(ID id, GXLPRECT lprc) const
  {
    CLBREAK;
    return Type_Error;
  }

  MOSprite::Type MOSpriteImpl::GetBounding(ID id, GXLPREGN lprg) const
  {
    CLBREAK;
    return Type_Error;
  }

  GXSIZE_T MOSpriteImpl::GetImageCount() const
  {    
    return m_ImageArray.size();
  }

  GXBOOL MOSpriteImpl::GetImage(GXImage** pImage, GXUINT index) const
  {
    if (index >= m_ImageArray.size()) {
      return FALSE;
    }
    *pImage = m_ImageArray[index];
    SAFE_ADDREF(*pImage);
    return TRUE;
  }

  clStringW MOSpriteImpl::GetImageFileW(GXUINT index) const
  {
    if(index >= m_loader.aFiles.size()) {
      return L"";
    }
    return clStringW(m_loader.aFiles[index]);
  }

  clStringA MOSpriteImpl::GetImageFileA(GXUINT index) const
  {
    if(index >= m_loader.aFiles.size()) {
      return L"";
    }
    return m_loader.aFiles[index];
  }

  GXBOOL MOSpriteImpl::Initialize(GXGraphics* pGraphics, const SPRITE_DESC_LOADER* pDesc)
  {
    m_loader = *pDesc;
    m_ImageArray.reserve(pDesc->aFiles.size());
    int i = 0;

    std::for_each(pDesc->aFiles.begin(), pDesc->aFiles.end(), [&](const clStringA& str)
    {
      GXImage* pImage = NULL;

      if(clpathfile::IsRelativeA(str)) {
        clStringA strFullPath;
        clpathfile::CombinePathA(strFullPath, pDesc->strImageDir, str);
        m_loader.aFiles[i] = strFullPath;
        pImage = pGraphics->CreateImageFromFile(clStringW(strFullPath));
      }
      else {
        pImage = pGraphics->CreateImageFromFile(clStringW(str));
      }
      m_ImageArray.push_back(pImage);
      i++;
    });

    ASSERT(m_ImageArray.size() == pDesc->aFiles.size());
    return TRUE;
  }
} // namespace Marimo 

