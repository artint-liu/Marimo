#ifndef _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_
#define _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_

namespace Marimo
{
  class DataPoolVariableImpl;
  // 内部实现的函数表
  struct DataPoolVariable::VTBL
  {
    typedef DataPoolVariable Variable;
    typedef DataPoolVariableImpl VarImpl;
    

    GXUINT    (*GetSize     )(GXCONST VarImpl* pThis);  // 字节大小, 数组是数组大小, 动态数据大小可变, 结构是是结构体大小
    //GXUINT    (*GetOffset   )(GXCONST VarImpl* pThis);  // 偏移,全局变量是全局偏移, 结构体变量是结构体内偏移
    //GXLPCSTR  (*GetName     )(GXCONST VarImpl* pThis);  // 获得定义名, 变量名, 数组变量名或者结构体变量名
    //GXLPCSTR  (*GetTypeName )(GXCONST VarImpl* pThis);  // 类型, 变量为变量名, 数组为"Type[n]"形式, 动态数组为"Type[]"形式, 结构体为"struct Name"形式

    // 结构体专用
    Variable  (*GetMember   )(GXCONST VarImpl* pThis, GXLPCSTR szName);    // 获得成员

    // 数组或动态数组专用 
    Variable  (*GetIndex    )(GXCONST VarImpl* pThis, int nIndex);         // 获得特定索引的变量
    GXUINT    (*GetLength   )(GXCONST VarImpl* pThis);                     // 获得数组的成员个数, 注意与GetSize区别
    Variable  (*NewBack     )(        VarImpl* pThis, GXUINT nIncrease);   // 在动态数组上追加数据, 动态数组专用
    GXBOOL    (*Remove      )(        VarImpl* pThis, GXUINT nIndex, GXUINT nCount);      // 移出动态数组指定索引的数据, 动态数组专用

    // 变量专用
    //clStringW (*ToStringW   )(GXCONST VarImpl* pThis);                   // 变量按照其含义转值, 数组和结构体等同于GetTypeName()
    //clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*ParseW      )(        VarImpl* pThis, GXLPCWSTR szString, GXUINT length); // 按照变量类型转值(unicode)
    GXBOOL    (*ParseA      )(        VarImpl* pThis, GXLPCSTR szString, GXUINT length);  // 按照变量类型转值
    u32       (*ToInteger   )(GXCONST VarImpl* pThis);
    u64       (*ToInt64     )(GXCONST VarImpl* pThis);
    float     (*ToFloat     )(GXCONST VarImpl* pThis);
    clStringW (*ToStringW   )(GXCONST VarImpl* pThis);
    clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*SetAsInteger)(        VarImpl* pThis, u32 val);             // 如果变量不足32位会被截断
    GXBOOL    (*SetAsInt64  )(        VarImpl* pThis, u64 val);
    GXBOOL    (*SetAsFloat  )(        VarImpl* pThis, float val);
    GXBOOL    (*SetAsStringW)(        VarImpl* pThis, GXLPCWSTR szString);
    GXBOOL    (*SetAsStringA)(        VarImpl* pThis, GXLPCSTR szString);
    GXBOOL    (*Retain      )(        VarImpl* pThis, GUnknown* pUnknown);
    GXBOOL    (*Query       )(GXCONST VarImpl* pThis, GUnknown** ppUnknown);
    GXBOOL    (*GetData     )(GXCONST VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize);
    GXBOOL    (*SetData     )(        VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize);
  };

  // 如果是数字类型，则转换返回TRUE，否则返回FALSE
  // 支持0x十六进制，0b二进制，0开头的八进制和十进制
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



  struct DataPoolBuildTime // Build time 缩写，用于构建时储存临时对象
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
      // 基类中TypeDesc成员不起作用!
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
        ASSERT( ! IsDynamicArray()); // 不应该是动态数组
        return nCount * TypeSize();
      }
    };

    typedef clhash_map<clStringA, BUILDTIME_TYPE_DECLARATION> BuildTimeTypeDeclarationDict;
    typedef clmap<clStringA, BT_TYPE_DESC>    BTTypeDict;
    typedef clvector<BT_VARIABLE_DESC>        BTVarDescArray;
    typedef clvector<DataPool::ENUM_DESC>     EnumDescArray;

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

  protected:
    BuildTimeTypeDeclarationDict sBuildTimeTypeDeclDict;
    BTTypeDict          m_TypeDict;         // 类型
    BTVarDescArray      m_aVar;             // 全局变量
    BTVarDescArray      m_aStructMember;    // 所有的结构体成员描述都存在这张表上
    EnumDescArray       m_aEnumPck;         // 所有枚举成员都在这个表上
    clstd::StringSetA   NameSet;
    GXUINT              m_bPtr64 : 1;       // 64位指针兼容模式
    GXUINT              m_bFixedPool : 1;

    DataPoolBuildTime() : m_bPtr64(0), m_bFixedPool(1) {}
  };



} // namespace Marimo

#endif // _DATAPOOL_VARIABLE_VIRTUAL_TABLE_H_