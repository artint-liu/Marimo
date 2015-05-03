#if defined(_WINDOWS) || defined(_WIN32)
#include <windows.h>

BOOL SetClientSize(HWND hWnd, HWND hWndInsertAfter,int X,int Y,int cx,int cy,UINT uFlags)
{
  RECT wrect;
  RECT crect;

  GetWindowRect(hWnd, &wrect);
  wrect.right -= wrect.left;
  wrect.bottom -= wrect.top;
  GetClientRect(hWnd, &crect);
  return SetWindowPos(
    hWnd,
    hWndInsertAfter,
    X,
    Y,
    cx + wrect.right - crect.right, 
    cy + wrect.bottom - crect.bottom,
    uFlags
    );
}

#endif // _WINDOWS