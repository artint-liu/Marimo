#ifndef _CLSTD_SMART_STOCK_H_
#define _CLSTD_SMART_STOCK_H_

// ������Ƶ��½ӿ� SmartStock���������̣������������û������ļ������ݸ�ʽ
// ���Ŀ����Ϊ���滻�ɵ�SmartProfile��
// ����SmartProfile��Ҫԭ���ǣ� SmartProfile�кܶ�ӿڶ��岻�������е����
// �ṹ������Ȼ�����𲽽�����ʽ���õ��ĲŽ���

#ifndef _SMARTSTREAM_2_H_
#error Must be include "smartstream.h" first.
#endif // _SMARTSTREAM_2_H_

class clBuffer;
struct SmartStock_TraitsW
{
  static clsize _StrLen(const wch*);
  static b32    _CheckBoolean(const wch*);
  typedef wch _TCh;
  typedef SmartStream_TraitsW SmartStream_Traits;
};

struct SmartStock_TraitsA
{
  static clsize _StrLen(const ch*);
  static b32    _CheckBoolean(const ch*);
  typedef ch _TCh;
  typedef SmartStream_TraitsA SmartStream_Traits;
};

//////////////////////////////////////////////////////////////////////////

template<
  class    _TStr, 
  class    _Traits>
class SmartStockT
{
public:
  typedef typename _Traits::_TCh _TCh;
  typedef typename SmartStreamT<_TStr, typename _Traits::SmartStream_Traits> _SmartStreamT;
  typedef typename _SmartStreamT::iterator _MyIterator;
  typedef typename _TStr::LPCSTR T_LPCSTR;
  typedef typename _TStr::TChar  TChar;

protected:
  typedef clvector<void*>  HandleArray;

  _SmartStreamT m_SmartStream;
  clBuffer*     m_pBuffer;
  HandleArray   m_aHandles;

  static void ReverseByteOrder16(u16* ptr, clsize nCount);

public:
  struct SECTION_DESC;
  struct PARAMETER
  {
    const SECTION_DESC* pSection;
    _MyIterator   itKey;   // ��
    _MyIterator   itValue; // ֵ

    PARAMETER(){}
    PARAMETER(SECTION_DESC* pCoSection) : pSection(pCoSection){}

    b32     NextKey     ();
    _TStr   SectionName () const;
    _TStr   KeyName     () const;
    int     ToInt       () const;
    _TStr   ToString    () const;
    float   ToFloat     () const;
    b32     ToBoolean   () const;
    _TStr&  KeyName     (_TStr& str) const;
    _TStr&  ToString    (_TStr& str) const;
  };

  //////////////////////////////////////////////////////////////////////////

  struct SECTION_DESC
  {
    SmartStockT*  pStock;
    int           nDepth;   // ������ȣ������ı�����. ����0, �����<0��˵�����Section�Ѿ�ʧЧ
    _MyIterator   itSectionName;
    _MyIterator   itBegin;  // Section��ʼ��'{'λ��
    _MyIterator   itEnd;    // Section������'}'λ��

#ifdef _DEBUG
    b32 DbgCheck() const
    {
      return ( ! IsValid() ) || ((itBegin.marker[0] == '{' || itBegin == itBegin.pContainer->begin()) &&
        (itEnd.marker[0] == '}' || itEnd == itBegin.pContainer->end()));
    }
#else
    b32 DbgCheck() const { return TRUE; }
#endif
    SECTION_DESC(){}
    SECTION_DESC(SmartStockT* pCoStock)
      : pStock  (pCoStock)
      , itBegin (pCoStock->m_SmartStream.begin())
      , itEnd   (pCoStock->m_SmartStream.end())
      , nDepth  (0)
    {}

    b32     IsValid             () const;
    _TStr   SectionName         () const;
    b32     NextSection         (T_LPCSTR szName);
    b32     Rename              (T_LPCSTR szNewName);
    b32     FirstKey            (PARAMETER& param) const;

    b32     GetKey              (T_LPCSTR szKey, PARAMETER& param) const;
    int     GetKeyAsString      (T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount) const;
    _TStr   GetKeyAsString      (T_LPCSTR szKey, const _TStr& strDefault) const;
    int     GetKeyAsInteger     (T_LPCSTR szKey, int nDefault) const;
    float   GetKeyAsFloat       (T_LPCSTR szKey, float fDefault) const;
    b32     GetKeyAsBoolean     (T_LPCSTR szKey, b32 bDefault) const;
    b32     SetKey              (T_LPCSTR szKey, T_LPCSTR szValue);
    b32     DeleteKey           (T_LPCSTR szKey);
  };


  typedef SECTION_DESC* Section;

public:
  SmartStockT();
  virtual ~SmartStockT();

  b32 Close();
  
  b32 LoadA(const ch* lpProfile);
  b32 SaveA(const ch* lpProfile) const;
  b32 LoadW(const wch* lpProfile);
  b32 SaveW(const wch* lpProfile) const;

//#ifdef _UNICODE
//  b32 Load(const wch* lpProfile) { return LoadW(lpProfile); }
//  b32 Save(const wch* lpProfile) const { return SaveW(lpProfile); }
//#else
//  b32 Load(const ch* lpProfile) { return LoadA(lpProfile); }
//  b32 Save(const ch* lpProfile) const { return SaveA(lpProfile); }
//#endif // #ifdef _UNICODE

  //////////////////////////////////////////////////////////////////////////
  // new

  b32 CloseSection(Section sect);
  
  //************************************
  // Method:    Create ����section
  // Qualifier:
  // Parameter: _TCh * szPath
  // szPath֧��"sect3/sect2/sect1/sect0"��"sect"���ָ�ʽ
  // �������·����sect3��sect2��sect1·����ִ�в��ҹ��ܣ�����Ҳ�����ᴴ��
  // ����sect0��ֱ�Ӵ���
  // �������·������ֱ���ڸ��ϴ���sect
  // ע�⣺����ʹ��ʱ��Ҫ��CloseSection�ر�
  //************************************
  Section Create(T_LPCSTR szPath);
  Section CreateChild(Section sect, T_LPCSTR szPath);

  //************************************
  // Method:    Open ��ָ����Section
  // Qualifier:
  // Parameter: _TCh * szPath
  // ��Section��·��������"sect1/sect0"����"sect"
  // ע�⣺����ʹ��ʱ��Ҫ��CloseSection�ر�
  //************************************
  Section Open(T_LPCSTR szPath);
  Section OpenChild(Section sect, T_LPCSTR szPath);

  //************************************
  // Method:    DeleteSection ɾ��ָ����Section
  // Returns:   b32 ɾ���ɹ�����true�����ָ����Section�������򷵻�false
  // Qualifier:
  // Parameter: _TCh * szPath
  // Section��·�����������"sect"������ʽ����ֱ��ɾ����������������Section
  // �����"sect3/sect2/sect1/sect0"����ɾ������"sect3/sect2/sect1"·���µ�"sect0"
  //************************************
  b32 DeleteSection(T_LPCSTR szPath);

  inline T_LPCSTR GetText(clsize* length)
  {
    if(length) {
      *length = m_pBuffer->GetSize() / sizeof(_TCh);
    }
    return (T_LPCSTR)m_pBuffer->GetPtr();
  }

protected:
  b32     Append            (T_LPCSTR szText, clsize nCount);
  b32     Insert            (clsize nPos, T_LPCSTR szText, clsize nCount);
  b32     Replace           (clsize nPos, clsize nReplaced, T_LPCSTR szText, clsize nCount);
  b32     FindSigleSection  (const SECTION_DESC* pFindSect, T_LPCSTR szName, SECTION_DESC* pOutSect); // szNameΪNULL��ʾ�����κ�Section;
  b32     NewSection        (const SECTION_DESC* pSection, T_LPCSTR szName, SECTION_DESC* pNewSect);
  clsize  InsertString      (const _MyIterator& it, const _TStr& str);

  b32     Remove            (const _MyIterator& itBegin, const _MyIterator& itEnd);
  Section AddSection        (Section sect);
  b32     DelSection        (Section sect);
  b32     RelocateIterator  (_MyIterator& it, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
  b32     RelocateSection   (T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
  void    TrimFrontTab      (clsize& uOffset);
};

//template<class _STR>
//_STR FromProfileString(const _STR& str);
//
//template<class _STR>
//_STR ToProfileString(const _STR& str);

class SmartStockA : public SmartStockT<clStringA, SmartStock_TraitsA> {};
class SmartStockW : public SmartStockT<clStringW, SmartStock_TraitsW> {};

//#ifdef _UNICODE
//typedef SmartStockW SmartStock;
//#else
//typedef SmartStockA SmartStock;
//#endif



#endif // _CLSTD_SMART_STOCK_H_