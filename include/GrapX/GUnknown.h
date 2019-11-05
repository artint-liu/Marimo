#ifndef _GRAPH_UNKNOWN_H_
#define _GRAPH_UNKNOWN_H_

#pragma warning(disable : 4275)
#pragma warning(disable : 4251)

#define GXSUCCEEDED(r)      (((GXHRESULT)(r)) >= 0)
#define GXFAILED(r)         (!GXSUCCEEDED(r))
#define GXSTDINTERFACE(x)  virtual x = 0
#define GXSTDIMPLEMENT(x)  x override

// 虚函数化的AddRef和Release
#define ENABLE_VIRTUALIZE_ADDREF_RELEASE

class GUnknown
{
protected:
  GUnknown() : m_uRefCount(0)
  {
#ifdef _CL_ARCH_X86
# ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    STATIC_ASSERT(sizeof(GUnknown) == 8);
# else
    STATIC_ASSERT(sizeof(GUnknown) == 4);
# endif
#elif defined(_CL_ARCH_X64)
    //STATIC_ASSERT(sizeof(GUnknown) == 16);
#endif // #ifdef _X86
  }

  virtual ~GUnknown() {}

  union
  {
    GXULONG m_uRefCount;
    GXLONG  m_nRefCount;
  };
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(GXHRESULT AddRef  ());
  GXSTDINTERFACE(GXHRESULT Release ());
#else
  GXHRESULT AddRef  ();
  GXHRESULT Release ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
};

//
// 示例:
//GXHRESULT GUnknown::AddRef()
//{
//  m_uRefCount++;
//  if(m_uRefCount == 1)
//  {
//    // TODO: Initialize Here ...
//    if(<SUCCEEDED>) {
//      return 0;
//    }
//    else
//      return <ERROR CODE>
//  }
//  return m_uRefCount;
//}
//
//GXLRESULT GUnknown::Release()
//{
//  m_uRefCount--;
//  if(m_uRefCount == 0)
//  {
//    // 这里写释放代码
//    delete this;
//    return GX_OK;
//  }
//  return m_uRefCount;
//}

namespace GrapX
{
  template<class _TUnknown>
  class ObjectT
  {
  protected:
    _TUnknown* m_pObject;

  public:
    typedef void (*unspecified_bool_type)(ObjectT***);

    ObjectT() : m_pObject(NULL)
    {}

    explicit ObjectT(_TUnknown* pObj)
      : m_pObject(pObj)
    {
      SAFE_ADDREF(pObj);
    }

    explicit ObjectT(const ObjectT& NewObject)
      : m_pObject(NULL)
    {
      operator=(NewObject.operator->());
    }

    ~ObjectT()
    {
      SAFE_RELEASE(m_pObject);
    }

    _TUnknown** operator&()
    {
      return &m_pObject;
    }

    _TUnknown* operator->()
    {
      return m_pObject;
    }

    _TUnknown* operator->() const
    {
      return m_pObject;
    }

    operator _TUnknown*() const
    {
      return m_pObject;
    }

    bool operator!() const
    {
      return IsNullPtr();
    }

    bool operator<(const ObjectT& obj) const
    {
      return m_pObject < obj.m_pObject;
    }

    bool IsNullPtr() const
    {
      return m_pObject == NULL;
    }

    template<class ObjectT2>
    ObjectT2* CastPtr()
    {
      return static_cast<ObjectT2*>(m_pObject);
    }

    template<class ObjectT2>
    ObjectT2* CastPtr() const
    {
      return static_cast<ObjectT2*>(m_pObject);
    }

    template<class ObjectT2>
    ObjectT2* ReinterpretCastPtr()
    {
      return reinterpret_cast<ObjectT2*>(m_pObject);
    }

    template<class ObjectT2>
    ObjectT2& Cast()
    {
      return *reinterpret_cast<ObjectT2*>(&*this);
    }

    template<class ObjectT2>
    const ObjectT2& Cast() const
    {
      return *reinterpret_cast<const ObjectT2*>(&*this);
    }

    ObjectT& operator=(const ObjectT& NewObject)
    {
      return operator=(NewObject.operator->());
    }

    ObjectT& operator=(_TUnknown* pNewObject)
    {
      if(m_pObject != pNewObject) {
        if (pNewObject) {
          pNewObject->AddRef();
        }
        if(m_pObject) {
          m_pObject->Release();
        }
        m_pObject = pNewObject;
      }
      return *this;
    }

    bool operator==(const GUnknown* pCmpObject) const
    {
      return m_pObject == pCmpObject;
    }

    bool operator!=(const GUnknown* pCmpObject) const
    {
      return m_pObject != pCmpObject;
    }

    //template<class _TUnknown2>
    //bool equals(const ObjectT<_TUnknown2>& Object2) const
    //{
    //  return m_pObject == Object2.operator->();
    //}

    operator unspecified_bool_type() const
    {
      return IsNullPtr() ? NULL : __unspecified_bool_type;
    }

  protected:
    static void __unspecified_bool_type(ObjectT***)
    {
    }
  };

  STATIC_ASSERT(sizeof(ObjectT<GUnknown>) == sizeof(GUnknown*));
} // namespace GrapX

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_UNKNOWN_H_