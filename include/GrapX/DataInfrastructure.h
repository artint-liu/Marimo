#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GX_DATA_INFRASTRUCTURE_H_
#define _GX_DATA_INFRASTRUCTURE_H_
class GUnknown;
namespace Marimo
{
  class DataPoolVariable;
  class DataPool;
} // namespace Marimo


namespace GXUI
{
  class IDataOutlet;
  class IDataPool;
  typedef Marimo::DataPoolVariable MOVariable;
  typedef Marimo::DataPool MODataPool;

  enum DataAction
  {
    DA_Change,
    DA_PotentialChange,   // TODO: ȥ�����
    DA_Insert,
    DA_Delete,
  };

  struct REFLECTKNOCK
  {
    const IDataOutlet* pSender;
    const IDataPool*   pOccurDP;
    DataAction        eAction;
    GXLPVOID          lpData;
    GXINT             nIndex;     // ArrayDataPoolһ����, ������һ��������Ч
  };
  typedef REFLECTKNOCK*        LPREFLECTKNOCK;
  typedef const REFLECTKNOCK*  LPCREFLECTKNOCK;

  class IDataOutlet : public GUnknown
  {
  protected:
    GXHWND m_hWndOwner;
  public:
    IDataOutlet();
    GXHRESULT ReflectKnock   (LPCREFLECTKNOCK pReflectKnock) const;
    //GXSTDINTERFACE(GXHWND    GetLastWndSender() const);
    inline GXHWND SetWndOwner(GXHWND hWnd);
    inline GXHWND GetWndOwner() const;
  };

  GXHWND IDataOutlet::SetWndOwner(GXHWND hWnd)
  {
    GXHWND hPrevWnd = m_hWndOwner;
    m_hWndOwner = hWnd;
    return hPrevWnd;       
  }

  GXHWND IDataOutlet::GetWndOwner() const
  {
    return m_hWndOwner;
  }
  //////////////////////////////////////////////////////////////////////////
  // ����������
  class IDataAdapter : public IDataOutlet
  {
  public:
  protected:
  private:
  };

  //class GXDLL IDataContainer : public IDataOutlet
  //{
  //public:
  //  //typedef clvector<GXHWND>  WndHandleArray;
  //protected:
  //private:
  //  //WndHandleArray  m_aHandles; // TODO:һ��Containerֻ����һ��Wnd
  //public:
  //  //GXHRESULT RegisterWnd   (GXHWND hWnd);
  //  //GXHRESULT UnregisterWnd (GXHWND hWnd);
  //  virtual GXHRESULT Knock         (GXHWND hSender, DataAction eAction, GXLPVOID lpData, GXINT nIndex) const = NULL;
  //  //virtual GXHRESULT ReflectKnock  (LPCREFLECTKNOCK pReflectKnock) const;
  //};

  //class GXDLL IDataPool : public GUnknown
  //{
  //public:
  //  typedef clvector<IDataOutlet*> OutletArray;
  //protected:
  //  size_t      m_uSelfSize;
  //  OutletArray* m_pOutlets;  // ��ָ������Ϊvector��lib��app�д����޸�����,����޷���ȷά���ڴ������
  //private:
  //public:
  //  GXSTDINTERFACE(GXHRESULT AddRef  ());
  //  GXSTDINTERFACE(GXHRESULT Release ());

  //  IDataPool(size_t uSelf);
  //  GXHRESULT RegisterOutlet  (IDataOutlet* pOutlet);
  //  GXHRESULT UnregisterOutlet(IDataOutlet* pOutlet);
  //  GXHRESULT Knock           (const IDataOutlet* pSender, DataAction eAction, GXLPVOID lpData, GXINT nIndex) const;
  //  virtual GXBOOL    IsMember        (GXLPVOID ptr) const;
  //  virtual GXLPVOID  GetMember       (size_t uOffset) const;
  //  virtual size_t    GetMemberOffset (GXLPVOID ptr) const;
  //};

  // PileDataPool
  // ArrayDataPool

  //template <class _TCls>
  //class IArrayDataPool : public IDataPool
  //{
  //public:
  //  //typedef clvector<DataOutlet*> OutletArray;
  //protected:
  //  //size_t      m_uSelfSize;
  //  //OutletArray* m_pOutlets;  // ��ָ������Ϊvector��lib��app�д����޸�����,����޷���ȷά���ڴ������
  //  _TCls*        m_pData;
  //  size_t        m_uCapacity;  // ��������
  //  size_t        m_uSize;      // ���ݴ�С
  //  size_t        m_uCursor;    // �α�
  //  const size_t  m_uIncrease;  // ����
  //private:
  //  size_t GrowTo(size_t uNewSize)
  //  {
  //    _TCls* pNewData = new _TCls[uNewSize];
  //    //memcpy(pNewData, m_pData, sizeof(_TCls) * m_uSize);
  //    for(size_t i = 0; i < m_uSize; i++) {
  //      pNewData[i] = m_pData[i];
  //    }
  //    delete [] m_pData;
  //    m_pData = pNewData;
  //    m_uCapacity = uNewSize;
  //    return uNewSize;
  //  }
  //  GXBOOL IsArrayMember(GXLPVOID ptr) const
  //  {
  //    return ((GXDWORD_PTR)ptr >= (GXDWORD_PTR)m_pData && (GXDWORD_PTR)ptr < (GXDWORD_PTR)&m_pData[m_uSize]);
  //  }
  //public:
  //  GXSTDINTERFACE(GXHRESULT AddRef  ());
  //  GXSTDINTERFACE(GXHRESULT Release ());

  //  IArrayDataPool(size_t uSelf, size_t uInit, size_t uIncrease)
  //    : IDataPool    (uSelf)
  //    , m_pData     (NULL)
  //    , m_uCapacity (uInit)
  //    , m_uSize     (0)
  //    , m_uIncrease (uIncrease)
  //    , m_uCursor   (0)
  //  {
  //    m_pData = new _TCls[uInit];
  //    ASSERT(m_pData != NULL);
  //  }

  //  virtual ~IArrayDataPool()
  //  {
  //    delete [] m_pData;
  //  }
  //  //GXHRESULT RegisterOutlet  (DataOutlet* pOutlet);
  //  //GXHRESULT UnregisterOutlet(DataOutlet* pOutlet);
  //  //GXHRESULT Knock           (const DataOutlet* pSender, GXHWND hSender) const;
  //  virtual GXBOOL   IsMember   (GXLPVOID ptr) const
  //  {
  //    return IsArrayMember(ptr) || IDataPool::IsMember(ptr);
  //  }

  //  virtual GXLPVOID GetMember  (size_t uOffset) const
  //  {
  //    return ((GXBYTE*)&m_pData[m_uCursor]) + uOffset;
  //  }

  //  size_t GetMemberOffset(GXLPVOID ptr) const
  //  {
  //    if(IsArrayMember(ptr))
  //    {
  //      return ((size_t)ptr - (size_t)m_pData) % sizeof(_TCls);
  //    }
  //    return IDataPool::GetMemberOffset(ptr);
  //  }

  //  void SetCursor(size_t uPos)
  //  {
  //    ASSERT(uPos < m_uSize);
  //    m_uCursor = uPos;
  //  }

  //  void push_back(const _TCls& val)
  //  {
  //    if(m_uSize + 1 > m_uCapacity) {
  //      GrowTo(m_uCapacity + m_uIncrease);
  //    }
  //    m_pData[m_uSize++] = val;
  //    Knock(NULL, DA_Insert, &m_pData[m_uSize - 1], m_uSize - 1);
  //  }

  //  void clear()
  //  {
  //    delete[] m_pData;
  //    m_pData = new _TCls[m_uIncrease];
  //    m_uSize = 0;
  //    m_uCapacity = m_uIncrease;
  //    m_uCursor = 0;
  //    Knock(NULL, DA_Delete, NULL, -1);
  //  }

  //  size_t size() const
  //  {
  //    return m_uSize;
  //  }

  //  void pop_back(const _TCls& val)
  //  {
  //    Knock(NULL, DA_Delete, &m_pData[m_uSize - 1], m_uSize - 1);
  //    ASSERT(m_uSize != 0);
  //    m_uSize--;
  //  }

  //  _TCls& front() const
  //  {
  //    return m_pData[0];
  //  }

  //  _TCls& back() const
  //  {
  //    return m_pData[m_uSize - 1];
  //  }

  //  _TCls& operator[](int idx)  // TODO: �����Ϊconst����
  //  {
  //    ASSERT(idx < (int)m_uSize);
  //    Knock(NULL, DA_PotentialChange, &m_pData[idx], idx);
  //    return m_pData[idx];
  //  }

  //};
  //////////////////////////////////////////////////////////////////////////
  class GXDLL IListDataAdapter : public IDataAdapter
  {
  public:
    struct GETSTRW
    {
      // ����ֵ
      GXINT     item;       // list item ����
      GXINT     element;    // rich list��Ч��item �пؼ�������
      GXLPCWSTR name;       // rich list��Ч��item �пؼ���
      GXHWND    hItemWnd;   // rich list��Ч��item �пؼ����
      GXRECT    rect;       // item�Ĵ�С��list box client�ռ������

      // rich list �ؼ��У�
      // hItemWndΪNULLʱ��ʾ��Ҫ���Ĭ���ַ�����һ�������LB_GETTEXT��Ϣ��

      // ����ֵ
      MOVariable  VarString;
      clStringW   sString;
    };
  protected:
    GXHWND  m_hWnd;
  public:
    IListDataAdapter(GXHWND hWnd);
    virtual ~IListDataAdapter();

    //virtual GXHRESULT AddRef  ();
    //virtual GXHRESULT Release ();

    //virtual GXHRESULT Knock         (GXHWND hSender, GXLPVOID lpData) const;

    // Ҫ֧��szName == NULL
//#ifdef ENABLE_DATAPOOL_WATCHER
//    GXSTDINTERFACE(GXBOOL     IsAutoKnock       () GXCONST);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    GXSTDINTERFACE(GXSIZE_T   GetCount          () GXCONST);
    //GXSTDINTERFACE(GXBOOL     IsFixedHeight     () GXCONST);  // MultiColumn ����ʱ�����Ч,��Ϊ��ͳһ�߶�
    //GXSTDINTERFACE(GXINT      GetItemHeight     (GXINT nIdx) GXCONST); 
    //GXSTDINTERFACE(GXBOOL GetItemStatistics (GXINT nIdx, LISTBOXITEMSTAT* pStatictist) const);
    GXSTDINTERFACE(GXINT      AddStringW        (GXLPCWSTR szName, GXLPCWSTR lpString));
    GXSTDINTERFACE(GXBOOL     GetStringW        (GETSTRW* pItemStrDesc));   // ����TRUE����List������Ĭ�ϵ���ʾ��������ListSimple�л����ַ�����ListCustomize��SetWindowText������false��ʾGetStringW�Ѿ��Լ���������ʾ��ʽ��
    GXSTDINTERFACE(MOVariable GetVariable       ());
    GXSTDINTERFACE(GXHRESULT  GetDataPool       (MODataPool** ppDataPool));

    //virtual GXHWND    GetLastWndSender  () const;  // �����ʵû����
    //virtual GXHRESULT     ReflectKnock     (LPCREFLECTKNOCK pReflectKnock) const;
  };
} // namespace GXUI



//typedef ScriptedDataPool::DataPool MODataPool;

//class GXUnifiedVariableElement
//{
//  clString name;
//  clString val;
//  union {
//    GXBOOL  bval;
//    GXINT   nval;
//    GXDWORD dwval;
//    GXFLOAT fval;
//  };
//};
//typedef GXUnifiedVariableElement GXUVE;
#endif // _GX_DATA_INFRASTRUCTURE_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE