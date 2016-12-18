#ifndef _DEV_DISABLE_UI_CODE
#ifndef IMAGELIST
/*
*  ImageList implementation
*
*  Copyright 1998 Eric Kohl
*  Copyright 2000 Jason Mawdsley
*  Copyright 2001, 2004 Michael Stefaniuc
*  Copyright 2001 Charles Loep for CodeWeavers
*  Copyright 2002 Dimitrie O. Paun
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*
* NOTE
*
* This code was audited for completeness against the documented features
* of Comctl32.dll version 6.0 on Sep. 12, 2002, by Dimitrie O. Paun.
*
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
*
*  TODO:
*    - Add support for GXILD_PRESERVEALPHA, GXILD_SCALE, GXILD_DPISCALE
*    - Add support for ILS_GLOW, ILS_SHADOW, ILS_SATURATE, ILS_ALPHA
*    - Thread-safe locking
*/

#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXImm.h"

#include <stdarg.h>
//#include <string.h>
#include <stdlib.h>


//#include <stdarg.h>
//#include <stdlib.h>
//#include <string.h>

#define COBJMACROS

//#include "winerror.h"
//#include "windef.h"
//#include "winbase.h"
//#include "objbase.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "imagelist.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(imagelist);

#include "GXCommCtrl.h"
#include <GrapX/WineComm.H>
#include "_imagelist.h"

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

#define MAX_OVERLAYIMAGE 15

/* internal image list data used for Drag & Drop operations */
typedef struct
{
  GXHWND  hwnd;
  GXHIMAGELIST  himl;
  /* position of the drag image relative to the window */
  GXINT    x;
  GXINT    y;
  /* offset of the hotspot relative to the origin of the image */
  GXINT    dxHotspot;
  GXINT    dyHotspot;
  /* is the drag image visible */
  GXBOOL  bShow;
  /* saved background */
  GXHBITMAP  hbmBg;
} INTERNALDRAG;

static INTERNALDRAG InternalDrag = { 0, 0, 0, 0, 0, 0, FALSE, 0 };

static GXHBITMAP gxImageList_CreateImage(GXHDC hdc, GXHIMAGELIST himl, GXUINT count, GXUINT width);

static inline GXBOOL is_valid(GXHIMAGELIST himl)
{
#ifdef ENABLE_SEH
  __try{
#endif // ENABLE_SEH

    return himl && himl->magic == IMAGELIST_MAGIC;

#ifdef ENABLE_SEH
  }__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH){
  ASSERT(false);
  return FALSE;
  }
#endif // ENABLE_SEH
}

/*
* An imagelist with N images is tiled like this:
*
*   N/4 ->
*
* 4 048C..
*   159D..
* | 26AE.N
* V 37BF.
*/

#define TILE_COUNT 4

static inline GXUINT imagelist_height( GXUINT count )
{
  return ((count + TILE_COUNT - 1)/TILE_COUNT);
}

static inline void imagelist_point_from_index( GXHIMAGELIST himl, GXUINT index, LPGXPOINT pt )
{
  pt->x = (index%TILE_COUNT) * himl->cx;
  pt->y = (index/TILE_COUNT) * himl->cy;
}

static inline void imagelist_get_bitmap_size( GXHIMAGELIST himl, GXUINT count, GXUINT cx, GXSIZE *sz )
{
  sz->cx = cx * TILE_COUNT;
  sz->cy = imagelist_height( count ) * himl->cy;
}

/*
* imagelist_copy_images()
*
* Copies a block of count images from offset src in the list to offset dest.
* Images are copied a row at at time. Assumes hdcSrc and hdcDest are different.
*/
static inline void imagelist_copy_images( GXHIMAGELIST himl, GXHDC hdcSrc, GXHDC hdcDest,
                     GXUINT src, GXUINT count, GXUINT dest )
{
  GXPOINT ptSrc, ptDest;
  GXSIZE sz;
  GXUINT i;

  for ( i=0; i<TILE_COUNT; i++ )
  {
    imagelist_point_from_index( himl, src+i, &ptSrc );
    imagelist_point_from_index( himl, dest+i, &ptDest );
    sz.cx = himl->cx;
    sz.cy = himl->cy * imagelist_height( count - i );

    gxBitBlt( hdcDest, ptDest.x, ptDest.y, sz.cx, sz.cy,
      hdcSrc, ptSrc.x, ptSrc.y, GXSRCCOPY );
  }
}

/*************************************************************************
* IMAGELIST_InternalExpandBitmaps [Internal]
*
* Expands the bitmaps of an image list by the given number of images.
*
* PARAMS
*     himl        [I] handle to image list
*     nImageCount [I] number of images to add
*
* RETURNS
*     nothing
*
* NOTES
*     This function CANNOT be used to reduce the number of images.
*/
static void
IMAGELIST_InternalExpandBitmaps (GXHIMAGELIST himl, GXINT nImageCount, GXINT cx, GXINT cy)
{
  GXHDC     hdcBitmap;
  GXHBITMAP hbmNewBitmap, hbmNull;
  GXINT     nNewCount;
  GXSIZE    sz;

  if ((himl->cCurImage + nImageCount <= himl->cMaxImage)
    && (himl->cy >= cy))
    return;

  if (cx == 0) cx = himl->cx;
  nNewCount = himl->cCurImage + nImageCount + himl->cGrow;

  imagelist_get_bitmap_size(himl, nNewCount, cx, &sz);

  TRACE("Create expanded bitmaps : himl=%p x=%d y=%d count=%d\n", himl, sz.cx, cy, nNewCount);
  hdcBitmap = gxCreateCompatibleDC (0);

  hbmNewBitmap = gxImageList_CreateImage(hdcBitmap, himl, nNewCount, cx);

  if (hbmNewBitmap == 0) {
    ERR("creating new image bitmap (x=%d y=%d)!\n", sz.cx, cy);
  }

  if (himl->cCurImage)
  {
    hbmNull = (GXHBITMAP)gxSelectObject (hdcBitmap, hbmNewBitmap);
    gxBitBlt (hdcBitmap, 0, 0, sz.cx, sz.cy,
      himl->hdcImage, 0, 0, GXSRCCOPY);
    gxSelectObject (hdcBitmap, hbmNull);
  }
  gxSelectObject (himl->hdcImage, hbmNewBitmap);
  gxDeleteObject (himl->hbmImage);
  himl->hbmImage = hbmNewBitmap;

  if (himl->flags & GXILC_MASK)
  {
    hbmNewBitmap = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);

    if (hbmNewBitmap == 0) {
      ERR("creating new mask bitmap!\n");
    }

    if(himl->cCurImage)
    {
      hbmNull = (GXHBITMAP)gxSelectObject (hdcBitmap, hbmNewBitmap);
      gxBitBlt (hdcBitmap, 0, 0, sz.cx, sz.cy,
        himl->hdcMask, 0, 0, GXSRCCOPY);
      gxSelectObject (hdcBitmap, hbmNull);
    }
    gxSelectObject (himl->hdcMask, hbmNewBitmap);
    gxDeleteObject (himl->hbmMask);
    himl->hbmMask = hbmNewBitmap;
  }

  himl->cMaxImage = nNewCount;

  gxDeleteDC (hdcBitmap);
}


/*************************************************************************
* gxImageList_Add [COMCTL32.@]
*
* Add an image or images to an image list.
*
* PARAMS
*     himl     [I] handle to image list
*     hbmImage [I] handle to image bitmap
*     hbmMask  [I] handle to mask bitmap
*
* RETURNS
*     Success: Index of the first new image.
*     Failure: -1
*/

GXINT GXDLLAPI
gxImageList_Add (GXHIMAGELIST himl,  GXHBITMAP hbmImage, GXHBITMAP hbmMask)
{
  GXHDC     hdcBitmap, hdcTemp;
  GXINT     nFirstIndex, nImageCount, i;
  GXBITMAP  bmp;
  GXHBITMAP hOldBitmap, hOldBitmapTemp;
  GXPOINT   pt;

  TRACE("himl=%p hbmimage=%p hbmmask=%p\n", himl, hbmImage, hbmMask);
  if (!is_valid(himl))
    return -1;

  if (!gxGetObjectW(hbmImage, sizeof(GXBITMAP), (GXLPVOID)&bmp))
    return -1;

  nImageCount = bmp.bmWidth / himl->cx;

  IMAGELIST_InternalExpandBitmaps (himl, nImageCount, bmp.bmWidth, bmp.bmHeight);

  hdcBitmap = gxCreateCompatibleDC(0);

  hOldBitmap = (GXHBITMAP)gxSelectObject(hdcBitmap, hbmImage);

  for (i=0; i<nImageCount; i++)
  {
    imagelist_point_from_index( himl, himl->cCurImage + i, &pt );

    /* Copy result to the imagelist
    */
    gxBitBlt( himl->hdcImage, pt.x, pt.y, himl->cx, bmp.bmHeight,
      hdcBitmap, i*himl->cx, 0, GXSRCCOPY );

    if (!himl->hbmMask)
      continue;

    hdcTemp   = gxCreateCompatibleDC(0);
    hOldBitmapTemp = (GXHBITMAP)gxSelectObject(hdcTemp, hbmMask);

    gxBitBlt( himl->hdcMask, pt.x, pt.y, himl->cx, bmp.bmHeight,
      hdcTemp, i*himl->cx, 0, GXSRCCOPY );

    gxSelectObject(hdcTemp, hOldBitmapTemp);
    gxDeleteDC(hdcTemp);

    /* Remove the background from the image
    */
    gxBitBlt( himl->hdcImage, pt.x, pt.y, himl->cx, bmp.bmHeight,
      himl->hdcMask, pt.x, pt.y, 0x220326 ); /* NOTSRCAND */
  }

  gxSelectObject(hdcBitmap, hOldBitmap);
  gxDeleteDC(hdcBitmap);

  nFirstIndex = himl->cCurImage;
  himl->cCurImage += nImageCount;

  return nFirstIndex;
}


/*************************************************************************
* gxImageList_AddIcon [COMCTL32.@]
*
* Adds an icon to an image list.
*
* PARAMS
*     himl  [I] handle to image list
*     hIcon [I] handle to icon
*
* RETURNS
*     Success: index of the new image
*     Failure: -1
*/
#undef gxImageList_AddIcon
GXINT GXDLLAPI gxImageList_AddIcon (GXHIMAGELIST himl, GXHICON hIcon)
{
  return gxImageList_ReplaceIcon (himl, -1, hIcon);
}


/*************************************************************************
* gxgxImageList_AddMasked [COMCTL32.@]
*
* Adds an image or images to an image list and creates a mask from the
* specified bitmap using the mask color.
*
* PARAMS
*     himl    [I] handle to image list.
*     hBitmap [I] handle to bitmap
*     clrMask [I] mask color.
*
* RETURNS
*     Success: Index of the first new image.
*     Failure: -1
*/

GXINT GXDLLAPI
gxImageList_AddMasked (GXHIMAGELIST himl, GXHBITMAP hBitmap, GXCOLORREF clrMask)
{
  GXHDC    hdcMask, hdcBitmap;
  GXINT    i, nIndex, nImageCount;
  GXBITMAP bmp;
  GXHBITMAP hOldBitmap;
  GXHBITMAP hMaskBitmap=0;
  GXCOLORREF bkColor;
  GXPOINT  pt;

  TRACE("himl=%p hbitmap=%p clrmask=%x\n", himl, hBitmap, clrMask);
  if (!is_valid(himl))
    return -1;

  if (!gxGetObjectW(hBitmap, sizeof(GXBITMAP), &bmp))
    return -1;

  if (himl->cx > 0)
    nImageCount = bmp.bmWidth / himl->cx;
  else
    nImageCount = 0;

  IMAGELIST_InternalExpandBitmaps (himl, nImageCount, bmp.bmWidth, bmp.bmHeight);

  nIndex = himl->cCurImage;
  himl->cCurImage += nImageCount;

  hdcBitmap = gxCreateCompatibleDC(0);
  hOldBitmap = (GXHBITMAP)gxSelectObject(hdcBitmap, hBitmap);

  /* Create a temp Mask so we can remove the background of the Image */
  hdcMask = gxCreateCompatibleDC(0);
  hMaskBitmap = gxCreateBitmap(bmp.bmWidth, bmp.bmHeight, 1, 1, NULL);
  gxSelectObject(hdcMask, hMaskBitmap);

  /* create monochrome image to the mask bitmap */
  bkColor = (clrMask != GXCLR_DEFAULT) ? clrMask : gxGetPixel (hdcBitmap, 0, 0);
  gxSetBkColor (hdcBitmap, bkColor);
  gxBitBlt (hdcMask, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcBitmap, 0, 0, GXSRCCOPY);

  gxSetBkColor(hdcBitmap, GXRGB(255,255,255));

  /*
  * Remove the background from the image
  *
  * WINDOWS BUG ALERT!!!!!!
  *  The statement below should not be done in common practice
  *  but this is how gxImageList_AddMasked works in Windows.
  *  It overwrites the original bitmap passed, this was discovered
  *  by using the same bitmap to iterate the different styles
  *  on windows where it failed (BUT gxImageList_Add is OK)
  *  This is here in case some apps rely on this bug
  *
  *  Blt mode 0x220326 is NOTSRCAND
  */
  gxBitBlt(hdcBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcMask, 0, 0, 0x220326);

  /* Copy result to the imagelist */
  for (i=0; i<nImageCount; i++)
  {
    imagelist_point_from_index( himl, nIndex + i, &pt );
    gxBitBlt(himl->hdcImage, pt.x, pt.y, himl->cx, bmp.bmHeight,
      hdcBitmap, i*himl->cx, 0, GXSRCCOPY);
    gxBitBlt(himl->hdcMask, pt.x, pt.y, himl->cx, bmp.bmHeight,
      hdcMask, i*himl->cx, 0, GXSRCCOPY);
  }

  /* Clean up */
  gxSelectObject(hdcBitmap, hOldBitmap);
  gxDeleteDC(hdcBitmap);
  gxDeleteObject(hMaskBitmap);
  gxDeleteDC(hdcMask);

  return nIndex;
}


/*************************************************************************
* gxImageList_BeginDrag [COMCTL32.@]
*
* Creates a temporary image list that contains one image. It will be used
* as a drag image.
*
* PARAMS
*     himlTrack [I] handle to the source image list
*     iTrack    [I] index of the drag image in the source image list
*     dxHotspot [I] X position of the hot spot of the drag image
*     dyHotspot [I] Y position of the hot spot of the drag image
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_BeginDrag (GXHIMAGELIST himlTrack, GXINT iTrack,
             GXINT dxHotspot, GXINT dyHotspot)
{
  GXINT cx, cy;

  TRACE("(himlTrack=%p iTrack=%d dx=%d dy=%d)\n", himlTrack, iTrack,
    dxHotspot, dyHotspot);

  if (!is_valid(himlTrack))
    return FALSE;

  if (InternalDrag.himl)
    gxImageList_EndDrag ();

  cx = himlTrack->cx;
  cy = himlTrack->cy;

  InternalDrag.himl = gxImageList_Create (cx, cy, himlTrack->flags, 1, 1);
  if (InternalDrag.himl == NULL) {
    WARN("Error creating drag image list!\n");
    return FALSE;
  }

  InternalDrag.dxHotspot = dxHotspot;
  InternalDrag.dyHotspot = dyHotspot;

  /* copy image */
  gxBitBlt (InternalDrag.himl->hdcImage, 0, 0, cx, cy, himlTrack->hdcImage, iTrack * cx, 0, GXSRCCOPY);

  /* copy mask */
  gxBitBlt (InternalDrag.himl->hdcMask, 0, 0, cx, cy, himlTrack->hdcMask, iTrack * cx, 0, GXSRCCOPY);

  InternalDrag.himl->cCurImage = 1;

  return TRUE;
}


/*************************************************************************
* gxImageList_Copy [COMCTL32.@]
*
*  Copies an image of the source image list to an image of the
*  destination image list. Images can be copied or swapped.
*
* PARAMS
*     himlDst [I] handle to the destination image list
*     iDst    [I] destination image index.
*     himlSrc [I] handle to the source image list
*     iSrc    [I] source image index
*     uFlags  [I] flags for the copy operation
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     Copying from one image list to another is possible. The original
*     implementation just copies or swaps within one image list.
*     Could this feature become a bug??? ;-)
*/

GXBOOL GXDLLAPI
gxImageList_Copy (GXHIMAGELIST himlDst, GXINT iDst,  GXHIMAGELIST himlSrc,
          GXINT iSrc, GXUINT uFlags)
{
  GXPOINT ptSrc, ptDst;

  TRACE("himlDst=%p iDst=%d himlSrc=%p iSrc=%d\n", himlDst, iDst, himlSrc, iSrc);

  if (!is_valid(himlSrc) || !is_valid(himlDst))
    return FALSE;
  if ((iDst < 0) || (iDst >= himlDst->cCurImage))
    return FALSE;
  if ((iSrc < 0) || (iSrc >= himlSrc->cCurImage))
    return FALSE;

  imagelist_point_from_index( himlDst, iDst, &ptDst );
  imagelist_point_from_index( himlSrc, iSrc, &ptSrc );

  if (uFlags & GXILCF_SWAP) {
    /* swap */
    GXHDC     hdcBmp;
    GXHBITMAP hbmTempImage, hbmTempMask;

    hdcBmp = gxCreateCompatibleDC (0);

    /* create temporary bitmaps */
    hbmTempImage = gxCreateBitmap (himlSrc->cx, himlSrc->cy, 1,
      himlSrc->uBitsPixel, NULL);
    hbmTempMask = gxCreateBitmap (himlSrc->cx, himlSrc->cy, 1,
      1, NULL);

    /* copy (and stretch) destination to temporary bitmaps.(save) */
    /* image */
    gxSelectObject (hdcBmp, hbmTempImage);
    gxStretchBlt   (hdcBmp, 0, 0, himlSrc->cx, himlSrc->cy,
      himlDst->hdcImage, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      GXSRCCOPY);
    /* mask */
    gxSelectObject (hdcBmp, hbmTempMask);
    gxStretchBlt   (hdcBmp, 0, 0, himlSrc->cx, himlSrc->cy,
      himlDst->hdcMask, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      GXSRCCOPY);

    /* copy (and stretch) source to destination */
    /* image */
    gxStretchBlt   (himlDst->hdcImage, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      himlSrc->hdcImage, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      GXSRCCOPY);
    /* mask */
    gxStretchBlt   (himlDst->hdcMask, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      himlSrc->hdcMask, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      GXSRCCOPY);

    /* copy (without stretching) temporary bitmaps to source (restore) */
    /* mask */
    gxBitBlt       (himlSrc->hdcMask, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      hdcBmp, 0, 0, GXSRCCOPY);

    /* image */
    gxBitBlt       (himlSrc->hdcImage, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      hdcBmp, 0, 0, GXSRCCOPY);
    /* delete temporary bitmaps */
    gxDeleteObject (hbmTempMask);
    gxDeleteObject (hbmTempImage);
    gxDeleteDC(hdcBmp);
  }
  else {
    /* copy image */
    gxStretchBlt   (himlDst->hdcImage, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      himlSrc->hdcImage, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      GXSRCCOPY);

    /* copy mask */
    gxStretchBlt   (himlDst->hdcMask, ptDst.x, ptDst.y, himlDst->cx, himlDst->cy,
      himlSrc->hdcMask, ptSrc.x, ptSrc.y, himlSrc->cx, himlSrc->cy,
      GXSRCCOPY);
  }

  return TRUE;
}


/*************************************************************************
* gxImageList_Create [COMCTL32.@]
*
* Creates a new image list.
*
* PARAMS
*     cx       [I] image height
*     cy       [I] image width
*     flags    [I] creation flags
*     cInitial [I] initial number of images in the image list
*     cGrow    [I] number of images by which image list grows
*
* RETURNS
*     Success: Handle to the created image list
*     Failure: NULL
*/
GXHIMAGELIST GXDLLAPI
gxImageList_Create (GXINT cx, GXINT cy, GXUINT flags,
          GXINT cInitial, GXINT cGrow)
{
  GXHIMAGELIST himl;
  GXINT      nCount;
  GXHBITMAP  hbmTemp;
  GXUINT     ilc = (flags & 0xFE);
  static const GXWORD aBitBlend25[] =
  {0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00};

  static const GXWORD aBitBlend50[] =
  {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

  TRACE("(%d %d 0x%x %d %d)\n", cx, cy, flags, cInitial, cGrow);

  himl = (GXHIMAGELIST)Alloc (sizeof(struct _GXIMAGELIST));
  if (!himl)
    return NULL;

  cGrow = (cGrow < 4) ? 4 : (cGrow + 3) & ~3;

  himl->magic     = IMAGELIST_MAGIC;
  himl->cx        = cx;
  himl->cy        = cy;
  himl->flags     = flags;
  himl->cMaxImage = cInitial + 1;
  himl->cInitial  = cInitial;
  himl->cGrow     = cGrow;
  himl->clrFg     = GXCLR_DEFAULT;
  himl->clrBk     = GXCLR_NONE;

  /* initialize overlay mask indices */
  for (nCount = 0; nCount < MAX_OVERLAYIMAGE; nCount++)
    himl->nOvlIdx[nCount] = -1;

  /* Create Image & Mask DCs */
  himl->hdcImage = gxCreateCompatibleDC (0);
  if (!himl->hdcImage)
    goto cleanup;
  if (himl->flags & GXILC_MASK){
    himl->hdcMask = gxCreateCompatibleDC(0);
    if (!himl->hdcMask)
      goto cleanup;
  }

  /* Default to GXILC_COLOR4 if none of the GXILC_COLOR* flags are specified */
  if (ilc == GXILC_COLOR)
    ilc = GXILC_COLOR4;

  if (ilc >= GXILC_COLOR4 && ilc <= GXILC_COLOR32)
    himl->uBitsPixel = ilc;
  else
    himl->uBitsPixel = (GXUINT)gxGetDeviceCaps (himl->hdcImage, GXBITSPIXEL);

  if (himl->cMaxImage > 0) {
    himl->hbmImage = gxImageList_CreateImage(himl->hdcImage, himl, himl->cMaxImage, cx);
    gxSelectObject(himl->hdcImage, himl->hbmImage);
  } else
    himl->hbmImage = 0;

  if ((himl->cMaxImage > 0) && (himl->flags & GXILC_MASK)) {
    GXSIZE sz;

    imagelist_get_bitmap_size(himl, himl->cMaxImage, himl->cx, &sz);
    himl->hbmMask = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);
    if (himl->hbmMask == 0) {
      ERR("Error creating mask bitmap!\n");
      goto cleanup;
    }
    gxSelectObject(himl->hdcMask, himl->hbmMask);
  }
  else
    himl->hbmMask = 0;

  /* create blending brushes */
  hbmTemp = gxCreateBitmap (8, 8, 1, 1, aBitBlend25);
  himl->hbrBlend25 = gxCreatePatternBrush (hbmTemp);
  gxDeleteObject (hbmTemp);

  hbmTemp = gxCreateBitmap (8, 8, 1, 1, aBitBlend50);
  himl->hbrBlend50 = gxCreatePatternBrush (hbmTemp);
  gxDeleteObject (hbmTemp);

  TRACE("created imagelist %p\n", himl);
  return himl;

cleanup:
  if (himl) gxImageList_Destroy(himl);
  return NULL;
}


/*************************************************************************
* gxImageList_Destroy [COMCTL32.@]
*
* Destroys an image list.
*
* PARAMS
*     himl [I] handle to image list
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_Destroy (GXHIMAGELIST himl)
{
  if (!is_valid(himl))
    return FALSE;

  /* delete image bitmaps */
  if (himl->hbmImage)
    gxDeleteObject (himl->hbmImage);
  if (himl->hbmMask)
    gxDeleteObject (himl->hbmMask);

  /* delete image & mask DCs */
  if (himl->hdcImage)
    gxDeleteDC(himl->hdcImage);
  if (himl->hdcMask)
    gxDeleteDC(himl->hdcMask);

  /* delete blending brushes */
  if (himl->hbrBlend25)
    gxDeleteObject (himl->hbrBlend25);
  if (himl->hbrBlend50)
    gxDeleteObject (himl->hbrBlend50);

  ZeroMemory(himl, sizeof(*himl));
  Free (himl);

  return TRUE;
}


/*************************************************************************
* gxImageList_DragEnter [COMCTL32.@]
*
* Locks window update and displays the drag image at the given position.
*
* PARAMS
*     hwndLock [I] handle of the window that owns the drag image.
*     x        [I] X position of the drag image.
*     y        [I] Y position of the drag image.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     The position of the drag image is relative to the window, not
*     the client area.
*/

GXBOOL GXDLLAPI
gxImageList_DragEnter (GXHWND hwndLock, GXINT x, GXINT y)
{
  TRACE("(hwnd=%p x=%d y=%d)\n", hwndLock, x, y);

  if (!is_valid(InternalDrag.himl))
    return FALSE;

  if (hwndLock)
    InternalDrag.hwnd = hwndLock;
  else
    InternalDrag.hwnd = gxGetDesktopWindow ();

  InternalDrag.x = x;
  InternalDrag.y = y;

  /* draw the drag image and save the background */
  if (!gxImageList_DragShowNolock(TRUE)) {
    return FALSE;
  }

  return TRUE;
}


/*************************************************************************
* gxImageList_DragLeave [COMCTL32.@]
*
* Unlocks window update and hides the drag image.
*
* PARAMS
*     hwndLock [I] handle of the window that owns the drag image.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_DragLeave (GXHWND hwndLock)
{
  /* As we don't save drag info in the window this can lead to problems if
  an app does not supply the same window as DragEnter */
  /* if (hwndLock)
  InternalDrag.hwnd = hwndLock;
  else
  InternalDrag.hwnd = GetDesktopWindow (); */
  if(!hwndLock)
    hwndLock = gxGetDesktopWindow();
  if(InternalDrag.hwnd != hwndLock) {
    FIXME("DragLeave hWnd != DragEnter hWnd\n");
  }

  gxImageList_DragShowNolock (FALSE);

  return TRUE;
}


/*************************************************************************
* gxImageList_InternalDragDraw [Internal]
*
* Draws the drag image.
*
* PARAMS
*     hdc [I] device context to draw into.
*     x   [I] X position of the drag image.
*     y   [I] Y position of the drag image.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     The position of the drag image is relative to the window, not
*     the client area.
*
*/

static inline void
gxImageList_InternalDragDraw (GXHDC hdc, GXINT x, GXINT y)
{
  GXIMAGELISTDRAWPARAMS imldp;

  ZeroMemory (&imldp, sizeof(imldp));
  imldp.cbSize  = sizeof(imldp);
  imldp.himl    = InternalDrag.himl;
  imldp.i       = 0;
  imldp.hdcDst  = hdc,
    imldp.x       = x;
  imldp.y       = y;
  imldp.rgbBk   = GXCLR_DEFAULT;
  imldp.rgbFg   = GXCLR_DEFAULT;
  imldp.fStyle  = GXILD_NORMAL;
  imldp.fState  = GXILS_ALPHA;
  imldp.Frame   = 128;

  /* FIXME: instead of using the alpha blending, we should
  * create a 50% mask, and draw it semitransparantly that way */
  gxImageList_DrawIndirect (&imldp);
}

/*************************************************************************
* gxImageList_DragMove [COMCTL32.@]
*
* Moves the drag image.
*
* PARAMS
*     x [I] X position of the drag image.
*     y [I] Y position of the drag image.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     The position of the drag image is relative to the window, not
*     the client area.
*
* BUGS
*     The drag image should be drawn semitransparent.
*/

GXBOOL GXDLLAPI
gxImageList_DragMove (GXINT x, GXINT y)
{
  TRACE("(x=%d y=%d)\n", x, y);

  if (!is_valid(InternalDrag.himl))
    return FALSE;

  /* draw/update the drag image */
  if (InternalDrag.bShow) {
    GXHDC hdcDrag;
    GXHDC hdcOffScreen;
    GXHDC hdcBg;
    GXHBITMAP hbmOffScreen;
    GXINT origNewX, origNewY;
    GXINT origOldX, origOldY;
    GXINT origRegX, origRegY;
    GXINT sizeRegX, sizeRegY;


    /* calculate the update region */
    origNewX = x - InternalDrag.dxHotspot;
    origNewY = y - InternalDrag.dyHotspot;
    origOldX = InternalDrag.x - InternalDrag.dxHotspot;
    origOldY = InternalDrag.y - InternalDrag.dyHotspot;
    origRegX = min(origNewX, origOldX);
    origRegY = min(origNewY, origOldY);
    sizeRegX = InternalDrag.himl->cx + abs(x - InternalDrag.x);
    sizeRegY = InternalDrag.himl->cy + abs(y - InternalDrag.y);

    hdcDrag = gxGetDCEx(InternalDrag.hwnd, 0,
      GXDCX_WINDOW | GXDCX_CACHE | GXDCX_LOCKWINDOWUPDATE);
    hdcOffScreen = gxCreateCompatibleDC(hdcDrag);
    hdcBg = gxCreateCompatibleDC(hdcDrag);

    hbmOffScreen = gxCreateCompatibleBitmap(hdcDrag, sizeRegX, sizeRegY);
    gxSelectObject(hdcOffScreen, hbmOffScreen);
    gxSelectObject(hdcBg, InternalDrag.hbmBg);

    /* get the actual background of the update region */
    gxBitBlt(hdcOffScreen, 0, 0, sizeRegX, sizeRegY, hdcDrag,
      origRegX, origRegY, GXSRCCOPY);
    /* erase the old image */
    gxBitBlt(hdcOffScreen, origOldX - origRegX, origOldY - origRegY,
      InternalDrag.himl->cx, InternalDrag.himl->cy, hdcBg, 0, 0,
      GXSRCCOPY);
    /* save the background */
    gxBitBlt(hdcBg, 0, 0, InternalDrag.himl->cx, InternalDrag.himl->cy,
      hdcOffScreen, origNewX - origRegX, origNewY - origRegY, GXSRCCOPY);
    /* draw the image */
    gxImageList_InternalDragDraw(hdcOffScreen, origNewX - origRegX, 
      origNewY - origRegY);
    /* draw the update region to the screen */
    gxBitBlt(hdcDrag, origRegX, origRegY, sizeRegX, sizeRegY,
      hdcOffScreen, 0, 0, GXSRCCOPY);

    gxDeleteDC(hdcBg);
    gxDeleteDC(hdcOffScreen);
    gxDeleteObject(hbmOffScreen);
    gxReleaseDC(InternalDrag.hwnd, hdcDrag);
  }

  /* update the image position */
  InternalDrag.x = x;
  InternalDrag.y = y;

  return TRUE;
}


/*************************************************************************
* gxImageList_DragShowNolock [COMCTL32.@]
*
* Shows or hides the drag image.
*
* PARAMS
*     bShow [I] TRUE shows the drag image, FALSE hides it.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* BUGS
*     The drag image should be drawn semitransparent.
*/

GXBOOL GXDLLAPI
gxImageList_DragShowNolock (GXBOOL bShow)
{
  GXHDC hdcDrag;
  GXHDC hdcBg;
  GXINT x, y;

  if (!is_valid(InternalDrag.himl))
    return FALSE;

  TRACE("bShow=0x%X!\n", bShow);

  /* DragImage is already visible/hidden */
  if ((InternalDrag.bShow && bShow) || (!InternalDrag.bShow && !bShow)) {
    return FALSE;
  }

  /* position of the origin of the DragImage */
  x = InternalDrag.x - InternalDrag.dxHotspot;
  y = InternalDrag.y - InternalDrag.dyHotspot;

  hdcDrag = gxGetDCEx (InternalDrag.hwnd, 0,
    GXDCX_WINDOW | GXDCX_CACHE | GXDCX_LOCKWINDOWUPDATE);
  if (!hdcDrag) {
    return FALSE;
  }

  hdcBg = gxCreateCompatibleDC(hdcDrag);
  if (!InternalDrag.hbmBg) {
    InternalDrag.hbmBg = gxCreateCompatibleBitmap(hdcDrag,
      InternalDrag.himl->cx, InternalDrag.himl->cy);
  }
  gxSelectObject(hdcBg, InternalDrag.hbmBg);

  if (bShow) {
    /* save the background */
    gxBitBlt(hdcBg, 0, 0, InternalDrag.himl->cx, InternalDrag.himl->cy,
      hdcDrag, x, y, GXSRCCOPY);
    /* show the image */
    gxImageList_InternalDragDraw(hdcDrag, x, y);
  } else {
    /* hide the image */
    gxBitBlt(hdcDrag, x, y, InternalDrag.himl->cx, InternalDrag.himl->cy,
      hdcBg, 0, 0, GXSRCCOPY);
  }

  InternalDrag.bShow = !InternalDrag.bShow;

  gxDeleteDC(hdcBg);
  gxReleaseDC (InternalDrag.hwnd, hdcDrag);
  return TRUE;
}


/*************************************************************************
* gxImageList_Draw [COMCTL32.@]
*
* Draws an image.
*
* PARAMS
*     himl   [I] handle to image list
*     i      [I] image index
*     hdc    [I] handle to device context
*     x      [I] x position
*     y      [I] y position
*     fStyle [I] drawing flags
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* SEE
*     gxImageList_DrawEx.
*/

GXBOOL GXDLLAPI
gxImageList_Draw (GXHIMAGELIST himl, GXINT i, GXHDC hdc, GXINT x, GXINT y, GXUINT fStyle)
{
  return gxImageList_DrawEx (himl, i, hdc, x, y, 0, 0, 
    GXCLR_DEFAULT, GXCLR_DEFAULT, fStyle);
}


/*************************************************************************
* gxImageList_DrawEx [COMCTL32.@]
*
* Draws an image and allows to use extended drawing features.
*
* PARAMS
*     himl   [I] handle to image list
*     i      [I] image index
*     hdc    [I] handle to device context
*     x      [I] X position
*     y      [I] Y position
*     dx     [I] X offset
*     dy     [I] Y offset
*     rgbBk  [I] background color
*     rgbFg  [I] foreground color
*     fStyle [I] drawing flags
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     Calls gxImageList_DrawIndirect.
*
* SEE
*     gxImageList_DrawIndirect.
*/

GXBOOL GXDLLAPI
gxImageList_DrawEx (GXHIMAGELIST himl, GXINT i, GXHDC hdc, GXINT x, GXINT y,
          GXINT dx, GXINT dy, GXCOLORREF rgbBk, GXCOLORREF rgbFg,
          GXUINT fStyle)
{
  GXIMAGELISTDRAWPARAMS imldp;

  ZeroMemory (&imldp, sizeof(imldp));
  imldp.cbSize  = sizeof(imldp);
  imldp.himl    = himl;
  imldp.i       = i;
  imldp.hdcDst  = hdc,
    imldp.x       = x;
  imldp.y       = y;
  imldp.cx      = dx;
  imldp.cy      = dy;
  imldp.rgbBk   = rgbBk;
  imldp.rgbFg   = rgbFg;
  imldp.fStyle  = fStyle;

  return gxImageList_DrawIndirect (&imldp);
}


/*************************************************************************
* gxImageList_DrawIndirect [COMCTL32.@]
*
* Draws an image using various parameters specified in pimldp.
*
* PARAMS
*     pimldp [I] pointer to GXIMAGELISTDRAWPARAMS structure.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_DrawIndirect (GXIMAGELISTDRAWPARAMS *pimldp)
{
  GXINT cx, cy, nOvlIdx;
  GXDWORD fState, dwRop;
  GXUINT fStyle;
  GXCOLORREF oldImageBk, oldImageFg;
  GXHDC hImageDC, hImageListDC, hMaskListDC;
  GXHBITMAP hImageBmp, hOldImageBmp, hBlendMaskBmp;
  GXBOOL bIsTransparent, bBlend, bResult = FALSE, bMask;
  GXHIMAGELIST himl;
  GXPOINT pt;

  if (!pimldp || !(himl = pimldp->himl)) return FALSE;
  if (!is_valid(himl)) return FALSE;
  if ((pimldp->i < 0) || (pimldp->i >= himl->cCurImage)) return FALSE;

  imagelist_point_from_index( himl, pimldp->i, &pt );
  pt.x += pimldp->xBitmap;
  pt.y += pimldp->yBitmap;

  fState = pimldp->cbSize < sizeof(GXIMAGELISTDRAWPARAMS) ? GXILS_NORMAL : pimldp->fState;
  fStyle = pimldp->fStyle & ~GXILD_OVERLAYMASK;
  cx = (pimldp->cx == 0) ? himl->cx : pimldp->cx;
  cy = (pimldp->cy == 0) ? himl->cy : pimldp->cy;

  bIsTransparent = (fStyle & GXILD_TRANSPARENT);
  if( pimldp->rgbBk == GXCLR_NONE )
    bIsTransparent = TRUE;
  if( ( pimldp->rgbBk == GXCLR_DEFAULT ) && ( himl->clrBk == GXCLR_NONE ) )
    bIsTransparent = TRUE;
  bMask = (himl->flags & GXILC_MASK) && (fStyle & GXILD_MASK) ;
  bBlend = (fStyle & (GXILD_BLEND25 | GXILD_BLEND50) ) && !bMask;

  TRACE("himl(%p) hbmMask(%p) iImage(%d) x(%d) y(%d) cx(%d) cy(%d)\n",
    himl, himl->hbmMask, pimldp->i, pimldp->x, pimldp->y, cx, cy);

  /* we will use these DCs to access the images and masks in the ImageList */
  hImageListDC = himl->hdcImage;
  hMaskListDC  = himl->hdcMask;

  /* these will accumulate the image and mask for the image we're drawing */
  hImageDC = gxCreateCompatibleDC( pimldp->hdcDst );
  hImageBmp = gxCreateCompatibleBitmap( pimldp->hdcDst, cx, cy );
  hBlendMaskBmp = bBlend ? gxCreateBitmap(cx, cy, 1, 1, NULL) : 0;

  /* Create a compatible DC. */
  if (!hImageListDC || !hImageDC || !hImageBmp ||
    (bBlend && !hBlendMaskBmp) || (himl->hbmMask && !hMaskListDC))
    goto cleanup;

  hOldImageBmp = (GXHBITMAP)gxSelectObject(hImageDC, hImageBmp);

  /*
  * To obtain a transparent look, background color should be set
  * to white and foreground color to black when blting the
  * monochrome mask.
  */
  oldImageFg = gxSetTextColor( hImageDC, GXRGB( 0, 0, 0 ) );
  oldImageBk = gxSetBkColor( hImageDC, GXRGB( 0xff, 0xff, 0xff ) );

  /*
  * Draw the initial image
  */
  if( bMask ) {
    if (himl->hbmMask) {
      GXHBRUSH hOldBrush;
      hOldBrush = (GXHBRUSH)gxSelectObject (hImageDC, gxCreateSolidBrush (gxGetTextColor(pimldp->hdcDst)));
      gxPatBlt( hImageDC, 0, 0, cx, cy, GXPATCOPY );
      gxBitBlt(hImageDC, 0, 0, cx, cy, hMaskListDC, pt.x, pt.y, GXSRCPAINT);
      gxDeleteObject (gxSelectObject (hImageDC, hOldBrush));
      if( bIsTransparent )
      {
        gxBitBlt ( pimldp->hdcDst, pimldp->x,  pimldp->y, cx, cy, hImageDC, 0, 0, GXPATCOPY);
        bResult = TRUE;
        goto end;
      }
    } else {
      GXHBRUSH hOldBrush = (GXHBRUSH)gxSelectObject (hImageDC, gxGetStockObject(GXBLACK_BRUSH));
      gxPatBlt( hImageDC, 0, 0, cx, cy, GXPATCOPY);
      gxSelectObject(hImageDC, hOldBrush);
    }
  } else {
    /* blend the image with the needed solid background */
    GXCOLORREF colour = GXRGB(0,0,0);
    GXHBRUSH hOldBrush;

    if( !bIsTransparent )
    {
      colour = pimldp->rgbBk;
      if( colour == GXCLR_DEFAULT )
        colour = himl->clrBk;
      if( colour == GXCLR_NONE )
        colour = gxGetBkColor(pimldp->hdcDst);
    }

    hOldBrush = (GXHBRUSH)gxSelectObject (hImageDC, gxCreateSolidBrush (colour));
    gxPatBlt( hImageDC, 0, 0, cx, cy, GXPATCOPY );
    if (himl->hbmMask)
    {
      gxBitBlt( hImageDC, 0, 0, cx, cy, hMaskListDC, pt.x, pt.y, GXPATCOPY );
      gxBitBlt( hImageDC, 0, 0, cx, cy, hImageListDC, pt.x, pt.y, GXSRCPAINT );
    }
    else
      gxBitBlt( hImageDC, 0, 0, cx, cy, hImageListDC, pt.x, pt.y, GXSRCCOPY);
    gxDeleteObject (gxSelectObject (hImageDC, hOldBrush));
  }

  /* Time for blending, if required */
  if (bBlend) {
    GXHBRUSH hBlendBrush, hOldBrush;
    GXCOLORREF clrBlend = pimldp->rgbFg;
    GXHDC hBlendMaskDC = hImageListDC;
    GXHBITMAP hOldBitmap;

    /* Create the blend Mask */
    hOldBitmap = (GXHBITMAP)gxSelectObject(hBlendMaskDC, hBlendMaskBmp);
    hBlendBrush = fStyle & GXILD_BLEND50 ? himl->hbrBlend50 : himl->hbrBlend25;
    hOldBrush = (GXHBRUSH) gxSelectObject(hBlendMaskDC, hBlendBrush);
    gxPatBlt(hBlendMaskDC, 0, 0, cx, cy, GXPATCOPY);
    gxSelectObject(hBlendMaskDC, hOldBrush);

    /* Modify the blend mask if an Image Mask exist */
    if(himl->hbmMask) {
      gxBitBlt(hBlendMaskDC, 0, 0, cx, cy, hMaskListDC, pt.x, pt.y, 0x220326); /* NOTSRCAND */
      gxBitBlt(hBlendMaskDC, 0, 0, cx, cy, hBlendMaskDC, 0, 0, GXNOTSRCCOPY);
    }

    /* now apply blend to the current image given the BlendMask */
    if (clrBlend == GXCLR_DEFAULT) clrBlend = gxGetSysColor (GXCOLOR_HIGHLIGHT);
    else if (clrBlend == GXCLR_NONE) clrBlend = gxGetTextColor (pimldp->hdcDst);
    hOldBrush = (GXHBRUSH) gxSelectObject (hImageDC, gxCreateSolidBrush(clrBlend));
    //gxBitBlt (hImageDC, 0, 0, cx, cy, hBlendMaskDC, 0, 0, 0xB8074A); /* PSDPxax */
    //gxBitBlt (hImageDC, 0, 0, cx, cy, hBlendMaskDC, 0, 0, GXSRCCOPY);
    gxPatBlt(hImageDC, 0, 0, cx, cy, GXPATCOPY);
    gxDeleteObject(gxSelectObject(hImageDC, hOldBrush));
    gxSelectObject(hBlendMaskDC, hOldBitmap);
  }

  /* Now do the overlay image, if any */
  nOvlIdx = (pimldp->fStyle & GXILD_OVERLAYMASK) >> 8;
  if ( (nOvlIdx >= 1) && (nOvlIdx <= MAX_OVERLAYIMAGE)) {
    nOvlIdx = himl->nOvlIdx[nOvlIdx - 1];
    if ((nOvlIdx >= 0) && (nOvlIdx < himl->cCurImage)) {
      GXPOINT ptOvl;
      imagelist_point_from_index( himl, nOvlIdx, &ptOvl );
      ptOvl.x += pimldp->xBitmap;
      if (himl->hbmMask && !(fStyle & GXILD_IMAGE))
        gxBitBlt (hImageDC, 0, 0, cx, cy, hMaskListDC, ptOvl.x, ptOvl.y, GXPATCOPY);
      gxBitBlt (hImageDC, 0, 0, cx, cy, hImageListDC, ptOvl.x, ptOvl.y, GXSRCPAINT);
    }
  }

  if (fState & GXILS_SATURATE) { FIXME("ILS_SATURATE: unimplemented!\n"); }
  if (fState & GXILS_GLOW) { FIXME("ILS_GLOW: unimplemented!\n"); }
  if (fState & GXILS_SHADOW) { FIXME("ILS_SHADOW: unimplemented!\n"); }
  if (fState & GXILS_ALPHA) { FIXME("ILS_ALPHA: unimplemented!\n"); }

  if (fStyle & GXILD_PRESERVEALPHA) { FIXME("GXILD_PRESERVEALPHA: unimplemented!\n"); }
  if (fStyle & GXILD_SCALE) { FIXME("GXILD_SCALE: unimplemented!\n"); }
  if (fStyle & GXILD_DPISCALE) { FIXME("GXILD_DPISCALE: unimplemented!\n"); }

  /* now copy the image to the screen */
  dwRop = GXSRCCOPY;
  if (himl->hbmMask && bIsTransparent ) {
    GXCOLORREF oldDstFg = gxSetTextColor(pimldp->hdcDst, GXRGB( 0, 0, 0 ) );
    GXCOLORREF oldDstBk = gxSetBkColor(pimldp->hdcDst, GXRGB( 0xff, 0xff, 0xff ));
    gxBitBlt (pimldp->hdcDst, pimldp->x,  pimldp->y, cx, cy, hMaskListDC, pt.x, pt.y, GXPATCOPY);
    gxSetBkColor(pimldp->hdcDst, oldDstBk);
    gxSetTextColor(pimldp->hdcDst, oldDstFg);
    dwRop = GXSRCPAINT;
  }
  if (fStyle & GXILD_ROP) dwRop = pimldp->dwRop;
  gxBitBlt (pimldp->hdcDst, pimldp->x,  pimldp->y, cx, cy, hImageDC, 0, 0, dwRop);

  bResult = TRUE;
end:
  /* cleanup the mess */
  gxSetBkColor(hImageDC, oldImageBk);
  gxSetTextColor(hImageDC, oldImageFg);
  gxSelectObject(hImageDC, hOldImageBmp);
cleanup:
  gxDeleteObject(hBlendMaskBmp);
  gxDeleteObject(hImageBmp);
  gxDeleteDC(hImageDC);

  return bResult;
}


/*************************************************************************
* gxImageList_Duplicate [COMCTL32.@]
*
* Duplicates an image list.
*
* PARAMS
*     himlSrc [I] source image list handle
*
* RETURNS
*     Success: Handle of duplicated image list.
*     Failure: NULL
*/

GXHIMAGELIST GXDLLAPI
gxImageList_Duplicate (GXHIMAGELIST himlSrc)
{
  GXHIMAGELIST himlDst;

  if (!is_valid(himlSrc)) {
    ERR("Invalid image list handle!\n");
    return NULL;
  }

  himlDst = gxImageList_Create (himlSrc->cx, himlSrc->cy, himlSrc->flags,
    himlSrc->cInitial, himlSrc->cGrow);

  if (himlDst)
  {
    GXSIZE sz;

    imagelist_get_bitmap_size(himlSrc, himlSrc->cCurImage, himlSrc->cx, &sz);
    gxBitBlt (himlDst->hdcImage, 0, 0, sz.cx, sz.cy,
      himlSrc->hdcImage, 0, 0, GXSRCCOPY);

    if (himlDst->hbmMask)
      gxBitBlt (himlDst->hdcMask, 0, 0, sz.cx, sz.cy,
      himlSrc->hdcMask, 0, 0, GXSRCCOPY);

    himlDst->cCurImage = himlSrc->cCurImage;
    himlDst->cMaxImage = himlSrc->cMaxImage;
  }
  return himlDst;
}


/*************************************************************************
* gxImageList_EndDrag [COMCTL32.@]
*
* Finishes a drag operation.
*
* PARAMS
*     no Parameters
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXVOID GXDLLAPI
gxImageList_EndDrag ()
{
  /* cleanup the InternalDrag struct */
  InternalDrag.hwnd = 0;
  gxImageList_Destroy (InternalDrag.himl);
  InternalDrag.himl = 0;
  InternalDrag.x= 0;
  InternalDrag.y= 0;
  InternalDrag.dxHotspot = 0;
  InternalDrag.dyHotspot = 0;
  InternalDrag.bShow = FALSE;
  gxDeleteObject(InternalDrag.hbmBg);
  InternalDrag.hbmBg = 0;
}


/*************************************************************************
* gxImageList_GetBkColor [COMCTL32.@]
*
* Returns the background color of an image list.
*
* PARAMS
*     himl [I] Image list handle.
*
* RETURNS
*     Success: background color
*     Failure: GXCLR_NONE
*/

GXCOLORREF GXDLLAPI
gxImageList_GetBkColor (GXHIMAGELIST himl)
{
  return himl ? himl->clrBk : GXCLR_NONE;
}


/*************************************************************************
* gxImageList_GetDragImage [COMCTL32.@]
*
* Returns the handle to the internal drag image list.
*
* PARAMS
*     ppt        [O] Pointer to the drag position. Can be NULL.
*     pptHotspot [O] Pointer to the position of the hot spot. Can be NULL.
*
* RETURNS
*     Success: Handle of the drag image list.
*     Failure: NULL.
*/

GXHIMAGELIST GXDLLAPI
gxImageList_GetDragImage (GXPOINT *ppt, GXPOINT *pptHotspot)
{
  if (is_valid(InternalDrag.himl)) {
    if (ppt) {
      ppt->x = InternalDrag.x;
      ppt->y = InternalDrag.y;
    }
    if (pptHotspot) {
      pptHotspot->x = InternalDrag.dxHotspot;
      pptHotspot->y = InternalDrag.dyHotspot;
    }
    return (InternalDrag.himl);
  }

  return NULL;
}


/*************************************************************************
* gxImageList_GetFlags [COMCTL32.@]
*
* Gets the flags of the specified image list.
*
* PARAMS
*     himl [I] Handle to image list
*
* RETURNS
*     Image list flags.
*
* BUGS
*    Stub.
*/

GXDWORD GXDLLAPI
gxImageList_GetFlags(GXHIMAGELIST himl)
{
  TRACE("%p\n", himl);

  return is_valid(himl) ? himl->flags : 0;
}


/*************************************************************************
* gxImageList_GetIcon [COMCTL32.@]
*
* Creates an icon from a masked image of an image list.
*
* PARAMS
*     himl  [I] handle to image list
*     i     [I] image index
*     flags [I] drawing style flags
*
* RETURNS
*     Success: icon handle
*     Failure: NULL
*/

GXHICON GXDLLAPI
gxImageList_GetIcon (GXHIMAGELIST himl, GXINT i, GXUINT fStyle)
{
  GXICONINFO ii;
  GXHICON hIcon;
  GXHBITMAP hOldDstBitmap;
  GXHDC hdcDst;
  GXPOINT pt;

  TRACE("%p %d %d\n", himl, i, fStyle);
  if (!is_valid(himl) || (i < 0) || (i >= himl->cCurImage)) return NULL;

  ii.fIcon = TRUE;
  ii.xHotspot = 0;
  ii.yHotspot = 0;

  /* create colour bitmap */
  hdcDst = gxGetDC(0);
  ii.hbmColor = gxCreateCompatibleBitmap(hdcDst, himl->cx, himl->cy);
  gxReleaseDC(0, hdcDst);

  hdcDst = gxCreateCompatibleDC(0);

  imagelist_point_from_index( himl, i, &pt );

  /* draw mask*/
  ii.hbmMask  = gxCreateBitmap (himl->cx, himl->cy, 1, 1, NULL);
  hOldDstBitmap = (GXHBITMAP)gxSelectObject (hdcDst, ii.hbmMask);
  if (himl->hbmMask) {
    gxBitBlt (hdcDst, 0, 0, himl->cx, himl->cy,
      himl->hdcMask, pt.x, pt.y, GXSRCCOPY);
  }
  else
    gxPatBlt (hdcDst, 0, 0, himl->cx, himl->cy, GXBLACKNESS);

  /* draw image*/
  gxSelectObject (hdcDst, ii.hbmColor);
  gxBitBlt (hdcDst, 0, 0, himl->cx, himl->cy,
    himl->hdcImage, pt.x, pt.y, GXSRCCOPY);

  /*
  * CreateIconIndirect requires us to deselect the bitmaps from
  * the DCs before calling
  */
  gxSelectObject(hdcDst, hOldDstBitmap);

  hIcon = gxCreateIconIndirect (&ii);

  gxDeleteObject (ii.hbmMask);
  gxDeleteObject (ii.hbmColor);
  gxDeleteDC (hdcDst);

  return hIcon;
}


/*************************************************************************
* gxImageList_GetIconSize [COMCTL32.@]
*
* Retrieves the size of an image in an image list.
*
* PARAMS
*     himl [I] handle to image list
*     cx   [O] pointer to the image width.
*     cy   [O] pointer to the image height.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*     All images in an image list have the same size.
*/

GXBOOL GXDLLAPI
gxImageList_GetIconSize (GXHIMAGELIST himl, GXINT *cx, GXINT *cy)
{
  if (!is_valid(himl))
    return FALSE;
  if ((himl->cx <= 0) || (himl->cy <= 0))
    return FALSE;

  if (cx)
    *cx = himl->cx;
  if (cy)
    *cy = himl->cy;

  return TRUE;
}


/*************************************************************************
* gxImageList_GetImageCount [COMCTL32.@]
*
* Returns the number of images in an image list.
*
* PARAMS
*     himl [I] handle to image list
*
* RETURNS
*     Success: Number of images.
*     Failure: 0
*/

GXINT GXDLLAPI
gxImageList_GetImageCount (GXHIMAGELIST himl)
{
  if (!is_valid(himl))
    return 0;

  return himl->cCurImage;
}


/*************************************************************************
* gxImageList_GetImageInfo [COMCTL32.@]
*
* Returns information about an image in an image list.
*
* PARAMS
*     himl       [I] handle to image list
*     i          [I] image index
*     pImageInfo [O] pointer to the image information
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_GetImageInfo (GXHIMAGELIST himl, GXINT i, GXIMAGEINFO *pImageInfo)
{
  GXPOINT pt;

  if (!is_valid(himl) || (pImageInfo == NULL))
    return FALSE;
  if ((i < 0) || (i >= himl->cCurImage))
    return FALSE;

  pImageInfo->hbmImage = himl->hbmImage;
  pImageInfo->hbmMask  = himl->hbmMask;

  imagelist_point_from_index( himl, i, &pt );
  pImageInfo->rcImage.top    = pt.y;
  pImageInfo->rcImage.bottom = pt.y + himl->cy;
  pImageInfo->rcImage.left   = pt.x;
  pImageInfo->rcImage.right  = pt.x + himl->cx;

  return TRUE;
}


/*************************************************************************
* gxImageList_GetImageRect [COMCTL32.@]
*
* Retrieves the rectangle of the specified image in an image list.
*
* PARAMS
*     himl   [I] handle to image list
*     i      [I] image index
*     lpRect [O] pointer to the image rectangle
*
* RETURNS
*    Success: TRUE
*    Failure: FALSE
*
* NOTES
*    This is an UNDOCUMENTED function!!!
*/

GXBOOL GXDLLAPI
gxImageList_GetImageRect (GXHIMAGELIST himl, GXINT i, GXLPRECT lpRect)
{
  GXPOINT pt;

  if (!is_valid(himl) || (lpRect == NULL))
    return FALSE;
  if ((i < 0) || (i >= himl->cCurImage))
    return FALSE;

  imagelist_point_from_index( himl, i, &pt );
  lpRect->left   = pt.x;
  lpRect->top    = pt.y;
  lpRect->right  = pt.x + himl->cx;
  lpRect->bottom = pt.y + himl->cy;

  return TRUE;
}


/*************************************************************************
* gxImageList_LoadImage  [COMCTL32.@]
* gxImageList_LoadImageA [COMCTL32.@]
*
* Creates an image list from a bitmap, icon or cursor.
*
* See gxImageList_LoadImageW.
*/

GXHIMAGELIST GXDLLAPI
gxImageList_LoadImageA (GXHINSTANCE hi, GXLPCSTR lpbmp, GXINT cx, GXINT cGrow,
            GXCOLORREF clrMask, GXUINT uType, GXUINT uFlags)
{
  GXHIMAGELIST himl;
  GXLPWSTR lpbmpW;
  GXDWORD len;

  if (!GXHIWORD(lpbmp))
    return gxImageList_LoadImageW(hi, (GXLPCWSTR)lpbmp, cx, cGrow, clrMask,
    uType, uFlags);

  len = gxMultiByteToWideChar(GXCP_ACP, 0, lpbmp, -1, NULL, 0);
  lpbmpW = (GXLPWSTR)Alloc(len * sizeof(GXWCHAR));
  gxMultiByteToWideChar(GXCP_ACP, 0, lpbmp, -1, lpbmpW, len);

  himl = gxImageList_LoadImageW(hi, lpbmpW, cx, cGrow, clrMask, uType, uFlags);
  Free (lpbmpW);
  return himl;
}


/*************************************************************************
* gxImageList_LoadImageW [COMCTL32.@]
*
* Creates an image list from a bitmap, icon or cursor.
*
* PARAMS
*     hi      [I] instance handle
*     lpbmp   [I] name or id of the image
*     cx      [I] width of each image
*     cGrow   [I] number of images to expand
*     clrMask [I] mask color
*     uType   [I] type of image to load
*     uFlags  [I] loading flags
*
* RETURNS
*     Success: handle to the loaded image list
*     Failure: NULL
*
* SEE
*     gxLoadImage ()
*/

GXHIMAGELIST GXDLLAPI
gxImageList_LoadImageW (GXHINSTANCE hi, GXLPCWSTR lpbmp, GXINT cx, GXINT cGrow,
            GXCOLORREF clrMask, GXUINT uType, GXUINT uFlags)
{
  GXHIMAGELIST himl = NULL;
  GXHANDLE   handle;
  GXINT      nImageCount;

  handle = gxLoadImageW ((GXHINSTANCE)hi, lpbmp, uType, 0, 0, uFlags);
  if (!handle) {
    WARN("Couldn't load image\n");
    return NULL;
  }

  if (uType == GXIMAGE_BITMAP) {
    GXBITMAP bmp;
    gxGetObjectW ((GXHGDIOBJ)handle, sizeof(GXBITMAP), &bmp);

    /* To match windows behavior, if cx is set to zero and
    the flag DI_DEFAULTSIZE is specified, cx becomes the
    system metric value for icons. If the flag is not specified
    the function sets the size to the height of the bitmap */
    if (cx == 0)
    {
      if (uFlags & GXDI_DEFAULTSIZE)
        cx = gxGetSystemMetrics (GXSM_CXICON);
      else
        cx = bmp.bmHeight;
    }

    nImageCount = bmp.bmWidth / cx;

    himl = gxImageList_Create (cx, bmp.bmHeight, GXILC_MASK | GXILC_COLOR,
      nImageCount, cGrow);
    if (!himl) {
      gxDeleteObject ((GXHGDIOBJ)handle);
      return NULL;
    }
    gxImageList_AddMasked (himl, (GXHBITMAP)handle, clrMask);
  }
  else if ((uType == GXIMAGE_ICON) || (uType == GXIMAGE_CURSOR)) {
    GXICONINFO ii;
    GXGDIBITMAP bmp;

    gxGetIconInfo ((GXHICON)handle, &ii);
    gxGetObjectW (ii.hbmColor, sizeof(GXBITMAP), (GXLPVOID)&bmp);
    himl = gxImageList_Create (bmp.bmWidth, bmp.bmHeight,
      GXILC_MASK | GXILC_COLOR, 1, cGrow);
    if (!himl) {
      gxDeleteObject (ii.hbmColor);
      gxDeleteObject (ii.hbmMask);
      gxDeleteObject ((GXHGDIOBJ)handle);
      return NULL;
    }
    gxImageList_Add (himl, ii.hbmColor, ii.hbmMask);
    gxDeleteObject (ii.hbmColor);
    gxDeleteObject (ii.hbmMask);
  }

  gxDeleteObject ((GXHGDIOBJ)handle);

  return himl;
}


/*************************************************************************
* gxImageList_Merge [COMCTL32.@]
*
* Create an image list containing a merged image from two image lists.
*
* PARAMS
*     himl1 [I] handle to first image list
*     i1    [I] first image index
*     himl2 [I] handle to second image list
*     i2    [I] second image index
*     dx    [I] X offset of the second image relative to the first.
*     dy    [I] Y offset of the second image relative to the first.
*
* RETURNS
*     Success: The newly created image list. It contains a single image
*              consisting of the second image merged with the first.
*     Failure: NULL, if either himl1 or himl2 are invalid.
*
* NOTES
*   - The returned image list should be deleted by the caller using
*     gxImageList_Destroy() when it is no longer required.
*   - If either i1 or i2 are not valid image indices they will be treated
*     as a blank image.
*/
GXHIMAGELIST GXDLLAPI
gxImageList_Merge (GXHIMAGELIST himl1, GXINT i1, GXHIMAGELIST himl2, GXINT i2,
           GXINT dx, GXINT dy)
{
  GXHIMAGELIST himlDst = NULL;
  GXINT      cxDst, cyDst;
  GXINT      xOff1, yOff1, xOff2, yOff2;
  GXPOINT    pt1, pt2;

  TRACE("(himl1=%p i1=%d himl2=%p i2=%d dx=%d dy=%d)\n", himl1, i1, himl2,
    i2, dx, dy);

  if (!is_valid(himl1) || !is_valid(himl2))
    return NULL;

  if (dx > 0) {
    cxDst = max (himl1->cx, dx + himl2->cx);
    xOff1 = 0;
    xOff2 = dx;
  }
  else if (dx < 0) {
    cxDst = max (himl2->cx, himl1->cx - dx);
    xOff1 = -dx;
    xOff2 = 0;
  }
  else {
    cxDst = max (himl1->cx, himl2->cx);
    xOff1 = 0;
    xOff2 = 0;
  }

  if (dy > 0) {
    cyDst = max (himl1->cy, dy + himl2->cy);
    yOff1 = 0;
    yOff2 = dy;
  }
  else if (dy < 0) {
    cyDst = max (himl2->cy, himl1->cy - dy);
    yOff1 = -dy;
    yOff2 = 0;
  }
  else {
    cyDst = max (himl1->cy, himl2->cy);
    yOff1 = 0;
    yOff2 = 0;
  }

  himlDst = gxImageList_Create (cxDst, cyDst, GXILC_MASK | GXILC_COLOR, 1, 1);

  if (himlDst)
  {
    imagelist_point_from_index( himl1, i1, &pt1 );
    imagelist_point_from_index( himl1, i2, &pt2 );

    /* copy image */
    gxBitBlt (himlDst->hdcImage, 0, 0, cxDst, cyDst, himl1->hdcImage, 0, 0, GXBLACKNESS);
    if (i1 >= 0 && i1 < himl1->cCurImage)
      gxBitBlt (himlDst->hdcImage, xOff1, yOff1, himl1->cx, himl1->cy, himl1->hdcImage, pt1.x, pt1.y, GXSRCCOPY);
    if (i2 >= 0 && i2 < himl2->cCurImage)
    {
      gxBitBlt (himlDst->hdcImage, xOff2, yOff2, himl2->cx, himl2->cy, himl2->hdcMask , pt2.x, pt2.y, GXPATCOPY);
      gxBitBlt (himlDst->hdcImage, xOff2, yOff2, himl2->cx, himl2->cy, himl2->hdcImage, pt2.x, pt2.y, GXSRCPAINT);
    }

    /* copy mask */
    gxBitBlt (himlDst->hdcMask, 0, 0, cxDst, cyDst, himl1->hdcMask, 0, 0, GXWHITENESS);
    if (i1 >= 0 && i1 < himl1->cCurImage)
      gxBitBlt (himlDst->hdcMask,  xOff1, yOff1, himl1->cx, himl1->cy, himl1->hdcMask,  pt1.x, pt1.y, GXSRCCOPY);
    if (i2 >= 0 && i2 < himl2->cCurImage)
      gxBitBlt (himlDst->hdcMask,  xOff2, yOff2, himl2->cx, himl2->cy, himl2->hdcMask,  pt2.x, pt2.y, GXPATCOPY);

    himlDst->cCurImage = 1;
  }

  return himlDst;
}


/***********************************************************************
*           DIB_GetDIBWidthBytes
*
* Return the width of a DIB bitmap in bytes. DIB bitmap data is 32-bit aligned.
*/
static int DIB_GetDIBWidthBytes( int width, int depth )
{
  int words;

  switch(depth)
  {
  case 1:  words = (width + 31) / 32; break;
  case 4:  words = (width + 7) / 8; break;
  case 8:  words = (width + 3) / 4; break;
  case 15:
  case 16: words = (width + 1) / 2; break;
  case 24: words = (width * 3 + 3)/4; break;

  default:
    WARN("(%d): Unsupported depth\n", depth );
    /* fall through */
  case 32:
    words = width;
    break;
  }
  return 4 * words;
}

/***********************************************************************
*           DIB_GetDIBImageBytes
*
* Return the number of bytes used to hold the image in a DIB bitmap.
*/
static int DIB_GetDIBImageBytes( int width, int height, int depth )
{
  return DIB_GetDIBWidthBytes( width, depth ) * abs( height );
}


/* helper for gxImageList_Read, see comments below */
//static GXBOOL _read_bitmap(GXHDC hdcIml, LPSTREAM pstm)
//{
//    BITMAPFILEHEADER  bmfh;
//    int bitsperpixel, palspace;
//    char bmi_buf[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
//    LPBITMAPINFO bmi = (LPBITMAPINFO)bmi_buf;
//    int                result = FALSE;
//    LPBYTE             bits = NULL;
//
//    if (!GXSUCCEEDED(IStream_Read ( pstm, &bmfh, sizeof(bmfh), NULL)))
//        return FALSE;
//
//    if (bmfh.bfType != (('M'<<8)|'B'))
//        return FALSE;
//
//    if (!GXSUCCEEDED(IStream_Read ( pstm, &bmi->bmiHeader, sizeof(bmi->bmiHeader), NULL)))
//        return FALSE;
//
//    if ((bmi->bmiHeader.biSize != sizeof(bmi->bmiHeader)))
//        return FALSE;
//
//    TRACE("width %u, height %u, planes %u, bpp %u\n",
//          bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight,
//          bmi->bmiHeader.biPlanes, bmi->bmiHeader.biBitCount);
//
//    bitsperpixel = bmi->bmiHeader.biPlanes * bmi->bmiHeader.biBitCount;
//    if (bitsperpixel<=8)
//        palspace = (1<<bitsperpixel)*sizeof(RGBQUAD);
//    else
//        palspace = 0;
//
//    bmi->bmiHeader.biSizeImage = DIB_GetDIBImageBytes(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight, bitsperpixel);
//
//    /* read the palette right after the end of the bitmapinfoheader */
//    if (palspace && !GXSUCCEEDED(IStream_Read(pstm, bmi->bmiColors, palspace, NULL)))
//  goto error;
//
//    bits = Alloc(bmi->bmiHeader.biSizeImage);
//    if (!bits)
//        goto error;
//    if (!GXSUCCEEDED(IStream_Read(pstm, bits, bmi->bmiHeader.biSizeImage, NULL)))
//        goto error;
//
//    if (!StretchDIBits(hdcIml, 0, 0, bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight,
//                  0, 0, bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight,
//                  bits, bmi, DIB_RGB_COLORS, GXSRCCOPY))
//        goto error;
//    result = TRUE;
//
//error:
//    Free(bits);
//    return result;
//}

/*************************************************************************
* gxImageList_Read [COMCTL32.@]
*
* Reads an image list from a stream.
*
* PARAMS
*     pstm [I] pointer to a stream
*
* RETURNS
*     Success: handle to image list
*     Failure: NULL
*
* The format is like this:
*  ILHEAD      ilheadstruct;
*
* for the color image part:
*  BITMAPFILEHEADER  bmfh;
*  BITMAPINFOHEADER  bmih;
* only if it has a palette:
*  RGBQUAD    rgbs[nr_of_paletted_colors];
*
*  GXBYTE      colorbits[imagesize];
*
* the following only if the GXILC_MASK bit is set in ILHEAD.ilFlags:
*  BITMAPFILEHEADER  bmfh_mask;
*  BITMAPINFOHEADER  bmih_mask;
* only if it has a palette (it usually does not):
*  RGBQUAD    rgbs[nr_of_paletted_colors];
*
*  GXBYTE      maskbits[imagesize];
*/
//GXHIMAGELIST GXDLLAPI gxImageList_Read (LPSTREAM pstm)
//{
//    ILHEAD  ilHead;
//    GXHIMAGELIST  himl;
//    int    i;
//
//    TRACE("%p\n", pstm);
//
//    if (!GXSUCCEEDED(IStream_Read (pstm, &ilHead, sizeof(ILHEAD), NULL)))
//  return NULL;
//    if (ilHead.usMagic != (('L' << 8) | 'I'))
//  return NULL;
//    if (ilHead.usVersion != 0x101) /* probably version? */
//  return NULL;
//
//    TRACE("cx %u, cy %u, flags 0x%04x, cCurImage %u, cMaxImage %u\n",
//          ilHead.cx, ilHead.cy, ilHead.flags, ilHead.cCurImage, ilHead.cMaxImage);
//
//    himl = gxImageList_Create(ilHead.cx, ilHead.cy, ilHead.flags, ilHead.cCurImage, ilHead.cMaxImage);
//    if (!himl)
//  return NULL;
//
//    if (!_read_bitmap(himl->hdcImage, pstm))
//    {
//  WARN("failed to read bitmap from stream\n");
//  return NULL;
//    }
//    if (ilHead.flags & GXILC_MASK)
//    {
//        if (!_read_bitmap(himl->hdcMask, pstm))
//        {
//            WARN("failed to read mask bitmap from stream\n");
//      return NULL;
//  }
//    }
//
//    himl->cCurImage = ilHead.cCurImage;
//    himl->cMaxImage = ilHead.cMaxImage;
//
//    gxImageList_SetBkColor(himl,ilHead.bkcolor);
//    for (i=0;i<4;i++)
//  gxImageList_SetOverlayImage(himl,ilHead.ovls[i],i+1);
//    return himl;
//}


/*************************************************************************
* gxImageList_Remove [COMCTL32.@]
*
* Removes an image from an image list
*
* PARAMS
*     himl [I] image list handle
*     i    [I] image index
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* FIXME: as the image list storage test shows, native comctl32 simply shifts
* images without creating a new bitmap.
*/
GXBOOL GXDLLAPI
gxImageList_Remove (GXHIMAGELIST himl, GXINT i)
{
  GXHBITMAP hbmNewImage, hbmNewMask;
  GXHDC     hdcBmp;
  GXSIZE    sz;

  TRACE("(himl=%p i=%d)\n", himl, i);

  if (!is_valid(himl)) {
    ERR("Invalid image list handle!\n");
    return FALSE;
  }

  if ((i < -1) || (i >= himl->cCurImage)) {
    TRACE("index out of range! %d\n", i);
    return FALSE;
  }

  if (i == -1) {
    GXINT nCount;

    /* remove all */
    if (himl->cCurImage == 0) {
      /* remove all on empty ImageList is allowed */
      TRACE("remove all on empty ImageList!\n");
      return TRUE;
    }

    himl->cMaxImage = himl->cInitial + himl->cGrow - 1;
    himl->cCurImage = 0;
    for (nCount = 0; nCount < MAX_OVERLAYIMAGE; nCount++)
      himl->nOvlIdx[nCount] = -1;

    hbmNewImage = gxImageList_CreateImage(himl->hdcImage, himl, himl->cMaxImage, himl->cx);
    gxSelectObject (himl->hdcImage, hbmNewImage);
    gxDeleteObject (himl->hbmImage);
    himl->hbmImage = hbmNewImage;

    if (himl->hbmMask) {

      imagelist_get_bitmap_size(himl, himl->cMaxImage, himl->cx, &sz);
      hbmNewMask = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);
      gxSelectObject (himl->hdcMask, hbmNewMask);
      gxDeleteObject (himl->hbmMask);
      himl->hbmMask = hbmNewMask;
    }
  }
  else {
    /* delete one image */
    TRACE("Remove single image! %d\n", i);

    /* create new bitmap(s) */
    TRACE(" - Number of images: %d / %d (Old/New)\n",
      himl->cCurImage, himl->cCurImage - 1);

    hbmNewImage = gxImageList_CreateImage(himl->hdcImage, himl, himl->cMaxImage, himl->cx);

    imagelist_get_bitmap_size(himl, himl->cMaxImage, himl->cx, &sz );
    if (himl->hbmMask)
      hbmNewMask = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);
    else
      hbmNewMask = 0;  /* Just to keep compiler happy! */

    hdcBmp = gxCreateCompatibleDC (0);

    /* copy all images and masks prior to the "removed" image */
    if (i > 0) {
      TRACE("Pre image copy: Copy %d images\n", i);

      gxSelectObject (hdcBmp, hbmNewImage);
      imagelist_copy_images( himl, himl->hdcImage, hdcBmp, 0, i, 0 );

      if (himl->hbmMask) {
        gxSelectObject (hdcBmp, hbmNewMask);
        imagelist_copy_images( himl, himl->hdcMask, hdcBmp, 0, i, 0 );
      }
    }

    /* copy all images and masks behind the removed image */
    if (i < himl->cCurImage - 1) {
      TRACE("Post image copy!\n");

      gxSelectObject (hdcBmp, hbmNewImage);
      imagelist_copy_images( himl, himl->hdcImage, hdcBmp, i + 1,
        (himl->cCurImage - i), i );

      if (himl->hbmMask) {
        gxSelectObject (hdcBmp, hbmNewMask);
        imagelist_copy_images( himl, himl->hdcMask, hdcBmp, i + 1,
          (himl->cCurImage - i), i );
      }
    }

    gxDeleteDC (hdcBmp);

    /* delete old images and insert new ones */
    gxSelectObject (himl->hdcImage, hbmNewImage);
    gxDeleteObject (himl->hbmImage);
    himl->hbmImage = hbmNewImage;
    if (himl->hbmMask) {
      gxSelectObject (himl->hdcMask, hbmNewMask);
      gxDeleteObject (himl->hbmMask);
      himl->hbmMask = hbmNewMask;
    }

    himl->cCurImage--;
  }

  return TRUE;
}


/*************************************************************************
* gxImageList_Replace [COMCTL32.@]
*
* Replaces an image in an image list with a new image.
*
* PARAMS
*     himl     [I] handle to image list
*     i        [I] image index
*     hbmImage [I] handle to image bitmap
*     hbmMask  [I] handle to mask bitmap. Can be NULL.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_Replace (GXHIMAGELIST himl, GXINT i, GXHBITMAP hbmImage,
           GXHBITMAP hbmMask)
{
  GXHDC hdcImage;
  GXBITMAP bmp;
  GXHBITMAP hOldBitmap;
  GXPOINT pt;

  TRACE("%p %d %p %p\n", himl, i, hbmImage, hbmMask);

  if (!is_valid(himl)) {
    ERR("Invalid image list handle!\n");
    return FALSE;
  }

  if ((i >= himl->cMaxImage) || (i < 0)) {
    ERR("Invalid image index!\n");
    return FALSE;
  }

  if (!gxGetObjectW(hbmImage, sizeof(GXBITMAP), (GXLPVOID)&bmp))
    return FALSE;

  hdcImage = gxCreateCompatibleDC (0);

  /* Replace Image */
  hOldBitmap = (GXHBITMAP)gxSelectObject (hdcImage, hbmImage);

  imagelist_point_from_index(himl, i, &pt);
  gxStretchBlt (himl->hdcImage, pt.x, pt.y, himl->cx, himl->cy,
    hdcImage, 0, 0, bmp.bmWidth, bmp.bmHeight, GXSRCCOPY);

  if (himl->hbmMask)
  {
    GXHDC hdcTemp;
    GXHBITMAP hOldBitmapTemp;

    hdcTemp   = gxCreateCompatibleDC(0);
    hOldBitmapTemp = (GXHBITMAP)gxSelectObject(hdcTemp, hbmMask);

    gxStretchBlt (himl->hdcMask, pt.x, pt.y, himl->cx, himl->cy,
      hdcTemp, 0, 0, bmp.bmWidth, bmp.bmHeight, GXSRCCOPY);
    gxSelectObject(hdcTemp, hOldBitmapTemp);
    gxDeleteDC(hdcTemp);

    /* Remove the background from the image
    */
    gxBitBlt (himl->hdcImage, pt.x, pt.y, bmp.bmWidth, bmp.bmHeight,
      himl->hdcMask, pt.x, pt.y, 0x220326); /* NOTSRCAND */
  }

  gxSelectObject (hdcImage, hOldBitmap);
  gxDeleteDC (hdcImage);

  return TRUE;
}


/*************************************************************************
* gxImageList_ReplaceIcon [COMCTL32.@]
*
* Replaces an image in an image list using an icon.
*
* PARAMS
*     himl  [I] handle to image list
*     i     [I] image index
*     hIcon [I] handle to icon
*
* RETURNS
*     Success: index of the replaced image
*     Failure: -1
*/

GXINT GXDLLAPI
gxImageList_ReplaceIcon (GXHIMAGELIST himl, GXINT nIndex, GXHICON hIcon)
{
  GXHDC     hdcImage;
  GXHICON   hBestFitIcon;
  GXHBITMAP hbmOldSrc;
  GXICONINFO  ii;
  GXBITMAP  bmp;
  GXBOOL    ret;
  GXPOINT   pt;

  TRACE("(%p %d %p)\n", himl, nIndex, hIcon);

  if (!is_valid(himl)) {
    ERR("invalid image list\n");
    return -1;
  }
  if ((nIndex >= himl->cMaxImage) || (nIndex < -1)) {
    ERR("invalid image index %d / %d\n", nIndex, himl->cMaxImage);
    return -1;
  }

  hBestFitIcon = (GXHICON)gxCopyImage(
    hIcon, GXIMAGE_ICON,
    himl->cx, himl->cy,
    GXLR_COPYFROMRESOURCE);
  /* the above will fail if the icon wasn't loaded from a resource, so try
  * again without LR_COPYFROMRESOURCE flag */
  if (!hBestFitIcon)
    hBestFitIcon = (GXHICON)gxCopyImage(
    hIcon, GXIMAGE_ICON,
    himl->cx, himl->cy,
    0);
  if (!hBestFitIcon)
    return -1;

  ret = gxGetIconInfo (hBestFitIcon, &ii);
  if (!ret) {
    gxDestroyIcon(hBestFitIcon);
    return -1;
  }

  ret = gxGetObjectW (ii.hbmMask, sizeof(GXBITMAP), (GXLPVOID)&bmp);
  if (!ret) {
    ERR("couldn't get mask bitmap info\n");
    if (ii.hbmColor)
      gxDeleteObject (ii.hbmColor);
    if (ii.hbmMask)
      gxDeleteObject (ii.hbmMask);
    gxDestroyIcon(hBestFitIcon);
    return -1;
  }

  if (nIndex == -1) {
    if (himl->cCurImage + 1 > himl->cMaxImage)
      IMAGELIST_InternalExpandBitmaps (himl, 1, 0, 0);

    nIndex = himl->cCurImage;
    himl->cCurImage++;
  }

  hdcImage = gxCreateCompatibleDC (0);
  TRACE("hdcImage=%p\n", hdcImage);
  if (hdcImage == 0) {
    ERR("invalid hdcImage!\n");
  }

  imagelist_point_from_index(himl, nIndex, &pt);

  gxSetTextColor(himl->hdcImage, GXRGB(0,0,0));
  gxSetBkColor  (himl->hdcImage, GXRGB(255,255,255));

  if (ii.hbmColor)
  {
    hbmOldSrc = (GXHBITMAP)gxSelectObject (hdcImage, ii.hbmColor);
    gxStretchBlt (himl->hdcImage, pt.x, pt.y, himl->cx, himl->cy,
      hdcImage, 0, 0, bmp.bmWidth, bmp.bmHeight, GXSRCCOPY);
    if (himl->hbmMask)
    {
      gxSelectObject (hdcImage, ii.hbmMask);
      gxStretchBlt (himl->hdcMask, pt.x, pt.y, himl->cx, himl->cy,
        hdcImage, 0, 0, bmp.bmWidth, bmp.bmHeight, GXSRCCOPY);
    }
  }
  else
  {
    GXUINT height = bmp.bmHeight / 2;
    hbmOldSrc = (GXHBITMAP)gxSelectObject (hdcImage, ii.hbmMask);
    gxStretchBlt (himl->hdcImage, pt.x, pt.y, himl->cx, himl->cy,
      hdcImage, 0, height, bmp.bmWidth, height, GXSRCCOPY);
    if (himl->hbmMask)
      gxStretchBlt (himl->hdcMask, pt.x, pt.y, himl->cx, himl->cy,
      hdcImage, 0, 0, bmp.bmWidth, height, GXSRCCOPY);
  }

  gxSelectObject (hdcImage, hbmOldSrc);

  gxDestroyIcon(hBestFitIcon);
  if (hdcImage)
    gxDeleteDC (hdcImage);
  if (ii.hbmColor)
    gxDeleteObject (ii.hbmColor);
  if (ii.hbmMask)
    gxDeleteObject (ii.hbmMask);

  TRACE("Insert index = %d, himl->cCurImage = %d\n", nIndex, himl->cCurImage);
  return nIndex;
}


/*************************************************************************
* gxImageList_SetBkColor [COMCTL32.@]
*
* Sets the background color of an image list.
*
* PARAMS
*     himl  [I] handle to image list
*     clrBk [I] background color
*
* RETURNS
*     Success: previous background color
*     Failure: GXCLR_NONE
*/

GXCOLORREF GXDLLAPI
gxImageList_SetBkColor (GXHIMAGELIST himl, GXCOLORREF clrBk)
{
  GXCOLORREF clrOldBk;

  if (!is_valid(himl))
    return GXCLR_NONE;

  clrOldBk = himl->clrBk;
  himl->clrBk = clrBk;
  return clrOldBk;
}


/*************************************************************************
* gxImageList_SetDragCursorImage [COMCTL32.@]
*
* Combines the specified image with the current drag image
*
* PARAMS
*     himlDrag  [I] handle to drag image list
*     iDrag     [I] drag image index
*     dxHotspot [I] X position of the hot spot
*     dyHotspot [I] Y position of the hot spot
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* NOTES
*   - The names dxHotspot, dyHotspot are misleading because they have nothing
*     to do with a hotspot but are only the offset of the origin of the new
*     image relative to the origin of the old image.
*
*   - When this function is called and the drag image is visible, a
*     short flickering occurs but this matches the Win9x behavior. It is
*     possible to fix the flickering using code like in gxImageList_DragMove.
*/

GXBOOL GXDLLAPI
gxImageList_SetDragCursorImage (GXHIMAGELIST himlDrag, GXINT iDrag,
                GXINT dxHotspot, GXINT dyHotspot)
{
  GXHIMAGELIST himlTemp;
  GXBOOL visible;

  if (!is_valid(InternalDrag.himl) || !is_valid(himlDrag))
    return FALSE;

  TRACE(" dxH=%d dyH=%d nX=%d nY=%d\n",
    dxHotspot, dyHotspot, InternalDrag.dxHotspot, InternalDrag.dyHotspot);

  visible = InternalDrag.bShow;

  himlTemp = gxImageList_Merge (InternalDrag.himl, 0, himlDrag, iDrag,
    dxHotspot, dyHotspot);

  if (visible) {
    /* hide the drag image */
    gxImageList_DragShowNolock(FALSE);
  }
  if ((InternalDrag.himl->cx != himlTemp->cx) ||
    (InternalDrag.himl->cy != himlTemp->cy)) {
      /* the size of the drag image changed, invalidate the buffer */
      gxDeleteObject(InternalDrag.hbmBg);
      InternalDrag.hbmBg = 0;
  }

  gxImageList_Destroy (InternalDrag.himl);
  InternalDrag.himl = himlTemp;

  if (visible) {
    /* show the drag image */
    gxImageList_DragShowNolock(TRUE);
  }

  return TRUE;
}


/*************************************************************************
* gxImageList_SetFilter [COMCTL32.@]
*
* Sets a filter (or does something completely different)!!???
* It removes 12 Bytes from the stack (3 Parameters).
*
* PARAMS
*     himl     [I] SHOULD be a handle to image list
*     i        [I] COULD be an index?
*     dwFilter [I] ???
*
* RETURNS
*     Success: TRUE ???
*     Failure: FALSE ???
*
* BUGS
*     This is an UNDOCUMENTED function!!!!
*     empty stub.
*/

GXBOOL GXDLLAPI
gxImageList_SetFilter (GXHIMAGELIST himl, GXINT i, GXDWORD dwFilter)
{
  FIXME("(%p 0x%x 0x%x):empty stub!\n", himl, i, dwFilter);

  return FALSE;
}


/*************************************************************************
* gxImageList_SetFlags [COMCTL32.@]
*
* Sets the image list flags.
*
* PARAMS
*     himl  [I] Handle to image list
*     flags [I] Flags to set
*
* RETURNS
*     Old flags?
*
* BUGS
*    Stub.
*/

GXDWORD GXDLLAPI
gxImageList_SetFlags(GXHIMAGELIST himl, GXDWORD flags)
{
  FIXME("(%p %08x):empty stub\n", himl, flags);
  return 0;
}


/*************************************************************************
* gxImageList_SetIconSize [COMCTL32.@]
*
* Sets the image size of the bitmap and deletes all images.
*
* PARAMS
*     himl [I] handle to image list
*     cx   [I] image width
*     cy   [I] image height
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_SetIconSize (GXHIMAGELIST himl, GXINT cx, GXINT cy)
{
  GXINT nCount;
  GXHBITMAP hbmNew;

  if (!is_valid(himl))
    return FALSE;

  /* remove all images */
  himl->cMaxImage = himl->cInitial + 1;
  himl->cCurImage = 0;
  himl->cx        = cx;
  himl->cy        = cy;

  /* initialize overlay mask indices */
  for (nCount = 0; nCount < MAX_OVERLAYIMAGE; nCount++)
    himl->nOvlIdx[nCount] = -1;

  hbmNew = gxImageList_CreateImage(himl->hdcImage, himl, himl->cMaxImage, himl->cx);
  gxSelectObject (himl->hdcImage, hbmNew);
  gxDeleteObject (himl->hbmImage);
  himl->hbmImage = hbmNew;

  if (himl->hbmMask) {
    GXSIZE sz;
    imagelist_get_bitmap_size(himl, himl->cMaxImage, himl->cx, &sz);
    hbmNew = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);
    gxSelectObject (himl->hdcMask, hbmNew);
    gxDeleteObject (himl->hbmMask);
    himl->hbmMask = hbmNew;
  }

  return TRUE;
}


/*************************************************************************
* gxImageList_SetImageCount [COMCTL32.@]
*
* Resizes an image list to the specified number of images.
*
* PARAMS
*     himl        [I] handle to image list
*     iImageCount [I] number of images in the image list
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_SetImageCount (GXHIMAGELIST himl, GXUINT iImageCount)
{
  GXHDC     hdcBitmap;
  GXHBITMAP hbmNewBitmap, hbmOld;
  GXINT     nNewCount, nCopyCount;

  TRACE("%p %d\n",himl,iImageCount);

  if (!is_valid(himl))
    return FALSE;
  if (himl->cMaxImage > iImageCount)
  {
    himl->cCurImage = iImageCount;
    /* TODO: shrink the bitmap when cMaxImage-cCurImage>cGrow ? */
    return TRUE;
  }

  nNewCount = iImageCount + himl->cGrow;
  nCopyCount = min(himl->cCurImage, iImageCount);

  hdcBitmap = gxCreateCompatibleDC (0);

  hbmNewBitmap = gxImageList_CreateImage(hdcBitmap, himl, nNewCount, himl->cx);

  if (hbmNewBitmap != 0)
  {
    hbmOld = (GXHBITMAP)gxSelectObject (hdcBitmap, hbmNewBitmap);
    imagelist_copy_images( himl, himl->hdcImage, hdcBitmap, 0, nCopyCount, 0 );
    gxSelectObject (hdcBitmap, hbmOld);

    /* FIXME: delete 'empty' image space? */

    gxSelectObject (himl->hdcImage, hbmNewBitmap);
    gxDeleteObject (himl->hbmImage);
    himl->hbmImage = hbmNewBitmap;
  }
  else
    ERR("Could not create new image bitmap !\n");

  if (himl->hbmMask)
  {
    GXSIZE sz;
    imagelist_get_bitmap_size( himl, nNewCount, himl->cx, &sz );
    hbmNewBitmap = gxCreateBitmap (sz.cx, sz.cy, 1, 1, NULL);
    if (hbmNewBitmap != 0)
    {
      hbmOld = (GXHBITMAP)gxSelectObject (hdcBitmap, hbmNewBitmap);
      imagelist_copy_images( himl, himl->hdcMask, hdcBitmap, 0, nCopyCount, 0 );
      gxSelectObject (hdcBitmap, hbmOld);

      /* FIXME: delete 'empty' image space? */

      gxSelectObject (himl->hdcMask, hbmNewBitmap);
      gxDeleteObject (himl->hbmMask);
      himl->hbmMask = hbmNewBitmap;
    }
    else
      ERR("Could not create new mask bitmap!\n");
  }

  gxDeleteDC (hdcBitmap);

  /* Update max image count and current image count */
  himl->cMaxImage = nNewCount;
  himl->cCurImage = iImageCount;

  return TRUE;
}


/*************************************************************************
* gxImageList_SetOverlayImage [COMCTL32.@]
*
* Assigns an overlay mask index to an existing image in an image list.
*
* PARAMS
*     himl     [I] handle to image list
*     iImage   [I] image index
*     iOverlay [I] overlay mask index
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/

GXBOOL GXDLLAPI
gxImageList_SetOverlayImage (GXHIMAGELIST himl, GXINT iImage, GXINT iOverlay)
{
  if (!is_valid(himl))
    return FALSE;
  if ((iOverlay < 1) || (iOverlay > MAX_OVERLAYIMAGE))
    return FALSE;
  if ((iImage!=-1) && ((iImage < 0) || (iImage > himl->cCurImage)))
    return FALSE;
  himl->nOvlIdx[iOverlay - 1] = iImage;
  return TRUE;
}



/* helper for gxImageList_Write - write bitmap to pstm
* currently everything is written as 24 bit RGB, except masks
*/
//static GXBOOL
//_write_bitmap(GXHBITMAP hBitmap, LPSTREAM pstm)
//{
//    LPBITMAPFILEHEADER bmfh;
//    LPBITMAPINFOHEADER bmih;
//    LPBYTE data = NULL, lpBits;
//    BITMAP bm;
//    GXINT bitCount, sizeImage, offBits, totalSize;
//    GXHDC xdc;
//    GXBOOL result = FALSE;
//
//    if (!gxGetObjectW(hBitmap, sizeof(BITMAP), (GXLPVOID)&bm))
//        return FALSE;
//
//    bitCount = bm.bmBitsPixel == 1 ? 1 : 24;
//    sizeImage = DIB_GetDIBImageBytes(bm.bmWidth, bm.bmHeight, bitCount);
//
//    totalSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//    if(bitCount != 24)
//  totalSize += (1 << bitCount) * sizeof(RGBQUAD);
//    offBits = totalSize;
//    totalSize += sizeImage;
//
//    data = Alloc(totalSize);
//    bmfh = (LPBITMAPFILEHEADER)data;
//    bmih = (LPBITMAPINFOHEADER)(data + sizeof(BITMAPFILEHEADER));
//    lpBits = data + offBits;
//
//    /* setup BITMAPFILEHEADER */
//    bmfh->bfType      = (('M' << 8) | 'B');
//    bmfh->bfSize      = offBits;
//    bmfh->bfReserved1 = 0;
//    bmfh->bfReserved2 = 0;
//    bmfh->bfOffBits   = offBits;
//
//    /* setup BITMAPINFOHEADER */
//    bmih->biSize          = sizeof(BITMAPINFOHEADER);
//    bmih->biWidth         = bm.bmWidth;
//    bmih->biHeight        = bm.bmHeight;
//    bmih->biPlanes        = 1;
//    bmih->biBitCount      = bitCount;
//    bmih->biCompression   = BI_RGB;
//    bmih->biSizeImage     = sizeImage;
//    bmih->biXPelsPerMeter = 0;
//    bmih->biYPelsPerMeter = 0;
//    bmih->biClrUsed       = 0;
//    bmih->biClrImportant  = 0;
//
//    xdc = GetDC(0);
//    result = GetDIBits(xdc, hBitmap, 0, bm.bmHeight, lpBits, (BITMAPINFO *)bmih, DIB_RGB_COLORS) == bm.bmHeight;
//    gxReleaseDC(0, xdc);
//    if (!result)
//  goto failed;
//
//    TRACE("width %u, height %u, planes %u, bpp %u\n",
//          bmih->biWidth, bmih->biHeight,
//          bmih->biPlanes, bmih->biBitCount);
//
//    if(bitCount == 1) {
//        /* Hack. */
//  LPBITMAPINFO inf = (LPBITMAPINFO)bmih;
//  inf->bmiColors[0].rgbRed = inf->bmiColors[0].rgbGreen = inf->bmiColors[0].rgbBlue = 0;
//  inf->bmiColors[1].rgbRed = inf->bmiColors[1].rgbGreen = inf->bmiColors[1].rgbBlue = 0xff;
//    }
//
//    if(!GXSUCCEEDED(IStream_Write(pstm, data, totalSize, NULL)))
//  goto failed;
//
//    result = TRUE;
//
//failed:
//    Free(data);
//
//    return result;
//}


/*************************************************************************
* gxImageList_Write [COMCTL32.@]
*
* Writes an image list to a stream.
*
* PARAMS
*     himl [I] handle to image list
*     pstm [O] Pointer to a stream.
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*
* BUGS
*     probably.
*/

//GXBOOL GXDLLAPI
//gxImageList_Write (GXHIMAGELIST himl, LPSTREAM pstm)
//{
//    ILHEAD ilHead;
//    int i;
//
//    TRACE("%p %p\n", himl, pstm);
//
//    if (!is_valid(himl))
//  return FALSE;
//
//    ilHead.usMagic   = (('L' << 8) | 'I');
//    ilHead.usVersion = 0x101;
//    ilHead.cCurImage = himl->cCurImage;
//    ilHead.cMaxImage = himl->cMaxImage;
//    ilHead.cGrow     = himl->cGrow;
//    ilHead.cx        = himl->cx;
//    ilHead.cy        = himl->cy;
//    ilHead.bkcolor   = himl->clrBk;
//    ilHead.flags     = himl->flags;
//    for(i = 0; i < 4; i++) {
//  ilHead.ovls[i] = himl->nOvlIdx[i];
//    }
//
//    TRACE("cx %u, cy %u, flags 0x04%x, cCurImage %u, cMaxImage %u\n",
//          ilHead.cx, ilHead.cy, ilHead.flags, ilHead.cCurImage, ilHead.cMaxImage);
//
//    if(!GXSUCCEEDED(IStream_Write(pstm, &ilHead, sizeof(ILHEAD), NULL)))
//  return FALSE;
//
//    /* write the bitmap */
//    if(!_write_bitmap(himl->hbmImage, pstm))
//  return FALSE;
//
//    /* write the mask if we have one */
//    if(himl->flags & GXILC_MASK) {
//  if(!_write_bitmap(himl->hbmMask, pstm))
//      return FALSE;
//    }
//
//    return TRUE;
//}


static GXHBITMAP gxImageList_CreateImage(GXHDC hdc, GXHIMAGELIST himl, GXUINT count, GXUINT width)
{
  GXHBITMAP hbmNewBitmap;
  GXUINT ilc = (himl->flags & 0xFE);
  GXSIZE sz;

  imagelist_get_bitmap_size( himl, count, width, &sz );

  if ((ilc >= GXILC_COLOR4 && ilc <= GXILC_COLOR32) || ilc == GXILC_COLOR)
  {
    GXVOID* bits;
    GXBITMAPINFO *bmi;

    TRACE("Creating DIBSection %d x %d, %d Bits per Pixel\n",
      sz.cx, sz.cy, himl->uBitsPixel);

    if (himl->uBitsPixel <= GXILC_COLOR8)
    {
      GXLPPALETTEENTRY pal;
      GXULONG i, colors;
      GXBYTE temp;

      colors = 1 << himl->uBitsPixel;
      bmi = (GXBITMAPINFO*)Alloc(sizeof(GXBITMAPINFOHEADER) +
        sizeof(GXPALETTEENTRY) * colors);

      pal = (GXLPPALETTEENTRY)bmi->bmiColors;
      gxGetPaletteEntries((GXHPALETTE)gxGetStockObject(GXDEFAULT_PALETTE), 0, colors, pal);

      /* Swap colors returned by GetPaletteEntries so we can use them for
      * CreateDIBSection call. */
      for (i = 0; i < colors; i++)
      {
        temp = pal[i].peBlue;
        bmi->bmiColors[i].rgbRed = pal[i].peRed;
        bmi->bmiColors[i].rgbBlue = temp;
      }
    }
    else
    {
      bmi = (GXBITMAPINFO*)Alloc(sizeof(GXBITMAPINFOHEADER));
    }

    bmi->bmiHeader.biSize = sizeof(GXBITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = sz.cx;
    bmi->bmiHeader.biHeight = sz.cy;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = himl->uBitsPixel;
    bmi->bmiHeader.biCompression = GXBI_RGB;
    bmi->bmiHeader.biSizeImage = 0;
    bmi->bmiHeader.biXPelsPerMeter = 0;
    bmi->bmiHeader.biYPelsPerMeter = 0;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;

    hbmNewBitmap = gxCreateDIBSection(hdc, bmi, GXDIB_RGB_COLORS, &bits, 0, 0);

    Free (bmi);
  }
  else /*if (ilc == GXILC_COLORDDB)*/
  {
    TRACE("Creating Bitmap: %d Bits per Pixel\n", himl->uBitsPixel);

    hbmNewBitmap = gxCreateBitmap (sz.cx, sz.cy, 1, himl->uBitsPixel, NULL);
  }
  TRACE("returning %p\n", hbmNewBitmap);
  return hbmNewBitmap;
}

/*************************************************************************
* gxImageList_SetColorTable [COMCTL32.@]
*
* Sets the color table of an image list.
*
* PARAMS
*     himl        [I] Handle to the image list.
*     uStartIndex [I] The first index to set.
*     cEntries    [I] Number of entries to set.
*     prgb        [I] New color information for color table for the image list.
*
* RETURNS
*     Success: Number of entries in the table that were set.
*     Failure: Zero.
*
* SEE
*     gxImageList_Create(), SetDIBColorTable()
*/

GXUINT GXDLLAPI
gxImageList_SetColorTable (GXHIMAGELIST himl, GXUINT uStartIndex, GXUINT cEntries, const GXRGBQUAD * prgb)
{
  ASSERT(FALSE);
  return 0;
  //return SetDIBColorTable(himl->hdcImage, uStartIndex, cEntries, prgb);
}
#endif // IMAGELIST
#endif // _DEV_DISABLE_UI_CODE