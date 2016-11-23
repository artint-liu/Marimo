#include <stdlib.h>

#include "clstd.h"
#include "clString.H"
//#include "../clFile.H"
//#include "../clBuffer.H"
#include "smart/smartstream.h"
#include "clTokens.h"
#include "clStock.h"
#include "clUtility.H"

namespace clstd
{
  //////////////////////////////////////////////////////////////////////////
  //
  // 显式声明模板类
  //
  // template class StockT<clStringA>;
  // template class StockT<clStringW>;

#define _SSP_TEMPL template<class _TStr>
#define _SSP_IMPL StockT<_TStr>

  //template clStringW FromProfileString (const clStringW&);
  //template clStringA  FromProfileString (const clStringA&);
  //template clStringW ToProfileString   (const clStringW&);
  //template clStringA  ToProfileString   (const clStringA&);

  //////////////////////////////////////////////////////////////////////////
  //b32 SmartStock_TraitsW::_StrCmpN(const wch* pStr1, const wch* pStr2, int nCount)
  //{
  //  return wcsncmp(pStr1, pStr2, nCount) == 0;
  //}
  //clsize SmartStock_TraitsW::_StrLen(const wch* pStr)
  //{
  //  return wcslen(pStr);
  //}
  //clsize SmartStock_TraitsW::_TToI(const wch* pStr)
  //{
  //  return clstd::_xstrtoi(pStr);
  //}
  //double SmartStock_TraitsW::_TToD(const wch* pStr)
  //{
  //  return clstd::_xstrtof(pStr);
  //}
  //b32 SmartStock_TraitsW::_CheckBoolean(const wch* pStr)
  //{
  //  return clstd::strcmpiT(pStr, L"ok") == 0 ||
  //    clstd::strcmpiT(pStr, L"true") == 0 ||
  //    clstd::strcmpiT(pStr, L"yes") == 0 ||
  //    clstd::strcmpiT(pStr, L"1") == 0;
  //}

  //b32 SmartStock_TraitsA::_StrCmpN(const ch* pStr1, const ch* pStr2, int nCount)
  //{
  //  return strncmp(pStr1, pStr2, nCount) == 0;
  //}
  //clsize SmartStock_TraitsA::_StrLen(const ch* pStr)
  //{
  //  return strlen(pStr);
  //}
  //clsize SmartStock_TraitsA::_TToI(const ch* pStr)
  //{
  //  return atoi(pStr);
  //}
  //double SmartStock_TraitsA::_TToD(const ch* pStr)
  //{
  //  return atof(pStr);
  //}
  //b32 SmartStock_TraitsA::_CheckBoolean(const ch* pStr)
  //{
  //  return clstd::strcmpiT(pStr, "ok") == 0 ||
  //    clstd::strcmpiT(pStr, "true") == 0 ||
  //    clstd::strcmpiT(pStr, "yes") == 0 ||
  //    clstd::strcmpiT(pStr, "1") == 0;
  //}

#ifdef _DEBUG
# define IS_OUT_OF_DATE ASSERT(nModify == pStock->m_nModify)
#else
# define IS_OUT_OF_DATE
#endif

  template <typename _TCh>
  b32 _CheckBoolean(const _TCh* pStr)
  {
    static const _TCh sz_1[]    = {'1','\0'};
    static const _TCh sz_ok[]   = {'o','k','\0'};
    static const _TCh sz_yes[]  = {'y','e','s','\0'};
    static const _TCh sz_true[] = {'t','r','u','e','\0'};

    return
      clstd::strcmpiT(pStr, sz_1) == 0 ||
      clstd::strcmpiT(pStr, sz_ok) == 0 ||
      clstd::strcmpiT(pStr, sz_yes) == 0 ||
      clstd::strcmpiT(pStr, sz_true) == 0 ;
  }
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL
    _SSP_IMPL::ATTRIBUTE::~ATTRIBUTE()
  {
    pSection = NULL;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::ATTRIBUTE::IsEmpty() const
  {
    return ( ! pSection) || ( ! pSection->pStock) || (pSection->nModify != pSection->pStock->m_nModify);
  }

  _SSP_TEMPL
  _SSP_IMPL::ATTRIBUTE::operator unspecified_bool_type() const
  {
    return IsEmpty() ? 0 : __unspecified_bool_type<ATTRIBUTE>;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::ATTRIBUTE::NextKey()
  {
    if(IsEmpty()) {
      return FALSE;
    }

    auto it = value;
    auto itNext = it;

    if(itNext == pSection->iter_end) { return FALSE; }

    while(++itNext != pSection->iter_end) {
      if(itNext.marker[0] == '=') {
        key = it;
        value = ++itNext;
        return TRUE;
      }
      else if(itNext.marker[0] == '{') {
        _MyIterator itBegin;
        if( ! SmartStreamUtility::FindPair(itNext, itBegin, it, (TChar*)L"{", (TChar*)L"}")) {
          return FALSE;
        }
        itNext = it;
        continue;
      }
      it = itNext;
    }
    return FALSE;
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::ATTRIBUTE::SectionName() const
  {
    _TStr str;
    return SmartStreamUtility::TranslateQuotation(pSection->name, str);
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::ATTRIBUTE::KeyName() const
  {
    _TStr str;
    return SmartStreamUtility::TranslateQuotation(key, str);
  }

  _SSP_TEMPL
    _TStr& _SSP_IMPL::ATTRIBUTE::KeyName(_TStr& str) const
  {
    return SmartStreamUtility::TranslateQuotation(key, str);
  }

  _SSP_TEMPL
    int _SSP_IMPL::ATTRIBUTE::ToInt(int nDefault) const
  {
    if(value.length) {
      _TStr str;
      return SmartStreamUtility::TranslateQuotation(value, str).ToInteger();
    }
    return nDefault;
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::ATTRIBUTE::ToString(T_LPCSTR szDefault) const
  {
    if(value.length) {
      _TStr str;
      return SmartStreamUtility::TranslateQuotation(value, str);
    }
    return szDefault;
  }

  _SSP_TEMPL
    _TStr& _SSP_IMPL::ATTRIBUTE::ToString(_TStr& str, T_LPCSTR szDefault) const
  {
    if(value.length) {
      return SmartStreamUtility::TranslateQuotation(value, str);
    }
    str = szDefault;
    return str;
  }

  _SSP_TEMPL
    float _SSP_IMPL::ATTRIBUTE::ToFloat(float fDefault) const
  {
    if(value.length) {
      _TStr str;
      return (float)SmartStreamUtility::TranslateQuotation(value, str).ToFloat();
    }
    return fDefault;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::ATTRIBUTE::ToBoolean(b32 bDefault) const
  {
    if(value.length) {
      _TStr str;
      return _CheckBoolean((T_LPCSTR)SmartStreamUtility::TranslateQuotation(value, str));
    }
    return bDefault;
  }

  //_SSP_TEMPL
  //void _SSP_IMPL::ATTRIBUTE::SetValue(T_LPCSTR str)
  //{
  //  _TStr strValue;
  //  SmartStreamUtility::MakeQuotation(strValue, str);
  //  pSection->pStock->Replace(this, value.offset(), value.length, strValue, strValue.GetLength());
  //}

  _SSP_TEMPL
  b32 _SSP_IMPL::ATTRIBUTE::operator==(const ATTRIBUTE& attr) const
  {
    ASSERT(pSection == attr.pSection);
    return ! operator!=(attr);
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::ATTRIBUTE::operator!=(const ATTRIBUTE& attr) const
  {
    ASSERT(pSection == attr.pSection);
    return key != attr.key || value != attr.value;
  }

  _SSP_TEMPL
  typename _SSP_IMPL::ATTRIBUTE& _SSP_IMPL::ATTRIBUTE::operator++()
  {
    if( ! NextKey()) {
      key = value = pSection->iter_end;
      pSection = NULL;
    }
    return *this;
  }

  //_SSP_TEMPL
  //typename _SSP_IMPL::T_LPCSTR _SSP_IMPL::ATTRIBUTE::operator=(T_LPCSTR str)
  //{
  //}

  //_SSP_TEMPL
  //int _SSP_IMPL::ATTRIBUTE::operator=(int val)
  //{
  //}

  //_SSP_TEMPL
  //float _SSP_IMPL::ATTRIBUTE::operator=(float val)
  //{
  //}

  //_SSP_TEMPL
  //unsigned int _SSP_IMPL::ATTRIBUTE::operator=(unsigned int val)
  //{
  //}

  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL 
    _SSP_IMPL::StockT()
    : m_nModify(0)
  {
  }

  _SSP_TEMPL 
    _SSP_IMPL::~StockT()
  {
    Close();
  }

  //_SSP_TEMPL 
  //  b32 _SSP_IMPL::Create(_TCh* szRoot)
  //{
  //  _TStr strBuffer = szRoot;
  //  strBuffer.Append((_TCh)L'{');
  //  strBuffer.Append((_TCh)L'\r');
  //  strBuffer.Append((_TCh)L'\n');
  //  strBuffer.Append((_TCh)L'}');
  //
  //  m_pBuffer = new clBuffer;
  //  m_Buffer.Append((CLLPVOID)&strBuffer.Front(), strBuffer.GetLength() * sizeof(_TCh));
  //  m_SmartStream.Initialize((_TCh*)m_Buffer.GetPtr(), (u32)m_Buffer.GetSize()/sizeof(_TCh));
  //  m_SmartStream.SetFlags(SmartStream::F_SYMBOLBREAK);
  //  return true;
  //}

  _SSP_TEMPL 
    b32 _SSP_IMPL::LoadW(const wch* lpProfile)
  {
    clFile file;
    if(file.OpenExistingW(lpProfile) == FALSE) {
      return FALSE;
    }

    if( ! file.ReadToBuffer(&m_Buffer)) {
      // 如果Load一个空文件，则自己创建一个缓冲
      m_Buffer.Resize(0, FALSE);
      m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), (u32)m_Buffer.GetSize()/sizeof(TChar));
      return TRUE;
    }

    m_nModify++;
    ToNativeCodec();
    m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), (u32)m_Buffer.GetSize()/sizeof(TChar));
    return TRUE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SaveW(const wch* lpProfile) const
  {
    clFile file;
    if(file.CreateAlwaysW(lpProfile) == FALSE) {
      return FALSE;
    }

    if(sizeof(TChar) == 2)
    {
      const u16 wFlag = 0xfeff;
      file.Write(&wFlag, 2);
    }

    if(m_Buffer.GetSize()) {
      file.Write(m_Buffer.GetPtr(), (u32)m_Buffer.GetSize());
    }

    return true;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::Set(BufferBase* pBuffer)
  {
    m_Buffer.Append(pBuffer->GetPtr(), pBuffer->GetSize());
    m_nModify++;
    if(ToNativeCodec()) {
      return m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), (u32)m_Buffer.GetSize()/sizeof(TChar));
    }
    return FALSE;
  }

  _SSP_TEMPL 
  b32 _SSP_IMPL::Set(T_LPCSTR str, clsize nCount)
  {
    m_Buffer.Append(str, nCount * sizeof(TChar));
    m_nModify++;
    if(ToNativeCodec()) {
      return m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), (u32)m_Buffer.GetSize()/sizeof(TChar));
    }
    return FALSE;
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::Close()
  {
    m_nModify++;
    return TRUE;
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::LoadA(const ch* lpProfile)
  {
    clStringW strProfileW = lpProfile;
    return LoadW(strProfileW);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SaveA(const ch* lpProfile) const
  {
    clStringW strProfileW = lpProfile;
    return SaveW(strProfileW);
  }


  _SSP_TEMPL
    b32 _SSP_IMPL::Remove( Section* pSect, const _MyIterator& itBegin, const _MyIterator& iter_end )
  {
    auto pStreamPtr = m_SmartStream.GetStreamPtr();
    auto nStreamEndPtr = pStreamPtr + m_SmartStream.GetStreamCount();

    auto pBegin = itBegin.marker;
    auto pEnd = iter_end.marker + iter_end.length;

    while(--pBegin >= pStreamPtr && (*pBegin == ' ' || *pBegin == '\t')) {} // 向前，跳过空白
    ++pBegin;
    while(pEnd < nStreamEndPtr && (*pEnd == ' ' || *pEnd == '\t')) { ++pEnd; } // 向后，先跳过空白
    while(pEnd < nStreamEndPtr && (*pEnd == '\r' || *pEnd == '\n')) { ++pEnd; } // 向后，再跳过换行

    return Replace(pSect, pBegin - pStreamPtr, pEnd - pBegin, NULL, 0);
  }

  //_SSP_TEMPL 
  //  b32 _SSP_IMPL::CloseSection(SECTION* desc)
  //{
  //  if(RemoveSection(desc)) {
  //    delete desc;
  //  }
  //  return true;
  //}

  //_SSP_TEMPL 
  //  typename _SSP_IMPL::SECTION* _SSP_IMPL::AddSection(SECTION* desc)
  //{
  //  ASSERT(desc->pStock != NULL);
  //  ASSERT(desc->DbgCheck());
  //  SECTION* sectNew = new SECTION;
  //  *sectNew = *desc;
  //  m_aHandles.push_back(sectNew);
  //  return sectNew;
  //}

  //_SSP_TEMPL 
  //  b32 _SSP_IMPL::RemoveSection(SECTION* desc)
  //{
  //  for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it)
  //  {
  //    if(*it == desc)
  //    {
  //      m_aHandles.erase(it);
  //      return true;
  //    }
  //  }
  //  return false;
  //}

  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL 
    void _SSP_IMPL::TrimFrontTab(clsize& uOffset)
  {
    while(uOffset > 0 && (((TChar*)m_Buffer.GetPtr())[uOffset - 1]) == (TChar)L'\t') uOffset--;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::Section::empty() const
  {
    // return this && pStock && (nModify == pStock->m_nModify) && nDepth >= 0;
    return ( ! pStock) || (nModify != pStock->m_nModify) || nDepth < 0;
  }

  _SSP_TEMPL
  _SSP_IMPL::Section::operator unspecified_bool_type() const
  {
    return empty() ? 0 : __unspecified_bool_type<Section>;
  }

  _SSP_TEMPL
    StockT<_TStr>* _SSP_IMPL::Section::GetStock() const
  {
    return pStock;
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::Section::SectionName() const
  {
    _TStr str;
    IS_OUT_OF_DATE;
    return name.pContainer == NULL ? "" : SmartStreamUtility::TranslateQuotation(name, str);
  }

  _SSP_TEMPL
  typename _SSP_IMPL::Section _SSP_IMPL::Section::Open(T_LPCSTR szSubPath) const
  {
    IS_OUT_OF_DATE;
    return pStock->OpenSection((Section*)this, szSubPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::Section _SSP_IMPL::Section::Create(T_LPCSTR szSubPath)
  {
    IS_OUT_OF_DATE;
    return pStock->CreateSection(this, szSubPath);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::NextSection(T_LPCSTR szName)
  {
    if(empty()) {
      return FALSE;
    }
    ASSERT(DbgCheck());

    auto it = iter_end;
    auto itNext = ++it;
    auto itGlobalEnd = pStock->m_SmartStream.end();

    // 已经到了末尾
    if(it == itGlobalEnd) {
      return FALSE;
    }


    while(++itNext != itGlobalEnd) {
      if((szName == NULL || it == szName) && itNext.marker[0] == '{') {
        name = it;

        // itNext应该与输出的ItBegin是同一个值
        return SmartStreamUtility::FindPair(itNext,
          iter_begin, iter_end, (TChar*)L"{", (TChar*)L"}");
      }
      else if(it.marker[0] == '}') {
        break;
      }
      it = itNext;
    }
    return FALSE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::Rename( T_LPCSTR szNewName )
  {
    if(empty() || name.marker == NULL || name.length == 0) {
      return FALSE;
    }

    return pStock->Replace(this, name.offset(), name.length, szNewName, clstd::strlenT(szNewName));
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::FirstKey( ATTRIBUTE& param ) const
  {
    if(empty()) {
      return FALSE;
    }
    ASSERT(DbgCheck());
    param.pSection = this;
    param.value = iter_begin;

    if(nDepth > 0) {
      ASSERT(param.value.marker[0] == '{');
      ++param.value;
    }
    return param.NextKey();
  }

  _SSP_TEMPL
    typename _SSP_IMPL::ATTRIBUTE _SSP_IMPL::Section::FirstKey() const
  {
    ATTRIBUTE attr;
    FirstKey(attr);
    return attr;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::Section::GetKey( T_LPCSTR szKey, ATTRIBUTE& param ) const
  {
    IS_OUT_OF_DATE;
    if( ! FirstKey(param)) {
      return FALSE;
    }

    do {
      if(param.key == szKey) {
        return TRUE;
      }
    } while(param.NextKey());
    return FALSE;
  }

  _SSP_TEMPL
    int _SSP_IMPL::Section::GetKeyAsString( T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount ) const
  {
    T_LPCSTR pSource = NULL;
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      pSource = p.value.marker;
      nCount = clMin(nCount, (int)p.value.length);
    }
    else {
      pSource = szDefault;
      if(szDefault != NULL)
        nCount = clMin(nCount, (int)clstd::strlenT(szDefault));
    }

    int n = 0;
    if(nCount != 0 && pSource != NULL)
    {
      for(int i = 0;i < nCount; i++) {
        if(pSource[i] != '\"' && pSource[i] != '\'') {
          szBuffer[n++] = pSource[i];
        }
      }
      szBuffer[n] = 0;
    }
    return n;
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::Section::GetKeyAsString( T_LPCSTR szKey, const _TStr& strDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToString();
    }
    return strDefault;
  }

  _SSP_TEMPL
    int _SSP_IMPL::Section::GetKeyAsInteger( T_LPCSTR szKey, int nDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToInt();
    }
    return nDefault;
  }

  _SSP_TEMPL
    float _SSP_IMPL::Section::GetKeyAsFloat( T_LPCSTR szKey, float fDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToFloat();
    }
    return fDefault;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::GetKeyAsBoolean( T_LPCSTR szKey, b32 bDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToBoolean();
    }
    return bDefault;
  }

  //_SSP_TEMPL
  //  int _SSP_IMPL::Section::GetKeyAsIntegerArray( T_LPCSTR szKey, clBuffer** ppBuffer ) const
  //{
  //
  //}

  _SSP_TEMPL
  b32 _SSP_IMPL::Section::SetKey( T_LPCSTR szKey, T_LPCSTR szValue )
  {
    ATTRIBUTE param;
    IS_OUT_OF_DATE;
    if(szValue == NULL || szValue[0] == L'\0') {
      return FALSE;
    }

    //ASSERT(DbgCheck());
    if(GetKey(szKey, param)) {
      _TStr strValue;
      SmartStreamUtility::MakeQuotation(strValue, szValue);
      return pStock->Replace(this, param.value.offset(), param.value.length, strValue, strValue.GetLength());
    }
    else {
      return InsertKey(szKey, szValue);
      //_TStr strBuffer;
      //strBuffer.Append(' ', nDepth);
      //strBuffer.Append(szKey);
      //strBuffer.Append('=');
      //strBuffer.Append(strValue);
      //strBuffer.Append(';');
      //strBuffer.Append('\r');
      //strBuffer.Append('\n');

      //clsize nPos = pStock->InsertString(this, itEnd, strBuffer);
      //ASSERT(DbgCheck());
    }
    return TRUE;
  }
  
  _SSP_TEMPL
    b32 _SSP_IMPL::Section::SetKey(T_LPCSTR szKey, int val)
  {
    _TStr str(val);
    return SetKey(szKey, str);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::SetKey(T_LPCSTR szKey, size_t val)
  {
    _TStr str(val);
    return SetKey(szKey, str);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::SetKey(T_LPCSTR szKey, float val)
  {
    _TStr str(val);
    return SetKey(szKey, str);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::SetKey(T_LPCSTR szKey, b32 bValue, T_LPCSTR szTrue, T_LPCSTR szFalse)
  {
    return SetKey(szKey, bValue ? szTrue : szFalse);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Section::DeleteKey( T_LPCSTR szKey )
  {
    ATTRIBUTE param;
    if(GetKey(szKey, param))
    {
      auto it = pStock->m_SmartStream.find(param.value, 1, L";");

      return pStock->Remove(this, param.key, it);

      //auto pStreamPtr = pStock->m_SmartStream.GetStreamPtr();
      //auto nStreamEndPtr = pStreamPtr + pStock->m_SmartStream.GetStreamCount();
      //auto pBegin = param.key.marker;
      //auto pEnd = it.marker + it.length;
      //while(--pBegin >= pStreamPtr && (*pBegin == ' ' || *pBegin == '\t')) {} // 向前，跳过空白
      //++pBegin;
      //while(pEnd < nStreamEndPtr && (*pEnd == ' ' || *pEnd == '\t')) { ++pEnd; } // 向后，先跳过空白
      //while(pEnd < nStreamEndPtr && (*pEnd == '\r' || *pEnd == '\n')) { ++pEnd; } // 向后，再跳过换行

      //pStock->Replace(pBegin - pStreamPtr,
      //  pEnd - pBegin, NULL, 0);
      //return TRUE;
    }
    return FALSE;
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::Section::InsertKey(T_LPCSTR szKey, T_LPCSTR val)
  {
    _TStr strValue;
    SmartStreamUtility::MakeQuotation(strValue, val);

    _TStr strBuffer;
    strBuffer.Append(' ', nDepth);
    strBuffer.Append(szKey);
    strBuffer.Append('=');
    strBuffer.Append(strValue);
    strBuffer.Append(';');
    strBuffer.Append('\r');
    strBuffer.Append('\n');

    clsize nPos = pStock->InsertString(this, iter_end, strBuffer);
    ASSERT(DbgCheck());
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL
    typename _SSP_IMPL::Section _SSP_IMPL::CreateSection( T_LPCSTR szPath )
  {
    return CreateSection(NULL, szPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::Section _SSP_IMPL::CreateSection(Section* desc, T_LPCSTR szPath)
  {
    if(desc && (desc->empty() || ! szPath || szPath[0] == '\0')) {
      return Section();
    }

    // 初始化Smart对象
    if( ! m_Buffer.GetSize()) {
      m_Buffer.Reserve(1024);
      m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), m_Buffer.GetSize() / sizeof(TChar));
    }

    Section sDesc;
    Section sResult;

    if(desc) {
      sDesc = *desc;
    }
    else {
      new(&sDesc) Section(this);
    }

    if( ! szPath || szPath[0] == '\0') {
      return sDesc;
    }

    clstd::StringCutter<_TStr> rsc(szPath);
    _TStr strSectName;
    do {
      rsc.Cut(strSectName, '/');
      sDesc.pParent = desc;
      if(rsc.IsEndOfString()) {
        NewSection(&sDesc, strSectName, sResult);
        sResult.pParent = desc;
        return sResult;
        //return AddSection(&sResult);
      }
      else if( ! FindSingleSection(&sDesc, strSectName, sResult)) {
        NewSection(&sDesc, strSectName, sResult);
      }
      sDesc = sResult;
    } while(1);
    return Section();
  }

  _SSP_TEMPL
    typename _SSP_IMPL::Section _SSP_IMPL::OpenSection( T_LPCSTR szPath ) const
  {
    return OpenSection(NULL, szPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::Section _SSP_IMPL::OpenSection(Section* desc, T_LPCSTR szPath) const
  {
    // sect != NULL
    // OpenChild(NULL, NULL); 返回根Section
    // OpenChild(sect, NULL); 返回sect下的第一个Section
    // OpenChild(NULL, "sect1/sect0"); 返回根下的"sect1/sect0"

    if(desc && (desc->empty() || desc->pStock != this)) {
      return Section();
    }

    if((szPath != NULL && szPath[0] == L'\0') || ! m_Buffer.GetSize())
    {
      return Section((StockT*)this);
    }

    // FIXME: 目前重名Section包含不同子Section，查找只会匹配第一个找到的Section

    Section sDesc;
    Section sResult;

    if(desc && ( ! desc->empty()))
    {
      sDesc = *desc;
      if(szPath == NULL)
      {
        if( ! FindSingleSection(&sDesc, NULL, sResult)) { // sect下的第一个Section
          return Section();
        }
        //return AddSection(&sResult);
        sResult.pParent = desc;
        return sResult;
      }
    }
    else {
      new(&sDesc) Section((StockT*)this);
      if(szPath == NULL) {
        //return AddSection(&sDesc); // 根Section
        return sDesc;
      }
    }

    clstd::StringCutter<_TStr> rsc(szPath);
    _TStr strSectName;
    while( ! rsc.IsEndOfString())
    {
      rsc.Cut(strSectName, '/');

      // 这里不必 sResult.pStock = NULL;
      if( ! FindSingleSection(&sDesc, strSectName, sResult)) {
        return Section();
      }
      sDesc = sResult;
    };

    ASSERT(sResult.pStock);
    sResult.pParent = desc;
    return sResult;
    //return sResult.pStock ? AddSection(&sResult) : NULL;
    //return sResult.pStock ? sResult : NULL;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::DeleteSection( T_LPCSTR szPath )
  {
    // 1. 删除对应的Section
    // 2. 如果在Section列表查找到了删除的Section，调整

    clstd::StringCutter<_TStr> rsc(szPath);
    Section sDesc(this);
    Section sResult;

    _TStr strSectName;

    while( ! rsc.IsEndOfString())
    {
      rsc.Cut(strSectName, '/');
      if( ! FindSingleSection(&sDesc, strSectName, sResult)) {
        return FALSE;
      }
      sDesc = sResult;
    };

    if(sResult.pStock) {
      sResult.pParent = NULL;
      return Remove(&sResult, sResult.name, sResult.iter_end);
      //return Replace(sResult.section.offset(), sResult.itEnd.offset() - sResult.section.offset() + sResult.itEnd.length, NULL, 0);
    }
    return FALSE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::RelocateIterator(_MyIterator& it, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert)
  {
    // uActPos是数据发生变化的位置，发生变化指的是插入或者删除操作

    clsize uMyPos = it.marker - lpOldPtr;

    // 如果 iterator 持有的指针在变化位置之前，则直接修正指针
    if(uMyPos < uActPos)
    {
      it.marker = lpNewPtr + uMyPos;
      ASSERT(uMyPos + it.length < uActPos); // 改变的位置不应该在 iterator 中
      return true;
    }
    else if(uActPos + sizeReplaced > uMyPos) // 变化区间包含了这个Iterator，Iterator设置为失效
    {
      it.marker = NULL;
      it.length = 0;
      return false;
    }

    it.marker = lpNewPtr + uMyPos - sizeReplaced + sizeInsert;
    return true;
  }

  // uPos 是字符位置,不是字节位置
  _SSP_TEMPL b32 _SSP_IMPL::RelocateSection(Section* pSection, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert)
  {
    //for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it)
    //{
    //  Section* pSection = (Section*)*it;
      if(( ! RelocateIterator(pSection->iter_begin, lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) ||
        ( ! RelocateIterator(pSection->iter_end,    lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) ||
        ( ! RelocateIterator(pSection->name,        lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) )
      {
        // 如果改变的区间与Iterator范围重叠，则使这个Section失效
        pSection->nDepth = -1;
        return FALSE;
      }
      ASSERT(pSection->DbgCheck());
    //}
    return TRUE;
  }

  _SSP_TEMPL
    void _SSP_IMPL::ReverseByteOrder16(u16* ptr, clsize nCount)
  {
    for(clsize i = 0; i < nCount; i++) {
      *ptr = (*ptr >> 8) | (*ptr << 8); // 不确定是循环位移还是位移,但是都不需要&操作.
      ptr++;
    }
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::ToNativeCodec()
  {
    u32 dwBOM = *(u32*)m_Buffer.GetPtr();
    if(sizeof(TChar) == 1)
    {
      b32 bUnicode = FALSE;
      if((dwBOM & 0xFFFF) == BOM_UNICODE)   {  // UNICODE 格式头
        m_Buffer.Replace(0, 2, NULL, 0);
        bUnicode = TRUE;
      }
      else if((dwBOM & 0xFFFF) == BOM_UNICODE_BIGENDIAN)
      {
        m_Buffer.Replace(0, 2, NULL, 0);

        // 以16字节方式交换高低字节
        u16* ptr = (u16*)m_Buffer.GetPtr();
        clsize nCount = m_Buffer.GetSize() / sizeof(u16);
        ReverseByteOrder16(ptr, nCount);
        bUnicode = TRUE;
      }
      else if((dwBOM & 0xFFFFFF) == BOM_UTF8)
      {
        CLBREAK; // FIXME: 暂时不支持
        bUnicode = TRUE;
        return FALSE;
      }

      if(bUnicode)
      {
        CLBREAK; // TODO: 测试过这段代码后去掉 CLBREAK
        wch chEnd[] = L"\0";
        Buffer sNewBuffer;
        m_Buffer.Append(chEnd, sizeof(chEnd));
        sNewBuffer.Resize(m_Buffer.GetSize(), FALSE);

        clsize size = wcstombs((char*)sNewBuffer.GetPtr(), (const wchar_t*)m_Buffer.GetPtr(), sNewBuffer.GetSize());
        ASSERT(size != (clsize)-1); // TODO: 需要 setlocale(LC_ALL,"");

        //sNewBuffer.Resize(size, FALSE);
        //delete m_pBuffer;
        //m_pBuffer = pNewBuffer;
        //m_Buffer.Resize(size);
        m_Buffer.Replace(0, m_Buffer.GetSize(), sNewBuffer.GetPtr(), size);
      }
    }
    else if(sizeof(TChar) == 2)  // UNICODE
    {
      if((dwBOM & 0xFFFF) == BOM_UNICODE)   {  // UNICODE 格式头
        m_Buffer.Replace(0, 2, NULL, 0);
      }
      else if((dwBOM & 0xFFFF) == BOM_UNICODE_BIGENDIAN)
      {
        m_Buffer.Replace(0, 2, NULL, 0);

        // 以16字节方式交换高低字节
        u16* ptr = (u16*)m_Buffer.GetPtr();
        clsize nCount = m_Buffer.GetSize() / sizeof(u16);
        ReverseByteOrder16(ptr, nCount);
      }
      else if((dwBOM & 0xFFFFFF) == BOM_UTF8)
      {
        CLBREAK; // FIXME: 暂时不支持
        return FALSE;
      }
      else {
        ch chEnd[] = "\0";
        Buffer sNewBuffer;
        m_Buffer.Append(chEnd, sizeof(chEnd));
        sNewBuffer.Resize(m_Buffer.GetSize() * 2, FALSE);

        clsize size = mbstowcs((wchar_t*)sNewBuffer.GetPtr(), (const char*)m_Buffer.GetPtr(), m_Buffer.GetSize());

        //sNewBuffer.Resize(size * sizeof(TChar), FALSE);
        //delete m_pBuffer;
        //m_pBuffer = pNewBuffer;
        m_Buffer.Replace(0, m_Buffer.GetSize(), sNewBuffer.GetPtr(), size * sizeof(TChar));
      }
    }
    return TRUE;
  }



  //////////////////////////////////////////////////////////////////////////

  //
  //
  //
  //template<class _STR>
  //_STR FromProfileString(const _STR& str)
  //{
  //  size_t size = str.GetLength();
  //  if(size < 1)
  //    return str;
  //  size_t top = 0;
  //  if(str.Front() == _T('\"')) {
  //    size--;
  //    top++;
  //  }
  //  if(str.Back() == _T('\"')) {
  //    size--;
  //  }
  //
  //  _STR strBackslash;
  //  strBackslash.Append('\\');
  //  strBackslash.Append('\"');
  //
  //  _STR strTemp = str.SubString(top, size);
  //  clsize nStart = 0;
  //  while((nStart = strTemp.Find(strBackslash, nStart)) != _STR::npos)
  //  {
  //    strTemp.Remove(nStart, 1);
  //  }
  //  return strTemp;
  //}
  //
  //template<class _STR>
  //_STR ToProfileString(const _STR& str)
  //{
  //  _STR strTemp = "\"";
  //  _STR strQuot;
  //  _STR strBackslash;
  //  size_t size = str.GetLength();
  //  if(size < 1)
  //    return str;
  //
  //  strTemp += str;
  //  clsize nStart = 1;
  //
  //  while((nStart = strTemp.Find('\"', nStart)) != _STR::npos)
  //  {
  //    strTemp.Insert(nStart, '\\');
  //    nStart += 2;
  //  }
  //
  //  strTemp += '\"';
  //
  //  return strTemp;
  //}

  _SSP_TEMPL
    b32 _SSP_IMPL::NewSection(Section* pSection, T_LPCSTR szName, Section& pNewSect )
  {
    ASSERT(pSection != NULL && pSection->DbgCheck());
    _TStr strBuffer;
    static TChar szSectBegin[] = {'{', '\r', '\n'};
    static TChar szSectEnd[]   = {'}', '\r', '\n'};
    strBuffer.Append(' ', pSection->nDepth).Append(szName)
      .Append(szSectBegin).Append(' ', pSection->nDepth).Append(szSectEnd);

    clsize nPos = InsertString(pSection, pSection->iter_end, strBuffer);

    pNewSect.pStock  = this;
    pNewSect.pParent = NULL;
    pNewSect.nModify = m_nModify;
    pNewSect.name    = m_SmartStream.nearest(nPos);
    pNewSect.nDepth  = pSection->nDepth + 1;
    SmartStreamUtility::FindPair(pNewSect.name, pNewSect.iter_begin, pNewSect.iter_end, (TChar*)L"{", (TChar*)L"}");
    return TRUE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::FindSingleSection(Section* pFindSect, T_LPCSTR szName, Section& pOutSect ) const /* szName为NULL表示查找任何Section */
  {
    if(pFindSect->iter_begin == pFindSect->iter_end) {
      return FALSE;
    }

    ASSERT(pFindSect->DbgCheck());

    auto it = pFindSect->iter_begin;
    auto itNext = pFindSect->nDepth == 0 ? it : ++it; // 如果是根，itBegin指向的就不是'{'标记，否则跳过'{'标记

    if(itNext == pFindSect->iter_end) { return FALSE; }

    while(++itNext != pFindSect->iter_end) {
      if((szName == NULL || it == szName) && itNext.marker[0] == '{') {
        pOutSect.pStock  = (StockT*)this;
        pOutSect.pParent = pFindSect;
        pOutSect.nModify = m_nModify;
        pOutSect.name    = it;
        pOutSect.nDepth  = pFindSect->nDepth + 1;

        // itNext应该与输出的ItBegin是同一个值
        return SmartStreamUtility::FindPair(itNext, pOutSect.iter_begin, pOutSect.iter_end, (TChar*)L"{", (TChar*)L"}");
      }
      it = itNext;
    }
    return FALSE;
  }

  _SSP_TEMPL
    clsize _SSP_IMPL::InsertString(Section* pSect, const _MyIterator& it, const _TStr& str)
  {
    clsize nPos;
    if(it == m_SmartStream.end()) {
      nPos = m_Buffer.GetSize();
      //Append(str, str.GetLength());
      Replace(pSect, -1, 0, str, str.GetLength());
    }
    else {
      auto pStreamPtr = m_SmartStream.GetStreamPtr();
      auto pMarker = it.marker;

      // 用来向前跳过空白，尽量插入的Section在上一个换行符号之后
      while(--pMarker >= pStreamPtr && (*pMarker == ' ' || *pMarker == '\t')) {}
      nPos = pMarker - pStreamPtr + 1;
      ASSERT(nPos < m_Buffer.GetSize());
      //Insert(nPos, str, str.GetLength());
      Replace(pSect, nPos, 0, str, str.GetLength());
    }
    return nPos;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Replace(Section* pSect, clsize nPos, clsize nReplaced, T_LPCSTR szText, clsize nCount)
  {
    if(nPos == (clsize)-1) {
      nPos = m_SmartStream.GetStreamCount();
    }

    TChar* pOldPtr = (TChar*)m_Buffer.GetPtr();
    const clsize cbSize = nCount * sizeof(TChar);
    const clsize cbReplaced = nReplaced * sizeof(TChar);
    const clsize cbPos = nPos * sizeof(TChar);

    m_Buffer.Replace(cbPos, cbReplaced, (CLLPCVOID)szText, cbSize);

    // 重新设置SmartStream：指针变化，大小变化都要重新初始化
    m_SmartStream.Attach((TChar*)m_Buffer.GetPtr(), m_Buffer.GetSize() / sizeof(TChar));

    // m_SmartStream.Initialize()之后会修正end()，RelocateSection内部断言检查需要准确的end()
    m_nModify++;
    do {
      RelocateSection(pSect, pOldPtr, (TChar*)m_Buffer.GetPtr(), nPos, nReplaced, nCount);
      pSect->nModify = m_nModify;
      pSect = pSect->pParent;
    } while(pSect);

    return true;
  }

  //_SSP_TEMPL
  //  b32 _SSP_IMPL::Insert( clsize nPos, T_LPCSTR szText, clsize nCount )
  //{
  //  return Replace(nPos, 0, szText, nCount);
  //}

  //_SSP_TEMPL
  //  b32 _SSP_IMPL::Append( T_LPCSTR szText, clsize nCount )
  //{
  //  return Replace(-1, 0, szText, nCount);
  //}

  _SSP_TEMPL
  typename _SSP_IMPL::T_LPCSTR _SSP_IMPL::GetText(clsize* length) const
  {
    if(length) {
      *length = m_Buffer.GetSize() / sizeof(TChar);
    }
    return (T_LPCSTR)m_Buffer.GetPtr();
  }


  //////////////////////////////////////////////////////////////////////////
  _SSP_TEMPL
    _SSP_IMPL::Section::Section()
    : pStock(NULL)
  {
  }

  _SSP_TEMPL
  _SSP_IMPL::Section::Section(StockT* pCoStock)
    : pStock  (pCoStock)
    , pParent (NULL)
    , nModify (pCoStock->m_nModify)
    , nDepth  (0)
    , iter_begin(pCoStock->m_SmartStream.begin())
    , iter_end(pCoStock->m_SmartStream.end())
  {}

  //_SSP_TEMPL
  //_SSP_IMPL::Section::Section(Section& desc)
  //  : m_desc(desc)
  //{
  //}

  _SSP_TEMPL
  _SSP_IMPL::Section::~Section()
  {
    clear();
  }

  _SSP_TEMPL
    void _SSP_IMPL::Section::clear()
  {
    pStock = NULL;
    pParent = NULL;
  }

  //_SSP_TEMPL
  //typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator=(SECTION* desc)
  //{
  //  Close();
  //  m_desc = desc;
  //  return m_desc;
  //}

  //_SSP_TEMPL
  //typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator->() const
  //{
  //  return m_desc;
  //}

  //_SSP_TEMPL
  //  typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator&() const
  //{
  //  return m_desc;
  //}

  //_SSP_TEMPL
  //typename _SSP_IMPL::SECTION& _SSP_IMPL::Section::operator*() const
  //{
  //  return *m_desc;
  //}

  _SSP_TEMPL
  typename _SSP_IMPL::ATTRIBUTE _SSP_IMPL::Section::operator[](T_LPCSTR name) const
  {
    ATTRIBUTE attr;
    if( ! GetKey(name, attr)) {
      attr.pSection = NULL;
      attr.key.length = 0;
      attr.value.length = 0;
    }
    return attr;
  }

  _SSP_TEMPL
    void _SSP_IMPL::Section::operator++()
  {
    if( ! NextSection(NULL)) {
      clear();
    }
  }

  template class StockT<clStringA>;
  template class StockT<clStringW>;

} // namespace clstd
