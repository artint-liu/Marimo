#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"

#include <Smart/smartstream.h>
#include <Smart/Stock.h>

#include "User/GrapX.Hxx"
#include "User/GXWindow.h"
#include "User/UILayoutMgr.h"

#include "GrapX/GResource.H"
#include "GrapX/GXFont.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXUser.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"

#include "GXUICtrlBase.h"
#include "GXUIStatic.h"
#include "GXUIButton.h"
//#include "GXUISlider.h"
#include "GrapX/gxDevice.H"

using namespace clstd;

namespace GXUI
{
  CtrlBase::CtrlBase(GXLPCWSTR szIdName)
    : m_nLock           (1)
    , m_hWnd            (NULL)
    , m_hNotifyWnd      (NULL)
    , m_pFont           (NULL)
    , m_strIdentityName (szIdName ? szIdName : L"")
  {
  }

  CtrlBase::~CtrlBase()
  {
  }

  GXDWORD CtrlBase::RegisterClass(GXHINSTANCE hInst, GXWNDPROC lpWndProc, GXLPCTSTR szClassName)
  {
    GXWNDCLASSEX wcex;

    wcex.cbSize = sizeof(GXWNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = lpWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = sizeof(CtrlBase*);
    wcex.hInstance      = hInst;
    wcex.hIcon          = NULL;
    wcex.hCursor        = gxLoadCursor(NULL, (GXLPCWSTR)IDC_ARROW);
    wcex.hbrBackground  = (GXHBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szClassName;
    wcex.hIconSm        = NULL;

    return gxRegisterClassExW(&wcex);
  }

  GXDWORD CtrlBase::TryRegisterClass(GXHINSTANCE hInst, GXWNDPROC lpWndProc, GXLPCTSTR szClassName)
  {
    GXWNDCLASSEX wcex;
    if(gxGetClassInfoEx(hInst, szClassName, &wcex) == FALSE) {
      return RegisterClass(hInst, lpWndProc, szClassName);
    }
    return 0;
  }

  GXLRESULT CtrlBase::Destroy()
  {
    SAFE_RELEASE(m_pFont);
    return 0;
  }

  GXBOOL CtrlBase::SolveDefinition(const GXDefinitionArrayW& aDefinitions)
  {
    for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
      it != aDefinitions.end(); ++it)
    {
      if(it->Name == L"DataPool") {
        SetDataPoolVariableW(it->Value);
      }
    }
    return TRUE;
  }

  GXHRESULT CtrlBase::SetFont(LPCWSTR szFontName, int nFontSize)
  {
    SAFE_RELEASE(m_pFont);
    GXGraphics* pGraphics = GXGetGraphics(m_hWnd);
    if(pGraphics != NULL) {
      m_pFont = pGraphics->CreateFontW(0, nFontSize, szFontName);
    }
    return GX_OK;
  }

  GXLPCWSTR CtrlBase::IntGetText() const
  {
    GXLPWND lpWnd = GXWND_PTR(m_hWnd);
    return lpWnd->m_pText;
  }

  GXBOOL CtrlBase::Invalidate(GXBOOL bErase)
  {
    return gxInvalidateRect(m_hWnd, NULL, bErase);
  }

  GXBOOL CtrlBase::InvalidateRect(GXLPCRECT lpRect, GXBOOL bErase)
  {
    return gxInvalidateRect(m_hWnd, lpRect, bErase);
  }

  GXBOOL CtrlBase::NotifyParent(GXDWORD wNotiMsg)
  {
    GXHWND hParent = gxGetWindow(m_hWnd, GXGW_PARENT);
    if(hParent) {
      GXDWORD wId = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_ID);
      gxSendMessage(hParent, GXWM_COMMAND, GXMAKEWPARAM(wId, wNotiMsg), (GXLPARAM)m_hWnd);
    }
    return FALSE;
  }

  GXBOOL CtrlBase::SetDataPoolVariableA(const clStringA& strExpression)
  {
    MOVariable Var;
    GXHRESULT hval = MODataPool::FindVariable(NULL, &Var, strExpression);
    if(GXSUCCEEDED(hval)) {
      SetVariable(&Var);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL CtrlBase::SetDataPoolVariableW(const clStringW& strExpression)
  {
    clStringA strExpressionA = (GXLPCWSTR)strExpression;
    return SetDataPoolVariableA(strExpressionA);
  }

  GXLPCWSTR CtrlBase::GetName()
  {
    return m_strIdentityName;
  }

  GXLRESULT CtrlBase::DefWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam, CtrlBase* pThis)
  {
    switch(message)
    {
    case GXWM_PAINT:
      {
        GXWndCanvas canvas(hWnd);
        //gxSendMessageW(hWnd, GXWM_ERASEBKGND, (GXWPARAM)lpPaint->hdc, 0);
        pThis->OnPaint(canvas);
        gxValidateRect(hWnd, NULL);
      }
      break;

    case GXWM_GETIDNAMEW:
      return (GXLRESULT)(GXLPCWSTR)pThis->m_strIdentityName;
      break;

    case GXWM_NCDESTROY:
      {
        pThis->Unlock();
        gxSetWindowLong(hWnd, 0, (GXLONG_PTR)0);
      }
      break;

    case GXWM_IMPULSE:
      {
        pThis->OnImpulse((LPCDATAIMPULSE)lParam);
      }
      break;
    }
    return gxDefWindowProcW(hWnd, message, wParam, lParam);
  }

  CtrlBase* CtrlBase::PreClassObj(GXHWND hWnd, GXUINT message, GXLPARAM lParam)
  {
    CtrlBase* pThis = NULL;
    if(message == GXWM_NCCREATE) {
      GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;
      gxSetWindowLong(hWnd, 0, (GXLONG_PTR)lpcs->lpCreateParams);
      pThis = (CtrlBase*)lpcs->lpCreateParams;
      if(pThis) // TODO: 重构,去掉这种模式,参考Static改!
      {
        ASSERT(lpcs->hwndParent != NULL);
        pThis->m_hNotifyWnd = lpcs->hwndParent;
        pThis->m_hWnd = hWnd;
      }
    }
    else {
      pThis = (CtrlBase*)gxGetWindowLongW(hWnd, 0);
    }
    return pThis;
  }

  GXLRESULT CtrlBase::OnNCCreate(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    const CtrlBase* pThis = this; // 不这么写clang下会报警告
    if(pThis == NULL) {
      CLOG_ERROR("%s : Can't create Toolbar object.\r\n", __FUNCTION__);
      return FALSE;
    }

    ASSERT(message == GXWM_NCCREATE);
    GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT)lParam;

    InlSetNewObjectT(m_pFont, GXUIGetStock()->pDefaultFont);
    m_hWnd = hWnd;
    m_hNotifyWnd = lpcs->hwndParent;
    gxSetWindowLong(hWnd, 0, (GXLONG_PTR)this);
    return gxDefWindowProc(hWnd, message, wParam, lParam);
  }

  GXBOOL CtrlBase::SetDataVariable(GXHWND hWnd, MOVariable& var, MOVariable* pNewVar)
  {
    if(var.IsValid()) {
      var.GetPoolUnsafe()->Ignore(&var, hWnd);
    }
    if(pNewVar && pNewVar->IsValid())
    {
      var = *pNewVar;
      var.GetPoolUnsafe()->Watch(&var, hWnd);
      return TRUE;
    }
    var.Free();
    return FALSE;
  }
} // namespace GXUI



namespace DlgXM
{

#define APPENDMARK(_MASK, _CLS)           \
  DlgCombMark.dwMask = _MASK;             \
  DlgCombMark.eClass = _CLS;              \
  s_CombMarkDict[#_MASK] = DlgCombMark;
#define DEFINE_COLOR(_CLRTEXT, _VALUE) s_ColorMarkDict[_CLRTEXT] = (_VALUE | 0xff000000)

  GXVOID InitializeDefinationTable()
  {
    DLGCOMBINEDMARK DlgCombMark;
    s_CombMarkDict.empty();
    APPENDMARK(GXWS_CAPTION     , CMC_WndStyle);
    APPENDMARK(GXWS_VISIBLE     , CMC_WndStyle);
    APPENDMARK(GXWS_OVERLAPPED  , CMC_WndStyle);
    APPENDMARK(GXWS_POPUP       , CMC_WndStyle);
    APPENDMARK(GXWS_CHILD       , CMC_WndStyle);
    APPENDMARK(GXWS_MINIMIZE    , CMC_WndStyle);
    APPENDMARK(GXWS_DISABLED    , CMC_WndStyle);
    APPENDMARK(GXWS_CLIPSIBLINGS, CMC_WndStyle);
    APPENDMARK(GXWS_CLIPCHILDREN, CMC_WndStyle);
    APPENDMARK(GXWS_MAXIMIZE    , CMC_WndStyle);
    APPENDMARK(GXWS_BORDER      , CMC_WndStyle);
    APPENDMARK(GXWS_DLGFRAME    , CMC_WndStyle);
    APPENDMARK(GXWS_VSCROLL     , CMC_WndStyle);
    APPENDMARK(GXWS_HSCROLL     , CMC_WndStyle);
    APPENDMARK(GXWS_SYSMENU     , CMC_WndStyle);
    APPENDMARK(GXWS_THICKFRAME  , CMC_WndStyle);
    APPENDMARK(GXWS_GROUP       , CMC_WndStyle);
    APPENDMARK(GXWS_TABSTOP     , CMC_WndStyle);
    APPENDMARK(GXWS_MINIMIZEBOX , CMC_WndStyle);
    APPENDMARK(GXWS_MAXIMIZEBOX , CMC_WndStyle);

    APPENDMARK(GXWS_EX_TRANSPARENT, CMC_WndExStyle);
    APPENDMARK(GXWS_EX_IMPALPABLE, CMC_WndExStyle);

    APPENDMARK(GXDS_CENTER, CMC_DlgStyle);

    APPENDMARK(GXUISS_LEFT,       CMC_StaticLabel);
    APPENDMARK(GXUISS_CENTER,     CMC_StaticLabel);
    APPENDMARK(GXUISS_RIGHT,      CMC_StaticLabel);
    APPENDMARK(GXUISS_TOP,        CMC_StaticLabel);
    APPENDMARK(GXUISS_VCENTER,    CMC_StaticLabel);
    APPENDMARK(GXUISS_BOTTOM,     CMC_StaticLabel);
    APPENDMARK(GXUISS_NOPREFIX,   CMC_StaticLabel);
    APPENDMARK(GXUISS_SINGLELINE, CMC_StaticLabel);
    APPENDMARK(GXUISS_WORDBREAK,  CMC_StaticLabel);
    APPENDMARK(GXUISS_SIMPLE,     CMC_StaticLabel);
    APPENDMARK(GXUISS_NOTIFY,     CMC_StaticLabel);
    APPENDMARK(GXUISS_CONTRAST,   CMC_StaticLabel);
    APPENDMARK(GXUISS_EXPANDTABS, CMC_StaticLabel);
    //APPENDMARK(GXUISS_DRAGNOTIFY, CMC_StaticLabel);

    APPENDMARK(GXUIBS_TEXT,       CMC_ButtonStyle);
    APPENDMARK(GXUIBS_LEFT,       CMC_ButtonStyle);
    APPENDMARK(GXUIBS_RIGHT,      CMC_ButtonStyle);
    APPENDMARK(GXUIBS_CENTER,     CMC_ButtonStyle);
    APPENDMARK(GXUIBS_TOP,        CMC_ButtonStyle);
    APPENDMARK(GXUIBS_BOTTOM,     CMC_ButtonStyle);
    APPENDMARK(GXUIBS_VCENTER,    CMC_ButtonStyle);
    APPENDMARK(GXUIBS_MULTILINE,  CMC_ButtonStyle);

    APPENDMARK(GXES_LEFT,         CMC_EditStyle);
    APPENDMARK(GXES_CENTER,       CMC_EditStyle);
    APPENDMARK(GXES_RIGHT,        CMC_EditStyle);
    APPENDMARK(GXES_MULTILINE,    CMC_EditStyle);
    APPENDMARK(GXES_UPPERCASE,    CMC_EditStyle);
    APPENDMARK(GXES_LOWERCASE,    CMC_EditStyle);
    APPENDMARK(GXES_PASSWORD,     CMC_EditStyle);
    APPENDMARK(GXES_AUTOVSCROLL,  CMC_EditStyle);
    APPENDMARK(GXES_AUTOHSCROLL,  CMC_EditStyle);
    APPENDMARK(GXES_NOHIDESEL,    CMC_EditStyle);
    APPENDMARK(GXES_OEMCONVERT,   CMC_EditStyle);
    APPENDMARK(GXES_READONLY,     CMC_EditStyle);
    APPENDMARK(GXES_WANTRETURN,   CMC_EditStyle);
    APPENDMARK(GXES_NUMBER,       CMC_EditStyle);

    APPENDMARK(GXLBS_MULTIPLESEL, CMC_StdListBox);
    APPENDMARK(GXLBS_NOSEL,       CMC_StdListBox);
    APPENDMARK(GXLBS_NOTIFY,      CMC_StdListBox);
    APPENDMARK(GXLBS_MULTICOLUMN, CMC_StdListBox);
    APPENDMARK(GXLBS_LTRSCROLLED, CMC_StdListBox);

    APPENDMARK(LPS_WNDITEM        , CMC_LayoutPanel);
    APPENDMARK(LPS_HBINARYPANEL   , CMC_LayoutPanel);
    APPENDMARK(LPS_VBINARYPANEL   , CMC_LayoutPanel);
    APPENDMARK(LPS_CROSSPANEL     , CMC_LayoutPanel);
    APPENDMARK(LPS_FREEPANEL      , CMC_LayoutPanel);
    APPENDMARK(LPS_TYPESETPANEL   , CMC_LayoutPanel);
    APPENDMARK(LPS_FIXED          , CMC_LayoutPanel);
    APPENDMARK(LPS_KEEPFIRST      , CMC_LayoutPanel);
    APPENDMARK(LPS_KEEPSECOND     , CMC_LayoutPanel);
    APPENDMARK(LPS_KEEPTHIRD      , CMC_LayoutPanel);
    APPENDMARK(LPS_KEEPFOURTH     , CMC_LayoutPanel);
    APPENDMARK(LPS_KEEPWEIGHT     , CMC_LayoutPanel);
    APPENDMARK(LPS_LINEWRAP       , CMC_LayoutPanel);


    APPENDMARK(GXUISLDS_HORZ      , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_VERT      , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_TRACKMINOR, CMC_SliderStyle);
    APPENDMARK(GXUISLDS_DISCRETE  , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_SCALING   , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_NOTIFY    , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_THUMB     , CMC_SliderStyle);
    APPENDMARK(GXUISLDS_FLOAT     , CMC_SliderStyle);


    APPENDMARK(GXMFT_STRING       , CMC_MenuType);
    APPENDMARK(GXMFT_BITMAP       , CMC_MenuType);
    APPENDMARK(GXMFT_MENUBARBREAK , CMC_MenuType);
    APPENDMARK(GXMFT_MENUBREAK    , CMC_MenuType);
    APPENDMARK(GXMFT_OWNERDRAW    , CMC_MenuType);
    APPENDMARK(GXMFT_RADIOCHECK   , CMC_MenuType);
    APPENDMARK(GXMFT_SEPARATOR    , CMC_MenuType);
    APPENDMARK(GXMFT_RIGHTORDER   , CMC_MenuType);
    APPENDMARK(GXMFT_RIGHTJUSTIFY , CMC_MenuType);

    
    APPENDMARK(GXMFS_GRAYED       , CMC_MenuState);
    APPENDMARK(GXMFS_DISABLED     , CMC_MenuState);
    APPENDMARK(GXMFS_CHECKED      , CMC_MenuState);
    APPENDMARK(GXMFS_HILITE       , CMC_MenuState);
    APPENDMARK(GXMFS_ENABLED      , CMC_MenuState);
    APPENDMARK(GXMFS_UNCHECKED    , CMC_MenuState);
    APPENDMARK(GXMFS_UNHILITE     , CMC_MenuState);
    APPENDMARK(GXMFS_DEFAULT      , CMC_MenuState);


    APPENDMARK(TBSTATE_CHECKED    , CMC_ToolbarState);
    APPENDMARK(TBSTATE_HIDDEN     , CMC_ToolbarState);

    
    APPENDMARK(TBSTYLE_TOOLTIPS   , CMC_ToolbarStyle);
    

    APPENDMARK(BTNS_BUTTON        , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_SEP           , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_CHECK         , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_GROUP         , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_CHECKGROUP    , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_DROPDOWN      , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_AUTOSIZE      , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_NOPREFIX      , CMC_ToolbarBtnStyle);
    APPENDMARK(BTNS_SHOWTEXT      , CMC_ToolbarBtnStyle);
    

    //APPENDMARK(GXUIBS_NOTIFY,     CMC_ButtonStyle);

    s_ColorMarkDict.empty();
    //s_ColorMarkDict["WHITE"]  = 0xffffffff;
    //s_ColorMarkDict["BLACK"]  = 0xff000000;
    //s_ColorMarkDict["BLUE"]   = 0xff0000ff;
    //s_ColorMarkDict["GREEN"]  = 0xff00ff00;
    //s_ColorMarkDict["RED"]    = 0xffff0000;


      DEFINE_COLOR("ALICEBLUE",             clstd::Color::AliceBlue           );
      DEFINE_COLOR("ANTIQUEWHITE",          clstd::Color::AntiqueWhite        );
      DEFINE_COLOR("AQUA",                  clstd::Color::Aqua                );
      DEFINE_COLOR("AQUAMARINE",            clstd::Color::Aquamarine          );
      DEFINE_COLOR("AZURE",                 clstd::Color::Azure               );
      DEFINE_COLOR("BEIGE",                 clstd::Color::Beige               );
      DEFINE_COLOR("BISQUE",                clstd::Color::Bisque              );
      DEFINE_COLOR("BLACK",                 clstd::Color::Black               );
      DEFINE_COLOR("BLANCHEDALMOND",        clstd::Color::Blanchedalmond      );
      DEFINE_COLOR("BLUE",                  clstd::Color::Blue                );
      DEFINE_COLOR("BLUEVIOLET",            clstd::Color::BlueViolet          );
      DEFINE_COLOR("BROWN",                 clstd::Color::Brown               );
      DEFINE_COLOR("BURLYWOOD",             clstd::Color::BurlyWood           );
      DEFINE_COLOR("CADETBLUE",             clstd::Color::CadetBlue           );
      DEFINE_COLOR("CHARTREUSE",            clstd::Color::Chartreuse          );
      DEFINE_COLOR("CHOCOLATE",             clstd::Color::Chocolate           );
      DEFINE_COLOR("CORAL",                 clstd::Color::Coral               );
      DEFINE_COLOR("CORNFLOWERBLUE",        clstd::Color::CornflowerBlue      );
      DEFINE_COLOR("CORNSILK",              clstd::Color::Cornsilk            );
      DEFINE_COLOR("CRIMSON",               clstd::Color::Crimson             );
      DEFINE_COLOR("CYAN",                  clstd::Color::Cyan                );
      DEFINE_COLOR("DARKBLUE",              clstd::Color::DarkBlue            );
      DEFINE_COLOR("DARKCYAN",              clstd::Color::DarkCyan            );
      DEFINE_COLOR("DARKGOLDENROD",         clstd::Color::DarkGoldenrod       );
      DEFINE_COLOR("DARKGRAY",              clstd::Color::DarkGray            );
      DEFINE_COLOR("DARKGREEN",             clstd::Color::DarkGreen           );
      DEFINE_COLOR("DARKGREY",              clstd::Color::DarkGrey            );
      DEFINE_COLOR("DARKKHAKI",             clstd::Color::DarkKhaki           );
      DEFINE_COLOR("DARKMAGENTA",           clstd::Color::DarkMagenta         );
      DEFINE_COLOR("DARKOLIVEGREEN",        clstd::Color::DarkOliveGreen      );
      DEFINE_COLOR("DARKORANGE",            clstd::Color::DarkOrange          );
      DEFINE_COLOR("DARKORCHID",            clstd::Color::DarkOrchid          );
      DEFINE_COLOR("DARKRED",               clstd::Color::DarkRed             );
      DEFINE_COLOR("DARKSALMON",            clstd::Color::DarkSalmon          );
      DEFINE_COLOR("DARKSEAGREEN",          clstd::Color::DarkSeaGreen        );
      DEFINE_COLOR("DARKSLATEBLUE",         clstd::Color::DarkSlateBlue       );
      DEFINE_COLOR("DARKSLATEGRAY",         clstd::Color::DarkSlateGray       );
      DEFINE_COLOR("DARKSLATEGREY",         clstd::Color::DarkSlateGrey       );
      DEFINE_COLOR("DARKTURQUOISE",         clstd::Color::DarkTurquoise       );
      DEFINE_COLOR("DARKVIOLET",            clstd::Color::DarkViolet          );
      DEFINE_COLOR("DEEPPINK",              clstd::Color::DeepPink            );
      DEFINE_COLOR("DEEPSKYBLUE",           clstd::Color::DeepSkyBlue         );
      DEFINE_COLOR("DIMGRAY",               clstd::Color::DimGray             );
      DEFINE_COLOR("DIMGREY",               clstd::Color::DimGrey             );
      DEFINE_COLOR("DODGERBLUE",            clstd::Color::DodgerBlue          );
      DEFINE_COLOR("FIREBRICK",             clstd::Color::FireBrick           );
      DEFINE_COLOR("FLORALWHITE",           clstd::Color::FloralWhite         );
      DEFINE_COLOR("FORESTGREEN",           clstd::Color::ForestGreen         );
      DEFINE_COLOR("FUCHSIA",               clstd::Color::Fuchsia             );
      DEFINE_COLOR("GAINSBORO",             clstd::Color::Gainsboro           );
      DEFINE_COLOR("GHOSTWHITE",            clstd::Color::GhostWhite          );
      DEFINE_COLOR("GOLD",                  clstd::Color::Gold                );
      DEFINE_COLOR("GOLDENROD",             clstd::Color::Goldenrod           );
      DEFINE_COLOR("GRAY",                  clstd::Color::Gray                );
      DEFINE_COLOR("GREEN",                 clstd::Color::Green               );
      DEFINE_COLOR("GREENYELLOW",           clstd::Color::GreenYellow         );
      DEFINE_COLOR("GREY",                  clstd::Color::Grey                );
      DEFINE_COLOR("HONEYDEW",              clstd::Color::Honeydew            );
      DEFINE_COLOR("HOTPINK",               clstd::Color::HotPink             );
      DEFINE_COLOR("INDIANRED",             clstd::Color::IndianRed           );
      DEFINE_COLOR("INDIGO",                clstd::Color::Indigo              );
      DEFINE_COLOR("IVORY",                 clstd::Color::Ivory               );
      DEFINE_COLOR("KHAKI",                 clstd::Color::Khaki               );
      DEFINE_COLOR("LAVENDER",              clstd::Color::Lavender            );
      DEFINE_COLOR("LAVENDERBLUSH",         clstd::Color::LavenderBlush       );
      DEFINE_COLOR("LAWNGREEN",             clstd::Color::LawnGreen           );
      DEFINE_COLOR("LEMONCHIFFON",          clstd::Color::LemonChiffon        );
      DEFINE_COLOR("LIGHTBLUE",             clstd::Color::LightBlue           );
      DEFINE_COLOR("LIGHTCORAL",            clstd::Color::LightCoral          );
      DEFINE_COLOR("LIGHTCYAN",             clstd::Color::LightCyan           );
      DEFINE_COLOR("LIGHTGOLDENRODYELLOW",  clstd::Color::LightGoldenrodYellow);
      DEFINE_COLOR("LIGHTGRAY",             clstd::Color::LightGray           );
      DEFINE_COLOR("LIGHTGREEN",            clstd::Color::LightGreen          );
      DEFINE_COLOR("LIGHTGREY",             clstd::Color::LightGrey           );
      DEFINE_COLOR("LIGHTPINK",             clstd::Color::LightPink           );
      DEFINE_COLOR("LIGHTSALMON",           clstd::Color::LightSalmon         );
      DEFINE_COLOR("LIGHTSEAGREEN",         clstd::Color::LightSeaGreen       );
      DEFINE_COLOR("LIGHTSKYBLUE",          clstd::Color::LightSkyBlue        );
      DEFINE_COLOR("LIGHTSLATEGRAY",        clstd::Color::LightSlateGray      );
      DEFINE_COLOR("LIGHTSLATEGREY",        clstd::Color::LightSlateGrey      );
      DEFINE_COLOR("LIGHTSTEELBLUE",        clstd::Color::LightSteelBlue      );
      DEFINE_COLOR("LIGHTYELLOW",           clstd::Color::LightYellow         );
      DEFINE_COLOR("LIME",                  clstd::Color::Lime                );
      DEFINE_COLOR("LIMEGREEN",             clstd::Color::LimeGreen           );
      DEFINE_COLOR("LINEN",                 clstd::Color::Linen               );
      DEFINE_COLOR("MAGENTA",               clstd::Color::Magenta             );
      DEFINE_COLOR("MAROON",                clstd::Color::Maroon              );
      DEFINE_COLOR("MEDIUMAQUAMARINE",      clstd::Color::MediumAquamarine    );
      DEFINE_COLOR("MEDIUMBLUE",            clstd::Color::MediumBlue          );
      DEFINE_COLOR("MEDIUMORCHID",          clstd::Color::MediumOrchid        );
      DEFINE_COLOR("MEDIUMPURPLE",          clstd::Color::MediumPurple        );
      DEFINE_COLOR("MEDIUMSEAGREEN",        clstd::Color::MediumSeaGreen      );
      DEFINE_COLOR("MEDIUMSLATEBLUE",       clstd::Color::MediumSlateBlue     );
      DEFINE_COLOR("MEDIUMSPRINGGREEN",     clstd::Color::MediumSpringGreen   );
      DEFINE_COLOR("MEDIUMTURQUOISE",       clstd::Color::MediumTurquoise     );
      DEFINE_COLOR("MEDIUMVIOLETRED",       clstd::Color::MediumVioletRed     );
      DEFINE_COLOR("MIDNIGHTBLUE",          clstd::Color::MidnightBlue        );
      DEFINE_COLOR("MINTCREAM",             clstd::Color::Mintcream           );
      DEFINE_COLOR("MISTYROSE",             clstd::Color::Mistyrose           );
      DEFINE_COLOR("MOCCASIN",              clstd::Color::Moccasin            );
      DEFINE_COLOR("NAVAJOWHITE",           clstd::Color::NavajoWhite         );
      DEFINE_COLOR("NAVY",                  clstd::Color::Navy                );
      DEFINE_COLOR("OLDLACE",               clstd::Color::Oldlace             );
      DEFINE_COLOR("OLIVE",                 clstd::Color::Olive               );
      DEFINE_COLOR("OLIVEDRAB",             clstd::Color::Olivedrab           );
      DEFINE_COLOR("ORANGE",                clstd::Color::Orange              );
      DEFINE_COLOR("ORANGERED",             clstd::Color::OrangeRed           );
      DEFINE_COLOR("ORCHID",                clstd::Color::Orchid              );
      DEFINE_COLOR("PALEGOLDENROD",         clstd::Color::PaleGoldenrod       );
      DEFINE_COLOR("PALEGREEN",             clstd::Color::PaleGreen           );
      DEFINE_COLOR("PALETURQUOISE",         clstd::Color::PaleTurquoise       );
      DEFINE_COLOR("PALEVIOLETRED",         clstd::Color::PaleVioletRed       );
      DEFINE_COLOR("PAPAYAWHIP",            clstd::Color::Papayawhip          );
      DEFINE_COLOR("PEACHPUFF",             clstd::Color::PeachPuff           );
      DEFINE_COLOR("PERU",                  clstd::Color::Peru                );
      DEFINE_COLOR("PINK",                  clstd::Color::Pink                );
      DEFINE_COLOR("PLUM",                  clstd::Color::Plum                );
      DEFINE_COLOR("POWDERBLUE",            clstd::Color::PowderBlue          );
      DEFINE_COLOR("PURPLE",                clstd::Color::Purple              );
      DEFINE_COLOR("RED",                   clstd::Color::Red                 );
      DEFINE_COLOR("ROSYBROWN",             clstd::Color::RosyBrown           );
      DEFINE_COLOR("ROYALBLUE",             clstd::Color::RoyalBlue           );
      DEFINE_COLOR("SADDLEBROWN",           clstd::Color::SaddleBrown         );
      DEFINE_COLOR("SALMON",                clstd::Color::Salmon              );
      DEFINE_COLOR("SANDYBROWN",            clstd::Color::SandyBrown          );
      DEFINE_COLOR("SEAGREEN",              clstd::Color::SeaGreen            );
      DEFINE_COLOR("SEASHELL",              clstd::Color::SeaShell            );
      DEFINE_COLOR("SIENNA",                clstd::Color::Sienna              );
      DEFINE_COLOR("SILVER",                clstd::Color::Silver              );
      DEFINE_COLOR("SKYBLUE",               clstd::Color::SkyBlue             );
      DEFINE_COLOR("SLATEBLUE",             clstd::Color::SlateBlue           );
      DEFINE_COLOR("SLATEGRAY",             clstd::Color::SlateGray           );
      DEFINE_COLOR("SLATEGREY",             clstd::Color::SlateGrey           );
      DEFINE_COLOR("SNOW",                  clstd::Color::Snow                );
      DEFINE_COLOR("SPRINGGREEN",           clstd::Color::SpringGreen         );
      DEFINE_COLOR("STEELBLUE",             clstd::Color::SteelBlue           );
      DEFINE_COLOR("TAN",                   clstd::Color::Tan                 );
      DEFINE_COLOR("TEAL",                  clstd::Color::Teal                );
      DEFINE_COLOR("THISTLE",               clstd::Color::Thistle             );
      DEFINE_COLOR("TOMATO",                clstd::Color::Tomato              );
      DEFINE_COLOR("TURQUOISE",             clstd::Color::Turquoise           );
      DEFINE_COLOR("VIOLET",                clstd::Color::Violet              );
      DEFINE_COLOR("WHEAT",                 clstd::Color::Wheat               );
      DEFINE_COLOR("WHITE",                 clstd::Color::White               );
      DEFINE_COLOR("WHITESMOKE",            clstd::Color::WhiteSmoke          );
      DEFINE_COLOR("YELLOW",                clstd::Color::Yellow              );
  }
                                                                  
  GXVOID FinalizeDefinationTable()
  {
    s_CombMarkDict.clear();
    s_ColorMarkDict.clear();
  }

  GXDWORD ParseCombinedFlags(const clStringW& str, LPCWSTR szPrefix, CmbMarkCls eClass)
  {
    clStringArrayW aString;
    GXDWORD dwFlags = NULL;
    ResolveString(str, L"|", aString);

    if(s_CombMarkDict.size() == 0) {
      InitializeDefinationTable();
    }

    for(clStringArrayW::iterator it = aString.begin();
      it != aString.end(); ++it)
    {
      CombMarkDict::iterator itMark = s_CombMarkDict.find(clStringW(szPrefix) + *it);

      if(itMark == s_CombMarkDict.end()) {
        continue;
      }

      if(itMark->second.eClass != eClass) {
        continue;
      }

      dwFlags |= itMark->second.dwMask;
    }
    return dwFlags;
  }

  GXINT GetPixelSizeFromMarkW(const GXWCHAR* szString)
  {
    int nLen = GXSTRLEN(szString);
    int nValue = GXATOI(szString);
    if(szString[nLen - 2] == 'p' && szString[nLen - 1] == 'x') {
      return nValue;
    }
    return nValue;
  }

  GXCOLORREF GetColorFromMarkW(const GXWCHAR* szMark)
  {
    if(szMark[0] == '#')
    {
      return clstd::xtou((wch*)&szMark[1], 16);
    }

    clStringW strMark = szMark;
    strMark.MakeUpper();
    ColorMarkDict::iterator it = s_ColorMarkDict.find(strMark);
    if(it != s_ColorMarkDict.end())
    {
      return it->second;
    }
    return 0xff000000;
  }

  //////////////////////////////////////////////////////////////////////////


} // namespace DlgXM

#endif // #ifndef _DEV_DISABLE_UI_CODE