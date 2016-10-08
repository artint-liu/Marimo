#ifndef _GRAP_X_UI_LIST_H_
#define _GRAP_X_UI_LIST_H_

//class CMOWnd;
class CMODialog;
class CMODlgItem;
class CMOListBoxReceiver;
//class MODataPool;

namespace Marimo
{
  class DataPoolVariable;
  class DataPool;
} // namespace Marimo

namespace GXUI
{
  class IListDataAdapter;
  typedef Marimo::DataPoolVariable MOVariable;
  typedef Marimo::DataPool MODataPool;
}

class GAMEENGINE_API CMOListBox : public CMODlgItem
{
private:
  CMOListBoxReceiver*    m_pReceiver;
public:
  GXHRESULT   GetDataAdapter  (GXUI::IListDataAdapter** ppAdapter);
  GXHRESULT   GetDataPool     (GXUI::MODataPool** ppDataPool);
  GXHRESULT   GetArrayElement (GXUI::MOVariable* pVariable);

  GXINT       AddStringW      (GXLPCWSTR lpString);
  GXINT       AddStringA      (GXLPCSTR lpString);
  GXINT       GetStringLength (GXINT nIndex);
  GXLRESULT   GetStringW      (GXINT nIndex, clStringW& str);
  GXLRESULT   GetStringA      (GXINT nIndex, clStringA& str);
  GXINT       DeleteString    (GXINT nIndex);
  void        ResetContent    ();   // 清除所有项目
  GXINT       GetCount        () const;
  GXINT       GetCurSel       () const;
  GXBOOL      IsSelected      (GXSIZE_T index) const;
  GXHRESULT   SetAdapter      (GXUI::IListDataAdapter* pAdapter);
  GXCOLORREF  SetColor        (GXUINT nType, GXCOLORREF color);
  GXUINT      SetColumnsWidth (const GXUINT* pColumns, GXUINT nCount);
  GXUINT      GetColumnsWidth (GXUINT* pColumns, GXUINT nCount);

  //template <class _VectorT>
  //GXUINT      SetColumnsWidth (const _VectorT<GXUINT>& aColumns)
  //{
  //  return SetColumnsWidth(&aColumns.front(), aColumns.size());
  //}

  //template <class _VectorT>
  //GXUINT      GetColumnsWidth (_VectorT<GXUINT>& aColumns)
  //{
  //  GXUINT nNeed = GetColumnsWidth(NULL, 0);
  //  aColumns.insert(aColumns.begin(), nNeed, 0);
  //  return GetColumnsWidth(&aColumns.front(), aColumns.size());
  //}

#ifdef REFACTOR_GXFC
#else
  void        SetReceiver     (CMOListBoxReceiver* pReceiver);
  virtual CGXReceiver*  GetReceiver   ();
  virtual void        SetReceiver         (CGXReceiver* pReceiver);
#endif // #ifdef REFACTOR_GXFC

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;
#ifdef REFACTOR_GXFC
#else
  virtual GXLRESULT   InvokeReceiver      (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT   InvokeReceiver      (GXNMHDR* pnmhdr);
  virtual GXLRESULT   Invoke              (CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT   Invoke              (CGXReceiver* pReceiver, GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  //static CGXListBox* GetDlgItem(GXHWND hParent, GXINT idCtrl);
public:
  CMOListBox();
  CMOListBox(GXHWND hWnd);
  virtual ~CMOListBox();
};

class GAMEENGINE_API CMOListBoxReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand  (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify   (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  //virtual GXLRESULT OnListBoxClicked        (CMOListBox* pSender, GXLPCWSTR szBtnName) { return 0L; }
  virtual GXVOID OnListBoxSelChange      (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxSelCancel      (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxErrorSpace     (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxDoubleClicked  (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxSetFocus       (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxKillFocus      (CMOListBox* pSender) {}
  virtual GXVOID OnListBoxCtrlCommand    (CMOListBox* pSender, GXHWND hTmplItemWnd, GXINT nListItem, GXINT nCommand) {}
};

//typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_UI_LIST_H_