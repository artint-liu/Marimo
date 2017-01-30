#ifndef _CLSTD_SMART_REPOSITORY_H_

namespace clstd
{
  class Repository
  {
  public:
    typedef ch TChar;
    typedef const TChar* LPCSTR;

  protected:
      template<typename T_LPCSTR>
      b32 LoadFromFileT(T_LPCSTR szFilename);

      struct FILE_HEADER
      {
        CLDWORD dwMagic; // CLRP
        CLDWORD nKeys;
        CLDWORD cbNames;
        CLDWORD cbData;
      };

      enum KeyType
      {
        //KT_Node,    // 节点
        KT_Varible,   // 变长数据
        //KT_Octet_0,   // 8字节数据，长度可以为0，仅存键值
        //KT_Octet_8,   // 8字节数据，长度8字节
      };

      struct KEY 
      {
        u32     name; // 字母偏移，不是字节
        KeyType type;
        u32     offset;
        u32     length;

        LPCSTR GetName(LPCSTR szFirstName) const
        {
          return szFirstName + name;
        }
      };

      struct KEYPAIR
      {
        KEY* pTable;
        b32 bInsert;
      };

  protected:
    MemBuffer m_Buffer;
    KEY* m_pKeys;
    KEY* m_pKeysEnd;
    KEY* m_pKeysCapacity; // 也是m_pNamesBegin
    LPCSTR m_pNamesEnd;
    CLBYTE* m_pData;      // 也是m_pNamesCapacity
    size_t m_cbDataLen;

    b32 ParseFromBuffer();
    KEYPAIR*  _PreInsertKey (KEYPAIR* pPair, LPCSTR szKey);
    void      _Locate       (size_t memdelta);
    b32       _AllocKey     (size_t cbKeyNameLen, const void* pData, size_t nLength, KEY*& rPos);
    b32       _InsertKey    ( KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength );
    b32       _ReplaceKey   (KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength);
    void      _Initialize   (size_t cbDataLen);
    KEY*      FindKey       (LPCSTR szKey) const;      

    LPCSTR GetNamesBegin() const
    {
      return reinterpret_cast<LPCSTR>(m_pKeysCapacity);
    }

    LPCSTR GetNamesCapacity() const
    {
      return reinterpret_cast<LPCSTR>(m_pData);
    }

    size_t GetDataCapacity() const
    {
      return ((size_t)m_Buffer.GetPtr() + m_Buffer.GetSize()) - (size_t)m_pData;
    }

  public:
      class iterator
      {
        const Repository* repo;
        KEY* key;

      public:
        iterator( const Repository* _repo, KEY* _key );

        LPCSTR  name  () const;
        void*   ptr   () const;
        size_t  size  () const;
        iterator& operator++();
        iterator& operator++(int);
        b32 operator==(const iterator& it);
        b32 operator!=(const iterator& it);
        b32 operator==(LPCSTR szKey);
        b32 operator!=(LPCSTR szKey);
      };

  public:
      Repository();
      ~Repository();

      iterator begin  () const;
      iterator end    () const;
      

      b32 LoadFromFile( CLLPCSTR szFilename );
      b32 LoadFromFile( CLLPCWSTR szFilename );
      b32 LoadFromMemory( const void* pData, size_t nLength );
      b32 SaveToFile( CLLPCSTR szFilename ) const;
      b32 SaveToFile( CLLPCWSTR szFilename ) const;
      b32 SaveToMemory( const void* pData, size_t nLength ) const;

      size_t GetNumOfKeys() const;
      b32 SetKey( LPCSTR szKey, const void* pData, size_t nLength );
      size_t GetKey( LPCSTR szKey, void* pData, size_t nLength ) const;
      void* GetDataPtr( LPCSTR szKey, size_t* pLength ) const;
      

      //b32 RemoveKey( LPCSTR szKey );
      //b32 RemoveKey( CLDWORD dwKey );
      //size_t GetKey( CLDWORD szKey, void* pData, size_t nLength ) const;
      //b32 SetKey( CLDWORD szKey, const void* pData, size_t nLength );


  };
} // namespace clstd

#endif