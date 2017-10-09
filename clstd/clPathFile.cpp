#include "clstd.h"

#if defined(_CL_SYSTEM_WINDOWS)
#include <Windows.h>
#include <Shlwapi.h>
#include <direct.h>
#pragma comment(lib, "shlwapi.lib")
#elif defined(_CL_SYSTEM_LINUX)
# include <sys/stat.h> 
# include <unistd.h>
#endif // #if defined(_CL_SYSTEM_WINDOWS)

#include "clString.h"
#include "clPathFile.h"
using namespace clstd;

#define CLMAKE64FROM32(_LOW32, _HIGH32)  ((((u64)_HIGH32) << 32) | _LOW32)

//template class clStringX<wch, g_Alloc_clStringW, clstd::StringW_traits>;
//template class clStringX<ch, g_Alloc_clStringA, clStringA_traits>;

//template b32 _GetFileExtensionT(const clStringA& strFile, clStringA* pstrFilename, clStringA* pstrExtension);
//template b32 _GetFileExtensionT(const clStringW& strFile, clStringW* pstrFilename, clStringW* pstrExtension);

template b32 _SplitPathT(const clStringA& strPath, clStringA* pstrDir, clStringA* pstrFile);
template b32 _SplitPathT(const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile);

extern clStringW s_strRootDir;

template<typename _TString>
b32 _SplitPathT(const _TString& strPath, _TString* pstrDir, _TString* pstrFile)
{
  size_t nPos = strPath.ReverseFind('\\');
  if(nPos == _TString::npos)
  {
    if(pstrDir != NULL)
      pstrDir->Clear();
    if(pstrFile != NULL)
      (*pstrFile) = strPath;
    return FALSE;
  }
  else if(nPos == strPath.GetLength() - 1)
  {
    ASSERT(0); // 验证后去掉
    if(pstrDir != NULL)
      (*pstrDir) = strPath;
    if(pstrFile != NULL)
      pstrFile->Clear();
    return FALSE;
  }
  else
  {
    if(pstrDir != NULL)
      (*pstrDir) = strPath.SubString(0, nPos + 1);
    if(pstrFile != NULL)
      (*pstrFile) = strPath.SubString(nPos + 1, strPath.GetLength());

  }
  return TRUE;
}

b32 SplitPath(const clStringA& strPath, clStringA* pstrDir, clStringA* pstrFile)
{
  return _SplitPathT(strPath, pstrDir, pstrFile);
}

b32 SplitPath(const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile)
{
  return _SplitPathT(strPath, pstrDir, pstrFile);
}

template<typename _TString>
b32 IsFullPathT(const _TString& strFilename)
{
  typename _TString::TChar c = strFilename[0];
  if(c == '\\' || c == '/')
    return TRUE;
  else if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
  {
    if(strFilename[1] == ':' && strFilename[2] == '\\')
      return TRUE;
  }
  return FALSE;
}

b32 IsFullPath(const clStringA& strFilename)
{
  return IsFullPathT(strFilename);
}
b32 IsFullPath(const clStringW& strFilename)
{
  return IsFullPathT(strFilename);
}

//void LocateRootDir()
//{
//#if defined(_WINDOWS) || defined(_WIN32)
//  tch szModulePath[MAX_PATH];
//  clString strDir;
//  clString strFile;
//  GetModuleFileName(NULL, szModulePath, MAX_PATH);
//  SplitPath(clString(szModulePath), &strDir, &strFile);
//  s_strRootDir = clpathfile::CanonicalizeT(strDir);
//#endif // _WINDOWS
//}

void ResetWorkingDir()
{
#if defined(_WINDOWS) || defined(_WIN32)
  wch szModulePath[MAX_PATH];
  clStringW strDir;
  clStringW strFile;
  GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
  SplitPath(clStringW(szModulePath), &strDir, &strFile);
  s_strRootDir = clpathfile::CanonicalizeT(strDir);
  SetCurrentDirectoryW(strDir);
#endif // _WINDOWS
}

//template<typename _TString>
//b32 _GetFileExtensionT(const _TString& strFile, _TString* pstrFilename, _TString* pstrExtension)
//{
//  // NSString *path = [[NSBundle mainBundle] bundlePath];
//  size_t nPos = strFile.ReverseFind(_T('.'));
//  if(nPos == _TString::npos)
//  {
//    if(pstrFilename != NULL)
//      pstrFilename->Clear();
//    if(pstrExtension != NULL)
//      (*pstrExtension) = strFile;
//    return FALSE;
//  }
//  else if(nPos == strFile.GetLength() - 1)
//  {
//    ASSERT(0); // 验证后去掉
//    if(pstrFilename != NULL)
//      (*pstrFilename) = strFile;
//    if(pstrExtension != NULL)
//      pstrExtension->Clear();
//    return FALSE;
//  }
//  else
//  {
//    if(pstrFilename != NULL)
//      (*pstrFilename) = strFile.SubString(0, nPos + 1);
//    if(pstrExtension != NULL)
//      (*pstrExtension) = strFile.SubString(nPos + 1, strFile.GetLength());
//  }
//  return TRUE;
//}
//
//b32 GetFileExtensionA(const clStringA& strFile, clStringA* pstrFilename, clStringA* pstrExtension)
//{
//  return _GetFileExtensionT(strFile, pstrFilename, pstrExtension);
//}
//b32 GetFileExtensionW(const clStringW& strFile, clStringW* pstrFilename, clStringW* pstrExtension)
//{
//  return _GetFileExtensionT(strFile, pstrFilename, pstrExtension);
//}

namespace clpathfile
{
#if defined(_WIN32) || defined(_WINDOWS)
  i16 s_PathSlash = '\\';
  i16 s_VicePathSlash = '/';
#else
  i16 s_PathSlash = '/';
  i16 s_VicePathSlash = '\\';
#endif // defined(_WIN32) || defined(_WINDOWS)

#define IS_ANY_SLASH(_CH) (_CH == s_PathSlash || _CH == s_VicePathSlash)

  template<typename _TString>
  b32 RenameExtensionT(_TString& strPath, typename _TString::LPCSTR szExt)
  {
    size_t p = FindExtensionT<typename _TString::TChar>(strPath);
    if(p == _TString::npos) {
      if(szExt[0] != '.') {
        strPath.Append('.');
      }
      strPath.Append(szExt);
    }
    else if(szExt) {
      strPath.Replace(p + 1, -1, 
        (typename _TString::LPCSTR)((szExt[0] == '.') ? (szExt + 1) : szExt));
    }
    else {
      strPath.Replace(p, -1, szExt);
    }
    return TRUE;
  }

  b32 RenameExtension(clStringA& strPath, clStringA::LPCSTR  szExt)
  {
    return RenameExtensionT(strPath, szExt);
  }
  b32 RenameExtension(clStringW& strPath, clStringW::LPCSTR  szExt)
  {
    return RenameExtensionT(strPath, szExt);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString>
  clsize FindFileNameT(const _TString& strPath)
  {
    size_t p = strPath.ReverseFind((typename _TString::TChar)s_PathSlash);
    if(p == _TString::npos) {
      return 0;
    }
    else if(p == strPath.GetLength() - 1) {
      p = strPath.ReverseFind((typename _TString::TChar)s_PathSlash);
      if(p == _TString::npos) {
        return 0;
      }
    }
    return (p + 1);
  }

  clsize FindFileName(const clStringA& strPath)
  {
    return FindFileNameT(strPath);
  }
  
  clsize FindFileName(const clStringW& strPath)
  {
    return FindFileNameT(strPath);
  }

  clsize FindFileName(const  ch* szPath)
  {
    return FindFileNameT<clStringA>(szPath);
  }

  clsize FindFileName(const wch* szPath)
  {
    return FindFileNameT<clStringW>(szPath);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TCh>
  clsize FindExtensionT(const _TCh* szPath)
  {
    clsize i = strlenT(szPath);
    while(--i != (clsize)-1) {
      if(szPath[i] == '.') {
        return i;
      }
      else if(IS_ANY_SLASH(szPath[i])) {
        break;
      }
    }
    return -1;
  }

  clsize FindExtension(const ch* szPath)
  {
    return FindExtensionT(szPath);
  }
  
  clsize FindExtension(const wch* szPath)
  {
    return FindExtensionT(szPath);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TCh>
  b32 CompareExtensionT(const _TCh* szPath, const _TCh* szExtList)
  {
    clsize pos = FindExtensionT(szPath);
    if(pos == (clsize)-1) {
      return FALSE;
    }

    szPath += (pos + 1);

    // 与 StringUtility::Resolve 实现基本一直，只是在匹配到符合的字符串时直接返回
    const _TCh* str_begin = szExtList;
    const _TCh* str_end   = szExtList + clstd::strlenT(szExtList);

    while(szExtList < str_end) {
      if(*szExtList == '|') {
        if(clstd::strncmpT(str_begin, szPath, szExtList - str_begin) == 0) {
          return TRUE;
        }
        str_begin = ++szExtList;
        continue;
      }
      szExtList++;
    }

    return (clstd::strncmpT(str_begin, szPath, szExtList - str_begin) == 0);
  }

  b32 CompareExtension(const ch* szPath, const ch* szExtList)
  {
    return CompareExtensionT(szPath, szExtList);
  }

  b32 CompareExtension(const wch* szPath, const wch* szExtList)
  {
    return CompareExtensionT(szPath, szExtList);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString>
  b32 AddSlashT(_TString& strPath)
  {
    strPath.Replace((typename _TString::TChar)s_VicePathSlash, (typename _TString::TChar)s_PathSlash, 0);
    if(strPath.Back() == (typename _TString::TChar)s_PathSlash) {
      return FALSE;
    }
    strPath.Append((typename _TString::TChar)s_PathSlash);
    return TRUE;
  }

  b32 AddSlash(clStringA& strPath)
  {
    return AddSlashT(strPath);
  }

  b32 AddSlash(clStringW& strPath)
  {
    return AddSlashT(strPath);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString>
  _TString& CombinePathT(_TString& strDestPath, typename _TString::LPCSTR szDir, typename _TString::LPCSTR szFile)
  {
    if( ! szDir && ! szFile) {
      strDestPath.Clear();
      return strDestPath;
    }
    else if( ! szFile) {
      strDestPath = szDir;

      if(strDestPath.IsNotEmpty() &&
        strDestPath.Back() != (typename _TString::TChar)s_PathSlash &&
        strDestPath.Back() != (typename _TString::TChar)s_VicePathSlash) {
          strDestPath.Append((typename _TString::TChar)s_PathSlash);
      }
      return strDestPath;
    }
    
    static typename _TString::TChar s_aFindList[] = {'\\', '/', 0};
    _TString strTempPath;
    if(IsRelativeT(szFile))
    {
      strTempPath = szDir;
      strTempPath.TrimRight((typename _TString::TChar)s_PathSlash);
      strTempPath.TrimRight((typename _TString::TChar)s_VicePathSlash);

      while(szFile[0] != '\0') {
        switch(szFile[0])
        {
        case '.':
          if(szFile[1] == '.') { // "..*"
            size_t pos = strTempPath.ReverseFindAny(s_aFindList);
            if(pos != _TString::npos) {
              strTempPath.Remove(pos, -1);
            }
            else if(strTempPath.IsNotEmpty()) {
              strTempPath.Clear();
            }
            else {
              break;
            }
            szFile += 2;
            continue;
          }
          else { // ".*"
            szFile += 1;
            continue;
          }
          break;

        case '\\': // "\\*"
        case '/':  // "/*"
          {
            szFile++;
            continue;
          }
        }
        break;
      }
    }
#if defined(_WIN32) || defined(_WINDOWS)
    else {
      // 如果是 szFile 没有指定盘符，szDir指定了盘符，就复制这个盘符路径
      if((szFile[0] == '\\' || szFile[0] == '/') && szDir[0] != '\0' &&
        ((szDir[0] >= 'A' && szDir[0] <= 'Z') || (szDir[0] >= 'a' && szDir[0] <= 'z')) &&
        szDir[1] == ':' && szDir[2] == '\\') {
          strTempPath.Append(szDir, 3);
          szFile++;
      }
    }
#endif
    // else: strDestPath = szFile


    if(strTempPath.IsNotEmpty() &&
      strTempPath.Back() != (typename _TString::TChar)s_PathSlash &&
      strTempPath.Back() != (typename _TString::TChar)s_VicePathSlash) {
        strTempPath.Append((typename _TString::TChar)s_PathSlash);
    }

    //clsize len = strTempPath.GetLength();
    //_TString::LPCSTR pBack = strTempPath.GetBuffer(len + )
    strTempPath.Reserve(strTempPath.GetLength() + clstd::strlenT(szFile));

    while(*szFile) {
      if(*szFile == (typename _TString::TChar)s_VicePathSlash) {
        if(strTempPath.EndsWith((typename _TString::TChar)s_VicePathSlash)) {
          strTempPath.Back() = (typename _TString::TChar)s_PathSlash;
        } else if( ! strTempPath.EndsWith((typename _TString::TChar)s_PathSlash)) {
          strTempPath.Append((typename _TString::TChar)s_PathSlash);
        }
      } else {
        strTempPath.Append(*szFile);
      }
      szFile++;
    }
    //strTempPath.Append(szFile);
    
    strDestPath = strTempPath;
    return strDestPath;
  }
  
  clStringA& CombinePath(clStringA& strDestPath, clStringA::LPCSTR szDir, clStringA::LPCSTR szFile)
  {
    return CombinePathT(strDestPath, szDir, szFile);
  }

  clStringW& CombinePath(clStringW& strDestPath, clStringW::LPCSTR szDir, clStringW::LPCSTR szFile)
  {
    return CombinePathT(strDestPath, szDir, szFile);
  }

  clStringA CombinePath(clStringA::LPCSTR szDir, clStringA::LPCSTR szFile)
  {
    clStringA str;
    return CombinePathT(str, szDir, szFile);
  }

  clStringW CombinePath(clStringW::LPCSTR szDir, clStringW::LPCSTR szFile)
  {
    clStringW str;
    return CombinePathT(str, szDir, szFile);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString>
  _TString& CombineAbsPathToT(_TString& strDestPath, const _TString& strSrcPath)
  {
    return CombinePathT(
      (_TString&)strDestPath, 
      (const _TString&)s_strRootDir, 
      (const _TString&)strSrcPath);
  }
  clStringA& CombineAbsPathTo(clStringA& strDestPath, const clStringA& strSrcPath)
  {
    return CombineAbsPathToT(strDestPath, strSrcPath);
  }
  clStringW& CombineAbsPathTo(clStringW& strDestPath, const clStringW& strSrcPath)
  {
    return CombineAbsPathToT(strDestPath, strSrcPath);
  }

  template<typename _TString>
  clsize CombineAbsPathT(_TString& strPath)
  {
    if(IsRelativeT<typename _TString::TChar>(strPath)) {
      ASSERT(s_strRootDir.IsNotEmpty());
      CombinePathT(
        (_TString&)strPath, 
        (const _TString&)s_strRootDir, 
        (const _TString&)strPath);
    }
    return strPath.GetLength();
  }
  clsize CombineAbsPath(clStringA& strPath)
  {
    return CombineAbsPathT(strPath);
  }
  clsize CombineAbsPath(clStringW& strPath)
  {
    return CombineAbsPathT(strPath);
  }

  template<typename _TString>
  clsize MakeFullPathT(_TString& strPath)
  {
    if(IsRelativeT<typename _TString::TChar>(strPath)) {
      _TString strCurrDir;
      CombinePathT(strPath, clpathfile::GetCurrentDirectory(strCurrDir), strPath);
      return strPath.GetLength();
    }
    return 0;
  }

  clsize MakeFullPath(clStringA& strPath)
  {
    return MakeFullPathT(strPath);
  }

  clsize MakeFullPath(clStringW& strPath)
  {
    return MakeFullPathT(strPath);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString>
  _TString CanonicalizeT(const _TString& strPath)
  {
    clsize nPos = 0;
    //clStringA str = strPath;
    _TString str = strPath;

    str.Replace((ch)s_VicePathSlash, (ch)s_PathSlash);

    while(1)
    {
      nPos = str.Find('.', nPos);
      if(nPos == _TString::npos)
        break;

      if(str[nPos + 1] == (typename _TString::TChar)'.')
      {
        clsize nPrevPath = str.ReverseFind((ch)s_PathSlash, 0, (int)nPos - 1);
        str.Replace(nPrevPath, nPos - nPrevPath + 2, NULL);
        nPos -= nPos - nPrevPath + 2;
      }
      else if((nPos > 0 && str[nPos - 1] == s_PathSlash) || (str[nPos + 1] == s_PathSlash))
      {
        str.Replace(nPos - 1, 2, NULL);
        nPos -= 2;
      }
      nPos++;
      //if(nPos >= str.GetLength())
      //  break;
    }

    str.Replace((typename _TString::TChar)s_VicePathSlash, (typename _TString::TChar)s_PathSlash);
    return str;
  }
  
  clStringA Canonicalize(const clStringA& strPath)
  {
    return CanonicalizeT(strPath);
  }

  clStringW Canonicalize(const clStringW& strPath)
  {
    return CanonicalizeT(strPath);
  }
  //////////////////////////////////////////////////////////////////////////
  template<class _Traits, typename _TCh>
  b32 IsFileSpecT (const _TCh* szPath)
  {
    if(_Traits::StringSearchChar(szPath, (_TCh)s_PathSlash) != NULL || 
      _Traits::StringSearchChar(szPath, (_TCh)s_VicePathSlash) != NULL || 
      _Traits::StringSearchChar(szPath, ':') != NULL) {
      return FALSE;
    }
    return TRUE;
  }

  b32 IsFileSpec(const  ch* szPath)
  {
    return IsFileSpecT<clstd::StringA_traits, ch>(szPath);
  }

  b32 IsFileSpec(const wch* szPath)
  {
    return IsFileSpecT<clstd::StringW_traits, wch>(szPath);
  }
  //////////////////////////////////////////////////////////////////////////
  template<typename _TString>
  b32 RemoveFileSpecT(_TString& strPath)
  {
    clsize p = strPath.GetLength();
    while(--p != (clsize)-1) {
      if(IS_ANY_SLASH(strPath[p])) {        
        break;
      }
    }

    if(p == _TString::npos) {
      strPath.Clear();
      return TRUE;
    }

    size_t nLen = strPath.GetLength();
    return (strPath.Remove(p, -1) - nLen != 0);
  }

  b32 RemoveFileSpec(clStringA& strPath)
  {
    return RemoveFileSpecT(strPath);
  }

  b32 RemoveFileSpec(clStringW& strPath)
  {
    return RemoveFileSpecT(strPath);
  }

  //////////////////////////////////////////////////////////////////////////
  template<typename _TCh>
  b32 IsRelativeT(const _TCh* szPath)
  {
#if defined(_CL_SYSTEM_WINDOWS)
    return ( ! (IS_ANY_SLASH(szPath[0]) || szPath[1] == ':') );
#else
    return ( ! IS_ANY_SLASH(szPath[0]));
#endif
  }

  b32 IsRelative(const  ch* szPath)
  {
    return IsRelativeT<ch>(szPath);
  }

  b32 IsRelative(const wch* szPath)
  {
    return IsRelativeT<wch>(szPath);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TString, typename _TCh>
  int CommonPrefixT(_TString& strCommDir, const _TCh* szPath1, const _TCh* szPath2)
  {
    // 没测试过
    clsize nLen1 = clstd::strlenT(szPath1);
    clsize nLen2 = clstd::strlenT(szPath2);
    if(nLen1 == 0 || nLen2 == 0)
    {
      strCommDir.Clear();
      return 0;
    }

    _TCh* pBuffer = strCommDir.GetBuffer((int)(nLen1 > nLen2 ? nLen1 : nLen2) + 1);
    int nSlash = -1; // 记录路径分割符的位置
    for(int i = 0;; i++)
    {
      if(IS_ANY_SLASH(szPath1[i]) && IS_ANY_SLASH(szPath2[i])) {
        pBuffer[i] = (_TCh)s_PathSlash;
        nSlash = i;
      }
      else if(clstd::tolowerT(szPath1[i]) == clstd::tolowerT(szPath2[i]))
      {
        pBuffer[i] = szPath1[i];
      }
      else break;
    }

    pBuffer[++nSlash] = _TCh('\0');
    strCommDir.ReleaseBuffer();
    return nSlash;
  }

  int CommonPrefix(clStringA& strCommDir, const  ch* szPath1, const  ch* szPath2)
  {
    return CommonPrefixT(strCommDir, szPath1, szPath2);
  }

  int CommonPrefix(clStringW& strCommDir, const wch* szPath1, const wch* szPath2)
  {
    return CommonPrefixT(strCommDir, szPath1, szPath2);
  }

  //////////////////////////////////////////////////////////////////////////

  template<class _TString, typename _TCh>
  b32 RelativePathToT(_TString& strOutput, const _TCh* szFromPath, b32 bFromIsDir, const _TCh* szToPath, b32 bToIsDir)
  {
    _TString strCommon;
    _TString strFromPath = szFromPath;
    _TString strToPath = szToPath;
    static _TCh szParentPath[] = {'.', '.', '\\', '\0'};
    
    CanonicalizeT(strFromPath);
    CanonicalizeT(strToPath);

    if( ! bFromIsDir) {
      RemoveFileSpecT(strFromPath);
    }

    //if( ! bToIsDir) {
    //  RemoveFileSpecA(strToPath);
    //}

    // FIXME: 如果strFromPath和strToPath只包含一个盘符，另一个以“\”开头，则去掉盘符

    if( ! CommonPrefixT(strCommon, szFromPath, szToPath)) {
      if(IsRelativeT(szToPath)) {
        strOutput = strToPath;
        return TRUE;
      }
      return FALSE;
    }

    strFromPath.Replace(0, strCommon.GetLength(), NULL);
    strToPath.Replace(0, strCommon.GetLength(), NULL);

    strOutput.Clear();
    if(strFromPath.IsNotEmpty())
    {
      clsize nCount = strFromPath.Replace((typename _TString::TChar)s_PathSlash, (typename _TString::TChar)s_PathSlash);
      for(clsize i = 0; i < nCount + 1; i++) {
        strOutput.Append(szParentPath);
      }
    }
    strOutput.Append(strToPath);
    return TRUE;
  }

  b32 RelativePathTo(clStringA& strOutput, const ch* szFromPath, b32 bFromIsDir, const ch* szToPath, b32 bToIsDir)
  {
    return RelativePathToT(strOutput, szFromPath, bFromIsDir, szToPath, bToIsDir);
  }

  b32 RelativePathTo(clStringW& strOutput, const wch* szFromPath, b32 bFromIsDir, const wch* szToPath, b32 bToIsDir)
  {
    return RelativePathToT(strOutput, szFromPath, bFromIsDir, szToPath, bToIsDir);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TCh>
  b32 MatchSpecT(const _TCh* szFile, const _TCh* szSpec)
  {
    while(*szFile && *szSpec) {
      if(*szSpec == '*') {
        const _TCh* pSpecEnd = ++szSpec;

        // '*' 是最后一个符号
        if(*pSpecEnd == '\0') {
          return TRUE;
        }

        // 测量夹在两个'*'之间的字符串长度
        while(*pSpecEnd != '*' && *pSpecEnd != '\0') {
          pSpecEnd++;
        }
        
        do {
          // TODO: 其实szFile长度可能会小于（pSpecEnd - szSpec）长度
          if(clstd::strncmpT(szFile, szSpec, pSpecEnd - szSpec) == 0) {
            szFile += (pSpecEnd - szSpec);
            szSpec = pSpecEnd;
            break;
          }
        } while (*(++szFile));

        if(*szFile == '\0') {
          return *szSpec == '\0';
        }
        continue;
      }
      else if(*szSpec != '?' && *szSpec != *szFile) {
        return FALSE;
      }
      szFile++;
      szSpec++;
    }
    return (*szFile == '\0' && *szSpec == '\0');
  }

  b32 MatchSpec(const ch* szFile, const ch* szSpec)
  {
    return MatchSpecT(szFile, szSpec);
  }

  b32 MatchSpec(const wch* szFile, const wch* szSpec)
  {
    return MatchSpecT(szFile, szSpec);
  }

  //////////////////////////////////////////////////////////////////////////
  template<typename _TCh>
  b32 MatchSpecExT(const _TCh* szFile, const _TCh* szSpec, u32 dwFlags)
  {
    if(dwFlags == MatchSpec_NORMAL) {
      return MatchSpec(szFile, szSpec);
    }
    else if(dwFlags == MatchSpec_MULTIPLE) {
      CLBREAK; // 未实现
    }
    return FALSE;
  }
  
  b32 MatchSpecEx(const ch* szFile, const ch* szSpec, u32 dwFlags)
  {
    return MatchSpecExT(szFile, szSpec, dwFlags);
  }

  b32 MatchSpecEx(const wch* szFile, const wch* szSpec, u32 dwFlags)
  {
    return MatchSpecExT(szFile, szSpec, dwFlags);
  }

//////////////////////////////////////////////////////////////////////////


#ifdef _WINDOWS
  b32 LocalWorkingDirA(CLLPCSTR szDir)
  {
    if(szDir) {
      clStringW strDir = szDir;
      return LocalWorkingDirW(strDir);
    }
    return LocalWorkingDirW(NULL);
  }

  b32 LocalWorkingDirW(CLLPCWSTR szDir)
  {
    if( ! IsRelative(szDir)) {
      return SetCurrentDirectoryW(szDir);
    }
    wch szModulePath[MAX_PATH];
    clStringW strDir;
    clStringW strFile;
    GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
    SplitPath(clStringW(szModulePath), &strDir, &strFile);
    if(szDir) {
      CombinePath(strDir, strDir, szDir);
    }
    s_strRootDir = clpathfile::CanonicalizeT(strDir);
    return SetCurrentDirectoryW(strDir);
  }

  clStringA& GetCurrentDirectory(clStringA& strDir)
  {
    auto str = strDir.GetBuffer(MAX_PATH);
    ::GetCurrentDirectoryA(MAX_PATH, str);
    strDir.ReleaseBuffer();
    return strDir;
  }

  clStringW& GetCurrentDirectory(clStringW& strDir)
  {
    auto str = strDir.GetBuffer(MAX_PATH);
    ::GetCurrentDirectoryW(MAX_PATH, str);
    strDir.ReleaseBuffer();
    return strDir;
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TCh>
  b32 IsPathExistT(const _TCh* szPath)
  {

  }

  b32 IsPathExist(const ch* szPath)
  {
    return PathFileExistsA(szPath);
  }

  b32 IsPathExist(const wch* szPath)
  {
    return PathFileExistsW(szPath);
  }

#elif defined(_CL_SYSTEM_LINUX)

  clStringA& GetCurrentDirectory(clStringA& strDir)
  {
    auto str = strDir.GetBuffer(MAX_PATH);
    const int cnt = readlink("/proc/self/exe", str, MAX_PATH);
    strDir.ReleaseBuffer();
    if(cnt < 0 || cnt > MAX_PATH) {
      strDir.Clear();
    } else {
      RemoveFileSpec(strDir);
    }
    return strDir;
  }

  clStringW& GetCurrentDirectory(clStringW& strDir)
  {
    clStringA strDirA;
    strDir = GetCurrentDirectory(strDirA).CStr();
    return strDir;
  }

  b32 IsPathExist(const ch* szPath)
  {
    return access(szPath, R_OK) == 0;
  }

  b32 IsPathExist(const wch* szPath)
  {
    clStringA str = szPath;
    return IsPathExist(str);
  }

#else

  b32 LocalWorkingDirA(CLLPCSTR szDir)
  {
    CLBREAK;
    return FALSE;
  }

  b32 LocalWorkingDirW(CLLPCWSTR szDir)
  {
    CLBREAK;
    return FALSE;
  }

#endif // #ifdef _WINDOWS

  //////////////////////////////////////////////////////////////////////////
  template<typename _TCh, class _Tmkdir>
  b32 CreateDirectoryAlwaysT(const _TCh* szDirName, _Tmkdir __mkdir)
  {
    size_t len = clstd::strlenT(szDirName);
    LocalBuffer<1024> buf;
    buf.Resize((len + 1) * sizeof(_TCh), FALSE);
    _TCh* szSubDir = reinterpret_cast<_TCh*>(buf.GetPtr());

    for(size_t i = 0;; i++)
    {
      if (szDirName[i] == s_PathSlash || szDirName[i] == s_VicePathSlash || szDirName[i] == '\0') {
        szSubDir[i] = '\0';
        if( ! IsPathExist(szSubDir)) {
#if defined(_CL_SYSTEM_WINDOWS)
          int result = __mkdir(szSubDir);
#else
          int result = __mkdir(szSubDir, S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP);
#endif
          if(result != 0) {
            return FALSE;
          }
        }

        if (szDirName[i] == '\0') {
          break;
        }
      }
      szSubDir[i] = szDirName[i];
    }
    return TRUE;
  }


  b32 CreateDirectoryAlways(const wch* szDirName)
  {
#if defined(_CL_SYSTEM_WINDOWS)
    return CreateDirectoryAlwaysT(szDirName, _wmkdir);
#else
    clStringA str(szDirName);
    return CreateDirectoryAlwaysT((const ch*)str, mkdir);
#endif
  }

  b32 CreateDirectoryAlways(const ch* szDirName)
  {
#if defined(_CL_SYSTEM_WINDOWS)
    return CreateDirectoryAlwaysT(szDirName, _mkdir);
#else
    return CreateDirectoryAlwaysT(szDirName, mkdir);
#endif
  }
  //_findfirst

#ifdef _CL_SYSTEM_WINDOWS
  CLDWORD GetFileAttributes(const wch* szPath)
  {
    const DWORD attr = ::GetFileAttributesW(szPath);
    // Win32 文档: 函数如果失败，返回0xffffffff
    if(attr == 0xffffffff) {
      return attr;
    }
    return FindFile::IntTranslateAttr(attr);
  }

  CLDWORD GetFileAttributes(const ch* szPath)
  {
    const DWORD attr = ::GetFileAttributesA(szPath);
    // Win32 文档: 函数如果失败，返回0xffffffff
    if(attr == 0xffffffff) {
      return attr;
    }
    return FindFile::IntTranslateAttr(attr);
  }
 
  b32 GetFileDescription(const wch* szPath, FILEATTRIBUTE* pFileAttr)
  {
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    BOOL bresult = ::GetFileAttributesExW(szPath, GetFileExInfoStandard, &wfad);
    if(_CL_NOT_(bresult))
    {
      CLOG_ERRORW(_CLTEXT("GetFileDescription(\"%s\") failed."), szPath);
      return FALSE;
    }

    pFileAttr->dwFileAttributes = FindFile::IntTranslateAttr(wfad.dwFileAttributes);
    pFileAttr->nCreationTime = CLMAKE64FROM32(wfad.ftCreationTime.dwLowDateTime, wfad.ftCreationTime.dwHighDateTime);
    pFileAttr->nLastAccessTime = CLMAKE64FROM32(wfad.ftLastAccessTime.dwLowDateTime, wfad.ftLastAccessTime.dwHighDateTime);
    pFileAttr->nLastWriteTime = CLMAKE64FROM32(wfad.ftLastWriteTime.dwLowDateTime, wfad.ftLastWriteTime.dwHighDateTime);
    pFileAttr->nFileSize = CLMAKE64FROM32(wfad.nFileSizeLow, wfad.nFileSizeHigh);       

    return bresult;
  }

  b32 GetFileDescription(const ch* szPath, FILEATTRIBUTE* pFileAttr)
  {
    clStringW strPath = szPath;
    return GetFileDescription(strPath, pFileAttr);
  }

#else
  CLDWORD GetFileAttributes(const wch* szPath)
  {
    clStringA strPath = szPath;
    return GetFileAttributes(strPath);
  }

  CLDWORD GetFileAttributes(const ch* szPath)
  {
    struct stat st_stat;
    int result = stat(szPath, st_stat); // to utf-8
    if(result != 0)
    {
      CLOG_ERROR("GetFileAttributes(%s) failed.", szPath);
      return 0xffffffff;
    }

    return FindFile::IntTranslateAttr(st_stat.st_mode);
  }

  b32 GetFileDescription(const wch* szPath, FILEATTRIBUTE* pFileAttr)
  {
    clStringA strPath = szPath;
    return GetFileDescription(strPath, pFileAttr);
  }

  b32 GetFileDescription(const ch* szPath, FILEATTRIBUTE* pFileAttr)
  {
    struct stat st_stat;
    int result = stat(szPath, st_stat); // to utf-8
    if(result != 0)
    {
      CLOG_ERROR("GetFileDescription(%s) failed.", szPath);
      return FALSE;
    }

    pFileAttr->dwFileAttributes = FindFile::IntTranslateAttr(st_stat.st_mode);
    pFileAttr->nCreationTime    = st_stat.st_ctime;
    pFileAttr->nLastAccessTime  = st_stat.st_atime;
    pFileAttr->nLastWriteTime   = st_stat.st_mtime;
    pFileAttr->nFileSize        = st_stat.st_size;
    return TRUE;
  }
#endif

} // namespace clpathfile

//////////////////////////////////////////////////////////////////////////

namespace clstd
{
#if defined(_CL_SYSTEM_WINDOWS)
  FindFile::FindFile()
    : hFind(INVALID_HANDLE_VALUE)
  {
    InlSetZeroT(wfd);
  }

  FindFile::FindFile(CLLPCSTR szFilename)
    : hFind(INVALID_HANDLE_VALUE)
  {
    NewFind(szFilename);
  }

  FindFile::FindFile(CLLPCWSTR szFilename)
    : hFind(INVALID_HANDLE_VALUE)
  {
    NewFind(szFilename);
  }

  b32 FindFile::NewFind(CLLPCWSTR szFilename)
  {
    if(hFind != INVALID_HANDLE_VALUE) {
      FindClose(hFind);
    }
    hFind = FindFirstFileW(szFilename, &wfd);
    return (hFind != INVALID_HANDLE_VALUE);
  }

  b32 FindFile::NewFind(CLLPCSTR szFilename)
  {
    clStringW strFilename = szFilename;
    return NewFind(strFilename);
  }

  b32 FindFile::GetFile(FINDFILEDATAW* FindFileData)
  {
    if(hFind == INVALID_HANDLE_VALUE) {
      return FALSE;
    }
    clstd::strcpynT(FindFileData->cFileName, wfd.cFileName, MAX_PATH);
    FindFileData->dwAttributes = IntTranslateAttr(wfd.dwFileAttributes);
    FindFileData->nFileSize       = CLMAKE64FROM32(wfd.nFileSizeLow, wfd.nFileSizeHigh);
    FindFileData->nCreationTime   = CLMAKE64FROM32(wfd.ftCreationTime.dwLowDateTime, wfd.ftCreationTime.dwHighDateTime);
    FindFileData->nLastAccessTime = CLMAKE64FROM32(wfd.ftLastAccessTime.dwLowDateTime, wfd.ftLastAccessTime.dwHighDateTime);
    FindFileData->nLastWriteTime  = CLMAKE64FROM32(wfd.ftLastWriteTime.dwLowDateTime, wfd.ftLastWriteTime.dwHighDateTime);
    if(!FindNextFileW(hFind, &wfd))
    {
      FindClose(hFind);
      hFind = INVALID_HANDLE_VALUE;
    }
    return TRUE;
  }

  b32 FindFile::GetFile(FINDFILEDATAA* FindFileData)
  {
    FINDFILEDATAW ffdW;
    const b32 bval = GetFile(&ffdW);
    WideCharToMultiByte(CP_ACP, NULL, ffdW.cFileName, -1, FindFileData->cFileName, MAX_PATH, NULL, NULL);
    FindFileData->dwAttributes = IntTranslateAttr(ffdW.dwAttributes);
    FindFileData->nFileSize       = ffdW.nFileSize;
    FindFileData->nCreationTime   = ffdW.nCreationTime;
    FindFileData->nLastAccessTime = ffdW.nLastAccessTime;
    FindFileData->nLastWriteTime  = ffdW.nLastWriteTime;
    return bval;
  }

  CLDWORD FindFile::IntTranslateAttr(CLDWORD uNativeAttr)
  {
    CLDWORD dwAttr = 0;
    if(TEST_FLAG(uNativeAttr, FILEATTRIBUTE_READONLY)) {
      SET_FLAG(dwAttr, FileAttribute_ReadOnly);
    }
    if(TEST_FLAG(uNativeAttr, FILEATTRIBUTE_HIDDEN)) {
      SET_FLAG(dwAttr, FileAttribute_Hidden);
    }
    if(TEST_FLAG(uNativeAttr, FILEATTRIBUTE_SYSTEM)) {
      SET_FLAG(dwAttr, FileAttribute_System);
    }
    if(TEST_FLAG(uNativeAttr, FILEATTRIBUTE_DIRECTORY)) {
      SET_FLAG(dwAttr, FileAttribute_Directory);
    }
    return dwAttr;
  }

#else
  FindFile::FindFile()
    : m_dir(NULL)
  {
    //InlSetZeroT(finddata);
  }

  FindFile::FindFile(CLLPCSTR szFilename)
    : m_dir(NULL)
  {
    NewFind(szFilename);
  }

  FindFile::FindFile(CLLPCWSTR szFilename)
    : m_dir(NULL)
  {
    NewFind(szFilename);
  }

  b32 FindFile::NewFind(CLLPCWSTR szFilename)
  {
    clStringA strFilename = szFilename;
    return NewFind(strFilename);
  }

  b32 FindFile::NewFind(CLLPCSTR szFilename)
  {
    if(m_dir) {
      closedir(m_dir);
      m_dir = NULL;
    }

    clStringA strFullPath;
    clStringA strFindDir;
    if(strcmpT(szFilename, "*") == 0 || strcmpT(szFilename, "*.*") == 0)
    {
      m_strMatchName = "*";
      szFilename = clpathfile::GetCurrentDirectory(strFindDir);
      // CLOG("%s(%d): %s", __FUNCTION__, __LINE__, szFilename);
    }
    else if(clpathfile::IsFileSpec(szFilename))
    {
      m_strMatchName = szFilename;
      szFilename = clpathfile::GetCurrentDirectory(strFindDir);
    }
    else if(clpathfile::IsRelative(szFilename))
    {
      clpathfile::CombinePath(strFullPath, 
        clpathfile::GetCurrentDirectory(strFindDir), szFilename);
      clsize pos = clpathfile::FindFileName(strFullPath);
      
      ASSERT(pos != clStringA::npos);

      m_strMatchName = &strFullPath.CStr()[pos];
      strFullPath.Remove(pos, -1);
      szFilename = strFullPath;
    }
    else
    {
      clsize pos = clpathfile::FindFileName(szFilename);
      
      ASSERT(pos != clStringA::npos);

      m_strMatchName = &szFilename[pos];
      strFullPath.Append(szFilename, pos);
      szFilename = strFullPath;
    }

    m_dir = opendir(szFilename);
    return (m_dir != NULL);
  }

  b32 FindFile::GetFile(FINDFILEDATAW* FindFileData)
  {
    FINDFILEDATAA finddataA;
    if(!GetFile(&finddataA)) {
      return FALSE;
    }
    clStringW str(finddataA.cFilename);
    clstd::strcpynT(FindFileData->szFilename, (const wch*)str, MAX_PATH);
    FindFileData->dwAttributes    = finddataA.dwAttributes;
    FindFileData->nFileSizeHigh   = finddataA.nFileSizeHigh;
    FindFileData->nFileSizeLow    = finddataA.nFileSizeLow;
    FindFileData->nCreationTime   = finddataA.nCreationTime;
    FindFileData->nLastAccessTime = finddataA.nLastAccessTime;
    FindFileData->nLastWriteTime  = finddataA.nLastWriteTime;

    return TRUE;
  }

  b32 FindFile::GetFile(FINDFILEDATAA* FindFileData)
  {
    if(!m_dir) {
      return FALSE;
    }

    dirent* ptr = NULL;

    while(1)
    {
      ptr = readdir(m_dir);

      if(ptr)
      {
        struct stat _sStat;

        if(!clpathfile::MatchSpec(ptr->d_name, m_strMatchName)) {
          continue;
        }

        clstd::strcpynT(FindFileData->szFilename, ptr->d_name, MAX_PATH);
        stat(ptr->d_name, &_sStat);

        FindFileData->dwAttributes = IntTranslateAttr(_sStat.st_mode);
        FindFileData->nFileSizeHigh = 0;
        FindFileData->nFileSizeLow = _sStat.st_size;
        FindFileData->nCreationTime = _sStat.st_ctime;
        FindFileData->nLastAccessTime = _sStat.st_atime;
        FindFileData->nLastWriteTime = _sStat.st_mtime;
      }
      break;
    }

    return (ptr != NULL);
  }

  CLDWORD FindFile::IntTranslateAttr(CLDWORD uNativeAttr)
  {
    CLDWORD dwAttr = 0;
    if(S_ISDIR(uNativeAttr))
    {
      SET_FLAG(dwAttr, FileAttribute_Directory);
    }
    if(TEST_FLAG_NOT(uNativeAttr, S_IWUSR))
    {
      SET_FLAG(dwAttr, FileAttribute_ReadOnly);
    }
    //if(TEST_FLAG(uNativeAttr, _A_HIDDEN)) {
    //  SET_FLAG(dwAttr, FileAttribute_Hidden);
    //}
    //if(TEST_FLAG(uNativeAttr, _A_SYSTEM)) {
    //  SET_FLAG(dwAttr, FileAttribute_System);
    //}
    return dwAttr;
  }
#endif // #if defined(_CL_SYSTEM_WINDOWS)
} // namespace clstd