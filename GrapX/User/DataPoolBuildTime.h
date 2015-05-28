#ifndef _DATAPOOL_BUILD_TIME_H_
#define _DATAPOOL_BUILD_TIME_H_

namespace Marimo
{
  class DataPoolVariableImpl;

  struct DataPoolBuildTime // Build time ��д�����ڹ���ʱ������ʱ����
  {
    friend class DataPoolImpl;
    //typedef clmap<clStringA, TYPE_DESC> TypeDict;

    struct BUILDTIME_TYPE_DECLARATION : public TYPE_DECLARATION
    {
      GXDWORD dwFlags; // BuildtimeTypeDeclarationFlags
    };

    struct BT_TYPE_DESC : public DataPoolImpl::TYPE_DESC
    {
      GXUINT nIndex;
    };

    struct BT_VARIABLE_DESC : public DATAPOOL_VARIABLE_DESC
    {
      // ������TypeDesc��Ա��������!
      DataPoolImpl::TYPE_DESC* pTypeDesc;

      inline const DataPoolImpl::TYPE_DESC* GetTypeDesc() const
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
    typedef clvector<DATAPOOL_ENUM_DESC>      EnumDescArray;

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

#endif // _DATAPOOL_BUILD_TIME_H_