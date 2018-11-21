#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GXCanvas.h>
#include <GrapX/GTexture.h>
#include <GrapX/GXRenderTarget.h>
#include <GrapX/GXFont.h>
#include <GrapX/GRegion.h>

// 私有头文件
#include "GXStation.h"
#include <User/GXWindow.h>
#include "GrapX/GXGDI.h"
#include "GrapX/GXUser.h"



GXBOOL GXDLLAPI gxIntersectRect(GXLPRECT,const GXRECT *,const GXRECT *);
GXBOOL GXDLLAPI gxUnionRect(GXLPRECT,const GXRECT *,const GXRECT *);
GXBOOL GXDLLAPI gxOffsetRect    (GXLPRECT lprc,GXINT dx,GXINT dy);
GXHANDLE  GXDLLAPI gxCopyImage    (GXHANDLE hImage,GXUINT uType,int cxDesired,int cyDesired,GXUINT fuFlags);

extern GXHFONT g_hSystemFont;
int GXDLLAPI gxGetObject(
        GXHGDIOBJ hgdiobj,  // handle to graphics object of interest
        GXINT cbBuffer,    // size of buffer for object information 
        GXLPVOID lpvObject   // pointer to buffer for object information  
        )
{
  if(hgdiobj == NULL || lpvObject == NULL || cbBuffer == 0) {
    return 0;
  }

  clsize nNumCopy = 0;
  switch (GXGDIOBJ_TYPE(hgdiobj))
  {
  case GXGDIOBJ_PEN:
    CLBREAK;
  case GXGDIOBJ_BRUSH:
    {
      nNumCopy = clMin(sizeof(GXLOGBRUSH), (size_t)cbBuffer);
      GXLOGBRUSH sLogBrush;
      GXGDIBRUSH* pBrush = GXGDI_BRUSH_PTR(hgdiobj);
      sLogBrush.lbColor = pBrush->crColor;
      sLogBrush.lbStyle = pBrush->uStyle;
      sLogBrush.lbHatch = GXHS_API_MAX;
      memcpy(lpvObject, &sLogBrush, nNumCopy);
    }
    break;

  case GXGDIOBJ_DC:
  case GXGDIOBJ_FONT:
  case GXGDIOBJ_BITMAP:
  case GXGDIOBJ_REGION:
  case GXGDIOBJ_MEMDC:
    CLBREAK;
  default:
    return 0;
  }
  return (int)nNumCopy;
}



GXDWORD GXDLLAPI gxGetObjectType(
            GXHGDIOBJ h   // handle of graphic object  
            )
{
  if(h == NULL) {
    return 0;
  }
  return GXGDIOBJ_TYPE(h);
}


// 初始化默认的 GXDC 结构
GXVOID _gxInitDefGXDC(GXHDC hDC)
{
}

//static GXDC hBeginPaintDC;
//static GXRECT rcBeginPaintSaved;
//////////////////////////////////////////////////////////////////////////
//static GXGDIBRUSH brushDef = {GXGDIOBJ_BRUSH, NULL, GXBS_SOLID, 0xFFFFFFFF, NULL};//TODO
//GXGDIFONT DefaultFont = {GXGDIOBJ_FONT, NULL};
GXHDC GXDLLAPI gxBeginPaint(
           GXHWND hwnd,        // handle to window
           GXLPPAINTSTRUCT lpPaint  // pointer to structure for paint information  
           )
{
  GXLPWND lpWnd = GXWND_PTR(hwnd);
  CHECK_LPWND_VAILD(lpWnd);

  GRegion* prgnUpdate = lpWnd->m_prgnUpdate;

  // 重复调用 gxBeginPaint 就返回
  if(prgnUpdate == NULL) {
    memset(lpPaint, 0, sizeof(GXPAINTSTRUCT));
    return NULL;
  }

  GXGDIREGION GDIRegion(prgnUpdate);
  GXHDC hdc = gxGetDCEx(hwnd, GXGDI_RGN_HANDLE(&GDIRegion), GXDCX_PARENTCLIP | GXDCX_CLIPSIBLINGS);

  lpPaint->hdc  = hdc;
  lpPaint->fErase = FALSE;
  gxGetClientRect(hwnd, &lpPaint->rcPaint);

  lpPaint->fRestore    = 0;
  lpPaint->fIncUpdate = 0;
  memset(lpPaint->rgbReserved, 0, sizeof(lpPaint->rgbReserved));

  if(gxSendMessageW(hwnd, GXWM_ERASEBKGND, (GXWPARAM)lpPaint->hdc, 0) == FALSE)
    gxDefWindowProcW(hwnd, GXWM_ERASEBKGND, (GXWPARAM)lpPaint->hdc, 0);

  return hdc;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxEndPaint(
        GXHWND hWnd,        // handle to window
        const GXPAINTSTRUCT *lpPaint   // pointer to structure for paint data  
        )
{
  if(lpPaint->hdc == NULL) {
    return FALSE;
  }
  gxValidateRect(hWnd, NULL);

  LPGXGDIDC pDC = (LPGXGDIDC)lpPaint->hdc;
  
  SAFE_DELETE(pDC->pWndCanvas);
  SAFE_DELETE(pDC);

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////

GXHGDIOBJ GXDLLAPI gxSelectObject(
             GXHDC hdc,      // handle of device context 
             GXHGDIOBJ hgdiobj   // handle of object  
             )
{
  //ASSERT(FALSE);
  GXHGDIOBJ  hPrevObj = NULL;
  //GXGDIECT *pObjType = (GXGDIECT*)hgdiobj;
  if(hgdiobj == NULL || hgdiobj == (GXHGDIOBJ)-1)  // TODO: SelectObject 上次返回空值，造成无法还原原来Object的问题
    return NULL;
  switch(GXGDIOBJ_TYPE(hgdiobj))
  {
  case GXGDIOBJ_BRUSH:
    hPrevObj = (GXHGDIOBJ)GXGDI_DC_PTR(hdc)->hBrush;
    GXGDI_DC_PTR(hdc)->hBrush = (LPGXGDIBRUSH)hgdiobj;
    break;
  case GXGDIOBJ_BITMAP:
    if(GXGDI_DC_PTR(hdc)->emObjType == GXGDIOBJ_MEMDC)
    {
      hPrevObj = (GXHGDIOBJ)GXGDI_DC_PTR(hdc)->lpBitmap;
      SAFE_DELETE(GXGDI_DC_PTR(hdc)->pCanvas);
      GXGDI_DC_PTR(hdc)->lpBitmap = (LPGXGDIBITMAP)hgdiobj;
    }
    else
      ASSERT(FALSE);
    break;
  case GXGDIOBJ_FONT:
    hPrevObj = (GXHGDIOBJ)GXGDI_DC_PTR(hdc)->hFont;
    GXGDI_DC_PTR(hdc)->hFont = (LPGXGDIFONT)hgdiobj;
    break;

  case GXGDIOBJ_PEN:
    hPrevObj = (GXHGDIOBJ)GXGDI_DC_PTR(hdc)->hPen;  // TODO: hPen ->pPen
    GXGDI_DC_PTR(hdc)->hPen = (LPGXGDIPEN)hgdiobj;
    break;

  default:
    ASSERT(FALSE);
  }
  GXGDIOBJ_PTR(hgdiobj)->pDC = GXGDI_DC_PTR(hdc);
  if(hPrevObj != NULL)
    GXGDIOBJ_PTR(hPrevObj)->pDC = NULL;
  return hPrevObj;
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GXDLLAPI gxExtTextOutW(
          GXHDC hdc,      // handle to device context 
          GXINT X,        // x-coordinate of reference point 
          GXINT Y,        // y-coordinate of reference point 
          GXUINT fuOptions,    // text-output options 
          const GXRECT *lprc,  // optional clipping and/or opaquing rectangle 
          GXLPCWSTR lpString,    // points to string 
          GXUINT cbCount,      // number of characters in string 
          const GXINT *lpDx    // pointer to array of intercharacter spacing values  
          )
{
  //ASSERT(lpDx == NULL);
  //hdc->pGraphics->GetSimple_XYZ_COLOR_Shader()->Apply();
  //hdc->pGraphics->FillRectangle(
  //  hdc->crTextBack, 
  //  lprc->left + hdc->wndOrgX, 
  //  lprc->top  + hdc->wndOrgY, 
  //  lprc->right  - lprc->left, 
  //  lprc->bottom - lprc->top
  //  );
  //hdc->pGraphics->GetSimpleShader()->Apply();
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  //lpDC->crTextBack = 0xffffffff;
  //lpDC->crText = 0xff0000ff;
  lpDC->pCanvas->FillRectangle(lprc->left, lprc->top, lprc->right-lprc->left, lprc->bottom-lprc->top, lpDC->crTextBack);

  if(lpString != NULL)
    lpDC->pCanvas->TextOut(GXGDI_FONT_PTR(g_hSystemFont)->lpFont, X, Y, lpString, cbCount, lpDC->crText);
  //  hdc->pGraphics->gxTextOutW(X + hdc->wndOrgX, Y + hdc->wndOrgY, (GXLPCWSTR)lpString, cbCount);
  //  //gxTextOutW(hdc, X + hdc->wndOrgX, Y + hdc->wndOrgY, lpString, cbCount);
  return TRUE;
}



//////////////////////////////////////////////////////////////////////////

GXHBRUSH GXDLLAPI gxCreateSolidBrush(
              GXCOLORREF crColor   // brush color value  
              )
{
  //ASSERT(FALSE);
  LPGXGDIBRUSH lpBrush;
  lpBrush = new GXGDIBRUSH;
  lpBrush->emObjType = GXGDIOBJ_BRUSH;
  lpBrush->uStyle    = GXBS_SOLID;
  lpBrush->crColor     = crColor | 0xff000000;
  lpBrush->hHatch    = NULL;
  return GXGDI_BRUSH_HANDLE(lpBrush);
}

//////////////////////////////////////////////////////////////////////////
GXCOLORREF GXDLLAPI gxSetBkColor(
  GXHDC      hdc,      // handle of device context  
  GXCOLORREF crColor   // background color value
  )
{
  GXGDIDC* pDC = GXGDI_DC_PTR(hdc);
  if(pDC == NULL) {
    return NULL;
  }
  GXCOLORREF crPrevious = pDC->crTextBack;  // 保存旧的颜色值

  if((crColor & 0xff000000) == 0) {   // 如果 Alpha 值为0
    crColor |= 0xff000000;            // 则强制为255，不透明模式
  }

  pDC->crTextBack = crColor;
  return crPrevious;                  // 根据Win32API文档返回前一个颜色设置
}

GXCOLORREF GXDLLAPI GXSetBkColor(GXHDC hdc, GXCOLORREF crColor)
{
  GXGDIDC* pDC = GXGDI_DC_PTR(hdc);
  if(pDC == NULL) {
    return NULL;
  }

  GXCOLORREF crPrevious = pDC->crTextBack;  // 保存旧的颜色值
  pDC->crTextBack = crColor;
  return crPrevious;                  // 根据Win32API文档返回前一个颜色设置
}
//////////////////////////////////////////////////////////////////////////
//gxGetBkColor
GXCOLORREF GXDLLAPI gxGetBkColor(
            GXHDC hdc   // handle of device context
            )
{
  return GXGDI_DC_PTR(hdc)->crTextBack;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDeleteDC(
        GXHDC hdc   // handle to device context 
        )
{
  ASSERT(GXGDI_DC_PTR(hdc)->emObjType == GXGDIOBJ_MEMDC);
  SAFE_DELETE(GXGDI_DC_PTR(hdc)->pCanvas);
  SAFE_DELETE(hdc);
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
GXINT GXDLLAPI gxGetClipRgn(
         GXHDC hdc,  // handle of device context  
         GXHRGN hrgn   // handle of region 
         )
{
  //ASSERT(FALSE);
  if(hrgn)
  {
    GXRECT rcClip;
    GXGDI_DC_PTR(hdc)->pCanvas->GetClipBox(&rcClip);

    GXGDI_RGN_PTR(hrgn)->rect.left   = 0;
    GXGDI_RGN_PTR(hrgn)->rect.top    = 0;
    GXGDI_RGN_PTR(hrgn)->rect.right  = rcClip.right  - rcClip.left;
    GXGDI_RGN_PTR(hrgn)->rect.bottom = rcClip.bottom - rcClip.top;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxBitBlt(
        GXHDC hdcDest,  // handle to destination device context 
        int nXDest,    // x-coordinate of destination rectangle's upper-left corner
        int nYDest,    // y-coordinate of destination rectangle's upper-left corner
        int nWidth,    // width of destination rectangle 
        int nHeight,    // height of destination rectangle 
        GXHDC hdcSrc,    // handle to source device context 
        int nXSrc,    // x-coordinate of source rectangle's upper-left corner  
        int nYSrc,    // y-coordinate of source rectangle's upper-left corner
        GXDWORD dwRop     // raster operation code 
        )
{
  ASSERT(dwRop == GXSRCCOPY);
  GXREGN rgDest(nXDest,nYDest,nWidth,nHeight);
  GXREGN rgSrc(nXSrc,nYSrc,nWidth,nHeight);
  if(GXGDI_DC_PTR(hdcDest)->emObjType == GXGDIOBJ_DC && GXGDI_DC_PTR(hdcSrc)->emObjType == GXGDIOBJ_MEMDC)
  {
    GXGDI_DC_PTR(hdcDest)->pCanvas->DrawTexture(
      ((LPGXGDIBITMAP)(GXGDI_DC_PTR(hdcSrc))->lpBitmap)->pDrawingTexture,
      &rgSrc, &rgDest);
  }
  else if (GXGDI_DC_PTR(hdcDest)->emObjType == GXGDIOBJ_MEMDC && GXGDI_DC_PTR(hdcSrc)->emObjType == GXGDIOBJ_MEMDC)
  {
    LPGXGDIDC lpDC = GXGDI_DC_PTR(hdcDest);
    GXREGN rgSrc(nXSrc, nYSrc, nWidth, nHeight);
    //rgSrc.left = nXSrc;
    //rgSrc.top  = nYSrc;
    //rgSrc.width = nWidth;
    //rgSrc.height = nHeight;
    lpDC->pCanvas->DrawTexture(GXGDI_BITMAP_PTR(GXGDI_DC_PTR(hdcSrc)->lpBitmap)->pDrawingTexture, nXDest, nYDest, &rgSrc);
    //ASSERT(FALSE);
  }
  else
  {
    TRACE("GXGDI_DC_PTR(hdcDest)->emObjType:%d\n", GXGDI_DC_PTR(hdcDest)->emObjType);
    TRACE("GXGDI_DC_PTR(hdcSrc )->emObjType:%d\n", GXGDI_DC_PTR(hdcSrc )->emObjType);
    ASSERT(FALSE);
  }
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// Internal Delete
GXBOOL GXDLLAPI _gxDeleteObject(
              GXHGDIOBJ hObject   // handle to graphic object  
              )
{
  if(hObject != NULL)
  {
    switch(GXGDIOBJ_TYPE(hObject))
    {
    case GXGDIOBJ_REGION:
      {
        GXGDIREGION* pGDIRegion = GXGDI_RGN_PTR(hObject);
        SAFE_RELEASE(pGDIRegion->lpRegion);
        SAFE_DELETE(hObject);
      }
      return TRUE;
    case GXGDIOBJ_BRUSH:
      if(GXGDI_BRUSH_PTR(hObject)->uStyle == GXBS_DIBPATTERN)
      {
        _gxDeleteObject((GXHGDIOBJ)GXGDI_BRUSH_PTR(hObject)->hHatch);
      }
      SAFE_DELETE(hObject);
      return TRUE;
    case GXGDIOBJ_BITMAP:
      //if(GXGDI_BITMAP_PTR(hObject)->pImage->GetSource() == GXImage::IS_Resource)
      //  return TRUE;
      SAFE_RELEASE(GXGDI_BITMAP_PTR(hObject)->pDrawingTexture);
      SAFE_RELEASE(GXGDI_BITMAP_PTR(hObject)->pTargetTexture);
      SAFE_DELETE(GXGDI_BITMAP_PTR(hObject)->bmBits);
      SAFE_DELETE(hObject);
      return TRUE;
    case GXGDIOBJ_FONT:
      // TODO: 会不会在 GXWin32APIEmu::ReleaseStatic() 时出错？
      //GXWin32APIEmu::EraseGXGdiObj(GXGDIOBJ_FONT, hObject);
      SAFE_RELEASE(GXGDI_FONT_PTR(hObject)->lpFont);
      SAFE_DELETE(hObject);
      return TRUE;
    case GXGDIOBJ_PEN:
      SAFE_DELETE(hObject);
      return TRUE;
    }
  }
  else return FALSE;
  ASSERT(FALSE);
  return FALSE;
}

GXBOOL GXDLLAPI gxDeleteObject(
          GXHGDIOBJ hObject   // handle to graphic object  
          )
{
  if(hObject != NULL)
  {
    LPGXGDIDC pDC = GXGDIOBJ_PTR(hObject)->pDC;
    GXGDIOBJ_PTR(hObject)->pDC = NULL;

    switch(GXGDIOBJ_TYPE(hObject))
    {
    case GXGDIOBJ_REGION:
      return _gxDeleteObject(hObject);
    case GXGDIOBJ_BRUSH:
      return _gxDeleteObject(hObject);
    case GXGDIOBJ_BITMAP:
      if(pDC != NULL && pDC->emObjType == GXGDIOBJ_MEMDC)
      {
        SAFE_DELETE(pDC->pCanvas);
        pDC->lpBitmap = NULL;
      }

      //if( GXGDI_BITMAP_PTR(hObject)->pImage != NULL &&
      //  GXGDI_BITMAP_PTR(hObject)->pImage->GetSource() == GXImage::IS_Resource)
      //  //(GXGDI_BITMAP_PTR(hObject)->pImage->GetCreateType() == GTexture::Resource || 
      //  //GXGDI_BITMAP_PTR(hObject)->pImage->GetCreateType() == GTexture::ResourceEx ))
      //  return TRUE;
      return _gxDeleteObject(hObject);
    case GXGDIOBJ_FONT:
      // TODO: 会不会在 GXWin32APIEmu::ReleaseStatic() 时出错？
      if(hObject == g_hSystemFont) {
        return TRUE;
      }
      //GXWin32APIEmu::EraseGXGdiObj(GXGDIOBJ_FONT, hObject);
      return _gxDeleteObject(hObject);
    }
  }
  return _gxDeleteObject(hObject);
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxStretchBlt(
          GXHDC hdcDest,    // handle of destination device context 
          int nXOriginDest,    // x-coordinate of upper-left corner of dest. rect. 
          int nYOriginDest,    // y-coordinate of upper-left corner of dest. rect. 
          int nWidthDest,    // width of destination rectangle 
          int nHeightDest,    // height of destination rectangle 
          GXHDC hdcSrc,      // handle of source device context 
          int nXOriginSrc,    // x-coordinate of upper-left corner of source rectangle 
          int nYOriginSrc,    // y-coordinate of upper-left corner of source rectangle 
          int nWidthSrc,    // width of source rectangle 
          int nHeightSrc,    // height of source rectangle 
          GXDWORD dwRop       // raster operation code 
          )
{
  ASSERT(dwRop == GXSRCCOPY);
  ASSERT(GXGDI_DC_PTR(hdcDest)->emObjType == GXGDIOBJ_DC);
  ASSERT(GXGDI_DC_PTR(hdcSrc )->emObjType == GXGDIOBJ_MEMDC);

  Marimo::REGNT<GXLONG> rgDest = {nXOriginDest,nYOriginDest,nWidthDest,nHeightDest};
  Marimo::REGNT<GXLONG> rgSrc = {nXOriginSrc,nYOriginSrc,nWidthSrc,nHeightSrc};
  GXGDI_DC_PTR(hdcDest)->pCanvas->DrawTexture(
    ((LPGXGDIBITMAP)(GXGDI_DC_PTR(hdcSrc))->lpBitmap)->pDrawingTexture,
    rgSrc, rgDest);
  //ASSERT(FALSE);
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
GXHDC GXDLLAPI gxCreateCompatibleDC(
             GXHDC hdc   // handle to memory device context 
             )
{
  LPGXGDIDC pNew    = new GXGDIDC;
  pNew->emObjType    = GXGDIOBJ_MEMDC;
  pNew->pCanvas    = NULL;
  pNew->lpBitmap    = NULL;
  if(hdc != NULL)
    pNew->hBindWnd    = ((LPGXGDIDC)hdc)->hBindWnd;
  else
    pNew->hBindWnd    = NULL;
  pNew->crTextBack  = 0xFFFFFFFF;
  pNew->crText    = 0xFF000000; 
  pNew->flag      = GXDCFLAG_OPAQUEBKMODE | GXDCFLAG_COMPATIBLE;
  pNew->hBrush    = &g_WhiteBrush;
  pNew->hPen      = &g_BlackPen;
  pNew->hFont      = GXGDI_FONT_PTR(g_hSystemFont);
  pNew->ptPen.x    = 0;
  pNew->ptPen.y    = 0;

  return GXGDI_DC_HANDLE(pNew);
}
//////////////////////////////////////////////////////////////////////////
GXCOLORREF GXDLLAPI gxSetTextColor(
            GXHDC hdc,      // handle of device context  
            GXCOLORREF crColor   // text color 
            )
{
  GXCOLORREF crPrevious = ((LPGXGDIDC)hdc)->crText;        // 保存前一个颜色
  if((crColor & 0xff000000) == 0)            // Win32API的兼容模式
    crColor |= 0xff000000;              // 如果Alpha为0则认为使用了
                            // Win32API的颜色设置，将此
                            // 默认转换为不透明（255）
  ((LPGXGDIDC)hdc)->crText = crColor;          // 设置hdc
  //hdc->pGraphics->SetFontColor(crColor | 0xff000000);  // 同时设置Graphics
  return crPrevious;                  // 按照文档返回前次颜色
}
//////////////////////////////////////////////////////////////////////////
//gxGetTextColor
GXCOLORREF GXDLLAPI gxGetTextColor(
            GXHDC hdc   // handle of device context 
            )
{
  //ASSERT(FALSE);
  return ((LPGXGDIDC)hdc)->crText;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDPtoLP( 
        GXHDC hdc,  // handle to device context 
        GXLPPOINT lpPoints,  // pointer to array of points  
        GXINT nCount   // count of points 
        )
{
  //ASSERT(FALSE);
  NOT_IMPLEMENT_FUNC_MAKER;
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXINT GXDLLAPI gxIntersectClipRect(
            GXHDC hdc,      // handle of device context 
            GXINT nLeftRect,  // x-coordinate of upper-left corner of rectangle 
            GXINT nTopRect,    // y-coordinate of upper-left corner of rectangle 
            GXINT nRightRect,  // x-coordinate of lower-right corner of rectangle  
            GXINT nBottomRect   // y-coordinate of lower-right corner of rectangle  
            )
{
  //ASSERT(FALSE);
  TRACE_UNACHIEVE("=== gxIntersectClipRect ===\n");
  return 0;
}
//////////////////////////////////////////////////////////////////////////
GXINT GXDLLAPI gxSelectClipRgn(
          GXHDC hdc,  // handle of device context 
          GXHRGN hrgn   // handle of region to be selected  
          )
{
  if(hrgn)
  {
    ASSERT(FALSE);
  }
  else
  {
    //((LPGXGDIDC)hdc)->pCanvas->m_stCanvasState.rcClip.left   = ((LPGXGDIDC)hdc)->pCanvas->m_xAbsOrigin;
    //((LPGXGDIDC)hdc)->pCanvas->m_stCanvasState.rcClip.top    = ((LPGXGDIDC)hdc)->pCanvas->m_yAbsOrigin;
    //((LPGXGDIDC)hdc)->pCanvas->m_stCanvasState.rcClip.right  = ((LPGXGDIDC)hdc)->pCanvas->m_xAbsOrigin + ((LPGXGDIDC)hdc)->pCanvas->m_xExt;
    //((LPGXGDIDC)hdc)->pCanvas->m_stCanvasState.rcClip.bottom = ((LPGXGDIDC)hdc)->pCanvas->m_xAbsOrigin + ((LPGXGDIDC)hdc)->pCanvas->m_yExt;
    return GXSIMPLEREGION;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////
GXHRGN GXDLLAPI gxCreateRectRgn(
           int nLeftRect,  // x-coordinate of region's upper-left corner 
           int nTopRect,  // y-coordinate of region's upper-left corner 
           int nRightRect,  // x-coordinate of region's lower-right corner  
           int nBottomRect   // y-coordinate of region's lower-right corner  
           )
{
  //ASSERT(FALSE);
  LPGXGDIREGION lpRegion = new GXGDIREGION;
  if(lpRegion == NULL) {
    return NULL;
  }

  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();

  lpRegion->emObjType = GXGDIOBJ_REGION;
  lpRegion->rect.left   = nLeftRect;
  lpRegion->rect.top    = nTopRect;
  lpRegion->rect.right  = nRightRect;
  lpRegion->rect.bottom = nBottomRect;

  lpStation->pGraphics->CreateRectRgn(&lpRegion->lpRegion, nLeftRect, nTopRect, nRightRect, nBottomRect);

  return (GXHRGN)lpRegion;
}
//////////////////////////////////////////////////////////////////////////
//gxCreateRectRgnIndirect
//gxGetTextExtentPointW
//gxSetWindowOrgEx
GXHRGN GXDLLAPI gxCreateRectRgnIndirect(
               const GXRECT *lprc   // pointer to the rectangle  
               )
{
  if(lprc == NULL)
    return gxCreateRectRgn(0, 0, 0, 0);
  return gxCreateRectRgn(lprc->left, lprc->top, lprc->right, lprc->bottom);
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxGetTextExtentPointW(
            GXHDC hdc,        // handle of device context 
            GXLPCWSTR lpString,    // address of text string 
            GXINT cbString,      // number of characters in string 
            GXLPSIZE lpSize     // address of structure for string size  
            )
{
  //GXBOOL result;
  ////float flAspect = hdc->pGraphics->GetFontSize() / ;
  //result = (GXBOOL)(GetTextExtentPointW(((LPGXGDIDC)hdc)->pCanvas->m_hWin32DC, lpString, (int)cbString, (LPSIZE)lpSize) == TRUE);
  ////lpSize->cx = lpSize->cx * hdc->pGraphics->GetFontSize() / hdc->pGraphics->GetFontOriSize();
  ////lpSize->cy = lpSize->cy * hdc->pGraphics->GetFontSize() / hdc->pGraphics->GetFontOriSize();
  //return result;

  return gxGetTextExtentPoint32W(hdc, lpString, cbString, lpSize);
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxSetWindowOrgEx(
          GXHDC hdc,        // handle of device context 
          GXINT X,        // new x-coordinate of window origin 
          GXINT Y,        // new y-coordinate of window origin 
          GXLPPOINT lpPoint    // address of structure receiving original origin  
          )
{
  LPGXGDIDC  lpDC  = GXGDI_DC_PTR(hdc);

  lpDC->pCanvas->SetViewportOrg(X, Y, lpPoint);

  //if(lpPoint)
  //{
  //  lpPoint->x = ((LPGXGDIDC)hdc)->pCanvas->m_xAbsOrigin - ((LPGXGDIDC)hdc)->pCanvas->m_xOrigin;
  //  lpPoint->y = ((LPGXGDIDC)hdc)->pCanvas->m_yAbsOrigin - ((LPGXGDIDC)hdc)->pCanvas->m_yOrigin;
  //}
  //((LPGXGDIDC)hdc)->pCanvas->m_xOrigin = ((LPGXGDIDC)hdc)->pCanvas->m_xAbsOrigin - X;
  //((LPGXGDIDC)hdc)->pCanvas->m_yOrigin = ((LPGXGDIDC)hdc)->pCanvas->m_yAbsOrigin - Y;
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
//gxTextOutW
GXBOOL GXDLLAPI gxTextOutW(
       GXHDC hdc,        // handle of device context 
       GXINT nXStart,      // x-coordinate of starting position  
       GXINT nYStart,      // y-coordinate of starting position  
       GXLPCWSTR lpString,    // address of string 
       GXINT cbString      // number of characters in string 
       )
{
  GXSIZE size;
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  GrapX::GXCanvas* pCanvas = lpDC->pCanvas;

  if(lpDC->flag & GXDCFLAG_OPAQUEBKMODE)
  {
    gxGetTextExtentPointW(hdc, lpString, cbString, &size);
    pCanvas->FillRectangle(nXStart, nYStart, size.cx, size.cy, ((LPGXGDIDC)hdc)->crTextBack);
  }
  LPGXGDIFONT lpFont = GXGDI_FONT_PTR(lpDC->hFont);
  pCanvas->TextOut(lpFont->lpFont, nXStart, nYStart, lpString, cbString, ((LPGXGDIDC)hdc)->crText);
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
//gxPatBlt
GXBOOL GXDLLAPI gxPatBlt(
      GXHDC hdc,      // handle to device context 
      GXINT nXLeft,    // x-coord. of upper-left corner of rect. to be filled 
      GXINT nYLeft,    // y-coord. of upper-left corner of rect. to be filled 
      GXINT nWidth,    // width of rectangle to be filled 
      GXINT nHeight,    // height of rectangle to be filled 
      GXDWORD dwRop     // raster operation code 
      )
{
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  LPGXGDIBRUSH lpBrush = GXGDI_BRUSH_PTR(lpDC->hBrush);
  GXREGN regn;
  regn.left  = nXLeft;
  regn.top  = nYLeft;
  regn.width  = nWidth;
  regn.height = nHeight;
  if(lpBrush->uStyle == GXBS_DIBPATTERN)
  {
    lpDC->pCanvas->DrawTexture(GXGDI_BITMAP_PTR(lpBrush->hHatch)->pDrawingTexture, &regn);
  }
  else if(lpBrush->uStyle == GXBS_SOLID)
  {
    lpDC->pCanvas->DrawRectangle(nXLeft, nYLeft, nWidth, nHeight, lpBrush->crColor);
  }
    return NULL;
  //ASSERT(FALSE);
  //TRACE_UNACHIEVE("=== gxPatBlt ===\n");
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//gxSetBkMode
GXINT GXDLLAPI gxSetBkMode(
        GXHDC hdc,    // handle of device context
        GXINT iBkMode   // flag specifying background mode
        )
{
  //ASSERT(FALSE);
  int iPrev = (((LPGXGDIDC)hdc)->flag & GXDCFLAG_OPAQUEBKMODE) != 0 ? GXOPAQUE : GXTRANSPARENT;

  if(iBkMode == GXOPAQUE)
    ((LPGXGDIDC)hdc)->flag |= GXDCFLAG_OPAQUEBKMODE;
  else if(iBkMode == GXTRANSPARENT)
    ((LPGXGDIDC)hdc)->flag &= (~GXDCFLAG_OPAQUEBKMODE);
  return iPrev;
}
//////////////////////////////////////////////////////////////////////////
//gxGetBkMode
GXINT GXDLLAPI gxGetBkMode(
        GXHDC hdc   // handle to device context of interest
        )
{
  return (((LPGXGDIDC)hdc)->flag & GXDCFLAG_OPAQUEBKMODE) != 0 ? GXOPAQUE : GXTRANSPARENT;
}
//////////////////////////////////////////////////////////////////////////
//gxSetRectRgn
GXBOOL GXDLLAPI gxSetRectRgn(
        GXHRGN hrgn,      // handle of region
        GXINT nLeftRect,    // x-coordinate of upper-left corner of rectangle
        GXINT nTopRect,      // y-coordinate of upper-left corner of rectangle
        GXINT nRightRect,    // x-coordinate of lower-right corner of rectangle  
        GXINT nBottomRect     // y-coordinate of lower-right corner of rectangle  
        )
{
  //ASSERT(FALSE);
  if(hrgn)
  {
    GXGDIREGION* lprgn = GXGDI_RGN_PTR(hrgn);
    lprgn->rect.left   = nLeftRect;
    lprgn->rect.top    = nTopRect;
    lprgn->rect.right  = nRightRect;
    lprgn->rect.bottom = nBottomRect;
    if(lprgn->lpRegion) {
      lprgn->lpRegion->SetRect(&lprgn->rect);
    }
    else {
      GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
      lpStation->pGraphics->CreateRectRgn(&lprgn->lpRegion, nLeftRect, nTopRect, nRightRect, nBottomRect);
    }
    return TRUE;
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
//gxGetObjectW
int GXDLLAPI gxGetObjectW(
        GXHGDIOBJ hgdiobj,    // handle to graphics object of interest
        int cbBuffer,        // size of buffer for object information 
        GXLPVOID lpvObject      // pointer to buffer for object information  
        )
{
  switch(GXGDIOBJ_TYPE(hgdiobj))
  {
  case GXGDIOBJ_BITMAP:
    ASSERT(cbBuffer == sizeof(GXBITMAP));
    GXGDI_BITMAP_PTR(hgdiobj)->pDrawingTexture->GetDesc((GXBITMAP*)lpvObject);
    return sizeof(GXBITMAP);
  case GXGDIOBJ_BRUSH:
    ASSERT(cbBuffer == sizeof(GXLOGBRUSH));
    ((GXLPLOGBRUSH)lpvObject)->lbStyle = GXBS_SOLID;
    ((GXLPLOGBRUSH)lpvObject)->lbColor = (GXCOLORREF)GXGDI_BRUSH_PTR(hgdiobj)->crColor;
    ((GXLPLOGBRUSH)lpvObject)->lbHatch = 0;
    return sizeof(GXLOGBRUSH);
  case GXGDIOBJ_FONT:
    ASSERT(cbBuffer == sizeof(GXLOGFONT));
    ASSERT(GXGDI_FONT_PTR(hgdiobj)->emObjType == GXGDIOBJ_FONT);
    //D3DXFONT_DESC d3dfd;
    GXGDI_FONT_PTR(hgdiobj)->lpFont->GetDescW((GXLPLOGFONTW)lpvObject);

    //memset(lpvObject, 0, cbBuffer);
    //((LPLOGFONT)lpvObject)->lfHeight      = d3dfd.Height;
    //((LPLOGFONT)lpvObject)->lfWidth        = d3dfd.Width;
    //((LPLOGFONT)lpvObject)->lfWeight      = d3dfd.Weight;
    //((LPLOGFONT)lpvObject)->lfItalic      = d3dfd.Italic;
    //((LPLOGFONT)lpvObject)->lfCharSet      = d3dfd.CharSet;
    //((LPLOGFONT)lpvObject)->lfOutPrecision    = d3dfd.OutputPrecision;
    //((LPLOGFONT)lpvObject)->lfQuality      = d3dfd.Quality;
    //((LPLOGFONT)lpvObject)->lfPitchAndFamily  = d3dfd.PitchAndFamily;
    //lstrcpynW(((LPLOGFONT)lpvObject)->lfFaceName, d3dfd.FaceName, 32);

    return sizeof(GXLOGFONT);
  }
  ASSERT(FALSE);
  return 0;
}
//////////////////////////////////////////////////////////////////////////
//gxCreateFontIndirectW
GXHFONT GXDLLAPI gxCreateFontIndirectW(
             const GXLOGFONTW *lplf   // pointer to logical font structure  
             )
{
  //D3DXFONT_DESC d3dfd;
  //d3dfd.Height      = lplf->lfHeight;
  //d3dfd.Width        = lplf->lfWidth;
  //d3dfd.Weight      = lplf->lfWeight;
  //d3dfd.MipLevels      = D3DX_DEFAULT;
  //d3dfd.Italic      = lplf->lfItalic;
  //d3dfd.CharSet      = lplf->lfCharSet;
  //d3dfd.OutputPrecision  = lplf->lfOutPrecision;
  //d3dfd.Quality      = lplf->lfQuality;
  //d3dfd.PitchAndFamily  = lplf->lfPitchAndFamily;
  //lstrcpynW(d3dfd.FaceName, lplf->lfFaceName, 32);

  GXGDIFONT *pFont;
  pFont = new GXGDIFONT;
  pFont->emObjType = GXGDIOBJ_FONT;

  //pFont->lpFont = NEW GXFont(&d3dfd);

  pFont->lpFont = GrapX::Internal::GetStationPtr()->pGraphics->CreateFontIndirectW((const GXLPLOGFONTW)lplf);

  //GXCreateFreeTypeFontIndirectW((const LPLOGFONTW)lplf, &pFont->lpFont);

  //GXWin32APIEmu::MapGXGdiObj(GXGDIOBJ_FONT, pFont, NULL);

  //if(FAILED(D3DXCreateFontIndirectW(g_pd3dDevice, &d3dfd, (LPD3DXFONT *)&pFont->lpD3DFont)))
  //{
  //  SAFE_DELETE(pFont);
  //  return NULL;
  //}

  return (GXHFONT)pFont;
}
//////////////////////////////////////////////////////////////////////////
//gxCombineRgn
int GXDLLAPI gxCombineRgn(
         GXHRGN hrgnDest,      // handle to destination region 
         GXHRGN hrgnSrc1,      // handle to source region 
         GXHRGN hrgnSrc2,      // handle to source region 
         int fnCombineMode     // region combining mode 
         )
{
  //ASSERT(FALSE);
  STATIC_ASSERT(GXERROR         == RC_ERROR  );
  STATIC_ASSERT(NULLREGION    == RC_NULL   );
  STATIC_ASSERT(GXSIMPLEREGION  == RC_SIMPLE );
  STATIC_ASSERT(GXCOMPLEXREGION == RC_COMPLEX);

  if(fnCombineMode == GXRGN_COPY) {
    if(hrgnDest == hrgnSrc1) {
      return GXERROR;
    }
    GXGDIREGION* lpGDIDestRgn = GXGDI_RGN_PTR(hrgnDest);
    GXGDIREGION* lpGDISrc1Rgn = GXGDI_RGN_PTR(hrgnSrc1);
    SAFE_RELEASE(lpGDIDestRgn->lpRegion);

    lpGDIDestRgn->lpRegion = lpGDISrc1Rgn->lpRegion->Clone();
    lpGDIDestRgn->rect = lpGDISrc1Rgn->rect;
    return lpGDIDestRgn->lpRegion->GetComplexity();
  }

  if(hrgnDest && hrgnSrc1 && hrgnSrc2)
  {
    if(hrgnDest == hrgnSrc1)
    {
      int eComplex = GXERROR;
      GXGDIREGION* lpGDIDestRgn = GXGDI_RGN_PTR(hrgnDest);
      GXGDIREGION* lpGDISrc2Rgn = GXGDI_RGN_PTR(hrgnSrc2);
      GRegion*& lpDest = lpGDIDestRgn->lpRegion;
      GRegion* lpSrc2 = lpGDISrc2Rgn->lpRegion;

      ASSERT(lpDest && lpSrc2);

      switch (fnCombineMode)
      {
      case RGN_AND:
        eComplex = (int)lpDest->Intersect(lpSrc2);
        break;

      case RGN_DIFF:
        CLBREAK; // 么实现！
        break;

      case RGN_OR:
        eComplex = (int)lpDest->Union(lpSrc2);
        break;

      case RGN_XOR:
        eComplex = (int)lpDest->Xor(lpSrc2);
        break;
      }

      lpDest->GetBounding(&lpGDIDestRgn->rect);
      return eComplex;
    }
    else
    {
      GXGDIREGION* lpGDIDestRgn = GXGDI_RGN_PTR(hrgnDest);
      GXGDIREGION* lpGDISrc1Rgn = GXGDI_RGN_PTR(hrgnSrc1);
      GXGDIREGION* lpGDISrc2Rgn = GXGDI_RGN_PTR(hrgnSrc2);

      GRegion*& lpDest = lpGDIDestRgn->lpRegion;
      GRegion* lpSrc1 = lpGDISrc1Rgn->lpRegion;
      GRegion* lpSrc2 = lpGDISrc2Rgn->lpRegion;

      ASSERT(lpSrc1 && lpSrc2);

      switch (fnCombineMode)
      {
      case RGN_AND:
        SAFE_RELEASE(lpDest);
        lpDest = lpSrc1->CreateIntersect(lpSrc2);
        break;

      case RGN_DIFF:
        SAFE_RELEASE(lpDest);
        CLBREAK; // 么实现！
        break;

      case RGN_OR:
        SAFE_RELEASE(lpDest);
        lpDest = lpSrc1->CreateUnion(lpSrc2);
        break;

      case RGN_XOR:
        SAFE_RELEASE(lpDest);
        lpDest = lpSrc1->CreateXor(lpSrc2);
        break;

      default:
        return GXERROR;
      }
      lpDest->GetBounding(&lpGDIDestRgn->rect);
      return (int)lpDest->GetComplexity();
    }
  }
  return GXERROR;
}
//gxGetTextMetricsW
GXBOOL GXDLLAPI gxGetTextMetricsW(
          GXHDC hdc,        // handle of device context 
          GXLPTEXTMETRICW lptm     // address of text metrics structure 
          )
{
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  GXGDIFONT* lpFont = GXGDI_FONT_PTR(lpDC->hFont);
  if(lpFont && lpFont->lpFont) {
    return lpFont->lpFont->GetMetricW(lptm);
  }
  return FALSE;
}
//gxGetClipBox
GXINT GXDLLAPI gxGetClipBox(
         GXHDC hdc,      // handle of the device context 
         GXLPRECT lprc       // address of structure with rectangle  
         )
{
  //ASSERT(FALSE);
  if(hdc == NULL)
    return GXERROR;
  gxGetClientRect(((LPGXGDIDC)hdc)->hBindWnd, lprc);
  return GXSIMPLEREGION;
}
//gxGetCurrentObject
GXHGDIOBJ GXDLLAPI gxGetCurrentObject(
             GXHDC hdc,      // handle of device context 
             GXUINT uObjectType   // object-type identifier 
             )
{
  switch(uObjectType)
  {
  case GXOBJ_FONT:
    return (GXHGDIOBJ)((LPGXGDIDC)hdc)->hFont;
  }
  ASSERT(FALSE);
  return NULL;
}
//gxGetTextExtentPoint32W
GXBOOL GXDLLAPI gxGetTextExtentPoint32W(
              GXHDC hdc,      // handle of device context 
              GXLPCWSTR lpString,    // address of text string 
              GXINT cbString,    // number of characters in string 
              GXLPSIZE lpSize     // address of structure for string size  
              )
{
  ////GXFLOAT flAspect = (GXFLOAT)hdc->pGraphics->GetFontSize() / (GXFLOAT)hdc->pGraphics->GetFontOriSize();
  //GXBOOL bRet = (GXBOOL)(GetTextExtentPoint32W(((LPGXGDIDC)hdc)->pCanvas->m_hWin32DC, lpString, (int)cbString, (LPSIZE)lpSize) == TRUE);
  ////lpSize->cx = (GXLONG)(flAspect * lpSize->cx);
  ////lpSize->cy = (GXLONG)(flAspect * lpSize->cy);
  //return bRet;
  GXRECT rect(0);

  LPGXGDIDC  lpDC  = GXGDI_DC_PTR(hdc);
  LPGXGDIFONT lpFont  = GXGDI_FONT_PTR(lpDC->hFont);

  lpDC->pCanvas->DrawText(lpFont->lpFont, lpString, cbString, &rect, GXDT_SINGLELINE | GXDT_CALCRECT, 0);
  lpSize->cx = rect.right;
  lpSize->cy = rect.bottom;
  ASSERT(cbString == 0 || (rect.right != 0 && rect.bottom != 0));
  return TRUE;
}

GXBOOL GXDLLAPI gxRectangle(
           GXHDC hdc,  // handle of device context 
           GXINT nLeftRect,  // x-coord. of bounding rectangle's upper-left corner 
           GXINT nTopRect,  // y-coord. of bounding rectangle's upper-left corner 
           GXINT nRightRect,  // x-coord. of bounding rectangle's lower-right corner  
           GXINT nBottomRect   // y-coord. of bounding rectangle's lower-right corner  
           )
{
  //ASSERT(FALSE);
  TRACE_UNACHIEVE("=== gxRectangle ===\n");
  return FALSE;
}
GXBOOL GXDLLAPI gxMoveToEx(
          GXHDC hdc,  // handle of device context 
          int X,  // x-coordinate of new current position  
          int Y,  // y-coordinate of new current position  
          LPGXPOINT lpPoint   // address of old current position 
          )
{
  //ASSERT(FALSE);
  ASSERT(lpPoint == NULL);
  ((LPGXGDIDC)hdc)->ptPen.x = X;
  ((LPGXGDIDC)hdc)->ptPen.y = Y;
  return TRUE;
}
GXBOOL GXDLLAPI gxLineTo(
        GXHDC hdc,  // device context handle 
        int nXEnd,  // x-coordinate of line's ending point  
        int nYEnd   // y-coordinate of line's ending point  
        )
{
  //ASSERT(FALSE);
  //TRACE_UNACHIEVE("=== gxLineTo ===\n");
  ((LPGXGDIDC)hdc)->pCanvas->DrawLine(
    ((LPGXGDIDC)hdc)->ptPen.x,
    ((LPGXGDIDC)hdc)->ptPen.y,
    nXEnd,
    nYEnd,
    0xFF000000
    );
  ((LPGXGDIDC)hdc)->ptPen.x = nXEnd;
  ((LPGXGDIDC)hdc)->ptPen.y = nYEnd;
  return FALSE;
}
GXBOOL GXDLLAPI gxSetViewportOrgEx(
              GXHDC hdc,  // handle of device context 
              int X,  // new x-coordinate of viewport origin 
              int Y,  // new y-coordinate of viewport origin 
              LPGXPOINT lpPoint   // address of structure receiving original origin  
              )
{
  return GXGDI_DC_PTR(hdc)->pCanvas->SetViewportOrg(X, Y, lpPoint);
}
GXHGDIOBJ GXDLLAPI gxGetStockObject(
               int fnObject   // type of stock object 
               )
{
  switch(fnObject)
  {
  case GXBLACK_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_BlackBrush);
  case GXDKGRAY_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_DarkGrayBrush);
  case GXDC_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_WhiteBrush);
  case GXGRAY_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_GrayBrush);
  case GXLTGRAY_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_LightGrayBrush);
  case NULL_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_NullBrush);
  case GXWHITE_BRUSH:
    return GXGDI_BRUSH_HANDLE(&g_WhiteBrush);
  case GXBLACK_PEN:
    return GXGDI_PEN_HANDLE(&g_BlackPen);
  case GXDC_PEN:
    return GXGDI_PEN_HANDLE(&g_WhitePen);
  case GXWHITE_PEN:
    return GXGDI_PEN_HANDLE(&g_WhitePen);
  case NULL_PEN:
    return GXGDI_PEN_HANDLE(&g_NullPen);
  case GXSYSTEM_FONT:
    return g_hSystemFont;
  default:
    ASSERT(FALSE);
    break;
  }
  return NULL;
}

GXHBITMAP GXDLLAPI gxCreateCompatibleBitmap(
        GXHDC hdc,  // handle to device context 
        int nWidth,  // width of bitmap, in pixels  
        int nHeight   // height of bitmap, in pixels  
        )
{
  LPGXGDIBITMAP  pNewBitmap = new GXGDIBITMAP;
  memset(pNewBitmap, 0, sizeof(GXGDIBITMAP));
  pNewBitmap->emObjType   = GXGDIOBJ_BITMAP;
  pNewBitmap->bmWidth     = nWidth;
  pNewBitmap->bmHeight   = nHeight;
  pNewBitmap->bmWidthBytes = nWidth * sizeof(GXDWORD);
  pNewBitmap->bmPlanes   = 1;
  pNewBitmap->bmBitsPixel   = 32;
  pNewBitmap->bmBits     = NULL;

  //pNewBitmap->pImage = GXGDI_DC_PTR(hdc)->pCanvas->GetGraphicsUnsafe()->CreateImage(nWidth, nHeight, GXFMT_A8R8G8B8, TRUE, NULL);
  GXGDI_DC_PTR(hdc)->pCanvas->GetGraphicsUnsafe()->CreateTexture(&pNewBitmap->pDrawingTexture, NULL, nWidth, nHeight, GXFMT_A8R8G8B8, GXResUsage::ReadWrite, 1);
  //GXCreateImage(nWidth, nHeight, NULL, TRUE, &pNewBitmap->pImage);
  //pNewBitmap->pImage = CreateTexture
  return GXGDI_BITMAP_HANDLE(pNewBitmap);
}

GXHBITMAP GXDLLAPI gxCreateBitmap(
                 int nWidth,      // bitmap width, in pixels 
                 int nHeight,      // bitmap height, in pixels 
                 GXUINT cPlanes,    // number of color planes used by device 
                 GXUINT cBitsPerPel,  // number of bits required to identify a color  
                 const GXVOID *lpvBits   // pointer to array containing color data 
                 )
{
  GXLPVOID lpTBits = NULL;
  LPGXGDIBITMAP  pNewBitmap = new GXGDIBITMAP;
  memset(pNewBitmap, 0, sizeof(GXGDIBITMAP));

  pNewBitmap->emObjType   = GXGDIOBJ_BITMAP;
  pNewBitmap->bmWidth     = nWidth;
  pNewBitmap->bmHeight       = nHeight;
  pNewBitmap->bmWidthBytes = nWidth * sizeof(GXDWORD);
  pNewBitmap->bmPlanes       = 1;
  pNewBitmap->bmBitsPixel   = 32;
  pNewBitmap->bmBits     = NULL;
  //if(lpvBits != NULL)
  //{
  //  if(cBitsPerPel == 1)
  //  {
  //    lpTBits = NEW GXBYTE[nWidth * nHeight * 4];
  //    //GXCOLORREF *lpColor = lpTBits;
  //    //GXBYTE *lpBytes = (GXBYTE*)lpvBits;
  //    GXUINT n = 0;

  //    for(int h = 0; h < nHeight; h++)
  //    {
  //      for(int w = 0; w < nWidth; w++)
  //      {
  //        if( ((((GXBYTE*)lpvBits)[n >> 3]) & (1 << (n & 7))) == 0)
  //          ((GXCOLORREF*)lpTBits)[n] = 0xff000000;
  //        else
  //          ((GXCOLORREF*)lpTBits)[n] = 0xffffffff;
  //        n++;
  //      }
  //    }
  //  }
  //  else if(cBitsPerPel == 32)
  //  {
  //    lpTBits = (GXLPVOID)lpvBits;
  //  }
  //  else ASSERT(false);
  //}

  // pNewBitmap->pImage = GrapX::Internal::GetStationPtr()->pGraphics->CreateImage(nWidth, nHeight, GXFMT_A8R8G8B8, TRUE, lpTBits);
  GrapX::Internal::GetStationPtr()->pGraphics
    ->CreateTexture(&pNewBitmap->pDrawingTexture, NULL, nWidth, nHeight, GXFMT_A8R8G8B8, GXResUsage::Default, 1, lpTBits);
  //GXCreateImage(nWidth, nHeight, lpTBits, TRUE, &pNewBitmap->pImage);
  //if(cBitsPerPel != 32 && lpTBits != lpvBits)
  //  SAFE_DELETE(lpTBits);
  return GXGDI_BITMAP_HANDLE(pNewBitmap);
}

GXHBRUSH GXDLLAPI gxCreatePatternBrush(
                     GXHBITMAP hbmp   // handle to bitmap 
                     )
{
  LPGXGDIBRUSH lpNewBrush;
  lpNewBrush = new GXGDIBRUSH;
  lpNewBrush->emObjType = GXGDIOBJ_BRUSH;
  lpNewBrush->uStyle = GXBS_DIBPATTERN;
  lpNewBrush->crColor = 0xff000000;
  lpNewBrush->hHatch = (GXLONG_PTR)gxCopyImage(hbmp, GXIMAGE_BITMAP, 0, 0, GXLR_COPYRETURNORG);
  //ASSERT(FALSE);
  return GXGDI_BRUSH_HANDLE(lpNewBrush);
}

GXCOLORREF GXDLLAPI gxSetPixel(
          GXHDC hdc,  // handle of device context  
          int X,  // x-coordinate of pixel 
          int Y,  // y-coordinate of pixel 
          GXCOLORREF crColor   // pixel color 
          )
{
  GXGDI_DC_PTR(hdc)->pCanvas->SetPixel(X, Y, crColor);
  //ASSERT(FALSE);
  return NULL;
}

GXCOLORREF GXDLLAPI gxGetPixel(
          GXHDC hdc,  // handle of device context  
          int XPos,    // x-coordinate of pixel 
          int nYPos   // y-coordinate of pixel 
          )
{
  ASSERT(FALSE);
  return NULL;
}
GXHBITMAP GXDLLAPI gxCreateDIBSection(
             GXHDC hdc,          // handle to device context
             const GXBITMAPINFO *pbmi,  // pointer to structure containing bitmap size, format, and color data
             GXUINT iUsage,        // color data type indicator: RGB values or palette indices
             GXVOID *ppvBits,      // pointer to variable to receive a pointer to the bitmap's bit values
             GXHANDLE hSection,      // optional handle to a file mapping object
             GXDWORD dwOffset      // offset to the bitmap bit values within the file mapping object
             )
{
  LPGXGDIBITMAP  pNewBitmap = new GXGDIBITMAP;
  memset(pNewBitmap, 0, sizeof(GXGDIBITMAP));
  pNewBitmap->emObjType   = GXGDIOBJ_BITMAP;
  pNewBitmap->bmWidth     = pbmi->bmiHeader.biWidth;
  pNewBitmap->bmHeight   = pbmi->bmiHeader.biHeight;
  pNewBitmap->bmWidthBytes = pbmi->bmiHeader.biWidth * sizeof(GXDWORD);
  pNewBitmap->bmPlanes   = pbmi->bmiHeader.biPlanes;
  pNewBitmap->bmBitsPixel   = 32;
  pNewBitmap->bmBits     = new GXCHAR[pNewBitmap->bmWidthBytes * pNewBitmap->bmHeight];
  //GXCreateImage(pNewBitmap->bmWidth, pNewBitmap->bmHeight, NULL, TRUE, &pNewBitmap->pImage);

  //pNewBitmap->pImage = GXGDI_DC_PTR(hdc)->pCanvas->GetGraphicsUnsafe()
  //  ->CreateImage(pNewBitmap->bmWidth, pNewBitmap->bmHeight, GXFMT_A8R8G8B8, TRUE, NULL);

  GXGDI_DC_PTR(hdc)->pCanvas->GetGraphicsUnsafe()
    ->CreateTexture(&pNewBitmap->pDrawingTexture, NULL, pNewBitmap->bmWidth, pNewBitmap->bmHeight, GXFMT_A8R8G8B8, GXResUsage::Default, 1);

  if(ppvBits != NULL)
    *(GXLPVOID*)ppvBits = pNewBitmap->bmBits;
  ASSERT(iUsage == GXDIB_RGB_COLORS && hSection == NULL && dwOffset == NULL);
  return GXGDI_BITMAP_HANDLE(pNewBitmap);
}

GXUINT GXDLLAPI gxGetPaletteEntries(
             GXHPALETTE hpal,  // handle of logical color palette 
             GXUINT iStartIndex,  // first entry to retrieve 
             GXUINT nEntries,  // number of entries to retrieve 
             GXLPPALETTEENTRY lppe   // address of array receiving entries  
             )
{
  ASSERT(FALSE);
  return NULL;
}

int GXDLLAPI gxGetDeviceCaps(
          GXHDC hdc,  // device-context handle 
          int nIndex   // index of capability to query  
          )
{
  ASSERT(FALSE);
  return NULL;
}

GXBOOL GXDLLAPI gxRectVisible(
         GXHDC hdc,  // handle of the device context 
         const GXRECT *lprc   // address of rectangle structure  
         )
{
  GXRECT rect;
  GXRECT rcClip;

  GXGDI_DC_PTR(hdc)->pCanvas->GetClipBox(&rcClip);
  return gxIntersectRect(&rect, lprc, &rcClip);
}

int GXDLLAPI gxSetROP2(
      GXHDC hdc,  // handle of device context  
      int fnDrawMode   // drawing mode
      )
{
  ASSERT(FALSE);
  return NULL;
}
GXHPEN GXDLLAPI gxExtCreatePen(
          GXDWORD dwPenStyle,  // pen style 
          GXDWORD dwWidth,  // pen width 
          const GXLOGBRUSH *lplb,  // pointer to structure for brush attributes 
          GXDWORD dwStyleCount,  // length of array containing custom style bits 
          const GXDWORD *lpStyle   // optional array of custom style bits 
          )
{
  LPGXGDIPEN lpPen;
  lpPen = new GXGDIPEN;
  lpPen->emObjType = GXGDIOBJ_PEN;
  lpPen->uStyle = GXPS_SOLID;
  lpPen->uWidth = 1;
  lpPen->crColor = 0xff000000;
  return (GXHPEN)lpPen;
}

GXHPEN GXDLLAPI gxCreatePen(
         int fnPenStyle,  // pen style 
         int nWidth,  // pen width  
         GXCOLORREF crColor   // pen color 
         )
{
  LPGXGDIPEN lpPen;
  lpPen = new GXGDIPEN;
  lpPen->emObjType = GXGDIOBJ_PEN;
  lpPen->uStyle = GXPS_SOLID;
  lpPen->uWidth = 1;
  lpPen->crColor = 0xff000000;
  return (GXHPEN)lpPen;
}
GXBOOL GXDLLAPI gxPolyPolyline(
          GXHDC hdc,  // handle of a device context 
          const GXPOINT *lppt,  // address of an array of points 
          const GXDWORD *lpdwPolyPoints,  // address of an array of values 
          GXDWORD cCount   // number of counts in the second array 
          )
{
  ASSERT(FALSE);
  return NULL;
}
GXBOOL GXDLLAPI gxOffsetWindowOrgEx(
             GXHDC hdc,      // handle to device context 
             int nXOffset,    // horizontal offset 
             int nYOffset,    // vertical offset 
             GXLPPOINT lpPoint   // pointer to structure receiving the original origin  
             )
{
  ASSERT(FALSE);
  return NULL;
}

GXBOOL GXDLLAPI gxInvertRect( GXHDC hdc, const GXRECT* lprc )
{
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  return lpDC->pCanvas->InvertRect(lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
}

GXDWORD GXDLLAPI gxSetLayout( GXHDC hdc, GXDWORD dwLayout )
{
  return 0;
}

GXDWORD GXDLLAPI gxGetLayout( GXHDC hdc )
{
  return 0;
}

#endif // _DEV_DISABLE_UI_CODE