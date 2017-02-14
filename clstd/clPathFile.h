#ifndef _FILEPATH_H_
#define _FILEPATH_H_

#ifndef _CLSTD_STRING_H_
# error you must include clString.h first
#endif

// 文件名不能包含的字符：\ / * ? " : < > |

// 分离路径为目录和文件名
// 返回值,如果输入的路径名为纯目录或文件名,返回FALSE,否则返回TRUE
template<typename _TString>
b32 _SplitPathT(const _TString& strPath, _TString* pstrDir, _TString* pstrFile);

b32 SplitPath (const clStringA& strPath, clStringA* pstrDir, clStringA* pstrFile);
b32 SplitPath (const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile);

// 重置当前工作目录, 这个目录是基于应用程序目录的
void ResetWorkingDir();

//template<typename _TString>
//b32 _GetFileExtensionT(const _TString& strFile, _TString* pstrFilename, _TString* pstrExtension);
//
//b32 GetFileExtensionA(const clStringA& strFile, clStringA* pstrFilename, clStringA* pstrExtension);
//b32 GetFileExtensionW(const clStringW& strFile, clStringW* pstrFilename, clStringW* pstrExtension);

// TODO: 这个将来废弃
template<typename _TString>
b32 IsFullPathT(const _TString& strFilename);

b32 IsFullPath(const clStringA& strFilename);
b32 IsFullPath(const clStringW& strFilename);

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

  // 修改路径中的扩展名, 参数中的扩展名接受".exe"或者"exe"两种形式的写法
  template<typename _TString>
  b32 RenameExtensionT (_TString& strPath, typename _TString::LPCSTR szExt);
  b32 RenameExtension  (clStringA& strPath, clStringA::LPCSTR szExt);
  b32 RenameExtension  (clStringW& strPath, clStringW::LPCSTR  szExt);

  // 在路径中查找文件名的位置
  template<typename _TString>
  clsize FindFileNameT   (const _TString& strPath);
  clsize FindFileName    (const clStringA& strPath);
  clsize FindFileName    (const clStringW& strPath);
  clsize FindFileName    (const  ch* szPath);
  clsize FindFileName    (const wch* szPath);

  // 在路径中查找扩展名的位置
  template<typename _TCh>
  clsize FindExtensionT   (const _TCh* szPath);
  clsize FindExtension    (const ch* szPath);
  clsize FindExtension    (const wch* szPath);

  // 比较文件扩展名（区分大小写）
  // 等于返回true，szExtList是扩展名列表，用竖线分割，例如"jpg|png"形式
  template<typename _TCh>
  b32 CompareExtensionT  (const _TCh* szPath, const _TCh* szExtList);
  b32 CompareExtension   (const ch*  szPath, const ch* szExtList);
  b32 CompareExtension   (const wch* szPath, const wch* szExtList);


  // vice slash 替换为 slash, 并在末尾添加 slash （如果末尾没有）
  template<typename _TString>
  b32 AddSlashT     (_TString& strPath);
  b32 AddSlash      (clStringA& strPath);
  b32 AddSlash      (clStringW& strPath);

  // 合并路径和文件名, 如果 szFile 开头（仅在开头）含有相对路径"."和"..", 合并过程中也会整理路径
  template<typename _TString>
  _TString&  CombinePathT (_TString& strDestPath, typename _TString::LPCSTR szDir, typename _TString::LPCSTR szFile);
  clStringA& CombinePath  (clStringA& strDestPath, clStringA::LPCSTR szDir, clStringA::LPCSTR szFile);
  clStringW& CombinePath  (clStringW& strDestPath, clStringW::LPCSTR szDir, clStringW::LPCSTR szFile);

  template<typename _TString>
  _TString  CombinePathT (typename _TString::LPCSTR szDir, typename _TString::LPCSTR szFile);
  clStringA CombinePath  (clStringA::LPCSTR szDir, clStringA::LPCSTR szFile);
  clStringW CombinePath  (clStringW::LPCSTR szDir, clStringW::LPCSTR szFile);

  // TODO: 考察准备废掉？
  template<typename _TString>
  _TString&  CombineAbsPathToT(_TString& strDestPath, const _TString& strSrcPath);
  clStringA& CombineAbsPathTo (clStringA& strDestPath, const clStringA& strSrcPath);
  clStringW& CombineAbsPathTo (clStringW& strDestPath, const clStringW& strSrcPath);

  // TODO: 考察准备废掉？
  template<typename _TString>
  clsize CombineAbsPathT(_TString& strPath);
  clsize CombineAbsPath (clStringA& strPath);
  clsize CombineAbsPath (clStringW& strPath);

  // TODO: 考察准备废掉？
  // 如果strPath是相对路径，则转换为基于当前路径的完整路径
  // 返回0表示失败，strPath可能已经是完整路径
  // 大于0表示成功，数值是转换后的字符串长度
  template<typename _TString>
  clsize MakeFullPathT(_TString& strPath);
  clsize MakeFullPath(clStringA& strPath);
  clsize MakeFullPath(clStringW& strPath);

  // 整理路径名, 消除"."和".."表示的相对路径, vice slash 替换为 slash
  template<typename _TString>
  _TString  CanonicalizeT (const _TString& strPath);
  clStringA Canonicalize  (const clStringA& strPath);
  clStringW Canonicalize  (const clStringW& strPath);

  // 判断这个路径只是单纯文件名，是返回1，否则返回0
  template<class _Traits, typename _TCh>
  b32 IsFileSpecT (const _TCh* szPath);
  b32 IsFileSpec  (const  ch* szPath);
  b32 IsFileSpec  (const wch* szPath);

  // 移除路径中的文件名和最后一个路径符号
  template<typename _TString>
  b32 RemoveFileSpecT (_TString&  strPath);
  b32 RemoveFileSpec  (clStringA& strPath);
  b32 RemoveFileSpec  (clStringW& strPath);

  // 判断是否为相对路径
  template<typename _TCh>
  b32 IsRelativeT (const _TCh* szPath);
  b32 IsRelative  (const  ch* szPath);
  b32 IsRelative  (const wch* szPath);

  // 获得两个路径相同的路径前缀
  template<typename _TString, typename _TCh>
  int CommonPrefixT(_TString& strCommDir, const _TCh* szPath1, const _TCh* szPath2);
  int CommonPrefix (clStringA& strCommDir, const  ch* szPath1, const  ch* szPath2);
  int CommonPrefix (clStringW& strCommDir, const wch* szPath1, const wch* szPath2);

  // 获得一个路径对另一个路径的相对路径（绕口令？）
  template<class _TString, typename _TCh>
  b32 RelativePathToT(_TString& strOutput, const _TCh* szFromPath, b32 bFromIsDir, const _TCh* szToPath, b32 bToIsDir);
  b32 RelativePathTo (clStringA& strOutput, const ch* szFromPath, b32 bFromIsDir, const ch* szToPath, b32 bToIsDir);
  b32 RelativePathTo (clStringW& strOutput, const wch* szFromPath, b32 bFromIsDir, const wch* szToPath, b32 bToIsDir);

  // 含有通配符的比较, "?"表示任意一个字母, "*"表示任意字母
  template<typename _TCh>
  b32 MatchSpecT(const _TCh* szFile, const _TCh* szSpec); // 大小写敏感
  b32 MatchSpec (const ch* szFile, const ch* szSpec);
  b32 MatchSpec (const wch* szFile, const wch* szSpec);

  enum MatchSpecFlags
  {
    MatchSpec_NORMAL   = 0x00000000,
    MatchSpec_MULTIPLE = 0x00000001,
  };

  // FIXME: 部分实现
  template<typename _TCh>
  b32 MatchSpecExT(const _TCh* szFile, const _TCh* szSpec, u32 dwFlags);
  b32 MatchSpecEx (const ch* szFile, const ch* szSpec, u32 dwFlags);
  b32 MatchSpecEx (const wch* szFile, const wch* szSpec, u32 dwFlags);

  template<typename _TCh>
  b32 IsPathExistT(const _TCh* szPath);
  b32 IsPathExist(const ch* szPath);
  b32 IsPathExist(const wch* szPath);

  // 创建目录，如果中间层路径不存在，则逐级创建每层目录
  template<typename _TCh, class _Tmkdir>
  b32 CreateDirectoryAlwaysT(const _TCh* szDirName, _Tmkdir __mkdir);
  b32 CreateDirectoryAlways(const wch* szDirName);
  b32 CreateDirectoryAlways(const ch* szDirName);
} // namespace clpathfile

//////////////////////////////////////////////////////////////////////////

namespace clpathfile {
#if defined(_CL_SYSTEM_WINDOWS)
  namespace Win32 {
    template<class _TString, class _Fn>
    void RecursiveSearchDir(typename _TString::LPCSTR szStartDir, _Fn fn)
    {
      WIN32_FIND_DATA wfd;
      typename _TString::TChar szFilename[] = {'*','.','*','\0'};
      _TString strDir = szStartDir;
      _TString strSearch;
      if(strDir.Back() != '\\' && strDir.Back() != '/') {
        strDir += "\\";
      }
      strSearch = strDir + szFilename;

      HANDLE hFind = FindFirstFile(strSearch, &wfd);
      if(hFind != INVALID_HANDLE_VALUE)
      {
        do {
          if(wfd.cFileName[0] == '.') {
            continue;
          }

          fn(strDir, wfd);
          if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
            _TString strChildDir = strDir + wfd.cFileName;
            RecursiveSearchDir<_TString>(strChildDir, fn);
          }
        } while (FindNextFile(hFind, &wfd));
      }
    }

    // 从一个路径收集文件，
    // 如果这个路径是目录，则遍历子目录，如果是文件，就填入list后返回
    template<class _TString, typename _TCh, typename _TStringList, class _Fn>
    void GenerateFiles(_TStringList& rFileList, const _TCh* szPath, _Fn fn)
    {
      DWORD dwAttri = GetFileAttributes(szPath);
      if(dwAttri == 0xffffffff) {
        return;
      }

      if(dwAttri & FILE_ATTRIBUTE_DIRECTORY)
      {
        RecursiveSearchDir<_TString>(szPath, [&rFileList, &fn](_TString& strDir, const WIN32_FIND_DATA& wfd) {
          if(fn(wfd)) {
            _TString str;
            str = strDir + wfd.cFileName;
            rFileList.push_back(str);
          }
        });
      }
      else {
        rFileList.push_back(szPath);
      }
    }
  } // namespace Win32
#endif // #if defined(_CL_SYSTEM_WINDOWS)
} // namespace clpathfile

#endif // _FILEPATH_H_