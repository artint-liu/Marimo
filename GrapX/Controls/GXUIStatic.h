#ifndef _GXUI_STATIC_H_
#define _GXUI_STATIC_H_

//#define GXUISS_DRAGNOTIFY   0x00002000

class GXSprite;
namespace GXUI
{
  //extern GXLPCTSTR c_szGXUIClassNameStatic;
  //GXLRESULT GXCALLBACK StaticWndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  class Static : public CtrlBase
  {
    typedef Marimo::KNOCKACTION KNOCKACTION;
  public:
    enum Type
    {
      Type_Label     = GXUISS_TYPE_LABEL,
      Type_Sprite    = GXUISS_TYPE_SPRITE,
      Type_Rectangle = GXUISS_TYPE_RECT,
    };
  private:
    Type    m_eType;
  protected:
    static  GXLRESULT GXCALLBACK WndProc    (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    
    //static  GXDWORD              RegisterClass  (GXHINSTANCE hInst);
    virtual GXLRESULT           OnPaint     (GXWndCanvas& canvas) = NULL;
    virtual GXLRESULT           Destroy     ();
    virtual GXLRESULT           SetVariable (MOVariable* pVariable);
    virtual GXVOID              OnImpulse   (LPCDATAIMPULSE pImpulse);
    //virtual GXHRESULT           OnKnock     (KNOCKACTION* pKnock);
    //virtual GXBOOL              SolveDefinition (const GXDefinitionArray& aDefinitions);
  public:
    Static(GXLPCWSTR szIdName, Type eType);
    ~Static()
    {
      //__asm nop
    }
    static Static* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static Static* Create  (GXHINSTANCE hInst, GXHWND hParent, Type eType, GXLPCWSTR szTemplate, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
  public:
    inline Type   GetType  ();
    inline GXBOOL ShouldNotifyParent();
    //GXHRESULT      SetFont (LPCWSTR szFontName, int nFontSize);

    //inline GXHWND Get();
  };
  inline Static::Type Static::GetType()
  {
    return m_eType;
  }
  inline GXBOOL Static::ShouldNotifyParent()
  {
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    return TEST_FLAG(dwStyle, GXUISS_NOTIFY);
  }
  //////////////////////////////////////////////////////////////////////////
  class StaticLabel : public Static
  {
    friend class Static;
    typedef Marimo::KNOCKACTION KNOCKACTION;
  protected:
    GXCOLORREF  m_crText;
    MOVariable  m_VarText;
  protected:
    virtual GXLRESULT   OnPaint         (GXWndCanvas& canvas);
    GXDWORD             GetDrawTextFlag ();
    virtual GXLRESULT   SetVariable     (MOVariable* pVariable);
    virtual GXVOID      OnImpulse       (LPCDATAIMPULSE pImpulse);
    //virtual GXHRESULT   OnKnock         (KNOCKACTION* pKnock);
  public:
    StaticLabel(GXLPCWSTR szIdName);
  public:
    virtual GXLRESULT Measure   (GXRegn* pRegn);
    GXCOLORREF        SetColor  (GXCOLORREF crText);
    GXCOLORREF        GetColor  ();
  };

  class StaticRectangle : public Static
  {
  protected:
    GXCOLORREF  m_crMeta;
  protected:
    virtual GXLRESULT OnPaint       (GXWndCanvas& canvas);
  public:
    StaticRectangle(GXLPCWSTR szIdName);
  public:
    virtual GXLRESULT Measure(GXRegn* pRegn);
    GXCOLORREF SetColor(GXCOLORREF crText);
    GXCOLORREF GetColor();
  };

  class StaticSprite : public Static
  {
  protected:
    clStringW   m_strSpriteFile;
    clStringW   m_strSpriteName;
    GXSprite*   m_pSprite;
    //int         m_nSpriteIdx;
    GXINT       m_nSpriteIdx;
  protected:
    virtual GXLRESULT   OnPaint       (GXWndCanvas& canvas);
    virtual GXLRESULT   Destroy       ();
    virtual GXLRESULT   SetVariable   (MOVariable* pVariable);
    virtual GXVOID      OnImpulse     (LPCDATAIMPULSE pImpulse);

    void ClearSprite();
  public:
    StaticSprite(GXLPCWSTR szIdName);
  public:
    virtual GXLRESULT Measure(GXRegn* pRegn);
    GXBOOL SetSpriteByFilenameW   (GXLPCWSTR szSpriteFile);
    GXBOOL SetSprite              (GXSprite* pSprite);
    GXBOOL SetByNameW             (GXLPCWSTR szName);
    GXBOOL SetByIndex             (GXINT index);
    GXBOOL GetSpriteByFilenameW   (GXLPWSTR szSpriteFile, GXINT nMaxBuffer);
    GXBOOL GetSprite              (GXSprite** ppSprite);
    GXBOOL GetByNameW             (GXLPWSTR szName, GXINT nMaxBuffer);
    GXINT  GetByIndex             ();
  };

  //////////////////////////////////////////////////////////////////////////
  //
  // inline function
  //
}


#endif // _GXUI_STATIC_H_