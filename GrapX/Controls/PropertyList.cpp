//#include "stdafx.h"
//#include <tchar.h>
//#include <windows.h>
//#include <commdlg.h>
//#include <uxtheme.h>
//#include <vsstyle.h>

// 全局头文件
#include "GrapX.h"
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXImage.h"
#include "GrapX/GXFont.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GXUICtrlBase.h"
#include "PropertyList.h"
//#include <CommCtrl.h>
//#include "resource.h"

//#ifndef SAFE_DELETE
//#define SAFE_DELETE(x)  if((x) != NULL){delete x; x = NULL;}
//#endif
//#define ASSERT(x)    if(!(x)) __asm int 3
#pragma comment(lib, "comdlg32.lib")
void trace(char* fmt, ...);

//#define TRACE trace
//#pragma comment(lib, "UxTheme.lib")

#define BLACK_COLOR    0xE6fff8
#define WHITE_COLOR    0xFFFFFF
#define SPLIT_COLOR    0x808080
#define THICKNESS    2

#define COLOR_A_DARK  0xE9EEBC
#define COLOR_B_DARK  0x98D7D0
#define COLOR_A_LIGHT 0xF9FECC
#define COLOR_B_LIGHT 0xA8E7E0
#define COLOR_TITLE   0XFFE06E

#define TIMER_ID_SPINUP      100
#define TIMER_ID_SPINDOWN    101
#define TIMER_ID_PAGESLIDELEFT  102
#define TIMER_ID_PAGESLIDERIGHT  103

//using namespace std;

namespace GXUIEXT
{

  namespace PropertyList
  {
    static COLORREF g_crCustom[16];
    GXBOOL DrawUpDownControl(GXHDC hdc, GXLPRECT lpRect, int nBtnState)
    {
      GXRECT rect = *lpRect;
      rect.bottom = (lpRect->top + lpRect->bottom) >> 1;
      gxDrawFrameControl(hdc, &rect, DFC_SCROLL, 
        ((nBtnState & 1) ? (DFCS_SCROLLUP | DFCS_PUSHED) : DFCS_SCROLLUP) |
        ((nBtnState & ITEM::F_DISABLE) ? DFCS_INACTIVE : 0));
      rect.top = rect.bottom;
      rect.bottom = lpRect->bottom;
      gxDrawFrameControl(hdc, &rect, DFC_SCROLL, 
        ((nBtnState & 2) ? (DFCS_SCROLLDOWN | DFCS_PUSHED) : DFCS_SCROLLDOWN) |
        ((nBtnState & ITEM::F_DISABLE) ? DFCS_INACTIVE : 0));

      return TRUE;
    }

    Form::Form(GXLPCWSTR szName, 
      GXLPCWSTR lpWindowName, GXDWORD dwStyle, GXDWORD dwExStyle, int x, int y, 
      int nWidth, int nHeight, GXHWND hWndParent, GXHINSTANCE hInst)
      : GXUI::CtrlBase(szName)
      , m_fwKeys(NULL)
      , m_nPrevTopIndex(NULL)
      , m_hEdit(NULL)
      , m_hListBox(NULL)
      , m_dwOldEditWndProc(NULL)
      , m_nEditing(-1)
      , m_Root(nWidth / 2)
      , m_pCurPage(&m_Root)
      , m_hFont(NULL)
      , m_eLBDown(ITEM::HT_BLANK)
      , m_bChangedNumber(FALSE)
      , m_xLeft(0)
      , m_bShowScorollBar(FALSE)
    {
      m_Root.m_pPropSheet = this;

      m_hBrush[0] = gxCreateSolidBrush(COLOR_A_DARK);
      m_hBrush[1] = gxCreateSolidBrush(COLOR_B_DARK);
      m_hBrush[2] = gxCreateSolidBrush(COLOR_A_LIGHT);
      m_hBrush[3] = gxCreateSolidBrush(COLOR_B_LIGHT);

      m_hPen = gxCreatePen(PS_SOLID, 1, SPLIT_COLOR);
    }

    //PropSheet::~PropSheet()
    //{
    //  Release();
    //}

    GXLRESULT GXCALLBACK Form::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
    {
      GXPAINTSTRUCT ps;
      GXHDC hdc;

      Form* pThis = (Form*)GXUI::CtrlBase::PreClassObj(hWnd, message, lParam);

      switch (message)
      {
      case WM_PAINT:
        hdc = gxBeginPaint(hWnd, &ps);
        pThis->GetCurPage()->DrawItems(hdc);
        gxEndPaint(hWnd, &ps);
        break;
      case WM_LBUTTONDOWN:
        pThis->LButtonDown(wParam, lParam);
        break;

      case GXWM_LBUTTONUP:
        pThis->LButtonUp(wParam, lParam);
        break;

      case GXWM_MOUSEMOVE:
        pThis->MouseMove(wParam, lParam);
        break;
      case GXWM_TIMER:
        pThis->OnTimer((DWORD)wParam);
        break;

      case GXWM_PARENTNOTIFY:
        {
          if(GXLOWORD(wParam) == GXWM_LBUTTONDOWN)
          {
            //gxSendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
            return 0;
          }
          else if(GXLOWORD(wParam) == GXWM_DESTROY)
          {
            pThis->DestroyChildDlg((GXHWND)lParam);
            return 0;
          }
        }
        break;

      case GXWM_ERASEBKGND:
        return 1;

      case GXWM_SIZE:
        pThis->CheckScrolled();
        break;

        //case WM_CREATE:
        //  {
        //    LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        //    gxSetWindowLong(hWnd, 0, (LONG)(LONG _w64)lpcs->lpCreateParams);
        //  }
        //  break;

      case GXWM_SETFONT:
        pThis->SetFont((GXHFONT)wParam);
        break;
      case GXWM_COMMAND:
        if(HIWORD(wParam) == LBN_SELCHANGE)
        {
          pThis->SubmitList(TRUE);
        }
        break;
        //case WM_NCDESTROY:
        //  pThis->Release();
        //  break;
      case GXWM_DESTROYEDITCTL:
        pThis->DestroyEditCtrl();
        pThis->DestroyListCtrl();
        break;

      case PSM_SETDATALAYOUT:
        {
          PROPLIST_DATALAYOUT* pDataLayout = (PROPLIST_DATALAYOUT*)lParam;
          return pThis->Initialize(pDataLayout->pBasePtrList, pDataLayout->pDataDesc);
        }

      case PSM_UPLOADDATA:
        {
          PROPLIST_DATALAYOUT* pDataLayout = (PROPLIST_DATALAYOUT*)lParam;
          return pThis->UploadData(pDataLayout->pBasePtrList, pDataLayout->pDataDesc,
            pDataLayout->nIDFirst, pDataLayout->nIDLast);
        }

      case PSM_DOWNLOADDATA:
        {
          PROPLIST_DATALAYOUT* pDataLayout = (PROPLIST_DATALAYOUT*)lParam;
          return pThis->DownloadData(pDataLayout->pBasePtrList, pDataLayout->pDataDesc,
            pDataLayout->nIDFirst, pDataLayout->nIDLast);
        }

      case GXWM_NCCREATE:
        {
          GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;
          Form* pForm = NULL;
          GXLPCWSTR szIdName = IS_IDENTIFY(lpcs->hMenu) ? NULL : (GXLPCWSTR)lpcs->hMenu;
          pForm = new Form(szIdName, szIdName, lpcs->style, lpcs->dwExStyle,
            lpcs->x, lpcs->y, lpcs->cx, lpcs->cy, lpcs->hwndParent, lpcs->hInstance);
          return pForm->OnNCCreate(hWnd, message, wParam, lParam);
        }
      }
      return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);  
    }

    GXLRESULT Form::SetFont(GXHFONT hFont)
    {
      m_hFont = hFont;
      m_Root.UpdateHeightList();
      gxInvalidateRect(m_hWnd, NULL, TRUE);
      return 0;
    }

    Page* Form::FindPage(GXINT nId)
    {
      if(nId <= 0)
        return &m_Root;
      return m_Root.FindPageById(nId);
    }

    GXLRESULT CALLBACK Form::EditStringWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
    {
      Form* pThis = (Form*)(GXLONG _w64)gxGetWindowLong(hWnd, GXGWL_USERDATA);
      if(message == GXWM_KILLFOCUS && pThis->m_hEdit != NULL)
      {
        pThis->SubmitString(TRUE);
      }
      else if(message == WM_CHAR)
      {
        TCHAR chCharCode = (TCHAR) wParam;

        if(chCharCode == VK_RETURN) {
          pThis->SubmitString(TRUE);
        }
        else if(chCharCode == VK_TAB) {
          pThis->PopupNextEditLabel((gxGetKeyState(GXVK_SHIFT) & 0x8000) ? -1 : 1);
        }
      }
      GXLRESULT lret = gxCallWindowProc((GXWNDPROC)(GXDWORD _w64)pThis->m_dwOldEditWndProc, hWnd, message, wParam, lParam);
      if(message == WM_NCDESTROY)
      {
        pThis->m_dwOldEditWndProc = NULL;
      }
      return lret;
    }

    GXLRESULT GXCALLBACK Form::EditIntegerWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
    {
      Form* pThis = (Form*)(LONG _w64)gxGetWindowLong(hWnd, GXGWL_USERDATA);
      if(message == GXWM_KILLFOCUS && pThis->m_hEdit != NULL)
      {
        pThis->SubmitString(TRUE);
      }
      else if(message == WM_CHAR)
      {
        TCHAR chCharCode = (TCHAR) wParam;
        if(chCharCode == VK_RETURN) {
          pThis->SubmitString(TRUE);
        }
        else if(chCharCode == VK_BACK) {
          goto PROCESS_IT;
        }
        else if(chCharCode == VK_TAB) {
          pThis->PopupNextEditLabel((gxGetKeyState(GXVK_SHIFT) & 0x8000) ? -1 : 1);
        }
        else if( chCharCode != _T('-') && (chCharCode < _T('0') || chCharCode > _T('9')) ) {
          return 0;
        }
      }
PROCESS_IT:
      GXLRESULT lret = gxCallWindowProc((GXWNDPROC)(DWORD _w64)pThis->m_dwOldEditWndProc, hWnd, message, wParam, lParam);
      if(message == WM_NCDESTROY)
      {
        pThis->m_dwOldEditWndProc = NULL;
      }
      return lret;
    }

    GXLRESULT GXCALLBACK Form::EditFloatWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
    {
      Form* pThis = (Form*)(LONG _w64)gxGetWindowLong(hWnd, GXGWL_USERDATA);
      if(message == GXWM_KILLFOCUS && pThis->m_hEdit != NULL)
      {
        pThis->SubmitString(TRUE);
      }
      else if(message == WM_CHAR)
      {
        TCHAR chCharCode = (TCHAR) wParam;
        if(chCharCode == VK_RETURN) {
          pThis->SubmitString(TRUE);
        }
        else if(chCharCode == VK_BACK) {
          goto PROCESS_IT;
        }
        else if(chCharCode == VK_TAB) {
          pThis->PopupNextEditLabel((gxGetKeyState(GXVK_SHIFT) & 0x8000) ? -1 : 1);
        }
        else if( chCharCode != _T('-') && chCharCode != _T('.') && (chCharCode < _T('0') || chCharCode > _T('9'))) {
          return 0;
        }
      }
      else if(message == GXWM_KEYDOWN) {
        TCHAR nVirKey = (TCHAR) wParam;
      }
PROCESS_IT:
      GXLRESULT lret = gxCallWindowProc((GXWNDPROC)(DWORD _w64)pThis->m_dwOldEditWndProc, hWnd, message, wParam, lParam);
      if(message == WM_NCDESTROY)
      {
        pThis->m_dwOldEditWndProc = NULL;
      }
      return lret;
    }

    GXBOOL Form::SubmitString(GXBOOL bDelay)
    {
      if(m_nEditing >= 0 && m_nEditing < (int)m_pCurPage->GetList().size())
      {
        ITEM& item = m_pCurPage->GetList()[m_nEditing];

        if( item.eType != PST_STRING &&
          item.eType != PST_INTEGER &&
          item.eType != PST_UINTEGER &&
          item.eType != PST_FLOAT ) {
            return FALSE;
        }

        if(m_hEdit != NULL)
        {
          INT nLen = gxGetWindowTextLengthA(m_hEdit) + 1;
          CHAR* pbuf = new CHAR[nLen];
          gxGetWindowTextA(m_hEdit, pbuf, nLen);
          if(item.eType == PST_STRING) {
            item.strVal = pbuf;
          }
          else if(item.eType == PST_INTEGER) {
            item.nVal = clstd::xtoi(pbuf);
          }
          else if(item.eType == PST_UINTEGER) {
            item.nVal = clstd::xtou(pbuf);
          }
          else if(item.eType == PST_FLOAT) {
            item.fVal = (float)clstd::xtof(pbuf);
            //sscanf(pbuf, "%f", &item.fVal);  // TODO: 换成clString支持的
          }
          SAFE_DELETE_ARRAY(pbuf);
          NotifyParent(GXNM_RETURN, item);
        }
      }
      if(bDelay != FALSE) {
        gxPostMessage(m_hWnd, GXWM_DESTROYEDITCTL, NULL, NULL);
      }
      else {
        DestroyEditCtrl();
      }
      return TRUE;
    }

    GXBOOL Form::SubmitList(GXBOOL bDelay)
    {
      if(m_nEditing >= 0 && m_nEditing < (int)m_pCurPage->GetList().size())
      {
        ITEM& item = m_pCurPage->GetList()[m_nEditing];

        if(item.eType != PST_LIST)
          return FALSE;

        if(item.pGeneral == NULL || m_hListBox == NULL)
          return FALSE;

        LBItemList*const pList = item.pListBoxItem;
        int nSel = (int)gxSendMessage(m_hListBox, LB_GETCURSEL, 0, 0);
        LPARAM lParam = gxSendMessage(m_hListBox, LB_GETITEMDATA, nSel, 0);

        int n = 0;
        for(LBItemList::iterator it = pList->begin(); it != pList->end(); ++it, n++)
        {
          if(it->lParam == lParam)
          {
            item.nVal = n;
            GXRECT rect;
            m_pCurPage->GetItemRect(m_nEditing, &rect);
            gxInvalidateRect(m_hWnd, &rect, FALSE);
            NotifyParent(GXNM_RETURN, item);
            break;
          }
        }
      }
      if(bDelay != FALSE)
        gxPostMessage(m_hWnd, GXWM_DESTROYEDITCTL, NULL, NULL);
      else
        DestroyListCtrl();
      return TRUE;
    }

    GXBOOL Form::DestroyEditCtrl()
    {
      if(m_hEdit != NULL)
      {
        GXHWND hEdit = m_hEdit;
        m_hEdit = NULL;
        m_nEditing = -1;
        //m_dwOldEditWndProc = NULL;
        gxDestroyWindow(hEdit);
      }
      return TRUE;
    }

    GXBOOL Form::DestroyListCtrl()
    {
      if(m_hListBox != NULL)
      {
        GXHWND hListBox = m_hListBox;
        m_hListBox = NULL;
        m_nEditing = -1;
        gxDestroyWindow(hListBox);
      }
      return TRUE;
    }

    void Form::LButtonDown(GXWPARAM wParam, GXLPARAM lParam)
    {
      gxSetCapture(m_hWnd);
      m_nPrevTopIndex = m_pCurPage->GetTopIndex();
      m_ptLButton.x = (short)LOWORD(lParam);
      m_ptLButton.y = (short)HIWORD(lParam);
      m_fwKeys |= MK_LBUTTON;

      GXRECT rcItem;
      int nItem = m_pCurPage->HitTest(m_ptLButton.x, m_ptLButton.y);
      if(nItem >= 0 && nItem < (int)m_pCurPage->GetList().size())
      {
        m_pCurPage->GetItemRect(nItem, &rcItem);
        GXHDC        hdc = NULL;
        //int        nSplit = (rcItem.left + rcItem.right) >> 1;
        ITEM&      item = m_pCurPage->GetList()[nItem];
        ITEM::HITTEST  eHit = item.HitTest(&rcItem, m_pCurPage->GetSplit(), &m_ptLButton);

        if( item.eType == PST_BOOLEAN || 
          item.eType == PST_BUTTON ||
          item.eType == PST_FLOAT ||
          item.eType == PST_UINTEGER ||
          item.eType == PST_INTEGER ||
          item.eType == PST_PROPSHEETPAGE)
        {
          hdc = gxGetDC(m_hWnd);
          gxSelectObject(hdc, m_hFont);
        }

        if(eHit == ITEM::HT_BOOL_CHECKBOX)
        {
          item.DrawAsCheckBox(hdc, &rcItem, (item.bVal ? DFCS_BUTTONCHECK | DFCS_CHECKED : DFCS_BUTTONCHECK) | DFCS_INACTIVE);
          m_eLBDown = eHit;
        }
        else if(eHit == ITEM::HT_SPINUP || eHit == ITEM::HT_SPINDOWN)
        {
          GXRECT rect = rcItem;
          rect.left = rect.right - (rect.bottom - rect.top);
          gxInflateRect(&rect, -2, -2);
          int nBtnState;
          UINT idTimer;
          if(eHit == ITEM::HT_SPINUP)
          {
            nBtnState = 1;
            idTimer = TIMER_ID_SPINUP;
          }
          else
          {
            nBtnState = 2;
            idTimer = TIMER_ID_SPINDOWN;
          }
          m_eLBDown = eHit;
          DrawUpDownControl(hdc, &rect, nBtnState);
          gxSetTimer(m_hWnd, idTimer, 100, NULL);
          m_bChangedNumber = FALSE;
          SubmitString(FALSE); // 会清除 m_nEditing
          m_SpinBtnDown.nVal = item.nVal;
          m_nEditing = nItem;
        }

        switch(item.eType)
        {
        case PST_PROPSHEETPAGE:
          {
            GXRECT rect;
            gxSetRect(&rect, rcItem.right - rcItem.bottom + rcItem.top, rcItem.top, rcItem.right, rcItem.bottom);
            gxInflateRect(&rect, -2, -2);
            //if(PtInRect(&rect, m_ptLButton))
            if(eHit == ITEM::HT_PAGE_CHILD)
            {
              gxDrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLRIGHT | DFCS_PUSHED);
              m_eLBDown = eHit;
            }
          }
          break;
        case PST_BUTTON:
          {
            item.DrawAsButton(hdc, &rcItem, TRUE);
            m_eLBDown = eHit;
          }
          break;
        }
        if(hdc != NULL)
          gxReleaseDC(m_hWnd, hdc);
      }
    }

    void Form::LButtonUp(GXWPARAM wParam, GXLPARAM lParam)
    {
      //GXRECT rcClinet;
      GXPOINT pt;
      pt.x = (short)LOWORD(lParam);
      pt.y = (short)HIWORD(lParam);
      GXRECT rcItem;

      if(m_eLBDown == ITEM::HT_SPINUP || m_eLBDown == ITEM::HT_SPINDOWN)
      {
        if(m_nEditing >= 0 && m_nEditing < (int)m_pCurPage->GetList().size())
        {
          GXHDC hdc = gxGetDC(m_hWnd);
          ITEM& item = m_pCurPage->GetList()[m_nEditing];
          m_pCurPage->GetItemRect(m_nEditing, &rcItem);
          //rcItem.left = rcItem.right - (rcItem.bottom - rcItem.top);
          //InflateRect(&rcItem, -2, -2);
          //DrawUpDownControl(hdc, &rcItem, 0);
          gxKillTimer(m_hWnd, m_eLBDown == ITEM::HT_SPINUP ? TIMER_ID_SPINUP : TIMER_ID_SPINDOWN);
          if(m_bChangedNumber == FALSE)
          {
            if(m_eLBDown == ITEM::HT_SPINUP)
            {
              if(item.eType == PST_FLOAT)
                item.fVal += item.fIncrease;
              else
                item.nVal++;
            }
            else if(m_eLBDown == ITEM::HT_SPINDOWN)
            {
              if(item.eType == PST_FLOAT)
                item.fVal -= item.fIncrease;
              else
                item.nVal--;
            }
          }
          //int nSplit = (rcItem.left + rcItem.right) >> 1;
          rcItem.left = m_pCurPage->GetSplit();
          gxSelectObject(hdc, m_hFont);
          gxSetBkColor(hdc, m_nEditing & 1 ? COLOR_A_LIGHT : COLOR_B_LIGHT);
          gxFillRect(hdc, &rcItem, m_nEditing & 1 ? m_hBrush[2] : m_hBrush[3]);
          item.DrawAsNumber(hdc, &rcItem, 0);
          gxReleaseDC(m_hWnd, hdc);
          NotifyParent(GXNM_CLICK, item);
          SubmitString(FALSE);
        }
      }
      else if(m_eLBDown == ITEM::HT_SPINTRACK)
      {
        goto RESET_PARAM;
      }
      else if( abs(pt.y - m_ptLButton.y) < GetSystemMetrics(SM_CYDOUBLECLK) &&
        (m_fwKeys & MK_LBUTTON) )
      {
        GXHDC hdc = NULL;
        //GetClientRect(m_hWnd, &rcClinet);
        int nItem = m_pCurPage->HitTest(pt.x, pt.y);
        if(nItem >= 0 && nItem < (int)m_pCurPage->GetList().size())
        {
          m_pCurPage->GetItemRect(nItem, &rcItem);
          ITEM& item = m_pCurPage->GetList()[nItem];
          ITEM::HITTEST  eHit = item.HitTest(&rcItem, m_pCurPage->GetSplit(), &pt);
          //int        nSplit = (rcItem.left + rcItem.right) >> 1;

          if( item.eType == PST_BOOLEAN || 
            item.eType == PST_FLOAT ||
            item.eType == PST_UINTEGER ||
            item.eType == PST_INTEGER ||
            item.eType == PST_BUTTON ||
            item.eType == PST_PROPSHEETPAGE)
          {
            hdc = gxGetDC(m_hWnd);
            gxSelectObject(hdc, m_hFont);
          }

          if(eHit == ITEM::HT_BOOL_CHECKBOX)
          {
            item.bVal = !item.bVal;
            item.DrawAsCheckBox(hdc, &rcItem, item.bVal ? DFCS_BUTTONCHECK | DFCS_CHECKED : DFCS_BUTTONCHECK);
            NotifyParent(GXNM_CLICK, item);
          }

          if(m_hEdit)
          {
            SubmitString(FALSE);
            //NotifyParent(item);
          }
          if(m_hListBox)
          {
            GXBOOL bRet = (m_nEditing == nItem);  // 两次选择了同一个项目则关闭ListBox
            SubmitList(FALSE);
            if(bRet)
              goto RESET_PARAM;
          }

          switch(item.eType)
          {
          case PST_FLOAT:
          case PST_INTEGER:
          case PST_UINTEGER:
            if(eHit == ITEM::HT_EDIT)
            {
              PopupEditLabel(nItem, &rcItem, &pt);
            }
            else if(eHit == ITEM::HT_SPINDOWN || eHit == ITEM::HT_SPINUP)
            {
              GXRECT rect;
              rect = rcItem;
              rect.left = rect.right - (rect.bottom - rect.top);
              gxInflateRect(&rect, -2, -2);
              DrawUpDownControl(hdc, &rect, 0);
              gxKillTimer(m_hWnd, m_eLBDown == ITEM::HT_SPINUP ? TIMER_ID_SPINUP : TIMER_ID_SPINDOWN);
            }
            break;
          case PST_PROPSHEETPAGE:
            {
              GXRECT rect;
              gxSetRect(&rect, rcItem.right - rcItem.bottom + rcItem.top, rcItem.top, rcItem.right, rcItem.bottom);
              gxInflateRect(&rect, -2, -2);
              if(gxPtInRect(&rect, pt))
              {
                gxDrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLRIGHT);
                SwitchPage(item.pChildPage);
              }

            }
            break;

          case PST_STRING:
            {
              PopupEditLabel(nItem, &rcItem, &pt);
            }
            break;
          case PST_LIST:
            {

              LBItemList& LBItem = *item.pListBoxItem;
              m_nEditing = nItem;

              m_hListBox = gxCreateWindowEx(NULL, _CLTEXT("LISTBOX"), NULL, 
                WS_OVERLAPPED | WS_CHILD | WS_VISIBLE | GXWS_VSCROLL | CBS_DROPDOWNLIST | GXWS_BORDER | /*LBS_SORT | */LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
                m_pCurPage->GetSplit(), rcItem.bottom, rcItem.right - m_pCurPage->GetSplit(), (int)(item.nHeight * (LBItem.size() + 1)),
                m_hWnd, NULL, (GXHINSTANCE)(GXLONG_PTR)gxGetWindowLong(m_hWnd, GXGWL_HINSTANCE), NULL);
              gxSendMessage(m_hListBox, GXWM_SETFONT, (GXWPARAM)m_hFont, NULL);

              int n = 0;
              for(LBItemList::iterator it = LBItem.begin(); it != LBItem.end(); ++it, n++)
              {
                int nIndex = (int)gxSendMessageA(m_hListBox, LB_ADDSTRING, 0, (GXLPARAM)(const GXWCHAR*)it->strName);
                gxSendMessage(m_hListBox, LB_SETITEMDATA, nIndex, it->lParam);
                if(n == item.nVal)
                {
                  gxSendMessage(m_hListBox, LB_SETCURSEL, nIndex, NULL);
                }
              }
            }
            break;
          case PST_COLOR:
            {
              // 不支持颜色选择
              //CHOOSECOLOR sChooseColor;
              //memset(&sChooseColor, 0, sizeof(sChooseColor));
              //sChooseColor.lStructSize  = sizeof(sChooseColor);
              //sChooseColor.hwndOwner    = m_hWnd;
              //sChooseColor.rgbResult    = item.crVal; 
              //sChooseColor.Flags      = CC_RGBINIT;
              //sChooseColor.lpCustColors  = g_crCustom;
              //g_crCustom[0] = item.crVal;

              //if(ChooseColor(&sChooseColor) != 0)
              //{
              //  item.crVal = sChooseColor.rgbResult;
              //  NotifyParent(item);
              //  gxInvalidateRect(m_hWnd, NULL, FALSE);
              //}
            }
            break;
          case PST_BUTTON:
            {
              item.DrawAsButton(hdc, &rcItem, FALSE);
              SwitchPage(m_pCurPage->GetParent());
            }
            break;
          case PST_IMAGEPATHA:
          case PST_IMAGEPATHW:
            {
              GXBOOL bChangeTex = FALSE;
#ifdef _WINDOWS
              OPENFILENAMEW ofn;
              GXWCHAR szFilename[MAX_PATH];
              InlSetZeroT(ofn);
              InlSetZeroT(szFilename);

              ofn.lStructSize = sizeof(ofn);
              ofn.hInstance = GetModuleHandle(NULL);
              ofn.lpstrFilter = _T("所有支持的图像格式\0*.jpg;*.png;*.dds;*.tga;*.bmp;*.tif\0all files(*.*)\0*.*\0");
              ofn.lpstrFile = reinterpret_cast<LPWSTR>(szFilename);
              ofn.nMaxFile = MAX_PATH;
              ofn.lpstrTitle = _T("Open Image File");
              ofn.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;
              if(GetOpenFileNameW(&ofn))
              {
                if(item.strVal != szFilename)
                {
                  item.strVal = szFilename; // FIXME: 转换为相对路径?
                  bChangeTex = TRUE;
                }

              }
#endif // #ifdef _WINDOWS
              if(bChangeTex) {
                SAFE_RELEASE(item.pImage);
                LPGXGRAPHICS pGraphics = GXGetGraphics(m_hWnd);
                item.pImage = pGraphics->CreateImageFromFile(item.strVal);
                NotifyParent(GXNM_RETURN, item);
                Invalidate(FALSE);
              }
            }
            break;
          }
          if(hdc != NULL) {
            gxReleaseDC(m_hWnd, hdc);
          }
        }
      }
RESET_PARAM:
      gxReleaseCapture();
      m_fwKeys &= (~MK_LBUTTON);
      m_eLBDown = ITEM::HT_BLANK;
      if(m_bShowScorollBar == TRUE)
      {
        GXRECT rcClient;
        gxGetClientRect(m_hWnd, &rcClient);
        rcClient.left = rcClient.right - 10;
        gxInvalidateRect(m_hWnd, &rcClient, FALSE);
        m_bShowScorollBar = FALSE;
      }
    }

    GXBOOL CALLBACK HideChildProc(GXHWND hwnd, GXLPARAM lParam)
    {
      if(gxGetParent(hwnd) == (GXHWND)lParam)
        gxShowWindow(hwnd, SW_HIDE);
      return TRUE;
    }


    GXBOOL Form::SwitchPage(Page* pPage)
    {
      GXRECT rcClient;
      if(pPage == NULL)
        return FALSE;
      gxGetClientRect(m_hWnd, &rcClient);
      if(pPage->GetParent() == m_pCurPage)
      {
        gxSetTimer(m_hWnd, TIMER_ID_PAGESLIDELEFT, 10, NULL);
        m_xLeft = rcClient.right;
      }
      else
      {
        gxSetTimer(m_hWnd, TIMER_ID_PAGESLIDERIGHT, 10, NULL);
        m_xLeft = -rcClient.right;
      }

      gxEnumChildWindows(m_hWnd, HideChildProc, (LPARAM)m_hWnd);

      m_pCurPage = pPage;
      return TRUE;
    }

    void Form::DrawScrollBar(GXHDC hdc, const GXLPRECT lpClient, GXLPRECT prcOut /* = NULL */)
    {
      GXRECT rect;
      rect.left = lpClient->right - 10;
      rect.right = lpClient->right;
      rect.top = -((m_pCurPage->GetTopIndex() * lpClient->bottom) / m_pCurPage->GetTotalItemHeight());
      rect.bottom = ((lpClient->bottom * lpClient->bottom) / m_pCurPage->GetTotalItemHeight()) + rect.top + 1;
      if(prcOut != NULL)
        *prcOut = rect;
      gxFillRect(hdc, &rect, (GXHBRUSH)gxGetStockObject(BLACK_BRUSH));
    }

    void Form::NotifyParent(GXUINT code, ITEM& item)
    {
      GXHWND hParent = gxGetParent(m_hWnd);
      GXDWORD idCtrl = (GXDWORD)gxGetWindowLong(m_hWnd, GWL_ID);
      NM_PROPSHEETW NMPropSheet;
      NMPropSheet.nmhdr.code      = code;
      NMPropSheet.nmhdr.hwndFrom  = m_hWnd;
      NMPropSheet.nmhdr.idFrom    = idCtrl;
      //NMPropSheet.pItem           = &item;

      NMPropSheet.eType   = item.eType;
      NMPropSheet.dwStyle = item.dwStyle;
      NMPropSheet.dwId    = item.dwId;
      NMPropSheet.nHeight = item.nHeight;
      NMPropSheet.strName = item.strName;
      NMPropSheet.nVal    = item.nVal;
      NMPropSheet.strVal  = item.strVal;

      if(item.eType == PST_FLOAT) {
        NMPropSheet.fIncrease = item.fIncrease;
      } else {
        NMPropSheet.fIncrease = 0;
      }

      gxSendMessage(hParent, GXWM_NOTIFY, idCtrl, (LPARAM)&NMPropSheet);
    }

    GXBOOL Form::DestroyChildDlg(GXHWND hDlg)
    {
      if(hDlg != m_hEdit || hDlg != m_hListBox)
        return FALSE;

      INT nItem = m_pCurPage->FindByDlgHandle(hDlg);
      if(nItem != -1)
      {
        ItemList& aList = m_pCurPage->GetList();
        aList.erase(aList.begin() + nItem);
        m_pCurPage->UpdateHeightList();
        CheckScrolled();
        gxInvalidateRect(m_hWnd, NULL, TRUE);
        return TRUE;
      }
      ASSERT(0);
      return FALSE;
    }

    GXBOOL Form::CheckScrolled()
    {
      GXRECT rect;
      const INT nTopIndex = m_pCurPage->GetTopIndex();
      const INT nTotalHeight = m_pCurPage->GetTotalItemHeight();
      gxGetClientRect(m_hWnd, &rect);
      if(nTopIndex + nTotalHeight < rect.bottom)
      {
        m_pCurPage->SetTopIndex(rect.bottom - nTotalHeight);
        return TRUE;
      }
      return FALSE;
    }

    void Form::OnTimer(GXDWORD dwTiemrID)
    {
      switch(dwTiemrID)
      {
      case TIMER_ID_SPINUP:
      case TIMER_ID_SPINDOWN:
        {
          GXRECT rcClient;
          if(m_nEditing >= 0 && m_nEditing < (int)m_pCurPage->GetList().size())
          {
            ITEM& item = m_pCurPage->GetList()[m_nEditing];
            m_pCurPage->GetItemRect(m_nEditing, &rcClient);
            if(item.eType == PST_INTEGER)
            {
              if(dwTiemrID == TIMER_ID_SPINUP)
                item.nVal++;
              else
                item.nVal--;
            }
            else if(item.eType == PST_UINTEGER)
            {
              if(dwTiemrID == TIMER_ID_SPINUP)
                item.nVal++;
              else
                item.nVal--;
            }
            else if(item.eType == PST_FLOAT)
            {
              if(dwTiemrID == TIMER_ID_SPINUP)
                item.fVal += item.fIncrease;
              else
                item.fVal -= item.fIncrease;
            }
            else ASSERT(0);

            NotifyParent(GXNM_CLICK, item);

            GXHDC hdc = gxGetDC(m_hWnd);
            //int nSplit = (rcClient.left + rcClient.right) >> 1;

            gxSelectObject(hdc, m_hFont);
            gxSetBkColor(hdc, m_nEditing & 1 ? COLOR_A_LIGHT : COLOR_B_LIGHT);
            rcClient.left = m_pCurPage->GetSplit();
            gxFillRect(hdc, &rcClient, m_nEditing & 1 ? m_hBrush[2] : m_hBrush[3]);
            item.DrawAsNumber(hdc, &rcClient, dwTiemrID == TIMER_ID_SPINUP ? 1 : 2);
            gxReleaseDC(m_hWnd, hdc);
            m_bChangedNumber = TRUE;
          }
        }
        break;
      case TIMER_ID_PAGESLIDELEFT:
      case TIMER_ID_PAGESLIDERIGHT:
        m_xLeft = (int)((float)m_xLeft * 0.65f);
        if(m_xLeft == 0)
          gxKillTimer(m_hWnd, dwTiemrID);
        gxInvalidateRect(m_hWnd, NULL, FALSE);
        break;
      }
    }

    void Form::MouseMove(GXWPARAM wParam, GXLPARAM lParam)
    {
      GXRECT rcClient;
      GXRECT rcItem;
      GXPOINT ptPos;
      ptPos.x = (short)LOWORD(lParam);
      ptPos.y = (short)HIWORD(lParam);

      //if(xPos >= m_nSplit - 2 && xPos <= m_nSplit + 2)
      //  SetCursor(LoadCursor(NULL, IDC_SIZEWE));

      gxGetClientRect(m_hWnd, &rcClient);

      if(((m_fwKeys & wParam) & MK_LBUTTON) == MK_LBUTTON)
      {
        if( abs(ptPos.y - m_ptLButton.y) < GetSystemMetrics(SM_CYDOUBLECLK) && 
          abs(ptPos.x - m_ptLButton.x) < GetSystemMetrics(SM_CXDOUBLECLK) )
          return;
        if(m_eLBDown == ITEM::HT_SPINUP || m_eLBDown == ITEM::HT_SPINDOWN)
        {
          ITEM& item = m_pCurPage->GetList()[m_nEditing];
          m_pCurPage->GetItemRect(m_nEditing, &rcItem);
          ITEM::HITTEST  eHit = item.HitTest(&rcItem, m_pCurPage->GetSplit(), &ptPos);

          if(eHit == ITEM::HT_SPINTRACK)
          {
            gxKillTimer(m_hWnd, m_eLBDown == ITEM::HT_SPINUP 
              ? TIMER_ID_SPINUP 
              : TIMER_ID_SPINDOWN);
            m_eLBDown = eHit;
          }        
          return;
        }
        else if( m_eLBDown == ITEM::HT_SPINTRACK)
        {
          ITEM& item = m_pCurPage->GetList()[m_nEditing];
          m_pCurPage->GetItemRect(m_nEditing, &rcItem);
          INT nOffset = ((rcItem.top + rcItem.bottom) >> 1) - ptPos.y + 1;
          union  // 保存当前显示的值
          {
            INT    nVal;
            float  fVal;
          };

          if(item.eType == PST_INTEGER)
          {
            nVal = item.nVal;
            item.nVal = m_SpinBtnDown.nVal + nOffset;
          }
          else if(item.eType == PST_UINTEGER)
          {
            nVal = item.nVal;
            item.nVal = m_SpinBtnDown.nVal + nOffset;
          }
          else if(item.eType == PST_FLOAT)
          {
            fVal = item.fVal;
            item.fVal = m_SpinBtnDown.fVal + (float)nOffset * item.fIncrease;
          }
          else ASSERT(0);

          NotifyParent(GXNM_HOVER, item);

          if(nVal != item.nVal)
          {
            GXHDC hdc = gxGetDC(m_hWnd);
            gxSelectObject(hdc, m_hFont);
            gxSetBkColor(hdc, m_nEditing & 1 ? COLOR_A_LIGHT : COLOR_B_LIGHT);
            rcItem.left = m_pCurPage->GetSplit();
            gxFillRect(hdc, &rcItem, m_nEditing & 1 ? m_hBrush[2] : m_hBrush[3]);
            item.DrawAsNumber(hdc, &rcItem, 0);
            gxReleaseDC(m_hWnd, hdc);
            m_bChangedNumber = TRUE;
          }
          return;
        }
        else if( m_eLBDown == ITEM::HT_BUTTON || 
          m_eLBDown == ITEM::HT_PAGE_CHILD || 
          m_eLBDown == ITEM::HT_BOOL_CHECKBOX )
        {
          gxInvalidateRect(m_hWnd, NULL, FALSE);
          m_eLBDown = ITEM::HT_BLANK;
        }

        INT nTopIndex = m_nPrevTopIndex + (ptPos.y - m_ptLButton.y);
        if(nTopIndex > 0)
          nTopIndex = 0;
        else if((rcClient.bottom - nTopIndex) > m_pCurPage->GetTotalItemHeight())
        {
          if(m_pCurPage->GetTotalItemHeight() < rcClient.bottom)
            return;
          nTopIndex = rcClient.bottom - m_pCurPage->GetTotalItemHeight();
        }

        if(nTopIndex == m_pCurPage->GetTopIndex())
          return;

        nTopIndex = m_pCurPage->SetTopIndex(nTopIndex) - nTopIndex;
        if(m_hEdit != NULL)
          SubmitString(FALSE);
        if(m_hListBox != NULL)
          SubmitList(FALSE);

        m_bShowScorollBar = TRUE;

        // 滚动窗口内容
        // 不能使用 ScrollWindowEx 带CHILD的方式,这中方式只能滚动子窗口,而不影响子窗口的子窗口
        gxScrollWindow(m_hWnd, 0, -nTopIndex, &rcClient, NULL);

        // 下面的代码全都是为了解决滚动条闪烁的问题,靠了的!
        rcClient.left = rcClient.right - 10;
        gxInvalidateRect(m_hWnd, &rcClient, TRUE);
        GXHDC hdc = gxGetDC(m_hWnd);
        DrawScrollBar(hdc, &rcClient, &rcClient);
        gxReleaseDC(m_hWnd, hdc);
        gxValidateRect(m_hWnd, &rcClient);
      }
    }

    GXBOOL Page::UpdateHeightList()
    {
      GXHWND hWnd = m_pPropSheet->Get();
      GXHDC hdc = gxGetDC(hWnd);
      GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, m_pPropSheet->m_hFont);
      GXBOOL bRet = UpdateHeightList(hdc);
      gxSelectObject(hdc, hOldFont);
      gxReleaseDC(hWnd, hdc);
      return bRet;
    }

    GXBOOL Page::UpdateHeightList(GXHDC hdc)
    {
      GXRECT rcClient;
      GXRECT rect;
      m_nTotalItemHeight = 0;

      gxGetClientRect(m_pPropSheet->Get(), &rcClient);
      gxSetRectEmpty(&rect);

      for(ItemList::iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it)
      {
        ITEM& item = *it;

        // 测试字符串高度
        rect.left  = 0;
        rect.top = 0;
        if(item.dwStyle & ITEM::F_VISIBLE)
        {
          if(item.eType == PST_DESCRIBE)
          {
            rect.right = rcClient.right;
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
          }
          else if(item.eType == PST_DIALOG)
          {
            goto SUM_HEIGHT;        
          }
          else if(item.eType == PST_IMAGEPATHA || item.eType == PST_IMAGEPATHW)
          {
            gxSetRect(&rect, 0, 0, rcClient.right, (rcClient.right >> 2) * 3);
          }
#ifdef _DEBUG
          // 这么写是为了检查新增类型的
          else if(item.eType == PST_UNKNOWN ||
            item.eType == PST_BOOLEAN       ||
            item.eType == PST_INTEGER       ||
            item.eType == PST_UINTEGER      ||
            item.eType == PST_FLOAT         ||
            item.eType == PST_STRING        ||
            item.eType == PST_DESCRIBE      ||
            item.eType == PST_LIST          ||
            item.eType == PST_PROPSHEETPAGE ||
            item.eType == PST_COLOR         ||
            item.eType == PST_IMAGEPATHW    ||
            item.eType == PST_IMAGEPATHA    ||
            item.eType == PST_BUTTON        ||
            item.eType == PST_DIALOG) {
              gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, DT_SINGLELINE | DT_CALCRECT);
          }
          else {
            CLBREAK; // 新增类型如果没有处理会在此出现
          }

#else
          else {
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, DT_SINGLELINE | DT_CALCRECT);
          }
#endif // #ifdef _DEBUG

          if(item.eType == PST_PROPSHEETPAGE)
          {
            item.pChildPage->UpdateHeightList(hdc);
          }
        }
        else
        {
          rect.top    = 0;
          rect.bottom = 0;
        }
        item.nHeight = rect.bottom - rect.top + THICKNESS * 2;
SUM_HEIGHT:
        m_nTotalItemHeight += item.nHeight;
      }  
      return TRUE;
    }

    void Page::DrawItems(GXHDC hdc) const
    {
      GXRECT  rcClient;
      GXRECT  rcItem;
      GXRECT  rect;
      GXHBRUSH  hBrushD;
      GXHBRUSH  hBrushL;
      LPGXWNDCANVAS pCanvas = NULL;

      gxGetClientRect(m_pPropSheet->Get(), &rcClient);
      int yTop = m_nTopIndex;
      const int xLeft = m_pPropSheet->m_xLeft;
      int n = -1;
      const int nSplit = m_nSplit;

      gxSelectObject(hdc, m_pPropSheet->m_hFont);

      memset(&rect, 0, sizeof(rect));

      for(ItemList::const_iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it)
      {
        const ITEM& item = *it;
        if((item.dwStyle & ITEM::F_VISIBLE) == FALSE)
          continue;

        n++;
        if((n & 1) != 0)
        {
          hBrushD = m_pPropSheet->m_hBrush[0];
          hBrushL = m_pPropSheet->m_hBrush[2];
          gxSetBkColor(hdc, COLOR_A_DARK);
        }
        else
        {
          hBrushD = m_pPropSheet->m_hBrush[1];
          hBrushL = m_pPropSheet->m_hBrush[3];
          gxSetBkColor(hdc, COLOR_B_DARK);
        }

        // 测试字符串高度
        rect.left   = m_pPropSheet->m_xLeft;
        rect.top    = yTop;
        rect.right  = rcClient.right + xLeft;
        rect.bottom = yTop + item.nHeight;

        rcItem.left   = rect.left;
        rcItem.top    = rect.top;
        rcItem.bottom = rect.bottom;

        if(item.eType == PST_DESCRIBE)
        {
          rcItem.right  = rect.right;
          pCanvas = GXGetWndCanvas(hdc);
          if(pCanvas)
          {
            pCanvas->FillRect(&rcItem, COLOR_TITLE | 0xff000000);
          }
          //gxFillRect(hdc, &rcItem, hBrushD);
          rect.top += THICKNESS;
          gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, DT_LEFT | DT_WORDBREAK);
          yTop += item.nHeight;
          continue;
        }
        else if(item.eType == PST_IMAGEPATHA || item.eType == PST_IMAGEPATHW)
        {
          if(item.pImage)
          {
            pCanvas = GXGetWndCanvas(hdc);
            if(pCanvas)
            {
              GXREGN rgDest(rcItem.left, rcItem.top, rect.right, item.nHeight);
              pCanvas->DrawImage(item.pImage, &rgDest, NULL);
            }
          }
        }
        else if(item.eType == PST_DIALOG)
        {
          ;  // 啥也不做
        }
        else
        {
          // 填充背景颜色
          rcItem.right  = nSplit + xLeft;
          gxFillRect(hdc, &rcItem, hBrushD);

          rcItem.left    = rcItem.right;
          rcItem.right  = rect.right;
          gxFillRect(hdc, &rcItem, hBrushL);
        }

        // 绘制标题
        if(item.eType != PST_BUTTON)
        {
          rect.right  = nSplit + xLeft;
          if(item.dwStyle & ITEM::F_DISABLE)
          {
            gxSetBkMode(hdc, TRANSPARENT);

            gxSetTextColor(hdc, 0xc0c0c0);
            //OffsetRect(&rect, -1, -1);
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, DT_SINGLELINE|DT_VCENTER);

            //SetTextColor(hdc, 0xe0e0e0);
            //OffsetRect(&rect, 1, 1);
            //DrawTextA(hdc, item.strName.c_str(), (int)item.strName.length(), &rect, DT_SINGLELINE);

            //SetTextColor(hdc, 0xe0e0e0);
            //DrawTextA(hdc, item.strName.c_str(), (int)item.strName.length(), &rect, DT_SINGLELINE);

            //OffsetRect(&rect, -1, -1);
            //SetTextColor(hdc, 0xc0c0c0);
            //DrawTextA(hdc, item.strName.c_str(), (int)item.strName.length(), &rect, DT_SINGLELINE);

            //OffsetRect(&rect, 1, 1);


            gxSetTextColor(hdc, 0x000000);
            gxSetBkMode(hdc, OPAQUE);
          }
          else if(item.eType == PST_IMAGEPATHA || item.eType == PST_IMAGEPATHW)
          {
            gxOffsetRect(&rect, 1, 1);
            gxSetTextColor(hdc, 0xffffff);
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, GXDT_SINGLELINE);

            gxOffsetRect(&rect, -1, -1);
            gxSetTextColor(hdc, 0x000000);
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, GXDT_SINGLELINE);
          }
          else {
            gxDrawTextW(hdc, item.strName, (int)item.strName.GetLength(), &rect, GXDT_SINGLELINE | GXDT_VCENTER);
          }
        }

        if((n & 1) != 0) {
          gxSetBkColor(hdc, COLOR_A_LIGHT);
        }
        else {
          gxSetBkColor(hdc, COLOR_B_LIGHT);
        }

        // 绘制Item的值
        rect.left  = nSplit + xLeft;
        rect.right  = rcClient.right + xLeft;
        if(item.eType == PST_STRING)
        {
          gxSetRectEmpty(&rcItem);
          int nTextLength = (int)item.strVal.GetLength();
          UINT uFlags = DT_SINGLELINE|DT_VCENTER;
          gxDrawTextW(hdc, item.strVal, nTextLength, &rcItem, DT_SINGLELINE | DT_CALCRECT);
          if(rcItem.right > rect.right - rect.left)
            uFlags |= DT_LEFT;
          else
            uFlags |= DT_RIGHT;
          gxDrawTextW(hdc, item.strVal, nTextLength, &rect, uFlags);
        }
        else if(item.eType == PST_INTEGER || item.eType == PST_UINTEGER || item.eType == PST_FLOAT)
        {
          item.DrawAsNumber(hdc, &rect, item.dwStyle & ITEM::F_DISABLE);
        }
        else if(item.eType == PST_BOOLEAN)
        {
          item.DrawAsCheckBox(hdc, &rect, item.bVal ? DFCS_BUTTONCHECK | DFCS_CHECKED : DFCS_BUTTONCHECK);
        }
        else if(item.eType == PST_LIST)
        {
          if(item.pListBoxItem != NULL)
          {
            LBItemList& LBItem = *item.pListBoxItem;
            gxDrawTextW(hdc, LBItem[item.nVal].strName, (int)LBItem[item.nVal].strName.GetLength(), &rect, DT_SINGLELINE | DT_RIGHT|DT_VCENTER);
          }
        }
        else if(item.eType == PST_DIALOG)
        {
          UINT dwFlags = SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW;
          if(gxIsWindowVisible(item.hDlg) == FALSE)
            dwFlags |= SWP_SHOWWINDOW;

          gxSetWindowPos(item.hDlg, NULL, xLeft, rect.top, 0, 0, dwFlags);
          GXPOINT ptOrigin;
          gxSetViewportOrgEx(hdc, xLeft, rect.top, &ptOrigin);
          ASSERT(0);
          //gxPrintWindow(item.hDlg, hdc, NULL);
          gxSetViewportOrgEx(hdc, ptOrigin.x, ptOrigin.y, NULL);
        }
        else if(item.eType == PST_PROPSHEETPAGE)
        {
          if(item.pGeneral != NULL)
          {
            rect.left = rect.right - item.nHeight;
            gxInflateRect(&rect, -2, -2);
            gxDrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLRIGHT);
            gxInflateRect(&rect, 2, 2);
          }
        }
        else if(item.eType == PST_COLOR)
        {
          GXHBRUSH hSolidBrush = gxCreateSolidBrush(item.crVal & 0xffffff);
          gxInflateRect(&rect, -4, -4);
          gxFillRect(hdc, &rect, hSolidBrush);
          gxInflateRect(&rect, 4, 4);
          gxDeleteObject(hSolidBrush);
        }
        else if(item.eType == PST_BUTTON)
        {
          gxSetRect(&rcItem, xLeft, rect.top, rcClient.right + xLeft, rect.bottom);
          item.DrawAsButton(hdc, &rcItem, FALSE);
        }

        yTop += item.nHeight;
        if(yTop > rcClient.bottom)
          goto DRAW_SCROLLBAR;
      }
      if(yTop < rcClient.bottom)
      {
        // 绘制背景
        //gxSetRect(&rect, xLeft, yTop, rcClient.right, rcClient.bottom);
        //gxFillRect(hdc, &rect, (GXHBRUSH)gxGetStockObject(WHITE_BRUSH));
      }
DRAW_SCROLLBAR:
      if(m_pPropSheet->m_bShowScorollBar == TRUE)
        m_pPropSheet->DrawScrollBar(hdc, &rcClient);
    }

    int Page::HitTest(int xPos, int yPos)
    {
      int nTopPos = yPos - m_nTopIndex;
      int n = 0;

      for(ItemList::const_iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it, ++n)
      {
        const ITEM& item = *it;
        if(nTopPos < (INT)item.nHeight)
        {
          if(item.dwStyle & ITEM::F_DISABLE)
            return -1;
          return n;
        }
        nTopPos -= item.nHeight;
      }
      return -1;
    }

    GXINT Page::SetTopIndex(INT nTopIndex)
    {
      INT nPrevTopIndex = m_nTopIndex;
      if(nTopIndex > 0)
        m_nTopIndex = 0;
      else
        m_nTopIndex = nTopIndex;
      return nPrevTopIndex;
    }

    GXINT Page::FindByDlgHandle(GXHWND hDlg) const
    {
      INT n = 0;
      for(ItemList::const_iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it, ++n)
      {
        if((*it).eType == PST_DIALOG && (*it).hDlg == hDlg)
          return n;
      }
      return -1;
    }

    GXINT  Page::FindById(INT nStart, INT nId) const
    {
      INT n = 0;
      if(nStart >= (INT)m_aItemList.size())
        return -1;
      for(ItemList::const_iterator it = m_aItemList.begin() + nStart; 
        it != m_aItemList.end(); ++it, ++n)
      {
        if((*it).dwId == nId)
          return n;
      }
      return -1;
    }

    ITEM* Page::FindItemById(INT nId)
    {
      for(ItemList::iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it)
      {
        if((*it).dwId == nId)
          return &*it;
        else if((*it).eType == PST_PROPSHEETPAGE)
        {
          ITEM* pRet = (*it).pChildPage->FindItemById(nId);
          if(pRet != NULL)
            return pRet;
        }
      }
      return NULL;
    }

    Page* Page::FindPageById(INT nId)
    {
      for(ItemList::iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it)
      {
        if((*it).eType == PST_PROPSHEETPAGE)
        {
          if((*it).dwId == nId)
            return (*it).pChildPage;
          else
            return (*it).pChildPage->FindPageById(nId);
        }
      }
      return NULL;
    }

    GXINT Page::SetSplit(int nSplit)
    {
      INT nPrevSplit = m_nSplit;
      m_nSplit = nSplit;
      return nPrevSplit;
    }

    void Page::GetItemRect(int nItem, GXRECT* rect) const
    {
      INT nTotalHeight = 0;
      INT nItemHeight = 0;
      int n = 0;
      for(auto it = m_aItemList.begin(); it != m_aItemList.end(); ++it, ++n)
      {
        const ITEM& item = *it;
        if(n >= nItem)
        {
          nItemHeight = item.nHeight;
          break;
        }
        nTotalHeight += item.nHeight;
      }
      GXRECT rcClient;
      gxGetClientRect(m_pPropSheet->Get(), &rcClient);
      rect->left = 0;
      rect->top = nTotalHeight + m_nTopIndex;
      rect->right = rcClient.right;
      rect->bottom = nTotalHeight + nItemHeight + m_nTopIndex;
    }

    GXBOOL Page::Initialize(const LPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink)
    {
      ITEM item;
      if(m_pParent != NULL)
      {
        item.strName = "<<返回";
        item.dwId   = IDCLOSE;
        item.eType   = PST_BUTTON;
        m_aItemList.push_back(item);
      }

      for(int i = 0;; i++)
      {
        if(pDataLink[i].eType == PST_UNKNOWN) {
          break;
        }
        item.strName  = pDataLink[i].lpName;
        item.dwId     = pDataLink[i].dwId;
        item.eType    = (ITEM::TYPE)pDataLink[i].eType;
        item.pGeneral = NULL;

        BYTE* pBasePtr = pDataLink[i].nBasePtrIdx >= 0 
          ? (BYTE*)pBasePtrList[pDataLink[i].nBasePtrIdx] + pDataLink[i].pDestOffsetPtr
          : NULL;

        switch(pDataLink[i].eType)
        {
        case PST_BOOLEAN:
          item.bVal = *(GXBOOL*)pBasePtr;
          break;
        case PST_INTEGER:
          item.nVal = *(int*)pBasePtr;
          break;
        case PST_UINTEGER:
          item.nVal = *(int*)pBasePtr;
          break;
        case PST_FLOAT:
          item.fVal = *(float*)pBasePtr;
          item.fIncrease = pDataLink[i].fIncrease;
          break;
        case PST_STRING:
          item.strVal = *(clStringW*)pBasePtr;
          break;
        case PST_DESCRIBE:
          break;
        case PST_LIST:
          {
            item.pListBoxItem = new LBItemList;
            Form::Build(*item.pListBoxItem, (const PROPSHEET_LISTBOX*)pDataLink[i].pValue);
            item.nVal = pBasePtr == NULL ? NULL : *(int*)pBasePtr;
          }
          break;
        case PST_PROPSHEETPAGE:
          {
            item.pChildPage = new Page(m_pPropSheet, m_nSplit, this);
            item.pChildPage->Initialize(pBasePtrList, (const PROPLIST_DATALINK*)pDataLink[i].pValue);
          }
          break;
        case PST_COLOR:
          {
            item.crVal = *(COLORREF*)pBasePtr;
          }
          break;
        case PST_DIALOG:
          {
            GXRECT rect;
            GXHWND hWnd = m_pPropSheet->Get();
            GXHINSTANCE hInst = (GXHINSTANCE)(GXLONG_PTR)gxGetWindowLong(hWnd, GXGWL_HINSTANCE);
            GXHWND hDlg = gxCreateDialogParamW(hInst, pDataLink[i].lpTemplate, hWnd, pDataLink[i].lpDlgProc, pDataLink[i].dwInitParam);
            //ShowWindow(hDlg, SW_SHOWNORMAL);
            if(hDlg == NULL)
              continue;
            item.hDlg = hDlg;
            gxGetWindowRect(hDlg, &rect);
            item.nHeight = rect.bottom - rect.top;
            gxGetClientRect(hWnd, &rect);
            gxSetWindowPos(hDlg, NULL, 0, 0, rect.right, item.nHeight, GXSWP_NOMOVE | GXSWP_NOACTIVATE);
          }
          break;
        case PST_IMAGEPATHA:
          {
            item.strVal = *(clStringA*)pBasePtr;
          }
        case PST_IMAGEPATHW:
          {
            if(pDataLink[i].eType != PST_IMAGEPATHA) {
              item.strVal = *(clStringW*)pBasePtr;
            }

            if(item.strVal.IsNotEmpty())
            {
              GXHWND hWnd = m_pPropSheet->Get();
              LPGXGRAPHICS pGraphics = GXGetGraphics(hWnd);
              ASSERT(item.pImage == NULL);
              item.pImage = pGraphics->CreateImageFromFile(item.strVal);
            }
          }
          break;
        default:
          ASSERT(0);
        }
        m_aItemList.push_back(item);
      }
      UpdateHeightList();
      return TRUE;
    }

    GXVOID Page::Release()
    {
      for(ItemList::iterator it = m_aItemList.begin(); 
        it != m_aItemList.end(); ++it)
      {
        ITEM& item = *it;
        if(item.eType == PST_PROPSHEETPAGE)
        {
          item.pChildPage->Release();
          SAFE_DELETE(item.pChildPage);
        }
        else if(item.eType == PST_IMAGEPATHA || item.eType == PST_IMAGEPATHW)
        {
          SAFE_RELEASE(item.pImage);
        }

        else if(item.eType == PST_LIST)
        {
          SAFE_DELETE(item.pListBoxItem);
        }
      }
      m_aItemList.clear();
    }

    GXBOOL Page::UploadData(const LPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, INT nIDFirst, INT nIDLast)
    {
      int i = 0;
      GXBOOL bRet = TRUE;
      ItemList::iterator it = m_aItemList.begin();
      if(m_pParent != NULL)
        ++it;

      for(;it != m_aItemList.end(); ++it, ++i)
      {
        if(pDataLink[i].eType == PST_UNKNOWN)
          break;
        ITEM& item = *it;

        if(item.dwId != pDataLink[i].dwId || 
          item.eType != (ITEM::TYPE)pDataLink[i].eType)
        {
          bRet = FALSE;
          continue;
        }
        else if(((INT)item.dwId < nIDFirst || (INT)item.dwId > nIDLast) && item.eType != PST_PROPSHEETPAGE)
          continue;

        BYTE* pBasePtr = (BYTE*)pBasePtrList[pDataLink[i].nBasePtrIdx] + pDataLink[i].pDestOffsetPtr;

        switch(pDataLink[i].eType)
        {
        case PST_BOOLEAN:
          item.bVal = *(GXBOOL*)pBasePtr;
          break;
        case PST_INTEGER:
          item.nVal = *(int*)pBasePtr;
          break;
        case PST_UINTEGER:
          item.nVal = *(int*)pBasePtr;
          break;
        case PST_FLOAT:
          item.fVal = *(float*)pBasePtr;
          break;
        case PST_STRING:
        case PST_IMAGEPATHW:
          item.strVal = *(clStringW*)pBasePtr;
          break;
        case PST_IMAGEPATHA:
          item.strVal = *(clStringA*)pBasePtr;
          break;
        case PST_DESCRIBE:
        case PST_DIALOG:
          break;
        case PST_LIST:
          item.nVal = *(int*)pBasePtr;
          break;
        case PST_PROPSHEETPAGE:
          {
            item.pChildPage->UploadData(pBasePtrList, (const PROPLIST_DATALINK*)pDataLink[i].pValue, nIDFirst, nIDLast);
          }
          break;
        case PST_COLOR:
          item.crVal = *(COLORREF*)pBasePtr;
          break;
        default:
          CLBREAK;
        }
      }
      UpdateHeightList();
      return bRet;
    }

    GXBOOL Page::DownloadData(const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast)
    {
      int i = 0;
      ITEM item;
      GXBOOL bRet = TRUE;
      ItemList::iterator it = m_aItemList.begin();
      if(m_pParent != NULL)
        ++it;

      for(;it != m_aItemList.end(); ++it, ++i)
      {
        if(pDataLink[i].eType == PST_UNKNOWN)
          break;
        ITEM& item = *it;

        if(item.dwId != pDataLink[i].dwId || 
          item.eType != (ITEM::TYPE)pDataLink[i].eType)
        {
          bRet = FALSE;
          continue;
        }
        else if((GXINT)item.dwId < nIDFirst || (GXINT)item.dwId > nIDLast)
          continue;

        GXBYTE* pBasePtr = (GXBYTE*)pBasePtrList[pDataLink[i].nBasePtrIdx] + pDataLink[i].pDestOffsetPtr;

        switch(pDataLink[i].eType)
        {
        case PST_BOOLEAN:
          *(GXBOOL*)pBasePtr = item.bVal;
          break;
        case PST_INTEGER:
          *(int*)pBasePtr = item.nVal;
          break;
        case PST_UINTEGER:
          *(int*)pBasePtr = item.nVal;
          break;
        case PST_FLOAT:
          *(float*)pBasePtr = item.fVal;
          break;
        case PST_STRING:
        case PST_IMAGEPATHW:
          *(clStringW*)pBasePtr = item.strVal;
          break;
        case PST_IMAGEPATHA:
          *(clStringA*)pBasePtr = item.strVal;
          break;
        case PST_DESCRIBE:
        case PST_DIALOG:
          break;
        case PST_LIST:
          *(int*)pBasePtr = item.nVal;
          break;
        case PST_PROPSHEETPAGE:
          {
            item.pChildPage->DownloadData(pBasePtrList, (const PROPLIST_DATALINK*)pDataLink[i].pValue, nIDFirst, nIDLast);
          }
          break;
        case PST_COLOR:
          *(GXCOLORREF*)pBasePtr = item.crVal;
          break;
        default:
          CLBREAK;
        }
      }
      return TRUE;
    }


    GXBOOL Form::Build(LBItemList& aLBItem, const PROPSHEET_LISTBOX* pListBox)
    {
      aLBItem.clear();
      LISTBOXITEM lbi;

      for(int i = 0;; i++)
      {
        if(pListBox[i].lpName == NULL)
          break;
        lbi.strName = pListBox[i].lpName;
        lbi.lParam = pListBox[i].lParam;
        aLBItem.push_back(lbi);
      }
      return TRUE;
    }

    GXBOOL Form::Initialize(const LPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink)
    {
      m_aBastPtr.clear();
      int nBasePtr = GetMaxPtrIndex(pDataLink);
      m_aBastPtr.reserve(nBasePtr);
      for(int i = 0; i < nBasePtr; i++)
      {
        m_aBastPtr.push_back(pBasePtrList[i]);
      }

      m_Root.Release();

      GXBOOL bRet = m_Root.Initialize(pBasePtrList, pDataLink);
      gxInvalidateRect(m_hWnd, NULL, TRUE);
      return bRet;
    }

    GXBOOL Form::UploadData(const LPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, INT nIDFirst, INT nIDLast)
    {
      GXBOOL bRet = m_Root.UploadData(pBasePtrList, pDataLink, nIDFirst, nIDLast);
      gxInvalidateRect(m_hWnd, NULL, TRUE);
      return bRet;
    }

    GXBOOL Form::DownloadData(const LPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, INT nIDFirst, INT nIDLast)
    {
      return m_Root.DownloadData(pBasePtrList, pDataLink, nIDFirst, nIDLast);
    }


    GXVOID Form::Release()
    {
      gxDeleteObject(m_hBrush[0]);
      gxDeleteObject(m_hBrush[1]);
      gxDeleteObject(m_hBrush[2]);
      gxDeleteObject(m_hBrush[3]);
      gxDeleteObject(m_hPen);

      m_Root.Release();
      m_pCurPage          = NULL;
      m_hEdit             = NULL;
      m_nEditing          = NULL;
      m_dwOldEditWndProc  = NULL;
      m_eLBDown           = ITEM::HT_BLANK;
      m_hListBox          = NULL;
      m_bChangedNumber    = FALSE;
      m_bShowScorollBar   = FALSE;
      m_xLeft             = 0;
    }

    GXBOOL Form::UpdateData(const PROPLIST_DATALINK* pDataLink)
    {
      CLBREAK; // 没实现
      return TRUE;
    }

    Page::Page(GXINT nSplit, Page* pParent /* = NULL */)
      : m_pPropSheet(NULL)
      , m_pParent(pParent)
      , m_nTopIndex(0)
      , m_nSplit(nSplit)
      , m_nTotalItemHeight(0)
    {
    }

    Page::Page(Form* pPropSheet, GXINT nSplit, Page* pParent /* = NULL */)
      : m_pPropSheet(pPropSheet)
      , m_pParent(pParent)
      , m_nTopIndex(0)
      , m_nSplit(nSplit)
      , m_nTotalItemHeight(0)
    {
    }

    GXINT Page::GetTotalItemHeight() const
    {
      return m_nTotalItemHeight;
    }

    Page* Page::GetParent() const
    {
      return m_pParent;
    }

    GXINT Page::GetTopIndex() const
    {
      return m_nTopIndex;
    }

    GXINT Page::GetSplit() const
    {
      return m_nSplit;
    }

    GXVOID ITEM::DrawAsButton(GXHDC hdc, GXRECT* pRect, GXBOOL bPrushed) const
    {
      GXRECT rect = *pRect;
      int nTextLen = (int)strName.GetLength();

      gxInflateRect(&rect, -1, -1);
      gxDrawFrameControl(hdc, &rect, DFC_BUTTON, bPrushed ? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH);
      gxSetBkMode(hdc, TRANSPARENT);
      if(bPrushed != FALSE)
        gxOffsetRect(&rect, 1, 1);
      gxDrawTextW(hdc, strName, nTextLen, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
      gxSetBkMode(hdc, OPAQUE);
    }
    void ITEM::DrawAsCheckBox(GXHDC hdc, GXRECT* pRect, UINT uState) const
    {
      GXRECT rect;
      rect.top  = pRect->top + 2;
      rect.left  = pRect->right - nHeight + 2;
      rect.right  = pRect->right - 2;
      rect.bottom  = pRect->bottom - 2;

      gxDrawFrameControl(hdc, &rect, DFC_BUTTON, uState);
    }

    GXVOID ITEM::DrawAsNumber(GXHDC hdc, GXRECT* pRect, int nBtnState) const
    {
      //char buffer[128];
      clStringW str;
      GXRECT rect = *pRect;
      if(eType == PST_INTEGER)
      {
        str.Format(_CLTEXT("%d"), nVal);
      }
      else if(eType == PST_UINTEGER)
      {
        str.Format(_CLTEXT("%u"), bVal);
      }
      else 
      {
        str.AppendFloat(fVal, 'R');
        //ReadlizeFloatString(str);
      }

      rect.right -= nHeight;
      int nLen = (int)str.GetLength();
      ASSERT(nLen < 128);

      if(nBtnState & F_DISABLE)
      {
        gxSetTextColor(hdc, 0xc0c0c0);
        gxDrawTextW(hdc, str, nLen, &rect, DT_SINGLELINE | DT_RIGHT);
        gxSetTextColor(hdc, 0);
      }
      else
        gxDrawText(hdc, str, nLen, &rect, DT_SINGLELINE | DT_RIGHT);

      rect.left = rect.right;
      rect.right += nHeight;
      gxInflateRect(&rect, -2, -2);
      DrawUpDownControl(hdc, &rect, nBtnState);
    }

    ITEM::ITEM()
      : eType(PST_UNKNOWN)
      , dwId(0)
      , dwStyle(F_VISIBLE)
      , pGeneral(NULL)
      , nHeight(0)
    {
      ASSERT(6 * 4 + sizeof(clStringW) * 2 == sizeof(ITEM));
      nVal = 0;
    }
    
    //ITEM::ITEM(const ITEM& I)
    //{
    //  *this = I;
    //  if((eType == PST_IMAGEPATHA || eType == PST_IMAGEPATHW) && pImage)
    //  {
    //    pImage->AddRef();
    //  }
    //}

    ITEM::HITTEST ITEM::HitTest(GXRECT* rcItem, INT nSplit, GXPOINT* ptHit)
    {
      GXRECT rect;

      GXBOOL bName = (ptHit->x >= 0 && ptHit->x < nSplit && 
        ptHit->y >= rcItem->top && ptHit->y < rcItem->bottom);

      switch(eType)
      {
      case PST_BOOLEAN:
        gxSetRect(&rect, rcItem->right - rcItem->bottom + rcItem->top, rcItem->top, 
          rcItem->right, rcItem->bottom);
        gxInflateRect(&rect, -2, -2);
        if(gxPtInRect(&rect, *ptHit))
        {
          return HT_BOOL_CHECKBOX;
        }
        if(ptHit->x >= nSplit && ptHit->x < rcItem->right && 
          ptHit->y >= rcItem->top && ptHit->y < rcItem->bottom)
          return HT_BLANK;

        return HT_NAME;

      case PST_FLOAT:
      case PST_INTEGER:
      case PST_UINTEGER:
        if(bName)
          return HT_NAME;
        if(ptHit->x >= nSplit && ptHit->x < rcItem->right - rcItem->bottom + rcItem->top && 
          ptHit->y >= rcItem->top && ptHit->y < rcItem->bottom)
          return HT_EDIT;
        if(ptHit->y < rcItem->top || ptHit->y >= rcItem->bottom)
          return HT_SPINTRACK;
        else if(ptHit->y < ((rcItem->top + rcItem->bottom) >> 1))
          return HT_SPINUP;
        return HT_SPINDOWN;

      case PST_STRING:
        if(bName)
          return HT_NAME;
        if(ptHit->x >= nSplit && ptHit->x < rcItem->right && 
          ptHit->y >= rcItem->top && ptHit->y < rcItem->bottom)
          return HT_EDIT;

      case PST_DESCRIBE:
        return HT_NAME;

      case PST_LIST:
        if(bName)
          return HT_NAME;
        return HT_LIST_SELECT;

      case PST_PROPSHEETPAGE:
        if(bName)
          return HT_NAME;
        gxSetRect(&rect, rcItem->right - rcItem->bottom + rcItem->top, rcItem->top, rcItem->right, rcItem->bottom);
        gxInflateRect(&rect, -2, -2);

        if(gxPtInRect(&rect, *ptHit))
          return HT_PAGE_CHILD;
        return HT_BLANK;

      case PST_COLOR:
        if(bName)
          return HT_NAME;
        gxSetRect(&rect, nSplit, rcItem->top, rcItem->right, rcItem->bottom);
        gxInflateRect(&rect, -4, -4);
        if(gxPtInRect(&rect, *ptHit))
          return HT_COLOR;

      case PST_BUTTON:
        return HT_BUTTON;
      
      case PST_IMAGEPATHW:
      case PST_IMAGEPATHA:
        return HT_BUTTON;

      default:
        CLBREAK;
      }
      return HT_ERROR;
    }

    GXBOOL ITEM::EnableItem(GXBOOL bEnable)
    {
      GXBOOL bRet = !(dwStyle & F_DISABLE);
      if(bEnable == TRUE)
        dwStyle &= (~F_DISABLE);
      else
        dwStyle |= F_DISABLE;
      return bRet;
    }

    GXBOOL ITEM::ShowItem(GXBOOL bShow)
    {
      GXBOOL bRet = !(dwStyle & F_VISIBLE);
      if(bShow == TRUE)
        dwStyle |= F_VISIBLE;
      else
        dwStyle &= (~F_VISIBLE);
      return bRet;
    }

    Form* Form::Create( GXHINSTANCE hInst, GXHWND hParent, GXLPCWSTR szTemplate, 
      const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions )
    {
      Form* pPropSheet = NULL;

      GXWNDCLASSEX wcex;
      if(gxGetClassInfoEx(hInst, GXUICLASSNAME_PROPLIST, &wcex) == FALSE) {
        GXUI::CtrlBase::RegisterClass(hInst, Form::WndProc, GXUICLASSNAME_PROPLIST);
      }
      GXDWORD dwStyle = pDlgParam->dwStyle | GXWS_VISIBLE | GXWS_CHILD;
      GXHWND hWnd = gxCreateWindowEx(pDlgParam->dwExStyle, GXUICLASSNAME_PROPLIST, pDlgParam->strCaption, 
        dwStyle, pDlgParam->regn.left, pDlgParam->regn.top, 
        pDlgParam->regn.width, pDlgParam->regn.height, hParent, (GXHMENU)(GXLPCWSTR)pDlgParam->strName, hInst, NULL);

      pPropSheet = (Form*)gxGetWindowLongPtrW(hWnd, 0);

      if(pPropSheet->m_hWnd == NULL) {
        pPropSheet->Destroy();
        delete pPropSheet;
        pPropSheet = NULL;
      }
      return pPropSheet;
    }

    GXLRESULT Form::OnPaint( GXWndCanvas& canvas )
    {
      return 0;
    }

    GXLRESULT Form::Measure( GXRegn* pRegn )
    {
      return 0;
    }

    GXLRESULT Form::Destroy()
    {
      Release();
      return GXUI::CtrlBase::Destroy();
    }

    GXINT Form::GetMaxPtrIndex( const PROPLIST_DATALINK* pDataLayout )
    {
      int nMax = 0;
      for(int i = 0; pDataLayout[i].lpName != NULL; i++)
      {
        if(pDataLayout[i].eType == PST_PROPSHEETPAGE) {
          nMax = clMax(nMax, GetMaxPtrIndex(
            (const PROPLIST_DATALINK*)pDataLayout[i].pValue));
        }
        else {
          nMax = clMax(nMax, pDataLayout->nBasePtrIdx);
        }
      }
      return nMax;
    }

    //************************************
    // Method:    PopupEditLabel
    // Qualifier:
    // Parameter: int nItem       item索引
    // Parameter: GXLPCRECT prc   item的区域，如果为NULL则使用默认区域
    // Parameter: GXLPCPOINT pt   鼠标点击位置，激活Edit控件后，用来定位光标，使它出现在鼠标点击所在的字母附近
    //************************************
    GXBOOL Form::PopupEditLabel(int nItem, GXLPCRECT prc, GXLPCPOINT pt)
    {
      clStringW str;
      GXLONG lWndProc = 0;
      GXRECT rcItem;
      ITEM& item = m_pCurPage->GetList()[nItem];
      if(prc) {
        rcItem = *prc;
      }
      else {
        m_pCurPage->GetItemRect(nItem, &rcItem);
      }

      if(item.eType == PST_FLOAT)
      {
        str.AppendFloat(item.fVal, 'R');
        //ReadlizeFloatString(str);
        lWndProc = (GXLONG)(GXLONG _w64)EditFloatWndProc;
      }
      else if(item.eType == PST_INTEGER)
      {
        str.Format(_CLTEXT("%d"), item.nVal);
        lWndProc = (GXLONG)(GXLONG _w64)EditIntegerWndProc;
      }
      else if(item.eType == PST_UINTEGER)
      {
        str.Format(_CLTEXT("%u"), item.nVal);
        lWndProc = (GXLONG)(GXLONG _w64)EditIntegerWndProc;
      }
      else if(item.eType == PST_STRING)
      {
        //str = item.strVal;
        lWndProc = (GXLONG)(GXLONG _w64)EditStringWndProc;
        //m_nEditing = nItem;
        //ASSERT(m_hEdit == NULL);
        //gxSetWindowLong(m_hEdit, GWL_USERDATA, (GXLONG_PTR)this);
        //gxSendMessage(m_hEdit, WM_SETFONT, (GXWPARAM)m_hFont, NULL);
        //m_dwOldEditWndProc = gxGetWindowLong(m_hEdit, GWL_WNDPROC);
        //gxSetWindowLong(m_hEdit, GWL_WNDPROC, (GXLONG_PTR)EditStringWndProc);
        //gxSetFocus(m_hEdit);

        //return TRUE;
      }
      else { CLBREAK; }

      ASSERT(m_hEdit == NULL);
      m_nEditing = nItem;

      // 字符串和数字输入控件不同点：
      // 1.Window text 不同
      // 2.宽度不同，数字输入控件考虑了UpDown按钮的宽度。
      if(item.eType == PST_STRING)
      {
        m_hEdit = gxCreateWindowEx(NULL, GXUICLASSNAME_EDIT, item.strVal, GXWS_CHILD | GXWS_VISIBLE | GXWS_BORDER | GXES_AUTOHSCROLL | GXES_RIGHT,
          m_pCurPage->GetSplit(), rcItem.top, rcItem.right - m_pCurPage->GetSplit(), rcItem.bottom - rcItem.top,
          m_hWnd, NULL, (GXHINSTANCE)(GXLONG _w64)gxGetWindowLong(m_hWnd, GXGWL_HINSTANCE), NULL);
      }
      else 
      {
        m_hEdit = gxCreateWindowEx(NULL, GXUICLASSNAME_EDIT, str, GXWS_CHILD | GXWS_VISIBLE | GXWS_BORDER | GXES_AUTOHSCROLL | GXES_RIGHT,
          m_pCurPage->GetSplit(), rcItem.top, rcItem.right - m_pCurPage->GetSplit() - (rcItem.bottom - rcItem.top), rcItem.bottom - rcItem.top,
          m_hWnd, NULL, (GXHINSTANCE)(GXLONG_PTR)gxGetWindowLong(m_hWnd, GXGWL_HINSTANCE), NULL);
      }

      gxSetWindowLong(m_hEdit, GXGWL_USERDATA, (LONG)(LONG _w64)this);
      gxSendMessage(m_hEdit, GXWM_SETFONT, (WPARAM)m_hFont, NULL);
      m_dwOldEditWndProc = gxGetWindowLong(m_hEdit, GXGWL_WNDPROC);
      gxSetWindowLong(m_hEdit, GXGWL_WNDPROC, lWndProc);
      gxSetFocus(m_hEdit);

      if(pt) {
        gxSendMessage(m_hEdit, GXWM_LBUTTONDOWN, GXMK_LBUTTON, GXMAKELPARAM(pt->x - rcItem.left - m_pCurPage->GetSplit(), pt->y - rcItem.top));
      }

      return TRUE;
    }

    GXBOOL Form::PopupNextEditLabel(int nDir)
    {
      ASSERT(nDir == -1 || nDir == 1);
      ItemList& aItems = m_pCurPage->GetList();

      // 找到下一个需要编辑控件的Item
      int nNext = m_nEditing + nDir;
      while(nNext >= 0 && nNext < (int)aItems.size() &&
        aItems[nNext].eType != PST_FLOAT &&
        aItems[nNext].eType != PST_INTEGER &&
        aItems[nNext].eType != PST_UINTEGER &&
        aItems[nNext].eType != PST_STRING)
      {
        nNext += nDir;
      }

      // 找不到则不做任何处理
      if(nNext < 0 || nNext >= (int)aItems.size()) {
        return FALSE;
      }

      SubmitString(FALSE);
      return PopupEditLabel(nNext, NULL, NULL);
    }

  } // namespace PropertyList
}  // namespace GXUIEXT
