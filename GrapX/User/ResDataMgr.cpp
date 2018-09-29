#include <GrapX.h>
#include <User/ResDataMgr.h>

//
// 为毛还写过一个这么复杂且没有测试的东东？
//

//////////////////////////////////////////////////////////////////////////
class GrapXResData_WStrMap : public GrapXResData
{
private:
  typedef std::map<clStringW, void*> GXRD_Map;
  GXRD_Map* m_pMap;
public:
  GXBOOL SetPtr(GXLPVOID ptr, GXLPVOID pHandle)
  {
    (*m_pMap)[((GXWCHAR*)ptr)] = pHandle;
    return TRUE;
  }
  GXBOOL GetPtr(GXLPVOID ptr, GXLPVOID* ppHandle)
  {
    GXRD_Map::iterator it = m_pMap->find((GXWCHAR*)ptr);
    if(it != m_pMap->end())
    {
      *ppHandle = it->second;
      return TRUE;
    }
    return FALSE;
  }
  GXBOOL Remove(GXLPVOID ptr)
  {
    GXRD_Map::iterator it = m_pMap->find((GXWCHAR*)ptr);
    if(it != m_pMap->end())
    {
      m_pMap->erase(it);
      return TRUE;
    }
    return FALSE;
  }
  GrapXResData_WStrMap()
  {
    m_pMap = new GXRD_Map;
  }
  GXVOID Release()
  {
    SAFE_DELETE(m_pMap);
  }

  GXSIZE_T Count() const
  {
    return m_pMap->size();
  }
};
//////////////////////////////////////////////////////////////////////////
class GrapXResData_PVoidMap : public GrapXResData
{
private:
  typedef std::map<void*, void*> GXRD_Map;
  GXRD_Map* m_pMap;
public:
  GXBOOL SetPtr(GXLPVOID ptr, GXLPVOID pHandle)
  {
    (*m_pMap)[ptr] = pHandle;
    return TRUE;
  }
  GXBOOL GetPtr(GXLPVOID ptr, GXLPVOID* ppHandle)
  {
    GXRD_Map::iterator it = m_pMap->find(ptr);
    if(it != m_pMap->end())
    {
      *ppHandle = it->second;
      return TRUE;
    }
    return FALSE;
  }
  GXBOOL Remove(GXLPVOID ptr)
  {
    GXRD_Map::iterator it = m_pMap->find(ptr);
    if(it != m_pMap->end())
    {
      m_pMap->erase(it);
      return TRUE;
    }
    return FALSE;
  }
  GrapXResData_PVoidMap()
  {
    m_pMap = new GXRD_Map;
  }
  GXVOID Release()
  {
    SAFE_DELETE(m_pMap);
  }

  GXSIZE_T Count() const
  {
    return m_pMap->size();
  }
};
//////////////////////////////////////////////////////////////////////////
GrapXResData* GrapXResData_Create(GXUINT uType)
{
  GrapXResData* pResData = NULL;
  if(uType == GXRESDATA_STRMAP)
    pResData = new GrapXResData_WStrMap;
  else if(uType == GXRESDATA_PVOIDMAP)
    pResData = new GrapXResData_PVoidMap;

  return pResData;
}

GXVOID GrapXResData_Destroy(GrapXResData* pResData)
{
  if(pResData != NULL)
  {
    ((GrapXResData*)pResData)->Release();
    SAFE_DELETE(pResData);
  }
}
//
//GXBOOL GrapXResData_SetPtr(HGXRESDATA hResData, GXLPVOID ptr, GXLPVOID pHandle)
//{
//  return ((GrapXResData*)hResData)->SetPtr(ptr, pHandle);
//}
//GXBOOL GrapXResData_GetPtr(HGXRESDATA hResData, GXLPVOID ptr, GXLPVOID* ppHandle)
//{
//  if(hResData != NULL)
//    return ((GrapXResData*)hResData)->GetPtr(ptr, ppHandle);
//  return FALSE;
//}
//
//GXBOOL GrapXResData_Remove(HGXRESDATA hResData, GXLPVOID ptr)
//{
//  return ((GrapXResData*)hResData)->Remove(ptr);
//}
//
//GXSIZE_T GrapXResData_Count(HGXRESDATA hResData)
//{
//  return ((GrapXResData*)hResData)->Count();
//}