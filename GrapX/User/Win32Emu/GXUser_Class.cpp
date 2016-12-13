#ifndef _DEV_DISABLE_UI_CODE
#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "GrapX/GXUser.H"
#include <GXStation.H>
#include <User/GXWindow.h>
#include "_dpa.H"

extern GXLPSTATION g_pCurStation;
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

GXVOID ClsAtomToWndCls(LPGXWNDCLASSEX lpWndCls, GXCONST LPGXWNDCLSATOM lpClsAtom)
{
  lpWndCls->cbSize        = sizeof(GXWNDCLASSEX);
  lpWndCls->style         = lpClsAtom->style;
  lpWndCls->lpfnWndProc   = lpClsAtom->lpfnWndProc;
  lpWndCls->cbClsExtra    = lpClsAtom->cbClsExtra;
  lpWndCls->cbWndExtra    = lpClsAtom->cbWndExtra;
  lpWndCls->hInstance     = NULL;
  lpWndCls->hIcon         = lpClsAtom->hIcon;
  lpWndCls->hCursor       = lpClsAtom->hCursor;
  lpWndCls->hbrBackground = lpClsAtom->hbrBackground;
  if(lpClsAtom->szMenuName[0] == NULL)
    lpWndCls->lpszMenuName = (GXLPCWSTR)(*(GXDWORD_PTR*)&lpClsAtom->szMenuName[4]);
  else
    lpWndCls->lpszMenuName = lpClsAtom->szMenuName;
  lpWndCls->lpszClassName = lpClsAtom->szClassName;
  lpWndCls->hIconSm = NULL;
  ASSERT(lpClsAtom->nRefCount >= 0);
}

GXVOID WndClsToClsAtom(LPGXWNDCLSATOM lpClsAtom, GXCONST LPGXWNDCLASSEX lpWndCls)
{
  lpClsAtom->nRefCount = 0;
  lpClsAtom->style = lpWndCls->style;
  lpClsAtom->lpfnWndProc = lpWndCls->lpfnWndProc;
  lpClsAtom->cbClsExtra = lpWndCls->cbClsExtra; 
  lpClsAtom->cbWndExtra = lpWndCls->cbWndExtra; 

  lpClsAtom->hStation = GXSTATION_HANDLE(g_pCurStation);
  lpClsAtom->hIcon = lpWndCls->hIcon; 
  lpClsAtom->hCursor = lpWndCls->hCursor; 
  lpClsAtom->hbrBackground = lpWndCls->hbrBackground;
  if(IS_IDENTIFY(lpWndCls->lpszMenuName))
    *(GXDWORD_PTR*)&lpClsAtom->szMenuName[4] = (GXDWORD_PTR)lpWndCls->lpszMenuName;
  else
    GXSTRCPY(lpClsAtom->szMenuName, lpWndCls->lpszMenuName);
  GXSTRCPY(lpClsAtom->szClassName, lpWndCls->lpszClassName); 
  ASSERT(GXSTRLEN(lpClsAtom->szClassName) > 0);
}
//////////////////////////////////////////////////////////////////////////
int GXDLLAPI gxGetClassNameW(
  GXHWND    hWnd,        // handle of window
  GXLPWSTR  lpClassName, // address of buffer for class name
  int       nMaxCount    // size of buffer, in characters
  )
{
  if(hWnd == NULL) {
    return 0;
  }

  if(nMaxCount > 0) {
    GXLPWND lpWnd = GXWND_PTR(hWnd);
    GXSTRCPYN(lpClassName, lpWnd->m_lpClsAtom->szClassName, nMaxCount);
    return (int)GXSTRLEN(lpClassName);
  }

  return 0;
}


GXDWORD GXDLLAPI gxRegisterClassExW(
                 GXCONST GXWNDCLASSEX *lpwcx  // address of structure with class data
                 )
{
  if(gxGetClassInfoExW(NULL, lpwcx->lpszClassName, NULL) != NULL)
  {
    return NULL;
  }
  
  GXLPWNDCLSATOM lpWndClsAtom = (GXLPWNDCLSATOM)new GXBYTE[sizeof(GXWNDCLSATOM) + lpwcx->cbClsExtra];
  WndClsToClsAtom(lpWndClsAtom, (GXCONST LPGXWNDCLASSEX)lpwcx);

  if(gxDPA_InsertPtr(g_pCurStation->hClassDPA, 0, lpWndClsAtom) == -1)
  {
    SAFE_DELETE_ARRAY(lpWndClsAtom);
    return NULL;
  }

  return (GXDWORD)(GXDWORD_PTR)lpWndClsAtom;
}

int GXCALLBACK DPA_GetClassInfoExW_CallBack(void *p1, void *p2, GXLPARAM lParam)
{
  return GXSTRCMPI(((GXLPWNDCLSATOM)p1)->szClassName,((GXLPWNDCLSATOM)p2)->szClassName);
}

GXBOOL GXDLLAPI gxUnregisterClassW(
                GXLPCWSTR lpClassName,  // address of class name string
                GXHINSTANCE hInstance   // handle of application instance
                )
{
  ASSERT(hInstance == NULL);

  //GXLPWNDCLASSEX lpWndCls;
  GXWNDCLSATOM FindClsAtom;
  //FindClsInfo.lpszClassName = lpClassName;
  GXSTRCPY(FindClsAtom.szClassName, lpClassName);
  int nIndex = gxDPA_Search(g_pCurStation->hClassDPA, &FindClsAtom, 0, DPA_GetClassInfoExW_CallBack, NULL, NULL);
  if(nIndex == -1)
    return FALSE;
  LPGXWNDCLSATOM lpClsAtom;
  lpClsAtom = (LPGXWNDCLSATOM)gxDPA_GetPtr(g_pCurStation->hClassDPA, nIndex);
  if(lpClsAtom->nRefCount == 0)
  {
    SAFE_DELETE(lpClsAtom);
    gxDPA_DeletePtr(g_pCurStation->hClassDPA, nIndex);
    return TRUE;
  }

  CLBREAK;  // 不能删除还在使用的类
  return FALSE;
}

GXDWORD GXDLLAPI gxGetClassInfoExW(
                GXHINSTANCE hinst,  // handle of application instance
                GXLPCWSTR lpszClass,  // address of class name string
                GXLPWNDCLASSEX lpwcx  // address of structure for class data
                )
{
  ASSERT(hinst == NULL);
  //GXLPWNDCLASSEX lpWndClsInfo;
  GXWNDCLSATOM FindClsAtom;
  GXSTRCPY(FindClsAtom.szClassName, lpszClass);
  int nIndex = gxDPA_Search(g_pCurStation->hClassDPA, &FindClsAtom, 0, DPA_GetClassInfoExW_CallBack, NULL, NULL);
  if(nIndex == -1)
    return (GXDWORD)NULL;
  LPGXWNDCLSATOM lpClsAtom = (LPGXWNDCLSATOM)gxDPA_GetPtr(g_pCurStation->hClassDPA, nIndex);
  if(lpwcx != NULL)
  {
    ClsAtomToWndCls(lpwcx, lpClsAtom);
    //    *lpwcx = *(GXLPWNDCLASSEX);
  }
  return (GXDWORD)(GXDWORD_PTR)lpClsAtom;
  //if(GrapXResData_GetPtr(hControlRegMap, (GXLPVOID)lpszClass, (GXLPVOID*)&lpWndClsInfo) == FALSE)
  //  return FALSE;
  //*lpwcx = *lpWndClsInfo;
  //return TRUE;
  //GrapXControlRegMap::iterator&it = pControlRegMap->find(lpszClass);
  //if(it == pControlRegMap->end())
  //  return FALSE;

  //if(lpwcx != NULL)
  //  *lpwcx = it->second;
  //return TRUE;
}
//GXDWORD GXDLLAPI GetClassLong(
//           GXHWND hWnd,  // handle of window
//           int nIndex   // offset of value to retrieve 
//           )
//{
//  if(nIndex >= 0)
//  {
//    LPGXWNDCLSATOM lpWndClsAtom = GXWND_PTR(hWnd)->m_lpClsAtom;
//    ASSERT(lpWndClsAtom->cbClsExtra >= nIndex + sizeof(GXDWORD));
//    return ((GXDWORD*)(((GXBYTE*)lpWndClsAtom) + sizeof(GXWNDCLSATOM)))[nIndex];
//  }
//  // TODO: 未支持
//  ASSERT(FALSE);
//}
//GXDWORD GXDLLAPI gxSetClassLong(
//           GXHWND hWnd,  // handle of window
//           int nIndex,  // index of value to change
//           GXLONG dwNewLong   // new value
//           )
//{
//  switch(nIndex)
//  {
//  case GCL_WNDPROC:
//
//  }
//}
#ifdef __cplusplus
};  // extern "C"
#endif // __cplusplus
#endif // _DEV_DISABLE_UI_CODE