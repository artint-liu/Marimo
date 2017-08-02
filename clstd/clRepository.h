#ifndef _CLSTD_REPOSITORY_H_

namespace clstd
{
  namespace _internal
  {
    struct KEY;
    struct KEYPAIR;
  } // namespace

  class RepoReader
  {
  public:
    typedef ch CHAR;
    typedef const CHAR* LPCSTR;
    typedef _internal::KEY KEY;
    typedef _internal::KEYPAIR KEYPAIR;

  protected:
    KEY*      m_pKeys;
    KEY*      m_pKeysEnd;
    KEY*      m_pKeysCapacity; // 也是m_pNamesBegin
    LPCSTR    m_pNamesEnd;
    CLBYTE*   m_pData;         // 也是m_pNamesCapacity
    size_t    m_cbDataLen;

  public:
    class iterator
    {
      const RepoReader* repo;
      KEY* key;

    public:
      iterator( const RepoReader* _repo, KEY* _key );

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

    // 获得各个段的有效数据
    struct RAWDATA
    {
      CLBYTE  header[20];
      size_t  cbHeader;    // header的有效长度，最大20字节
      KEY*    keys;
      size_t  cbKeys;
      LPCSTR  names;
      size_t  cbNames;
      CLBYTE* data;
      size_t  cbData;
    };

  protected:
    KEY*      _FindKey          (LPCSTR szKey) const;
    LPCSTR    _GetNamesBegin    () const;
    LPCSTR    _GetNamesCapacity () const;

  public:
    RepoReader();
    RepoReader(const void* pData, size_t nLength); // 附加到内存数据

    b32     Attach        (const void* pData, size_t nLength); // 附加到内存数据
    size_t  GetNumOfKeys  () const;
    size_t  GetKey        (LPCSTR szKey, void* pData, size_t nLength) const;
    void*   GetDataPtr    (LPCSTR szKey, size_t* pLength = NULL) const;
    size_t  GetRawData    (RAWDATA* pRaw) const;

    iterator begin  () const;
    iterator end    () const;
  };

  //////////////////////////////////////////////////////////////////////////

  class Repository : public RepoReader
  {
  protected:
    MemBuffer m_Buffer;

    template<typename T_LPCSTR>
    b32 LoadFromFileT(T_LPCSTR szFilename); // load 表示一次性从文件加载，read表示一点点从文件加载
    b32 _WriteToFile(File& file) const;

    b32       _DbgCheckDataOverlay() const;
    b32       _ParseFromBuffer  ();
    void      _ResizeKey        (KEY*& rKey, size_t delta);
    b32       _ReallocData      (KEY*& rKey, size_t new_length);
    KEYPAIR*  _PreInsertKey     (KEYPAIR* pPair, LPCSTR szKey);
    void      _Locate           (size_t memdelta);
    b32       _AllocKey         (size_t cbKeyNameLen/*, const void* pData*/, size_t nLength, KEY*& rPos);
    b32       _InsertKey        ( KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength );
    b32       _ReplaceKey       (KEY* pPos, LPCSTR szKey, const void* pData, size_t nLength);
    void      _Initialize       (size_t cbDataLen);
    //KEY*      _FindKey          (LPCSTR szKey) const;
    size_t    _ResizeGlobalBuf  (size_t delta, KEY*& rpKey);
    u32       _GetSeqOffset     (KEY* pPos);
    //LPCSTR    _GetNamesBegin    () const;
    //LPCSTR    _GetNamesCapacity () const;
    size_t    _GetDataCapacity  () const;

  public:
      Repository();
      ~Repository();

      Repository& operator=(const Repository& repo);
    
      void    Clear         ();

      b32     LoadFromFile  (CLLPCSTR szFilename );
      b32     LoadFromFile  (CLLPCWSTR szFilename );
      b32     LoadFromMemory(const void* pData, size_t nLength);
      b32     LoadFromMemory(const BufferBase& buf);
      b32     SaveToFile    (CLLPCSTR szFilename) const;
      b32     SaveToFile    (CLLPCWSTR szFilename) const;
      b32     SaveToMemory  (MemBuffer& buffer) const;

      b32     SetKey        (LPCSTR szKey, const void* pData, size_t nLength);
      b32     SetKey        (LPCSTR szKey, const wch* str);
      b32     SetKey        (LPCSTR szKey, const ch* str);
#ifdef _CLSTD_STRING_H_
      b32     SetKey        (LPCSTR szKey, const clStringW& str);
      b32     SetKey        (LPCSTR szKey, const clStringA& str);
#endif // _CLSTD_STRING_H_
      b32     SetKey        (LPCSTR szKey, i32 value);
      b32     SetKey        (LPCSTR szKey, u32 value);
      b32     SetKey        (LPCSTR szKey, i64 value);
      b32     SetKey        (LPCSTR szKey, u64 value);

      //size_t  GetKey        (LPCSTR szKey, void* pData, size_t nLength) const;
      //void*   GetDataPtr    (LPCSTR szKey, size_t* pLength = NULL) const;
      //size_t  GetRawData    (RAWDATA* pRaw) const;

      // 传入一定量数据用来计算整个文件的大小，数据长度至少要20字节，前四个字节必须是"CLRP"
      static size_t GetRequiredSize(void* pData, size_t nLength);
      static i32    IsRepository(const void* pData, size_t nLength); // nLength 只是校验，小于4时返回false，little-endian的头返回1，big-endian返回-1
      

      //b32 RemoveKey( LPCSTR szKey );
      //b32 RemoveKey( CLDWORD dwKey );
      //size_t GetKey( CLDWORD szKey, void* pData, size_t nLength ) const;
      //b32 SetKey( CLDWORD szKey, const void* pData, size_t nLength );


  };
} // namespace clstd

#endif // _CLSTD_REPOSITORY_H_