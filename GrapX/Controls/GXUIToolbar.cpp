#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.h"
#include "GrapX/GResource.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXSprite.h"
#include "GrapX/GXGraphics.h"

#include "GrapX/guxtheme.h"

#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GXUICtrlBase.h"
#include "GXUIToolbar.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/gxDevice.h"
STATIC_ASSERT(sizeof(GXColor32) == 4);
namespace GXUI
{
  Toolbar::Toolbar(GXLPCWSTR szIdName)
    : CtrlBase      (szIdName)
    , m_pSprite     (NULL)
    , m_nCurItem    (-1)
    //, m_nPressdItem (-1)
    , m_crHover     (128, 244, 209, 159)
    , m_crPressd    (239, 199, 189)
    , m_crChecked   (255, 166, 155)
    , m_crText      (0xff000000)
    , m_nMaxBtnWidth(100)
    , m_bIndexedCommand(0)
    , m_hToolTip(NULL)
  {
    m_ButtonSize.cx = 24;
    m_ButtonSize.cy = 22;
    m_BitmapSize.cx = 16;
    m_BitmapSize.cy = 15;
  }

  Toolbar* Toolbar::Create(
    GXDWORD       dwExStyle, 
    GXLPCWSTR     lpWindowName, 
    GXDWORD       dwStyle, 
    const GXRegn* pRegn, 
    GXHWND        hWndParent, 
    GXHMENU       hMenu, 
    GXHINSTANCE   hInstance, 
    GXLPCWSTR     szIdName,
    GXLPVOID      lpParam)
  {
    //GXWNDCLASSEX wcex;
    //if(gxGetClassInfoEx(hInstance, GXUICLASSNAME_TOOLBAR, &wcex) == FALSE) {
    //  RegisterClass(hInstance, WndProc, GXUICLASSNAME_TOOLBAR);
    //}
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_TOOLBAR);

    GXHWND hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_TOOLBAR, lpWindowName, dwStyle, pRegn->left, pRegn->top, 
      pRegn->width, pRegn->height, hWndParent, (GXHMENU)szIdName, hInstance, NULL);


    Toolbar* pToolbar = (Toolbar*)gxGetWindowLongPtrW(hWnd, 0);

    if(pToolbar->m_hWnd == NULL) {
      pToolbar->Destroy();
      delete pToolbar;
      pToolbar = NULL;
    }
    return pToolbar;
  }

  Toolbar* Toolbar::Create(GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions)
  {
    GXDWORD dwStyle = GXWS_VISIBLE | GXWS_CHILD | pDlgParam->dwStyle;
    Toolbar* pToolbar = Create(pDlgParam->dwExStyle, pDlgParam->strCaption, dwStyle, &pDlgParam->regn, hParent, NULL, hInst, pDlgParam->strName, NULL);
    if(pToolbar && pDefinitions) {
      pToolbar->SolveDefinition(*pDefinitions);
    }
    return pToolbar;
  }

  GXLRESULT Toolbar::Destroy()
  {
    if(m_hToolTip) {
      gxDestroyWindow(m_hToolTip);
      m_hToolTip = NULL;
    }

    std::for_each(m_aButtons.begin(), m_aButtons.end(), [](TOOLBARBUTTON& tb)
    {
      if( ! IS_IDENTIFY(tb.idCommand)) {
        clStringW& str = *(clStringW*)&tb.idCommand;
        str.~clStringW();
      }
    });

    m_aButtons.clear();
    SAFE_RELEASE(m_pSprite);
    return CtrlBase::Destroy();
  }

  GXLRESULT Toolbar::Measure(GXRegn* pRegn)
  {
    if(m_aButtons.size() == 0) {
      return -1;
    }
    TOOLBARBUTTON& tbb = m_aButtons.back();
    pRegn->x = 0;
    pRegn->y = 0;
    pRegn->w = tbb.wLeft + tbb.wWidth;
    pRegn->h = m_ButtonSize.cy;
    return 0;
  }

  GXBOOL Toolbar::SetSpriteFile(GXLPCWSTR szSpriteFile)
  {
    SAFE_RELEASE(m_pSprite);
    GXGraphics* pGraphics = GXGetGraphics(m_hWnd);
    GXBOOL bval = GXSUCCEEDED(GXCreateSpriteFromFileW(pGraphics, szSpriteFile, &m_pSprite));
    if(bval) {
      Invalidate(TRUE);
    }
    return bval;
  }

  GXBOOL Toolbar::SetSpriteObj(GXSprite* pSprite)
  {
    SAFE_RELEASE(m_pSprite);
    m_pSprite = pSprite;
    m_pSprite->AddRef();
    Invalidate(TRUE);
    return TRUE;
  }

  GXVOID Toolbar::CalcButtonSize(int nStart)
  {
    GXWndCanvas* pCanvas = NULL;
    int nLeft = nStart == 0 ? 0 : m_aButtons[nStart - 1].wLeft + m_aButtons[nStart - 1].wWidth;

    for(ToolBtnArray::iterator it = m_aButtons.begin() + nStart;
      it != m_aButtons.end(); ++it) {
        
        TOOLBARBUTTON& tbb = *it;

        if(TEST_FLAG(tbb.fsState, TBSTATE_HIDDEN)) {
          tbb.wWidth = 0;
          tbb.wLeft = nLeft;
          continue;
        }

        if(TEST_FLAG(tbb.fsStyle, BTNS_SHOWTEXT) && 
          tbb.strText.GetLength() > 0 && pCanvas == NULL) {
            pCanvas = new GXWndCanvas(m_hWnd, NULL);
        }

        tbb.wWidth = GetBtnWidth(pCanvas, &tbb);
        tbb.wLeft = nLeft;
        nLeft += tbb.wWidth;
        
        if(m_hToolTip && tbb.strText.IsNotEmpty())
        {
          GXTOOLINFOW ti = {sizeof(GXTOOLINFOW)};
          ti.uFlags = TTF_SUBCLASS;
          ti.hwnd = m_hWnd;
          ti.uId = it - m_aButtons.begin() + 1;
          GetItemRect(it, &ti.rect);
          gxSendMessage(m_hToolTip, GXTTM_NEWTOOLRECTW, 0, (GXLPARAM)&ti);
        }
    }
    SAFE_DELETE(pCanvas);
  }

  GXBOOL Toolbar::AddButton(const GXTBBUTTON* pTBButton)
  {
    TOOLBARBUTTON tbb;
    tbb.nSprite   = pTBButton->iBitmap;
    if(IS_IDENTIFY(pTBButton->idCommand)) {
      tbb.idCommand = (GXLPCWSTR)pTBButton->idCommand;
    }
    else {
      clStringW* pStr = new(&tbb.idCommand) clStringW((GXLPCWSTR)pTBButton->idCommand);
      //*pStr = (GXLPCWSTR)pTBButton->idCommand;
      m_bIndexedCommand = 1;
    }
    tbb.fsState   = pTBButton->fsState;
    tbb.fsStyle   = pTBButton->fsStyle;
    tbb.dwData    = pTBButton->dwData;
    tbb.strText   = pTBButton->iString == 0 ? _CLTEXT("") : (GXLPCWSTR)pTBButton->iString;
    tbb.wLeft     = 0;
    tbb.wWidth    = 0;

    m_aButtons.push_back(tbb);

    if(m_hToolTip && tbb.strText.IsNotEmpty())
    {
      GXTOOLINFOW ti = {sizeof(GXTOOLINFOW)};
      ti.uFlags = TTF_SUBCLASS;
      ti.hwnd = m_hWnd;
      ti.lpszText = tbb.strText.GetBuffer();
      ti.uId = m_aButtons.size();
      gxSendMessage(m_hToolTip, GXTTM_ADDTOOLW, 0, (GXLPARAM)&ti);
    }
    return TRUE;
  }

  void Toolbar::GetItemRect(ToolBtnArray::iterator it, GXRECT* rect)
  {
    rect->left   = it->wLeft;
    rect->top    = 0;
    rect->right  = rect->left + it->wWidth;
    rect->bottom = rect->top + m_ButtonSize.cy;
  }

  void Toolbar::GetItemRect(int nBtn, GXRECT* rect)
  {
    GetItemRect(m_aButtons.begin() + nBtn, rect);
  }

  int Toolbar::HitTestItem(int x, int y)
  {
    int nCount = (int)m_aButtons.size();
    GXRECT rect;
    GXPOINT pt = {x, y};
    for(int i = 0; i < nCount; i++)
    {
      GetItemRect(i, &rect);
      if(gxPtInRect(&rect, pt)) {
        return i;
      }        
    }
    return -1;
  }

  int Toolbar::OnMouseMove(int fwKeys, int x, int y)
  {
    //if(TEST_FLAG(fwKeys, GXMK_LBUTTON) && gxGetCapture() != m_hWnd)
    //  return 0;

    int nItem = HitTestItem(x, y);
    if(nItem != m_nCurItem)
    {
      GXWndCanvas canvas(m_hWnd, NULL);
      DrawItem(canvas, nItem, m_crHover, FALSE);
      DrawItem(canvas, m_nCurItem, 0, TRUE);
      m_nCurItem = nItem;
    }

    return 0;
  }

  int Toolbar::IntOnMouseMove(int nFocusItem, int x, int y)
  {
    //if(TEST_FLAG(fwKeys, GXMK_LBUTTON) && gxGetCapture() != m_hWnd)
    //  return 0;

    int nItem = HitTestItem(x, y);
    if(nItem != m_nCurItem)
    {
      GXWndCanvas canvas(m_hWnd, NULL);
      if(nItem == nFocusItem) {
        DrawItem(canvas, nItem, m_crPressd, FALSE);
        TRACE("nItem\n");
      }
      else {
        DrawItem(canvas, nFocusItem, 0, TRUE);
        TRACE("nFocusItem\n");
      }
      m_nCurItem = nItem;
    }

    return 0;
  }

  int Toolbar::OnLButtonDown(int fwKeys, int x, int y)
  {
    int nPressdItem = HitTestItem(x, y);
    //m_nPressdItem = nItem;
    m_nCurItem = nPressdItem;

    {
      GXWndCanvas canvas(m_hWnd, NULL);
      DrawItem(canvas, nPressdItem, m_crPressd, FALSE);
    }

    gxSetCapture(m_hWnd);

    GXMSG msg;
    while(gxGetMessage(&msg, NULL))
    {
      if(msg.message == GXWM_LBUTTONUP) {
        int nItem = HitTestItem(GXGET_X_LPARAM(msg.lParam), GXGET_Y_LPARAM(msg.lParam));
        gxReleaseCapture();
        if(nItem != -1 && nPressdItem == nItem) {
          IntOnLButtonUp(nItem);
        }
        break;
      }
      else if(msg.message == GXWM_MOUSEMOVE) {
        IntOnMouseMove(nPressdItem, GXGET_X_LPARAM(msg.lParam), GXGET_Y_LPARAM(msg.lParam));
      }
      else {
        gxTranslateMessage(&msg);
        gxDispatchMessageW(&msg);
      }
    }

    return 0;
  }

  int Toolbar::IntOnLButtonUp(int nFocusItem)
  {
    ASSERT(nFocusItem != -1);

    // TODO: 发送按下消息
    TOOLBARBUTTON& tbb = m_aButtons[nFocusItem];
    CheckButtonByIndex(nFocusItem, ! TEST_FLAG(tbb.fsState, TBSTATE_CHECKED));

    gxSendMessage(m_hNotifyWnd, GXWM_COMMAND, GXMAKEWPARAM(m_bIndexedCommand 
      ? nFocusItem + 1: (int)tbb.idCommand, GXBN_CLICKED), (GXLPARAM)m_hWnd);

    m_nCurItem = -1;
    //ASSERT(m_nCurItem == m_nPressdItem);
    //m_nPressdItem = -1;

    return 0;
  }

  GXLRESULT GXCALLBACK Toolbar::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    ToolbarObjRef pThis = (Toolbar*)CtrlBase::PreClassObj(hWnd, message, lParam);

    switch(message)
    {
    case GXUITB_SETSPRITEFILE:
      return pThis->SetSpriteFile((GXLPCWSTR)lParam);
    case GXUITB_SETSPRITEOBJ:
      return pThis->SetSpriteObj((GXSprite*)lParam);
    case GXTB_ADDBUTTONS:
      {
        //GXWndCanvas* pCanvas = NULL;
        const GXTBBUTTON* pButtons = (GXTBBUTTON*)lParam;
        int nStart = (int)pThis->m_aButtons.size();
        for(GXUINT i = 0; i < (GXUINT)wParam; i++, pButtons++)
        {
          pThis->AddButton(pButtons);
        }
        pThis->CalcButtonSize(nStart);
        //SAFE_DELETE(pCanvas);
      }
      break;

    case GXTB_HIDEBUTTON:
      return pThis->HideButton((GXINT_PTR)wParam, (GXBOOL)lParam);

    case GXTB_AUTOSIZE:
      pThis->AutoSize();
      break;

    case GXTB_SETBUTTONSIZE:
      pThis->SetButtonSize(GXLOWORD(lParam), GXHIWORD(lParam));
      break;

    case GXTB_SETBITMAPSIZE:
      pThis->SetBitmapSize(GXLOWORD(lParam), GXHIWORD(lParam));
      break;

    case GXTB_GETBUTTONINFOW:
      return pThis->GetButtonInfo((int)wParam, (GXTBBUTTONINFOW*)lParam);

    case GXTB_SETBUTTONINFOW:
      return pThis->SetButtonInfo((int)wParam, (GXTBBUTTONINFOW*)lParam);

    case GXWM_MOUSEMOVE:
      {
        int x = GXLOWORD(lParam);
        int y = GXHIWORD(lParam);
        pThis->OnMouseMove((int)wParam, x, y);
      }
      return 0;
    case GXWM_MOUSELEAVE:
      pThis->ClearItem();
      break;
    case GXWM_LBUTTONDOWN:
      {
        int x = GXLOWORD(lParam);
        int y = GXHIWORD(lParam);
        pThis->OnLButtonDown((int)wParam, x, y);
      }
      return 0;

    //case GXWM_LBUTTONUP:
    //  {
    //    int x = GXLOWORD(lParam);
    //    int y = GXHIWORD(lParam);
    //    pThis->OnLButtonUp((int)wParam, x, y);
    //  }
    //  return 0;

    case GXWM_GETIDNAMEW:
      {
        if(wParam == 0) {
          return (GXLRESULT)(GXLPCWSTR)pThis->m_strIdentityName;
        }
        else{
          return (GXLRESULT)pThis->GetIdNameW((GXINT)wParam);
        }
      }
      break;

    case GXTB_ISBUTTONCHECKED:
      return pThis->IsButtonChecked((int)wParam);

    case GXTB_CHECKBUTTON:
      return pThis->CheckButton((int)wParam, GXLOWORD(lParam));

    case GXWM_NCCREATE:
      {
        GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;
        Toolbar* pToolbar = NULL;
        GXLPCWSTR szIdName = IS_IDENTIFY(lpcs->hMenu) ? NULL : (GXLPCWSTR)lpcs->hMenu;
        pToolbar = new Toolbar(szIdName);

        TRACE("Toolbar:%p hWnd:%p\n", pToolbar, hWnd);


        if(TEST_FLAG(lpcs->style, TBSTYLE_TOOLTIPS))
        {
          pToolbar->InitToolTips(hWnd);
        }

        return pToolbar->OnNCCreate(hWnd, message, wParam, lParam);
      }
    case GXWM_NCDESTROY:
      //if(pThis->m_hToolTip)
      //{
      //  gxDestroyWindow(pThis->m_hToolTip);
      //  pThis->m_hToolTip = NULL;
      //}
      break;
      //return gxDefWindowProc(hWnd, message, wParam, lParam);
    }
    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  GXLRESULT Toolbar::OnPaint(GXWndCanvas& canvas)
  {
    if(m_pSprite == NULL)
      return -1;

    GXRECT rect;

    gxGetClientRect(m_hWnd, &rect);

    int x = 0, y = 0;
    int ox = (m_ButtonSize.cx - m_BitmapSize.cx) / 2;
    int oy = (m_ButtonSize.cy - m_BitmapSize.cy) / 2;

    int nCount = (int)m_aButtons.size();
    for(int i = 0; i < nCount; i++)
    {
      GXColor32 clr = 0;
      /*if(i == m_nPressdItem) {
        clr = m_crPressd;
      }
      else */if(i == m_nCurItem) {
        clr = m_crHover;
      }
      DrawItem(canvas, i, clr, ox, oy, x, y);
    }

    return 0;
  }

  void Toolbar::ClearItem()
  {
    if(m_nCurItem != -1)
    {
      int nItem = m_nCurItem;
      m_nCurItem = -1;
      GXWndCanvas canvas(m_hWnd, NULL);
      DrawItem(canvas, nItem, 0, TRUE);
    }
  }

  void Toolbar::AutoSize()
  {
    GXRegn regn;
    if(Measure(&regn) == 0)
    {
      gxSetWindowPos(m_hWnd, NULL, 0, 0, regn.w, regn.h, NULL);
    }
  }

  void Toolbar::DrawItem(GXWndCanvas& canvas, int nItem, GXColor32 clr, GXBOOL bErase)
  {
    if(nItem < 0 || TEST_FLAG(m_aButtons[nItem].fsStyle, BTNS_SEP) ||
      TEST_FLAG(m_aButtons[nItem].fsState, TBSTATE_HIDDEN)) {
        return;
    }
    const int ox = (m_ButtonSize.cx - m_BitmapSize.cx) / 2;
    const int oy = (m_ButtonSize.cy - m_BitmapSize.cy) / 2;
    
    GXRECT rect;
    GetItemRect(nItem, &rect);

    if(bErase)
    {
      GXHTHEME hTheme = gxGetWindowTheme(m_hWnd);
      if(hTheme)
      {
        GXCanvas* pCanvas = canvas.GetCanvasUnsafe();
        pCanvas->SetCompositingMode(CM_SourceCopy);
        GXDrawThemeBackground(hTheme, pCanvas, GXWP_DIALOG, 0, &rect, NULL);
        pCanvas->SetCompositingMode(CM_SourceOver);
      }
    }
    DrawItem(canvas, nItem, clr, ox, oy, (int&)rect.left, (int&)rect.top);
  }

  void Toolbar::DrawItem(GXWndCanvas& canvas, int nItem, GXColor32 clr, int ox, int oy, int& x, int& y)
  {
    TOOLBARBUTTON& btn = m_aButtons[nItem];
    if(TEST_FLAG(btn.fsState, TBSTATE_HIDDEN)) {
      return;
    }

    if(TEST_FLAG(btn.fsStyle, BTNS_SEP))
    {
      int nWidth = btn.wWidth;
      canvas.DrawLine(x + nWidth / 2, y, x + nWidth / 2, y + m_ButtonSize.cy, 0xff000000);
      x += nWidth;
      return;
    }

    if(TEST_FLAG(btn.fsState, TBSTATE_CHECKED))
    {
      clr = m_crChecked;
    }

    if(clr != (GXDWORD)0) {
      GXRECT rect;
      GetItemRect(nItem, &rect);

      canvas.FillRect(x, y, rect.right - x, m_ButtonSize.cy, clr.color);
      canvas.DrawRect(x, y, rect.right - x, m_ButtonSize.cy, clr.color | 0xff000000);
    }

    int nBtnSize = ox;
    if(m_pSprite && btn.nSprite >= 0) {
      GXRegn regn(x + ox, y + oy, m_BitmapSize.cx, m_BitmapSize.cy);
      m_pSprite->PaintModule(canvas.GetCanvasUnsafe(), btn.nSprite, regn);
      nBtnSize = m_ButtonSize.cx;
    }
    
    if(TEST_FLAG(btn.fsStyle, BTNS_SHOWTEXT) && btn.strText.GetLength() > 0) {
      GXRECT rect;
      rect.left   = x + nBtnSize;
      rect.top    = y;
      rect.right  = x + btn.wWidth;
      rect.bottom = y + m_ButtonSize.cy;
      canvas.DrawTextW(m_pFont, btn.strText, -1, &rect, GXDT_SINGLELINE|GXDT_VCENTER, m_crText.ARGB);
    }

    x += btn.wWidth;
  }

  //GXLPCWSTR Toolbar::GetBtnText(const TOOLBARBUTTON* pTBBtn) const
  //{
  //  return pTBBtn->strText;
  //}

  //int Toolbar::GetBtnWidth(const TOOLBARBUTTON* pTBBtn)
  //{
  //  if(TEST_FLAG(pTBBtn->fsStyle, BTNS_SHOWTEXT)) {
  //    GXWndCanvas canvas(m_hWnd, NULL);
  //    return GetBtnWidth(&canvas, pTBBtn);
  //  }
  //  return GetBtnWidth(NULL, pTBBtn);
  //}

  int Toolbar::GetBtnWidth(GXWndCanvas* pCanvas, const TOOLBARBUTTON* pTBBtn)
  {
    // 分割线
    if(TEST_FLAG(pTBBtn->fsStyle, BTNS_SEP)) {
      return 5;
    }

    const int nBtnSize = pTBBtn->nSprite < 0 
      ? (m_ButtonSize.cx - m_BitmapSize.cx) / 2 
      : m_ButtonSize.cx;

    if(TEST_FLAG(pTBBtn->fsStyle, BTNS_SHOWTEXT)) {
      ASSERT(pCanvas != NULL);
      GXRECT rect(0);
      pCanvas->DrawTextW(m_pFont, pTBBtn->strText, -1, &rect, 
        GXDT_SINGLELINE | GXDT_CALCRECT, 0);
      return clMin(m_nMaxBtnWidth, nBtnSize + rect.right + (m_ButtonSize.cx - m_BitmapSize.cx) / 2);
    }
    return nBtnSize;
  }

  int Toolbar::GetButtonIndex(GXINT_PTR idCommand) const
  {
    for(ToolBtnArray::const_iterator it = m_aButtons.begin();
      it != m_aButtons.end(); ++it)
    {
      if((IS_PTR(idCommand) && IS_PTR(it->idCommand) && *(clStringW*)&it->idCommand == (GXLPCWSTR)idCommand) ||
        (it->idCommand == (GXLPCWSTR)idCommand)) {
        return (int)(it - m_aButtons.begin());
      }
    }
    return -1;
  }

  int Toolbar::GetCheckedGroupButtonByIndex(int nIndex) const
  {
    if(TEST_FLAGS_ALL(m_aButtons[nIndex].fsStyle, BTNS_CHECK | BTNS_GROUP)) {
      if(TEST_FLAG(m_aButtons[nIndex].fsState, TBSTATE_CHECKED)) {
        return nIndex;
      }}

    int nRunButton = nIndex - 1;
    int nButtonCount = (int)m_aButtons.size();

    while(nRunButton >= 0) {
      if(TEST_FLAG(m_aButtons[nRunButton].fsStyle, BTNS_GROUP)) {
        if(TEST_FLAG(m_aButtons[nRunButton].fsState, TBSTATE_CHECKED)) {
          return nRunButton;
        }
      }
      else break;
      --nRunButton;
    }
    nRunButton = nIndex + 1;
    while(nRunButton < nButtonCount) {
      if(TEST_FLAG(m_aButtons[nRunButton].fsStyle, BTNS_GROUP)) {
        if(TEST_FLAG(m_aButtons[nRunButton].fsState, TBSTATE_CHECKED)) {
          return nRunButton;
        }
      }
      else break;
      ++nRunButton;
    }
    return -1;
  }

  int Toolbar::GetCheckedGroupButton(int idCommand) const
  {
    const int nIndex = GetButtonIndex(idCommand);
    if(nIndex < 0) {
      return -1;
    }

    return GetCheckedGroupButtonByIndex(nIndex);   
  }

  GXBOOL Toolbar::IsButtonChecked(int idCommand)
  {
    const int nIndex = GetButtonIndex(idCommand);
    if(nIndex < 0) {
      return -1;
    }

    return m_aButtons[nIndex].fsState & TBSTATE_CHECKED;
  }
  
  GXBOOL Toolbar::SetButtonSize(int cx, int cy)
  {
    m_ButtonSize.cx = cx;
    m_ButtonSize.cy = cy;
    CalcButtonSize(0);
    Invalidate(FALSE);
    return TRUE;
  }

  GXBOOL Toolbar::SetBitmapSize(int cx, int cy)
  {
    m_BitmapSize.cx = cx;
    m_BitmapSize.cy = cy;
    CalcButtonSize(0);
    Invalidate(FALSE);
    return TRUE;
  }

  GXBOOL Toolbar::CheckButton( int idCommand, GXBOOL fCheck )
  {
    const int nIndex = GetButtonIndex(idCommand);
    if(nIndex < 0) {
      return FALSE;
    }

    return CheckButtonByIndex(nIndex, fCheck);
  }

  GXBOOL Toolbar::CheckButtonByIndex( int nIndex, GXBOOL fCheck )
  {
    TOOLBARBUTTON& tbb = m_aButtons[nIndex];
    if(TEST_FLAG(tbb.fsState, TBSTATE_CHECKED) == fCheck) {
      return TRUE;
    }

    GXWndCanvas canvas(m_hWnd, NULL);
    if(TEST_FLAG(tbb.fsStyle, BTNS_GROUP))
    {
      int nPrevChecked = GetCheckedGroupButtonByIndex(nIndex);
      if(nPrevChecked >= 0)
      {
        RESET_FLAG(m_aButtons[nPrevChecked].fsState, TBSTATE_CHECKED);
        DrawItem(canvas, nPrevChecked, 0, TRUE);
      }
      SET_FLAG(tbb.fsState, TBSTATE_CHECKED);
      DrawItem(canvas, nIndex, 0, FALSE);
    }
    else 
    {
      if(TEST_FLAG(tbb.fsStyle, BTNS_CHECK))
      {
        SWITCH_FLAG(tbb.fsState, TBSTATE_CHECKED);
      }
      //DrawItem(canvas, m_nPressdItem, m_crHover, TRUE);
      DrawItem(canvas, nIndex, m_crHover, TRUE);
    }
    return TRUE;
  }

  GXLPCWSTR Toolbar::GetIdNameW( GXINT Id )
  {
    if(m_bIndexedCommand) {
      // 作为索引的话为了避开GXWM_GETIDNAMEW消息对wParam==0的处理
      // 发送消息时索引加1，在这里就要减1得到正确索引
      if(Id > (GXINT)m_aButtons.size()) {
        return NULL;
      }
      return m_aButtons[Id - 1].idCommand;
    }
    return (GXLPCWSTR)Id;
  }

  GXBOOL Toolbar::SolveDefinition( const GXDefinitionArrayW& aDefinitions )
  {
    const GXDWORD dwStyle = (DWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);    

    for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
      it != aDefinitions.end(); ++it)
    {
      if(it->Name == _CLTEXT("SpritePath")) {
        LPGXGRAPHICS pGraphics = GXGetGraphics(m_hWnd);
        clStringW strFilename = it->Value;
        pGraphics->ConvertToAbsolutePathW(strFilename);
        SetSpriteFile(strFilename);
      }
    }
    return TRUE;
  }

  GXBOOL Toolbar::GetButtonInfo(GXINT_PTR idCommand, GXTBBUTTONINFOW* pInfo) const
  {
    //////////// 如果idCommand是字符串模式的话,标志中必须指定按照索引查找
    //////////if(m_bIndexedCommand && TEST_FLAG_NOT(pInfo->dwMask, GXTBIF_BYINDEX)) {
    //////////  return FALSE;
    //////////}

    const int nIndex = TEST_FLAG(pInfo->dwMask, GXTBIF_BYINDEX) 
      ? (int)idCommand : GetButtonIndex(idCommand);

    const TOOLBARBUTTON& sButton = m_aButtons[nIndex];
    if(pInfo->cbSize == sizeof(GXTBBUTTONINFOW))
    {
      if(TEST_FLAG(pInfo->dwMask, GXTBIF_COMMAND)) {
        pInfo->idCommand = m_bIndexedCommand ? nIndex : (int)sButton.idCommand;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_IMAGE)) {
        pInfo->iImage = sButton.nSprite;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_LPARAM)) {
        pInfo->lParam = sButton.dwData;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_SIZE)) {
        pInfo->cx = sButton.wWidth;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_STATE)) {
        pInfo->fsState = sButton.fsState;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_STYLE)) {
        pInfo->fsStyle = sButton.fsStyle;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_TEXT)) {
        GXSTRCPYN(pInfo->pszText, (GXLPCWSTR)sButton.strText, pInfo->cchText);
      }
    }
    return TRUE;
  }

  GXBOOL Toolbar::SetButtonInfo(GXINT_PTR idCommand, const GXTBBUTTONINFOW* pInfo)
  {
    //////////// 如果idCommand是字符串模式的话,标志中必须指定按照索引查找
    //////////if(m_bIndexedCommand && TEST_FLAG_NOT(pInfo->dwMask, GXTBIF_BYINDEX)) {
    //////////  return FALSE;
    //////////}

    const int nIndex = TEST_FLAG(pInfo->dwMask, GXTBIF_BYINDEX) 
      ? (int)idCommand : GetButtonIndex(idCommand);
    TOOLBARBUTTON& sButton = m_aButtons[nIndex];
    GXBOOL bRedraw = FALSE;

    if(pInfo->cbSize == sizeof(GXTBBUTTONINFOW))
    {
      // 不支持更改idCommand
      //if(TEST_FLAG(pInfo->dwMask, GXTBIF_COMMAND)) {
      //  pInfo->idCommand = m_bIndexedCommand ? nIndex : (int)sButton.idCommand;
      //}

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_IMAGE) && sButton.nSprite != pInfo->iImage) {
        sButton.nSprite = pInfo->iImage;
        bRedraw = TRUE;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_LPARAM) && sButton.dwData != pInfo->lParam) {
        sButton.dwData = pInfo->lParam;
        bRedraw = TRUE;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_SIZE) && sButton.wWidth != pInfo->cx) {
        sButton.wWidth = pInfo->cx;
        bRedraw = TRUE;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_STATE) && sButton.fsState != pInfo->fsState) {
        sButton.fsState = pInfo->fsState;
        bRedraw = TRUE;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_STYLE) && sButton.fsStyle != pInfo->fsStyle) {
        sButton.fsStyle = pInfo->fsStyle;
        bRedraw = TRUE;
      }

      if(TEST_FLAG(pInfo->dwMask, GXTBIF_TEXT) && sButton.strText != pInfo->pszText) {
        sButton.strText = pInfo->pszText;
        bRedraw = TRUE;
      }
    }

    if(bRedraw) {
      Invalidate(FALSE);
    }
    return TRUE;
  }

  GXBOOL Toolbar::HideButton(GXINT_PTR idButton, GXBOOL bHide)
  {
    const int nIndex = GetButtonIndex(idButton);
    if(nIndex < 0) {
      return FALSE;
    }

    TOOLBARBUTTON& sButton = m_aButtons[nIndex];

    if(bHide && TEST_FLAG_NOT(sButton.fsState, TBSTATE_HIDDEN)) {
      SET_FLAG(sButton.fsState, TBSTATE_HIDDEN);
    }
    else if( ! bHide && TEST_FLAG(sButton.fsState, TBSTATE_HIDDEN)) {
      RESET_FLAG(sButton.fsState, TBSTATE_HIDDEN);
    }
    else {
      return FALSE;
    }
    CalcButtonSize(nIndex);
    Invalidate(FALSE);
    return TRUE;
  }

  void Toolbar::InitToolTips(GXHWND hWnd)
  {
    m_hToolTip = gxCreateWindowEx(NULL, TOOLTIPS_CLASSW, _CLTEXT(""), GXWS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,
      GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT, NULL, NULL, NULL, NULL);

    //GXTOOLINFOW ti = {sizeof(GXTOOLINFOW)};
    //ti.uFlags = TTF_SUBCLASS;
    //ti.hwnd = hWnd;
    //ti.lpszText = L"1234";
    //ti.uId = 
    //gxGetClientRect(hWnd, &ti.rect);
    //gxSendMessage(pToolbar->m_hToolTip, GXTTM_ADDTOOLW, 0, (GXLPARAM)&ti);
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE