#ifndef _GXUI_SLIDER_H_
#define _GXUI_SLIDER_H_

#define GXSLDN_BEGINDRAG      GXTBN_BEGINDRAG
#define GXSLDN_ENDDRAG        GXTBN_ENDDRAG
#define GXSLDN_BEGINTHUMB     GXTBN_BEGINADJUST
#define GXSLDN_ENDTHUMB       GXTBN_ENDADJUST

#define GXSLDM_SETPOS         

namespace DlgXM
{
  struct DLGSLIDERSPRITE;
} // namespace DlgXM

class GXSprite;
namespace GXUI
{
  class Slider : public CtrlBase
  {
  public:
    enum HTSlider // 这个和 IntCalcRects 返回结构相关,修改要慎重
    {
      HTS_HANDLE,
      HTS_EMPTY,
      HTS_FULL,
      HTS_COUNT,
    };
  private:
    GXSprite* m_pSprite;
    int       m_idHandle;
    int       m_idFullBar;
    int       m_idEmptyBar;
    int       m_idDial;
    int       m_idVertFullBar;
    int       m_idVertEmptyBar;
    int       m_idVertDial;

    union
    {
      struct
      {
        GXINT     Begin;    // 允许 Begin > End
        GXINT     End;
      }n;

      struct
      {
        GXFLOAT   Begin;    // 允许 Begin > End
        GXFLOAT   End;
      }f;
    }m_Prime;

    float       m_fMinorPercent;  // 第二进度条百分比
    int         m_nPos;           // 像素位置
    GXUINT      m_nLength;        // 像素尺寸,每次 IntCalcRects 更新
    MOVariable  m_VarPos;
  protected:
    static  GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    
    virtual GXLRESULT           OnPaint         (GXWndCanvas& canvas);
    virtual GXLRESULT           Measure         (GXRegn* pRegn);
    virtual GXLRESULT           Destroy         ();
    virtual GXLRESULT           SetVariable     (MOVariable* pVariable);
    //virtual GXHRESULT           OnKnock         (KNOCKACTION* pKnock);
    virtual GXVOID              OnImpulse       (LPCDATAIMPULSE pImpulse);
    virtual GXBOOL              SolveDefinition (const GXDefinitionArrayW& aDefinitions);

    void                        IntCalcRects    (GXRECT* rcClient, GXDWORD dwStyle, GXOUT GXREGN* pRegns); // rect 数量应该与 HTS_COUNT 一致
    HTSlider                    IntHitTest      (GXRECT* rcClient, GXPOINT* pos, GXOUT GXREGN* pRegns);

    GXLRESULT                   OnLButtonDown (GXLPPOINT pos);
    GXLRESULT                   OnLButtonUp   (GXLPPOINT pos);
    GXLRESULT                   SetRangeBegin (GXDWORD dwStyle, GXINT nValue);
    GXLRESULT                   SetRangeEnd   (GXDWORD dwStyle, GXINT nValue);
    GXLRESULT                   SetRangeBegin (GXDWORD dwStyle, GXFLOAT fValue);
    GXLRESULT                   SetRangeEnd   (GXDWORD dwStyle, GXFLOAT fValue);
    GXLRESULT                   SetPos        (GXDWORD dwStyle, GXINT nValue);
    GXLRESULT                   SetPos        (GXDWORD dwStyle, GXFLOAT fValue);

    void                        AlignPos      (GXDWORD dwStyle);
    void                        PaintDial     (GrapX::GXCanvas* pCanvas, GXDWORD dwStyle, int x, int y);
    inline  int                 GetDialLeft   (GXREGN* regn);
    inline  int                 GetDialTop    (GXREGN* regn);
    void                        UpdatePosFromVar(GXDWORD dwStyle);
    inline void                 UpdateVarFromPos(GXDWORD dwStyle);
  public:
    Slider(GXLPCWSTR szIdName);
    ~Slider()
    {
      //__asm nop
    }
    static Slider* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static Slider* Create  (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
  public:
    GXBOOL SetSprite(const DlgXM::DLGSLIDERSPRITE& Desc);
    inline GXBOOL ShouldNotifyParent();
  };

  inline GXBOOL Slider::ShouldNotifyParent()
  {
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    return TEST_FLAG(dwStyle, GXUISLDS_NOTIFY);
  }

  inline int Slider::GetDialLeft(GXREGN* regn)
  {
    return clMin(regn[HTS_EMPTY].left, regn[HTS_FULL].left);
  }

  inline int Slider::GetDialTop(GXREGN* regn)
  {
    return clMin(regn[HTS_EMPTY].top, regn[HTS_FULL].top);
  }


  //////////////////////////////////////////////////////////////////////////
  //
  // inline function
  //
}


#endif // #ifndef _GXUI_SLIDER_H_