#include <stdlib.h>

#include "../clstd.h"
#include "../clmemory.h"
#include "../clString.H"
#include "../clFile.H"
#include "../clBuffer.H"
#include "smartstream.h"
#include "SmartProfile.h"
#include "../clUtility.H"
//////////////////////////////////////////////////////////////////////////
//
// 显式声明模板类
//
template class SmartProfileT<const ch, clStringA, SmartProfile_TraitsA>;
template class SmartProfileT<const wch, clStringW, SmartProfile_TraitsW>;

#define _SSP_TEMPL template<typename _CTCh, class _TStr, class _Traits>
#define _SSP_IMPL SmartProfileT<_CTCh, _TStr, _Traits>

template clStringW FromProfileString (const clStringW&);
template clStringA FromProfileString (const clStringA&);
template clStringW ToProfileString   (const clStringW&);
template clStringA ToProfileString   (const clStringA&);

#ifdef _WINDOWS
#pragma warning(disable : 4267) // 类型转换警告，因为这个类不想维护了，所以暂时禁止
#endif // #ifdef _WINDOWS

//////////////////////////////////////////////////////////////////////////
b32 SmartProfile_TraitsW::_StrCmpN(const wch* pStr1, const wch* pStr2, int nCount)
{
  return wcsncmp(pStr1, pStr2, nCount) == 0;
}
clsize SmartProfile_TraitsW::_StrLen(const wch* pStr)
{
  return wcslen(pStr);
}
clsize SmartProfile_TraitsW::_TToI(const wch* pStr)
{
  return clstd::_xstrtou<clsize>(pStr);
}
double SmartProfile_TraitsW::_TToD(const wch* pStr)
{
  return clstd::_xstrtof(pStr);
}
b32 SmartProfile_TraitsW::_CheckBoolean(const wch* pStr)
{
  return clstd::strcmpiT(pStr, L"ok") == 0 ||
    clstd::strcmpiT(pStr, L"true") == 0 ||
    clstd::strcmpiT(pStr, L"yes") == 0 ||
    clstd::strcmpiT(pStr, L"1") == 0;
}

b32 SmartProfile_TraitsA::_StrCmpN(const ch* pStr1, const ch* pStr2, int nCount)
{
  return strncmp(pStr1, pStr2, nCount) == 0;
}
clsize SmartProfile_TraitsA::_StrLen(const ch* pStr)
{
  return strlen(pStr);
}
clsize SmartProfile_TraitsA::_TToI(const ch* pStr)
{
  return atoi(pStr);
}
double SmartProfile_TraitsA::_TToD(const ch* pStr)
{
  return atof(pStr);
}
b32 SmartProfile_TraitsA::_CheckBoolean(const ch* pStr)
{
  return clstd::strcmpiT(pStr, "ok") == 0 ||
    clstd::strcmpiT(pStr, "true") == 0 ||
    clstd::strcmpiT(pStr, "yes") == 0 ||
    clstd::strcmpiT(pStr, "1") == 0;
}



//////////////////////////////////////////////////////////////////////////
_SSP_TEMPL _TStr _SSP_IMPL::VALUE::KeyName()
{
  return itKeyName.ToString();
}

_SSP_TEMPL _TStr _SSP_IMPL::VALUE::SectionName()
{
  return itSection.ToString();
}

_SSP_TEMPL int _SSP_IMPL::VALUE::ToInt()
{
  return (int)_Traits::_TToI(itValue.ToString());
}

_SSP_TEMPL _TStr _SSP_IMPL::VALUE::ToString()
{
  return itValue.ToString();
}

_SSP_TEMPL float _SSP_IMPL::VALUE::ToFloat()
{
  return (float)_Traits::_TToD(itValue.ToString());
}
_SSP_TEMPL b32 _SSP_IMPL::VALUE::ToBoolean()
{
  return _Traits::_CheckBoolean(itValue.ToString());
}
//////////////////////////////////////////////////////////////////////////

_SSP_TEMPL 
  _SSP_IMPL::SmartProfileT()
  : m_pBuffer(NULL)
{
}

_SSP_TEMPL 
  _SSP_IMPL::~SmartProfileT()
{
  Close();
}

_SSP_TEMPL 
  b32 _SSP_IMPL::Create(_CTCh* szRoot)
{
  _TStr strBuffer = szRoot;
  strBuffer.Append((_CTCh)L'{');
  strBuffer.Append((_CTCh)L'\r');
  strBuffer.Append((_CTCh)L'\n');
  strBuffer.Append((_CTCh)L'}');

  m_pBuffer = new clBuffer;
  m_pBuffer->Append((CLLPVOID)&strBuffer.Front(), strBuffer.GetLength() * sizeof(_CTCh));
  m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(_CTCh));
  m_SmartStream.SetFlags(_SmartStreamT::F_SYMBOLBREAK);
  return true;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::LoadW(const wch* lpProfile)
{
  clFile file;
  if(file.OpenExistingW(lpProfile) == FALSE) {
    return FALSE;
  }

  if(file.MapToBuffer(&m_pBuffer) == FALSE) {
    return FALSE;
  }

  u32 dwBOM = *(u32*)m_pBuffer->GetPtr();
  if(sizeof(_CTCh) == 1)
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
      clBuffer* pNewBuffer = new clBuffer;
      m_pBuffer->Append(chEnd, sizeof(chEnd));
      pNewBuffer->Resize(m_pBuffer->GetSize(), FALSE);

      clsize size = wcstombs((char*)pNewBuffer->GetPtr(), (const wchar_t*)m_pBuffer->GetPtr(), pNewBuffer->GetSize());

      pNewBuffer->Resize(size, FALSE);
      delete m_pBuffer;
      m_pBuffer = pNewBuffer;
    }
  }
  else if(sizeof(_CTCh) == 2)  // UNICODE
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
      clBuffer* pNewBuffer = new clBuffer;
      m_pBuffer->Append(chEnd, sizeof(chEnd));
      pNewBuffer->Resize(m_pBuffer->GetSize() * 2, FALSE);

      clsize size = mbstowcs((wchar_t*)pNewBuffer->GetPtr(), (const char*)m_pBuffer->GetPtr(), m_pBuffer->GetSize());

      pNewBuffer->Resize(size * sizeof(_CTCh), FALSE);
      delete m_pBuffer;
      m_pBuffer = pNewBuffer;
    }
  }
  m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(_CTCh));
  return TRUE;
}

_SSP_TEMPL
  b32 _SSP_IMPL::SaveW(const wch* lpProfile)
{
  clFile file;
  if(file.CreateAlwaysW(lpProfile) == FALSE)
    return FALSE;
  if(sizeof(_CTCh) == 2)
  {
    const u16 wFlag = 0xfeff;
    file.Write(&wFlag, 2);
  }
  file.Write(m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize());
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
  b32 _SSP_IMPL::SaveA(const ch* lpProfile)
{
  clStringW strProfileW = lpProfile;
  return SaveW(strProfileW);
}

_SSP_TEMPL 
  typename _SSP_IMPL::HANDLE _SSP_IMPL::FindFirstSection(HANDLE hSiblingSect, b32 bSiblingSect, _CTCh* szOffset, _CTCh* szFindName)
{
  HANDLE hFind = new FINDSECT;
  hFind->strPath.Clear();

  if(hSiblingSect == NULL)
  {
    hFind->itBegin = m_SmartStream.begin();
    hFind->itEnd   = m_SmartStream.end();

    //m_SmartStream.find_pair(m_SmartStream.begin(), hFind->itBegin, hFind->itEnd, (_CTCh*)L"{", (_CTCh*)L"}");
    //hFind->itBegin = m_SmartStream.begin();
  }
  else
  {
    if(bSiblingSect) {
      hFind->itBegin = hSiblingSect->itBegin;
      hFind->itEnd   = hSiblingSect->itEnd;
    }
    else {
      hFind->itCur = m_SmartStream.find(hSiblingSect->itCur, 1, (_CTCh*)L"{");
      SmartStreamUtility::FindPair(hFind->itCur, hFind->itBegin, hFind->itEnd, (_CTCh*)L"{", (_CTCh*)L"}");
    }
  }
  hFind->itCur = hFind->itBegin;

  if(szOffset != NULL && szOffset[0] != 0)
  {
    _TStr strOffset = szOffset;
    _TStr strName;
    HANDLE hLocalFind = hSiblingSect;

    while(1)
    {
      size_t nPos = strOffset.Find((_CTCh)'\\');
      if(nPos == _TStr::npos)
      {
        strName = strOffset;
        strOffset.Clear();
      }
      else
      {
        strName = strOffset.SubString(0, nPos);
        strOffset.Remove(0, nPos + 1);
      }

      if(strName.GetLength() > 1)
      {
        HANDLE hFindResult = FindFirstSection(hLocalFind, TRUE, NULL, strName);

        //  删除循环中的上一个Handle, 但不能删除参数中传过来Handle
        if(hLocalFind != hSiblingSect)
          FindClose(hLocalFind);

        // 不存在则返回
        if(hFindResult == NULL)
        {
          SAFE_DELETE(hFind);
          return NULL;
        }
        hLocalFind = hFindResult;

        hLocalFind->itCur = m_SmartStream.find(hLocalFind->itCur, 1, (_CTCh*)L"{");
        SmartStreamUtility::FindPair(hLocalFind->itCur, hLocalFind->itBegin, hLocalFind->itEnd, (_CTCh*)L"{", (_CTCh*)L"}");
        ++hLocalFind->itCur;
      }
      if(strOffset.GetLength() == 0)
        break;
    }
    *hFind = *hLocalFind;
    FindClose(hLocalFind);

    // 不在递归中
    // 合成查找路径
    if(hSiblingSect != NULL)
      hFind->strPath = hSiblingSect->strPath;

    if(szOffset != NULL)
    {
      if(hFind->strPath.GetLength() > 0)
        hFind->strPath = hFind->strPath + (_CTCh*)L"\\" + szOffset;
      else
        hFind->strPath = szOffset;
    }
  }


  hFind->bSection    = true;
  hFind->strFindName  = szFindName == NULL ? (_CTCh*)L"" : szFindName;

  //if(szFindName != NULL)
  //{
  //  m_SmartStream.find_pair(hFind->itCur, hFind->itBegin, hFind->itEnd, (_CTCh*)L"{", (_CTCh*)L"}");
  //}

  if(FindSection(hFind) == TRUE)
  {
    hFind->itSection = hFind->itCur;
    hFind->bWrite = 0;
    AddHandle(hFind);
    return hFind;
  }

  // 空的 Section 或者枚举的section没有任何section
  // 这个为了能够枚举里面的Key
  if(hFind->strFindName.GetLength() == 0)
  {
    hFind->itSection = hFind->itCur;
    hFind->bWrite = 0;
    AddHandle(hFind);
    return hFind;
  }

  SAFE_DELETE(hFind);
  return NULL;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::FindNextSection(HANDLE hFind)
{
  ASSERT(hFind->bSection == true);

  ++hFind->itCur;
  if(EnumSectKey(hFind->itCur) == false)
    return false;
  b32 bRet = FindSection(hFind);
  hFind->itSection = hFind->itCur;
  return bRet;
}

_SSP_TEMPL 
  _TStr _SSP_IMPL::GetSectionName(HANDLE hSect)
{
  ASSERT(hSect->bSection == true);
  if(hSect->itSection == hSect->itEnd)
    return (_CTCh*)L"";
  return hSect->itSection.ToString();
}

_SSP_TEMPL 
  _TStr _SSP_IMPL::GetPathName(HANDLE hSect)
{
  ASSERT(hSect->bSection == true);
  return hSect->strPath;
}

_SSP_TEMPL 
typename _SSP_IMPL::HANDLE _SSP_IMPL::OpenSection(_CTCh* szDir)
{
  _TStr strDir = szDir;
  size_t nPos = strDir.ReverseFind((_CTCh)'\\');

  // 如果是单一的 Section 名字,则直接打开
  if(nPos == _TStr::npos) {
    HANDLE hval = FindFirstSection(NULL, NULL, NULL, szDir);
    //++hval->itBegin;
    return hval;
  }

  // 对于目录式的名字, 把最后一个 Section 拆出来形成 "目录名字偏移+目录名字" 形式
  // "a\b\c\d" ==> "a\b\c", "d"
  _TStr strSect = strDir.SubString(nPos + 1, _TStr::npos);
  strDir = strDir.SubString(0, nPos);
  return FindFirstSection(NULL, NULL, strDir, strSect);
}

_SSP_TEMPL 
  b32 _SSP_IMPL::FindSection(HANDLE hFind)
{
  ASSERT(hFind->bSection == true);
  _MyIterator it = hFind->itCur;

  while(it < hFind->itEnd)
  {
    if((hFind->strFindName.GetLength() == 0 || it == hFind->strFindName)
      && (it + 1) == (_CTCh*)L"{")
    {
      hFind->itCur = it;
      return true;
    }
    ++it;
    if(EnumSectKey(it) == false)
      return false;
  }
  return false;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::EnumSectKey(_MyIterator& it) const
{
  if(it == (_CTCh*)L"{")
  {
    _MyIterator itBegin, itEnd;
    if(SmartStreamUtility::FindPair(it, itBegin, itEnd, (_CTCh*)L"{", (_CTCh*)L"}") == false)
      return false;
    it = itEnd + 1;
  }
  else if(it == (_CTCh*)L"=")
  {
    _MyIterator itEndExp = m_SmartStream.find(it, 2, (_CTCh*)L";", (_CTCh*)L"}");
    if(itEndExp == (_CTCh*)L"}")
      return false;
    it = itEndExp + 1;
  }
  return true;
}

_SSP_TEMPL 
  typename _SSP_IMPL::HANDLE _SSP_IMPL::FindFirstKey(HANDLE hSection, VALUE& value)
{
  if(hSection->bSection != true || hSection->bWrite)
  {
    ASSERT(0);
    return NULL;
  }
  HANDLE hFindKey = new FINDSECT;
  hFindKey->bSection = false;
  hFindKey->strFindName = (_CTCh*)L"";
#ifdef _DEBUG
  ASSERT(hSection->strFindName == (_CTCh*)L"" || hSection->strFindName == GetSectionName(hSection));
#endif
  if(hSection->strPath.GetLength() > 0)
    hFindKey->strPath = hSection->strPath + (_CTCh*)L"\\" + GetSectionName(hSection);
  else
    hFindKey->strPath = GetSectionName(hSection);

  if( SmartStreamUtility::FindPair(hSection->itCur, hFindKey->itBegin, hFindKey->itEnd, (_CTCh*)L"{", (_CTCh*)L"}") == false)
  {
    SAFE_DELETE(hFindKey);
    return NULL;
  }
  value.itSection = hSection->itCur;
  hFindKey->itCur = hSection->itCur + 1;
  if(FindNextKey(hFindKey, value) == false)
  {
    SAFE_DELETE(hFindKey);
    return NULL;
  }
  AddHandle(hFindKey);
  return hFindKey;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::FindNextKey(HANDLE hKey, VALUE& value)
{
  _MyIterator it = hKey->itCur;
  while(it < hKey->itEnd)
  {
    if((it + 1) == (_CTCh*)L"=")
    {
      value.itKeyName = it;
      value.itValue = it + 2;

      hKey->itCur = m_SmartStream.find(value.itValue, 1, (_CTCh*)L";") + 1;
      return true;
    }
    ++it;
    if(EnumSectKey(it) == false)
      return false;
  }
  return false;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::FindKey(HANDLE hSection, _CTCh* szKey, VALUE& value) const
{
  if(hSection->bSection != true || hSection->bWrite)
  {
    ASSERT(0);
    return false;
  }
  _MyIterator it = hSection->itCur + 2;
  while(it < hSection->itEnd)
  {
    if(it == szKey && (it + 1) == (_CTCh*)L"=")
    {
      value.itSection = hSection->itCur;
      value.itKeyName = it;
      value.itValue = it + 2;
      return true;
    }
    ++it;
    if(EnumSectKey(it) == false)
      break;
    else if(it == (_CTCh*)L"}")
      break;
  }
  return false;
}
_SSP_TEMPL 
  int _SSP_IMPL::FindKeyAsString(HANDLE hSection, _CTCh* szKey, _CTCh* szDefault, _TCh* szBuffer, int nCount) const
{
  VALUE value;
  _CTCh* pSource = NULL;
  if(FindKey(hSection, szKey, value) == TRUE)
  {
    pSource = value.itValue.marker;
    nCount = clMin(nCount, (int)value.itValue.length);
  }
  else
  {
    pSource = szDefault;
    if(szDefault != NULL)
      nCount = clMin(nCount, (int)_Traits::_StrLen(szDefault));
  }
  int n = 0;
  if(nCount != 0 && pSource != NULL)
  {
    for(int i = 0;i < nCount; i++)
      if(pSource[i] != '\"' && pSource[i] != '\'')
        szBuffer[n++] = pSource[i];
    szBuffer[n] = 0;
  }
  return n;
}
_SSP_TEMPL 
  _TStr _SSP_IMPL::FindKeyAsString(HANDLE hSection, _CTCh* szKey, _TStr strDefault) const
{
  VALUE value;
  if(FindKey(hSection, szKey, value) == false)
    return strDefault;
  return value.ToString();
}
_SSP_TEMPL 
  int _SSP_IMPL::FindKeyAsInteger(HANDLE hSection, _CTCh* szKey, int nDefault) const
{
  VALUE value;
  if(FindKey(hSection, szKey, value) == false)
    return nDefault;
  return value.ToInt();
}
_SSP_TEMPL 
  float _SSP_IMPL::FindKeyAsFloat(HANDLE hSection, _CTCh* szKey, float fDefault)
{
  VALUE value;
  if(FindKey(hSection, szKey, value) == false)
    return fDefault;
  return value.ToFloat();
}
_SSP_TEMPL 
  b32 _SSP_IMPL::FindKeyAsBoolean(HANDLE hSection, _CTCh* szKey, b32 bDefault)
{
  VALUE value;
  if(FindKey(hSection, szKey, value) == false)
    return bDefault;
  return value.ToBoolean();
}
_SSP_TEMPL 
  int _SSP_IMPL::FindKeyAsIntegerArray(HANDLE hSection, _CTCh* szKey, clBuffer** ppBuffer)
{
  VALUE value;
  if(FindKey(hSection, szKey, value) == false)
  {
    *ppBuffer = NULL;
    return 0;
  }
  clBuffer* pBuffer = new clBuffer;
  _TStr str = value.ToString();
  size_t pos = 0, begin = 0, length = str.GetLength();
  while(pos != length)
  {
    pos = str.Find(',', begin);
    if(pos == _TStr::npos)
      pos = str.GetLength();

    _TStr sub = str.SubString(begin, pos - begin);
    clsize nValue = _Traits::_TToI(sub);
    begin = pos + 1;
    pBuffer->Append(&nValue, sizeof(int));
  }
  *ppBuffer = pBuffer;
  return (int)pBuffer->GetSize() / sizeof(int);
}
_SSP_TEMPL 
  b32 _SSP_IMPL::FindClose(HANDLE hFind)
{
  return CloseHandle(hFind);
}

_SSP_TEMPL 
b32 _SSP_IMPL::CloseHandle(HANDLE hHandle)
{
  DelHandle(hHandle);
  SAFE_DELETE(hHandle);
  return true;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::AddHandle(HANDLE hHandle)
{
  m_aHandles.push_back(hHandle);
  return true;
}

_SSP_TEMPL 
  b32 _SSP_IMPL::DelHandle(HANDLE hHandle)
{
  HandleArray& aHandles = m_aHandles;
  for(auto it = aHandles.begin();
    it != aHandles.end(); ++it)
  {
    if(*it == hHandle)
    {
      aHandles.erase(it);
      return true;
    }
  }
  return false;
}
//////////////////////////////////////////////////////////////////////////
//_SSP_TEMPL 
//int _SSP_IMPL::CommitChange()
//{
//  int nRet = (int)m_aHandles.size();
//
//  if(m_aHandles.size() != 0)
//    return -nRet;
//
//  _TStr strTable;
//  _TStr strBuffer;
//
//  for(ModifyArray::iterator it = m_aModify.begin(); it != m_aModify.end(); ++it)
//  {
//    MODIFY& mod = *it;
//    if(mod.eType == MT_ADDSECT)
//    {
//      HANDLE hHandle = FindFirstSection(NULL, mod.strDestPath.c_str(), NULL);
//      if(hHandle == NULL)
//        continue;
//
//      u32 uOffset = hHandle->itEnd.offset();
//      while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
//
//      PathDepthToTable(mod.strDestPath, &strTable);
//      strBuffer += strTable;
//      strBuffer += mod.strSectKey;
//      strBuffer.push_back((_CTCh)L'{');
//      strBuffer.push_back((_CTCh)L'\r');
//      strBuffer.push_back((_CTCh)L'\n');
//      strBuffer += strTable;
//      strBuffer.push_back((_CTCh)L'}');
//      strBuffer.push_back((_CTCh)L'\r');
//      strBuffer.push_back((_CTCh)L'\n');
//
//      m_pBuffer->Add(uOffset, (CLLPVOID)strBuffer.c_str(), strBuffer.length());
//      m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), m_pBuffer->GetSize()/sizeof(_CTCh));
//      nRet--;
//      FindClose(hHandle);
//      strBuffer.clear();
//      strTable.clear();
//    }
//    else if(mod.eType == MT_DELSECT)
//    {
//      HANDLE hHandle = FindFirstSection(NULL, mod.strDestPath.c_str(), mod.strSectKey.c_str());
//      if(hHandle == NULL)
//        continue;
//
//      //
//      u32 uOffsetBegin = hHandle->itCur.offset();
//      while(uOffsetBegin > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffsetBegin - 1]) == (_CTCh)L'\t') uOffsetBegin--;
//    
//      //
//      _MyIterator itBegin, itEnd;
//      m_SmartStream.find_pair(hHandle->itCur, itBegin, itEnd, (_CTCh*)L"{", (_CTCh*)L"}");
//      _MyIterator it = itEnd;
//      while(it != m_SmartStream.end())
//      {
//        if(it == (_CTCh*)L"}" || it == (_CTCh*)L";")
//          ++it;
//        else
//          break;
//      }
//      u32 uOffsetEnd = it.offset();
//      while(uOffsetEnd > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffsetEnd - 1]) == (_CTCh)L'\t') uOffsetEnd--;
//      m_pBuffer->Replace(uOffsetBegin, uOffsetEnd - uOffsetBegin, NULL, NULL);
//      m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), m_pBuffer->GetSize()/sizeof(_CTCh));
//      nRet--;
//      FindClose(hHandle);
//    }
//    else if(mod.eType == MT_SETKEY || mod.eType == MT_DELKEY)
//    {
//      HANDLE hHandle = FindFirstSection(NULL, mod.strDestPath.c_str(), NULL);
//      if(hHandle == NULL)
//        continue;
//      _MyIterator it = hHandle->itCur;
//      _MyIterator itKey;  // MT_DELKEY
//      while(it != hHandle->itEnd)
//      {
//        if(it == mod.strSectKey && (it + 1) == (_CTCh*)L"=")
//        {
//          itKey = it;
//          it = it + 2;
//          break;
//        }
//        ++it;
//        if(EnumSectKey(it) == false)
//          goto FAILED_KEY;
//      }
//      
//      u32 uOffset = it.offset();
//      
//      //
//      if(it == hHandle->itEnd)
//      {
//        if(mod.eType == MT_DELKEY)
//          goto FAILED_KEY;
//
//        while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
//
//        PathDepthToTable(mod.strDestPath, &strTable);
//        strBuffer += strTable;
//        strBuffer += mod.strSectKey;
//        strBuffer.push_back((_CTCh)L'=');
//        strBuffer += mod.strValue;
//        strBuffer.push_back((_CTCh)L';');
//        strBuffer.push_back((_CTCh)L'\r');
//        strBuffer.push_back((_CTCh)L'\n');
//
//        m_pBuffer->Replace(uOffset, 0, (CLLPVOID)strBuffer.c_str(), strBuffer.length());
//      }
//      else  //
//      {
//        if(mod.eType == MT_DELKEY)
//        {
//          uOffset = itKey.offset();
//          while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
//          
//          ++it;
//          while(it != m_SmartStream.end())
//          {
//            if(it == (_CTCh*)L";")
//              ++it;
//            else break;
//          }
//          PathDepthToTable(mod.strDestPath, &strTable);
//          u32 uOffEnd = it.offset();
//          m_pBuffer->Replace(uOffset, uOffEnd - uOffset, (CLLPVOID)strTable.c_str(), strTable.length());
//        }
//        else
//          m_pBuffer->Replace(uOffset, it.length, (CLLPVOID)mod.strValue.c_str(), mod.strValue.length());
//      }
//
//      m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), m_pBuffer->GetSize()/sizeof(_CTCh));
//      nRet--;
//FAILED_KEY:
//      FindClose(hHandle);
//
//      strBuffer.clear();
//      strTable.clear();
//    }
//  }
//   return nRet;
//}
//
//_SSP_TEMPL 
//b32 _SSP_IMPL::DiscardChange()
//{
//  m_aModify.clear();
//  return true;
//}

//
//_SSP_TEMPL 
//b32 _SSP_IMPL::CreateSection(_CTCh* szPath, _CTCh* szSect)
//{
//  MODIFY m;
//  m.eType = MT_ADDSECT;
//  m.strDestPath = szPath;
//  m.strSectKey = szSect;
//  m.strValue.clear();
//
//  m_aModify.push_back(m);
//  return true;
//}
//
//_SSP_TEMPL 
//b32 _SSP_IMPL::DeleteSection(_CTCh* szPath, _CTCh* szSect)
//{
//  MODIFY m;
//  m.eType = MT_DELSECT;
//  m.strDestPath = szPath;
//  m.strSectKey = szSect;
//  m.strValue.clear();
//
//  m_aModify.push_back(m);
//  return false;
//}
//
//_SSP_TEMPL 
//b32 _SSP_IMPL::SetKey(_CTCh* szPath, _CTCh* szKeyName, _CTCh* szValue)
//{
//  MODIFY m;
//  m.eType = MT_SETKEY;
//  m.strDestPath  = szPath;
//  m.strSectKey  = szKeyName;
//  m.strValue    = szValue;
//
//  m_aModify.push_back(m);
//  return true;
//}
//
//_SSP_TEMPL 
//b32 _SSP_IMPL::DeleteKey(_CTCh* szPath, _CTCh* szKeyName)
//{
//  MODIFY m;
//  m.eType = MT_DELKEY;
//  m.strDestPath  = szPath;
//  m.strSectKey  = szKeyName;
//  m.strValue.clear();
//
//  m_aModify.push_back(m);
//  return true;
//}
_SSP_TEMPL 
  void _SSP_IMPL::TrimFrontTab(clsize& uOffset)
{
  while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
}

_SSP_TEMPL 
  typename _SSP_IMPL::HANDLE _SSP_IMPL::CreateSection(_CTCh* szPath, _CTCh* szSect)
{
  _TStr strTable;
  _TStr strBuffer;
  HANDLE hHandle = FindFirstSection(NULL, NULL, szPath, NULL);
  if(hHandle == NULL)
    return NULL;

  clsize uOffset = hHandle->itEnd.offset();
  //while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
  TrimFrontTab(uOffset);

  PathDepthToTable(szPath, &strTable);
  strBuffer += strTable;
  strBuffer += szSect;
  strBuffer.Append((_CTCh)L'{');
  strBuffer.Append((_CTCh)L'\r');
  strBuffer.Append((_CTCh)L'\n');
  strBuffer += strTable;
  strBuffer.Append((_CTCh)L'}');
  strBuffer.Append((_CTCh)L'\r');
  strBuffer.Append((_CTCh)L'\n');

  _CTCh* pOldPtr = (_CTCh*)m_pBuffer->GetPtr();

  m_pBuffer->Replace(uOffset * sizeof(_CTCh), 0, (CLLPVOID)&strBuffer.Front(), strBuffer.GetLength() * sizeof(_CTCh));
  m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize()/sizeof(_CTCh));
  UpdateHandle(uOffset, pOldPtr, 0, (_CTCh*)m_pBuffer->GetPtr(), (u32)strBuffer.GetLength());

  // 直接使用了原来的 Handle
  FINDSECT* pFindSect = (FINDSECT*)hHandle;
  pFindSect->bSection = true;
  pFindSect->strFindName.Clear();
  if(pFindSect->strPath.GetLength() > 0)
    pFindSect->strPath = pFindSect->strPath + (_CTCh*)L"\\" + szSect;
  else
    pFindSect->strPath = szSect;
  pFindSect->itSection.marker = (_CTCh*)m_pBuffer->GetPtr() + uOffset;
  pFindSect->itSection.length = (u32)_TStr(szSect).GetLength();
  pFindSect->itCur = pFindSect->itSection;
  pFindSect->bWrite = 1;
  SmartStreamUtility::FindPair(pFindSect->itCur, pFindSect->itBegin, pFindSect->itEnd,(_CTCh*)L"{", (_CTCh*)L"}");

  //nRet--;
  //FindClose(hHandle);
  //strBuffer.clear();
  //strTable.clear();
  //AddHandle(hHandle);
  return hHandle;
}

//_SSP_TEMPL 
//b32 _SSP_IMPL::DeleteSection(_CTCh* szPath, _CTCh* szSect)
//{
//  MODIFY m;
//  m.eType = MT_DELSECT;
//  m.strDestPath = szPath;
//  m.strSectKey = szSect;
//  m.strValue.clear();
//
//  m_aModify.push_back(m);
//  return false;
//}
//

_SSP_TEMPL 
  b32 _SSP_IMPL::SetKey(HANDLE hSect, _CTCh* szKeyName, _CTCh* szValue, b32 bReplace)
{
  _TStr strTable;
  _TStr strBuffer;
  clsize uOffset = 0;
  _CTCh* pOldPtr = NULL;

  //HANDLE hHandle = FindFirstSection(NULL, mod.strDestPath.c_str(), NULL);
  if(hSect == NULL)
    return false;
  if(hSect->bWrite == 0) {
    ASSERT(0);
    return false;
  }
  _MyIterator it = hSect->itBegin;
  _MyIterator itKey;  // MT_DELKEY 用的
  while(it < hSect->itEnd)
  {
    if(it == szKeyName && (it + 1) == (_CTCh*)L"=")
    {
      itKey = it;
      it = it + 2;
      break;
    }
    ++it;
    if(EnumSectKey(it) == false)
      goto FAILED_KEY;
  }
  if(bReplace == FALSE && it != hSect->itEnd)
  {
    while(it < hSect->itEnd)
      ++it;
  }
  uOffset = it.offset();
  pOldPtr = (_CTCh*)m_pBuffer->GetPtr();

  // 没有对应键值
  if(it == hSect->itEnd)
  {
    //while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;
    TrimFrontTab(uOffset);

    PathDepthToTable(GetPathName(hSect), &strTable);
    strBuffer += strTable;
    strBuffer += szKeyName;
    strBuffer.Append((_CTCh)L'=');
    strBuffer += szValue;
    strBuffer.Append((_CTCh)L';');
    strBuffer.Append((_CTCh)L'\r');
    strBuffer.Append((_CTCh)L'\n');

    m_pBuffer->Replace(uOffset * sizeof(_CTCh), 0, (CLLPVOID)&strBuffer.Front(), strBuffer.GetLength() * sizeof(_CTCh));
    m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize() / sizeof(_CTCh));
    UpdateHandle(uOffset, pOldPtr, 0, (_CTCh*)m_pBuffer->GetPtr(), (u32)strBuffer.GetLength());
  }
  else  // 修改存在的键值
  {
    /*if(mod.eType == MT_DELKEY)
    {
    uOffset = itKey.offset();
    while(uOffset > 0 && (((_CTCh*)m_pBuffer->GetPtr())[uOffset - 1]) == (_CTCh)L'\t') uOffset--;

    ++it;
    while(it != m_SmartStream.end())
    {
    if(it == (_CTCh*)L";")
    ++it;
    else break;
    }
    PathDepthToTable(mod.strDestPath, &strTable);
    u32 uOffEnd = it.offset();
    m_pBuffer->Replace(uOffset, uOffEnd - uOffset, (CLLPVOID)strTable.c_str(), strTable.length());
    }
    else*/
    _TStr strValue = szValue;
    m_pBuffer->Replace(uOffset * sizeof(_CTCh), it.length * sizeof(_CTCh), (CLLPVOID)&strValue.Front(), strValue.GetLength() * sizeof(_CTCh));

    m_SmartStream.Initialize((_CTCh*)m_pBuffer->GetPtr(), (u32)m_pBuffer->GetSize() / sizeof(_CTCh));
    UpdateHandle(uOffset, pOldPtr, it.length, (_CTCh*)m_pBuffer->GetPtr(), (u32)strValue.GetLength());
  }
  return true;

  //nRet--;

FAILED_KEY:
  return false;
  //FindClose(hHandle);
  //
  //strBuffer.clear();
  //strTable.clear();
}


//_SSP_TEMPL 
//b32 _SSP_IMPL::DeleteKey(_CTCh* szPath, _CTCh* szKeyName)
//{
//  MODIFY m;
//  m.eType = MT_DELKEY;
//  m.strDestPath  = szPath;
//  m.strSectKey  = szKeyName;
//  m.strValue.clear();
//
//  m_aModify.push_back(m);
//  return true;
//}


_SSP_TEMPL 
  int _SSP_IMPL::PathDepthToTable(const _TStr& strPath, _TStr* strTable)
{
  int nDepth = 1;
  size_t nPos = 0;

  while((nPos = strPath.Find((_CTCh)L'\\', nPos)) != _TStr::npos)
  {
    nDepth++;
    nPos++;
  }

  if(strTable != NULL)
  {
    for(int i = 0; i < nDepth; i++)
    {
      strTable->Append((_CTCh)L'\t');
    }
  }
  return nDepth;
}

_SSP_TEMPL
  b32 _SSP_IMPL::UpdateIterator(_MyIterator& it, u32 uPos, _CTCh* lpOldPtr, u32 sizeOld, _CTCh* lpNewPtr, u32 sizeNew)
{
  clsize uMyPos = it.marker - lpOldPtr;
  if(uMyPos < uPos)
  {
    it.marker = lpNewPtr + uMyPos;
    // 改变的位置不应该在 iterator 中
    ASSERT(uMyPos + it.length < uPos);
    return true;
  }
  if(uPos + sizeOld > uMyPos)
  {
    it.pContainer = NULL;
    it.marker = NULL;
    it.length = NULL;
    return false;
  }
  it.marker = lpNewPtr + uMyPos - sizeOld + sizeNew;
  return true;
}

// uPos 是字符位置,不是字节位置
_SSP_TEMPL
  b32 _SSP_IMPL::UpdateHandle(u32 uPos, _CTCh* lpOldPtr, u32 sizeOld, _CTCh* lpNewPtr, u32 sizeNew)
{
  for(auto it = m_aHandles.begin(); it != m_aHandles.end(); ++it)
  {
    FINDSECT* pHandle = (FINDSECT*)*it;
    UpdateIterator(pHandle->itBegin,   uPos, lpOldPtr, sizeOld, lpNewPtr, sizeNew);
    UpdateIterator(pHandle->itEnd,     uPos, lpOldPtr, sizeOld, lpNewPtr, sizeNew);
    UpdateIterator(pHandle->itCur,     uPos, lpOldPtr, sizeOld, lpNewPtr, sizeNew);
    UpdateIterator(pHandle->itSection, uPos, lpOldPtr, sizeOld, lpNewPtr, sizeNew);
  }
  return true;
}

_SSP_TEMPL
void _SSP_IMPL::ReverseByteOrder16(u16* ptr, clsize nCount)
{
  for(clsize i = 0; i < nCount; i++) {
    *ptr = (*ptr >> 8) | (*ptr << 8); // 不确定是循环位移还是位移,但是都不需要&操作.
    ptr++;
  }
}

template<class _STR>
_STR FromProfileString(const _STR& str)
{
  size_t size = str.GetLength();
  if(size < 1)
    return str;
  size_t top = 0;
  if(str.Front() == _T('\"')) {
    size--;
    top++;
  }
  if(str.Back() == _T('\"')) {
    size--;
  }

  //if(str[size - 1] == _T('\"'))
  //  size--;
  //if(str[0] == _T('\"'))
  //{
  //  size--;
  //  top++;
  //}
  _STR strBackslash;
  strBackslash.Append('\\');
  strBackslash.Append('\"');

  _STR strTemp = str.SubString(top, size);
  clsize nStart = 0;
  while((nStart = strTemp.Find(strBackslash, nStart)) != _STR::npos)
  {
    strTemp.Remove(nStart, 1);
  }
  //for(typename _STR::iterator it = strTemp.begin();
  //  it != strTemp.end(); ++it)
  //{
  //  if(*it == _T('\\'))
  //  {
  //    if(it + 1 != strTemp.end() && *(it + 1) == _T('\\'))
  //      it = strTemp.erase(it);
  //  }
  //}
  return strTemp;
}

template<class _STR>
_STR ToProfileString(const _STR& str)
{
  _STR strTemp = "\"";
  _STR strQuot;
  _STR strBackslash;
  size_t size = str.GetLength();
  if(size < 1)
    return str;

  //strQuot = '\"';
  //strBackslash = '\\';

  strTemp += str;
  clsize nStart = 1;

  while((nStart = strTemp.Find('\"', nStart)) != _STR::npos)
  {
    strTemp.Insert(nStart, '\\');
    nStart += 2;
  }

  //nStart = 1;
  //while((nStart = strTemp.Find('\\', nStart)) != _STR::npos)
  //{
  //  strTemp.Insert(nStart, '\\');
  //  nStart += 2;
  //}

  strTemp += '\"';

  //strTemp = '\"';
  //strTemp += str;
  //strTemp += '\"';
  //if(str[size - 1] != L'\"')
  //  strTemp += L'\"';

  //if(str[0] != L'\"')
  //  strTemp.insert(0, strQuot);

  //for(typename _STR::iterator it = strTemp.begin();
  //  it != strTemp.end(); ++it)
  //{
  //  if(*it == L'\\')
  //  {
  //    it = strTemp.insert(it, strBackslash[0]);
  //    ++it;
  //    if(it == strTemp.end())
  //      break;
  //  }
  //}
  return strTemp;
}
