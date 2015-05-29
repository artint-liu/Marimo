#ifndef _GXUI_BUTTON_H_
#define _GXUI_BUTTON_H_

class GXSprite;

#define GXUIBUTTON_STATE_PRESSED  0x00000001
#define GXUIBUTTON_STATE_HOVER    0x00000002

//#define GXUIBS_LEFTTEXT     GXBS_LEFTTEXT    //     0x00000020L
#define GXUIBS_TEXT         GXBS_TEXT        //     0x00000000L
//#define GXUIBS_ICON         GXBS_ICON        //     0x00000040L
//#define GXUIBS_BITMAP       GXBS_BITMAP      //     0x00000080L
#define GXUIBS_LEFT         GXBS_LEFT        //     0x00000100L
#define GXUIBS_RIGHT        GXBS_RIGHT       //     0x00000200L
#define GXUIBS_CENTER       GXBS_CENTER      //     0x00000300L
#define GXUIBS_TOP          GXBS_TOP         //     0x00000400L
#define GXUIBS_BOTTOM       GXBS_BOTTOM      //     0x00000800L
#define GXUIBS_VCENTER      GXBS_VCENTER     //     0x00000C00L
//#define GXUIBS_PUSHLIKE     GXBS_PUSHLIKE    //     0x00001000L
#define GXUIBS_MULTILINE    GXBS_MULTILINE   //     0x00002000L
//#define GXUIBS_NOTIFY       GXBS_NOTIFY      //     0x00004000L
//#define GXUIBS_FLAT         GXBS_FLAT        //     0x00008000L
//#define GXUIBS_RIGHTBUTTON  GXBS_RIGHTBUTTON //     GXBS_LEFTTEXT


namespace GXUI
{
  //GXLRESULT GXCALLBACK ButtonWndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  class Button : public CtrlBase
  {
  public:

  protected:
    GXSprite* m_pSprite;
    int       m_nNormal;
    int       m_nHover;
    int       m_nPressed;
    int       m_nDisabled;
    int       m_nDefault;
    GXDWORD   m_dwState;
    GXColor32 m_crText;
    GXColor32 m_crDisabledText;
  protected:
    static  GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    //static  GXDWORD              RegisterClass  (GXHINSTANCE hInst);
    virtual GXLRESULT   OnPaint        (GXWndCanvas& canvas);
    virtual GXLRESULT   Destroy        ();
    virtual GXLRESULT   SetVariable    (MOVariable* pVariable);
    virtual GXVOID      OnImpulse      (LPCDATAIMPULSE pImpulse);

    GXDWORD GetDrawTextFlag ();
    void    CommandNotify   ();
  public:
    Button(GXLPCWSTR szIdName);
    static Button* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static Button* Create  (GXHINSTANCE hInst, GXHWND hParent, GXLPCTSTR szText, GXDWORD dwStyle, GXLPCWSTR szIdName, const GXRegn* pRegn, const GXDefinitionArrayW* pDefinitions);
    virtual GXLRESULT Measure(GXRegn* pRegn);
  public:
    //GXHRESULT      SetFont (LPCWSTR szFontName, int nFontSize);
    GXBOOL SetSprite(const DlgXM::DLGBTNSPRITE& Desc);
    GXBOOL SetSprite(GXLPCWSTR szSpriteFile, GXLPCWSTR szNormal, GXLPCWSTR szHover, GXLPCWSTR szPressed, GXLPCWSTR szDisabled, GXLPCWSTR szDefault);

    //inline GXHWND Get();
  };
  //////////////////////////////////////////////////////////////////////////
 
  //////////////////////////////////////////////////////////////////////////
  //
  // inline function
  //
}


#endif // _GXUI_BUTTON_H_