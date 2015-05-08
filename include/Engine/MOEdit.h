#ifndef _GRAP_X_UI_EDIT_H_
#define _GRAP_X_UI_EDIT_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class GAMEENGINE_API CMOEdit : public CMODlgItem
{
public:
  void SetSelect      (GXINT nStart, GXINT nEnd);
  void ReplaceSelectW (GXLPCWSTR szReplace, GXBOOL bCanUndo);
  void ReplaceSelectA (GXLPCSTR szReplace, GXBOOL bCanUndo);
  GXUINT SetLimitText (GXUINT nMaxLength);
  GXUINT GetLimitText ();

  // TODO: 增加一个直接设置Int的接口

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;

public:
  CMOEdit();
  CMOEdit(GXHWND hWnd);
  virtual ~CMOEdit();
};
typedef CMOEdit *LPCGXEdit;
#endif // #ifndef _GRAP_X_WINE_EDIT_H_