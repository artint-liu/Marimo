#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataInfrastructure.H"
#include "GrapX/GXUser.H"

namespace GXUI
{
  //GXHRESULT DataOutlet::DispatchKnock(DataOutlet* pSender, GXHWND hSender) const
  //{
  //  
  //}

  //GXHRESULT DataContainer::RegisterWnd(GXHWND hWnd)
  //{
  //  for(WndHandleArray::iterator it = m_aHandles.begin();
  //    it != m_aHandles.end(); ++it)
  //  {
  //    if(*it == hWnd) {
  //      return -1;
  //    }
  //  }
  //  m_aHandles.push_back(hWnd);
  //  return GX_OK;;
  //}

  //GXHRESULT DataContainer::UnregisterWnd(GXHWND hWnd)
  //{
  //  for(WndHandleArray::iterator it = m_aHandles.begin();
  //    it != m_aHandles.end(); ++it)
  //  {
  //    if(*it == hWnd) {
  //      m_aHandles.erase(it);
  //      return GX_OK;
  //    }
  //  }
  //  return GX_FAIL;
  //}
  IDataOutlet::IDataOutlet()
    : m_hWndOwner(NULL)
  {
  }

  GXHRESULT IDataOutlet::ReflectKnock(LPCREFLECTKNOCK pReflectKnock) const
  {
    //for(WndHandleArray::const_iterator it = m_aHandles.begin();
    //  it != m_aHandles.end(); ++it)
    //{
    //  gxSendMessage(*it, GXWM_KNOCK, (GXWPARAM)pSender->GetWndOwner(), (GXLPARAM)pSender);
    //}
    GXWPARAM wParam = NULL;
    if(pReflectKnock != NULL && pReflectKnock->pSender != NULL) {
      wParam = (GXWPARAM)(pReflectKnock->pSender->GetWndOwner());
    }

    gxSendMessage(m_hWndOwner, GXWM_IMPULSE, wParam, (GXLPARAM)pReflectKnock);
    return GX_OK;
  }
  //GXHRESULT DataContainer::Knock(GXHWND hSender) const
  //{
  //  for(WndHandleArray::const_iterator it = m_aHandles.begin();
  //    it != m_aHandles.end(); ++it)
  //  {
  //    gxSendMessage(*it, GXWM_KNOCK, (GXWPARAM)hSender, 0);
  //  }
  //  return GX_OK;
  //}
  //////////////////////////////////////////////////////////////////////////
  //IDataPool::IDataPool(size_t uSelf)
  //  : m_uSelfSize(uSelf)
  //  , m_pOutlets(NULL)
  //{
  //}

  //GXBOOL IDataPool::IsMember(void* ptr) const
  //{
  //  const size_t sizeofDataPool = sizeof(IDataPool);
  //  return ((GXDWORD_PTR)ptr >= (GXDWORD_PTR)this + sizeofDataPool) &&
  //    ((GXDWORD_PTR)ptr < (GXDWORD_PTR)this + m_uSelfSize);
  //}

  //GXHRESULT IDataPool::RegisterOutlet(IDataOutlet* pOutlet)
  //{
  //  if(m_pOutlets == NULL) {
  //    m_pOutlets = new OutletArray;
  //    m_pOutlets->push_back(pOutlet);
  //    return GX_OK;
  //  }
  //  for(OutletArray::iterator it = m_pOutlets->begin();
  //    it != m_pOutlets->end(); ++it)
  //  {
  //    if(*it == pOutlet) {
  //      return GX_FAIL;
  //    }
  //  }
  //  m_pOutlets->push_back(pOutlet);
  //  return GX_OK;
  //}

  //GXHRESULT IDataPool::UnregisterOutlet(IDataOutlet* pOutlet)
  //{
  //  for(OutletArray::iterator it = m_pOutlets->begin();
  //    it != m_pOutlets->end(); ++it)
  //  {
  //    if(*it == pOutlet) {
  //      m_pOutlets->erase(it);
  //      if(m_pOutlets->size() == 0) {
  //        SAFE_DELETE(m_pOutlets);
  //      }
  //      return GX_OK;
  //    }
  //  }
  //  return GX_FAIL;
  //}

  //GXHRESULT IDataPool::Knock(const IDataOutlet* pSender, DataAction eAction, GXLPVOID lpData, GXINT nIndex) const
  //{
  //  if(m_pOutlets == NULL) 
  //    return GX_OK;
  //  REFLECTKNOCK ReflectKnock;
  //  ReflectKnock.pSender  = pSender;
  //  ReflectKnock.pOccurDP = this;
  //  ReflectKnock.eAction  = eAction;
  //  ReflectKnock.lpData   = lpData;
  //  ReflectKnock.nIndex   = nIndex;

  //  for(OutletArray::const_iterator it = m_pOutlets->begin();
  //    it != m_pOutlets->end(); ++it)
  //  {
  //    (*it)->ReflectKnock(&ReflectKnock);
  //  }
  //  return GX_OK;
  //}

  //GXLPVOID IDataPool::GetMember(size_t uOffset) const
  //{
  //  return GXLPVOID(((GXBYTE*)this) + uOffset);
  //}

  //size_t IDataPool::GetMemberOffset(GXLPVOID ptr) const
  //{
  //  ASSERT(IsMember(ptr));
  //  return (size_t)ptr - (size_t)this;
  //}
  //////////////////////////////////////////////////////////////////////////
  //ArrayDataPool::ArrayDataPool(size_t uSelf, size_t uInit, size_t uIncrease)
  //  : m_pData     (NULL)
  //  , m_uCapacity (uInit)
  //  , m_uSize     (0)
  //  , m_uIncrease (uIncrease)
  //  , m_uCursor   (0)
  //{

  //}
  ////GXHRESULT RegisterOutlet  (DataOutlet* pOutlet);
  ////GXHRESULT UnregisterOutlet(DataOutlet* pOutlet);
  ////GXHRESULT Knock           (const DataOutlet* pSender, GXHWND hSender) const;
  //virtual GXBOOL   IsMember   (void* ptr) const;
  //virtual GXLPVOID GetMember  (size_t uOffset) const;

  //void push_back(const _TCls& val);
  //void pop_back(const _TCls& val);
  //_TCls& front() const;
  //_TCls& back() const;
  ListDataAdapter::ListDataAdapter(GXHWND hWnd)
    : m_hWnd(hWnd)
  {
    SetWndOwner(hWnd);
  }

  ListDataAdapter::~ListDataAdapter()
  {
  }
} // namespace GXUI

#endif // #ifndef _DEV_DISABLE_UI_CODE