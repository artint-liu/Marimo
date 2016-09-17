#ifndef _FILEPATH_H_
#define _FILEPATH_H_

// 文件名不能包含的字符：\ / * ? " : < > |

// 分离路径为目录和文件名
// 返回值,如果输入的路径名为纯目录或文件名,返回FALSE,否则返回TRUE
template<typename _TString>
b32 _SplitPathT(const _TString& strPath, _TString* pstrDir, _TString* pstrFile);

b32 SplitPathA(const clStringA& strPath, clStringA* pstrDir, clStringA* pstrFile);
b32 SplitPathW(const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile);

// 重置当前工作目录, 这个目录是基于应用程序目录的
void ResetWorkingDir();

//template<typename _TString>
//b32 _GetFileExtensionT(const _TString& strFile, _TString* pstrFilename, _TString* pstrExtension);
//
//b32 GetFileExtensionA(const clStringA& strFile, clStringA* pstrFilename, clStringA* pstrExtension);
//b32 GetFileExtensionW(const clStringW& strFile, clStringW* pstrFilename, clStringW* pstrExtension);

// TODO: 这个将来废弃
template<typename _TString>
b32 IsFullPath(const _TString& strFilename);

b32 IsFullPathA(const clStringA& strFilename);
b32 IsFullPathW(const clStringW& strFilename);

namespace clpathfile
{
  // 定位工作目录
  // 如果szDir为NULL，则工作路径设置为当前应用程序所在目录
  // 如果szDir是一个相对路径，那么工作路径是相对于应用程序所在目录的路径
  // 如果szDir是一个绝对路径，则按照此绝对路径设置当前路径
  // 这个函数目前只在Windows系统下有效
  b32 LocalWorkingDirA(CLLPCSTR szDir);
  b32 LocalWorkingDirW(CLLPCWSTR szDir);

  // 获得当前路径
  // 返回值就是strDir，这么声明就是为了减少内存复制和便于使用
  clStringA& GetCurrentDirectory(clStringA& strDir);
  clStringW& GetCurrentDirectory(clStringW& strDir);

  inline i16 Slash()         // 路径分隔符
  {
    extern i16 s_PathSlash;
    return s_PathSlash;
  }

  inline i16 ViceSlash()    // 次级路径分隔符, 有些系统下这个可能无效
  {
    extern i16 s_VicePathSlash;
    return s_VicePathSlash;
  }

  template<typename _TString>
  b32 RenameExtensionT  (_TString& strPath, typename _TString::LPCSTR pszExt);
  b32 RenameExtensionA  (clStringA& strPath, clStringA::LPCSTR pszExt);
  b32 RenameExtensionW  (clStringW& strPath, clStringW::LPCSTR  pszExt);

  //template<typename _TCh>
  //b32 RenameExtensionT  (_TCh* szPath, CLLPCTSTR pszExt);
  //b32 RenameExtensionA  (ch*   szPath, CLLPCTSTR pszExt);
  //b32 RenameExtensionW  (wch*  szPath, CLLPCTSTR pszExt);

  template<typename _TString>
  clsize FindFileNameT    (const _TString& strPath);
  clsize FindFileNameA    (const clStringA& strPath);
  clsize FindFileNameW    (const clStringW& strPath);

  template<typename _TCh>
  clsize FindExtensionT    (const _TCh* szPath);
  clsize FindExtensionA    (const ch* szPath);
  clsize FindExtensionW    (const wch* szPath);

  template<typename _TString>
  b32 AddSlashT      (_TString& strPath);
  b32 AddSlashA      (clStringA& strPath);
  b32 AddSlashW      (clStringW& strPath);

  template<typename _TString>
  _TString&  CombinePathT  (_TString& strDestPath, const _TString& strDir, const _TString& strFile);
  clStringA& CombinePathA  (clStringA& strDestPath, const clStringA& strDir, const clStringA& strFile);
  clStringW& CombinePathW  (clStringW& strDestPath, const clStringW& strDir, const clStringW& strFile);

  template<typename _TString>
  _TString&  CombineAbsPathToT(_TString& strDestPath, const _TString& strSrcPath);
  clStringA& CombineAbsPathToA(clStringA& strDestPath, const clStringA& strSrcPath);
  clStringW& CombineAbsPathToW(clStringW& strDestPath, const clStringW& strSrcPath);

  template<typename _TString>
  clsize CombineAbsPathT(_TString& strPath);
  clsize CombineAbsPathA(clStringA& strPath);
  clsize CombineAbsPathW(clStringW& strPath);

  // 如果strPath是相对路径，则转换为基于当前路径的完整路径
  // 返回0表示失败，strPath可能已经是完整路径
  // 大于0表示成功，数值是转换后的字符串长度
  template<typename _TString>
  clsize MakeFullPath(_TString& strPath);
  clsize MakeFullPathA(clStringA& strPath);
  clsize MakeFullPathW(clStringW& strPath);

  template<typename _TString>
  _TString  CanonicalizeT  (const _TString& strPath);
  clStringA CanonicalizeA  (const clStringA& strPath);
  clStringW CanonicalizeW  (const clStringW& strPath);

  template<class _Traits, typename _TCh>
  b32 IsFileSpecT (const _TCh* szPath);
  b32 IsFileSpecA (const  ch* szPath);
  b32 IsFileSpecW (const wch* szPath);

  // 移除路径中的文件名和最后一个路径符号
  template<typename _TString>
  b32 RemoveFileSpecT (_TString&  strPath);
  b32 RemoveFileSpecA (clStringA& strPath);
  b32 RemoveFileSpecW (clStringW& strPath);

  template<typename _TCh>
  b32 IsRelativeT (const _TCh* szPath);
  b32 IsRelativeA (const  ch* szPath);
  b32 IsRelativeW (const wch* szPath);

  template<typename _TString, typename _TCh>
  int CommonPrefixT(_TString& strCommDir, const _TCh* szPath1, const _TCh* szPath2);
  int CommonPrefixA(clStringA& strCommDir, const  ch* szPath1, const  ch* szPath2);
  int CommonPrefixW(clStringW& strCommDir, const wch* szPath1, const wch* szPath2);

  template<class _TString, typename _TCh>
  b32 RelativePathToT(_TString& strOutput, const _TCh* szFromPath, b32 bFromIsDir, const _TCh* szToPath, b32 bToIsDir);
  b32 RelativePathToA(clStringA& strOutput, const ch* szFromPath, b32 bFromIsDir, const ch* szToPath, b32 bToIsDir);
  b32 RelativePathToW(clStringW& strOutput, const wch* szFromPath, b32 bFromIsDir, const wch* szToPath, b32 bToIsDir);

  template<typename _TCh>
  b32 MatchSpecT(const _TCh* szFile, const _TCh* szSpec); // 大小写敏感
  b32 MatchSpec (const ch* szFile, const ch* szSpec);
  b32 MatchSpec (const wch* szFile, const wch* szSpec);

  enum MatchSpecFlags
  {
    MatchSpec_NORMAL   = 0x00000000,
    MatchSpec_MULTIPLE = 0x00000001,
  };

  template<typename _TCh>
  b32 MatchSpecExT(const _TCh* szFile, const _TCh* szSpec, u32 dwFlags);
  b32 MatchSpecEx (const ch* szFile, const ch* szSpec, u32 dwFlags);
  b32 MatchSpecEx (const wch* szFile, const wch* szSpec, u32 dwFlags);
} // namespace clpathfile

//#ifdef _UNICODE
//#define GetFileExtension GetFileExtensionW
//#define SplitPath SplitPathW
//#else
//#define GetFileExtension GetFileExtensionA
//#define SplitPath SplitPathA
//#endif // _UNICODE

#endif // _FILEPATH_H_