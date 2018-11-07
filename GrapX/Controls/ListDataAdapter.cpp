#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.h"
#include "GrapX/GResource.h"

#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataInfrastructure.h"

#include "ListDataAdapter.h"

namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////
  CDefListDataAdapter::CDefListDataAdapter(GXHWND hWnd)
    : IListDataAdapter(hWnd)
    , m_pDataPool     (NULL)
    , m_DynArray      ()
    , m_nHeight       (10)  // 绝对是临时的!
  {
  }

  CDefListDataAdapter::~CDefListDataAdapter()
  {
    m_pDataPool->Ignore(&m_DynArray, m_hWnd);
    SAFE_RELEASE(m_pDataPool);
  }

  GXBOOL CDefListDataAdapter::Initialize()
  {
    static
      Marimo::DATAPOOL_VARIABLE_DECLARATION s_DefaultListItem[] = {
        {"string", "name", 0, -1, 0},
        {NULL, NULL},
    };
    Marimo::DataPool::CreateDataPool(&m_pDataPool, NULL, NULL, s_DefaultListItem);
    m_strArrayName = "name";
    m_pDataPool->QueryByName(m_strArrayName, &m_DynArray);
    m_pDataPool->Watch(&m_DynArray, m_hWnd);

    return TRUE;
  }

  GXBOOL CDefListDataAdapter::Initialize(MOVariable& Var)
  {
    m_strArrayName = Var.GetName();
    if(m_strArrayName.GetLength() == 0) {
      CLBREAK;
      return FALSE;
    }
    m_DynArray = Var;
    Var.GetPool(&m_pDataPool);
    m_pDataPool->Watch(&Var, m_hWnd);

    return TRUE;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT CDefListDataAdapter::AddRef()
  {
    const GXLONG nRef = gxInterlockedIncrement(&m_nRefCount);
    return nRef;
  }

  GXHRESULT CDefListDataAdapter::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      //m_pDataPool->UnregisterOutlet(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

//  GXBOOL CDefListDataAdapter::IsAutoKnock() const
//  {
//#ifdef ENABLE_DATAPOOL_WATCHER
//    if(m_pDataPool) {
//      return m_pDataPool->IsAutoKnock();
//    }
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
//    return FALSE;
//  }

  GXSIZE_T CDefListDataAdapter::GetCount() const
  {
    GXSIZE_T count = m_DynArray.GetLength();
    return count;
  }

  GXSIZE_T CDefListDataAdapter::AddStringW(GXLPCWSTR szName, GXLPCWSTR lpString)
  {
    MOVariable newstr = m_DynArray.NewBack();
    newstr.Set(lpString);
//#ifdef ENABLE_DATAPOOL_WATCHER
//    if( ! m_pDataPool->IsAutoKnock()) {
//      newstr.Impulse(Marimo::DATACT_Insert);
//    }
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return m_DynArray.GetLength() - 1;
  }

  GXBOOL CDefListDataAdapter::GetStringW(GETSTRW* pItemStrDesc)
  {
    pItemStrDesc->VarString = m_DynArray.IndexOf(pItemStrDesc->item);
    pItemStrDesc->sString   = pItemStrDesc->VarString.ToStringW();
    //pItemStrDesc->top       = pItemStrDesc->nIdx * m_nHeight;
    //pItemStrDesc->bottom    = pItemStrDesc->top + m_nHeight;
    //pItemStrDesc->dwStatus  = 0;
    return TRUE;
  }
  
  //GXBOOL CDefListDataAdapter::SetItemData(GETSTRW* pItemStrDesc, GXLPARAM lParam)
  //{
  //}

  //GXLPARAM CDefListDataAdapter::GetItemData(GETSTRW* pItemStrDesc)
  //{
  //}

  MOVariable CDefListDataAdapter::GetVariable()
  {
    return m_DynArray;
  }

  GXHRESULT CDefListDataAdapter::GetDataPool(MODataPool** ppDataPool)
  {
    *ppDataPool = m_pDataPool;
    if(m_pDataPool != NULL) {
      return m_pDataPool->AddRef();
    }
    return GX_FAIL;
  }

  //GXINT CDefListDataAdapter::GetItemBottoms( GXINT* bottoms, const GXINT* indices, int count ) const
  //{
  //  // 参数合法性要有调用者保证,这里只是校验
  //  ASSERT(count >= 1);
  //  ASSERT(bottoms && indices);

  //  int i = 0;
  //  do {
  //    bottoms[i] = m_nHeight + m_nHeight * indices[i];
  //  } while (++i < count);
  //  return i;
  //}

  //GXINT CDefListDataAdapter::SetReferenceHeight( GXINT nItemHeight )
  //{
  //  GXINT nPrev = m_nHeight;
  //  m_nHeight = nItemHeight;
  //  return nPrev;
  //}

  //GXDWORD CDefListDataAdapter::GetStatus(GXINT item) const
  //{
  //  return 0;
  //}

  //GXDWORD CDefListDataAdapter::SetStatus(GXINT item, GXDWORD dwNewStatus)
  //{
  //  return 0;
  //}

  //////////////////////////////////////////////////////////////////////////
} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE