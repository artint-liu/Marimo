#ifndef _GRAP_X_WINE_LISTVIEW_H_
#define _GRAP_X_WINE_LISTVIEW_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class GAMEENGINE_API CWEListView : public CMODlgItem
{
public:
  enum SetImageListType
  {
    NORMAL = LVSIL_NORMAL,  // Image list with large icons
    SMALL = LVSIL_SMALL,  // Image list with small icons
    STATE = LVSIL_STATE,  // Image list with state images
  };
  void      DeleteAllItems  ();
  BOOL      GetItemText    (int nItem, int nSubItem, LPWSTR lpText, int nLength);
  GXINT      InsertColumn  (GXLPWSTR lpText, int fmt, int cx, int iSub);
  GXINT      InsertItem    (GXINT iItem, GXLPWSTR lpText);
  GXINT      InsertItem    (GXINT iItem, GXLPWSTR lpText, GXLPARAM lParam);
  GXINT      InsertItem    (GXINT iItem, GXLPWSTR lpText, int iImage, GXLPARAM lParam);
  GXHIMAGELIST  SetImageList  (GXHIMAGELIST hImageList, SetImageListType eType);
  GXBOOL      SetItemText    (GXINT iItem, int iSub, GXLPWSTR lpText);
  GXBOOL      SetItemImage  (GXINT iItem, int iSub, int nImage);
  GXINT      GetCount    ();

  static CWEListView* GetDlgItem(GXHWND hParent, GXINT idCtrl);
public:
  CWEListView(GXHWND hWnd);
  virtual ~CWEListView();
};
typedef CWEListView *LPWELISTVIEW, *PWELISTVIEW;

#endif // _GRAP_X_WINE_LISTVIEW_H_