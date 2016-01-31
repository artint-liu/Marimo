#ifndef _GRAPH_UNKNOWN_H_
#define _GRAPH_UNKNOWN_H_

#pragma warning(disable : 4275)
#pragma warning(disable : 4251)

#define GXSUCCEEDED(r)      (((GXHRESULT)(r)) >= 0)
#define GXFAILED(r)         (!GXSUCCEEDED(r))
#define GXSTDINTERFACE(x)  virtual x = 0

// 虚函数化的AddRef和Release
#define ENABLE_VIRTUALIZE_ADDREF_RELEASE

class GUnknown
{
protected:
  GUnknown() : m_uRefCount(0)
  {
#ifdef _X86
# ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    STATIC_ASSERT(sizeof(GUnknown) == 8);
# else
    STATIC_ASSERT(sizeof(GUnknown) == 4);
# endif
#elif _X64
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

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_UNKNOWN_H_