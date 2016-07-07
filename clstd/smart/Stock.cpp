#include <stdlib.h>

#include "../clstd.h"
#include "../clmemory.h"
#include "../clString.H"
#include "../clFile.H"
#include "../clBuffer.H"
#include "smartstream.h"
#include "Stock.h"
#include "../clUtility.H"

namespace clstd
{
  //////////////////////////////////////////////////////////////////////////
  //
  // 显式声明模板类
  //
  template class StockT<clStringA>;
  template class StockT<clStringW>;

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
    b32 _SSP_IMPL::ATTRIBUTE::NextKey()
  {
    if(pSection == NULL) {
      return FALSE;
    }

    auto it = itValue;
    auto itNext = it;

    if(itNext == pSection->itEnd) { return FALSE; }

    while(++itNext != pSection->itEnd) {
      if(itNext.marker[0] == '=') {
        itKey = it;
        itValue = ++itNext;
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
    return SmartStreamUtility::TranslateQuotation(pSection->itSectionName, str);
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::ATTRIBUTE::KeyName() const
  {
    _TStr str;
    return SmartStreamUtility::TranslateQuotation(itKey, str);
  }

  _SSP_TEMPL
    _TStr& _SSP_IMPL::ATTRIBUTE::KeyName(_TStr& str) const
  {
    return SmartStreamUtility::TranslateQuotation(itKey, str);
  }

  _SSP_TEMPL
    int _SSP_IMPL::ATTRIBUTE::ToInt() const
  {
    return ToString().ToInteger();
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::ATTRIBUTE::ToString() const
  {
    _TStr str;
    return SmartStreamUtility::TranslateQuotation(itValue, str);
  }

  _SSP_TEMPL
    _TStr& _SSP_IMPL::ATTRIBUTE::ToString(_TStr& str) const
  {
    return SmartStreamUtility::TranslateQuotation(itValue, str);
  }

  _SSP_TEMPL
    float _SSP_IMPL::ATTRIBUTE::ToFloat() const
  {
    return (float)ToString().ToFloat();
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::ATTRIBUTE::ToBoolean() const
  {
    _TStr str;
    SmartStreamUtility::TranslateQuotation(itValue, str);
    return _CheckBoolean((T_LPCSTR)str);
  }

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
    return itKey != attr.itKey || itValue != attr.itValue;
  }

  _SSP_TEMPL
  typename _SSP_IMPL::ATTRIBUTE& _SSP_IMPL::ATTRIBUTE::operator++()
  {
    if( ! NextKey()) {
      itKey = itValue = pSection->itEnd;
    }
    return *this;
  }

  //////////////////////////////////////////////////////////////////////////


  _SSP_TEMPL 
    _SSP_IMPL::StockT()
    : m_pBuffer(NULL)
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
  //  m_pBuffer->Append((CLLPVOID)&strBuffer.Front(), strBuffer.GetLength() * sizeof(_TCh));
  //  m_SmartStream.Initialize((_TCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(_TCh));
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

    if(file.MapToBuffer(&m_pBuffer) == FALSE) {
      // 如果Load一个空文件，则自己创建一个缓冲
      ASSERT(m_pBuffer == NULL);
      m_pBuffer = new Buffer();
      m_pBuffer->Resize(0, FALSE);
      m_SmartStream.Initialize((TChar*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(TChar));
      return TRUE;
    }

    u32 dwBOM = *(u32*)m_pBuffer->GetPtr();
    if(sizeof(TChar) == 1)
    {
      b32 bUnicode = FALSE;
      if((dwBOM & 0xFFFF) == BOM_UNICODE)   {  // UNICODE 格式头
        m_pBuffer->Replace(0, 2, NULL, 0);
        bUnicode = TRUE;
      }
      else if((dwBOM & 0xFFFF) == BOM_UNICODE_BIGENDIAN)
      {
        m_pBuffer->Replace(0, 2, NULL, 0);

        // 以16字节方式交换高低字节
        u16* ptr = (u16*)m_pBuffer->GetPtr();
        clsize nCount = m_pBuffer->GetSize() / sizeof(u16);
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
        Buffer* pNewBuffer = new Buffer;
        m_pBuffer->Append(chEnd, sizeof(chEnd));
        pNewBuffer->Resize(m_pBuffer->GetSize(), FALSE);

        clsize size = wcstombs((char*)pNewBuffer->GetPtr(), (const wchar_t*)m_pBuffer->GetPtr(), pNewBuffer->GetSize());

        pNewBuffer->Resize(size, FALSE);
        delete m_pBuffer;
        m_pBuffer = pNewBuffer;
      }
    }
    else if(sizeof(TChar) == 2)  // UNICODE
    {
      if((dwBOM & 0xFFFF) == BOM_UNICODE)   {  // UNICODE 格式头
        m_pBuffer->Replace(0, 2, NULL, 0);
      }
      else if((dwBOM & 0xFFFF) == BOM_UNICODE_BIGENDIAN)
      {
        m_pBuffer->Replace(0, 2, NULL, 0);

        // 以16字节方式交换高低字节
        u16* ptr = (u16*)m_pBuffer->GetPtr();
        clsize nCount = m_pBuffer->GetSize() / sizeof(u16);
        ReverseByteOrder16(ptr, nCount);
      }
      else if((dwBOM & 0xFFFFFF) == BOM_UTF8)
      {
        CLBREAK; // FIXME: 暂时不支持
        return FALSE;
      }
      else {
        ch chEnd[] = "\0";
        Buffer* pNewBuffer = new Buffer;
        m_pBuffer->Append(chEnd, sizeof(chEnd));
        pNewBuffer->Resize(m_pBuffer->GetSize() * 2, FALSE);

        clsize size = mbstowcs((wchar_t*)pNewBuffer->GetPtr(), (const char*)m_pBuffer->GetPtr(), m_pBuffer->GetSize());

        pNewBuffer->Resize(size * sizeof(TChar), FALSE);
        delete m_pBuffer;
        m_pBuffer = pNewBuffer;
      }
    }
    m_SmartStream.Initialize((TChar*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(TChar));
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

    if(m_pBuffer) {
      file.Write(m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize());
    }

    return true;
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::Close()
  {
    // 没有释放所有Handle
    ASSERT(m_aHandles.size() == 0);
    for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it) {
      SAFE_DELETE(*it);
    }
    m_aHandles.clear();
    SAFE_DELETE(m_pBuffer);
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
    b32 _SSP_IMPL::Remove( const _MyIterator& itBegin, const _MyIterator& itEnd )
  {
    auto pStreamPtr = m_SmartStream.GetStreamPtr();
    auto nStreamEndPtr = pStreamPtr + m_SmartStream.GetStreamCount();

    auto pBegin = itBegin.marker;
    auto pEnd = itEnd.marker + itEnd.length;

    while(--pBegin >= pStreamPtr && (*pBegin == ' ' || *pBegin == '\t')) {} // 向前，跳过空白
    ++pBegin;
    while(pEnd < nStreamEndPtr && (*pEnd == ' ' || *pEnd == '\t')) { ++pEnd; } // 向后，先跳过空白
    while(pEnd < nStreamEndPtr && (*pEnd == '\r' || *pEnd == '\n')) { ++pEnd; } // 向后，再跳过换行

    return Replace(pBegin - pStreamPtr, pEnd - pBegin, NULL, 0);
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::CloseSection(SECTION* desc)
  {
    if(RemoveSection(desc)) {
      delete desc;
    }
    return true;
  }

  _SSP_TEMPL 
    typename _SSP_IMPL::SECTION* _SSP_IMPL::AddSection(SECTION* desc)
  {
    ASSERT(desc->pStock != NULL);
    ASSERT(desc->DbgCheck());
    SECTION* sectNew = new SECTION;
    *sectNew = *desc;
    m_aHandles.push_back(sectNew);
    return sectNew;
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::RemoveSection(SECTION* desc)
  {
    for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it)
    {
      if(*it == desc)
      {
        m_aHandles.erase(it);
        return true;
      }
    }
    return false;
  }

  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL 
    void _SSP_IMPL::TrimFrontTab(clsize& uOffset)
  {
    while(uOffset > 0 && (((TChar*)m_pBuffer->GetPtr())[uOffset - 1]) == (TChar)L'\t') uOffset--;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::IsValid() const
  {
    return this && nDepth >= 0;
  }

  _SSP_TEMPL
    _TStr _SSP_IMPL::SECTION::SectionName() const
  {
    _TStr str;
    return itSectionName.pContainer == NULL ? "" : SmartStreamUtility::TranslateQuotation(itSectionName, str);
  }

  _SSP_TEMPL
  typename _SSP_IMPL::SECTION* _SSP_IMPL::SECTION::Open(T_LPCSTR szSubPath)
  {
    return pStock->Open(this, szSubPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::SECTION::Create(T_LPCSTR szSubPath)
  {
    return pStock->Create(this, szSubPath);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::NextSection(T_LPCSTR szName)
  {
    if( ! IsValid()) {
      return FALSE;
    }
    ASSERT(DbgCheck());

    auto it = itEnd;
    auto itNext = ++it;
    auto itGlobalEnd = pStock->m_SmartStream.end();

    // 已经到了末尾
    if(it == itGlobalEnd) {
      return FALSE;
    }


    while(++itNext != itGlobalEnd) {
      if((szName == NULL || it == szName) && itNext.marker[0] == '{') {
        itSectionName = it;

        // itNext应该与输出的ItBegin是同一个值
        return SmartStreamUtility::FindPair(itNext,
          itBegin, itEnd, (TChar*)L"{", (TChar*)L"}");
      }
      else if(it.marker[0] == '}') {
        break;
      }
      it = itNext;
    }
    return FALSE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::Rename( T_LPCSTR szNewName )
  {
    if( ! IsValid() || itSectionName.marker == NULL || itSectionName.length == 0) {
      return FALSE;
    }

    return pStock->Replace(itSectionName.offset(), itSectionName.length, szNewName, clstd::strlenT(szNewName));
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::FirstKey( ATTRIBUTE& param ) const
  {
    if( ! IsValid()) {
      return FALSE;
    }
    ASSERT(DbgCheck());
    param.pSection = this;
    param.itValue = itBegin;

    if(nDepth > 0) {
      ASSERT(param.itValue.marker[0] == '{');
      ++param.itValue;
    }
    return param.NextKey();
  }


  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::GetKey( T_LPCSTR szKey, ATTRIBUTE& param ) const
  {
    if( ! FirstKey(param)) {
      return FALSE;
    }

    do {
      if(param.itKey == szKey) {
        return TRUE;
      }
    } while(param.NextKey());
    return FALSE;
  }

  _SSP_TEMPL
    int _SSP_IMPL::SECTION::GetKeyAsString( T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount ) const
  {
    T_LPCSTR pSource = NULL;
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      pSource = p.itValue.marker;
      nCount = clMin(nCount, (int)p.itValue.length);
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
    _TStr _SSP_IMPL::SECTION::GetKeyAsString( T_LPCSTR szKey, const _TStr& strDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToString();
    }
    return strDefault;
  }

  _SSP_TEMPL
    int _SSP_IMPL::SECTION::GetKeyAsInteger( T_LPCSTR szKey, int nDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToInt();
    }
    return nDefault;
  }

  _SSP_TEMPL
    float _SSP_IMPL::SECTION::GetKeyAsFloat( T_LPCSTR szKey, float fDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToFloat();
    }
    return fDefault;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::GetKeyAsBoolean( T_LPCSTR szKey, b32 bDefault ) const
  {
    ATTRIBUTE p;
    if(GetKey(szKey, p)) {
      return p.ToBoolean();
    }
    return bDefault;
  }

  //_SSP_TEMPL
  //  int _SSP_IMPL::SECTION::GetKeyAsIntegerArray( T_LPCSTR szKey, clBuffer** ppBuffer ) const
  //{
  //
  //}

  _SSP_TEMPL
    b32 _SSP_IMPL::SECTION::SetKey( T_LPCSTR szKey, T_LPCSTR szValue )
  {
    ATTRIBUTE param;

    if(szValue == NULL || szValue[0] == L'\0') {
      return FALSE;
    }

    //ASSERT(DbgCheck());
    _TStr strValue;
    SmartStreamUtility::MakeQuotation(strValue, szValue);

    if(GetKey(szKey, param)) {
      return pStock->Replace(param.itValue.offset(), 
        param.itValue.length, strValue, strValue.GetLength());
    }
    else {
      _TStr strBuffer;
      strBuffer.Append(' ', nDepth);
      strBuffer.Append(szKey);
      strBuffer.Append('=');
      strBuffer.Append(strValue);
      strBuffer.Append(';');
      strBuffer.Append('\r');
      strBuffer.Append('\n');

      clsize nPos = pStock->InsertString(itEnd, strBuffer);
      ASSERT(DbgCheck());
    }
    return TRUE;
  }

  _SSP_TEMPL 
    b32 _SSP_IMPL::SECTION::DeleteKey( T_LPCSTR szKey )
  {
    ATTRIBUTE param;
    if(GetKey(szKey, param))
    {
      auto it = pStock->m_SmartStream.find(param.itValue, 1, L";");

      return pStock->Remove(param.itKey, it);

      //auto pStreamPtr = pStock->m_SmartStream.GetStreamPtr();
      //auto nStreamEndPtr = pStreamPtr + pStock->m_SmartStream.GetStreamCount();
      //auto pBegin = param.itKey.marker;
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

  //////////////////////////////////////////////////////////////////////////

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::Create( T_LPCSTR szPath )
  {
    return Create(NULL, szPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::Create(SECTION* desc, T_LPCSTR szPath)
  {
    if(szPath == NULL) {
      return NULL;
    }

    // 初始化Smart对象
    if(m_pBuffer == NULL) {
      m_pBuffer = new Buffer();

      if(m_pBuffer == NULL) {
        CLOG_ERROR("%s: Out of memory.\r\n", __FUNCTION__);
        return NULL;
      }
      m_pBuffer->Reserve(1024);
      m_SmartStream.Initialize((TChar*)m_pBuffer->GetPtr(), m_pBuffer->GetSize() / sizeof(TChar));
    }

    clstd::StringCutter<_TStr> rsc(szPath);
    SECTION sDesc;
    SECTION sResult;

    if(desc) {
      sDesc = *desc;
    }
    else {
      new(&sDesc) SECTION(this);
    }

    _TStr strSectName;
    do {
      rsc.Cut(strSectName, '/');
      if(rsc.IsEndOfString()) {
        NewSection(&sDesc, strSectName, &sResult);
        return AddSection(&sResult);
      }
      else if( ! FindSigleSection(&sDesc, strSectName, &sResult)) {
        NewSection(&sDesc, strSectName, &sResult);
      }
      sDesc = sResult;
    } while(1);
    return NULL;
  }

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::Open( T_LPCSTR szPath )
  {
    return Open(NULL, szPath);
  }

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::Open(SECTION* desc, T_LPCSTR szPath)
  {
    // sect != NULL
    // OpenChild(NULL, NULL); 返回根Section
    // OpenChild(sect, NULL); 返回sect下的第一个Section
    // OpenChild(NULL, "sect1/sect0"); 返回根下的"sect1/sect0"

    if((szPath != NULL && szPath[0] == L'\0') || m_pBuffer == NULL || 
      (desc != NULL && desc->pStock != this))
    {
      return NULL;
    }

    // FIXME: 目前重名Section包含不同子Section，查找只会匹配第一个找到的Section

    SECTION sDesc;
    SECTION sResult;

    if(desc)
    {
      sDesc = *desc;
      if(szPath == NULL)
      {
        if( ! FindSigleSection(&sDesc, NULL, &sResult)) { // sect下的第一个Section
          return NULL;
        }
        return AddSection(&sResult);
      }
    }
    else {
      new(&sDesc) SECTION(this);
      if(szPath == NULL) {
        return AddSection(&sDesc); // 根Section
      }
    }

    clstd::StringCutter<_TStr> rsc(szPath);
    _TStr strSectName;
    while( ! rsc.IsEndOfString())
    {
      rsc.Cut(strSectName, '/');

      // 这里不必 sResult.pStock = NULL;
      if( ! FindSigleSection(&sDesc, strSectName, &sResult)) {
        return NULL;
      }
      sDesc = sResult;
    };
    return sResult.pStock ? AddSection(&sResult) : NULL;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::DeleteSection( T_LPCSTR szPath )
  {
    // 1. 删除对应的Section
    // 2. 如果在Section列表查找到了删除的Section，调整

    clstd::StringCutter<_TStr> rsc(szPath);
    SECTION sDesc(this);
    SECTION sResult;

    _TStr strSectName;

    while( ! rsc.IsEndOfString())
    {
      rsc.Cut(strSectName, '/');
      if( ! FindSigleSection(&sDesc, strSectName, &sResult)) {
        return FALSE;
      }
      sDesc = sResult;
    };

    if(sResult.pStock) {
      return Remove(sResult.itSectionName, sResult.itEnd);
      //return Replace(sResult.itSectionName.offset(), sResult.itEnd.offset() - sResult.itSectionName.offset() + sResult.itEnd.length, NULL, 0);
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
  _SSP_TEMPL b32 _SSP_IMPL::RelocateSection(T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert)
  {
    for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it)
    {
      SECTION* pSection = (SECTION*)*it;
      if(( ! RelocateIterator(pSection->itBegin,        lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) ||
        ( ! RelocateIterator(pSection->itEnd,          lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) ||
        ( ! RelocateIterator(pSection->itSectionName,  lpOldPtr, lpNewPtr, uActPos, sizeReplaced, sizeInsert)) )
      {
        // 如果改变的区间与Iterator范围重叠，则使这个Section失效
        pSection->nDepth = -1;
        return FALSE;
      }
      ASSERT(pSection->DbgCheck());
    }
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
    b32 _SSP_IMPL::NewSection( const SECTION* pSection, T_LPCSTR szName, SECTION* pNewSect )
  {
    ASSERT(pSection != NULL && pSection->DbgCheck());
    _TStr strBuffer;
    //strBuffer.Format("%s {\r\n};\r\n", szName);
    strBuffer.Append(' ', pSection->nDepth);
    strBuffer.Append(szName);
    strBuffer.Append('{');
    strBuffer.Append('\r');
    strBuffer.Append('\n');
    strBuffer.Append(' ', pSection->nDepth);
    strBuffer.Append('}');
    strBuffer.Append('\r');
    strBuffer.Append('\n');

    clsize nPos = InsertString(pSection->itEnd, strBuffer);

    pNewSect->pStock        = this;
    pNewSect->itSectionName = m_SmartStream.nearest(nPos);
    pNewSect->nDepth        = pSection->nDepth + 1;
    SmartStreamUtility::FindPair(pNewSect->itSectionName,
      pNewSect->itBegin, pNewSect->itEnd, (TChar*)L"{", (TChar*)L"}");
    return TRUE;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::FindSigleSection( const SECTION* pFindSect, T_LPCSTR szName, SECTION* pOutSect ) /* szName为NULL表示查找任何Section */
  {
    ASSERT(pFindSect != NULL);

    if(pFindSect->itBegin == pFindSect->itEnd) {
      return FALSE;
    }

    ASSERT(pFindSect->DbgCheck());

    auto it = pFindSect->itBegin;
    auto itNext = pFindSect->nDepth == 0 ? it : ++it; // 如果是根，itBegin指向的就不是'{'标记，否则跳过'{'标记

    if(itNext == pFindSect->itEnd) { return FALSE; }

    while(++itNext != pFindSect->itEnd) {
      if((szName == NULL || it == szName) && itNext.marker[0] == '{') {
        pOutSect->pStock = this;
        pOutSect->itSectionName = it;
        pOutSect->nDepth = pFindSect->nDepth + 1;

        // itNext应该与输出的ItBegin是同一个值
        return SmartStreamUtility::FindPair(itNext,
          pOutSect->itBegin, pOutSect->itEnd, (TChar*)L"{", (TChar*)L"}");
      }
      it = itNext;
    }
    return FALSE;
  }

  _SSP_TEMPL
    clsize _SSP_IMPL::InsertString( const _MyIterator& it, const _TStr& str )
  {
    clsize nPos;
    if(it == m_SmartStream.end()) {
      nPos = m_pBuffer->GetSize();
      Append(str, str.GetLength());
    }
    else {
      auto pStreamPtr = m_SmartStream.GetStreamPtr();
      auto pMarker = it.marker;

      // 用来向前跳过空白，尽量插入的Section在上一个换行符号之后
      while(--pMarker >= pStreamPtr && (*pMarker == ' ' || *pMarker == '\t')) {}
      nPos = pMarker - pStreamPtr + 1;
      ASSERT(nPos < m_pBuffer->GetSize());
      Insert(nPos, str, str.GetLength());
    }
    return nPos;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Replace( clsize nPos, clsize nReplaced, T_LPCSTR szText, clsize nCount )
  {
    if(nPos == (clsize)-1) {
      nPos = m_SmartStream.GetStreamCount();
    }

    TChar* pOldPtr = (TChar*)m_pBuffer->GetPtr();
    const clsize cbSize = nCount * sizeof(TChar);
    const clsize cbReplaced = nReplaced * sizeof(TChar);
    const clsize cbPos = nPos * sizeof(TChar);

    m_pBuffer->Replace(cbPos, cbReplaced, (CLLPCVOID)szText, cbSize);

    // 重新设置SmartStream：指针变化，大小变化都要重新初始化
    m_SmartStream.Initialize((TChar*)m_pBuffer->GetPtr(), m_pBuffer->GetSize() / sizeof(TChar));

    // m_SmartStream.Initialize()之后会修正end()，RelocateSection内部断言检查需要准确的end()
    RelocateSection(pOldPtr, (TChar*)m_pBuffer->GetPtr(), nPos, nReplaced, nCount);
    return true;
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Insert( clsize nPos, T_LPCSTR szText, clsize nCount )
  {
    return Replace(nPos, 0, szText, nCount);
  }

  _SSP_TEMPL
    b32 _SSP_IMPL::Append( T_LPCSTR szText, clsize nCount )
  {
    return Replace(-1, 0, szText, nCount);
  }

  _SSP_TEMPL
  typename _SSP_IMPL::T_LPCSTR _SSP_IMPL::GetText(clsize* length) const
  {
    if(length) {
      *length = m_pBuffer->GetSize() / sizeof(TChar);
    }
    return (T_LPCSTR)m_pBuffer->GetPtr();
  }


  //////////////////////////////////////////////////////////////////////////
  _SSP_TEMPL
    _SSP_IMPL::Section::Section()
    : m_desc(NULL)
  {
  }

  _SSP_TEMPL
  _SSP_IMPL::Section::Section(SECTION* desc)
    : m_desc(desc)
  {
  }

  _SSP_TEMPL
  _SSP_IMPL::Section::~Section()
  {
    Close();
  }

  _SSP_TEMPL
  b32 _SSP_IMPL::Section::IsValid() const
  {
    return m_desc != NULL && m_desc->IsValid();
  }

  _SSP_TEMPL
    void _SSP_IMPL::Section::Close()
  {
    if(m_desc) {
      m_desc->pStock->CloseSection(m_desc);
      m_desc = NULL;
    }
  }

  _SSP_TEMPL
  typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator=(SECTION* desc)
  {
    Close();
    m_desc = desc;
    return m_desc;
  }

  _SSP_TEMPL
  typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator->() const
  {
    return m_desc;
  }

  _SSP_TEMPL
    typename _SSP_IMPL::SECTION* _SSP_IMPL::Section::operator&() const
  {
    return m_desc;
  }

  _SSP_TEMPL
  typename _SSP_IMPL::SECTION& _SSP_IMPL::Section::operator*() const
  {
    return *m_desc;
  }

  _SSP_TEMPL
  typename _SSP_IMPL::ATTRIBUTE _SSP_IMPL::Section::operator[](T_LPCSTR name) const
  {
    ATTRIBUTE attr;
    m_desc->GetKey(name, attr);
    return attr;
  }

  _SSP_TEMPL
    void _SSP_IMPL::Section::operator++()
  {
    if( ! m_desc->NextSection(NULL)) {
      Close();
    }
  }

  _SSP_TEMPL
    typename _SSP_IMPL::ATTRIBUTE _SSP_IMPL::Section::begin()
  {
    ATTRIBUTE attr;
    if(m_desc && m_desc->FirstKey(attr)) {
      return attr;
    }
    return end();
  }

  _SSP_TEMPL
    typename _SSP_IMPL::ATTRIBUTE _SSP_IMPL::Section::end()
  {
    ATTRIBUTE attr;
    attr.pSection = m_desc;
    attr.itKey    = m_desc->itEnd;
    attr.itValue  = m_desc->itEnd;
    return attr;
  }

} // namespace clstd
