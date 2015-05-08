#ifndef _DEV_DISABLE_UI_CODE
#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "GrapX/GXUser.H"
#include "GrapX/GXKernel.H"
#include "GXStation.H"
#include "GrapX/gxDevice.H"
#include <User/GXWindow.h>

GXHGLOBAL g_hClipboard = NULL;  // TODO: 改为线程独立的

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxEmptyClipboard()
{
#if defined(_WINDOWS) || defined(_WIN32)
  return EmptyClipboard();
#else
  return FALSE;
#endif // #if defined(_WINDOWS) || defined(_WIN32)
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxOpenClipboard(
           GXHWND hWndNewOwner   // handle to window opening clipboard  
           )
{
#if defined(_WINDOWS) || defined(_WIN32)
  GXLPWND lpWnd = GXWND_PTR(hWndNewOwner);
  return OpenClipboard(GXLPWND_STATION_PTR(lpWnd)->hBindWin32Wnd);
#else
  return FALSE;
#endif // #if defined(_WINDOWS) || defined(_WIN32)
}
//////////////////////////////////////////////////////////////////////////
GXHANDLE GXDLLAPI gxSetClipboardData(
            GXUINT uFormat,  // clipboard format  
            GXHANDLE hMem   // data handle 
            )
{
#if defined(_WINDOWS) || defined(_WIN32)
  return (GXHANDLE)SetClipboardData((GXUINT)uFormat, (HANDLE)hMem);
#else
  return NULL;
#endif // #if defined(_WINDOWS) || defined(_WIN32)
}
//////////////////////////////////////////////////////////////////////////
GXHANDLE GXDLLAPI gxGetClipboardData(
            GXUINT uFormat   // clipboard format  
            )
{
#if defined(_WINDOWS) || defined(_WIN32)
  HANDLE handle = GetClipboardData((GXUINT)uFormat);
  if(handle)
  {
    LPVOID ptr = GlobalLock(handle);
    SIZE_T size = GlobalSize(handle);

    if(g_hClipboard) {
      gxGlobalFree(g_hClipboard);
    }

    g_hClipboard = gxGlobalAlloc(NULL, size);
    GXLPVOID pNative = gxGlobalLock(g_hClipboard);
    memcpy(pNative, ptr, size);
    gxGlobalUnlock(g_hClipboard);
    GlobalUnlock(handle);
    return (GXHANDLE)g_hClipboard;
  }
  return NULL;
#endif // #if defined(_WINDOWS) || defined(_WIN32)
}
//////////////////////////////////////////////////////////////////////////
#if defined(_WINDOWS) || defined(_WIN32)
GXBOOL GXDLLAPI gxCloseClipboard()
{
  if(g_hClipboard) {
    gxGlobalFree(g_hClipboard);
    g_hClipboard = NULL;
  }

  return CloseClipboard();
}

GXBOOL GXDLLAPI gxIsClipboardFormatAvailable(
  GXUINT format   // clipboard format  
  )
{
  return IsClipboardFormatAvailable(format);
}
#else
GXBOOL GXDLLAPI gxCloseClipboard()
{
  return FALSE;
}

GXBOOL GXDLLAPI gxIsClipboardFormatAvailable(
  GXUINT format   // clipboard format  
  )
{
  return FALSE;
}
#endif // #if defined(_WINDOWS) || defined(_WIN32)

#endif // _DEV_DISABLE_UI_CODE