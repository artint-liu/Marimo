#ifndef _GRAP_X_WINE_TREEVIEW_H_
#define _GRAP_X_WINE_TREEVIEW_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class CWETreeViewReceiver;
class GAMEENGINE_API CWETreeView : public CMODlgItem
{
  CWETreeViewReceiver* m_pReceiver;
public:
  enum Expand
  {
    COLLAPSE      = TVE_COLLAPSE,
    COLLAPSERESET = TVE_COLLAPSERESET,
    EXPAND        = TVE_EXPAND,
    TOGGLE        = TVE_TOGGLE,
  };
  enum Change
  {
    BYKEYBOARD = TVC_BYKEYBOARD,  // By a key stroke
    BYMOUSE    = TVC_BYMOUSE,    // By a mouse click
    UNKNOWN    = TVC_UNKNOWN,    // Unknown
  };
  enum GetNext
  {
    ROOT            = GXTVGN_ROOT,               // 0x0000
    NEXT            = GXTVGN_NEXT,               // 0x0001
    PREVIOUS        = GXTVGN_PREVIOUS,           // 0x0002
    PARENT          = GXTVGN_PARENT,             // 0x0003
    CHILD           = GXTVGN_CHILD,              // 0x0004
    FIRSTVISIBLE    = GXTVGN_FIRSTVISIBLE,       // 0x0005
    NEXTVISIBLE     = GXTVGN_NEXTVISIBLE,        // 0x0006
    PREVIOUSVISIBLE = GXTVGN_PREVIOUSVISIBLE,    // 0x0007
    DROPHILITE      = GXTVGN_DROPHILITE,         // 0x0008
    CARET           = GXTVGN_CARET,              // 0x0009
    LASTVISIBLE     = GXTVGN_LASTVISIBLE,        // 0x000A
    NEXTSELECTED    = GXTVGN_NEXTSELECTED,       // 0x000B
  };

  
  GXHTREEITEM  InsertItem  (GXHTREEITEM hParent, GXHTREEITEM hInsertAfter, GXLPCWSTR lpText, GXLPARAM lParam = NULL);
  GXHTREEITEM  InsertItem  (GXHTREEITEM hParent, GXHTREEITEM hInsertAfter, int iImage, int iSelectImage, GXLPCWSTR lpText, GXLPARAM lParam);
  GXHTREEITEM  GetParent  (GXHTREEITEM hItem);
  GXHTREEITEM  GetChild  (GXHTREEITEM hItem);
  GXHTREEITEM  GetSibling  (GXHTREEITEM hItem);
  GXHTREEITEM  GetNextItem  (GXHTREEITEM hItem, CWETreeView::GetNext eNext);
  BOOL    GetItemText  (GXHTREEITEM hItem, WCHAR* buffer, int nLength);
  BOOL    GetItemParam(GXHTREEITEM hItem, GXLPARAM* plParam);
  BOOL    SetItemParam(GXHTREEITEM hItem, GXLPARAM lParam);
#ifdef REFACTOR_GXFC
#else
  virtual CGXReceiver*  GetReceiver   ();

  virtual void    SetReceiver  (CGXReceiver* pReceiver);
  virtual GXLRESULT InvokeReceiver(NMHDR* pnmhdr);
  virtual GXLRESULT Invoke(CGXReceiver* pReceiver, NMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

public:
  CWETreeView(GXHWND hWnd);
  virtual ~CWETreeView();
};

class CWETreeViewReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify  (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  virtual void    DeleteItem    (CWETreeView* pTreeView);  // TODO: 没实现
  virtual void    GetDispInfo   (CWETreeView* pTreeView);  // TODO: 没实现
  virtual void    ItemExpanded  (CWETreeView* pTreeView, CWETreeView::Expand eAction, GXTV_ITEM* tvi);
  virtual GXBOOL  ItemExpanding (CWETreeView* pTreeView, CWETreeView::Expand eAction, GXTV_ITEM* tvi);
  virtual void    KeyDown       (CWETreeView* pTreeView);  // TODO: 没实现
  virtual void    SelChanged    (CWETreeView* pTreeView, CWETreeView::Change eAction, GXTV_ITEM* tviPrev, GXTV_ITEM* tviNew);
  virtual GXBOOL  SelChanging   (CWETreeView* pTreeView, CWETreeView::Change eAction, GXTV_ITEM* tviPrev, GXTV_ITEM* tviNew);
  virtual void    SetDispInfo   (CWETreeView* pTreeView);  // TODO: 没实现
};

typedef CWETreeView *LPWETREEVIEW, *PWETREEVIEW;

#endif // _GRAP_X_WINE_LISTVIEW_H_
