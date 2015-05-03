#define _GRAPHX_FOUNDATION_CLASS_H_ // 先禁止这个文件
#ifndef _GRAPHX_FOUNDATION_CLASS_H_
#define _GRAPHX_FOUNDATION_CLASS_H_

//_traceA
//_gx_assert
#ifdef _GXUI
//#include <PreDefine.H>
#include <GrapX.H>
#include "GXUser.H"

//
// List View
//
typedef GXLV_ITEM LV_ITEM;
#define LVIF_TEXT GXLVIF_TEXT
#define LVIF_TEXT GXLVIF_TEXT
#define LVIF_IMAGE GXLVIF_IMAGE
#define LVIF_PARAM GXLVIF_PARAM
#define LVIF_STATE GXLVIF_STATE
#define LVIF_INDENT GXLVIF_INDENT
#define LVIF_NORECOMPUTE GXLVIF_NORECOMPUTE
#define LVIF_GROUPID GXLVIF_GROUPID
#define LVIF_COLUMNS GXLVIF_COLUMNS
//#define LVIF_COLFMT GXLVIF_COLFMT

#define LVM_GETITEMCOUNT GXLVM_GETITEMCOUNT
#define LVM_INSERTCOLUMN GXLVM_INSERTCOLUMN
#define LVM_INSERTITEM GXLVM_INSERTITEM
#define LVM_INSERTITEM GXLVM_INSERTITEM
#define LVM_SETITEM GXLVM_SETITEM
//
// Tree View
//
#define TVGN_ROOT GXTVGN_ROOT
#define TVGN_NEXT GXTVGN_NEXT
#define TVGN_PREVIOUS GXTVGN_PREVIOUS
#define TVGN_PARENT GXTVGN_PARENT
#define TVGN_CHILD GXTVGN_CHILD
#define TVGN_FIRSTVISIBLE GXTVGN_FIRSTVISIBLE
#define TVGN_NEXTVISIBLE GXTVGN_NEXTVISIBLE
#define TVGN_PREVIOUSVISIBLE GXTVGN_PREVIOUSVISIBLE
#define TVGN_DROPHILITE GXTVGN_DROPHILITE
#define TVGN_CARET GXTVGN_CARET
#define TVGN_LASTVISIBLE GXTVGN_LASTVISIBLE
#define TVGN_NEXTSELECTED GXTVGN_NEXTSELECTED

#else
#include <Windows.h>
#include <CommCtrl.h>
#include <gxBaseTypes.H>
#include <clstd.h>

#define GXWC_BUTTONW    L"Button"
#define GXWC_EDITW      L"Edit"
#define GXWC_LISTBOXW    L"ListBox"
#define GXWC_SCROLLBARW    L"ScrollBar"
#define GXWC_STATICW    L"Static"
#define GXWC_LISTVIEWW    L"SysListView32"
#define GXWC_TREEVIEWW    L"SysTreeView32"

#define gxEnumChildWindows    EnumChildWindows
#define gxCreateDialogParamW  CreateDialogParamW
#define gxDialogBoxParamW    DialogBoxParamW
#define gxDefWindowProcW    DefWindowProcW
#define gxDefWindowProc      DefWindowProc
#define gxEndPaint        EndPaint
#define gxBeginPaint      BeginPaint
#define gxSetWindowLong      SetWindowLong
#define gxSetWindowLongW    SetWindowLongW
#define gxGetWindowLong      GetWindowLong
#define gxGetWindowLongW    GetWindowLongW
#define gxGetClassName      GetClassName
#define gxGetClassNameW      GetClassNameW
#define gxGetDlgItem      (::GetDlgItem)
#define gxEndDialog        (::EndDialog)
#define gxGetClientRect      (::GetClientRect)
#define gxDestroyWindow      DestroyWindow
#define gxEnableWindow      EnableWindow
#define gxShowWindow      ShowWindow
#define gxSetWindowTextW    SetWindowTextW
#define gxSetWindowPos      (::SetWindowPos)
#define gxSetCapture      (::SetCapture)
#define gxUpdateWindow      (::UpdateWindow)
#define gxInvalidateRect    (::InvalidateRect)
#define gxSendMessage      SendMessage
#define gxSendMessageW      SendMessageW
#define gxIsWindowVisible    IsWindowVisible
#define gxCheckRadioButton    (::CheckRadioButton)
#define gxSetTimer        (::SetTimer)
#define gxKillTimer        (::KillTimer)

#define GXHIWORD        HIWORD
#define GXLOWORD        LOWORD
#define GXMAKELPARAM    MAKELPARAM


typedef HWND        GXHWND;
typedef HINSTANCE   GXHINSTANCE;
typedef HIMAGELIST  GXHIMAGELIST;

typedef RECT        GXRECT;
typedef LPRECT      GXLPRECT;
typedef SIZE        GXSIZE;
typedef LPSIZE      GXLPSIZE, LPGXSIZE;

typedef DLGPROC     GXDLGPROC;
typedef WNDENUMPROC GXWNDENUMPROC;


//typedef LPARAM    GXLPARAM;
//typedef WPARAM    GXWPARAM;

typedef CREATESTRUCT  GXCREATESTRUCT;
typedef PAINTSTRUCT    GXPAINTSTRUCT;

//
// Tree View
//
typedef NMTREEVIEW GXNMTREEVIEW;

//
// List View
//
#define GXLVM_GETITEM  LVM_GETITEM
#define GXLVM_DELETEALLITEMS LVM_DELETEALLITEMS
#define GXLVM_SETIMAGELIST LVM_SETIMAGELIST
#endif // _GXUI
#else
#include <GrapX.H>
#include "GXUser.H"
#endif // _GRAPHX_FOUNDATION_CLASS_H_