#ifndef _GXUI_SIMPLE_LIST_DATA_ADAPTER_H_
#define _GXUI_SIMPLE_LIST_DATA_ADAPTER_H_

namespace GXUI
{
  class CDefListDataAdapter : public IListDataAdapter
  {
  private:
    MODataPool*   m_pDataPool;
    MOVariable    m_DynArray;
    clStringA     m_strArrayName;
    GXINT         m_nHeight;

  public:
    CDefListDataAdapter(GXHWND hWnd);
    virtual ~CDefListDataAdapter();

    GXBOOL Initialize();
    GXBOOL Initialize(MOVariable& Var);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef        ();
    virtual GXHRESULT   Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    //virtual GXBOOL      IsAutoKnock         () const;
    //virtual GXINT       SetReferenceHeight  (GXINT nItemHeight);
    virtual GXSIZE_T    GetCount        () const;
    //virtual GXINT       GetItemBottoms      (GXINT* bottoms, const GXINT* indices, int count) GXCONST;

    virtual GXSIZE_T    AddStringW          (GXLPCWSTR szName, GXLPCWSTR lpString);
    virtual GXBOOL      GetStringW          (GETSTRW* pItemStrDesc);
    //virtual GXDWORD     GetStatus           (GXINT item) GXCONST;
    //virtual GXDWORD     SetStatus           (GXINT item, GXDWORD dwNewStatus);
    virtual MOVariable  GetVariable         ();
    virtual GXHRESULT   GetDataPool         (MODataPool** ppDataPool);
  };

  namespace DefaultListDataAdapter
  {
    class CBase : public IListDataAdapter
    {
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT AddRef()
      {
        const GXLONG nRefCount = gxInterlockedIncrement(&m_nRefCount);
        return nRefCount;
      }

      virtual GXHRESULT Release()
      {
        GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
        if(nRefCount == 0)
        {
          delete this;
          return GX_OK;
        }
        return nRefCount;
      }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    };

    class CStructHasStringAndBottom : public CBase
    {

    };
  }
} // namespace GXUI

#endif // _GXUI_SIMPLE_LIST_DATA_ADAPTER_H_