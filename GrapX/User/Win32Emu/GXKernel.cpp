#include "GrapX.H"
#include "User/GrapX.Hxx"
#include "clPathFile.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"  // TODO: 只是为了包含CP_ACP, 想办法去掉这个包含
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/Platform.h"
#include "thread/clThread.h"

#include <time.h>

//extern GXSTATION g_gxStation;
extern GXLPSTATION g_pCurStation;
extern GXWCHAR wine_casemap_lower[];
extern inline GXWCHAR tolowerW( GXWCHAR ch )
{
  return ch + wine_casemap_lower[wine_casemap_lower[ch >> 8] + (ch & 0xff)];
}

int GXDLLAPI lstrncmpiW( const GXWCHAR *str1, const GXWCHAR *str2, int n )
{
  int ret = 0;
  for ( ; n > 0; n--, str1++, str2++)
    if ((ret = tolowerW(*str1) - tolowerW(*str2)) || !*str1) break;
  return ret;
}
GXLPWSTR GXDLLAPI strchrW (
               const GXWCHAR * str,
               int ch
               )
{
  while (*str && *str != (GXWCHAR)ch)
    str++;

  if (*str == (GXWCHAR)ch)
    return((GXWCHAR *)str);
  return(NULL);
}

int GXDLLAPI gxMultiByteToWideChar(
  GXUINT CodePage,          // code page 
  GXDWORD dwFlags,          // character-type options 
  GXLPCSTR lpMultiByteStr,  // address of string to map 
  int cchMultiByte,         // number of characters in string 
  GXLPWSTR lpWideCharStr,   // address of wide-character buffer 
  int cchWideChar           // size of buffer 
  )
{
  if(CodePage != GXCP_ACP || dwFlags != NULL)
    return 0;
  
  return (int)clstd::StringW_traits::XStringToNative(lpWideCharStr, cchWideChar, lpMultiByteStr, cchMultiByte);
}

int GXDLLAPI gxWideCharToMultiByte(
  GXUINT CodePage,          // code page 
  GXDWORD dwFlags,          // performance and mapping flags 
  GXLPCWSTR lpWideCharStr,  // address of wide-character string 
  int cchWideChar,          // number of characters in string 
  GXLPSTR lpMultiByteStr,   // address of buffer for new string 
  int cchMultiByte,         // size of buffer 
  GXLPCSTR lpDefaultChar,   // address of default for unmappable characters  
  GXBOOL* lpUsedDefaultChar // address of flag set when default char. used 
  )
{
  if(CodePage != GXCP_ACP || dwFlags != NULL)
    return 0;
  if(lpDefaultChar != NULL || lpUsedDefaultChar != NULL)
    return 0;

  return (int)clstd::StringA_traits::XStringToNative(lpMultiByteStr, cchMultiByte, lpWideCharStr, cchWideChar);
}

GXDWORD GXDLLAPI gxGetTickCount()
{
#if defined(_CL_SYSTEM_WINDOWS)
  return GetTickCount();
#elif defined(_CL_SYSTEM_IOS)
  clock_t t = clock();
  return (GXDWORD)(t / (CLOCKS_PER_SEC / 1000));
#else
  
  return 0;
#endif // defined(_WIN32) || defined(_WINDOWS)
}

u64 GXDLLAPI gxGetTickCount64()
{
//#if (defined(_WIN32) || defined(_WINDOWS)) && (_WIN32_WINNT >= 0x0600)
//  return GetTickCount64();
//#elif defined(_CL_SYSTEM_IOS)
//  return 0;
//  //return TickCount();
//#else
//  struct timeval current;
//  gettimeofday(&current, NULL);
//  currentTime = current.tv_sec * 1000 + current.tv_usec/1000;
//  return currentTime;
//#endif // (defined(_WIN32) || defined(_WINDOWS)) && _WIN32_WINNT >= 0x0600)
  return clstd::GetTime64();
}

static GXDWORD s_dwLastError = 0;  // TODO: 改成线程相关的
GXVOID GXDLLAPI gxSetLastError(
  GXDWORD dwErrCode   // per-thread error code  
  )
{
  s_dwLastError = dwErrCode;
}

GXDWORD GXDLLAPI gxGetLastError()
{
  return s_dwLastError;
}

//GXINT GXDLLAPI gxMulDiv( GXINT nMultiplicand, GXINT nMultiplier, GXINT nDivisor)
GXINT GXDLLAPI gxMulDiv(
  GXINT nNumber,      // 32-bit signed multiplicand  
  GXINT nNumerator,   // 32-bit signed multiplier 
  GXINT nDenominator  // 32-bit signed divisor 
  )
{
  GXLONGLONG ret;

  if (!nDenominator) return -1;

  /* We want to deal with a positive divisor to simplify the logic. */
  if (nDenominator < 0)
  {
    nNumber = - nNumber;
    nDenominator = -nDenominator;
  }

  /* If the result is positive, we "add" to round. else, we subtract to round. */
  if ( ( (nNumber <  0) && (nNumerator <  0) ) ||
    ( (nNumber >= 0) && (nNumerator >= 0) ) )
    ret = (((GXLONGLONG)nNumber * nNumerator) + (nDenominator/2)) / nDenominator;
  else
    ret = (((GXLONGLONG)nNumber * nNumerator) - (nDenominator/2)) / nDenominator;

  if ((ret > 2147483647) || (ret < -2147483647)) return -1;
  return (GXINT)ret;
}

GXLCID GXDLLAPI gxGetUserDefaultLCID()
{
  return 2; //LOCALE_SYSTEM_DEFAULT
}

GXBOOL GXDLLAPI gxIsValidLocale(
  GXLCID Locale,      // locale indentifier to validate
  GXDWORD dwFlags     // specifies validity test
  )
{
  return TRUE;
}
//u64 GXDLLAPI gxGetTickCount64()
//{
//#if (defined(_WIN32) || defined(_WINDOWS)) && (_WIN32_WINNT >= 0x0600)
//  return GetTickCount64();
//#else
//#endif // (defined(_WIN32) || defined(_WINDOWS)) && (_WIN32_WINNT >= 0x0600)
//}
//#if defined(_WIN32) || defined(_WINDOWS)
#if ! defined(_DEV_DISABLE_UI_CODE ) && ! defined(__clang__)
clStringA GXDLLAPI Win32ResourceTypeToStringA(GXLPCSTR szType)
{
  switch((GXDWORD)szType)
  {
  case (GXDWORD)GXRT_ACCELERATOR:
    return ("ACCELERATOR");
  case (GXDWORD)GXRT_ANICURSOR:
    return ("ANICURSOR");
  case (GXDWORD)GXRT_ANIICON:
    return ("ANIICON");
  case (GXDWORD)GXRT_BITMAP:
    return ("BITMAP");
  case (GXDWORD)GXRT_CURSOR:
    return ("CURSOR");
  case (GXDWORD)GXRT_DIALOG:
    return ("DIALOG");
  case (GXDWORD)GXRT_FONT:
    return ("FONT");
  case (GXDWORD)GXRT_FONTDIR:
    return ("FONTDIR");
  case (GXDWORD)GXRT_GROUP_CURSOR:
    return ("GROUP_CURSOR");
  case (GXDWORD)GXRT_GROUP_ICON:
    return ("GROUP_ICON");
  case (GXDWORD)GXRT_ICON:
    return ("ICON");
  case (GXDWORD)GXRT_MENU:
    return ("MENU");
  case (GXDWORD)GXRT_MESSAGETABLE:
    return ("MESSAGETABLE");
  case (GXDWORD)GXRT_RCDATA:
    return ("RCDATA");
  case (GXDWORD)GXRT_STRING:
    return ("STRING");
  case (GXDWORD)GXRT_VERSION:
    return ("VERSION");
  default:
    if(szType > (GXLPCSTR)0xffff)
      return szType;
  }
  clStringA strResource;
  strResource.Format(("CUSTOM_%d"), szType);
  return strResource;
}

clStringA GXDLLAPI Win32ResourceTypeNameToStringA(GXLPCSTR szType, GXLPCSTR szName)
{
  clStringA strUniqueResName = Win32ResourceTypeToStringA(szType);
  if(szName > (GXLPSTR)0xffff)
  {
    strUniqueResName.AppendFormat((".%s.bin"), szName);
  }
  else
  {
    strUniqueResName.AppendFormat((".ID_%05d.bin"), szName);
  }
  return strUniqueResName;
}
//#endif // #if defined(_WIN32) || defined(_WINDOWS)


GXHRSRC GXDLLAPI gxFindResourceA(GXHMODULE hModule, GXLPCSTR lpName, GXLPCSTR lpType)
{
  if(lpName == NULL)
    return NULL;

  if(hModule == NULL)
  {
    hModule = GXGetInstance(GXINSTTYPE_APP);
  }

  GXINSTANCE* lpModule = GXHINSTANCE_PTR(hModule);
  clStringA strName = Win32ResourceTypeNameToStringA(lpType, lpName);
  GXDWORD dwCode = strName.GetHash();
  GXINSTANCE::ResCodeDict::iterator it = lpModule->sResCodeDict.find(dwCode);
  if(it != lpModule->sResCodeDict.end())
  {
    return GXRSRC_HANDLE(it->second);
  }

  clStringA strFilename;
  clpathfile::CombinePathA(strFilename, lpModule->strModuleName, strName);
  clpathfile::CombinePathA(strFilename, "Resource", strFilename);
  clpathfile::CombinePathA(strFilename, lpModule->strRootDir, strFilename);

  clFile file;
  if(file.OpenExistingA(strFilename)) {
    GXRSRC* pRSrc = new GXRSRC(strFilename);
    lpModule->sResCodeDict[dwCode] = pRSrc;

    return GXRSRC_HANDLE(pRSrc);
  }
  return NULL;
}

GXHRSRC GXDLLAPI gxFindResourceW(
  GXHMODULE hModule,  // resource-module handle 
  GXLPCWSTR lpName,   // pointer to resource name  
  GXLPCWSTR lpType    // pointer to resource type 
  )
{
  if(lpName == NULL) {
    return NULL;
  }

  return gxFindResourceA(hModule,
    IS_IDENTIFY(lpName) ? (GXLPCSTR)lpName : (GXLPCSTR)clStringA(lpName), 
    IS_IDENTIFY(lpType) ? (GXLPCSTR)lpType : (GXLPCSTR)clStringA(lpType));

  //GXINSTANCE* lpModule = GXHINSTANCE_PTR(hModule);
  //clString strT = Win32ResourceTypeNameToString(lpType, lpName);
  //clStringA strName = (const wch*)strT;
  //GXDWORD dwCode = strName.GetHash();
  //GXINSTANCE::ResCodeDict::iterator it = lpModule->sResCodeDict.find(dwCode);
  //if(it != lpModule->sResCodeDict.end())
  //{
  //  return GXRSRC_HANDLE(it->second);
  //}

  //clStringA strFilename;
  //clpathfile::CombinePathA(strFilename, lpModule->strModuleName, strName);
  //clpathfile::CombinePathA(strFilename, "Resource", strFilename);
  //clpathfile::CombinePathA(strFilename, lpModule->strRootDir, strFilename);

  //GXRSRC* pRSrc = new GXRSRC(strFilename);
  //lpModule->sResCodeDict[dwCode] = pRSrc;
  //
  //return GXRSRC_HANDLE(pRSrc);
}

GXHGLOBAL GXDLLAPI gxLoadResource(
  GXHMODULE hModule,  // resource-module handle  
  GXHRSRC hResInfo   // resource handle 
  )
{
  GXRSRC* pRSrc = GXHRSRC_PTR(hResInfo);
  if(pRSrc->pBuffer != NULL)
  {
    return (GXHGLOBAL)pRSrc->pBuffer->GetPtr();
  }

  if(pRSrc->Load())
  {
    return (GXHGLOBAL)pRSrc->pBuffer->GetPtr();
  }
  return NULL;
}

GXLPVOID GXDLLAPI gxLockResource(
  GXHGLOBAL hResData   // handle to resource to lock 
  )
{
  return (GXLPVOID)hResData;
}
#endif // #ifndef _DEV_DISABLE_UI_CODE
//////////////////////////////////////////////////////////////////////////

GXHLOCAL GXDLLAPI gxLocalAlloc(GXIN GXUINT uFlags, GXIN GXSIZE_T uBytes)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)new GXBYTE[uBytes + sizeof(GXLOCALMEM_STRUCT)];
  lpMemory->dwMagic    = LOCALMEM_MAGIC;
  lpMemory->uSize      = uBytes;
  lpMemory->nLockCount = 0;
  
  // 只支持 LMEM_ZEROINIT, LMEM_MOVEABLE
  ASSERT((uFlags & (~(GXLMEM_ZEROINIT | GXLMEM_MOVEABLE))) == 0);

  if(TEST_FLAG(uFlags, GXLMEM_ZEROINIT))
  {
    memset(lpMemory + 1, 0, uBytes);
  }
  return GXLOCALMEM_HANDLE(lpMemory);
}

GXHLOCAL GXDLLAPI gxLocalReAlloc(GXIN GXHLOCAL hMem, GXIN GXSIZE_T uBytes, GXIN GXUINT uFlags)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)hMem;
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);

  // NOTE:
  // 只支持 LMEM_ZEROINIT 
  // 忽略 GXLMEM_MOVEABLE
  ASSERT((uFlags & (~(GXLMEM_ZEROINIT | GXLMEM_MOVEABLE))) == 0);

  if(uBytes > lpMemory->uSize)
  {
    GXLOCALMEM_STRUCT* lpNewMemory = (GXLOCALMEM_STRUCT*)gxLocalAlloc(NULL, uBytes);
    ASSERT(lpNewMemory->dwMagic == LOCALMEM_MAGIC);
    memcpy(lpNewMemory + 1, lpMemory + 1, lpMemory->uSize);
    if(TEST_FLAG(uFlags, GXLMEM_ZEROINIT))
    {
      memset(((GXLPBYTE)(lpNewMemory + 1)) + lpMemory->uSize, 0, lpNewMemory->uSize - lpMemory->uSize);
    }
    SAFE_DELETE(lpMemory);
    lpMemory = lpNewMemory;
  }
  return GXLOCALMEM_HANDLE(lpMemory);
}

GXLPVOID GXDLLAPI gxLocalLock(GXIN GXHLOCAL hMem)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)hMem;
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);
  ASSERT(lpMemory->nLockCount >= 0);
  lpMemory->nLockCount++;
  return lpMemory + 1;
}

GXHLOCAL GXDLLAPI gxLocalHandle(GXIN GXLPCVOID pMem)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)(((GXLPBYTE)pMem) - sizeof(GXLOCALMEM_STRUCT));
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);
  return GXLOCALMEM_HANDLE(lpMemory);
}

GXBOOL GXDLLAPI gxLocalUnlock(GXIN GXHLOCAL hMem)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)hMem;
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);
  lpMemory->nLockCount--;
  ASSERT(lpMemory->nLockCount >= 0);
  return lpMemory->nLockCount;
}

GXSIZE_T GXDLLAPI gxLocalSize(GXIN GXHLOCAL hMem)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)hMem;
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);
  return (GXSIZE_T)lpMemory->uSize;
}

GXUINT GXDLLAPI gxLocalFlags(GXIN GXHLOCAL hMem)
{
  return 0;
}

GXHLOCAL GXDLLAPI gxLocalFree(GXIN GXHLOCAL hMem)
{
  GXLOCALMEM_STRUCT* lpMemory = (GXLOCALMEM_STRUCT*)hMem;
  ASSERT(lpMemory->dwMagic == LOCALMEM_MAGIC);
  delete lpMemory;
  return NULL;
}

#if defined(_WIN32) && defined(_X86) && ! defined(__clang__)
__declspec(naked) GXDWORD GXDLLAPI gxGetCurrentThreadId()
{
  __asm mov eax, FS:[0x18]    // TEB
  __asm mov eax, [eax + 0x24] // ThreadId
  __asm ret
}
#else
GXDWORD GXDLLAPI gxGetCurrentThreadId()
{
  return (GXDWORD)clstd::thread::GetCurrentId();
}
#endif // #if defined(_WIN32) && defined(_X86)

GXDWORD GXDLLAPI GXUIGetStationDesc(GXStationDesc eDesc, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  switch(eDesc)
  {
  case GXSD_EXTENT:
    {
      ASSERT(wParam == 0);
      GXSIZE* pSize = (GXSIZE*)lParam;
      pSize->cx = lpStation->nWidth;
      pSize->cy = lpStation->nHeight;
      return TRUE;
    }
  case GXSD_MOUSEHOVERWND:
    {
      ASSERT(wParam == 0);
      GXHWND hWnd = GXWND_HANDLE(lpStation->m_pMouseFocus);
      if(lParam != NULL) {
        *(GXHWND*)lParam = hWnd;
      }
      return hWnd != NULL;
    }
  case GXSD_ROOTDIR:
    {
      GXSTRCPYN<GXWCHAR>((GXLPWSTR)lParam, lpStation->lpPlatform->GetRootDir(), wParam);
    }
    return TRUE;

  case GXSD_CONFIGPATH:
    {
      clStringW strCfgPath;
      GetModuleFileNameW(NULL, strCfgPath.GetBuffer(MAX_PATH), MAX_PATH);
      strCfgPath.ReleaseBuffer();

      clpathfile::RenameExtensionW(strCfgPath, L".config");
      GXSTRCPYN<GXWCHAR>((GXLPWSTR)lParam, (GXLPCWSTR)strCfgPath, wParam);

      if(strCfgPath.GetLength() > wParam) {
        return (GXDWORD)wParam;
      }
      else {
        return (GXDWORD)strCfgPath.GetLength();
      }
    }
    break; // 运行不到这里
  }
  return FALSE;
}

GXHINSTANCE GXDLLAPI GXGetInstance(GXInstanceType eType)
{
  //GXLPSTATION lpStation = &g_gxStation;
  if(eType == GXINSTTYPE_GRAPX)
  {
    return GXINSTANCE_HANDLE(g_pCurStation->pInstDll);
  }
  else if(eType == GXINSTTYPE_APP)
  {
    return GXINSTANCE_HANDLE(g_pCurStation->pInstApp);
  }
  else
  {
    return NULL;
  }
}

GXLPSTATION IntGetStationPtr()
{
  return g_pCurStation;
}

GXHSTATION GXDLLAPI GXUIGetStation()
{
  return GXSTATION_HANDLE(g_pCurStation);
}

#if defined(_WIN32) || defined(_WINDOWS)
GXHANDLE GXDLLAPI gxGetProcessHeap()
{
  return (GXHANDLE)GetProcessHeap();
}

GXLPVOID GXDLLAPI gxHeapAlloc(
  GXHANDLE hHeap,  // handle to the private heap block 
  GXDWORD dwFlags,  // heap allocation control flags 
  GXSIZE_T dwBytes   // number of bytes to allocate 
  )
{
  return HeapAlloc(hHeap, dwFlags, dwBytes);
}

GXBOOL GXDLLAPI gxHeapFree(
  GXHANDLE hHeap,  // handle to the heap 
  GXDWORD dwFlags,  // heap freeing flags 
  GXLPVOID lpMem   // pointer to the memory to free 
  )
{
  return HeapFree(hHeap, dwFlags, lpMem);
}

GXLPVOID GXDLLAPI gxHeapReAlloc(
  GXHANDLE hHeap,     // handle to a heap block 
  GXDWORD dwFlags,    // heap reallocation flags 
  GXLPVOID lpMem,     // pointer to the memory to reallocate 
  GXSIZE_T dwBytes     // number of bytes to reallocate 
  )
{
  return HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
}

GXSIZE_T GXDLLAPI gxHeapSize(
  GXHANDLE hHeap,      // handle to the heap 
  GXDWORD dwFlags,    // heap size control flags 
  GXLPCVOID lpMem      // pointer to memory to return size for  
  )
{
  return HeapSize(hHeap, dwFlags, lpMem);
}

GXHANDLE GXDLLAPI gxGetModuleHandle(
  GXLPCWSTR lpModuleName   // address of module name to return handle for  
  )
{
  return (GXHANDLE)GetModuleHandleW(lpModuleName);
}

GXLONG GXDLLAPI gxInterlockedIncrement(GXLONG volatile *Addend)
{
  return InterlockedIncrement(Addend);
}

GXLONG GXDLLAPI gxInterlockedDecrement(GXLONG volatile *Addend)
{
  return InterlockedDecrement(Addend);
}

GXLPVOID GXDLLAPI gxInterlockedCompareExchangePointer(
  GXLPVOID volatile* Destination,
  GXLPVOID Exchange,
  GXLPVOID Comperand
  )
{
  return InterlockedCompareExchangePointer(Destination, Exchange, Comperand);
}

GXHANDLE GXDLLAPI gxFindFirstFileW(
  GXLPCTSTR lpFileName,  // pointer to name of file to search for  
  GXLPWIN32_FIND_DATAW lpFindFileData   // pointer to returned information 
  )
{
  ASSERT(sizeof(WIN32_FIND_DATAW) == sizeof(GXWIN32_FIND_DATAW));
  return (GXHANDLE)FindFirstFileW(lpFileName, (WIN32_FIND_DATAW*)lpFindFileData);
}
GXBOOL GXDLLAPI gxFindNextFileW(
  GXHANDLE hFindFile,  // handle to search  
  GXLPWIN32_FIND_DATAW lpFindFileData   // pointer to structure for data on found file  
  )
{
  ASSERT(sizeof(WIN32_FIND_DATA) == sizeof(GXWIN32_FIND_DATAW));
  return FindNextFileW(hFindFile, (WIN32_FIND_DATAW*)lpFindFileData);
}
GXBOOL GXDLLAPI gxFindClose(
  GXHANDLE hFindFile   // file search handle 
  )
{
  return FindClose(hFindFile);
}

#else // defined(_WIN32) || defined(_WINDOWS)
GXHANDLE GXDLLAPI gxGetProcessHeap()
{
  return NULL;
}

GXLPVOID GXDLLAPI gxHeapAlloc(
  GXHANDLE hHeap,  // handle to the private heap block 
  GXDWORD dwFlags,  // heap allocation control flags 
  GXDWORD dwBytes   // number of bytes to allocate 
  )
{
  return gxLocalAlloc(NULL, dwBytes);
  //GXLPVOID lpMem = malloc(dwBytes);

  //if(TEST_FLAG(dwFlags, GXHEAP_ZERO_MEMORY))
  //  ZeroMemory(lpMem, dwBytes);

  //return lpMem;
}

GXBOOL GXDLLAPI gxHeapFree(
  GXHANDLE hHeap,  // handle to the heap 
  GXDWORD dwFlags,  // heap freeing flags 
  GXLPVOID lpMem   // pointer to the memory to free 
  )
{
  //free(lpMem);
  gxLocalFree((GXHLOCAL)lpMem);
  return TRUE;
}

GXLPVOID GXDLLAPI gxHeapReAlloc(
  GXHANDLE hHeap,     // handle to a heap block 
  GXDWORD dwFlags,    // heap reallocation flags 
  GXLPVOID lpMem,     // pointer to the memory to reallocate 
  GXDWORD dwBytes     // number of bytes to reallocate 
  )
{
  return gxLocalReAlloc((GXHLOCAL)lpMem, dwBytes, NULL);
  //(<#GXHLOCAL hMem#>, <#GXSIZE_T uBytes#>, <#GXUINT uFlags#>)(lpMem, dwBytes);
}

GXDWORD GXDLLAPI gxHeapSize(
  GXHANDLE hHeap,      // handle to the heap 
  GXDWORD dwFlags,    // heap size control flags 
  GXLPCVOID lpMem      // pointer to memory to return size for  
  )
{
  return gxLocalSize((GXHLOCAL)lpMem);
  //return msize(lpMem);
}

GXHANDLE GXDLLAPI gxGetModuleHandle(
  GXLPCWSTR lpModuleName   // address of module name to return handle for  
  )
{
  return NULL;
}

GXLONG GXDLLAPI gxInterlockedIncrement(GXLONG volatile *Addend)
{
  return Addend++;
}

GXLONG GXDLLAPI gxInterlockedDecrement(GXLONG volatile *Addend)
{
  return Addend--;
}

GXLPVOID GXDLLAPI gxInterlockedCompareExchangePointer(
  GXLPVOID volatile* Destination,
  GXLPVOID Exchange,
  GXLPVOID Comperand
  )
{

}
GXHANDLE GXDLLAPI gxFindFirstFile(
  GXLPCTSTR lpFileName,  // pointer to name of file to search for  
  GXLPWIN32_FIND_DATAW lpFindFileData   // pointer to returned information 
  )
{

}
GXBOOL GXDLLAPI gxFindNextFile(
  GXHANDLE hFindFile,  // handle to search  
  GXLPWIN32_FIND_DATAW lpFindFileData   // pointer to structure for data on found file  
  )
{

}
GXBOOL GXDLLAPI gxFindClose(
  GXHANDLE hFindFile   // file search handle 
  )
{

}
#endif // defined(_WIN32) || defined(_WINDOWS)

GXLONG GXDLLAPI gxInterlockedExchange(GXLONG volatile *Target, GXLONG Value)
{
  return clstd::InterlockedExchange(Target, Value);
}

GXLONG GXDLLAPI gxInterlockedExchangeAdd(GXLONG volatile *Addend, GXLONG Value)
{
  return clstd::InterlockedExchangeAdd(Addend, Value);
}

GXLONG GXDLLAPI gxInterlockedCompareExchange(GXLONG volatile *Destination, GXLONG Exchange, GXLONG Comperand)
{
  return clstd::InterlockedCompareExchange(Destination, Exchange, Comperand);
}

//////////////////////////////////////////////////////////////////////////
//
// GUnknown Implement
//
GXHRESULT GUnknown::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GUnknown::Release()
{
  const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0) {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
