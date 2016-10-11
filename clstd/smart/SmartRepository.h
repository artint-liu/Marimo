// 灵巧/敏捷的"内存-文件"数据库
// 适用于小规模的数据储存和查询
#ifndef _CLSTD_SMART_REPOSITORY_H_
#define _CLSTD_SMART_REPOSITORY_H_

//#define ENABLE_HASH_NAME

//#define CLERR_OK        0
//#define CLERR_NOTFOUND  -1

#define SRF_UNCOMPRESSED    0x00000000
#define SRF_COMPRESSED      0x00000001
#define SRF_APPLYCHILDNODE  0x00000010

namespace clstd
{
  class File;
  class FixedBuffer;

  class SmartRepository
  {
    typedef clstd::File clFile;
    typedef clstd::FixedBuffer clFixedBuffer;
  public:
    enum KeyType
    {
      KT_Node,    // 节点
      KT_Varible, // 变长数据
      KT_Octet,   // 8字节数据
    };

    struct HEADER
    {
      u32 dwMagic;
      u32 dwNumKeys;
    };
#pragma pack(push)
#pragma pack(4)  
    struct KEYDESC
    {
#ifdef ENABLE_HASH_NAME
      typedef clhash_map<clStringA, struct KEYDESC> KeyDict;
#else
      // 今后考虑可能不再支持hash_map的键值记录
      // 使用map的红黑树，键值字符串是有序的，未来在只读模式中可以用二分法查找
      typedef clmap<clStringA, struct KEYDESC> KeyDict;
#endif // #ifdef ENABLE_HASH_NAME

      u32       nKeyLength : 8;
      u32       eType : 8;
      u32       Flags : 16;
      union 
      {
        struct {
          u32 nOffset;
          u32 cbSize;
        }v; // varible
        struct {
          u32 dwLow;
          u32 dwHigh;
        }o; // octet
        struct {
          // 64位下存没有8字节对齐，可能性能会有问题
          // 因为这个只是运行时数据，考虑把它分离出来
          // 增加一个KEYDESC_RT来区别从文件读/写的KEYDESC数据
          SmartRepository* pSmart;
        }buf;
      };
    };

    typedef KEYDESC::KeyDict KeyDict;
    typedef SmartRepository SmartNode;

  private:
    Buffer* pBuffer;
    KeyDict   Keys;
    u32       m_bCompressed : 1;
  private:
    b32 ClearRecursive(SmartNode* pSmart);
    b32 LoadRecursive (SmartNode* pSmart, CLLPCSTR szGroupName, clFile& file);
    b32 SaveRecursive (SmartNode* pSmart, CLLPCSTR szGroupName, clFile& file);
  public:
    SmartRepository();
    virtual ~SmartRepository();
    b32 LoadA         (CLLPCSTR szFile);
    b32 SaveA         (CLLPCSTR szFile);
    b32 LoadW         (CLLPCWSTR szFile);
    b32 SaveW         (CLLPCWSTR szFile);
    b32 LoadSubNodeA  (CLLPCSTR szFile, CLLPCSTR szKeys);
    b32 LoadSubNodeW  (CLLPCWSTR szFile, CLLPCSTR szKeys);

    SmartNode*  CreateNode      (CLLPCSTR szKeys);
    SmartNode*  GetNode         (CLLPCSTR szKeys) const;
    u32         SetFlags        (u32 dwFlags);  // 查看 SRF_ 标志

    b32     Write       (SmartNode* pSmart, CLLPCSTR szKey, CLLPCVOID lpData, u32 cbSize);
    b32     Write64     (SmartNode* pSmart, CLLPCSTR szKey, u32 dwLow, u32 dwHigh);
    s32     GetLength   (const SmartNode* pSmart, CLLPCSTR szKey) const;
    s32     Read        (const SmartNode* pSmart, CLLPCSTR szKey, CLLPVOID lpData, u32 cbSize) const; // 如果传入的cbSize过大会被截断
    b32     Read64      (const SmartNode* pSmart, CLLPCSTR szKey, u32* dwLow, u32* dwHigh) const;
    s32     ReadToBuffer(const SmartNode* pSmart, CLLPCSTR szKey, Buffer* pBuffer) const;

    // 字符串存取函数不检查Key的属性, 如果用错了则可能出错
    b32     WriteStringW(SmartNode* pSmart, CLLPCSTR szKey, CLLPCWSTR szString);
    b32     WriteStringA(SmartNode* pSmart, CLLPCSTR szKey, CLLPCSTR  szString);
    b32     WriteStringW(SmartNode* pSmart, CLLPCSTR szKey, const clStringW& strString);
    b32     WriteStringA(SmartNode* pSmart, CLLPCSTR szKey, const clStringA& strString);
    s32     ReadStringW (SmartNode* pSmart, CLLPCSTR szKey, clStringW& strString);
    s32     ReadStringA (SmartNode* pSmart, CLLPCSTR szKey, clStringA& strString);

    b32     WriteNumeric(SmartNode* pSmart, CLLPCSTR szKey, s32 nNum);
    s32     ReadNumeric (SmartNode* pSmart, CLLPCSTR szKey);

    template<typename _T>
    b32 WriteStructT(SmartNode* pSmart, CLLPCSTR szKey, const _T& stru)
    {
      return Write(pSmart, szKey, &stru, sizeof(_T));
    }

    template<typename _T>
    s32 ReadStructT(SmartNode* pSmart, CLLPCSTR szKey, _T& stru)
    {
      return Read(pSmart, szKey, &stru, sizeof(_T));
    }

    template<typename _T>
    b32 WriteStructArrayT(SmartNode* pSmart, CLLPCSTR szKey, const _T& stru, u32 nCount)  // Count 是数组元素个数, 不是字节数
    {
      return Write(pSmart, szKey, &stru, sizeof(_T) * nCount);
    }

    template<typename _T>
    s32 ReadStructArrayT(SmartNode* pSmart, CLLPCSTR szKey, _T& stru, u32 nCount)  // Count 是数组元素个数, 不是字节数
    {
      return Read(pSmart, szKey, &stru, sizeof(_T) * nCount);
    }
  };
} // namespace clstd
#pragma pack(pop)
#endif // #ifndef _CLSTD_SMART_REPOSITORY_H_