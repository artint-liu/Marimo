#ifndef _DEV_DISABLE_UI_CODE
#include <GrapX.H>
#include "GXCommCtrl.H"
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXKernel.H"

GXCOMCTL32_SysColor  comctl32_color;
#define COMCTL32_wSubclass  L"CCSubCtrl"
GXHBRUSH  gxCOMCTL32_hPattern55AABrush = NULL;
#define ERR TRACE

GXLRESULT GXDLLAPI COMCTL32_SubclassProc (GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam);
GXLRESULT GXDLLAPI gxDefSubclassProc (GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam);

GXLPVOID GXDLLAPI Alloc(GXUINT nBytes)
{
  GXLPVOID lpPointer = new GXCHAR[nBytes];
  //TRACE("Alloc:%p\n", lpPointer);
  memset(lpPointer, 0, nBytes);
  return lpPointer;
}

GXLPVOID GXDLLAPI ReAlloc(GXLPVOID lpPointer, GXUINT nBytes)
{
  GXCHAR* lpNewPoint = new GXCHAR[nBytes];
  //TRACE("ReAlloc:%p\n", lpNewPoint);
  if(lpPointer != NULL)
  {
    memcpy(lpNewPoint, lpPointer, nBytes);
    delete[] (GXCHAR*)lpPointer;
  }
  return lpNewPoint;
}


GXVOID GXDLLAPI Free(GXLPVOID lpPointer)
{
  if(lpPointer != NULL)
    delete[](GXCHAR*)lpPointer;
}


/**************************************************************************
* Str_SetPtrW [COMCTL32.236]
*
* See Str_SetPtrA.
*/
GXBOOL GXDLLAPI gxStr_SetPtrW (GXLPWSTR *lppDest, GXLPCWSTR lpSrc)
{
  TRACE("(%p %p)\n", lppDest, lpSrc);

  if (lpSrc) {
    GXINT len = (GXINT)GXSTRLEN(lpSrc) + 1;
    Free(*lppDest);
    GXLPWSTR ptr = (GXLPWSTR)Alloc (len * sizeof(GXWCHAR));
    if (!ptr)
      return FALSE;
    GXSTRCPY (ptr, lpSrc);
    *lppDest = ptr;
  }
  else {
    Free (*lppDest);
    *lppDest = NULL;
  }

  return TRUE;
}
/**************************************************************************
* Str_GetPtrW [COMCTL32.235]
*
* See Str_GetPtrA.
*/
GXINT GXDLLAPI gxStr_GetPtrW(GXLPCWSTR lpSrc, GXLPWSTR lpDest, GXINT nMaxLen)
{
  GXINT len;

  TRACE("(%p %p %d)\n", lpSrc, lpDest, nMaxLen);

  if (!lpDest && lpSrc)
    return (GXINT)GXSTRLEN (lpSrc);

  if (nMaxLen == 0)
    return 0;

  if (lpSrc == NULL) {
    lpDest[0] = '\0';
    return 0;
  }

  len = (GXINT)GXSTRLEN (lpSrc);
  if (len >= nMaxLen)
    len = nMaxLen - 1;

  gxRtlMoveMemory (lpDest, lpSrc, len*sizeof(GXWCHAR));
  lpDest[len] = '\0';

  return len;
}

/**************************************************************************
* Str_GetPtrWtoA [internal]
*
* Converts a unicode string into a multi byte string
*
* PARAMS
*     lpSrc   [I] Pointer to the unicode source string
*     lpDest  [O] Pointer to caller supplied storage for the multi byte string
*     nMaxLen [I] Size, in bytes, of the destination buffer
*
* RETURNS
*     Length, in bytes, of the converted string.
*/

GXINT GXDLLAPI gxStr_GetPtrWtoA (GXLPCWSTR lpSrc, GXLPSTR lpDest, GXINT nMaxLen)
{
  GXINT len;

  //TRACE("(%s %p %d)\n", debugstr_w(lpSrc), lpDest, nMaxLen);

  if (!lpDest && lpSrc)
    return gxWideCharToMultiByte(GXCP_ACP, 0, lpSrc, -1, 0, 0, NULL, NULL);

  if (nMaxLen == 0)
    return 0;

  if (lpSrc == NULL) {
    lpDest[0] = '\0';
    return 0;
  }

  len = gxWideCharToMultiByte(GXCP_ACP, 0, lpSrc, -1, 0, 0, NULL, NULL);
  if (len >= nMaxLen)
    len = nMaxLen - 1;

  gxWideCharToMultiByte(GXCP_ACP, 0, lpSrc, -1, lpDest, (int)len, NULL, NULL);
  lpDest[len] = '\0';

  return len;
}

/**************************************************************************
* Str_GetPtrAtoW [internal]
*
* Converts a multibyte string into a unicode string
*
* PARAMS
*     lpSrc   [I] Pointer to the multibyte source string
*     lpDest  [O] Pointer to caller supplied storage for the unicode string
*     nMaxLen [I] Size, in characters, of the destination buffer
*
* RETURNS
*     Length, in characters, of the converted string.
*/

GXINT GXDLLAPI gxStr_GetPtrAtoW (GXLPCSTR lpSrc, GXLPWSTR lpDest, GXINT nMaxLen)
{
  GXINT len;

  //TRACE("(%s %p %d)\n", debugstr_a(lpSrc), lpDest, nMaxLen);

  if (!lpDest && lpSrc)
    return gxMultiByteToWideChar(GXCP_ACP, 0, lpSrc, -1, 0, 0);

  if (nMaxLen == 0)
    return 0;

  if (lpSrc == NULL) {
    lpDest[0] = '\0';
    return 0;
  }

  len = gxMultiByteToWideChar(GXCP_ACP, 0, lpSrc, -1, 0, 0);
  if (len >= nMaxLen)
    len = nMaxLen - 1;

  gxMultiByteToWideChar(GXCP_ACP, 0, lpSrc, -1, lpDest, (int)len);
  lpDest[len] = '\0';

  return len;
}


/**************************************************************************
* Str_SetPtrAtoW [internal]
*
* Converts a multi byte string to a unicode string.
* If the pointer to the destination buffer is NULL a buffer is allocated.
* If the destination buffer is too small to keep the converted multi byte
* string the destination buffer is reallocated. If the source pointer is
* NULL, the destination buffer is freed.
*
* PARAMS
*     lppDest [I/O] pointer to a pointer to the destination buffer
*     lpSrc   [I] pointer to a multi byte string
*
* RETURNS
*     TRUE: conversion successful
*     FALSE: error
*/
GXBOOL GXDLLAPI gxStr_SetPtrAtoW (GXLPWSTR *lppDest, GXLPCSTR lpSrc)
{
  TRACE("(%p %s)\n", lppDest, lpSrc);

  if (lpSrc) {
    GXINT len = gxMultiByteToWideChar(GXCP_ACP,0,lpSrc,-1,NULL,0);
    GXLPWSTR ptr = (GXLPWSTR)ReAlloc (*lppDest, len*sizeof(GXWCHAR));

    if (!ptr)
      return FALSE;
    gxMultiByteToWideChar(GXCP_ACP,0,lpSrc,-1,ptr,(int)len);
    *lppDest = ptr;
  }
  else {
    Free (*lppDest);
    *lppDest = NULL;
  }

  return TRUE;
}

/**************************************************************************
* Str_SetPtrWtoA [internal]
*
* Converts a unicode string to a multi byte string.
* If the pointer to the destination buffer is NULL a buffer is allocated.
* If the destination buffer is too small to keep the converted wide
* string the destination buffer is reallocated. If the source pointer is
* NULL, the destination buffer is freed.
*
* PARAMS
*     lppDest [I/O] pointer to a pointer to the destination buffer
*     lpSrc   [I] pointer to a wide string
*
* RETURNS
*     TRUE: conversion successful
*     FALSE: error
*/
GXBOOL GXDLLAPI gxStr_SetPtrWtoA (GXLPSTR *lppDest, GXLPCWSTR lpSrc)
{
  //TRACE("(%p %s)\n", lppDest, debugstr_w(lpSrc));

  if (lpSrc) {
    GXINT len = gxWideCharToMultiByte(GXCP_ACP,0,lpSrc,-1,NULL,0,NULL,FALSE);
    GXLPSTR ptr = (GXLPSTR)ReAlloc (*lppDest, len*sizeof(GXCHAR));

    if (!ptr)
      return FALSE;
    gxWideCharToMultiByte(GXCP_ACP,0,lpSrc,-1,ptr,(int)len,NULL,FALSE);
    *lppDest = ptr;
  }
  else {
    Free (*lppDest);
    *lppDest = NULL;
  }

  return TRUE;
}

/***********************************************************************
* SetWindowSubclass [COMCTL32.410]
*
* Starts a window subclass
*
* PARAMS
*     hWnd [in] handle to window subclass.
*     pfnSubclass [in] Pointer to new window procedure.
*     uIDSubclass [in] Unique identifier of sublass together with pfnSubclass.
*     dwRef [in] Reference data to pass to window procedure.
*
* RETURNS
*     Success: non-zero
*     Failure: zero
*
* BUGS
*     If an application manually subclasses a window after subclassing it with
*     this API and then with this API again, then none of the previous 
*     subclasses get called or the original window procedure.
*/

GXBOOL GXDLLAPI gxSetWindowSubclass (GXHWND hWnd, GXSUBCLASSPROC pfnSubclass,
                 GXUINT_PTR uIDSubclass, GXDWORD_PTR dwRef)
{
  LPSUBCLASS_INFO stack;
  LPSUBCLASSPROCS proc;

  ASSERT(gxGetCurrentThreadId() == gxGetWindowThreadProcessId(hWnd, NULL));
  //TRACE ("(%p, %p, %lx, %lx)\n", hWnd, pfnSubclass, uIDSubclass, dwRef);

  /* Since the window procedure that we set here has two additional arguments,
  * we can't simply set it as the new window procedure of the window. So we
  * set our own window procedure and then calculate the other two arguments
  * from there. */

  /* See if we have been called for this window */
  stack = (LPSUBCLASS_INFO)gxGetPropW (hWnd, COMCTL32_wSubclass);
  if (!stack) {
    /* allocate stack */
    stack = (SUBCLASS_INFO*)Alloc (sizeof(SUBCLASS_INFO));
    if (!stack) {
      ERR ("Failed to allocate our Subclassing stack\n");
      return FALSE;
    }
    gxSetPropW (hWnd, COMCTL32_wSubclass, (GXHANDLE)stack);

    /* set window procedure to our own and save the current one */
    if (gxIsWindowUnicode (hWnd))
      stack->origproc = (GXWNDPROC)gxSetWindowLongPtrW (hWnd, GXGWLP_WNDPROC,
      (GXDWORD_PTR)COMCTL32_SubclassProc);
    else
      stack->origproc = (GXWNDPROC)gxSetWindowLongPtrA (hWnd, GXGWLP_WNDPROC,
      (GXDWORD_PTR)COMCTL32_SubclassProc);
  }
  else {
    /* Check to see if we have called this function with the same uIDSubClass
    * and pfnSubclass */
    proc = stack->SubclassProcs;
    while (proc) {
      if ((proc->id == uIDSubclass) &&
        (proc->subproc == pfnSubclass)) {
          proc->ref = dwRef;
          return TRUE;
      }
      proc = proc->next;
    }
  }

  proc = (SUBCLASSPROCS*)Alloc(sizeof(SUBCLASSPROCS));
  if (!proc) {
    ERR ("Failed to allocate subclass entry in stack\n");
    if (gxIsWindowUnicode (hWnd))
      gxSetWindowLongPtrW (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    else
      gxSetWindowLongPtrA (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    Free (stack);
    gxRemovePropW( hWnd, COMCTL32_wSubclass );
    return FALSE;
  }

  proc->subproc = pfnSubclass;
  proc->ref = dwRef;
  proc->id = uIDSubclass;
  proc->next = stack->SubclassProcs;
  stack->SubclassProcs = proc;

  return TRUE;
}


/***********************************************************************
* GetWindowSubclass [COMCTL32.411]
*
* Gets the Reference data from a subclass.
*
* PARAMS
*     hWnd [in] Handle to window which were subclassing
*     pfnSubclass [in] Pointer to the subclass procedure
*     uID [in] Unique indentifier of the subclassing procedure
*     pdwRef [out] Pointer to the reference data
*
* RETURNS
*     Success: Non-zero
*     Failure: 0
*/

GXBOOL GXDLLAPI gxGetWindowSubclass (GXHWND hWnd, GXSUBCLASSPROC pfnSubclass,
                 GXUINT_PTR uID, GXDWORD_PTR *pdwRef)
{
  const SUBCLASS_INFO *stack;
  const SUBCLASSPROCS *proc;

  ASSERT(gxGetCurrentThreadId() == gxGetWindowThreadProcessId(hWnd, NULL));
  //TRACE ("(%p, %p, %lx, %p)\n", hWnd, pfnSubclass, uID, pdwRef);

  /* See if we have been called for this window */
  stack = (LPSUBCLASS_INFO)gxGetPropW (hWnd, COMCTL32_wSubclass);
  if (!stack)
    return FALSE;

  proc = stack->SubclassProcs;
  while (proc) {
    if ((proc->id == uID) &&
      (proc->subproc == pfnSubclass)) {
        *pdwRef = proc->ref;
        return TRUE;
    }
    proc = proc->next;
  }

  return FALSE;
}


/***********************************************************************
* RemoveWindowSubclass [COMCTL32.412]
*
* Removes a window subclass.
*
* PARAMS
*     hWnd [in] Handle to the window were subclassing
*     pfnSubclass [in] Pointer to the subclass procedure
*     uID [in] Unique identifier of this subclass
*
* RETURNS
*     Success: non-zero
*     Failure: zero
*/

GXBOOL GXDLLAPI gxRemoveWindowSubclass(GXHWND hWnd, GXSUBCLASSPROC pfnSubclass, GXUINT_PTR uID)
{
  LPSUBCLASS_INFO stack;
  LPSUBCLASSPROCS prevproc = NULL;
  LPSUBCLASSPROCS proc;
  GXBOOL ret = FALSE;

  ASSERT(gxGetCurrentThreadId() == gxGetWindowThreadProcessId(hWnd, NULL));
  TRACE ("(%p, %p, %lx)\n", hWnd, pfnSubclass, uID);

  /* Find the Subclass to remove */
  stack = (LPSUBCLASS_INFO)gxGetPropW (hWnd, COMCTL32_wSubclass);
  if (!stack) {
    return FALSE;
  }

  proc = stack->SubclassProcs;
  while (proc) {
    if ((proc->id == uID) && (proc->subproc == pfnSubclass)) {

        if (!prevproc)
        {
          stack->SubclassProcs = proc->next;
        }
        else
        {
          prevproc->next = proc->next;
        }

        if (stack->stackpos == proc)
          stack->stackpos = stack->stackpos->next;

        Free (proc);
        ret = TRUE;
        break;
    }
    prevproc = proc;
    proc = proc->next;
  }

  if (!stack->SubclassProcs && !stack->running) {
    TRACE("Last Subclass removed, cleaning up\n");
    /* clean up our heap and reset the original window procedure */
    if (gxIsWindowUnicode (hWnd))
      gxSetWindowLongPtrW (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    else
      gxSetWindowLongPtrA (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    Free (stack);
    gxRemovePropW( hWnd, COMCTL32_wSubclass );
  }

  return ret;
}

/***********************************************************************
* COMCTL32_SubclassProc (internal)
*
* Window procedure for all subclassed windows. 
* Saves the current subclassing stack position to support nested messages
*/
GXLRESULT GXDLLAPI COMCTL32_SubclassProc (GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  LPSUBCLASS_INFO stack;
  LPSUBCLASSPROCS proc;
  GXLRESULT ret;

  //TRACE ("(%p, 0x%08x, 0x%08lx, 0x%08lx)\n", hWnd, uMsg, wParam, lParam);

  stack = (LPSUBCLASS_INFO)gxGetPropW (hWnd, COMCTL32_wSubclass);
  if (!stack) {
    ERR ("Our sub classing stack got erased for %p!! Nothing we can do\n", hWnd);
    return 0;
  }

  /* Save our old stackpos to properly handle nested messages */
  proc = stack->stackpos;
  stack->stackpos = stack->SubclassProcs;
  stack->running++;
  ret = gxDefSubclassProc(hWnd, uMsg, wParam, lParam);
  stack->running--;
  stack->stackpos = proc;

  if (!stack->SubclassProcs && !stack->running) {
    TRACE("Last Subclass removed, cleaning up\n");
    /* clean up our heap and reset the original window procedure */
    if (gxIsWindowUnicode (hWnd))
    {
      gxSetWindowLongPtrW (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    }
    else
    {
      gxSetWindowLongPtrA (hWnd, GXGWLP_WNDPROC, (GXDWORD_PTR)stack->origproc);
    }
    Free (stack);
    gxRemovePropW( hWnd, COMCTL32_wSubclass );
  }
  return ret;
}

/***********************************************************************
* DefSubclassProc [COMCTL32.413]
*
* Calls the next window procedure (ie. the one before this subclass)
*
* PARAMS
*     hWnd [in] The window that we're subclassing
*     uMsg [in] Message
*     wParam [in] GXWPARAM
*     lParam [in] GXLPARAM
*
* RETURNS
*     Success: non-zero
*     Failure: zero
*/

GXLRESULT GXDLLAPI gxDefSubclassProc (GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  LPSUBCLASS_INFO stack;
  GXLRESULT ret;

  //TRACE ("(%p, 0x%08x, 0x%08lx, 0x%08lx)\n", hWnd, uMsg, wParam, lParam);

  /* retrieve our little stack from the Properties */
  stack = (LPSUBCLASS_INFO)gxGetPropW (hWnd, COMCTL32_wSubclass);
  if (!stack) {
    ERR ("Our sub classing stack got erased for %p!! Nothing we can do\n", hWnd);
    return 0;
  }

  /* If we are at the end of stack then we have to call the original
  * window procedure */
  if (!stack->stackpos) {
    if (gxIsWindowUnicode (hWnd))
      ret = gxCallWindowProcW (stack->origproc, hWnd, uMsg, wParam, lParam);
    else
      ret = gxCallWindowProcA (stack->origproc, hWnd, uMsg, wParam, lParam);
  } else {
    const SUBCLASSPROCS *proc = stack->stackpos;
    stack->stackpos = stack->stackpos->next; 
    /* call the Subclass procedure from the stack */
    ret = proc->subproc (hWnd, uMsg, wParam, lParam,
      proc->id, proc->ref);
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////

GXHWND    GXDLLAPI gxCreateWindowExW    (GXLPCWSTR lpClassName, GXLPCWSTR lpWindowName, GXDWORD dwStyle,
                    GXINT x, GXINT y, GXINT nWidth, GXINT nHeight, GXHWND hWndParent,
                    GXHMENU hMenu, GXHINSTANCE hInstance, GXLPVOID lpParam);
GXHWND    GXDLLAPI gxCreateWindowW    (GXLPCWSTR lpWindowName, GXDWORD dwStyle,
                    GXINT x, GXINT y, GXINT nWidth, GXINT nHeight, GXHWND hWndParent,
                    GXHMENU hMenu, GXHINSTANCE hInstance, GXLPVOID lpParam);

GXHWND gxCOMCTL32_CreateToolTip(GXHWND hwndOwner)
{
  GXHWND hwndToolTip;

  hwndToolTip = gxCreateWindowExW(0, TOOLTIPS_CLASSW, NULL, GXWS_POPUP,
    GXCW_USEDEFAULT, GXCW_USEDEFAULT,
    GXCW_USEDEFAULT, GXCW_USEDEFAULT, hwndOwner,
    0, 0, 0);

  /* Send NM_TOOLTIPSCREATED notification */
  if (hwndToolTip)
  {
    GXNMTOOLTIPSCREATED nmttc;
    /* true owner can be different if hwndOwner is a child window */
    GXHWND hwndTrueOwner = gxGetWindow(hwndToolTip, GXGW_OWNER);
    nmttc.hdr.hwndFrom = hwndTrueOwner;
    nmttc.hdr.idFrom = (GXUINT)gxGetWindowLongW(hwndTrueOwner, GXGWLP_ID);
    nmttc.hdr.code = GXNM_TOOLTIPSCREATED;
    nmttc.hwndToolTips = hwndToolTip;

    gxSendMessageW(gxGetParent(hwndTrueOwner), GXWM_NOTIFY,
      (GXWPARAM)gxGetWindowLongPtrW(hwndTrueOwner, GXGWLP_ID),
      (GXLPARAM)&nmttc);
  }

  return hwndToolTip;
}

GXVOID gxCOMCTL32_RefreshSysColors()
{
  comctl32_color.clrBtnHighlight = gxGetSysColor (GXCOLOR_BTNHIGHLIGHT);
  comctl32_color.clrBtnShadow = gxGetSysColor (GXCOLOR_BTNSHADOW);
  comctl32_color.clrBtnText = gxGetSysColor (GXCOLOR_BTNTEXT);
  comctl32_color.clrBtnFace = gxGetSysColor (GXCOLOR_BTNFACE);
  comctl32_color.clrHighlight = gxGetSysColor (GXCOLOR_HIGHLIGHT);
  comctl32_color.clrHighlightText = gxGetSysColor (GXCOLOR_HIGHLIGHTTEXT);
  comctl32_color.clrHotTrackingColor = gxGetSysColor (GXCOLOR_HOTLIGHT);
  comctl32_color.clr3dHilight = gxGetSysColor (GXCOLOR_3DHILIGHT);
  comctl32_color.clr3dShadow = gxGetSysColor (GXCOLOR_3DSHADOW);
  comctl32_color.clr3dDkShadow = gxGetSysColor (GXCOLOR_3DDKSHADOW);
  comctl32_color.clr3dFace = gxGetSysColor (GXCOLOR_3DFACE);
  comctl32_color.clrWindow = gxGetSysColor (GXCOLOR_WINDOW);
  comctl32_color.clrWindowText = gxGetSysColor (GXCOLOR_WINDOWTEXT);
  comctl32_color.clrGrayText = gxGetSysColor (GXCOLOR_GRAYTEXT);
  comctl32_color.clrActiveCaption = gxGetSysColor (GXCOLOR_ACTIVECAPTION);
  comctl32_color.clrInfoBk = gxGetSysColor (GXCOLOR_INFOBK);
  comctl32_color.clrInfoText = gxGetSysColor (GXCOLOR_INFOTEXT);
}

GXBOOL gxCOMCTL32_IsReflectedMessage(GXUINT uMsg)
{
  return false;
}
/***********************************************************************
* COMCTL32_DrawInsertMark [NOT AN API]
*
* Draws an insertion mark (which looks similar to an 'I').
*
* PARAMS
*     hDC           [I] Device context to draw onto.
*     lpRect        [I] Co-ordinates of insertion mark.
*     clrInsertMark [I] Colour of the insertion mark.
*     bHorizontal   [I] True if insert mark should be drawn horizontally,
*                       vertical otherwise.
*
* RETURNS
*     none
*
* NOTES
*     Draws up to but not including the bottom co-ordinate when drawing
*     vertically or the right co-ordinate when horizontal.
*/
void gxCOMCTL32_DrawInsertMark(GXHDC hDC, const GXRECT *lpRect, GXCOLORREF clrInsertMark, GXBOOL bHorizontal)
{
  GXHPEN hPen = gxCreatePen(GXPS_SOLID, 1, clrInsertMark);
  GXHPEN hOldPen;
  static const GXDWORD adwPolyPoints[] = {4,4,4};
  GXLONG lCentre = (bHorizontal ? 
    lpRect->top + (lpRect->bottom - lpRect->top)/2 : 
  lpRect->left + (lpRect->right - lpRect->left)/2);
  GXLONG l1 = (bHorizontal ? lpRect->left : lpRect->top);
  GXLONG l2 = (bHorizontal ? lpRect->right : lpRect->bottom);
  const GXPOINT aptInsertMark[] =
  {
    /* top (V) or left (H) arrow */
    {lCentre    , l1 + 2},
    {lCentre - 2, l1    },
    {lCentre + 3, l1    },
    {lCentre + 1, l1 + 2},
    /* middle line */
    {lCentre    , l2 - 2},
    {lCentre    , l1 - 1},
    {lCentre + 1, l1 - 1},
    {lCentre + 1, l2 - 2},
    /* bottom (V) or right (H) arrow */
    {lCentre    , l2 - 3},
    {lCentre - 2, l2 - 1},
    {lCentre + 3, l2 - 1},
    {lCentre + 1, l2 - 3},
  };
  hOldPen = (GXHPEN)gxSelectObject(hDC, hPen);
  gxPolyPolyline(hDC, aptInsertMark, adwPolyPoints, sizeof(adwPolyPoints)/sizeof(adwPolyPoints[0]));
  gxSelectObject(hDC, hOldPen);
  gxDeleteObject(hPen);
}
/***********************************************************************
* COMCTL32_EnsureBitmapSize [internal]
*
* If needed, enlarge the bitmap so that the width is at least cxMinWidth and
* the height is at least cyMinHeight. If the bitmap already has these
* dimensions nothing changes.
*
* PARAMS
*     hBitmap       [I/O] Bitmap to modify. The handle may change
*     cxMinWidth    [I]   If the width of the bitmap is smaller, then it will
*                         be enlarged to this value
*     cyMinHeight   [I]   If the height of the bitmap is smaller, then it will
*                         be enlarged to this value
*     cyBackground  [I]   The color with which the new area will be filled
*
* RETURNS
*     none
*/
void gxCOMCTL32_EnsureBitmapSize(GXHBITMAP *pBitmap, int cxMinWidth, int cyMinHeight, GXCOLORREF crBackground)
{
  int cxNew, cyNew;
  GXBITMAP bmp;
  GXHBITMAP hNewBitmap;
  GXHBITMAP hNewDCBitmap, hOldDCBitmap;
  GXHBRUSH hNewDCBrush;
  GXHDC hdcNew, hdcOld;

  if (!gxGetObjectW(*pBitmap, sizeof(GXBITMAP), &bmp))
    return;
  cxNew = (int)(cxMinWidth > bmp.bmWidth ? cxMinWidth : bmp.bmWidth);
  cyNew = (int)(cyMinHeight > bmp.bmHeight ? cyMinHeight : bmp.bmHeight);
  if (cxNew == bmp.bmWidth && cyNew == bmp.bmHeight)
    return;

  hdcNew = gxCreateCompatibleDC(NULL);
  hNewBitmap = gxCreateBitmap(cxNew, cyNew, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
  hNewDCBitmap = (GXHBITMAP)gxSelectObject(hdcNew, hNewBitmap);
  hNewDCBrush = (GXHBRUSH)gxSelectObject(hdcNew, gxCreateSolidBrush(crBackground));

  hdcOld = gxCreateCompatibleDC(NULL);
  hOldDCBitmap = (GXHBITMAP)gxSelectObject(hdcOld, *pBitmap);

  gxBitBlt(hdcNew, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcOld, 0, 0, GXSRCCOPY);
  if (bmp.bmWidth < cxMinWidth)
    gxPatBlt(hdcNew, bmp.bmWidth, 0, cxNew, bmp.bmHeight, GXPATCOPY);
  if (bmp.bmHeight < cyMinHeight)
    gxPatBlt(hdcNew, 0, bmp.bmHeight, bmp.bmWidth, cyNew, GXPATCOPY);
  if (bmp.bmWidth < cxMinWidth && bmp.bmHeight < cyMinHeight)
    gxPatBlt(hdcNew, bmp.bmWidth, bmp.bmHeight, cxNew, cyNew, GXPATCOPY);

  gxSelectObject(hdcNew, hNewDCBitmap);
  gxDeleteObject(gxSelectObject(hdcNew, hNewDCBrush));
  gxDeleteDC(hdcNew);
  gxSelectObject(hdcOld, hOldDCBitmap);
  gxDeleteDC(hdcOld);

  gxDeleteObject(*pBitmap);    
  *pBitmap = hNewBitmap;
  return;
}

GXHBITMAP GXDLLAPI gxCreateMappedBitmap(
               GXHINSTANCE hInstance,   
               int idBitmap,   
               GXUINT wFlags,   
               GXLPCOLORMAP lpColorMap,   
               int iNumMaps  
               )
{
  ASSERT(false);
  return NULL;
}
#endif // _DEV_DISABLE_UI_CODE