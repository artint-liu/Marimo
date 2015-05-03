#ifndef _MARIMO_DATA_POOL_VARIABLE_
#define _MARIMO_DATA_POOL_VARIABLE_

#ifndef _MARIMO_DATA_POOL_H_
#error ������֮ǰ����"DataPool.h"�ļ�
#endif // #ifndef _MARIMO_DATA_POOL_H_

namespace Marimo
{
  class GXDLL DataPoolVariable
  {
    //typedef const DataPool::VARIABLE_DESC   DPVD;
    typedef const DataPool::VARIABLE_DESC*  LPCVD;
  public:
    struct VTBL;
    typedef DataPoolUtility::iterator         iterator;
    typedef DataPoolUtility::element_iterator element_iterator;

    enum CAPS
    {
      CAPS_FIXED    = 0x00000001, // �ڴ��ַ����, �ȶ�������GetPtr()���صĵ�ַ�ǹ̶��ģ����ڶ�̬����ı����������������Ӷ��ı�
      CAPS_STRUCT   = 0x00000002, // �ṹ��
      CAPS_DYNARRAY = 0x00000004, // ��̬����
    };
  protected:
    VTBL*         m_vtbl;
    DataPool*     m_pDataPool;
    LPCVD         m_pVdd;
    clBufferBase* m_pBuffer;
    GXUINT        m_AbsOffset; // ����� m_pBuffer ָ���ƫ��

  public:
    DataPoolVariable(VTBL* vtbl, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nOffset);
    DataPoolVariable(VTBL* vtbl, DataPool* pDataPool, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nOffset);
    DataPoolVariable(const DataPoolVariable& val);
    DataPoolVariable();
    ~DataPoolVariable(); // ��������Ϊ�麯��!

  public:
    inline DataPoolVariable operator[](int nIndex)
    {
      return IndexOf(nIndex);
    }

    inline DataPoolVariable operator[](GXLPCSTR szMemberName)
    {
      return MemberOf(szMemberName);
    }

    template<class _Ty>
    inline _Ty& CastTo() // Reinterpret cast
    {
      return *(_Ty*)GetPtr();
    }

  public:
    DataPoolVariable& operator=   (const DataPoolVariable& val);

    // �֣������ز�ͬ���ͣ����ʹ��ģ����ܻ��������������������
    DataPoolVariable& operator=   (GXLPCWSTR szString);
    DataPoolVariable& operator=   (GXLPCSTR szString);
    DataPoolVariable& operator=   (float val);
    DataPoolVariable& operator=   (i32 val);            // �����������32λ�ᱻ�ض�
    DataPoolVariable& operator=   (i64 val);
    DataPoolVariable& operator=   (u32 val);            // �����������32λ�ᱻ�ض�
    DataPoolVariable& operator=   (u64 val);


#ifdef ENABLE_DATAPOOL_WATCHER
    GXHRESULT         Impulse         (DataAction eAct, GXUINT nIndex = 0);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    GXHRESULT         GetPool         (DataPool** ppDataPool) GXCONST;
    DataPool*         GetPoolUnsafe   () GXCONST;
    GXBOOL            IsSamePool      (DataPool* pDataPool) GXCONST;
    GXVOID            Free            ();
    GXBOOL            IsValid         () GXCONST;  // ������� Variable �Ƿ���Ч
    GXLPVOID          GetPtr          () GXCONST;  // ָ��, Ҫע�⶯̬����ָ�����Ϊ NewBack �����ı�
    GXUINT            GetSize         () GXCONST;  // �ֽڴ�С, �����������С, ��̬���ݴ�С�ɱ�, �ṹ�ǽṹ���С
    GXUINT            GetOffset       () GXCONST;  // ƫ��,ȫ�ֱ�����ȫ��ƫ��, �ṹ������ǽṹ����ƫ��
    GXLPCSTR          GetName         () GXCONST;  // ��ö�����, ������, ������������߽ṹ�������
    GXLPCSTR          GetTypeName     () GXCONST;  // ����, ����Ϊ������, ����Ϊ"Type[n]"��ʽ, ��̬����Ϊ"Type[]"��ʽ, �ṹ��Ϊ"struct Name"��ʽ
    TypeCategory      GetTypeCategory () GXCONST;  // �����ķ���
    GXDWORD           GetCaps         () GXCONST;  // �ο�CAPSö��
    clStringA         GetFullName     () GXCONST;  // ��ûʵ�֣���ñ�����ȫ��,����ڽṹ���У������ṹ����������������л������������,��ͬDataPool::QueryByExpression()��������.
    clBufferBase*     GetBuffer       () GXCONST;

    // �ṹ��ר��
    DataPoolVariable  MemberOf        (GXLPCSTR szName) GXCONST;     // ��ó�Ա

    // �����̬����ר�� 
    DataPoolVariable  IndexOf         (int nIndex) GXCONST;         // ����ض������ı���
    GXUINT            GetLength       () GXCONST;                   // �������ĳ�Ա����, ע����GetSize����
    DataPoolVariable  NewBack         (GXUINT nIncrease = 1);       // �ڶ�̬������׷������, ��̬����ר��, ���inc����1�����ص�һ���������������incΪ0�����������������������һ������
    GXBOOL            Remove          (GXUINT nIndex, GXUINT nCount = 1);        // �Ƴ���̬����ָ������������, ��̬����ר��

    // ����ר��
    GXBOOL            ParseW          (GXLPCWSTR szString, GXUINT length); // ���ձ�������תֵ, length=0��ʾ����'\0'��β
    GXBOOL            ParseA          (GXLPCSTR szString, GXUINT length);

    clStringW         ToStringW       () GXCONST;           // ���������京��תֵ, ����ͽṹ���ͬ��GetTypeName()
    clStringA         ToStringA       () GXCONST;

    GXBOOL            Set             (GXLPCWSTR szString);
    GXBOOL            Set             (GXLPCSTR szString);
    GXBOOL            Set             (float val);
    GXBOOL            Set             (i32 val);            // �����������32λ�ᱻ�ض�
    GXBOOL            Set             (i64 val);
    GXBOOL            Set             (u32 val);            // �����������32λ�ᱻ�ض�
    GXBOOL            Set             (u64 val);

    GXBOOL            Retain          (GUnknown* pUnknown); // ��ȫ�Ĵ���GUnknown��������NULLʱ�ͷ�GUnknown����
    GXBOOL            Query           (GUnknown** ppUnknown) GXCONST;

    float             ToFloat         () GXCONST;
    u32               ToInteger       () GXCONST;
    u64               ToInteger64     () GXCONST;

    GXBOOL            SetData         (GXLPCVOID lpData, GXUINT cbSize);
    GXBOOL            GetData         (GXLPVOID lpData, GXUINT cbSize) GXCONST;

    iterator          begin           ();
    iterator          end             ();
    element_iterator  array_begin     ();
    element_iterator  array_end       ();
  };


  //////////////////////////////////////////////////////////////////////////

  namespace Implement
  {
    typedef DataPoolVariable Variable;

    //
    // �������͵�ģ������
    //
    template<typename _Ty>
    class VarPrimaryT : public Variable
    {
    public:
      VarPrimaryT(){}
      VarPrimaryT(const _Ty& val){
        *this = val;
      }
      VarPrimaryT(const DataPoolVariable& val)
      {
        *this = val;
      }

      ~VarPrimaryT()
      {
        this->Variable::~Variable(); 
      }

      operator _Ty() const{
        return *(_Ty*)GetPtr();
      }

      const _Ty& operator=(const _Ty& val)
      {
        *(_Ty*)GetPtr() = val;
#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
        return *(_Ty*)GetPtr();
      }

      VarPrimaryT<_Ty>& operator=(const DataPoolVariable& val)
      {
        *(DataPoolVariable*)this = val;
        return *this;
      }
    };

    //
    // �ṹ����������
    //
    template<class _Ty>
    class VarComplexT : public Variable
    {
    public:
      VarComplexT(){}
      VarComplexT(const _Ty& val){
        *this = val;
      }

      ~VarComplexT()
      {
        this->Variable::~Variable(); 
      }

      operator _Ty&() const{
        return (_Ty&)*(_Ty*)GetPtr();
      }

      _Ty& operator=(_Ty& val)
      {
        *(_Ty*)GetPtr() = val;
#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
        return *(_Ty*)GetPtr();
      }

      const _Ty& operator=(const _Ty& val)
      {
        *(_Ty*)GetPtr() = val;
#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
        return *(_Ty*)GetPtr();
      }
    };
  } // namespace Implement
} // namespace Marimo

typedef Marimo::DataPool                          MODataPool;
typedef Marimo::DataPoolVariable                  MOVariable;
typedef Marimo::Implement::VarPrimaryT<GXINT>     MOVarInt;
typedef Marimo::Implement::VarPrimaryT<GXUINT>    MOVarUInt;
typedef Marimo::Implement::VarPrimaryT<float>     MOVarFloat;
typedef Marimo::Implement::VarComplexT<float2>    MOVarFloat2;
typedef Marimo::Implement::VarComplexT<float3>    MOVarFloat3;
typedef Marimo::Implement::VarComplexT<float4>    MOVarFloat4;
typedef Marimo::Implement::VarComplexT<float4x4>  MOVarMatrix4;

// �޷�ʵ���ַ������ͣ���Ϊ�ַ���������ʹ��ǰ������Ҫ��ȷ��ʼ��
// �����ģ��ġ�=����������û����ʽ��ʼ���ַ�������
//typedef Marimo::Implement::VarPrimaryT<clStringA> MOVarStringA;
//typedef Marimo::Implement::VarPrimaryT<clStringW> MOVarStringW;

#endif // #ifndef _MARIMO_DATA_POOL_VARIABLE_