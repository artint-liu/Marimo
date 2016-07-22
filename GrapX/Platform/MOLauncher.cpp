#if defined(_WINDOWS) && ! defined(__clang__)
#include <windows.h>

#include "GrapX.H"
#include "GXApp.H"
#include "res/resource.h"

#include <Smart/smartstream.h>
#include <Smart/Stock.h>
#include <clPathFile.H>

//#pragma comment(lib, "user32.lib")

struct PLATFORMLIST
{
  LPCTSTR szName;
  int     idEnum;
};

PLATFORMLIST g_PlatformList[] = {
  {_T("Win32 Direct3D 9.0c"),   GXPLATFORM_WIN32_DIRECT3D9},
  {_T("Win32 Direct3D 11"),     GXPLATFORM_WIN32_DIRECT3D11},
  {_T("Win32 OpenGL 2.0"),      GXPLATFORM_WIN32_OPENGL},
  {_T("X OpenGL ES 2.0 (Emu)"), GXPLATFORM_X_OPENGLES2},
  {NULL, 0},
};

INT_PTR CALLBACK PlatformSelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    {
      HWND hList = GetDlgItem(hDlg, IDC_PLATFORMLIST);
      int nSel = 0;
      for(int i = 0;; i++)
      {
        if(g_PlatformList[i].szName == NULL)
          break;
        int nItem = (int)SendMessage(hList, CB_ADDSTRING, NULL, (LPARAM)g_PlatformList[i].szName);
        SendMessage(hList, CB_SETITEMDATA, nItem, g_PlatformList[i].idEnum);
      }
      int nCount = (int)SendMessage(hList, CB_GETCOUNT, 0, 0);
      GXAPP_DESC* pDesc = (GXAPP_DESC*)lParam;
      for(int i = 0; i < nCount; i++)
      {
        if(SendMessage(hList, CB_GETITEMDATA, i, NULL) == pDesc->idPlatform) {
          SendMessage(hList, CB_SETCURSEL, i, 0);
          break;
        }
      }

      // -------------------这么写可以防止没有定义"_WIN32"或者"_WIN64"的情况,
      // -------------------这时会报编译错误.
      SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);  // GXAPP_DESC*
    }
    return (INT_PTR)TRUE;
  case WM_CLOSE:
    EndDialog(hDlg, wParam);
    return (INT_PTR)TRUE;
  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK)
    {
      HWND hList = GetDlgItem(hDlg, IDC_PLATFORMLIST);
      int nCurSel = (int)SendMessage(hList, CB_GETCURSEL, 0, 0);
      int idEnum = (int)SendMessage(hList, CB_GETITEMDATA, nCurSel, 0);

      GXAPP_DESC* pDesc = (GXAPP_DESC*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
      pDesc->idPlatform = (GXPlaformIdentity)idEnum;

      SendMessage(hDlg, WM_CLOSE, 0, 0);
    }
    else if (LOWORD(wParam) == IDCANCEL)
    {
      SendMessage(hDlg, WM_CLOSE, -1, 0);
    }
    break;
  }
  return (INT_PTR)FALSE;
}

GXBOOL GXDLLAPI MOUICreatePlatformSelectedDlg(HINSTANCE hInstance, GXAPP_DESC* pDesc)
{
  //typedef clstd::SmartProfileA::HANDLE sHANDLE;
  typedef clstd::StockA::Section Section;
  clstd::StockA sp;
  GXWCHAR buffer[MAX_PATH];
  clStringW strProfile;
  GetModuleFileNameW(hInstance, buffer, MAX_PATH);
  strProfile = buffer;
  clpathfile::RenameExtensionW(strProfile, L".smartfile");
  pDesc->idPlatform = GXPLATFORM_WIN32_DIRECT3D9;
  if(sp.LoadW(strProfile)) {
    Section handle = sp.Open("Startup/Info");
    if(handle.IsValid())
    {
      clStringW strPlatform = handle.GetKeyAsString("Platform", "");
      if( ! strPlatform.IsEmpty()) {
        pDesc->idPlatform = MOPlatformStringToEnumW(strPlatform);
      }
    }
    //sp.FindClose(handle);
    handle.Close();
    sp.Close();
  }

  extern HINSTANCE g_hDLLModule;
  INT_PTR val = DialogBoxParam(
    g_hDLLModule, MAKEINTRESOURCE(IDD_PLATFORM), 
    NULL, PlatformSelDlgProc, (LPARAM)pDesc);

  if(val == 0)
  {
    Section handle = sp.Create("Startup/Info");
    MOPlatformEnumToStringW(pDesc->idPlatform, buffer, MAX_PATH);
    clStringA strPlatform = buffer;
    handle.SetKey("Platform", strPlatform);
    //sp.Close(handle);
    handle.Close();
    sp.SaveW(strProfile);
  }

  return val == 0;
}
#endif // #if defined(_WINDOWS)