#ifndef _GRAP_X_WINE_LISTBOX_H_
#define _GRAP_X_WINE_LISTBOX_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class GAMEENGINE_API CWEListBox : public CMODlgItem
{
public:
  GXINT AddString(GXLPWSTR lpString);
  GXINT AddString(GXLPWSTR lpString, GXLPARAM lParam);
  GXINT DeleteString(GXINT nIndex);
  GXINT GetCount();
  static CWEListBox* GetDlgItem(GXHWND hParent, GXINT idCtrl);

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;

public:
  CWEListBox();
  CWEListBox(GXHWND hWnd);
  virtual ~CWEListBox();
};
typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_WINE_LISTBOX_H_