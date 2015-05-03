#ifndef _MARIMO_DATA_POOL_VARIABLE_
#define _MARIMO_DATA_POOL_VARIABLE_

#ifndef _MARIMO_DATA_POOL_H_
#error 必须在之前包含"DataPool.h"文件
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
      CAPS_FIXED    = 0x00000001, // 内存地址不变, 稳定变量的GetPtr()返回的地址是固定的，基于动态数组的变量可能随数据增加而改变
      CAPS_STRUCT   = 0x00000002, // 结构体
      CAPS_DYNARRAY = 0x00000004, // 动态数组
    };
  protected:
    VTBL*         m_vtbl;
    DataPool*     m_pDataPool;
    LPCVD         m_pVdd;
    clBufferBase* m_pBuffer;
    GXUINT        m_AbsOffset; // 相对于 m_pBuffer 指针的偏移

  public:
    DataPoolVariable(VTBL* vtbl, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nOffset);
    DataPoolVariable(VTBL* vtbl, DataPool* pDataPool, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nOffset);
    DataPoolVariable(const DataPoolVariable& val);
    DataPoolVariable();
    ~DataPoolVariable(); // 不能声明为虚函数!

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

    // 分！别！重载不同类型，如果使用模板可能会接受其他不期望的类型
    DataPoolVariable& operator=   (GXLPCWSTR szString);
    DataPoolVariable& operator=   (GXLPCSTR szString);
    DataPoolVariable& operator=   (float val);
    DataPoolVariable& operator=   (i32 val);            // 如果变量不足32位会被截断
    DataPoolVariable& operator=   (i64 val);
    DataPoolVariable& operator=   (u32 val);            // 如果变量不足32位会被截断
    DataPoolVariable& operator=   (u64 val);


#ifdef ENABLE_DATAPOOL_WATCHER
    GXHRESULT         Impulse         (DataAction eAct, GXUINT nIndex = 0);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    GXHRESULT         GetPool         (DataPool** ppDataPool) GXCONST;
    DataPool*         GetPoolUnsafe   () GXCONST;
    GXBOOL            IsSamePool      (DataPool* pDataPool) GXCONST;
    GXVOID            Free            ();
    GXBOOL            IsValid         () GXCONST;  // 返回这个 Variable 是否有效
    GXLPVOID          GetPtr          () GXCONST;  // 指针, 要注意动态数据指针会因为 NewBack 操作改变
    GXUINT            GetSize         () GXCONST;  // 字节大小, 数组是数组大小, 动态数据大小可变, 结构是结构体大小
    GXUINT            GetOffset       () GXCONST;  // 偏移,全局变量是全局偏移, 结构体变量是结构体内偏移
    GXLPCSTR          GetName         () GXCONST;  // 获得定义名, 变量名, 数组变量名或者结构体变量名
    GXLPCSTR          GetTypeName     () GXCONST;  // 类型, 变量为变量名, 数组为"Type[n]"形式, 动态数组为"Type[]"形式, 结构体为"struct Name"形式
    TypeCategory      GetTypeCategory () GXCONST;  // 变量的分类
    GXDWORD           GetCaps         () GXCONST;  // 参考CAPS枚举
    clStringA         GetFullName     () GXCONST;  // （没实现）获得变量的全称,如果在结构体中，包括结构体名，如果在数组中会包含数组索引,如同DataPool::QueryByExpression()参数那样.
    clBufferBase*     GetBuffer       () GXCONST;

    // 结构体专用
    DataPoolVariable  MemberOf        (GXLPCSTR szName) GXCONST;     // 获得成员

    // 数组或动态数组专用 
    DataPoolVariable  IndexOf         (int nIndex) GXCONST;         // 获得特定索引的变量
    GXUINT            GetLength       () GXCONST;                   // 获得数组的成员个数, 注意与GetSize区别
    DataPoolVariable  NewBack         (GXUINT nIncrease = 1);       // 在动态数组上追加数据, 动态数组专用, 如果inc大于1，返回第一个新增变量，如果inc为0，不会新增变量，返回最后一个数据
    GXBOOL            Remove          (GXUINT nIndex, GXUINT nCount = 1);        // 移出动态数组指定索引的数据, 动态数组专用

    // 变量专用
    GXBOOL            ParseW          (GXLPCWSTR szString, GXUINT length); // 按照变量类型转值, length=0表示按照'\0'结尾
    GXBOOL            ParseA          (GXLPCSTR szString, GXUINT length);

    clStringW         ToStringW       () GXCONST;           // 变量按照其含义转值, 数组和结构体等同于GetTypeName()
    clStringA         ToStringA       () GXCONST;

    GXBOOL            Set             (GXLPCWSTR szString);
    GXBOOL            Set             (GXLPCSTR szString);
    GXBOOL            Set             (float val);
    GXBOOL            Set             (i32 val);            // 如果变量不足32位会被截断
    GXBOOL            Set             (i64 val);
    GXBOOL            Set             (u32 val);            // 如果变量不足32位会被截断
    GXBOOL            Set             (u64 val);

    GXBOOL            Retain          (GUnknown* pUnknown); // 安全的储存GUnknown对象，设置NULL时释放GUnknown对象。
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
    // 基础类型的模板套用
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
    // 结构体类型套用
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

// 无法实现字符串类型，因为字符串类型在使用前可能需要正确初始化
// 而这个模板的“=”操作重载没有隐式初始化字符串类型
//typedef Marimo::Implement::VarPrimaryT<clStringA> MOVarStringA;
//typedef Marimo::Implement::VarPrimaryT<clStringW> MOVarStringW;

#endif // #ifndef _MARIMO_DATA_POOL_VARIABLE_