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
    DA_PotentialChange,   // TODO: 去掉这个
    DA_Insert,
    DA_Delete,
  };

  struct REFLECTKNOCK
  {
    const IDataOutlet* pSender;
    const IDataPool*   pOccurDP;
    DataAction        eAction;
    GXLPVOID          lpData;
    GXINT             nIndex;     // ArrayDataPool一定有, 其他不一定总是有效
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
  // 数据适配器
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
  //  //WndHandleArray  m_aHandles; // TODO:一个Container只属于一个Wnd
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
  //  OutletArray* m_pOutlets;  // 用指针是因为vector在lib和app中创建修改销毁,最后无法正确维护内存而出错
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
  //  //OutletArray* m_pOutlets;  // 用指针是因为vector在lib和app中创建修改销毁,最后无法正确维护内存而出错
  //  _TCls*        m_pData;
  //  size_t        m_uCapacity;  // 数组容量
  //  size_t        m_uSize;      // 数据大小
  //  size_t        m_uCursor;    // 游标
  //  const size_t  m_uIncrease;  // 增量
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

  //  _TCls& operator[](int idx)  // TODO: 这个改为const限制
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
      // 参数值
      GXINT     item;       // list item 索引
      GXINT     element;    // rich list有效，item 中控件的索引
      GXLPCWSTR name;       // rich list有效，item 中控件名
      GXHWND    hItemWnd;   // rich list有效，item 中控件句柄
      GXRECT    rect;       // item的大小，list box client空间的坐标

      // rich list 控件中：
      // hItemWnd为NULL时表示需要获得默认字符串，一般出现在LB_GETTEXT消息中

      // 返回值
      MOVariable  VarString;
      clStringW   sString;
    };
  protected:
    virtual ~IListDataAdapter();
    GXHWND  m_hWnd;

  public:
    IListDataAdapter(GXHWND hWnd);

    //virtual GXHRESULT AddRef  ();
    //virtual GXHRESULT Release ();

    //virtual GXHRESULT Knock         (GXHWND hSender, GXLPVOID lpData) const;

    // 要支持szName == NULL
//#ifdef ENABLE_DATAPOOL_WATCHER
//    GXSTDINTERFACE(GXBOOL     IsAutoKnock       () GXCONST);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    GXSTDINTERFACE(GXSIZE_T   GetCount          () GXCONST);
    //GXSTDINTERFACE(GXBOOL     IsFixedHeight     () GXCONST);  // MultiColumn 属性时这个无效,认为是统一高度
    //GXSTDINTERFACE(GXINT      GetItemHeight     (GXINT nIdx) GXCONST); 
    //GXSTDINTERFACE(GXBOOL GetItemStatistics (GXINT nIdx, LISTBOXITEMSTAT* pStatictist) const);
    GXSTDINTERFACE(GXSIZE_T   AddStringW        (GXLPCWSTR szName, GXLPCWSTR lpString));
    GXSTDINTERFACE(GXBOOL     GetStringW        (GETSTRW* pItemStrDesc));   // 返回TRUE代表List将进行默认的显示操作，如ListSimple中绘制字符串，ListCustomize中SetWindowText，返回false表示GetStringW已经自己处理了显示方式。
    //GXSTDINTERFACE(GXBOOL     SetItemData       (GETSTRW* pItemStrDesc, GXLPARAM lParam));
    //GXSTDINTERFACE(GXLPARAM   GetItemData       (GETSTRW* pItemStrDesc));
    GXSTDINTERFACE(MOVariable GetVariable       ());
    GXSTDINTERFACE(GXHRESULT  GetDataPool       (MODataPool** ppDataPool));
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