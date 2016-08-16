#ifndef _DATAPOOL_BUILD_TIME_H_
#define _DATAPOOL_BUILD_TIME_H_

namespace Marimo
{
  class DataPoolVariableImpl;

  struct DataPoolBuildTime // Build time 缩写，用于构建时储存临时对象
  {
    friend class DataPoolImpl;
    //typedef clmap<clStringA, TYPE_DESC> TypeDict;
    typedef clstd::StaticStringsDict::HashType HashType;
    typedef clstd::StaticStringsDict::len_t    len_t;
    struct HASH_ALGORITHM
    {
      typedef clvector<int> IntArray;
      HashType eType;
      len_t    nBucket;
      int      nPos;
      IntArray indices;
    };

    struct BUILDTIME_TYPE_DECLARATION : public TYPE_DECLARATION
    {
      GXDWORD dwFlags; // BuildtimeTypeDeclarationFlags
    };

    struct BT_TYPE_DESC // : public DataPoolImpl::STRUCT_DESC
    {
      GXLPCSTR     szName;       // 需要一个稳定的地址
      GXUINT       nNameIndex;   // NameSet 加入的索引
      TypeCategory Cate;
      GXUINT       cbSize;

      GXUINT       nMemberIndex; // m_aStructMember 索引
      GXUINT       nMemberCount;

      HASH_ALGORITHM HashInfo;
      GXINT_PTR      nTypeAddress;
    };

    struct BT_VARIABLE_DESC // : public DATAPOOL_VARIABLE_DESC
    {
      //GXUINT      TypeDesc;         // 减法自定位，指向(TYPE_DESC*)类型
      GXLPCSTR    szName;             // 需要一个稳定的地址
      GXUINT      nNameIndex;         // NameSet 的索引
      GXUINT      nOffset;            // 全局变量是绝对偏移，成员变量是结构体内偏移
      GXUINT      nCount   : 30;      // 数组大小,一元变量应该是1,动态数组暂定为0
      GXUINT      bDynamic : 1;       // 应该使用IsDynamicArray()接口
      GXUINT      bConst   : 1;

      // 基类中TypeDesc成员不起作用!
      //DataPoolImpl::TYPE_DESC* pTypeDesc;
      const BT_TYPE_DESC* pTypeDesc;

      inline const BT_TYPE_DESC* GetTypeDesc() const
      {
        return pTypeDesc;
      }

      inline GXUINT TypeSize() const
      {
        return GetTypeDesc()->cbSize;
      }

      inline GXBOOL IsDynamicArray() const
      {
        return bDynamic;
      }

      GXUINT GetSize() const
      {
        ASSERT( ! IsDynamicArray()); // 不应该是动态数组
        return nCount * TypeSize();
      }
    };

    typedef clhash_map<clStringA, BUILDTIME_TYPE_DECLARATION> BuildTimeTypeDeclarationDict;
    typedef clmap<clStringA, BT_TYPE_DESC>    BTTypeDict;
    typedef clvector<BT_VARIABLE_DESC>        BTVarDescArray;
    typedef clvector<DATAPOOL_ENUM_DESC>      EnumDescArray;

    // 构建时检查用的类型声明结构
    enum BuildtimeTypeDeclarationFlags{
      BuildtimeTypeDeclaration_Used    = 0x0001,  // 使用过
      BuildtimeTypeDeclaration_Checked = 0x0002,  // 检查过
    };


  public:
    GXBOOL  IntCheckTypeDecl  (LPCTYPEDECL pTypeDecl, GXBOOL bCheck);
    GXBOOL  CheckVarList      (LPCVARDECL pVarDecl);

    void    PutTypeToDict     (GXLPCSTR szTypeName);
    GXINT   CalculateVarSize  (LPCVARDECL pVarDecl, BTVarDescArray& aVariableDesc);

    static
    void    TryHash           (HASH_ALGORITHM& hash_info, const BTVarDescArray& aDescs);

  protected:
    BuildTimeTypeDeclarationDict sBuildTimeTypeDeclDict;
    BTTypeDict          m_TypeDict;         // 类型
    BTVarDescArray      m_aVar;             // 全局变量
    BTVarDescArray      m_aStructMember;    // 所有的结构体成员描述都存在这张表上
    EnumDescArray       m_aEnumPck;         // 所有枚举成员都在这个表上
    clstd::StringSetA   NameSet;
    HASH_ALGORITHM      m_VarHashInfo;
    size_t              m_nNumOfBuckets;
    size_t              m_nNumOfStructs;    // m_TypeDict.size() > m_nNumOfStructs
    GXUINT              m_bPtr64 : 1;       // 64位指针兼容模式
    GXUINT              m_bFixedPool : 1;

    DataPoolBuildTime() : m_bPtr64(0), m_bFixedPool(1), m_nNumOfStructs(0), m_nNumOfBuckets(0) {}
  };



} // namespace Marimo

#endif // _DATAPOOL_BUILD_TIME_H_