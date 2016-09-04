#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"

#include "User/GrapX.Hxx"
//#include "User/GXWindow.h"

#include "GrapX/GResource.H"
#include "GrapX/GXFont.H"
#include "GrapX/GXSprite.H"
#include "GrapX/GXGraphics.H"

#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"

#include "GXUICtrlBase.h"
#include "GXUISlider.h"
#include "GrapX/gxDevice.H"

namespace GXUI
{
  GXLPCTSTR c_szGXUIClassNameSlider = GXUICLASSNAME_SLIDER;
#define ENABLE_SCALING            TEST_FLAG(dwStyle, GXUISLDS_SCALING)
#define UPDATE_MINOR_PERCENT      if(TEST_FLAG(dwStyle, GXUISLDS_TRACKMINOR) && m_nLength > 0) { m_fMinorPercent = (float)m_nPos / m_nLength; }
//#define FIXED_SHIFT 16

  
  //////////////////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////////////////////////////////
  Slider::Slider(GXLPCWSTR szIdName)
    : CtrlBase(szIdName)
    , m_pSprite(NULL)
    , m_idHandle(0)
    , m_idFullBar(0)
    , m_idEmptyBar(0)
    , m_idDial(0)
    , m_idVertFullBar(0)
    , m_idVertEmptyBar(0)
    , m_idVertDial(0)
    , m_nPos(0)
    , m_nLength(0)
    , m_fMinorPercent(0)
  {
    m_Prime.n.Begin = 0;
    m_Prime.n.End = 100;
  }

  Slider* Slider::Create(
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
    Slider* pSlider = NULL;//new Slider(szIdName);

    //GXWNDCLASSEX wcex;
    //if(gxGetClassInfoEx(hInstance, GXUICLASSNAME_SLIDER, &wcex) == FALSE) {
    //  RegisterClass(hInstance, WndProc, GXUICLASSNAME_SLIDER);
    //}
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_SLIDER);

    GXHWND hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_SLIDER, lpWindowName, dwStyle, pRegn->left, pRegn->top, 
      pRegn->width, pRegn->height, hWndParent, NULL, hInstance, NULL);

    pSlider = (Slider*)gxGetWindowLongPtrW(hWnd, 0);

    if(pSlider->m_hWnd == NULL) {
      pSlider->Destroy();
      delete pSlider;
      pSlider = NULL;
    }
    return pSlider;
  }
  
  Slider* Slider::Create(GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions)
  {
    GXDWORD dwStyle = GXWS_VISIBLE | GXWS_CHILD | pDlgParam->dwStyle;
    Slider* pSlider = Slider::Create(pDlgParam->dwExStyle, NULL, dwStyle, &pDlgParam->regn, hParent, NULL, hInst, pDlgParam->strName, NULL);
    if(pSlider && pDefinitions) {
      pSlider->SolveDefinition(*pDefinitions);
    }
    return pSlider;
  }

  GXLRESULT Slider::Destroy()
  {
    SAFE_RELEASE(m_pSprite);
    return CtrlBase::Destroy();
  }

  GXBOOL Slider::SolveDefinition(const GXDefinitionArrayW& aDefinitions)
  {
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);    

    for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
      it != aDefinitions.end(); ++it)
    {
      if(it->Name == L"SliderBegin") {
        if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT)) {
          SetRangeBegin(dwStyle, (float)it->Value.ToFloat());
        }
        else {
          SetRangeBegin(dwStyle, it->Value.ToInteger());
        }
      }
      else if(it->Name == L"SliderEnd") {
        if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT)) {
          SetRangeEnd(dwStyle, (float)it->Value.ToFloat());
        }
        else {
          SetRangeEnd(dwStyle, it->Value.ToInteger());
        }
      }
      else if(it->Name == L"DataPool")
      {
        SetDataPoolVariableW(it->Value);
      }
    }
    return TRUE;
  }

  GXLRESULT GXCALLBACK Slider::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    Slider* pThis = (Slider*)CtrlBase::PreClassObj(hWnd, message, lParam);
    GXPOINT pos;
    switch(message)
    {
    case GXWM_LBUTTONDOWN:
      {
        pos.x = GXGET_X_LPARAM(lParam);
        pos.y = GXGET_Y_LPARAM(lParam);
        pThis->OnLButtonDown(&pos);
      }
      break;

    case GXWM_LBUTTONUP:
      {
        pos.x = GXGET_X_LPARAM(lParam);
        pos.y = GXGET_Y_LPARAM(lParam);
        pThis->OnLButtonUp(&pos);
      }
      break;

    case GXWM_DATAPOOLOPERATION:
      if(wParam == DPO_SETVARIABLE) {
        return pThis->SetVariable((MOVariable*)lParam);
      }
      else return -1;

    case GXWM_NCCREATE:
      {
        GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;
        GXLPCWSTR szIdName = IS_IDENTIFY(lpcs->hMenu) ? NULL : (GXLPCWSTR)lpcs->hMenu;
        Slider* pSlider = new Slider(szIdName);

        return pSlider->OnNCCreate(hWnd, message, wParam, lParam);
      }
      //if(pSlider == NULL) {
      //  CLBREAK;
      //  return FALSE;
      //}
      //pSlider->m_hWnd = hWnd;
      //pSlider->m_hNotifyWnd = lpcs->hwndParent;
      //gxSetWindowLong(hWnd, 0, (GXLONG_PTR)pSlider);
      //return gxDefWindowProc(hWnd, message, wParam, lParam);

    case GXSBM_SETPOS:
      {
        GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(hWnd, GXGWL_STYLE);
        return pThis->SetPos(dwStyle, (GXINT)wParam);
      }

    case GXSBM_GETPOS:
    case GXSBM_SETRANGE:
    case GXSBM_SETRANGEREDRAW:
    case GXSBM_GETRANGE:
    case GXSBM_ENABLE_ARROWS:
    case GXSBM_SETSCROLLINFO:
    case GXSBM_GETSCROLLINFO:
    case GXSBM_GETSCROLLBARINFO:
      CLBREAK; // TODO: 没实现
      break;

    }
    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  GXBOOL Slider::SetSprite(const DlgXM::DLGSLIDERSPRITE& Desc)
  {
    GXBOOL bval = TRUE;
    SAFE_RELEASE(m_pSprite);
    GXGraphics* pGraphics = GXGetGraphics(m_hWnd);

    if(GXSUCCEEDED(GXCreateSpriteFromFileW(pGraphics, Desc.strResource, &m_pSprite)))
    {
      m_idHandle = m_pSprite->Find(Desc.strHandle);
      if(m_idHandle < 0) {
        bval = FALSE;
        m_idHandle = 0;
      }

      m_idFullBar = m_pSprite->Find(Desc.strFull);
      if(m_idFullBar < 0) {
        bval = FALSE;
        m_idFullBar = 0;
      }

      m_idEmptyBar = m_pSprite->Find(Desc.strEmpty);
      if(m_idEmptyBar < 0) {
        bval = FALSE;
        m_idEmptyBar = 0;
      }

      m_idDial = m_pSprite->Find(Desc.strDial);
      if(m_idDial < 0) {
        bval = FALSE;
        m_idDial = 0;
      }

      m_idVertFullBar = m_pSprite->Find(Desc.strVertFull);
      if(m_idVertFullBar < 0) {
        bval = FALSE;
        m_idVertFullBar = 0;
      }

      m_idVertEmptyBar = m_pSprite->Find(Desc.strVertEmpty);
      if(m_idVertEmptyBar < 0) {
        bval = FALSE;
        m_idVertEmptyBar = 0;
      }

      m_idVertDial = m_pSprite->Find(Desc.strVertDial);
      if(m_idVertDial < 0) {
        bval = FALSE;
        m_idVertDial = 0;
      }

      if(m_VarPos.IsValid())
      {
        GXRECT rect;
        GXREGN regn[HTS_COUNT];

        gxGetClientRect(m_hWnd, &rect);
        GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);

        UPDATE_MINOR_PERCENT;
        IntCalcRects(&rect, dwStyle, regn);

        UpdatePosFromVar(dwStyle);
      }
    }
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  GXLRESULT Slider::OnLButtonDown(GXLPPOINT pos)
  {
    GXRECT rect;
    GXREGN regn[HTS_COUNT];

    gxGetClientRect(m_hWnd, &rect);
    HTSlider eTest = IntHitTest(&rect, pos, regn);
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    
    if(eTest == HTS_HANDLE)
    {
      GXMSG msg;
      int nPrevPos = m_nPos;
      //int nPrevMouse = pos->x;
      gxSetCapture(m_hWnd);
      while(gxGetMessage(&msg, NULL))
      {
        if(msg.message == GXWM_MOUSEMOVE)
        {
          GXPOINT ptCur = msg.pt;
          gxScreenToClient(m_hWnd, &ptCur);
          if(TEST_FLAG_NOT(dwStyle, GXUISLDS_VERT)) // 水平方向的滑动条
          {
            m_nPos = nPrevPos + ptCur.x - pos->x;
            ASSERT(rect.right - regn[HTS_HANDLE].width == m_nLength);
            clClamp(0, (int)m_nLength, &m_nPos);
          }
          else // 竖直方向的滑动条
          {
            m_nPos = nPrevPos + ptCur.y - pos->y;
            ASSERT(rect.bottom - regn[HTS_HANDLE].height == m_nLength);
            clClamp(0, (int)m_nLength, &m_nPos);
          }

          UPDATE_MINOR_PERCENT;
          UpdateVarFromPos(dwStyle);
          Invalidate(FALSE);
        }
        else if(msg.message >= GXWM_MOUSEFIRST && msg.message <= GXWM_MOUSELAST)
        {
          gxReleaseCapture();
          break;
        }
        gxDispatchMessageW(&msg);
      } // while

      if(TEST_FLAG(dwStyle, GXUISLDS_DISCRETE)) {
        AlignPos(dwStyle);
        Invalidate(FALSE);
      }
    }
    else if((eTest == HTS_FULL || eTest == HTS_EMPTY) && TEST_FLAG(dwStyle, GXUISLDS_THUMB))
    {
      if(TEST_FLAG_NOT(dwStyle, GXUISLDS_VERT)) {
        m_nPos = pos->x - GetDialLeft(regn);
      }
      else {
        m_nPos = pos->y - GetDialTop(regn);
      }
      UpdateVarFromPos(dwStyle);

      if(TEST_FLAG(dwStyle, GXUISLDS_DISCRETE)) {
        AlignPos(dwStyle);
      }
      Invalidate(FALSE);
    }
    return 0;
  }

  GXLRESULT Slider::OnLButtonUp(GXLPPOINT pos)
  {
    return 0;
  }

  GXLRESULT Slider::SetRangeBegin(GXDWORD dwStyle, GXINT nValue)
  {
    if(TEST_FLAG_NOT(dwStyle, GXUISLDS_FLOAT))
    {
      GXINT nPrev = m_Prime.n.Begin;
      m_Prime.n.Begin = nValue;
      return (GXLRESULT)nPrev;
    }
    else return SetRangeBegin(dwStyle, (float)nValue);
  }

  GXLRESULT Slider::SetRangeEnd(GXDWORD dwStyle, GXINT nValue)
  {
    if(TEST_FLAG_NOT(dwStyle, GXUISLDS_FLOAT))
    {
      GXINT nPrev = m_Prime.n.End;
      m_Prime.n.End = nValue;
      return (GXLRESULT)nPrev;
    }
    else return SetRangeEnd(dwStyle, (float)nValue);
  }

  GXLRESULT Slider::SetRangeBegin(GXDWORD dwStyle, GXFLOAT fValue)
  {
    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT))
    {
      GXFLOAT fPrev = m_Prime.f.Begin;
      m_Prime.f.Begin = fValue;
      return (GXLRESULT)fPrev;
    }
    else return SetRangeBegin(dwStyle, (GXINT)fValue);
  }

  GXLRESULT Slider::SetRangeEnd(GXDWORD dwStyle, GXFLOAT fValue)
  {
    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT))
    {
      GXFLOAT fPrev = m_Prime.f.End;
      m_Prime.f.End = fValue;
      return (GXLRESULT)fPrev;
    }
    else return SetRangeEnd(dwStyle, (GXINT)fValue);
  }

  GXLRESULT Slider::SetPos(GXDWORD dwStyle, GXINT nValue)
  {
    if(m_nLength == 0) {
      return GX_FAIL;
    }

    if( ! TEST_FLAG(dwStyle, GXUISLDS_FLOAT))
    {
      if((nValue >= m_Prime.n.Begin && nValue <= m_Prime.n.End) ||
        (nValue <= m_Prime.n.Begin && nValue >= m_Prime.n.End) )
      {
        if(m_Prime.n.End > m_Prime.n.Begin) {
          m_nPos = (nValue - m_Prime.n.Begin) * m_nLength / (m_Prime.n.End - m_Prime.n.Begin);
        }
        else {
          m_nPos = (nValue - m_Prime.n.End) * m_nLength / (m_Prime.n.End - m_Prime.n.Begin);
        }
      }
      else {
        return GX_FAIL;
      }
      return GX_OK;
    }
    else return SetPos(dwStyle, (GXFLOAT)nValue);
  }

  GXLRESULT Slider::SetPos(GXDWORD dwStyle, GXFLOAT fValue)
  {
    if(m_nLength == 0) {
      return GX_FAIL;
    }

    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT))
    {
      if((fValue >= m_Prime.f.Begin && fValue <= m_Prime.f.End) ||
        (fValue <= m_Prime.f.Begin && fValue >= m_Prime.f.End) )
      {
        if(m_Prime.f.End > m_Prime.f.Begin) {
          m_nPos = (int)((fValue - m_Prime.f.Begin) * m_nLength / (m_Prime.f.End - m_Prime.f.Begin));
        }
        else {
          m_nPos = (int)((fValue - m_Prime.f.End) * m_nLength / (m_Prime.f.End - m_Prime.f.Begin));
        }
      }
      else {
        return GX_FAIL;
      }
      return GX_OK;
    }
    else return SetPos(dwStyle, (GXINT)fValue);
  }


  Slider::HTSlider Slider::IntHitTest(GXRECT* rcClient, GXPOINT* pos, GXREGN* pRegns)
  {
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    IntCalcRects(rcClient, dwStyle, pRegns);
    if(gxPtInRegn(&pRegns[HTS_HANDLE], *pos))
    {
      TRACE("HTS_HANDLE\n");
      return HTS_HANDLE;
    }
    else if(gxPtInRegn(&pRegns[HTS_FULL], *pos))
    {
      TRACE("HTS_FULL\n");
      return HTS_FULL;
    }
    else if(gxPtInRegn(&pRegns[HTS_EMPTY], *pos))
    {
      TRACE("HTS_EMPTY\n");
      return HTS_EMPTY;
    }

    return HTS_COUNT;
  }

  void Slider::UpdatePosFromVar(GXDWORD dwStyle)
  {
    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT)) {
      SetPos(dwStyle, m_VarPos.ToFloat());
    }
    else {
      SetPos(dwStyle, (GXINT)m_VarPos.ToInteger());
    }
  }

  void Slider::UpdateVarFromPos(GXDWORD dwStyle)
  {
    if( ! m_VarPos.IsValid()) {
      return;
    }

    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT))
    {
      float s = clLerp(m_Prime.f.Begin, m_Prime.f.End, (float)m_nPos / (float)m_nLength);
      m_VarPos.Set(s);
    }
    else
    {
      int n = clLerp(m_Prime.n.Begin, m_Prime.n.End, (float)m_nPos / (float)m_nLength);
      m_VarPos.Set(n);
    }
  }


  GXLRESULT Slider::SetVariable(MOVariable* pVariable)
  {
    if(SetDataVariable(m_hWnd, m_VarPos, pVariable))
    {
      // TODO: 判断float/int类型
      const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
      UpdatePosFromVar(dwStyle);
    }
    return GX_OK;
  }

  //GXHRESULT Slider::OnKnock(KNOCKACTION* pKnock)
  GXVOID Slider::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    ASSERT(m_VarPos.IsValid());
    auto eCate = m_VarPos.GetTypeCategory();
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    switch(eCate)
    {
    case Marimo::T_FLOAT:
      SetPos(dwStyle, m_VarPos.ToFloat());
      break;
    case Marimo::T_BYTE:
    case Marimo::T_WORD:
    case Marimo::T_DWORD:
    case Marimo::T_QWORD:
    case Marimo::T_SBYTE:
    case Marimo::T_SWORD:
    case Marimo::T_SDWORD:
    case Marimo::T_SQWORD:
      SetPos(dwStyle, (GXINT)m_VarPos.ToInteger());
      break;
    default:
      return;
    }
    InvalidateRect(NULL, FALSE);
  }

  void Slider::IntCalcRects(GXRECT* rcClient, GXDWORD dwStyle, GXREGN* pRegns)
  {
    GXREGN rgHandle;
    GXREGN rgEmptyFull;

    m_pSprite->GetModuleRegion(m_idHandle, &rgHandle);

    GXINT nHandleSize;

    // 横向布局
    if(TEST_FLAG_NOT(dwStyle, GXUISLDS_VERT))
    {
      GXINT nBarHeight;
      m_pSprite->GetModuleRegion(m_idEmptyBar, &rgEmptyFull);

      if(ENABLE_SCALING)
      {
        float fScaling = (float)rcClient->bottom / rgHandle.height;
        nHandleSize = rcClient->bottom;
        nBarHeight = (GXINT)(rgEmptyFull.height * fScaling);
      }
      else
      {
        nHandleSize = rgHandle.height;
        nBarHeight = rgEmptyFull.height;
      }

      GXINT nPrime = rcClient->right - nHandleSize;
      GXINT nMinor = (GXINT)(nPrime * m_fMinorPercent);

      // 被 FullBar 覆盖的 EmptyBar 部分不渲染.还要考虑圆角问题
      GXINT nFactor = clMax(nMinor - rgEmptyFull.width, 0);

      gxSetRegn(&pRegns[HTS_EMPTY], 
        rcClient->left + nHandleSize / 2 + nFactor,
        (nHandleSize - nBarHeight) / 2, 
        nPrime - nFactor, nBarHeight);

      gxSetRegn(&pRegns[HTS_FULL], 
        rcClient->left + nHandleSize / 2, 
        (nHandleSize - nBarHeight) / 2, 
        nMinor, nBarHeight);

      m_nLength = (GXUINT)nPrime;

      //if(m_VarPos.IsValid()) {
      //  UpdatePosFromVar(dwStyle);
      //}

      gxSetRegn(&pRegns[HTS_HANDLE], 
        rcClient->left + m_nPos, rcClient->top, 
        nHandleSize, nHandleSize);
    }
    else // 纵向布局
    {
      GXINT nBarWidth;
      m_pSprite->GetModuleRegion(m_idVertEmptyBar, &rgEmptyFull);

      if(ENABLE_SCALING)
      {
        float fScaling = (float)rcClient->right / rgHandle.width;
        nHandleSize = rcClient->right;
        nBarWidth = (GXINT)(rgEmptyFull.width * fScaling);
      }
      else
      {
        nHandleSize = rgHandle.width;
        nBarWidth = rgEmptyFull.width;
      }

      GXINT nPrime = rcClient->bottom - nHandleSize;
      GXINT nMinor = (GXINT)(nPrime * m_fMinorPercent);

      // 被 FullBar 覆盖的 EmptyBar 部分不渲染.还要考虑圆角问题
      GXINT nFactor = clMax(nMinor - rgEmptyFull.height, 0);

      gxSetRegn(&pRegns[HTS_EMPTY], 
        (nHandleSize - nBarWidth) / 2,
        rcClient->top + nHandleSize / 2 + nFactor,
        nBarWidth, nPrime - nFactor);

      gxSetRegn(&pRegns[HTS_FULL],
        (nHandleSize - nBarWidth) / 2,
        rcClient->top + nHandleSize / 2, 
        nBarWidth, nMinor);

      m_nLength = (GXUINT)nPrime;

      //if(m_VarPos.IsValid()) {
      //  UpdatePosFromVar(dwStyle);
      //}

      gxSetRegn(&pRegns[HTS_HANDLE],
        rcClient->left, rcClient->top + m_nPos, 
        nHandleSize, nHandleSize);
    }
  }

  GXLRESULT Slider::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    GXREGN regn[HTS_COUNT];

    gxGetClientRect(m_hWnd, &rect);

    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);

    UPDATE_MINOR_PERCENT;
    IntCalcRects(&rect, dwStyle, regn);

    //if(m_VarPos.IsValid()) {
    //  UpdatePosFromVar(dwStyle);
    //}

    GXCanvas* pCanvas = canvas.GetCanvasUnsafe();

    if(TEST_FLAG_NOT(dwStyle, GXUISLDS_VERT))
    {
      m_pSprite->PaintModule3H(pCanvas, m_idEmptyBar, regn[HTS_EMPTY].left, 
        regn[HTS_EMPTY].top, regn[HTS_EMPTY].width, regn[HTS_EMPTY].height);

      if(regn[HTS_FULL].width > 0) {
        m_pSprite->PaintModule3H(pCanvas, m_idFullBar, regn[HTS_FULL].left, 
          regn[HTS_FULL].top, regn[HTS_FULL].width, regn[HTS_FULL].height);
      }

      // 刻度
      if(TEST_FLAG(dwStyle, GXUISLDS_DISCRETE))
      {
        PaintDial(pCanvas, dwStyle, GetDialLeft(regn), 
          regn[HTS_EMPTY].top + regn[HTS_EMPTY].height);
      }
    }
    else
    {
      m_pSprite->PaintModule3V(pCanvas, m_idVertEmptyBar, regn[HTS_EMPTY].left, 
        regn[HTS_EMPTY].top, regn[HTS_EMPTY].width, regn[HTS_EMPTY].height);

      if(regn[HTS_FULL].height > 0) {
        m_pSprite->PaintModule3V(pCanvas, m_idVertFullBar, regn[HTS_FULL].left, 
          regn[HTS_FULL].top, regn[HTS_FULL].width, regn[HTS_FULL].height);
      }

      // 竖直刻度
      if(TEST_FLAG(dwStyle, GXUISLDS_DISCRETE))
      {
        PaintDial(pCanvas, dwStyle, regn[HTS_EMPTY].left + regn[HTS_EMPTY].width,
          GetDialTop(regn));
      }
    }

    //m_pSprite->PaintModule(pCanvas, m_idHandle, regn[HTS_HANDLE].left, regn[HTS_HANDLE].top, regn[HTS_HANDLE].width, regn[HTS_HANDLE].height);
    m_pSprite->PaintModule(pCanvas, m_idHandle, &regn[HTS_HANDLE]);

    return 0;
  }

  void Slider::PaintDial(GXCanvas* pCanvas, GXDWORD dwStyle, int x, int y)
  {
    // 浮点模式不绘制刻度
    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT)) {
      return;
    }

    GXREGN rgDial;
    const GXUINT nRange = abs(m_Prime.n.End - m_Prime.n.Begin);

    if(nRange <= 0) {
      return;
    }

    const GXFLOAT fStep = (float)m_nLength / (float)nRange;

    // 刻度密度小于8(pixel)则不绘制刻度
    if((GXUINT)fStep < 8) {
      return;
    }

    if(TEST_FLAG_NOT(dwStyle, GXUISLDS_VERT))
    {
      m_pSprite->GetModuleRegion(m_idDial, &rgDial);
      GXFLOAT fLeft = (GXFLOAT)x;
      for(GXUINT n = 0; n <= nRange; n++)
      {
        m_pSprite->PaintModule(pCanvas, m_idDial, (GXUINT)fLeft - (rgDial.width >> 1), y);
        fLeft += fStep;
      }
    }
    else
    {
      m_pSprite->GetModuleRegion(m_idVertDial, &rgDial);
      GXFLOAT fTop = (GXFLOAT)y;
      for(GXUINT n = 0; n <= nRange; n++)
      {
        m_pSprite->PaintModule(pCanvas, m_idVertDial, x, (GXUINT)fTop - (rgDial.height >> 1));
        fTop += fStep;
      }
    }
  }

  void Slider::AlignPos(GXDWORD dwStyle)
  {
    // 浮点模式不支持
    if(TEST_FLAG(dwStyle, GXUISLDS_FLOAT)) {
      return;
    }

    const GXUINT nRange = abs(m_Prime.n.End - m_Prime.n.Begin);
    if(nRange <= 0) {
      return;
    }
    GXFLOAT fStep = (float)m_nLength / (float)nRange;
    m_nPos = (int)(floor((m_nPos + fStep * 0.5f) / fStep) * fStep);
  }

  GXLRESULT Slider::Measure(GXRegn* pRegn)
  {
    return -1;
  }

  //////////////////////////////////////////////////////////////////////////
} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE