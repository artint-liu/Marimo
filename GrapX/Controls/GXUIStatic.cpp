#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.h"

#include "User/GrapX.Hxx"
//#include "User/GXWindow.h"

#include "GrapX/GResource.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXSprite.h"
#include "GrapX/GXGraphics.h"

#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataPoolIterator.h"
#include "GXUICtrlBase.h"
#include "GXUIStatic.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/gxDevice.h"

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
        GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(hWnd, GXGWL_STYLE);

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

    //case GXWM_IMPULSE:
    //  {
    //    pThis->OnImpulse((LPCDATAIMPULSE)lParam);
    //  }
    //  break;

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

    if(pThis->GetType() == Type_Sprite)
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
        if(pSprite->SetByIndex((GXUINT)wParam)) {
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

      //default:
      //  CLBREAK;
      //  break;
      }
    }
    else if(pThis->GetType() == Type_Label) {
      switch(message)
      {
      case GXWM_SETTEXT:
        GXLRESULT lr = CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
        pThis->Invalidate(FALSE);
        return lr;
      }
    }
    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  GXLRESULT Static::SetVariable(MOVariable* pVariable)
  {
    return -1;
  }

  GXVOID Static::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    CLBREAK;
    //return 0;
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
    : Static    (szIdName, Static::Type_Label)
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
    int nLen = (int)GXSTRLEN(lpText);

    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GWL_STYLE);
    if(TEST_FLAG(dwStyle, GXUISS_SIMPLE))
    {
      canvas.TextOutW(m_pFont, rect.left, rect.top, lpText, nLen, m_crText);
    }
    else
    {
      const GXUINT uDrawStyle = GetDrawTextFlag();
      if(TEST_FLAG(dwStyle, GXUISS_CONTRAST)) {
        GXRECT rcContrast(rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1);
        canvas.DrawText(m_pFont, lpText, nLen, &rcContrast, uDrawStyle, 
          m_crText ^ 0xffffff);
      }
      canvas.DrawText(m_pFont, lpText, nLen, &rect, uDrawStyle, m_crText);
    }

    //SAFE_DELETE(lpText);
    return 0;
  }

  GXDWORD StaticLabel::GetDrawTextFlag()
  {
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GWL_STYLE);

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

  //GXHRESULT StaticLabel::OnKnock(KNOCKACTION* pKnock)
  GXVOID StaticLabel::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    ASSERT(m_VarText.IsValid());
    ASSERT(m_VarText.GetPtr() == pImpulse->sponsor->GetPtr());

    gxSetWindowTextW(m_hWnd, m_VarText.ToStringW());
    Invalidate(FALSE);
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
    ((GrapX::GXCanvas*)NULL)->DrawText(m_pFont, lpText, - 1, &rect, GetDrawTextFlag()|GXDT_CALCRECT, 0);
    *pRegn = rect;

    //SAFE_DELETE(lpText);
    return 0;
  }
  //////////////////////////////////////////////////////////////////////////
  StaticRectangle::StaticRectangle(GXLPCWSTR szIdName)
    : Static    (szIdName, Static::Type_Rectangle)
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
    : Static      (szIdName, Static::Type_Sprite)
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
      GXSprite::Type type;
      GXINT index = m_pSprite->UnpackIndex(m_nSpriteIdx, &type);
      if(index < 0) {
        CLBREAK;
        return GX_FAIL;
      }

      // TODO: 这个以后想想怎么改, 重新设计接口
      switch(type)
      {
      case GXSprite::Type_Module:
        if(m_pSprite->GetModuleRegion(index, &regn))
        {
          //const float xScale = (float)rect.right / regn.width;
          //const float yScale = (float)rect.bottom / regn.height;

          //GXRegn regn(-(GXINT)(regn.left * xScale), -(GXINT)(regn.top * yScale), (GXINT)xScale, (GXINT)yScale);
          m_pSprite->PaintModule(canvas.GetCanvasUnsafe(), index, rect.left, rect.top, rect.right, rect.bottom);
        }
        break;

      case GXSprite::Type_Frame:
        {
          //GXRECT rcFrame;
          //m_pSprite->GetFrameBounding(index, &rcFrame);
          regn = rect;
          m_pSprite->PaintFrame(canvas.GetCanvasUnsafe(), index, &rect);
        }
        //if(m_pSprite->GetFrameBounding(index, &regn))
        //{
        //  const float xScale = (float)rect.right / regn.width;
        //  const float yScale = (float)rect.bottom / regn.height;

        //  GXRegn regn(-(GXINT)(regn.left * xScale), -(GXINT)(regn.top * yScale), (GXINT)xScale, (GXINT)yScale);
        //  m_pSprite->PaintFrame(canvas.GetCanvasUnsafe(), index, &regn);
        //}
        break;

      case GXSprite::Type_Animation:
        m_pSprite->PaintAnimationFrame(canvas.GetCanvasUnsafe(), index, 0, &rect);
        //if(m_pSprite->GetModuleRegion(index, &regn))
        //{
        //  const float xScale = (float)rect.right / regn.width;
        //  const float yScale = (float)rect.bottom / regn.height;

        //  GXRegn regn(-(GXINT)(regn.left * xScale), -(GXINT)(regn.top * yScale), (GXINT)xScale, (GXINT)yScale);
        //  m_pSprite->PaintModule(canvas.GetCanvasUnsafe(), index, regn);
        //}
        break;
      default:
        CLBREAK;
        break;
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
    GrapX::Graphics* pGraphics = GXGetGraphics(m_hWnd);

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

  GXBOOL StaticSprite::SetByNameW(GXLPCWSTR szName)
  {
    GXSprite::Type type;
    m_nSpriteIdx = m_pSprite->Find(szName, &type);
    m_nSpriteIdx = m_pSprite->PackIndex(type, m_nSpriteIdx);

    //if(type == GXSprite::Type_Frame) {
    //  m_nSpriteIdx += (GXINT)m_pSprite->GetModuleCount();
    //}
    //else if(type == GXSprite::Type_Animation) {
    //  m_nSpriteIdx += (GXINT)(m_pSprite->GetModuleCount() + m_pSprite->GetFrameCount());
    //}
    //else {
    //  ASSERT(type == GXSprite::Type_Module);
    //}

    if(m_nSpriteIdx != -1)
    {
      m_strSpriteName = szName;
      Invalidate(FALSE);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL StaticSprite::SetByIndex(GXINT index)
  {
    ASSERT(index >= 0);
    m_strSpriteName.Clear();
    m_nSpriteIdx = index;
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

  GXINT StaticSprite::GetByIndex()
  {
    return m_nSpriteIdx;
  }

  GXLRESULT StaticSprite::Measure(GXRegn* pRegn)
  {
    return -1;
  }

  GXLRESULT StaticSprite::SetVariable( MOVariable* pVariable )
  {
    auto eCate = pVariable->GetTypeCategory();
    if(eCate == Marimo::DataPoolTypeClass::String)
    {
      clStringW str = pVariable->ToStringW();
      clStringW strFilename, strName;
      str.DivideBy(L':', strFilename, strName);
      SetSpriteByFilenameW(strFilename);
      SetByNameW(strName);
    }
    else if(eCate == Marimo::DataPoolTypeClass::Structure)
    {
      auto it = pVariable->begin();
      auto itEnd = pVariable->end();
      GXSprite* pSprite = NULL;
      clStringW strName;

      // 结构体对象
      // 通过迭代器扫描到Object和String类型成员，作为SpriteObject和SpriteName
      for(; it != itEnd; ++it)
      {
        auto eMemberCate = it.pVarDesc->GetTypeClass();
        if(eMemberCate == Marimo::DataPoolTypeClass::Object) {
          GUnknown* pObject = NULL;
          it.ToVariable().Query(&pObject);
          pSprite = dynamic_cast<GXSprite*>(pObject);
        }
        else if(eMemberCate == Marimo::DataPoolTypeClass::String) {
          strName = it.ToVariable().ToStringW();
        }

        if(pSprite && strName.IsNotEmpty()) {
          break;
        }
      }

      
      if(pSprite)
      {
        SetSprite(pSprite);
        SetByNameW(strName);
      }

      SAFE_RELEASE(pSprite);
    }
    return 0;
  }

  GXVOID StaticSprite::OnImpulse( LPCDATAIMPULSE pImpulse )
  {
    CLNOP
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE