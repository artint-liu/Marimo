#ifndef _SMART_PROFILE_H_
#define _SMART_PROFILE_H_

#ifndef _SMARTSTREAM_2_H_
#error Must be include "smartstream.h" first.
#endif // _SMARTSTREAM_2_H_

//#ifndef _CL_BUFFER_H_
//#error Must be include "clBuffer.h" first.
//#endif // _CL_BUFFER_H_
class clBuffer;
struct SmartProfile_TraitsW
{
  static b32    _StrCmpN(const wch*, const wch*, int);
  static clsize _StrLen(const wch*);
  static clsize _TToI(const wch*);
  static double _TToD(const wch*);
  static b32    _CheckBoolean(const wch*);
  typedef wch _TCh;
  typedef SmartStream_TraitsW SmartStream_Traits;
};

struct SmartProfile_TraitsA
{
  static b32    _StrCmpN(const ch*, const ch*, int);
  static clsize _StrLen(const ch*);
  static clsize _TToI(const ch*);
  static double _TToD(const ch*);
  static b32    _CheckBoolean(const ch*);
  typedef ch _TCh;
  typedef SmartStream_TraitsA SmartStream_Traits;
};

template<
  typename _CTCh, 
  class    _TStr, 
  class    _Traits>
class SmartProfileT
{
public:
  typedef typename _Traits::_TCh _TCh;
  typedef typename SmartProfileT<_CTCh, _TStr, typename _Traits::SmartStream_Traits> _SmartProfileT;
  typedef typename SmartStreamT<_TStr, typename _Traits::SmartStream_Traits> _SmartStreamT;
  typedef typename _SmartStreamT::iterator _MyIterator;

private:
  typedef clvector<void*>  HandleArray;

  _SmartStreamT m_SmartStream;
  clBuffer*     m_pBuffer;
  HandleArray   m_aHandles;

  static void ReverseByteOrder16(u16* ptr, clsize nCount);

public:
  //enum HandleType
  //{
  //  FT_Key,
  //  FT_Section,
  //  FT_VirSection,
  //  FT_VirKey,
  //};
  struct FINDSECT
  {
    //HandleType  eType;
    _MyIterator    itBegin;
    _MyIterator    itEnd;
    _MyIterator    itCur;
    _MyIterator    itSection;  // Sect 的名字

    u32            bSection : 1;
    u32            bWrite   : 1;
                   
    _TStr          strFindName;
    _TStr          strPath;
  };
  typedef FINDSECT*    HANDLE;

  // TODO: 增加自动析构定义“AUTO_HANDLE”
  // !! 但是仔细考虑后发现自动指针有很多问题，比如当参数传递时，Handle复制了两份
  // !! 如果其中一个析构，另外一个储存的handle其实已经失效了。不知道改怎么解决。
  //struct AUTOHANDLE
  //{
  //  HANDLE          handle;

  //  AUTOHANDLE(HANDLE _handle) : handle(_handle){}
  //  ~AUTOHANDLE()
  //  {
  //    if(handle) {
  //      handle->pSmart->CloseHandle(handle);
  //    }
  //  }

  //  AUTOHANDLE& operator=(HANDLE _handle)
  //  {
  //    if(handle) {
  //      handle->pSmart->CloseHandle(handle);
  //    }
  //    handle = _handle;
  //    return *this;
  //  }
  //};

  class VALUE
  {
    friend class SmartProfileT;
    _MyIterator  itSection;    // Sect 的名字
    _MyIterator  itKeyName;    // Key 的名字
    _MyIterator  itValue;      // 值
  public:
    _TStr SectionName ();
    _TStr KeyName     ();
    int   ToInt       ();
    _TStr ToString    ();
    float ToFloat     ();
    b32   ToBoolean   ();
  };
public:
  SmartProfileT();
  virtual ~SmartProfileT();

  b32 Create(_CTCh* szRoot);
  b32 LoadA(const ch* lpProfile);
  b32 SaveA(const ch* lpProfile);
  b32 LoadW(const wch* lpProfile);
  b32 SaveW(const wch* lpProfile);
  b32 Close();

  // 在 hParentSession 下的 szOffset 偏移下查找Session
  // szSect == NULL 则查找目标Section下的所有Section
  HANDLE  FindFirstSection  (HANDLE hSect, b32 bSiblingSect, _CTCh* szOffset, _CTCh* szSect);
  b32     FindNextSection   (HANDLE hFind);
  _TStr   GetSectionName    (HANDLE hSect);
  _TStr   GetPathName       (HANDLE hSect);
  HANDLE  OpenSection       (_CTCh* szDir);    // "a\b\c\d" 式的名字

  // 枚举用, Key 不支持重名使用, 所以没有查找多个Key功能
  HANDLE  FindFirstKey    (HANDLE hSection, VALUE& value);
  b32     FindNextKey     (HANDLE hKey, VALUE& value);

  b32     FindKey               (HANDLE hSection, _CTCh* szKey, VALUE& value) const;
  int     FindKeyAsString       (HANDLE hSection, _CTCh* szKey, _CTCh* szDefault, _TCh* szBuffer, int nCount) const;
  _TStr   FindKeyAsString       (HANDLE hSection, _CTCh* szKey, _TStr strDefault) const;
  int     FindKeyAsInteger      (HANDLE hSection, _CTCh* szKey, int nDefault) const;
  float   FindKeyAsFloat        (HANDLE hSection, _CTCh* szKey, float fDefault);
  b32     FindKeyAsBoolean      (HANDLE hSection, _CTCh* szKey, b32 bDefault);
  int     FindKeyAsIntegerArray (HANDLE hSection, _CTCh* szKey, clBuffer** ppBuffer);

  b32     FindClose      (HANDLE hFind);
  b32     CloseHandle    (HANDLE hHandle);

  HANDLE  CreateSection   (_CTCh* szPath, _CTCh* szSect);  // 其实新建的路径是 szPath\szSect
  b32     DeleteSection   (_CTCh* szPath, _CTCh* szSect);
  b32     SetKey          (HANDLE hSect, _CTCh* szKeyName, _CTCh* szValue, b32 bReplace = TRUE);
  b32     DeleteKey       (HANDLE hSect, _CTCh* szKeyName);

private:
  b32     FindSection       (HANDLE hFind);
  b32     EnumSectKey       (_MyIterator& it) const;
  b32     AddHandle         (HANDLE hHandle);
  b32     DelHandle         (HANDLE hHandle);
  int     PathDepthToTable  (const _TStr& strPath, _TStr* strTable);
  b32     UpdateIterator    (_MyIterator& it, u32 uPos, _CTCh* lpOldPtr, u32 sizeOld, _CTCh* lpNewPtr, u32 sizeNew);
  b32     UpdateHandle      (u32 uPos, _CTCh* lpOldPtr, u32 sizeOld, _CTCh* lpNewPtr, u32 sizeNew);
  void    TrimFrontTab      (clsize& uOffset);
};

template<class _STR>
_STR FromProfileString(const _STR& str);

template<class _STR>
_STR ToProfileString(const _STR& str);

class SmartProfileA : public SmartProfileT<const  ch, clStringA, SmartProfile_TraitsA> {};
class SmartProfileW : public SmartProfileT<const wch, clStringW, SmartProfile_TraitsW> {};

//#ifdef _UNICODE
//typedef SmartProfileW SmartProfile;
//#else
//typedef SmartProfileA SmartProfile;
//#endif



#endif // _SMART_PROFILE_H_