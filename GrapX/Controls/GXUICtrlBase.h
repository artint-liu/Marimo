#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GXUI_CONTROL_BASE_H_
#define _GXUI_CONTROL_BASE_H_

//#define GXUICLASSNAME_STATIC  _T("GXUIStatic")
//#define GXUICLASSNAME_BUTTON  _T("GXUIButton")
//#define GXUICLASSNAME_LIST    _T("GXUIList")
//#define GXUICLASSNAME_TOOLBAR _T("GXUIToolbar")

class GXFont;
class GXWndCanvas;
namespace Marimo
{
  struct KNOCKACTION;
}

namespace GXUI
{
  extern GXLPCTSTR c_szGXUIClassNameStatic;
  extern GXLPCTSTR c_szGXUIClassNameButton;
  extern GXLPCTSTR c_szGXUIClassNameList;
  extern GXLPCTSTR c_szGXUIClassNameToolbar;
  //template<GXLPCTSTR* _TClsName, GXWNDPROC _TWndProc>
  class CtrlBase
  {
  protected:
    //typedef Marimo::KNOCKACTION KNOCKACTION;
    //typedef Marimo::DATAPOOL_IMPULSE DATAPOOL_IMPULSE;
    typedef Marimo::LPCDATAIMPULSE LPCDATAIMPULSE;
  protected:
    GXLONG    m_nLock;  // 引用锁， 用来防止在消息处理中用户销毁控件导致内存被释放而引发崩溃
    GXHWND    m_hWnd;   // Self
    GXHWND    m_hNotifyWnd;
    GXFont*   m_pFont;
    clStringW m_strIdentityName;
    //static LPCTSTR s_szClassName;
  protected:
    static  GXDWORD   RegisterClass     (GXHINSTANCE hInst, GXWNDPROC lpWndProc, GXLPCTSTR szClassName);
    static  GXDWORD   TryRegisterClass  (GXHINSTANCE hInst, GXWNDPROC lpWndProc, GXLPCTSTR szClassName);
    static  GXLRESULT DefWndProc  (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam, CtrlBase* pThis);
    static  CtrlBase* PreClassObj(GXHWND hWnd, GXUINT message, GXLPARAM lParam);

    GXLRESULT OnNCCreate(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);

  public:
    static  GXBOOL    SetDataVariable(GXHWND hWnd, MOVariable& var, MOVariable* pNewVar);
    //{
    //  GXWNDCLASSEX wcex;

    //  wcex.cbSize = sizeof(GXWNDCLASSEX);

    //  wcex.style          = CS_HREDRAW | CS_VREDRAW;
    //  wcex.lpfnWndProc    = lpWndProc;
    //  wcex.cbClsExtra      = 0;
    //  wcex.cbWndExtra      = sizeof(CtrlBase*);
    //  wcex.hInstance      = hInst;
    //  wcex.hIcon          = NULL;
    //  wcex.hCursor        = gxLoadCursor(NULL, IDC_ARROW);
    //  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //  wcex.lpszMenuName   = NULL;
    //  wcex.lpszClassName  = szClassName;
    //  wcex.hIconSm        = NULL;

    //  return gxRegisterClassExW(&wcex);
    //}
    //GXBOOL CtrlBase::Create(
    //  GXDWORD       dwExStyle, 
    //  GXLPCWSTR     lpWindowName, 
    //  GXDWORD       dwStyle, 
    //  const GXRegn* pRegn, 
    //  HGXWND        hWndParent, 
    //  HGXMENU       hMenu, 
    //  GXHINSTANCE   hInstance, 
    //  GXLPVOID      lpParam)
    //{
    //  GXWNDCLASSEX wcex;
    //  if(gxGetClassInfoEx(hInstance, _TClsName, &wcex) == FALSE) {
    //    RegisterClass(hInstance);
    //  }

    //  m_hWnd = gxCreateWindowEx(dwExStyle, _TClsName, lpWindowName, dwStyle, pRegn->left, pRegn->top, 
    //    pRegn->width, pRegn->height, hWndParent, NULL, hInstance, this);

    //  m_pFont = GXUIGetStock()->pDefaultFont;
    //  m_pFont->AddRef();

    //  return TRUE;
    //}
    //static  GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    virtual GXLRESULT           OnPaint         (GXWndCanvas& canvas) = NULL;
    virtual GXLRESULT           Destroy         ();
    virtual GXLRESULT           SetVariable     (MOVariable* pVariable) { return TRUE; };
    //virtual GXHRESULT           OnKnock         (KNOCKACTION* pKnock) { return 0; };
    virtual GXVOID              OnImpulse       (LPCDATAIMPULSE pImpulse) {};
    virtual GXBOOL              SolveDefinition (const GXDefinitionArrayW& aDefinitions);
    //virtual GXLRESULT CtrlBase::Destroy()
    //{
    //  SAFE_RELEASE(m_pFont);
    //  return 0;
    //}
    GXLPCWSTR IntGetText          () const;
    GXBOOL    Invalidate          (GXBOOL bErase);
    GXBOOL    InvalidateRect      (GXLPCRECT lpRect, GXBOOL bErase);
    GXBOOL    NotifyParent        (GXDWORD wNotiMsg);
    GXBOOL    SetDataPoolVariableA(const clStringA& strExpression);
    GXBOOL    SetDataPoolVariableW(const clStringW& strExpression);
    //void      SetName       (GXLPCWSTR lpName);
  public:
    CtrlBase(GXLPCWSTR szIdName);
    virtual ~CtrlBase();
  public:
    virtual GXLRESULT Measure(GXRegn* pRegn) = NULL;
    GXHRESULT SetFont       (GXLPCWSTR szFontName, int nFontSize);
    GXLPCWSTR GetName       ();
    //GXHRESULT SetFont(LPCWSTR szFontName, int nFontSize)
    //{
    //  SAFE_RELEASE(m_pFont);
    //  GXGraphics* pGraphics = gxGetGraphics(m_hWnd);
    //  if(pGraphics != NULL) {
    //    m_pFont = pGraphics->CreateFontW(0, nFontSize, szFontName);
    //  }
    //  return GX_OK;
    //}

    inline GXHWND Get()
    {
      return m_hWnd;
    }

    inline GXLONG Lock()
    {
      return ++m_nLock;
    }

    inline GXLONG Unlock()
    {
      if(--m_nLock == 0) {
        Destroy();
        delete this;
        return 0;
      }
      return m_nLock;
    }
  };

  
  //////////////////////////////////////////////////////////////////////////

  //GXDWORD CtrlBase::RegisterClass(GXHINSTANCE hInst)
  //{
  //  GXWNDCLASSEX wcex;

  //  wcex.cbSize = sizeof(WNDCLASSEX);

  //  wcex.style          = CS_HREDRAW | CS_VREDRAW;
  //  wcex.lpfnWndProc    = (GXWNDPROC)CtrlBase::WndProc;
  //  wcex.cbClsExtra      = 0;
  //  wcex.cbWndExtra      = sizeof(CtrlBase*);
  //  wcex.hInstance      = hInst;
  //  wcex.hIcon          = NULL;
  //  wcex.hCursor        = gxLoadCursor(NULL, IDC_ARROW);
  //  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  //  wcex.lpszMenuName   = NULL;
  //  wcex.lpszClassName  = GXUICLASSNAME_STATIC;
  //  wcex.hIconSm        = NULL;

  //  return gxRegisterClassExW(&wcex);
  //}


  //////////////////////////////////////////////////////////////////////////
  //
  // inline function
  //
  //inline GXHWND CtrlBase::Get()
  //{
  //  return m_hWnd;
  //}
} // namespace GXUI


namespace DlgXM
{
  enum CmbMarkCls  // 可合并标记的分组
  {
    CMC_WndStyle,
    CMC_WndExStyle,
    CMC_DlgStyle,
    CMC_StaticLabel,
    CMC_ButtonStyle,
    CMC_EditStyle,
    CMC_SliderStyle,
    CMC_StdListBox,
    CMC_LayoutPanel,
    CMC_MenuType,
    CMC_MenuState,
    CMC_ToolbarBtnStyle,
    CMC_ToolbarState,
    CMC_ToolbarStyle
  };
  struct DLGCOMBINEDMARK  // 可合并的标记
  {
    GXDWORD     dwMask;
    CmbMarkCls  eClass;
    //LPCWSTR     szPrefix;
  };

  struct DLGBASICPARAMW
  {
    GXRegn    regn;
    clStringW strCaption;
    clStringW strMenu;
    clStringW strName;    // 对象识别名
    clStringW strStyle;   // 未解析的dwStyle字符串,用于控件解析自己的属性
    clStringW strExStyle; // 未解析的dwExStyle字符串
    GXDWORD   dwExStyle;
    GXDWORD   dwStyle;
    DLGBASICPARAMW() : regn(0), dwExStyle(NULL), dwStyle(NULL) {}
    void Reset()
    {
      CLBREAK; // 检查这里为什么不完全清除???
      regn.Empty();
      dwStyle = NULL;
      dwExStyle = NULL;
      strCaption.Clear();
    }
  };

  struct DLGBTNSPRITE
  {
    clStringW strResource;
    clStringW strNormal;
    clStringW strHover;
    clStringW strPressed;
    clStringW strDisable;
    clStringW strDefault;
  };

  struct DLGSLIDERSPRITE
  {
    clStringW strResource;
    clStringW strHandle;
    clStringW strEmpty;
    clStringW strFull;
    clStringW strDial;
    clStringW strVertEmpty;
    clStringW strVertFull;
    clStringW strVertDial;
  };

  struct DLGFONT
  {
    clStringW strResource;
    size_t uSize;
  };

  struct DLGFONTPARAMW
  {
    clStringW strFontName;
    GXINT   nFontSize;
    DLGFONTPARAMW() : nFontSize(0) {}
    void Reset()
    {
      strFontName.Clear();
      nFontSize = 0;
    }
    void Inherit(const DLGFONTPARAMW* lpBase)
    {
      if(strFontName.IsEmpty()) {
        strFontName = lpBase->strFontName;
      }
      if(nFontSize == 0) {
        nFontSize = lpBase->nFontSize;
      }
    }
    GXBOOL IsAvailable()
    {
      return (strFontName.IsNotEmpty() && nFontSize > 0);
    }
  };
    
  typedef clhash_map<clStringW, DLGCOMBINEDMARK> CombMarkDict;
  typedef clmap<clStringW, GXCOLORREF>           ColorMarkDict;
  static CombMarkDict   s_CombMarkDict;
  static ColorMarkDict  s_ColorMarkDict;
//#define APPENDMARK(_MASK, _CLS)           \
//  DlgCombMark.dwMask = _MASK;             \
//  DlgCombMark.eClass = _CLS;              \
//  s_CombMarkDict[#_MASK] = DlgCombMark;

  GXVOID InitializeDefinationTable();
  GXVOID FinalizeDefinationTable();

  GXDWORD     ParseCombinedFlags    (const clStringW& str, GXLPCWSTR szPrefix, CmbMarkCls eClass);
  GXINT       GetPixelSizeFromMarkW (const GXWCHAR* szString);
  GXCOLORREF  GetColorFromMarkW     (const GXWCHAR* szMark);

} // namespace DlgXM


#endif // _GXUI_CONTROL_BASE_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE