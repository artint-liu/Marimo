#ifndef _GRAP_X_WINE_EDIT_H_
#define _GRAP_X_WINE_EDIT_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class GAMEENGINE_API CWEEdit : public CMODlgItem
{
public:
  void SetSelect      (GXINT nStart, GXINT nEnd);
  void ReplaceSelectW (GXLPCWSTR szReplace, GXBOOL bCanUndo);

  // TODO: 增加一个直接设置Int的接口

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;

public:
  CWEEdit();
  CWEEdit(GXHWND hWnd);
  virtual ~CWEEdit();
};
typedef CWEEdit *LPWEEdit;
#endif // #ifndef _GRAP_X_WINE_EDIT_H_