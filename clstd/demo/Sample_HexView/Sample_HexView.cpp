// Sample_FileView.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <clstd.h>
#include <clUtility.h>
//#include <clFile.h>

#ifdef _DEBUG
# pragma comment(lib, "clstd_d.lib")
#else
# pragma comment(lib, "clstd.lib")
#endif

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL OpenFile(LPCWSTR szFilepath);
BOOL ReadBuffer(int nNumOfBlocks);

LPCWSTR szTitle = _T("HexViewer");
LPCWSTR szWindowClass = _T("HEXVIEWER");
HINSTANCE hInst;
clstd::File file;
clstd::MemBuffer HexBuffer;
int nNumOfBlocks = 32;

//int main(int argc, char* argv[])
int APIENTRY _tWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd )
{
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);

  if(argc != 1) {
    printf("HexView <filename>");
    return -1;
  }

  OpenFile(argv[0]);
  ReadBuffer(nNumOfBlocks);

  SetProcessDPIAware();

  MyRegisterClass(hInstance);
  if( ! InitInstance(hInstance, nShowCmd)) {
    return -1;
  }

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  file.Close(); // 其实没必要
	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = NULL;

  return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWnd;

  hInst = hInstance; // 将实例句柄存储在全局变量中

  hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

  if (!hWnd)
  {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

void OnPaint(HWND hWnd, HDC hdc)
{
  HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, L"Consolas");
  HGDIOBJ hOldFont = SelectObject(hdc, hFont);
  RECT rcClient;
  RECT rcText = {};
  GetClientRect(hWnd, &rcClient);

  DrawTextA(hdc, (char*)HexBuffer.GetPtr(), HexBuffer.GetSize(), &rcText, DT_LEFT | DT_TOP | DT_CALCRECT);
  nNumOfBlocks = rcClient.bottom / (rcText.bottom / nNumOfBlocks);
  ReadBuffer(nNumOfBlocks);

  DrawTextA(hdc, (char*)HexBuffer.GetPtr(), HexBuffer.GetSize(), &rcClient, DT_LEFT | DT_TOP);
  SelectObject(hdc, hOldFont);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  //char buffer[1024];

  //int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message)
  {
  case WM_COMMAND:
  //  wmId = LOWORD(wParam);
  //  wmEvent = HIWORD(wParam);
  //  // 分析菜单选择:
  //  switch (wmId)
  //  {
  //  case IDM_ABOUT:
  //    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
  //    break;
  //  case IDM_EXIT:
  //    DestroyWindow(hWnd);
  //    break;
  //  default:
  //    return DefWindowProc(hWnd, message, wParam, lParam);
  //  }
  //  break;

  //case WM_LBUTTONDBLCLK:
  //  if (g_fScale == 1.0f) {
  //    UpdateScale();
  //  }
  //  else {
  //    g_fScale = 1.0f;
  //  }
  //  UpdateOrigin();
  //  InvalidateRect(hWnd, NULL, FALSE);

    break;

  case WM_SIZE:
    //g_client_size.cx = LOWORD(lParam);
    //g_client_size.cy = HIWORD(lParam);
    //UpdateScale();
    //UpdateOrigin();
    break;

  case WM_LBUTTONDOWN:
  {
    //MSG msg;
    //POINT pt;
    //SetCapture(hWnd);
    //GetCursorPos(&pt);
    //POINT ptPrevOrigin = g_ptOrigin;

    //while (GetMessage(&msg, NULL, 0, 0))
    //{
    //  if (msg.message == WM_LBUTTONUP)
    //  {
    //    ReleaseCapture();
    //    break;
    //  }
    //  else if (msg.message == WM_MOUSEMOVE)
    //  {
    //    g_ptOrigin.x = ptPrevOrigin.x + msg.pt.x - pt.x;
    //    g_ptOrigin.y = ptPrevOrigin.y + msg.pt.y - pt.y;

    //    LONG half_client_width = g_client_size.cx / 2;
    //    LONG half_client_height = g_client_size.cy / 2;

    //    g_ptOrigin.x = cl_min(g_ptOrigin.x, half_client_width);
    //    g_ptOrigin.y = cl_min(g_ptOrigin.y, half_client_height);

    //    if (g_pBitmap)
    //    {
    //      LONG vis_width = (LONG)(g_pBitmap->GetWidth() * g_fScale + 0.5f);
    //      LONG vis_height = (LONG)(g_pBitmap->GetHeight() * g_fScale + 0.5f);
    //      LONG right = (LONG)(g_ptOrigin.x + vis_width);
    //      LONG bottom = (LONG)(g_ptOrigin.y + vis_height);
    //      if (right < half_client_width) {
    //        g_ptOrigin.x = cl_max(right, half_client_width) - vis_width;
    //      }
    //      if (bottom < half_client_height) {
    //        g_ptOrigin.y = cl_max(bottom, half_client_height) - vis_height;
    //      }
    //    }
    //    InvalidateRect(hWnd, NULL, FALSE);
    //  }
    //  TranslateMessage(&msg);
    //  DispatchMessage(&msg);
    //}

  }
  break;

  case WM_DROPFILES:
  {
    WCHAR szFilepath[MAX_PATH];
    DragQueryFile((HDROP)wParam, 0, szFilepath, MAX_PATH);
    OpenFile(szFilepath);
    InvalidateRect(hWnd, NULL, FALSE);
  }
  break;

  case WM_MOUSEWHEEL:
  {
    //int nDelta = (int)(short)HIWORD(wParam);
    //float fOldScale = g_fScale;
    //if (nDelta > 0)
    //{
    //  g_fScale = g_fScale * 0.5f;
    //  g_fScale = cl_max(g_fScale, 1.0f / 16.0f);
    //}
    //else if (nDelta < 0)
    //{
    //  g_fScale = g_fScale * 2.0f;
    //  g_fScale = cl_min(g_fScale, 1.0f * 32.0f);
    //}

    //if (g_fScale != fOldScale)
    //{
    //  POINT pt = { (LONG)(short)LOWORD(lParam), (LONG)(short)HIWORD(lParam) };
    //  ScreenToClient(hWnd, &pt);
    //  sprintf(buffer, "%d, %d", pt.x, pt.y);

    //  //pt.x = g_client_size.cx / 2;
    //  //pt.y = g_client_size.cy / 2;
    //  FocusImage(fOldScale, g_fScale, pt);

    //  //(g_fScale / fOldScale)
    //  //g_ptOrigin.x = (pt.x - g_ptOrigin.x) / fOldScale - pt.x / g_fScale;
    //  //g_ptOrigin.y = (pt.y - g_ptOrigin.y) / fOldScale - pt.y / g_fScale;
    //  //g_ptOrigin.y -= (pt.y - g_ptOrigin.y) / fOldScale - (pt.y - g_ptOrigin.y) / fOldScale * g_fScale;

    //  //g_ptOrigin.x = (LONG)(((g_ptOrigin.x + pt.x) / fOldScale) - pt.x / g_fScale + 0.5f);
    //  //g_ptOrigin.y = (LONG)(((g_ptOrigin.y + pt.y) / fOldScale) - pt.y / g_fScale + 0.5f);

    //  InvalidateRect(hWnd, NULL, FALSE);
    //}
  }
  break;

  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    OnPaint(hWnd, hdc);
    EndPaint(hWnd, &ps);
    break;

  case WM_CREATE:
    DragAcceptFiles(hWnd, TRUE);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL OpenFile(LPCWSTR szFilepath)
{
  //const size_t cbFileSizeLimit = 100 * 1024 * 1024;

  if (!file.OpenExisting(szFilepath)) {
    CLOG_ERROR("can not open file(%s).", szFilepath);
    return FALSE;
  }

  //u32 nFileSize = 0;
  //if (file.GetSize(&nFileSize) > cbFileSizeLimit) {
  //  CLOG_ERROR("file is too large to show(size:%l).", ((u64)nFileSize << 32) | file.GetSize(NULL));
  //  return -3;
  //}

  //const size_t cbViewSize = 512;
  //clstd::MemBuffer buf;
  //if (file.ReadToBuffer(&buf, 0, cbViewSize)) {
  //  HexBuffer.Resize(clstd::ViewMemory16(NULL, 0, buf.GetPtr(), cbViewSize, buf.GetPtr()), FALSE);
  //  clstd::ViewMemory16((ch*)HexBuffer.GetPtr(), HexBuffer.GetSize(), buf.GetPtr(), cbViewSize, buf.GetPtr());
  //  //printf((ch*)HexBuffer.GetPtr());
  //  return TRUE;
  //}
  return TRUE;
}

BOOL ReadBuffer(int nNumOfBlocks) // block = 16 bytes
{
  const int cbBlock = 16;
  const int cbViewSize = nNumOfBlocks * cbBlock;
  clstd::MemBuffer buf;
  if (file.ReadToBuffer(&buf, 0, cbViewSize)) {
    HexBuffer.Resize(clstd::ViewMemory16(NULL, 0, buf.GetPtr(), cbViewSize, buf.GetPtr()), FALSE);
    clstd::ViewMemory16((ch*)HexBuffer.GetPtr(), HexBuffer.GetSize(), buf.GetPtr(), cbViewSize, buf.GetPtr());
    return TRUE;
  }
  return FALSE;
}