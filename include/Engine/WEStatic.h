#ifndef _GRAP_X_WINE_STATIC_H_
#define _GRAP_X_WINE_STATIC_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class GAMEENGINE_API CWEStatic : public CMODlgItem
{
public:
//  GXINT AddString(GXLPWSTR lpString);
//  GXINT AddString(GXLPWSTR lpString, GXLPARAM lParam);
//  GXINT DeleteString(GXINT nIndex);
//  GXINT GetCount();
  static CWEStatic* GetDlgItem(GXHWND hParent, GXINT idCtrl);
public:
  CWEStatic(GXHWND hWnd);
  virtual ~CWEStatic();
};
typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_WINE_STATIC_H_