#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "Include/GXUser.H"
#include "Include/GXKernel.H"
#include <User/ResDataMgr.H>

GrapXResData* pWndPropertyMap;

GXVOID GXUSER_InitializeWndProperty()
{
  pWndPropertyMap = GrapXResData_Create(GXRESDATA_PVOIDMAP);
}

GXVOID GXUSER_ReleaseWndProperty()
{
  GrapXResData_Destroy(pWndPropertyMap);
}

GXBOOL GXDLLAPI gxSetPropW(
  GXHWND    hWnd,     // handle of window
  GXLPCWSTR lpString, // atom or address of string
  GXHANDLE  hData     // handle of data
  )
{
  GrapXResData* pWndProp;
  if(pWndPropertyMap->GetPtr(hWnd, (GXLPVOID*)&pWndProp) == FALSE)
  {
    pWndProp = GrapXResData_Create(GXRESDATA_STRMAP);
    if(pWndProp == NULL) {
      return FALSE;
    }
    pWndPropertyMap->SetPtr(hWnd, pWndProp);
  }
  return pWndProp->SetPtr((GXLPVOID)lpString, hData);
}

GXHANDLE GXDLLAPI gxGetPropW(
  GXHWND    hWnd,     // handle of window
  GXLPCWSTR lpString  // atom or address of string
  )
{
  GrapXResData* pWndProp;
  GXLPVOID lpData;
  if(pWndPropertyMap->GetPtr( hWnd, (GXLPVOID*)&pWndProp) == FALSE)
    return NULL;
  if(pWndProp->GetPtr((GXLPVOID)lpString, &lpData) == FALSE)
    return NULL;
  return (GXHANDLE)lpData;
}

GXHANDLE GXDLLAPI gxRemovePropW(
  GXHWND    hWnd,     // handle to window
  GXLPCWSTR lpString  // atom or address of string
  )
{
  ASSERT(gxGetCurrentThreadId() == gxGetWindowThreadProcessId(hWnd, NULL));
  GrapXResData* pWndProp;
  if( ! pWndPropertyMap->GetPtr(hWnd, (GXLPVOID*)&pWndProp)) {
    return NULL;
  }
  if( ! pWndProp->Remove((GXLPVOID)lpString)) {
    return NULL;
  }
  if(pWndProp->Count() == 0) {
    GrapXResData_Destroy(pWndProp);
    pWndPropertyMap->Remove(hWnd);
  }
  return (GXHANDLE)lpString;
}

void IntRemoveAllProp(GXHWND hWnd)
{
  ASSERT(gxGetCurrentThreadId() == gxGetWindowThreadProcessId(hWnd, NULL));
  GrapXResData* pWndProp;
  if( ! pWndPropertyMap->GetPtr(hWnd, (GXLPVOID*)&pWndProp)) {
    return;
  }
  GrapXResData_Destroy(pWndProp);
  pWndPropertyMap->Remove(hWnd);
}