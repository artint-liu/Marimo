#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"

#include "User/GrapX.Hxx"
//#include "User/GXWindow.h"

#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GXFont.H"
#include "Include/GXSprite.H"
#include "Include/GXGraphics.H"

#include "Include/GXUser.H"
#include "Include/GXGDI.H"
#include "Include/DataPool.H"
#include "Include/DataPoolVariable.H"
#include "GXUICtrlBase.h"
#include "GXUIStatic.h"
#include "Include/GXCanvas.H"
#include "gxDevice.H"

namespace GXUI
{
  GXLPCTSTR c_szGXUIClassNameStatic  = GXUICLASSNAME_STATIC;
  GXLPCTSTR c_szGXUIClassNameButton  = GXUICLASSNAME_BUTTON;
  GXLPCTSTR c_szGXUIClassNameList    = GXUICLASSNAME_LIST;
  GXLPCTSTR c_szGXUIClassNameToolbar = GXUICLASSNAME_TOOLBAR;

  
  //////////////////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////////////////////////////////
  Static::Static(GXLPCWSTR szIdName, Type eType)
    : CtrlBase(szIdName)
    , m_eType(eType)
  {
  }

  Static* Static::Create(
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
    Static* pStatic = NULL;

    // Check
    if(TEST_FLAG(dwStyle, GXUISS_NOTIFY) && TEST_FLAG(dwExStyle, GXWS_EX_IMPALPABLE)) {
      CLOG_WARNING("Warning: Style or ExStyle flags conflict.\n");
    }

    //GXWNDCLASSEX wcex;
    //if(gxGetClassInfoEx(hInstance, GXUICLASSNAME_STATIC, &wcex) == FALSE) {
    //  RegisterClass(hInstance, WndProc, GXUICLASSNAME_STATIC);
    //}
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_STATIC);

    GXHWND hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_STATIC, lpWindowName, dwStyle, pRegn->left, pRegn->top, 
      pRegn->width, pRegn->height, hWndParent, (GXHMENU)szIdName, hInstance, NULL);

    pStatic = (Static*)gxGetWindowLongPtrW(hWnd, 0);

    if(pStatic->m_hWnd == NULL) {
      pStatic->Destroy();
      delete pStatic;
      pStatic = NULL;
    }
    return pStatic;
  }
  
  Static* Static::Create(GXHINSTANCE hInst, GXHWND hParent, Type eType, GXLPCWSTR szTemplate, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions)
  {
    GXDWORD dwStyle = GXWS_VISIBLE | GXWS_CHILD | ((GXDWORD)eType) | pDlgParam->dwStyle;
    Static* pStatic = Static::Create(pDlgParam->dwExStyle, pDlgParam->strCaption, dwStyle, &pDlgParam->regn, hParent, NULL, hInst, szTemplate, NULL);
    if(pStatic && pDefinitions) {
      pStatic->SolveDefinition(*pDefinitions);
    }
    return pStatic;       
  }

  GXLRESULT Static::Destroy()
  {
    return CtrlBase::Destroy();
  }

  GXLRESULT GXCALLBACK Static::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    Static* pThis = (Static*)CtrlBase::PreClassObj(hWnd, message, lParam);
    switch(message)
    {
    case GXWM_LBUTTONDOWN:
      {
        GXDWORD dwStyle = gxGetWindowLong(hWnd, GXGWL_STYLE);

        if( ! pThis->ShouldNotifyParent())
          break;

        pThis->NotifyParent(GXSTN_CLICKED);
      }
      break;

    case GXWM_DATAPOOLOPERATION:
      if(wParam == DPO_SETVARIABLE) {
        return pThis->SetVariable((MOVariable*)lParam);
      }
      else return -1;
    case GXWM_KNOCK:
      {
        pThis->OnKnock((KNOCKACTION*)lParam);
      }
      break;

    case GXWM_NCCREATE:
      {
        GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;
        Static* pStatic = NULL;
        GXLPCWSTR szIdName = IS_IDENTIFY(lpcs->hMenu) ? NULL : (GXLPCWSTR)lpcs->hMenu;
        switch(lpcs->style & GXUISS_TYPE_MASK)
        {
        case GXUISS_TYPE_LABEL:
          pStatic = new StaticLabel(szIdName);
          break;
        case GXUISS_TYPE_SPRITE:
          pStatic = new StaticSprite(szIdName);
          break;
        case GXUISS_TYPE_RECT:
          pStatic = new StaticRectangle(szIdName);
          break;
        }
        return pStatic->OnNCCreate(hWnd, message, wParam, lParam);
        //if(pStatic == NULL) {
        //  CLBREAK;
        //  return FALSE;
        //}
        //InlSetNewObjectT(pStatic->m_pFont, GXUIGetStock()->pDefaultFont);
        //pStatic->m_hWnd = hWnd;
        //pStatic->m_hNotifyWnd = lpcs->hwndParent;
        //gxSetWindowLong(hWnd, 0, (GXLONG_PTR)pStatic);
        //return TRUE;
      }
      break;
    } // switch

    if(pThis->GetType() == Sprite)
    {
      StaticSprite* pSprite = static_cast<StaticSprite*>(pThis);
      switch(message)
      {
      case GXSSM_SETSPRITEBYFILENAMEW:
        if(pSprite->SetSpriteByFilenameW((GXLPCWSTR)lParam)) {
          return 0;
        }
        break;
      case GXSSM_SETSPRITE:
        if(pSprite->SetSprite((GXSprite*)lParam)) {
          return 0;
        }
        break;
      case GXSSM_SETMODULEBYNAMEW:
        if(pSprite->SetByNameW((GXLPCWSTR)lParam)) {
          return 0;
        }
        break;
      case GXSSM_SETMODULEBYINDEX:
        if(pSprite->SetByIndex((int)wParam)) {
          return 0;
        }
        break;

      case GXSSM_GETSPRITEBYFILENAMEW:
        if(pSprite->GetSpriteByFilenameW((GXLPWSTR)lParam, (GXINT)wParam)) {
          return 0;
        }
        break;
      case GXSSM_GETSPRITE:
        if(pSprite->GetSprite((GXSprite**)lParam)) {
          return 0;
        }
        break;
      case GXSSM_GETMODULEBYNAMEW:
        if(pSprite->GetByNameW((GXLPWSTR)lParam, (GXINT)wParam)) {
          return 0;
        }
        break;
      case GXSSM_GETMODULEBYINDEX:
        return pSprite->GetByIndex();
      }
    }
    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  GXLRESULT Static::SetVariable(MOVariable* pVariable)
  {
    return -1;
  }

  GXHRESULT Static::OnKnock(KNOCKACTION* pKnock)
  {
    return 0;
  }

  //GXBOOL Static::SolveDefinition(const GXDefinitionArray& aDefinitions)
  //{
  //  for(GXDefinitionArray::const_iterator it = aDefinitions.begin();
  //    it != aDefinitions.end(); ++it)
  //  {
  //    if(it->Name == "DataPool") {
  //      SetDataPoolVariable(it->Value);
  //    }
  //  }
  //  return TRUE;
  //}
  //////////////////////////////////////////////////////////////////////////
  StaticLabel::StaticLabel(GXLPCWSTR szIdName)
    : Static    (szIdName, Static::Label)
    , m_crText  (0xff000000)
  {

  }
  GXLRESULT StaticLabel::OnPaint(GXWndCanvas& canvas)
  {
    //int nLen = gxGetWindowTextLength(m_hWnd);

    GXLPCWSTR lpText = NULL;
    clStringW strVar;
    //if(m_VarText.IsValid()) {
    //  strVar = m_VarText.ToStringW();
    //  lpText = strVar;
    //}
    //else {
    lpText = IntGetText();
    //}
    //gxInternalGetWindowText(m_hWnd, lpText, nLen);
    int nLen = GXSTRLEN(lpText);

    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GWL_STYLE);
    if(TEST_FLAG(dwStyle, GXUISS_SIMPLE))
    {
      canvas.TextOutW(m_pFont, rect.left, rect.top, lpText, nLen, m_crText);
    }
    else
    {
      const GXUINT uDrawStyle = GetDrawTextFlag();
      if(TEST_FLAG(dwStyle, GXUISS_CONTRAST)) {
        GXRECT rcContrast = {rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1};
        canvas.DrawTextW(m_pFont, lpText, nLen, &rcContrast, uDrawStyle, 
          m_crText ^ 0xffffff);
      }
      canvas.DrawTextW(m_pFont, lpText, nLen, &rect, uDrawStyle, m_crText);
    }

    //SAFE_DELETE(lpText);
    return 0;
  }

  GXDWORD StaticLabel::GetDrawTextFlag()
  {
    GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GWL_STYLE);

    GXDWORD dwDrawTexStyle = NULL;
    if(TEST_FLAG(dwStyle, GXUISS_LEFT)) {
      dwDrawTexStyle |= GXDT_LEFT;
    }
    if(TEST_FLAG(dwStyle, GXUISS_CENTER)) {
      dwDrawTexStyle |= GXDT_CENTER;
    }
    if(TEST_FLAG(dwStyle, GXUISS_RIGHT)) {
      dwDrawTexStyle |= GXDT_RIGHT;
    }
    if(TEST_FLAG(dwStyle, GXUISS_TOP)) {
      dwDrawTexStyle |= GXDT_TOP;
    }
    if(TEST_FLAG(dwStyle, GXUISS_VCENTER)) {
      dwDrawTexStyle |= GXDT_VCENTER;
    }
    if(TEST_FLAG(dwStyle, GXUISS_BOTTOM)) {
      dwDrawTexStyle |= GXDT_BOTTOM;
    }
    if(TEST_FLAG(dwStyle, GXUISS_NOPREFIX)) {
      dwDrawTexStyle |= GXDT_NOPREFIX;
    }
    if(TEST_FLAG(dwStyle, GXUISS_SINGLELINE)) {
      dwDrawTexStyle |= GXDT_SINGLELINE;
    }
    if(TEST_FLAG(dwStyle, GXUISS_WORDBREAK)) {
      dwDrawTexStyle |= GXDT_WORDBREAK;
    }
    if(TEST_FLAG(dwStyle, GXUISS_EXPANDTABS)) {
      dwDrawTexStyle |= GXDT_EXPANDTABS;
    }
    return dwDrawTexStyle;
  }

  GXLRESULT StaticLabel::SetVariable(MOVariable* pVariable)
  {
    if(SetDataVariable(m_hWnd, m_VarText, pVariable))
    {
      gxSetWindowTextW(m_hWnd, m_VarText.ToStringW());
    }
    return 0;
  }

  GXHRESULT StaticLabel::OnKnock(KNOCKACTION* pKnock)
  {
    ASSERT(m_VarText.IsValid());
    if(pKnock->pSponsor != &m_VarText && m_VarText.GetName() == pKnock->Name)
    {
      gxSetWindowTextW(m_hWnd, m_VarText.ToStringW());
      Invalidate(FALSE);
    }
    return 0;
  }

  GXCOLORREF StaticLabel::SetColor(GXCOLORREF crText)
  {
    GXCOLORREF crPrevText = m_crText;
    m_crText = crText;
    return crPrevText;
  }

  GXCOLORREF StaticLabel::GetColor()
  {
    return m_crText;
  }
 
  GXLRESULT StaticLabel::Measure(GXRegn* pRegn)
  {
    int nLen = gxGetWindowTextLength(m_hWnd);
    if(nLen == 0) {
      return -1;
    }

    GXLPCWSTR lpText = IntGetText();
    //  new GXWCHAR[nLen + 1];
    //gxInternalGetWindowText(m_hWnd, lpText, nLen);

    //GXCanvas* pCanvas = 0;
    GXRECT rect;
    ((GXCanvas*)NULL)->DrawTextW(m_pFont, lpText, - 1, &rect, GetDrawTextFlag()|GXDT_CALCRECT, 0);
    *pRegn = rect;

    //SAFE_DELETE(lpText);
    return 0;
  }
  //////////////////////////////////////////////////////////////////////////
  StaticRectangle::StaticRectangle(GXLPCWSTR szIdName)
    : Static    (szIdName, Static::Rectangle)
    , m_crMeta  (0xff000000)
  {

  }
  GXLRESULT StaticRectangle::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    canvas.FillRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, m_crMeta);

    return 0;
  }

  GXCOLORREF StaticRectangle::SetColor(GXCOLORREF crText)
  {
    GXCOLORREF crPrevText = m_crMeta;
    m_crMeta = crText;
    return crPrevText;
  }

  GXCOLORREF StaticRectangle::GetColor()
  {
    return m_crMeta;
  }

  GXLRESULT StaticRectangle::Measure(GXRegn* pRegn)
  {
    pRegn->x = 0;
    pRegn->y = 0;
    pRegn->w = 128;
    pRegn->h = 128;
    return 0;
  }
  //////////////////////////////////////////////////////////////////////////
  StaticSprite::StaticSprite(GXLPCWSTR szIdName)
    : Static      (szIdName, Static::Sprite)
    , m_pSprite   (NULL)
    , m_nSpriteIdx(-1)
  {
  }

  GXLRESULT StaticSprite::OnPaint(GXWndCanvas& canvas)
  {
    if(m_pSprite != NULL && m_nSpriteIdx >= 0)
    {
      GXRECT rect;
      gxGetClientRect(m_hWnd, &rect);

      GXREGN regn;
      if(m_pSprite->GetSpriteBounding(m_nSpriteIdx, &regn))
      {
        const float xScale = (float)rect.right / regn.width;
        const float yScale = (float)rect.bottom / regn.height;

        m_pSprite->PaintSprite(canvas.GetCanvasUnsafe(), m_nSpriteIdx, 0,
          -(GXINT)(regn.left * xScale), -(GXINT)(regn.top * yScale), xScale, yScale);
      }
    }
    return 0;
  }
  GXLRESULT StaticSprite::Destroy()
  {
    SAFE_RELEASE(m_pSprite);
    return CtrlBase::Destroy();
  }

  void StaticSprite::ClearSprite()
  {
    SAFE_RELEASE(m_pSprite);
    m_strSpriteFile.Clear();
    m_strSpriteName.Clear();
    m_nSpriteIdx = -1;
  }
  
  GXBOOL StaticSprite::SetSpriteByFilenameW(GXLPCWSTR szSpriteFile)
  {
    GXSprite* pSprite = NULL;
    GXGraphics* pGraphics = GXGetGraphics(m_hWnd);

    if(GXSUCCEEDED(GXCreateSpriteFromFileW(pGraphics, szSpriteFile, &pSprite)))
    {
      ClearSprite();
      m_strSpriteFile = szSpriteFile;
      m_pSprite = pSprite;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL StaticSprite::SetSprite(GXSprite* pSprite)
  {
    if(pSprite != NULL) 
    {
      ClearSprite();
      m_pSprite = pSprite;
      m_pSprite->AddRef();
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL StaticSprite::SetByNameW(LPCWSTR szName)
  {
    m_nSpriteIdx = m_pSprite->FindByNameW(szName);
    if(m_nSpriteIdx >= 0)
    {
      m_strSpriteName = szName;
      Invalidate(FALSE);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL StaticSprite::SetByIndex(int nIndex)
  {
    m_strSpriteName.Clear();
    m_nSpriteIdx = nIndex;
    Invalidate(FALSE);
    return TRUE;
  }

  GXBOOL StaticSprite::GetSpriteByFilenameW(GXLPWSTR szSpriteFile, GXINT nMaxBuffer)
  {
    GXSTRCPYN(szSpriteFile, (GXLPCWSTR)m_strSpriteFile, nMaxBuffer);
    return TRUE;
  }

  GXBOOL StaticSprite::GetSprite(GXSprite** ppSprite)
  {
    if(m_pSprite) {
      *ppSprite = m_pSprite;
      m_pSprite->AddRef();
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL StaticSprite::GetByNameW(GXLPWSTR szName, GXINT nMaxBuffer)
  {
    GXSTRCPYN(szName, (GXLPCWSTR)m_strSpriteName, nMaxBuffer);
    return TRUE;
  }

  int StaticSprite::GetByIndex()
  {
    return m_nSpriteIdx;
  }

  GXLRESULT StaticSprite::Measure(GXRegn* pRegn)
  {
    return -1;
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE