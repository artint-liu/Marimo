#ifndef _GX_COMMON_CONTROL_H_
#define _GX_COMMON_CONTROL_H_

extern GXCOMCTL32_SysColor  comctl32_color;
extern GXHBRUSH  gxCOMCTL32_hPattern55AABrush;
extern "C"
{
  GXHIMAGELIST  GXDLLAPI gxImageList_Create      (int cx, int cy, GXUINT flags, int cInitial, int cGrow);
  GXBOOL      GXDLLAPI gxImageList_Destroy      (GXHIMAGELIST himl);

  int        GXDLLAPI gxImageList_GetImageCount    (GXHIMAGELIST himl);
  GXBOOL      GXDLLAPI gxImageList_SetImageCount    (GXHIMAGELIST himl, GXUINT uNewCount);

  int        GXDLLAPI gxImageList_Add        (GXHIMAGELIST himl, GXHBITMAP hbmImage, GXHBITMAP hbmMask);

  GXINT      GXDLLAPI gxImageList_ReplaceIcon    (GXHIMAGELIST himl, GXINT i, GXHICON hicon);
  GXCOLORREF    GXDLLAPI gxImageList_SetBkColor    (GXHIMAGELIST himl, GXCOLORREF clrBk);
  GXCOLORREF    GXDLLAPI gxImageList_GetBkColor    (GXHIMAGELIST himl);
  GXBOOL      GXDLLAPI gxImageList_SetOverlayImage  (GXHIMAGELIST himl, int iImage, int iOverlay);


  GXBOOL      GXDLLAPI gxImageList_Draw        (GXHIMAGELIST himl, int i, GXHDC hdcDst, int x, int y, GXUINT fStyle);
  GXBOOL      GXDLLAPI gxImageList_Replace      (GXHIMAGELIST himl, int i, GXHBITMAP hbmImage, GXHBITMAP hbmMask);

  GXINT      GXDLLAPI gxImageList_AddMasked      (GXHIMAGELIST himl, GXHBITMAP hbmImage, GXCOLORREF crMask);
  GXBOOL      GXDLLAPI gxImageList_DrawEx      (GXHIMAGELIST himl, int i, GXHDC hdcDst, int x, int y, int dx, int dy, GXCOLORREF rgbBk, GXCOLORREF rgbFg, GXUINT fStyle);
  GXBOOL      GXDLLAPI gxImageList_DrawIndirect    (GXIMAGELISTDRAWPARAMS* pimldp);
  GXBOOL      GXDLLAPI gxImageList_Remove      (GXHIMAGELIST himl, int i);
  GXHICON      GXDLLAPI gxImageList_GetIcon      (GXHIMAGELIST himl, int i, GXUINT flags);
  GXHIMAGELIST  GXDLLAPI gxImageList_LoadImageA    (GXHINSTANCE hi, GXLPCSTR lpbmp, int cx, int cGrow, GXCOLORREF crMask, GXUINT uType, GXUINT uFlags);
  GXHIMAGELIST  GXDLLAPI gxImageList_LoadImageW    (GXHINSTANCE hi, GXLPCWSTR lpbmp, int cx, int cGrow, GXCOLORREF crMask, GXUINT uType, GXUINT uFlags);
  GXBOOL      GXDLLAPI gxImageList_GetImageRect    (GXHIMAGELIST himl, GXINT i, GXLPRECT lpRect);

  GXBOOL      GXDLLAPI gxImageList_Copy        (GXHIMAGELIST himlDst, int iDst, GXHIMAGELIST himlSrc, int iSrc, GXUINT uFlags);
  GXBOOL      GXDLLAPI gxImageList_BeginDrag      (GXHIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot);
  void      GXDLLAPI gxImageList_EndDrag      ();
  GXBOOL      GXDLLAPI gxImageList_DragEnter      (GXHWND hwndLock, int x, int y);
  GXBOOL      GXDLLAPI gxImageList_DragLeave      (GXHWND hwndLock);
  GXBOOL      GXDLLAPI gxImageList_DragMove      (int x, int y);
  GXBOOL      GXDLLAPI gxImageList_SetDragCursorImage(GXHIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot);

  GXBOOL      GXDLLAPI gxImageList_DragShowNolock  (GXBOOL fShow);
  GXHIMAGELIST  GXDLLAPI gxImageList_GetDragImage    (GXPOINT *ppt, GXPOINT *pptHotspot);


  GXBOOL      GXDLLAPI gxImageList_GetIconSize    (GXHIMAGELIST himl, int *cx, int *cy);
  GXBOOL      GXDLLAPI gxImageList_SetIconSize    (GXHIMAGELIST himl, int cx, int cy);
  GXBOOL      GXDLLAPI gxImageList_GetImageInfo    (GXHIMAGELIST himl, int i, GXIMAGEINFO *pImageInfo);
  GXHIMAGELIST  GXDLLAPI gxImageList_Merge        (GXHIMAGELIST himl1, int i1, GXHIMAGELIST himl2, int i2, int dx, int dy);
  GXHIMAGELIST  GXDLLAPI gxImageList_Duplicate      (GXHIMAGELIST himl);

}



#define GXCLR_NONE                0xFFFFFFFFL
#define GXCLR_DEFAULT             0xFF000000L

#define GXILCF_MOVE          (0x00000000)
#define GXILCF_SWAP          (0x00000001)


#define GXILD_NORMAL              0x00000000
#define GXILD_TRANSPARENT         0x00000001
#define GXILD_MASK                0x00000010
#define GXILD_IMAGE               0x00000020
#define GXILD_ROP                 0x00000040
#define GXILD_BLEND25             0x00000002
#define GXILD_BLEND50             0x00000004
#define GXILD_OVERLAYMASK         0x00000F00
#define GXINDEXTOOVERLAYMASK(i)   ((i) << 8)
#define GXILD_PRESERVEALPHA       0x00001000  // This preserves the alpha channel in dest
#define GXILD_SCALE               0x00002000  // Causes the image to be scaled to cx, cy instead of clipped
#define GXILD_DPISCALE            0x00004000

#define GXILS_NORMAL              0x00000000 
#define GXILS_GLOW                0x00000001
#define GXILS_SHADOW              0x00000002
#define GXILS_SATURATE            0x00000004
#define GXILS_ALPHA               0x00000008

#define GXILC_MASK                0x00000001
#define GXILC_COLOR               0x00000000
#define GXILC_COLORDDB            0x000000FE
#define GXILC_COLOR4              0x00000004
#define GXILC_COLOR8              0x00000008
#define GXILC_COLOR16             0x00000010
#define GXILC_COLOR24             0x00000018
#define GXILC_COLOR32             0x00000020
#define GXILC_PALETTE             0x00000800      // (not implemented)
#define GXILC_MIRROR              0x00002000      // Mirror the icons contained, if the process is mirrored
#define GXILC_PERITEMMIRROR       0x00008000      // Causes the mirroring code to mirror each item when inserting a set of images, verses the whole strip

GXBOOL  GXDLLAPI gxStr_SetPtrW (GXLPWSTR *lppDest, GXLPCWSTR lpSrc);
GXINT  GXDLLAPI gxStr_GetPtrW (GXLPCWSTR lpSrc, GXLPWSTR lpDest, GXINT nMaxLen);
GXINT  GXDLLAPI gxStr_GetPtrWtoA (GXLPCWSTR lpSrc, GXLPSTR lpDest, GXINT nMaxLen);
GXINT  GXDLLAPI gxStr_GetPtrAtoW (GXLPCSTR lpSrc, GXLPWSTR lpDest, GXINT nMaxLen);
GXBOOL  GXDLLAPI gxStr_SetPtrAtoW (GXLPWSTR *lppDest, GXLPCSTR lpSrc);
GXBOOL  GXDLLAPI gxStr_SetPtrWtoA (GXLPSTR *lppDest, GXLPCWSTR lpSrc);

typedef GXLRESULT (GXCALLBACK *GXSUBCLASSPROC)(GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam,
                     GXLPARAM lParam, GXUINT_PTR uIdSubclass, GXDWORD_PTR dwRefData);

/* Our internal stack structure of the window procedures to subclass */
typedef struct _SUBCLASSPROCS {
  GXSUBCLASSPROC subproc;
  GXUINT_PTR id;
  GXDWORD_PTR ref;
  struct _SUBCLASSPROCS *next;
} SUBCLASSPROCS, *LPSUBCLASSPROCS;

typedef struct
{
  SUBCLASSPROCS *SubclassProcs;
  SUBCLASSPROCS *stackpos;
  GXWNDPROC origproc;
  int running;
} SUBCLASS_INFO, *LPSUBCLASS_INFO;

GXBOOL    GXDLLAPI gxSetWindowSubclass (GXHWND hWnd, GXSUBCLASSPROC pfnSubclass, GXUINT_PTR uIDSubclass, GXDWORD_PTR dwRef);
GXBOOL    GXDLLAPI gxGetWindowSubclass (GXHWND hWnd, GXSUBCLASSPROC pfnSubclass, GXUINT_PTR uID, GXDWORD_PTR *pdwRef);
GXBOOL    GXDLLAPI gxRemoveWindowSubclass(GXHWND hWnd, GXSUBCLASSPROC pfnSubclass, GXUINT_PTR uID);
GXLRESULT  GXDLLAPI gxDefSubclassProc (GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam);

GXLPVOID  GXDLLAPI Alloc    (GXUINT nBytes);
GXLPVOID  GXDLLAPI ReAlloc  (GXLPVOID lpPointer, GXUINT nBytes);
GXVOID    GXDLLAPI Free    (GXLPVOID lpPointer);

GXHWND gxCOMCTL32_CreateToolTip(GXHWND hwndOwner);
GXVOID gxCOMCTL32_RefreshSysColors();
GXBOOL gxCOMCTL32_IsReflectedMessage(GXUINT uMsg);
void gxCOMCTL32_DrawInsertMark(GXHDC hDC, const GXRECT *lpRect, GXCOLORREF clrInsertMark, GXBOOL bHorizontal);
void gxCOMCTL32_EnsureBitmapSize(GXHBITMAP *pBitmap, int cxMinWidth, int cyMinHeight, GXCOLORREF crBackground);

GXHBITMAP GXDLLAPI gxCreateMappedBitmap(GXHINSTANCE hInstance,int idBitmap,GXUINT wFlags,GXLPCOLORMAP lpColorMap,int iNumMaps);

#endif // _GX_COMMON_CONTROL_H_