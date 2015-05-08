#ifndef _TEST_OBJECT_H_
#define _TEST_OBJECT_H_

//
// 用来测试DataPool使用GUnknown对象的测试对象
//
class TestObject : public GUnknown
{
public:
  GXHRESULT AddRef  ()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT Release()
  {
    const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
  
  static GXHRESULT Create(TestObject** ppObject)
  {
    TestObject* pObject = new TestObject;
    if( ! InlCheckNewAndIncReference(pObject)) {
      return GX_ERROR_OUROFMEMORY;
    }

    *ppObject = pObject;
    return GX_OK;
  }
};

#endif // _TEST_OBJECT_H_