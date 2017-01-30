#include "clstd.h"
#include "clRepository.h"
#include "clString.h"
#include "clUtility.h"

#define KEYINCCOUNT 8
#define NAMEINCCOUNT 128

namespace clstd
{
  template<typename T_LPCSTR>
  b32 Repository::LoadFromFileT(T_LPCSTR szFilename)
  {
    File file;
    if( ! file.OpenExisting(szFilename)) {
      // ERROR:无法打开文件
      return false;
    }

    if( ! file.ReadToBuffer(&m_Buffer)) {
      // ERROR:无法读取文件
      return false;
    }

    return ParseFromBuffer();
  }

  Repository::Repository()
    : m_pKeys    (NULL)
    , m_pKeysEnd (NULL)
    , m_pKeysCapacity(NULL)
    , m_pNamesEnd  (NULL)
    , m_pData     (NULL)
    , m_cbDataLen (0)
  {
  }

  Repository::~Repository()
  {
  }

  Repository::iterator Repository::begin() const
  {
    iterator it(this, m_pKeys);
    return it;
  }

  Repository::iterator Repository::end() const
  {
    iterator it(this, m_pKeysEnd);
    return it;
  }

  b32 Repository::ParseFromBuffer()
  {
    size_t pData = reinterpret_cast<size_t>(m_Buffer.GetPtr());
    const FILE_HEADER* pHeader = reinterpret_cast<const FILE_HEADER*>(pData);
    if(pHeader->dwMagic != CLMAKEFOURCC('C','L','R','P')) {
      // ERROR: bad file magic
      return false;
    }

    m_pKeys = reinterpret_cast<KEY*>(pData + sizeof(FILE_HEADER));
    m_pKeysEnd = m_pKeys + pHeader->nKeys;
    m_pKeysCapacity = m_pKeysEnd;

    m_pNamesEnd = reinterpret_cast<LPCSTR>(reinterpret_cast<size_t>(m_pKeysEnd) + pHeader->cbNames);
    m_pData     = (CLBYTE*)m_pNamesEnd;
    m_cbDataLen = pHeader->cbData;

    size_t nCheckLen = sizeof(FILE_HEADER) + sizeof(KEY) * pHeader->nKeys + pHeader->cbNames + pHeader->cbData;
    if(nCheckLen > m_Buffer.GetSize()) {
      // ERROR: not enough data
      return false;
    }
    else if(nCheckLen < m_Buffer.GetSize())
    {
      // WARNING: has redundancy data
      return true;
    }
    return true;
  }

  Repository::KEY* Repository::_FindKey(LPCSTR szKey) const
  {
    LPCSTR szNamesFront = GetNamesBegin();
    KEY*const result =
      clstd::BinarySearch(m_pKeys, m_pKeysEnd, szKey,
      [szNamesFront](KEY* pMid, LPCSTR key) -> int
    {
      return strcmpT(pMid->GetName(szNamesFront), key);
    });
    return result;
  }

  Repository::KEYPAIR* Repository::_PreInsertKey(KEYPAIR* pPair, LPCSTR szKey)
  {
    LPCSTR szNamesFront = GetNamesBegin();
    pPair->pTable =
      clstd::BinaryInsertPos(m_pKeys, m_pKeysEnd, szKey, &pPair->bInsert,
      [szNamesFront](KEY* pMid, LPCSTR key) -> int
    {
        return strcmpT(pMid->GetName(szNamesFront), key);
    });
    return pPair;
  }

  void Repository::_Locate(size_t memdelta)
  {
    m_pKeys         = (KEY*)((size_t)m_pKeys + memdelta);
    m_pKeysEnd      = (KEY*)((size_t)m_pKeysEnd + memdelta);
    m_pKeysCapacity = (KEY*)((size_t)m_pKeysCapacity + memdelta);
    m_pNamesEnd     = (LPCSTR)((size_t)m_pNamesEnd + memdelta);
    m_pData         = (CLBYTE*)((size_t)m_pData + memdelta);
  }

  b32 Repository::_AllocKey(size_t cbKeyNameLen, const void* pData, size_t nLength, KEY*& rPos)
  {
    size_t extened = 0;
    size_t key_inc = 0;
    size_t name_inc = 0;
    ASSERT(m_pKeysEnd <= m_pKeysCapacity);
    ASSERT(m_pNamesEnd <= GetNamesCapacity());

    if(m_pKeysEnd == m_pKeysCapacity) {
      key_inc = KEYINCCOUNT * sizeof(KEY);
      extened += key_inc;
    }

    if((size_t)m_pNamesEnd + cbKeyNameLen > (size_t)GetNamesCapacity()) {
      name_inc = (cbKeyNameLen + NAMEINCCOUNT * sizeof(TChar));
      extened += name_inc;
    }

    if(m_cbDataLen + nLength > GetDataCapacity()) {
      extened += nLength;
    }

    if(extened > 0) {
      size_t mem_delta = m_Buffer.Resize(m_Buffer.GetSize() + extened, FALSE);
      if(mem_delta) {
        _Locate(mem_delta);
        rPos = (KEY*)((size_t)rPos + mem_delta);
      }

      if(name_inc > 0) {
        // 注意嵌套调整
        memcpy(m_pData + key_inc + name_inc, m_pData, m_cbDataLen);

        if(key_inc > 0) {
          const size_t cbNamesLen = (GetNamesCapacity() - GetNamesBegin()) * sizeof(TChar);
          memcpy((CLBYTE*)GetNamesBegin() + key_inc, GetNamesBegin(), cbNamesLen);
          m_pKeysCapacity = (KEY*)((CLBYTE*)m_pKeysCapacity + key_inc);
        }

        m_pData += (key_inc + name_inc);
      }
      else if(key_inc > 0) {
        const size_t cbNamesLen = (GetNamesCapacity() - GetNamesBegin()) * sizeof(TChar);
        memcpy((CLBYTE*)GetNamesBegin() + key_inc, GetNamesBegin(), cbNamesLen + m_cbDataLen);
        m_pKeysCapacity = (KEY*)((CLBYTE*)m_pKeysCapacity + key_inc);
      }
      return TRUE;
    }
    //m_cbDataLen += nLength;

    return FALSE;
  }

  b32 Repository::_InsertKey( KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength )
  {
    ASSERT(pPos >= m_pKeys && pPos <= m_pKeysEnd);
    const size_t cbKeyNameLen = ALIGN_2((strlenT(szKey) + 1) * sizeof(TChar));// 算上结尾'\0'，按照两字节对齐
    _AllocKey(cbKeyNameLen, pData, nLength, pPos);

    ASSERT(m_pKeysEnd < m_pKeysCapacity);
    
    const size_t cbKeyLen = (m_pKeysEnd - pPos) * sizeof(KEY);
    if(cbKeyLen) {
      memmove(pPos + 1, pPos, cbKeyLen);
    }
    m_pKeysEnd++;
    pPos->name = m_pNamesEnd - GetNamesBegin();
    pPos->type = KT_Varible;
    pPos->offset = m_cbDataLen;
    pPos->length = nLength;

    CLBYTE*const pName = (CLBYTE*)GetNamesBegin() + pPos->name;
    memset(pName, 0, cbKeyNameLen);
    strcpyT((TChar*)pName, szKey);
    m_pNamesEnd = (LPCSTR)((CLBYTE*)m_pNamesEnd + cbKeyNameLen);
    
    memcpy(m_pData + pPos->offset, pData, nLength);
    m_cbDataLen += nLength;
    return true;
  }

  b32 Repository::_ReplaceKey(KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength)
  {
    ASSERT(pPos >= m_pKeys && pPos <= m_pKeysEnd);
    if(pPos->length == nLength) {
      memcpy(m_pData + pPos->offset, pData, nLength);
      return TRUE;
    }
    CLBREAK; // 没实现
    return FALSE;
  }


  void Repository::_Initialize(size_t cbDataLen)
  {
    ASSERT(m_pKeys == NULL);
    m_Buffer.Resize(sizeof(KEY) * KEYINCCOUNT + sizeof(TChar) * NAMEINCCOUNT + cbDataLen, TRUE);
    m_pKeys         = reinterpret_cast<KEY*>(m_Buffer.GetPtr());
    m_pKeysEnd      = m_pKeys;
    m_pKeysCapacity = m_pKeys + KEYINCCOUNT;

    m_pNamesEnd     = GetNamesBegin();
    m_pData         = (CLBYTE*)GetNamesBegin() + sizeof(TChar) * NAMEINCCOUNT;          // Name capacity
    m_cbDataLen     = 0;
  }

  b32 Repository::LoadFromFile( CLLPCSTR szFilename )
  {
    return LoadFromFileT(szFilename);
  }

  b32 Repository::LoadFromFile( CLLPCWSTR szFilename )
  {
    return LoadFromFileT(szFilename);
  }

  b32 Repository::LoadFromMemory( const void* pData, size_t nLength )
  {
    m_Buffer.Append(pData, nLength);
    return ParseFromBuffer();
  }

  b32 Repository::SaveToFile( CLLPCSTR szFilename ) const
  {
    return false;
  }

  b32 Repository::SaveToMemory( const void* pData, size_t nLength ) const
  {
    return false;
  }

  size_t Repository::GetNumOfKeys() const
  {
    return m_pKeysEnd - m_pKeys;
  }

  b32 Repository::SetKey( LPCSTR szKey, const void* pData, size_t nLength )
  {
    if(m_Buffer.GetSize() == 0) {
      _Initialize(nLength);
      return _InsertKey(m_pKeys, szKey, pData, nLength);
    }

    KEYPAIR pair;
    if(_PreInsertKey(&pair, szKey)->bInsert) {
      return _InsertKey(pair.pTable, szKey, pData, nLength);
    }
    else {
      return _ReplaceKey(pair.pTable, szKey, pData, nLength);
    }
    return false;
  }

  size_t Repository::GetKey( LPCSTR szKey, void* pData, size_t nLength ) const
  {
    KEY* pKey = _FindKey(szKey);
    if(pKey) {
      size_t copy_size = 0;
      CLBYTE*const pSrcData = pKey->GetDataPtr(m_pData, &copy_size);

      if(pData == NULL || nLength == NULL) {
        return copy_size;
      }

      copy_size = clMin(copy_size, nLength);
      memcpy(pData, pSrcData, copy_size);
      return copy_size;
    }
    return 0;
  }

  void* Repository::GetDataPtr( LPCSTR szKey, size_t* pLength ) const
  {
    KEY* pKey = _FindKey(szKey);
    if(pKey) {
      size_t len = 0;
      return pKey->GetDataPtr(m_pData, pLength ? pLength : &len);
    }
    return NULL;
  }

  //b32 Repository::RemoveKey( LPCSTR szKey )
  //{
  //  return false;
  //}

  //b32 Repository::RemoveKey( CLDWORD dwKey )
  //{
  //  return false;
  //}

  //size_t Repository::GetKey( CLDWORD szKey, void* pData, size_t nLength ) const
  //{
  //  return false;
  //}

  //b32 Repository::SetKey( CLDWORD szKey, const void* pData, size_t nLength )
  //{
  //  return false;
  //}

  b32 Repository::SaveToFile( CLLPCWSTR szFilename ) const
  {
    return false;
  }

  Repository::iterator::iterator( const Repository* _repo, KEY* _key )
    : repo(_repo)
    , key(_key)
  {    
  }

  LPCSTR Repository::iterator::name() const
  {
    return key->GetName(repo->GetNamesBegin());
  }

  void* Repository::iterator::ptr() const
  {
    return key->GetDataPtr(repo->m_pData);
  }

  size_t Repository::iterator::size() const
  {
    return key->GetDataSize();
  }

  Repository::iterator& Repository::iterator::operator++()
  {
    ++key;
    return *this;
  }

  Repository::iterator& Repository::iterator::operator++(int)
  {
    return operator++();
  }

  b32 Repository::iterator::operator==(const iterator& it)
  {
    ASSERT(repo == it.repo);
    return key == it.key;
  }

  b32 Repository::iterator::operator!=(const iterator& it)
  {
    return ! operator==(it);
  }

  b32 Repository::iterator::operator==(LPCSTR szKey)
  {
    return strcmpT(name(), szKey) == 0;
  }

  b32 Repository::iterator::operator!=(LPCSTR szKey)
  {
    return ! operator==(szKey);
  }
} // namespace clstd
