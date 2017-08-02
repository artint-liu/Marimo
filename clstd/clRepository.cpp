#include "clstd.h"
#include "clString.h"
#include "clRepository.h"
#include "clUtility.h"

#define KEYINCCOUNT 8
#define NAMEINCCOUNT 128
#define OCTETSIZE 8

#define REPO_MAGIC           CLMAKEFOURCC('C','L','R','P')
#define REPO_MAGIC_BIGENDIAN CLMAKEFOURCC('P','R','L','C')

namespace clstd
{
  namespace _internal
  {
    enum KeyType
    {
      //KT_Node,    // 节点
      KeyType_Varible = 1,               // 变长数据
      KeyType_Octet   = 0x0010,          // 标志
      KeyType_Octet_0 = (0 | KeyType_Octet),  // 8字节数据，长度可以为0，仅存键值
      KeyType_Octet_8 = (8 | KeyType_Octet),  // 8字节数据，长度8字节
    };

    struct KEY
    {
      u32     name; // 字母偏移，不是字节
      KeyType type;
      union 
      {
        struct {
          u32 offset;
          u32 length;
        }v; // varible
        struct {
          CLBYTE data[8];
        }o; // octet
      };
      LPCSTR  GetName     (LPCSTR szFirstName) const;
      CLBYTE* GetDataPtr  (CLBYTE* pDataBase) const;
      CLBYTE* GetDataPtr  (CLBYTE* pDataBase, size_t* pSizeOut) const;
      size_t  GetDataSize () const;
    };

    struct KEYPAIR
    {
      KEY* pTable;
      b32 bInsert;
    };

    //////////////////////////////////////////////////////////////////////////

    enum HeaderFlag{
      HeaderFlag_KeyFieldMask     = 0x0003,
      HeaderFlag_KeyField_1Byte   = 0x0003, // 255 Keys
      HeaderFlag_KeyField_2Bytes  = 0x0002, // 65535 Keys
      HeaderFlag_KeyField_4Bytes  = 0x0000,

      HeaderFlag_NameFieldMask    = 0x000c,
      HeaderFlag_NameField_1Byte  = 0x000c, // 255 Bytes names buffer
      HeaderFlag_NameField_2Bytes = 0x0008,
      HeaderFlag_NameField_4Bytes = 0x0000,

      HeaderFlag_DataFieldMask    = 0x00010,
      HeaderFlag_DataField_64KB   = 0x00010, // 64K buffer
      HeaderFlag_DataField_4GB    = 0x00000, // 4G buffer
      
      HeaderFlag_AllFieldMask     = HeaderFlag_KeyFieldMask | HeaderFlag_NameFieldMask | HeaderFlag_DataFieldMask
    };

    struct FILE_HEADER
    {
      CLDWORD dwMagic; // CLRP
      CLDWORD dwFlags; // HeaderFlag
      CLDWORD nKeys;
      CLDWORD cbNames;
      CLDWORD cbData;

      size_t _GetRecordSize() const
      {
        class _R : public Repository
        {
        public:
          //typedef Repository::KEY KEY;
        };
        return nKeys * sizeof(KEY) + cbNames + cbData;
      }
    };

    template<typename dest_t, typename src_t>
    CLBYTE* WriteStream(CLBYTE* pDest, src_t src)
    {
      *reinterpret_cast<dest_t*>(pDest) = static_cast<dest_t>(src);
      return (pDest + sizeof(dest_t));
    }

    template<typename src_t, typename dest_t>
    const CLBYTE* ReadStream(dest_t& dest, const CLBYTE* pSrc)
    {
      dest = static_cast<dest_t>(*reinterpret_cast<const src_t*>(pSrc));
      return (pSrc + sizeof(src_t));
    }

    template<typename _key_t, typename _name_t, typename _data_t>
    struct FILE_HEADERT
    {
      CLDWORD dwMagic; // CLRP
      CLDWORD dwFlags; // HeaderFlag
      //u32     data[1];
      //_key_t  nKeys;
      //_name_t cbNames;
      //_data_t cbData;

      size_t CopyFrom(const FILE_HEADER& h, CLDWORD dwAddFlags)
      {
        dwMagic = h.dwMagic;
        dwFlags = h.dwFlags | dwAddFlags;
        CLBYTE* pVariData = reinterpret_cast<CLBYTE*>(this) + sizeof(FILE_HEADERT);
        //*reinterpret_cast<_key_t*>(pVariData) = static_cast<_key_t>(h.nKeys);
        //pVariData += sizeof(_key_t);

        //*reinterpret_cast<_name_t*>(pVariData) = static_cast<_name_t>(h.cbNames);
        //pVariData += sizeof(_name_t);

        //*reinterpret_cast<_data_t*>(pVariData) = static_cast<_data_t>(h.cbData);
        pVariData = WriteStream<_key_t>(pVariData, h.nKeys);
        pVariData = WriteStream<_name_t>(pVariData, h.cbNames);
        pVariData = WriteStream<_data_t>(pVariData, h.cbData);

        //cbNames = (_name_t)h.cbNames;
        //cbData  = (_data_t)h.cbData;
        return sizeof(FILE_HEADERT) + sizeof(_key_t) + sizeof(_name_t) + sizeof(_data_t);
      }

      size_t CopyTo(FILE_HEADER& h) const
      {
        h.dwMagic = dwMagic;
        h.dwFlags = dwFlags & (~HeaderFlag_AllFieldMask);
        const CLBYTE* pVariData = reinterpret_cast<const CLBYTE*>(this) + sizeof(FILE_HEADERT);

        //h.nKeys   = *reinterpret_cast<const _key_t*>(pVariData);
        //pVariData += sizeof(_key_t);

        //h.cbNames = *reinterpret_cast<const _name_t*>(pVariData);
        //pVariData += sizeof(_name_t);
        //
        //h.cbData  = *reinterpret_cast<const _data_t*>(pVariData);

        pVariData = ReadStream<_key_t>(h.nKeys, pVariData);
        pVariData = ReadStream<_name_t>(h.cbNames, pVariData);
        pVariData = ReadStream<_data_t>(h.cbData, pVariData);

        //return sizeof(FILE_HEADERT);
        return sizeof(FILE_HEADERT) + sizeof(_key_t) + sizeof(_name_t) + sizeof(_data_t);
      }
    };

    STATIC_ASSERT(sizeof(FILE_HEADERT<u32, u32, u32>) == 8);


    template<typename _key_t, typename _name_t> 
    size_t _PackHeader_DataT(const FILE_HEADER& header, u32 dwFlags, CLBYTE* pBuffer)
    {
      if(header.cbData <= 0xffff) {
        typedef FILE_HEADERT<_key_t, _name_t, u16> HEADER_INST;
        HEADER_INST* packed_header = reinterpret_cast<HEADER_INST*>(pBuffer);
        return packed_header->CopyFrom(header, dwFlags | HeaderFlag_DataField_64KB);
      }
      else
      {
        typedef FILE_HEADERT<_key_t, _name_t, u32> HEADER_INST;
        HEADER_INST* packed_header = reinterpret_cast<HEADER_INST*>(pBuffer);
        return packed_header->CopyFrom(header, dwFlags | HeaderFlag_DataField_4GB);
      }
    }

    template<typename _key_t> 
    size_t _PackHeader_NameDataT(const FILE_HEADER& header, u32 dwFlags, CLBYTE* pBuffer)
    {
      if(header.cbNames <= 0xff) {
        return _PackHeader_DataT<_key_t, u8>(header, dwFlags | HeaderFlag_NameField_1Byte, pBuffer);
      } else if(header.cbNames <= 0xffff) {
        return _PackHeader_DataT<_key_t, u16>(header, dwFlags | HeaderFlag_NameField_2Bytes, pBuffer);
      }
      return _PackHeader_DataT<_key_t, u32>(header, dwFlags | HeaderFlag_NameField_4Bytes, pBuffer);
    }

    size_t _PackHeader( const FILE_HEADER& header, CLBYTE* pBuffer )
    {
      //*
      if(header.nKeys <= 0xff) {
        return _PackHeader_NameDataT<u8>(header, HeaderFlag_KeyField_1Byte, pBuffer);
      } else if(header.nKeys <= 0xffff) {
        return _PackHeader_NameDataT<u16>(header, HeaderFlag_KeyField_2Bytes, pBuffer);
      }
      return _PackHeader_NameDataT<u32>(header, HeaderFlag_KeyField_4Bytes, pBuffer);
      /*/
      typedef FILE_HEADERT<u32, u32, u32> HEADER_PACKED_T;
      HEADER_PACKED_T* header_packed = reinterpret_cast<HEADER_PACKED_T*>(pBuffer);
      return header_packed->CopyFrom(header, 0);
      //*/
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename _key_t, typename _name_t> 
    size_t _UnpackHeader_DataT(FILE_HEADER& header, const CLBYTE* pBuffer)
    {
      if((header.dwFlags & HeaderFlag_DataFieldMask) == HeaderFlag_DataField_64KB) {
        typedef FILE_HEADERT<_key_t, _name_t, u16> HEADER_INST;
        const HEADER_INST* packed_header = reinterpret_cast<const HEADER_INST*>(pBuffer);
        return packed_header->CopyTo(header);
      }
      else
      {
        typedef FILE_HEADERT<_key_t, _name_t, u32> HEADER_INST;
        const HEADER_INST* packed_header = reinterpret_cast<const HEADER_INST*>(pBuffer);
        return packed_header->CopyTo(header);
      }
    }

    template<typename _key_t> 
    size_t _UnpackHeader_NameDataT(FILE_HEADER& header, const CLBYTE* pBuffer)
    {
      if((header.dwFlags & HeaderFlag_NameFieldMask) == HeaderFlag_NameField_1Byte) {
        return _UnpackHeader_DataT<_key_t, u8>(header, pBuffer);
      } else if((header.dwFlags & HeaderFlag_NameFieldMask) == HeaderFlag_NameField_2Bytes) {
        return _UnpackHeader_DataT<_key_t, u16>(header, pBuffer);
      }
      ASSERT((header.dwFlags & HeaderFlag_NameFieldMask) == HeaderFlag_NameField_4Bytes);
      return _UnpackHeader_DataT<_key_t, u32>(header, pBuffer);
    }

    size_t _UnpackHeader( FILE_HEADER& header, const CLBYTE* pBuffer )
    {
      if((header.dwFlags & HeaderFlag_KeyFieldMask) == HeaderFlag_KeyField_1Byte) {
        return _UnpackHeader_NameDataT<u8>(header, pBuffer);
      } else if((header.dwFlags & HeaderFlag_KeyFieldMask) == HeaderFlag_KeyField_1Byte) {
        return _UnpackHeader_NameDataT<u16>(header, pBuffer);
      }
      return _UnpackHeader_NameDataT<u32>(header, pBuffer);
    }
  } // namespace

  //////////////////////////////////////////////////////////////////////////

  RepoReader::RepoReader()
    : m_pKeys    (NULL)
    , m_pKeysEnd (NULL)
    , m_pKeysCapacity(NULL)
    , m_pNamesEnd (NULL)
    , m_pData     (NULL)
    , m_cbDataLen (0)
  {
  }

  RepoReader::RepoReader(const void* pData, size_t nLength)
  {
    Attach(pData, nLength);
  }

  b32 RepoReader::Attach(const void* pData, size_t nLength)
  {
    size_t nDataPtr = reinterpret_cast<size_t>(pData);
    _internal::FILE_HEADER header = { 0, ((CLDWORD*)nDataPtr)[1] }; // 需要设置dwFlags作为解压参数
    size_t packed_header_size = _UnpackHeader(header, (const CLBYTE*)nDataPtr);
    if(header.dwMagic != REPO_MAGIC) {
      // ERROR: bad file magic
      return false;
    }
    nDataPtr += packed_header_size;

    m_pKeys         = reinterpret_cast<KEY*>(nDataPtr);
    m_pKeysEnd      = m_pKeys + header.nKeys;
    m_pKeysCapacity = m_pKeysEnd;
    m_pNamesEnd     = reinterpret_cast<LPCSTR>(reinterpret_cast<size_t>(m_pKeysEnd) + header.cbNames);
    m_pData         = (CLBYTE*)m_pNamesEnd;
    m_cbDataLen     = header.cbData;

    size_t nCheckLen = packed_header_size + sizeof(KEY) * header.nKeys + header.cbNames + header.cbData;
    if(nCheckLen > nLength) {
      // ERROR: not enough data
      return false;
    }
    else if(nCheckLen < nLength)
    {
      // WARNING: has redundancy data
      return true;
    }
    return true;
  }

  size_t RepoReader::GetNumOfKeys() const
  {
    return m_pKeysEnd - m_pKeys;
  }

  size_t RepoReader::GetKey(LPCSTR szKey, void* pData, size_t nLength) const
  {
    KEY* pKey = _FindKey(szKey);
    if(pKey) {
      size_t copy_size = 0;
      CLBYTE*const pSrcData = pKey->GetDataPtr(m_pData, &copy_size);

      if(pData == NULL || nLength == 0) {
        return copy_size;
      }

      copy_size = clMin(copy_size, nLength);
      clmemmove(pData, pSrcData, copy_size);
      return copy_size;
    }
    return 0;
  }

  void* RepoReader::GetDataPtr( LPCSTR szKey, size_t* pLength ) const
  {
    KEY* pKey = _FindKey(szKey);
    if(pKey) {
      size_t len = 0;
      return pKey->GetDataPtr(m_pData, pLength ? pLength : &len);
    }
    return NULL;
  }

  size_t RepoReader::GetRawData(RAWDATA* pRaw) const
  {
    if(pRaw == NULL) {
      return 0;
    }

    _internal::FILE_HEADER header = { REPO_MAGIC };
    header.nKeys   = (CLDWORD)(m_pKeysEnd - m_pKeys);
    header.cbNames = (CLDWORD)(m_pNamesEnd - _GetNamesBegin()) * sizeof(CHAR);
    header.cbData  = (CLDWORD)m_cbDataLen;

    pRaw->keys      = m_pKeys;
    pRaw->cbKeys    = header.nKeys * sizeof(KEY);
    pRaw->names     = _GetNamesBegin();
    pRaw->cbNames   = header.cbNames;
    pRaw->data      = m_pData;
    pRaw->cbData    = header.cbData;
    pRaw->cbHeader  = _PackHeader(header, pRaw->header);

    return pRaw->cbHeader + pRaw->cbKeys + pRaw->cbNames + pRaw->cbData;
  }

  RepoReader::KEY* RepoReader::_FindKey(LPCSTR szKey) const
  {
    LPCSTR szNamesFront = _GetNamesBegin();
    KEY*const result =
      clstd::BinarySearch(m_pKeys, m_pKeysEnd, szKey,
        [szNamesFront](KEY* pMid, LPCSTR key) -> int
    {
      return strcmpT(pMid->GetName(szNamesFront), key);
    });
    return result;
  }

  LPCSTR RepoReader::_GetNamesBegin() const
  {
    return reinterpret_cast<LPCSTR>(m_pKeysCapacity);
  }

  LPCSTR RepoReader::_GetNamesCapacity() const
  {
    return reinterpret_cast<LPCSTR>(m_pData);
  }

  RepoReader::iterator RepoReader::begin() const
  {
    iterator it(this, m_pKeys);
    return it;
  }

  RepoReader::iterator RepoReader::end() const
  {
    iterator it(this, m_pKeysEnd);
    return it;
  }

  //////////////////////////////////////////////////////////////////////////

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

    return _ParseFromBuffer();
  }

  b32 Repository::_WriteToFile(File& file) const
  {
    _internal::FILE_HEADER header = {};
    header.dwMagic = REPO_MAGIC;
    header.nKeys   = (CLDWORD)(m_pKeysEnd - m_pKeys);
    header.cbNames = (CLDWORD)(m_pNamesEnd - _GetNamesBegin()) * sizeof(CHAR);
    header.cbData  = (CLDWORD)m_cbDataLen;

    CLBYTE packed_header[sizeof(_internal::FILE_HEADER)]; // 打包Header肯定比展开得Header小
    size_t packed_header_size = _PackHeader(header, packed_header);

    file.Write(packed_header, (u32)packed_header_size);
    file.Write(m_pKeys, (u32)((size_t)m_pKeysEnd - (size_t)m_pKeys));
    file.Write(_GetNamesBegin(), header.cbNames);
    file.Write(m_pData, (u32)m_cbDataLen);
    return TRUE;
  }

  b32 Repository::_DbgCheckDataOverlay() const
  {
    if((CLBYTE*)m_Buffer.GetPtr() + m_Buffer.GetSize() != m_pData + m_cbDataLen)
    {
      CLOG_ERROR("Buffer size doesn't match:\"m_Buffer.GetSize()\" is %d, m_cbDataLen is %d\n", 
        m_Buffer.GetSize(), m_pData - (CLBYTE*)m_Buffer.GetPtr() + m_cbDataLen);
      return FALSE;
    }

    KEY* pPrev = m_pKeys;
    while(pPrev < m_pKeysEnd) {
      if(pPrev->type == _internal::KeyType_Varible) {
        break;
      }
      pPrev++;
    }

    // 没找到KeyType_Varible
    if(pPrev == m_pKeysEnd) {
      return TRUE;
    }

    for(KEY* pIter = pPrev + 1; pIter < m_pKeysEnd; ++pIter) {
      if(pIter->type != _internal::KeyType_Varible) {
        continue;
      }
      if(pPrev->v.offset + pPrev->v.length != pIter->v.offset) {
        return FALSE;
      }
      pPrev = pIter;
    }

    return TRUE;
  }

  Repository::Repository()
  {
  }

  Repository::~Repository()
  {
  }

  Repository& Repository::operator=(const Repository& repo)
  {
    m_Buffer.Replace(0, m_Buffer.GetSize(), repo.m_Buffer.GetPtr(), repo.m_Buffer.GetSize());

    m_pKeys         = repo.m_pKeys;
    m_pKeysEnd      = repo.m_pKeysEnd;
    m_pKeysCapacity = repo.m_pKeysCapacity;
    m_pNamesEnd     = repo.m_pNamesEnd;
    m_pData         = repo.m_pData;
    m_cbDataLen     = m_cbDataLen;

    const size_t delta = (size_t)m_Buffer.GetPtr() - (size_t)repo.m_Buffer.GetPtr();
    _Locate(delta);
    return *this;
  }

  void Repository::Clear()
  {
    m_Buffer.Resize(0, FALSE);
    m_pKeys         = NULL;
    m_pKeysEnd      = NULL;
    m_pKeysCapacity = NULL;
    m_pNamesEnd     = NULL;
    m_pData         = NULL;
    m_cbDataLen     = 0;
  }

  b32 Repository::_ParseFromBuffer()
  {
    return Attach(m_Buffer.GetPtr(), m_Buffer.GetSize());
    //size_t pData = reinterpret_cast<size_t>(m_Buffer.GetPtr());
    //FILE_HEADER header = { 0, ((CLDWORD*)pData)[1] }; // 需要设置dwFlags作为解压参数
    //size_t packed_header_size = _UnpackHeader(header, (const CLBYTE*)pData);
    //if(header.dwMagic != REPO_MAGIC) {
    //  // ERROR: bad file magic
    //  return false;
    //}
    //pData += packed_header_size;

    //m_pKeys         = reinterpret_cast<KEY*>(pData);
    //m_pKeysEnd      = m_pKeys + header.nKeys;
    //m_pKeysCapacity = m_pKeysEnd;
    //m_pNamesEnd     = reinterpret_cast<LPCSTR>(reinterpret_cast<size_t>(m_pKeysEnd) + header.cbNames);
    //m_pData         = (CLBYTE*)m_pNamesEnd;
    //m_cbDataLen     = header.cbData;

    //size_t nCheckLen = packed_header_size + sizeof(KEY) * header.nKeys + header.cbNames + header.cbData;
    //if(nCheckLen > m_Buffer.GetSize()) {
    //  // ERROR: not enough data
    //  return false;
    //}
    //else if(nCheckLen < m_Buffer.GetSize())
    //{
    //  // WARNING: has redundancy data
    //  return true;
    //}
    //return true;
  }

  size_t Repository::_ResizeGlobalBuf(size_t delta, KEY*& rpKey)
  {
    size_t mem_delta = m_Buffer.Resize(m_Buffer.GetSize() + delta, FALSE);
    if(mem_delta) {
      _Locate(mem_delta);
      rpKey = (KEY*)((size_t)rpKey + mem_delta);
    }
    return mem_delta;
  }

  u32 Repository::_GetSeqOffset(KEY* pPos)
  {
    ASSERT(pPos >= m_pKeys && pPos <= m_pKeysEnd);

    while(pPos > m_pKeys) {
      if((--pPos)->type == _internal::KeyType_Varible) {
        return (pPos->v.offset + pPos->v.length);
      }
    }
    return 0;
  }

  size_t Repository::_GetDataCapacity() const
  {
    return ((size_t)m_Buffer.GetPtr() + m_Buffer.GetSize()) - (size_t)m_pData;
  }

  Repository::KEYPAIR* Repository::_PreInsertKey(KEYPAIR* pPair, LPCSTR szKey)
  {
    LPCSTR szNamesFront = _GetNamesBegin();
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

  b32 Repository::_AllocKey(size_t cbKeyNameLen/*, const void* pData*/, size_t nLength, KEY*& rPos)
  {
    size_t extened = 0;
    size_t key_inc = 0;
    size_t name_inc = 0;
    ASSERT(m_pKeysEnd <= m_pKeysCapacity);
    ASSERT(m_pNamesEnd <= _GetNamesCapacity());

    if(m_pKeysEnd == m_pKeysCapacity) {
      key_inc = KEYINCCOUNT * sizeof(KEY);
      extened += key_inc;
    }

    if((size_t)m_pNamesEnd + cbKeyNameLen > (size_t)_GetNamesCapacity()) {
      name_inc = (cbKeyNameLen + NAMEINCCOUNT * sizeof(CHAR));
      extened += name_inc;
    }

    if(m_cbDataLen + nLength > _GetDataCapacity()) {
      extened += nLength;
    }

    if(extened > 0) {
      _ResizeGlobalBuf(extened, rPos);

      if(name_inc > 0) {
        // 注意嵌套调整
        clmemmove(m_pData + key_inc + name_inc, m_pData, m_cbDataLen);

        if(key_inc > 0) {
          const size_t cbNamesLen = (_GetNamesCapacity() - _GetNamesBegin()) * sizeof(CHAR);
          clmemmove((CLBYTE*)_GetNamesBegin() + key_inc, _GetNamesBegin(), cbNamesLen);
          m_pKeysCapacity = (KEY*)((CLBYTE*)m_pKeysCapacity + key_inc);
        }

        m_pData += (key_inc + name_inc);
      }
      else if(key_inc > 0) {
        const size_t cbNamesLen = (_GetNamesCapacity() - _GetNamesBegin()) * sizeof(CHAR);
        clmemmove((CLBYTE*)_GetNamesBegin() + key_inc, _GetNamesBegin(), cbNamesLen + m_cbDataLen);
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
    const size_t cbKeyNameLen = ALIGN_2((strlenT(szKey) + 1) * sizeof(CHAR));// 算上结尾'\0'，按照两字节对齐
    if(nLength < OCTETSIZE)
    {

      _AllocKey(cbKeyNameLen, 0, pPos);

      ASSERT(m_pKeysEnd < m_pKeysCapacity);

      const size_t cbKeyLen = (m_pKeysEnd - pPos) * sizeof(KEY);
      if(cbKeyLen) {
        clmemmove(pPos + 1, pPos, cbKeyLen);
      }
      m_pKeysEnd++;
      pPos->name = (u32)(m_pNamesEnd - _GetNamesBegin());
      pPos->type = (_internal::KeyType)(_internal::KeyType_Octet_0 + nLength);
      clmemmove(pPos->o.data, pData, nLength);
    }
    else
    {
      _AllocKey(cbKeyNameLen, nLength, pPos);

      ASSERT(m_pKeysEnd < m_pKeysCapacity);

      const size_t cbKeyLen = (m_pKeysEnd - pPos) * sizeof(KEY);
      if(cbKeyLen) {
        clmemmove(pPos + 1, pPos, cbKeyLen);
      }
      m_pKeysEnd++;
      pPos->name = (u32)(m_pNamesEnd - _GetNamesBegin());
      pPos->type = _internal::KeyType_Varible;
      pPos->v.offset = _GetSeqOffset(pPos);
      pPos->v.length = 0;

      ASSERT((CLBYTE*)m_Buffer.GetPtr() + m_Buffer.GetSize() == m_pData + m_cbDataLen + nLength);
      _ResizeKey(pPos, nLength);
      ASSERT(_DbgCheckDataOverlay());
      clmemmove(m_pData + pPos->v.offset, pData, nLength);
    }

    // Copy name
    CLBYTE*const pName = (CLBYTE*)_GetNamesBegin() + pPos->name;
    memset(pName, 0, cbKeyNameLen);
    strcpyT((CHAR*)pName, szKey);
    m_pNamesEnd = (LPCSTR)((CLBYTE*)m_pNamesEnd + cbKeyNameLen);

    return true;
  }

  void Repository::_ResizeKey(KEY*& rKey, size_t delta)
  {
    ASSERT(rKey->type == _internal::KeyType_Varible);
    const size_t nDataEndian = rKey->v.offset + rKey->v.length;
    CLBYTE*const pDataEndian = m_pData + nDataEndian;

    // TODO: 如果保证数据顺序与Key顺序一致的话pIter可以从rKey开始
    for(KEY* pIter = m_pKeys; pIter < m_pKeysEnd; ++pIter)
    {
      if(TEST_FLAG_NOT(pIter->type, _internal::KeyType_Octet) && pIter != rKey && pIter->v.offset >= rKey->v.offset) {
        ASSERT(rKey->v.offset + rKey->v.length <= pIter->v.offset); // 检查数据覆盖
        pIter->v.offset += static_cast<u32>(delta);
      }
    }
    rKey->v.length += (u32)delta;
    //return m_pData + rKey->v.offset + rKey->v.length; // 返回当前键值数据的结尾
    if(m_cbDataLen > nDataEndian) {
      clmemmove(pDataEndian + delta, pDataEndian, m_cbDataLen - nDataEndian);
    }
    m_cbDataLen += delta;
  }

  b32 Repository::_ReallocData(KEY*& rKey, size_t new_length)
  {
    // 没有处理KT_Octet_0 ~ KT_Octet_8情况
    ASSERT(rKey->type == _internal::KeyType_Varible);
    ASSERT((CLBYTE*)m_Buffer.GetPtr() + m_Buffer.GetSize() == m_pData + m_cbDataLen);

    size_t delta = new_length - static_cast<size_t>(rKey->GetDataSize());
    //const size_t nDataEndian = rKey->v.offset + rKey->v.length;
    //CLBYTE*const pDataEndian = m_pData + nDataEndian;

    if(rKey->v.length < new_length)
    {
      _ResizeGlobalBuf(delta, rKey);

      _ResizeKey(rKey, delta);
      //memcpy(pDataEndian + delta, pDataEndian, m_cbDataLen - nDataEndian);

      //rKey->v.length += delta;
    }
    else if(rKey->v.length > new_length)
    {
      //CLBYTE*const pDataEndian = m_pData + nDataEndian;
      _ResizeKey(rKey, delta);
      //memcpy(pDataEndian + delta, pDataEndian, m_cbDataLen - nDataEndian);

      _ResizeGlobalBuf(delta, rKey);
    }
    else {
      CLBREAK; // 等于情况外面处理，这里限制
    }

    //m_cbDataLen += delta;

    ASSERT((CLBYTE*)m_Buffer.GetPtr() + m_Buffer.GetSize() == m_pData + m_cbDataLen);
    return TRUE;
  }

  b32 Repository::_ReplaceKey(KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength)
  {
    ASSERT(pPos >= m_pKeys && pPos <= m_pKeysEnd);
    if(nLength <= OCTETSIZE)
    {
      if(TEST_FLAG(pPos->type, _internal::KeyType_Octet)) {
        pPos->type = (_internal::KeyType)(_internal::KeyType_Octet + nLength);
        clmemmove(pPos->o.data, pData, nLength);
        ASSERT(_DbgCheckDataOverlay());
        return TRUE;
      }
      else if(pPos->type == _internal::KeyType_Varible) {
        const size_t key_delta = 0 - (size_t)pPos->v.length;
        _ResizeKey(pPos, key_delta);
        _ResizeGlobalBuf(key_delta, pPos);
        pPos->type = (_internal::KeyType)(_internal::KeyType_Octet + nLength);
        clmemmove(pPos->o.data, pData, nLength);
        ASSERT(_DbgCheckDataOverlay());
        return TRUE;
      }
      else {
        CLBREAK;
      }
    }
    else
    {
      if(TEST_FLAG(pPos->type, _internal::KeyType_Octet)) {
        pPos->type = _internal::KeyType_Varible;
        pPos->v.offset = _GetSeqOffset(pPos);
        pPos->v.length = 0;

        if(_ReallocData(pPos, nLength)) {
          clmemmove(m_pData + pPos->v.offset, pData, nLength);
          ASSERT(_DbgCheckDataOverlay());
          return TRUE;
        }

      }
      else if(pPos->type, _internal::KeyType_Varible) {
        if(pPos->v.length == nLength || _ReallocData(pPos, nLength)) {
          clmemmove(m_pData + pPos->v.offset, pData, nLength);
          ASSERT(_DbgCheckDataOverlay());
          return TRUE;
        }
      }
      else {
        CLBREAK;
      }
    }

    return FALSE;
  }


  void Repository::_Initialize(size_t cbDataLen)
  {
    ASSERT(m_pKeys == NULL);
    m_Buffer.Resize(sizeof(KEY) * KEYINCCOUNT + sizeof(CHAR) * NAMEINCCOUNT + cbDataLen, TRUE);
    m_pKeys         = reinterpret_cast<KEY*>(m_Buffer.GetPtr());
    m_pKeysEnd      = m_pKeys;
    m_pKeysCapacity = m_pKeys + KEYINCCOUNT;

    m_pNamesEnd     = _GetNamesBegin();
    m_pData         = (CLBYTE*)_GetNamesBegin() + sizeof(CHAR) * NAMEINCCOUNT;          // Name capacity
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
    m_Buffer.Replace(0, m_Buffer.GetSize(), pData, nLength);
    return _ParseFromBuffer();
  }

  b32 Repository::LoadFromMemory(const BufferBase& buf)
  {
    //m_Buffer = buf;
    m_Buffer.Replace(0, m_Buffer.GetSize(), buf.GetPtr(), buf.GetSize());
    return _ParseFromBuffer();
  }

  b32 Repository::SaveToFile(CLLPCSTR szFilename) const
  {
    File file;
    if(file.CreateAlways(szFilename)) {
      return _WriteToFile(file);
    }
    return false;
  }

  b32 Repository::SaveToFile( CLLPCWSTR szFilename ) const
  {
    File file;
    if(file.CreateAlways(szFilename)) {
      return _WriteToFile(file);
    }
    return false;
  }


  b32 Repository::SaveToMemory(MemBuffer& buffer) const
  {
    _internal::FILE_HEADER header = {};
    header.dwMagic = CLMAKEFOURCC('C', 'L', 'R', 'P');
    header.nKeys = (CLDWORD)(m_pKeysEnd - m_pKeys);
    header.cbNames = (CLDWORD)(m_pNamesEnd - _GetNamesBegin()) * sizeof(CHAR);
    header.cbData = (CLDWORD)m_cbDataLen;

    CLBYTE packed_header[sizeof(_internal::FILE_HEADER)]; // 打包Header肯定比展开得Header小
    size_t packed_header_size = _PackHeader(header, packed_header);

    buffer.Reserve(packed_header_size + (m_pKeysEnd - m_pKeys) * sizeof(KEY) +
      header.cbNames + m_cbDataLen);

    buffer.Append(packed_header, (u32)packed_header_size);
    buffer.Append(m_pKeys, (u32)((size_t)m_pKeysEnd - (size_t)m_pKeys));
    buffer.Append(_GetNamesBegin(), header.cbNames);
    buffer.Append(m_pData, (u32)m_cbDataLen);
    return TRUE;
  }

  b32 Repository::SetKey( LPCSTR szKey, const void* pData, size_t nLength )
  {
    if(m_Buffer.GetSize() == 0) {
      _Initialize(nLength > OCTETSIZE ? nLength : 0);
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

  b32 Repository::SetKey(LPCSTR szKey, const wch* str)
  {
    const size_t len = (strlenT(str) + 1) * sizeof(wch);
    return SetKey(szKey, str, len);
  }
  
  b32 Repository::SetKey(LPCSTR szKey, const ch* str)
  {
    const size_t len = (strlenT(str) + 1) * sizeof(ch);
    return SetKey(szKey, str, len);
  }

  b32 Repository::SetKey(LPCSTR szKey, const clStringW& str)
  {
    return SetKey(szKey, str, (str.GetLength() + 1) * sizeof(clStringW::TChar));
  }

  b32 Repository::SetKey(LPCSTR szKey, const clStringA& str)
  {
    return SetKey(szKey, str, (str.GetLength() + 1) * sizeof(clStringA::TChar));
  }

  b32 Repository::SetKey(LPCSTR szKey, i32 value)
  {
    return SetKey(szKey, &value, sizeof(value));
  }

  b32 Repository::SetKey(LPCSTR szKey, u32 value)
  {
    return SetKey(szKey, &value, sizeof(value));
  }

  b32 Repository::SetKey(LPCSTR szKey, i64 value)
  {
    return SetKey(szKey, &value, sizeof(value));
  }

  b32 Repository::SetKey(LPCSTR szKey, u64 value)
  {
    return SetKey(szKey, &value, sizeof(value));
  }

  size_t Repository::GetRequiredSize(void* pData, size_t nLength)
  {
    // 这个长度限制其实没有那么严格，因为文件头长度是可变的，最长20字节
    if(nLength < sizeof(_internal::FILE_HEADER) || *(u32*)pData != REPO_MAGIC) {
      return 0;
    }

    _internal::FILE_HEADER header = { 0, ((CLDWORD*)pData)[1] }; // 需要设置dwFlags作为解压参数
    size_t packed_header_size = _UnpackHeader(header, (const CLBYTE*)pData);
    return header._GetRecordSize() + packed_header_size;
  }

  i32 Repository::IsRepository(const void* pData, size_t nLength)
  {
    if(nLength < 4) {
      return 0;
    }
    else if(*reinterpret_cast<const u32*>(pData) == REPO_MAGIC) {
      return 1;
    }
    else if(*reinterpret_cast<const u32*>(pData) == REPO_MAGIC_BIGENDIAN) {
      return -1;
    }
    return 0;
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

  Repository::iterator::iterator( const RepoReader* _repo, KEY* _key )
    : repo(_repo)
    , key(_key)
  {    
  }

  LPCSTR Repository::iterator::name() const
  {
    return key->GetName(repo->_GetNamesBegin());
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

  //////////////////////////////////////////////////////////////////////////
  namespace _internal
  {
    LPCSTR KEY::GetName(LPCSTR szFirstName) const
    {
      return szFirstName + name;
    }

    CLBYTE* KEY::GetDataPtr(CLBYTE* pDataBase, size_t* pSizeOut) const
    {
      if(TEST_FLAG(type, KeyType_Octet)) {
        *pSizeOut = (size_t)(type - KeyType_Octet_0);
        return (CLBYTE*)o.data;
      }
      else if(type == KeyType_Varible) {
        *pSizeOut = v.length;
        return pDataBase + v.offset;
      }
      CLBREAK;
      return NULL;
    }

    CLBYTE* KEY::GetDataPtr(CLBYTE* pDataBase) const
    {
      if(TEST_FLAG(type, KeyType_Octet)) {
        return (CLBYTE*)o.data;
      }
      else if(type == KeyType_Varible) {
        return pDataBase + v.offset;
      }
      CLBREAK;
      return NULL;
    }

    size_t KEY::GetDataSize() const
    {
      if(TEST_FLAG(type, KeyType_Octet)) {
        return (size_t)(type - KeyType_Octet_0);
      }
      else if(type == KeyType_Varible) {
        return v.length;
      }
      CLBREAK;
      return 0;
    }
  } // namespace _internal
  //////////////////////////////////////////////////////////////////////////

} // namespace clstd
