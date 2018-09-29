#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.h"
#include "GrapX/GResource.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXSprite.h"
#include "GrapX/GXGraphics.h"

#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

#include "GXUICtrlBase.h"
#include "GXUIButton.h"
#include "GrapX/gxDevice.h"

namespace GXUI
{
  Button::Button(GXLPCWSTR szIdName)
    : CtrlBase    (szIdName)
    , m_pSprite   (NULL)
    , m_nNormal   (0)
    , m_nHover    (0)
    , m_nPressed  (0)
    , m_nDisabled (0)
    , m_nDefault  (0)
    , m_dwState   (NULL)
    , m_crText    (0xff000000)
    , m_crDisabledText(0xff808080)
  {
  }

  Button* Button::Create(
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
    Button* pButton = NULL;
    pButton = new Button(szIdName);
    //switch(dwStyle & GXUISS_TYPE_MASK)
    //{
    //case GXUISS_TYPE_LABEL:
    //  pStatic = new StaticLabel;
    //  break;
    //case GXUISS_TYPE_SPRITE:
    //  pStatic = new StaticSprite;
    //  break;
    //}

    //GXWNDCLASSEX wcex;
    //if(gxGetClassInfoEx(hInstance, GXUICLASSNAME_BUTTON, &wcex) == FALSE) {
    //  RegisterClass(hInstance, WndProc, GXUICLASSNAME_BUTTON);
    //}
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_BUTTON);

    pButton->m_hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_BUTTON, lpWindowName, dwStyle, pRegn->left, pRegn->top, 
      pRegn->width, pRegn->height, hWndParent, (GXHMENU)szIdName, hInstance, pButton);

    pButton->m_pFont = GXUIGetStock()->pDefaultFont;
    pButton->m_pFont->AddRef();

    return pButton;
  }

  Button* Button::Create(GXHINSTANCE hInst, GXHWND hParent, GXLPCTSTR szText, GXDWORD dwStyle, GXLPCWSTR szIdName, const GXRegn* pRegn, const GXDefinitionArrayW* pDefinitions)
  {
    dwStyle |= GXWS_VISIBLE | GXWS_CHILD;
    Button* pButton = Button::Create(NULL, szText, dwStyle, pRegn, hParent, NULL, hInst, szIdName, NULL);
    if(pButton && pDefinitions) {
      pButton->SolveDefinition(*pDefinitions);
    }
    return pButton;
  }

  GXLRESULT Button::Destroy()
  {
    SAFE_RELEASE(m_pSprite);
    return CtrlBase::Destroy();
  }
  
  GXLRESULT Button::Measure(GXRegn* pRegn)
  {
    return -1;
  }

  GXBOOL Button::SetSprite(const DlgXM::DLGBTNSPRITE& Desc)
  {
    return SetSprite(Desc.strResource, Desc.strNormal, 
      Desc.strHover, Desc.strPressed, Desc.strDisable, Desc.strDefault);
  }

  GXBOOL Button::SetSprite(LPCWSTR szSpriteFile, LPCWSTR szNormal, LPCWSTR szHover, LPCWSTR szPressed, LPCWSTR szDisabled, LPCWSTR szDefault)
  {
    GXBOOL bval = TRUE;
    SAFE_RELEASE(m_pSprite);
    GXGraphics* pGraphics = GXGetGraphics(m_hWnd);

    if(GXSUCCEEDED(GXCreateSpriteFromFileW(pGraphics, szSpriteFile, &m_pSprite)))
    {
      m_nNormal   = m_pSprite->Find(szNormal);
      if(m_nNormal < 0) {
        bval = FALSE;
        m_nNormal = 0;
      }

      m_nHover    = m_pSprite->Find(szHover);
      if(m_nHover < 0) {
        bval = FALSE;
        m_nHover = 0;
      }

      m_nPressed  = m_pSprite->Find(szPressed);
      if(m_nPressed < 0) {
        bval = FALSE;
        m_nPressed = 0;
      }

      m_nDisabled = m_pSprite->Find(szDisabled);
      if(m_nDisabled < 0) {
        bval = FALSE;
        m_nDisabled = 0;
      }

      m_nDefault  = m_pSprite->Find(szDefault);
      if(m_nDefault < 0) {
        bval = FALSE;
        m_nDefault = 0;
      }
    }
    else {
      CLOG_ERRORW(L"Can not load sprite(%s).\r\n", szSpriteFile);
    }
    return TRUE;
  }

  GXLRESULT GXCALLBACK Button::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    Button* pThis = (Button*)CtrlBase::PreClassObj(hWnd, message, lParam);

    switch(message)
    {
    case GXWM_MOUSEHOVER:
      SET_FLAG(pThis->m_dwState, GXUIBUTTON_STATE_HOVER);
      pThis->Invalidate(FALSE);
      break;

    case GXWM_MOUSELEAVE:
      RESET_FLAG(pThis->m_dwState, GXUIBUTTON_STATE_HOVER);
      pThis->Invalidate(FALSE);
      break;

    case GXWM_LBUTTONDOWN:
      SET_FLAG(pThis->m_dwState, GXUIBUTTON_STATE_PRESSED);
      pThis->Invalidate(FALSE);
      break;

    case GXWM_LBUTTONUP:
      RESET_FLAG(pThis->m_dwState, GXUIBUTTON_STATE_PRESSED);
      pThis->CommandNotify();
      pThis->Invalidate(FALSE);
      break;

    case GXWM_ENABLE:
      pThis->Invalidate(FALSE);
      break;
    }
    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  GXLRESULT Button::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    int nSprite = m_nNormal;

    gxGetClientRect(m_hWnd, &rect);
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);

    if(m_pSprite == NULL)
    {
      canvas.FillRect(rect.left, rect.top, rect.right, rect.bottom, 0xffff0000);
      return 0;
    }

    if(TEST_FLAG(dwStyle, GXWS_DISABLED))
    {
      nSprite = m_nDisabled;
    }
    else if(TEST_FLAGS_ALL(m_dwState, GXUIBUTTON_STATE_HOVER | GXUIBUTTON_STATE_PRESSED))
    {
      nSprite = m_nPressed;
    }
    else if(TEST_FLAG(m_dwState, GXUIBUTTON_STATE_HOVER))
    {
      nSprite = m_nHover;
    }

    m_pSprite->PaintModule3x3(canvas.GetCanvasUnsafe(), nSprite, FALSE, &rect);
    GXLPCWSTR lpText = IntGetText();
    if(lpText != NULL && GXSTRLEN(lpText) > 0) {
      GXDWORD dwDrawTextFlag = GetDrawTextFlag();
      GXRECT rcText = rect;

      if(TEST_FLAG(dwStyle, GXWS_DISABLED))
      {
        canvas.DrawTextW(m_pFont, lpText, -1, &rcText, dwDrawTextFlag, m_crDisabledText.color);
      }
      else
      {
        if(TEST_FLAG(m_dwState, GXUIBUTTON_STATE_PRESSED)) {
          gxOffsetRect(&rcText, 1, 1);
        }
        canvas.DrawTextW(m_pFont, lpText, -1, &rcText, dwDrawTextFlag, m_crText.color);
      }
    }
    return 0;
  }

  void Button::CommandNotify()
  {
    const int id = (int)gxGetWindowLong(m_hWnd, GXGWL_ID);
    gxSendMessage(m_hNotifyWnd, GXWM_COMMAND, GXMAKEWPARAM(id, GXBN_CLICKED), (GXLPARAM)m_hWnd);
  }

  GXDWORD Button::GetDrawTextFlag()
  {
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GWL_STYLE);
    GXDWORD dwDrawTexStyle = NULL;


    if(TEST_FLAG(dwStyle, GXUIBS_LEFT)) {
      dwDrawTexStyle |= GXDT_LEFT;
    }
    if(TEST_FLAG(dwStyle, GXUIBS_RIGHT)) {
      dwDrawTexStyle |= GXDT_RIGHT;
    }
    if(TEST_FLAG(dwStyle, GXUIBS_CENTER)) {
      dwDrawTexStyle |= GXDT_CENTER;
    }
    if(TEST_FLAG(dwStyle, GXUIBS_TOP)) {
      dwDrawTexStyle |= GXDT_TOP;
    }
    if(TEST_FLAG(dwStyle, GXUIBS_BOTTOM)) {
      dwDrawTexStyle |= GXDT_BOTTOM;
    }
    if(TEST_FLAG(dwStyle, GXUIBS_VCENTER)) {
      dwDrawTexStyle |= GXDT_VCENTER;
    }
    if( ! TEST_FLAG(dwStyle, GXUIBS_MULTILINE)) {
      dwDrawTexStyle |= GXDT_SINGLELINE;
    }
    return dwDrawTexStyle;
  }

  GXLRESULT Button::SetVariable( MOVariable* pVariable )
  {
    //if(SetDataVariable(m_hWnd, m_VarText, pVariable))
    //{
    //  gxSetWindowTextW(m_hWnd, m_VarText.ToStringW());
    //}
    gxSetWindowTextW(m_hWnd, pVariable->ToStringW());
    pVariable->GetPoolUnsafe()->Watch(pVariable, m_hWnd);
    return 0;
  }

  GXVOID Button::OnImpulse( LPCDATAIMPULSE pImpulse )
  {
    ASSERT(pImpulse->sponsor->IsValid());
    //ASSERT(pImpulse->sponsor->GetPtr() == pImpulse->sponsor->GetPtr());

    gxSetWindowTextW(m_hWnd, pImpulse->sponsor->ToStringW());
    Invalidate(FALSE);
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE