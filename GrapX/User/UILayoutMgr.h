#ifndef _UI_DLG_LAYOUT_H_
#define _UI_DLG_LAYOUT_H_

class CMOWnd;

namespace GXUI
{
  //namespace DlgLayout
  //{
  //  struct BINARYITEM
  //  {
  //    wstring strName;
  //  };
  //  typedef clvector<BINARYITEM> BinaryItemArray;

  //  struct BINARY
  //  {
  //    wstring strName;
  //    BinaryItemArray aItems;
  //  };
  //  typedef clvector<BINARY> BinaryArray;
  //} // namespace DlgLayout

// Layout Panel Style
#define LPS_WNDITEM       0x00000000  // Default
#define LPS_HBINARYPANEL  0x00000001  // 水平分割条的版式, 切分为上下两部分
#define LPS_VBINARYPANEL  0x00000002  // 竖直分割条的版式, 切分为左右两部分
#define LPS_CROSSPANEL    0x00000003  // 十字分割
#define LPS_FREEPANEL     0x00000004  // 暂时不支持
#define LPS_TYPESETPANEL  0x00000005  // 暂时不支持
#define LPS_LINEWRAP      0x00000006  // 排版换行模式,此模式下与上面的标志不能同时使用且优先级更高
#define LPS_TYPEMASK      0x0000000f  // 掩码
#define LPS_FIXED         0x00000100  // 用户不能调整
#define LPS_KEEPFIRST     0x00000000  // 保持第一个的尺寸[Default]
#define LPS_KEEPSECOND    0x00001000  // 保持第二个尺寸
#define LPS_KEEPTHIRD     0x00002000
#define LPS_KEEPFOURTH    0x00003000
#define LPS_KEEPWEIGHT    0x00004000  // 保持权重的尺寸调整, 十字Panel的水平和竖直方向都保持原来的权重调整
#define LPS_KEEPMASK      0x0000f000  // 掩码
#define LPS_INT_RETURN    0x01000000  // 内部标志, LPS_LINEWRAP布局下某个Wnd之后需要换行的标志.
  //enum PanelType
  //{
  //  PT_Unknown,
  //  PT_Item,
  //  PT_HorzBianry,
  //  PT_VertBianry,
  //};
  //
  //enum PanelAlignType
  //{
  //  PAT_FixedFirst,   // 用户不能改变大小, Size窗口时保持前面控件的大小
  //  PAT_FixedSecond,
  //  PAT_AlignFirst,   // Size窗口时保持前面控件的大小
  //  PAT_AlignSecond,
  //};

  struct DLGPANEL
  {
    clStringW           strName;
    clvector<DLGPANEL>  aPanels;
    GXDWORD             dwStyle;
    float               fScale[2];  // 分割值,用于2次和十字分割
  };
  typedef clvector<DLGPANEL> DlgPanelArray;

  //struct DLGLAYOUT
  //{
  //  DlgLayout::BinaryArray aBinary;
  //};
  //////////////////////////////////////////////////////////////////////////
  class PanelBase
  {
  public:
    enum ItemType
    {
      IT_Empty,
      IT_Item,
      IT_Panel,
    };

    struct HITTESTRESULT
    {
      GXHCURSOR   hCursor;
      PanelBase*  pPanel;
      GXPOINT     ptParam;  // 不同的Panel含义不同的参数,主要是鼠标位置相关的参数
    };
    //enum PanelMode
    //{
    //  PM_None,
    //  PM_Horz,
    //  PM_Vert,
    //  PM_Quarter,
    //  PM_Mask = 0x00000003,
    //};
    //enum PanelType
    //{
    //  PT_Empty,
    //  PT_Control,
    //  PT_Node,
    //  PT_MaskBase = 0x00000003,
    //};

  private:
    //CMOWnd* m_pWnd;
  public:
    PanelBase()
      //: m_pWnd(NULL)
    {
    }
    virtual ~PanelBase()
    {

    }
    virtual GXLRESULT Initialize  (GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel) = NULL;
    //virtual PanelBase* SetPanelItem (int nPart, PanelBase::PanelMode eMode) = NULL;
    //virtual void      SetControlItem  (int nPart, GXHWND hWnd) = NULL;
    virtual GXBOOL    HitTest     (GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult) = NULL;
    virtual GXLRESULT OnSize      (GXRECT* RECT) = NULL;
    virtual void      OnDrag      (GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam) = NULL;
  };
  //////////////////////////////////////////////////////////////////////////

  class BinaryPanel : public PanelBase
  {
  public:
    //enum ItemType
    //{
    //  IT_Empty,
    //  IT_Item,
    //  IT_Panel,
    //};
  protected:
    //typedef PanelBase::PanelMode PanelMode;
    //typedef PanelBase::PanelType PanelType;
    union{
      PanelBase*  m_pChild[2];
      GXHWND      m_hWnd[2];
    };
    GXRECT m_rect;
    GXLONG m_uPanel;
    //GXDWORD m_dwFlags;
    GXDWORD m_dwStyle;
    GXDWORD m_dwItemPanel0 : 8;
    GXDWORD m_dwItemPanel1 : 8;
    //GXDWORD m_dwPanelType  : 8;
    //GXDWORD m_dwAlignType  : 8;
    const static int c_nRange = 3; // 宽度
    //GXINT m_nPartOffset;  // 鼠标按下时与分割线的偏移
  protected:
    inline GXDWORD GetPanelType() const
    {
      return m_dwStyle & LPS_TYPEMASK;
    }
    inline GXBOOL IsFixed() const
    {
      return m_dwStyle & LPS_FIXED;
    }
    inline GXDWORD GetKeepIndex() const
    {
      return m_dwStyle & LPS_KEEPMASK;
    }

    inline void SetItemType(int nPart, ItemType eType)
    {
      ASSERT((nPart == 0 || nPart == 1) && 
        GetItemType(nPart) == IT_Empty);
      if(nPart == 0) {
        m_dwItemPanel0 = eType;
      }
      else if(nPart == 1) {
        m_dwItemPanel1 = eType;
      }
    }

    inline ItemType GetItemType(int nPart) const
    {
      ASSERT(nPart == 0 || nPart == 1);
      if(nPart == 0) {
        return (ItemType)m_dwItemPanel0;
      }
      else if(nPart == 1) {
        return (ItemType)m_dwItemPanel1;
      }
      return IT_Empty;
    }

    inline void CalcPanelFactor(GXLPCRECT lpRect, const float* fPartition)
    {
      ASSERT(fPartition[0] >= 0.0f && fPartition[0] <= 1.0f);
      if(GetPanelType() == LPS_VBINARYPANEL) {
        m_uPanel = clLerp(lpRect->left, lpRect->right, fPartition[0]);
      }
      else if(GetPanelType() == LPS_HBINARYPANEL) {
        m_uPanel = clLerp(lpRect->top, lpRect->bottom, fPartition[0]);
      }
    }

    inline void CalcRect(const GXRECT* rcParent, int nPart, GXRECT* rcPart) const
    {
      *rcPart = *rcParent;
      if(GetPanelType() == LPS_VBINARYPANEL)
      {
        if(nPart == 0) {
          rcPart->right = m_uPanel - c_nRange;
        }
        else if(nPart == 1) {
          rcPart->left = m_uPanel + c_nRange;
        }
        //ASSERT(rcPart->left <= rcPart->right);
      }
      else if(GetPanelType() == LPS_HBINARYPANEL)
      {
        if(nPart == 0) {
          rcPart->bottom = m_uPanel - c_nRange;
        }
        else if(nPart == 1) {
          rcPart->top = m_uPanel + c_nRange;
        }
        //ASSERT(rcPart->top <= rcPart->bottom);
      }
      //m_rect = *rcParent; // TODO: calc时不应该修改内部成员的值
    }
  public:
    BinaryPanel();
    virtual ~BinaryPanel();
    virtual GXLRESULT Initialize  (GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel);
    //virtual PanelBase* SetPanelItem(int nPart, PanelBase::PanelMode eMode);
    //virtual void      SetControlItem(int nPart, GXHWND hWnd);
    virtual void      OnDrag      (GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam);
    virtual GXBOOL    HitTest     (GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult);
    virtual GXLRESULT OnSize      (GXRECT* rect);
  };

  class BinaryPanel_KeepWeight : public BinaryPanel
  {
  private:
    float m_fWeight;
  public:
    BinaryPanel_KeepWeight();
    virtual ~BinaryPanel_KeepWeight();
    virtual GXLRESULT Initialize  (GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel);
    virtual void      OnDrag      (GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam);
    virtual GXLRESULT OnSize      (GXRECT* rect);
  };

  class CrossPanel : public PanelBase
  {
  public:
    //enum ItemType
    //{
    //  IT_Empty,
    //  IT_Item,
    //  IT_Panel,
    //};
  private:
    union{
      PanelBase*  m_pChild[4];
      GXHWND      m_hWnd[4];
    };
    GXRECT m_rect;
    GXLONG m_uPanel[2];
    GXDWORD m_dwStyle;
    GXDWORD m_dwItemPanel0 : 4;
    GXDWORD m_dwItemPanel1 : 4;
    GXDWORD m_dwItemPanel2 : 4;
    GXDWORD m_dwItemPanel3 : 4;
    const static int c_nRange = 3; // 宽度
  private:
    inline GXDWORD GetPanelType()
    {
      return m_dwStyle & LPS_TYPEMASK;
    }
    inline GXBOOL IsFixed()
    {
      return m_dwStyle & LPS_FIXED;
    }
    inline GXDWORD GetKeepIndex()
    {
      return m_dwStyle & LPS_KEEPMASK;
    }
    inline void SetItemType(int nPart, ItemType eType)
    {
      ASSERT((nPart >= 0 && nPart <= 3) && 
        GetItemType(nPart) == IT_Empty);
      if(nPart == 0) {
        m_dwItemPanel0 = eType;
      }
      else if(nPart == 1) {
        m_dwItemPanel1 = eType;
      }
      else if(nPart == 2) {
        m_dwItemPanel2 = eType;
      }
      else if(nPart == 3) {
        m_dwItemPanel3 = eType;
      }
    }

    inline ItemType GetItemType(int nPart)
    {
      ASSERT(nPart >= 0 && nPart <= 3);
      if(nPart == 0) {
        return (ItemType)m_dwItemPanel0;
      }
      else if(nPart == 1) {
        return (ItemType)m_dwItemPanel1;
      }
      else if(nPart == 2) {
        return (ItemType)m_dwItemPanel2;
      }
      else if(nPart == 3) {
        return (ItemType)m_dwItemPanel3;
      }
      return IT_Empty;
    }
    
    inline void CalcPanelFactor(GXLPCRECT lpRect, const float* fPartition)
    {
      ASSERT(fPartition[0] >= 0.0f && fPartition[0] <= 1.0f);
      ASSERT(fPartition[1] >= 0.0f && fPartition[1] <= 1.0f);
      m_uPanel[0] = clLerp(lpRect->left, lpRect->right, fPartition[0]);
      m_uPanel[1] = clLerp(lpRect->top, lpRect->bottom, fPartition[1]);
    }

    inline void CalcRect(const GXRECT* rcParent, int nPart, GXRECT* rcPart)
    {
      *rcPart = *rcParent;
      //if(GetPanelType() == LPS_VBINARYPANEL)
      //{

        if(nPart == 0) {
          rcPart->right  = m_uPanel[0] - c_nRange;
          rcPart->bottom = m_uPanel[1] - c_nRange;
        }
        else if(nPart == 1) {
          rcPart->left   = m_uPanel[0] + c_nRange;
          rcPart->bottom = m_uPanel[1] - c_nRange;
        }
        else if(nPart == 2) {
          rcPart->right = m_uPanel[0] - c_nRange;
          rcPart->top   = m_uPanel[1] + c_nRange;
        }
        else if(nPart == 3) {
          rcPart->left = m_uPanel[0] + c_nRange;
          rcPart->top  = m_uPanel[1] + c_nRange;
        }
      //}
      //else if(GetPanelType() == LPS_HBINARYPANEL)
      //{
      //  //m_uPanel = (rcPart->top + rcPart->bottom) / 2;
      //  //if(m_dwAlignType == PAT_FixedSecond || 
      //  //  m_dwAlignType == PAT_AlignSecond )
      //  //{
      //  //  m_uPanel = m_uPanel + rcParent->bottom - m_rect.bottom;
      //  //}

      //  if(nPart == 0)
      //    rcPart->bottom = m_uPanel - c_nRange;
      //  else if(nPart == 1)
      //    rcPart->top = m_uPanel + c_nRange;
      //}
      m_rect = *rcParent;
    }
  public:
    CrossPanel();
    virtual ~CrossPanel();
    virtual GXLRESULT Initialize  (GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel);
    //virtual PanelBase* SetPanelItem(int nPart, PanelBase::PanelMode eMode);
    //virtual void      SetControlItem(int nPart, GXHWND hWnd);
    virtual void      OnDrag      (GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam);
    virtual GXBOOL    HitTest     (GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult);
    virtual GXLRESULT OnSize      (GXRECT* rect);
  };

  class LineWrapPanel : public PanelBase
  {
  private:
    struct WNDITEM
    {
      GXHWND hWnd;
      GXDWORD dwStyle;
    };
    typedef clvector<WNDITEM> WndItemArray;
    WndItemArray m_aItems;
    const static int c_nGap = 3; // 间距
  public:
    LineWrapPanel();
    virtual ~LineWrapPanel();
    virtual GXLRESULT Initialize  (GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel);
    virtual GXBOOL    HitTest     (GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult);
    virtual void      OnDrag      (GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam);
    virtual GXLRESULT OnSize      (GXRECT* rect);
  };
  //////////////////////////////////////////////////////////////////////////
  class Layout
  {
  private:
    GXHWND      m_hWndOwner;
    PanelBase*  m_pRoot;
    GXWPARAM    m_fwPrevKeys;
    GXHCURSOR   m_hCursor;
    PanelBase*  m_pFocusPanel;
    //GXINT       m_nPartOffset;
    GXPOINT     m_ptParam;
    //static UINT s_uRefCount;
  public:

    Layout(GXHWND hWndOwner);
    virtual ~Layout();
    //static GXLRESULT GXCALLBACK WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    //static GXLRESULT RegisterClass(GXHINSTANCE hInst);
    //static GXLRESULT UnregisterClass();
    GXLRESULT Initialize  (const DLGPANEL* pPanel);
    GXLRESULT Finalize    ();

    PanelBase* GetRoot();
    virtual GXLRESULT OnSize(GXDWORD fwSizeType, GXSIZE& size);
    virtual GXBOOL    OnSetCursor(int nHittest);
    virtual GXLRESULT OnLButtonDown(GXWPARAM fwKeys, GXPOINT* ptMouse);
    virtual GXLRESULT OnLButtonUp(GXWPARAM fwKeys, GXPOINT* ptMouse);
    //virtual GXBOOL    OnSetCursor(GXHWND hWnd, int nHittest, GXWORD wMouseMsg);
    virtual GXVOID    OnMouseMove(GXWPARAM fwKeys, GXPOINT* ptMouse);
    virtual GXBOOL    OnHitTest(const GXPOINT* ptCursor);   // 随后要调用 gxDefWindowProc
    static PanelBase* CreatePanel(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel);
  };
} // namespace GXUI

#endif // _UI_DLG_LAYOUT_H_