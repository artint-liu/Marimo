#ifndef _FILEPATH_H_
#define _FILEPATH_H_

// �ļ������ܰ������ַ���\ / * ? " : < > |

// ����·��ΪĿ¼���ļ���
// ����ֵ,��������·����Ϊ��Ŀ¼���ļ���,����FALSE,���򷵻�TRUE
template<typename _TString>
b32 _SplitPathT(const _TString& strPath, _TString* pstrDir, _TString* pstrFile);

b32 SplitPathA(const clStringA& strPath, clStringA* pstrDir, clStringA* pstrFile);
b32 SplitPathW(const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile);

// ���õ�ǰ����Ŀ¼, ���Ŀ¼�ǻ���Ӧ�ó���Ŀ¼��
void ResetWorkingDir();

//template<typename _TString>
//b32 _GetFileExtensionT(const _TString& strFile, _TString* pstrFilename, _TString* pstrExtension);
//
//b32 GetFileExtensionA(const clStringA& strFile, clStringA* pstrFilename, clStringA* pstrExtension);
//b32 GetFileExtensionW(const clStringW& strFile, clStringW* pstrFilename, clStringW* pstrExtension);

// TODO: �����������
template<typename _TString>
b32 IsFullPath(const _TString& strFilename);

b32 IsFullPathA(const clStringA& strFilename);
b32 IsFullPathW(const clStringW& strFilename);

namespace clpathfile
{
  // ��λ����Ŀ¼
  // ���szDirΪNULL������·������Ϊ��ǰӦ�ó�������Ŀ¼
  // ���szDir��һ�����·������ô����·���������Ӧ�ó�������Ŀ¼��·��
  // ���szDir��һ������·�������մ˾���·�����õ�ǰ·��
  // �������Ŀǰֻ��Windowsϵͳ����Ч
  b32 LocalWorkingDirA(CLLPCSTR szDir);
  b32 LocalWorkingDirW(CLLPCWSTR szDir);

  inline i16 Slash()         // ·���ָ���
  {
    extern i16 s_PathSlash;
    return s_PathSlash;
  }

  inline i16 ViceSlash()    // �μ�·���ָ���, ��Щϵͳ�����������Ч
  {
    extern i16 s_VicePathSlash;
    return s_VicePathSlash;
  }

  template<typename _TString>
  b32 RenameExtensionT  (_TString& strPath, typename _TString::LPCSTR pszExt);
  b32 RenameExtensionA  (clStringA& strPath, CLLPCSTR pszExt);
  b32 RenameExtensionW  (clStringW& strPath, CLLPCWSTR pszExt);

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

  template<typename _TString>
  _TString  CanonicalizeT  (const _TString& strPath);
  clStringA CanonicalizeA  (const clStringA& strPath);
  clStringW CanonicalizeW  (const clStringW& strPath);

  template<class _Traits, typename _TCh>
  b32 IsFileSpecT (const _TCh* szPath);
  b32 IsFileSpecA (const  ch* szPath);
  b32 IsFileSpecW (const wch* szPath);

  // �Ƴ�·���е��ļ��������һ��·������
  template<typename _TString>
  b32 RemoveFileSpecT (_TString&   strPath);
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
} // namespace clpathfile

//#ifdef _UNICODE
//#define GetFileExtension GetFileExtensionW
//#define SplitPath SplitPathW
//#else
//#define GetFileExtension GetFileExtensionA
//#define SplitPath SplitPathA
//#endif // _UNICODE

#endif // _FILEPATH_H_