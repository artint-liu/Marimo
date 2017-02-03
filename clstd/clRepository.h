#ifndef _CLSTD_SMART_REPOSITORY_H_

namespace clstd
{
  class Repository
  {
  public:
    typedef ch TChar;
    typedef const TChar* LPCSTR;

  protected:
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

  protected:
    MemBuffer m_Buffer;
    KEY* m_pKeys;
    KEY* m_pKeysEnd;
    KEY* m_pKeysCapacity; // 也是m_pNamesBegin
    LPCSTR m_pNamesEnd;
    CLBYTE* m_pData;      // 也是m_pNamesCapacity
    size_t m_cbDataLen;

    template<typename T_LPCSTR>
    b32 LoadFromFileT(T_LPCSTR szFilename); // load 表示一次性从文件加载，read表示一点点从文件加载
    b32 _WriteToFile(File& file) const;

    b32       _DbgCheckDataOverlay() const;
    b32       _ParseFromBuffer();
    void      _ResizeKey    (KEY*& rKey, size_t delta);
    b32       _ReallocData  (KEY*& rKey, size_t new_length);
    KEYPAIR*  _PreInsertKey (KEYPAIR* pPair, LPCSTR szKey);
    void      _Locate       (size_t memdelta);
    b32       _AllocKey     (size_t cbKeyNameLen/*, const void* pData*/, size_t nLength, KEY*& rPos);
    b32       _InsertKey    ( KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength );
    b32       _ReplaceKey   (KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength);
    void      _Initialize   (size_t cbDataLen);
    KEY*      _FindKey       (LPCSTR szKey) const;      
    size_t    _ResizeGlobalBuf       (size_t delta, KEY*& rpKey);
    u32       _GetSeqOffset  (KEY* pPos);

    LPCSTR GetNamesBegin() const;
    LPCSTR GetNamesCapacity() const;
    size_t GetDataCapacity() const;

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
      //b32 SaveToMemory( const void* pData, size_t nLength ) const;

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