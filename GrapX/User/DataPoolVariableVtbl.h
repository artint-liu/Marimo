#ifndef _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_
#define _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_

namespace Marimo
{
  class DataPoolVariableImpl;
  // �ڲ�ʵ�ֵĺ�����
  struct DataPoolVariable::VTBL
  {
    typedef DataPoolVariable Variable;
    typedef DataPoolVariableImpl VarImpl;
    

    GXUINT    (*GetSize     )(GXCONST VarImpl* pThis);  // �ֽڴ�С, �����������С, ��̬���ݴ�С�ɱ�, �ṹ���ǽṹ���С
    //GXUINT    (*GetOffset   )(GXCONST VarImpl* pThis);  // ƫ��,ȫ�ֱ�����ȫ��ƫ��, �ṹ������ǽṹ����ƫ��
    //GXLPCSTR  (*GetName     )(GXCONST VarImpl* pThis);  // ��ö�����, ������, ������������߽ṹ�������
    //GXLPCSTR  (*GetTypeName )(GXCONST VarImpl* pThis);  // ����, ����Ϊ������, ����Ϊ"Type[n]"��ʽ, ��̬����Ϊ"Type[]"��ʽ, �ṹ��Ϊ"struct Name"��ʽ

    // �ṹ��ר��
    Variable  (*GetMember   )(GXCONST VarImpl* pThis, GXLPCSTR szName);    // ��ó�Ա

    // �����̬����ר�� 
    Variable  (*GetIndex    )(GXCONST VarImpl* pThis, int nIndex);         // ����ض������ı���
    GXUINT    (*GetLength   )(GXCONST VarImpl* pThis);                     // �������ĳ�Ա����, ע����GetSize����
    Variable  (*NewBack     )(        VarImpl* pThis, GXUINT nIncrease);   // �ڶ�̬������׷������, ��̬����ר��
    GXBOOL    (*Remove      )(        VarImpl* pThis, GXUINT nIndex, GXUINT nCount);      // �Ƴ���̬����ָ������������, ��̬����ר��

    // ����ר��
    //clStringW (*ToStringW   )(GXCONST VarImpl* pThis);                   // ���������京��תֵ, ����ͽṹ���ͬ��GetTypeName()
    //clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*ParseW      )(        VarImpl* pThis, GXLPCWSTR szString, GXUINT length); // ���ձ�������תֵ(unicode)
    GXBOOL    (*ParseA      )(        VarImpl* pThis, GXLPCSTR szString, GXUINT length);  // ���ձ�������תֵ
    u32       (*ToInteger   )(GXCONST VarImpl* pThis);
    u64       (*ToInt64     )(GXCONST VarImpl* pThis);
    float     (*ToFloat     )(GXCONST VarImpl* pThis);
    clStringW (*ToStringW   )(GXCONST VarImpl* pThis);
    clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*SetAsInteger)(        VarImpl* pThis, u32 val);             // �����������32λ�ᱻ�ض�
    GXBOOL    (*SetAsInt64  )(        VarImpl* pThis, u64 val);
    GXBOOL    (*SetAsFloat  )(        VarImpl* pThis, float val);
    GXBOOL    (*SetAsStringW)(        VarImpl* pThis, GXLPCWSTR szString);
    GXBOOL    (*SetAsStringA)(        VarImpl* pThis, GXLPCSTR szString);
    GXBOOL    (*Retain      )(        VarImpl* pThis, GUnknown* pUnknown);
    GXBOOL    (*Query       )(GXCONST VarImpl* pThis, GUnknown** ppUnknown);
    GXBOOL    (*GetData     )(GXCONST VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize);
    GXBOOL    (*SetData     )(        VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize);
  };

  // ������������ͣ���ת������TRUE�����򷵻�FALSE
  // ֧��0xʮ�����ƣ�0b�����ƣ�0��ͷ�İ˽��ƺ�ʮ����
  template<typename _TCh>
  GXBOOL IsNumericT(const _TCh* str, clsize len, GXINT* pInteger)
  {
    if(str[0] == '0') {
      if(str[1] == 'x' || str[1] == 'X')
      {
        if(clstd::IsNumericT(str + 2, 16, len - 2)) {
          *pInteger = GXATOI(str + 2, 16, len - 2);
          return TRUE;
        }
      }
      else if(str[1] == 'b' || str[1] == 'B')
      {
        if(clstd::IsNumericT(str + 2, 2, len - 2)) {
          *pInteger = GXATOI(str + 2, 2, len - 2);
          return TRUE;
        }
      }
      else
      {
        if(clstd::IsNumericT(str + 1, 8, len - 1)) {
          *pInteger = GXATOI(str + 1, 8, len - 1);
          return TRUE;
        }
      }
    }
    else if(clstd::IsNumericT(str, 10, len)) {
      *pInteger = GXATOI(str, 10, len);
      return TRUE;
    }
    return FALSE;
  }



  struct DataPoolBuildTime // Build time ��д�����ڹ���ʱ������ʱ����
  {
    friend class DataPool;
    //typedef clmap<clStringA, TYPE_DESC> TypeDict;

    struct BUILDTIME_TYPE_DECLARATION : public TYPE_DECLARATION
    {
      GXDWORD dwFlags; // BuildtimeTypeDeclarationFlags
    };

    struct BT_TYPE_DESC : public DataPool::TYPE_DESC
    {
      GXUINT nIndex;
    };

    struct BT_VARIABLE_DESC : public DATAPOOL_VARIABLE_DESC
    {
      // ������TypeDesc��Ա��������!
      DataPool::TYPE_DESC* pTypeDesc;

      inline const DataPool::TYPE_DESC* GetTypeDesc() const
      {
        return pTypeDesc;
      }

      inline GXUINT TypeSize() const
      {
        //return ((TYPE_DESC*)((GXLONG_PTR)pTypeDesc & (~3)))->cbSize;
        return GetTypeDesc()->cbSize;
      }

      inline GXBOOL IsDynamicArray() const
      {
        //return pTypeDesc->Name[0] == '#';
        return bDynamic;
      }

      GXUINT GetSize() const
      {
        ASSERT( ! IsDynamicArray()); // ��Ӧ���Ƕ�̬����
        return nCount * TypeSize();
      }
    };

    typedef clhash_map<clStringA, BUILDTIME_TYPE_DECLARATION> BuildTimeTypeDeclarationDict;
    typedef clmap<clStringA, BT_TYPE_DESC>    BTTypeDict;
    typedef clvector<BT_VARIABLE_DESC>        BTVarDescArray;
    typedef clvector<DataPool::ENUM_DESC>     EnumDescArray;

    // ����ʱ����õ����������ṹ
    enum BuildtimeTypeDeclarationFlags{
      BuildtimeTypeDeclaration_Used    = 0x0001,  // ʹ�ù�
      BuildtimeTypeDeclaration_Checked = 0x0002,  // ����
    };


  public:
    GXBOOL  IntCheckTypeDecl  (LPCTYPEDECL pTypeDecl, GXBOOL bCheck);
    GXBOOL  CheckVarList      (LPCVARDECL pVarDecl);

    void    PutTypeToDict     (GXLPCSTR szTypeName);
    GXINT   CalculateVarSize  (LPCVARDECL pVarDecl, BTVarDescArray& aVariableDesc);

  protected:
    BuildTimeTypeDeclarationDict sBuildTimeTypeDeclDict;
    BTTypeDict          m_TypeDict;         // ����
    BTVarDescArray      m_aVar;             // ȫ�ֱ���
    BTVarDescArray      m_aStructMember;    // ���еĽṹ���Ա�������������ű���
    EnumDescArray       m_aEnumPck;         // ����ö�ٳ�Ա�����������
    clstd::StringSetA   NameSet;
    GXUINT              m_bPtr64 : 1;       // 64λָ�����ģʽ
    GXUINT              m_bFixedPool : 1;

    DataPoolBuildTime() : m_bPtr64(0), m_bFixedPool(1) {}
  };



} // namespace Marimo

#endif // _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_