#ifndef _GRAPHICS_X_KERNEL_H_
#define _GRAPHICS_X_KERNEL_H_

#define GXHEAP_ZERO_MEMORY                0x00000008      

enum GXStationDesc
{
  GXSD_EXTENT        = 0,   // Station 的尺寸, GXLPARAM = GXSIZE*
  GXSD_MOUSEHOVERWND = 1,
  GXSD_ROOTDIR       = 2,
  GXSD_CONFIGPATH    = 3,   // 配置文件路径
};

enum GXInstanceType
{
  GXINSTTYPE_GRAPX = 0,
  GXINSTTYPE_APP = 1,
};

struct GXFILETIME {
  GXDWORD dwLowDateTime;
  GXDWORD dwHighDateTime;
};
typedef GXFILETIME *GXLPFILETIME;

struct GXWIN32_FIND_DATAW { // wfd  
  GXDWORD     dwFileAttributes; 
  GXFILETIME  ftCreationTime; 
  GXFILETIME  ftLastAccessTime; 
  GXFILETIME  ftLastWriteTime; 
  GXDWORD     nFileSizeHigh; 
  GXDWORD     nFileSizeLow; 
  GXDWORD     dwReserved0; 
  GXDWORD     dwReserved1; 
  GXWCHAR     cFileName[ MAX_PATH ]; 
  GXWCHAR     cAlternateFileName[ 14 ]; 
};
typedef GXWIN32_FIND_DATAW *GXLPWIN32_FIND_DATAW;
class IConsoleStaff;

//
//  Locale Enumeration Flags.
//
#define GXLCID_INSTALLED            0x00000001  // installed locale ids
#define GXLCID_SUPPORTED            0x00000002  // supported locale ids
#define GXLCID_ALTERNATE_SORTS      0x00000004  // alternate sort locale ids

/* Local Memory Flags */
#define GXLMEM_FIXED          0x0000
#define GXLMEM_MOVEABLE       0x0002
#define GXLMEM_NOCOMPACT      0x0010
#define GXLMEM_NODISCARD      0x0020
#define GXLMEM_ZEROINIT       0x0040
#define GXLMEM_MODIFY         0x0080
#define GXLMEM_DISCARDABLE    0x0F00
#define GXLMEM_VALID_FLAGS    0x0F72
#define GXLMEM_INVALID_HANDLE 0x8000

/* Global Memory Flags */
#define GXGMEM_FIXED          0x0000
#define GXGMEM_MOVEABLE       0x0002
#define GXGMEM_NOCOMPACT      0x0010
#define GXGMEM_NODISCARD      0x0020
#define GXGMEM_ZEROINIT       0x0040
#define GXGMEM_MODIFY         0x0080
#define GXGMEM_DISCARDABLE    0x0100
#define GXGMEM_NOT_BANKED     0x1000
#define GXGMEM_SHARE          0x2000
#define GXGMEM_DDESHARE       0x2000
#define GXGMEM_NOTIFY         0x4000
#define GXGMEM_LOWER          GMEM_NOT_BANKED
#define GXGMEM_VALID_FLAGS    0x7F72
#define GXGMEM_INVALID_HANDLE 0x8000


extern "C"
{
  GXBOOL    GXDLLAPI lstrncmpiW         (GXLPCWSTR, GXLPCWSTR, GXINT);
  GXLPWSTR  GXDLLAPI strchrW            (const GXWCHAR* str, int ch);

  GXHANDLE  GXDLLAPI gxGetProcessHeap   ();
  GXLPVOID  GXDLLAPI gxHeapAlloc        (GXHANDLE hHeap, GXDWORD dwFlags, GXSIZE_T dwBytes);
  GXBOOL    GXDLLAPI gxHeapFree         (GXHANDLE hHeap, GXDWORD dwFlags, GXLPVOID lpMem);
  GXLPVOID  GXDLLAPI gxHeapReAlloc      (GXHANDLE hHeap, GXDWORD dwFlags, GXLPVOID lpMem, GXSIZE_T dwBytes);
  GXSIZE_T  GXDLLAPI gxHeapSize         (GXHANDLE hHeap, GXDWORD dwFlags, GXLPCVOID lpMem);
  GXHANDLE  GXDLLAPI gxGetModuleHandle  (GXLPCWSTR lpModuleName);
  GXVOID    GXDLLAPI gxSetLastError     (GXDWORD dwErrCode);
  GXDWORD   GXDLLAPI gxGetLastError     ();
  GXINT     GXDLLAPI gxMulDiv           (GXINT nNumber, GXINT nNumerator, GXINT nDenominator);
  GXLCID    GXDLLAPI gxGetUserDefaultLCID();
  GXBOOL    GXDLLAPI gxIsValidLocale    (GXLCID Locale, GXDWORD dwFlags);

  GXHLOCAL  GXDLLAPI gxLocalAlloc       (GXIN GXUINT uFlags, GXIN GXSIZE_T uBytes);
  GXHLOCAL  GXDLLAPI gxLocalReAlloc     (GXIN GXHLOCAL hMem, GXIN GXSIZE_T uBytes, GXIN GXUINT uFlags);
  GXLPVOID  GXDLLAPI gxLocalLock        (GXIN GXHLOCAL hMem);
  GXHLOCAL  GXDLLAPI gxLocalHandle      (GXIN GXLPCVOID pMem);
  GXBOOL    GXDLLAPI gxLocalUnlock      (GXIN GXHLOCAL hMem);
  GXSIZE_T  GXDLLAPI gxLocalSize        (GXIN GXHLOCAL hMem);
  GXUINT    GXDLLAPI gxLocalFlags       (GXIN GXHLOCAL hMem);
  GXHLOCAL  GXDLLAPI gxLocalFree        (GXIN GXHLOCAL hMem);

  GXDWORD   GXDLLAPI gxGetCurrentThreadId();

  GXHRSRC   GXDLLAPI gxFindResourceA   (GXHMODULE hModule, GXLPCSTR lpName, GXLPCSTR lpType);
  GXHRSRC   GXDLLAPI gxFindResourceW   (GXHMODULE hModule, GXLPCWSTR lpName, GXLPCWSTR lpType);
  GXHGLOBAL GXDLLAPI gxLoadResource    (GXHMODULE hModule, GXHRSRC hResInfo);
  GXLPVOID  GXDLLAPI gxLockResource    (GXHGLOBAL hResData);

  GXHANDLE  GXDLLAPI gxFindFirstFileW (GXLPCWSTR lpFileName, GXLPWIN32_FIND_DATAW lpFindFileData);
  GXBOOL    GXDLLAPI gxFindNextFileW  (GXHANDLE hFindFile, GXLPWIN32_FIND_DATAW lpFindFileData);
  GXBOOL    GXDLLAPI gxFindClose      (GXHANDLE hFindFile);

  GXLONG    GXDLLAPI gxInterlockedIncrement(GXLONG volatile *Addend);
  GXLONG    GXDLLAPI gxInterlockedDecrement(GXLONG volatile *Addend);
  GXLPVOID  GXDLLAPI gxInterlockedCompareExchangePointer(GXLPVOID volatile* Destination, GXLPVOID Exchange, GXLPVOID Comperand);
  GXLONG    GXDLLAPI gxInterlockedExchange        (GXLONG volatile *Target, GXLONG Value);
  GXLONG    GXDLLAPI gxInterlockedExchangeAdd     (GXLONG volatile *Addend, GXLONG Value);
  GXLONG    GXDLLAPI gxInterlockedCompareExchange (GXLONG volatile *Destination, GXLONG Exchange, GXLONG Comperand);


  GXHINSTANCE GXDLLAPI GXGetInstance            (GXInstanceType eType);
  GXDWORD     GXDLLAPI GXUIGetStationDesc       (GXStationDesc eDesc, GXWPARAM wParam, GXLPARAM lParam);
  GXHSTATION  GXDLLAPI GXUIGetStation           ();
  GXBOOL      GXDLLAPI MOExecuteFileW           (GXLPCWSTR szFilename);
  GXBOOL      GXDLLAPI MOExecuteBatchCmdW       (GXLPCWSTR* szCommand, int nCount);
  GXBOOL      GXDLLAPI MOExecuteConsoleCmdW     (GXLPCWSTR szCommand);
  GXBOOL      GXDLLAPI MOExecuteConsoleCmdA     (GXLPCSTR szCommand);
  GXBOOL      GXDLLAPI MOGetConsoleCmdInfoA     (GXLPCSTR szCommand, int* nCmdIdx, IConsoleStaff** ppStaff);  // ppStaff need release after you never used it.
  GXHRESULT   GXDLLAPI MORegisterConsoleStaff   (IConsoleStaff* pStaff);
  GXHRESULT   GXDLLAPI MOUnregisterConsoleStaff (IConsoleStaff* pStaff);
  GXVOID      GXDLLAPI MOLogW                   (GXLPCWSTR szFormat, ...);
  GXVOID      GXDLLAPI MOLogA                   (GXLPCSTR szFormat, ...);
  GXVOID      GXDLLAPI MOLogOutputW             (GXLPCWSTR szText);
  GXVOID      GXDLLAPI MOLogOutputA             (GXLPCSTR szText);


#define gxGlobalAlloc   gxLocalAlloc
#define gxGlobalReAlloc gxLocalReAlloc
#define gxGlobalLock    gxLocalLock
#define gxGlobalUnlock  gxLocalUnlock
#define gxGlobalFree    gxLocalFree
}
#endif // _GRAPHICS_X_KERNEL_H_