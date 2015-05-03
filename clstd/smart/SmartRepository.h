// ����/���ݵ�"�ڴ�-�ļ�"���ݿ�
// ������С��ģ�����ݴ���Ͳ�ѯ
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
}

class SmartRepository
{
  typedef clstd::File clFile;
  typedef clstd::FixedBuffer clFixedBuffer;
public:
  enum KeyType
  {
    KT_Node,    // �ڵ�
    KT_Varible, // �䳤����
    KT_Octet,   // 8�ֽ�����
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
    // ����ǿ��ܲ���֧��hash_map�ļ�ֵ��¼
    // ʹ��map�ĺ��������ֵ�ַ���������ģ�δ����ֻ��ģʽ�п����ö��ַ�����
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
        // 64λ�´�û��8�ֽڶ��룬�������ܻ�������
        // ��Ϊ���ֻ������ʱ���ݣ����ǰ����������
        // ����һ��KEYDESC_RT��������ļ���/д��KEYDESC����
        SmartRepository* pSmart;
      }buf;
    };
  };

  typedef KEYDESC::KeyDict KeyDict;
  typedef SmartRepository SmartNode;

private:
  clBuffer* pBuffer;
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
  SmartNode*  GetNode         (CLLPCSTR szKeys) CLCONST;
  u32         SetFlags        (u32 dwFlags);  // �鿴 SRF_ ��־

  b32     Write       (SmartNode* pSmart, CLLPCSTR szKey, CLLPCVOID lpData, u32 cbSize);
  b32     Write64     (SmartNode* pSmart, CLLPCSTR szKey, u32 dwLow, u32 dwHigh);
  s32     GetLength   (CLCONST SmartNode* pSmart, CLLPCSTR szKey) CLCONST;
  s32     Read        (CLCONST SmartNode* pSmart, CLLPCSTR szKey, CLLPVOID lpData, u32 cbSize) CLCONST; // ��������cbSize����ᱻ�ض�
  b32     Read64      (CLCONST SmartNode* pSmart, CLLPCSTR szKey, u32* dwLow, u32* dwHigh) CLCONST;
  s32     ReadToBuffer(CLCONST SmartNode* pSmart, CLLPCSTR szKey, clBuffer* pBuffer) CLCONST;

  // �ַ�����ȡ���������Key������, ����ô�������ܳ���
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
  b32 WriteStructArrayT(SmartNode* pSmart, CLLPCSTR szKey, const _T& stru, u32 nCount)  // Count ������Ԫ�ظ���, �����ֽ���
  {
    return Write(pSmart, szKey, &stru, sizeof(_T) * nCount);
  }

  template<typename _T>
  s32 ReadStructArrayT(SmartNode* pSmart, CLLPCSTR szKey, _T& stru, u32 nCount)  // Count ������Ԫ�ظ���, �����ֽ���
  {
    return Read(pSmart, szKey, &stru, sizeof(_T) * nCount);
  }
};
#pragma pack(pop)
#endif // #ifndef _CLSTD_SMART_REPOSITORY_H_