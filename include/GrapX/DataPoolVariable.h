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
  public:
    struct VTBL;
    typedef const DATAPOOL_VARIABLE_DESC*     LPCVD;
    typedef DataPoolUtility::iterator         iterator;
    typedef DataPoolUtility::element_iterator element_iterator;

    enum CAPS
    {
      CAPS_FIXED    = 0x00000001,
      // CAPS_FIXED: 内存地址不变, 稳定变量的GetPtr()返回的地址是固定的，基于动态数组的变量可能随数据增加而改变
      //             这里需要注意，动态数组对象如果是根变量它也具有CAPS_FIXED属性，而它的数组元素不具有CAPS_FIXED属性
      CAPS_STRUCT   = 0x00000002, // 结构体
      CAPS_ARRAY    = 0x00000004, // 静态数组，也可以"GetLength() > 1"来判断
      CAPS_DYNARRAY = 0x00000008, // 动态数组
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
    inline DataPoolVariable operator[](int nIndex) const
    {
      return IndexOf((GXSIZE_T)nIndex);
    }

    inline DataPoolVariable operator[](GXLPCSTR szMemberName) const // 废弃
    {
      return MemberOf(szMemberName);
    }

    inline DataPoolVariable operator()(GXLPCSTR szMemberName) const
    {
      return MemberOf(szMemberName);
    }

    template<class _Ty>
    inline _Ty& CastTo() // Reinterpret cast
    {
      return *(_Ty*)GetPtr();
    }

  public:
    //DataPoolVariable& operator=   (const DataPoolVariable& val);

    // 分！别！重载不同类型，如果使用模板可能会接受其他不期望的类型
    DataPoolVariable& operator=   (GXLPCWSTR szString);
    DataPoolVariable& operator=   (GXLPCSTR szString);
    DataPoolVariable& operator=   (float val);
    DataPoolVariable& operator=   (i32 val);            // 如果变量不足32位会被截断
    DataPoolVariable& operator=   (i64 val);
    DataPoolVariable& operator=   (u32 val);            // 如果变量不足32位会被截断
    DataPoolVariable& operator=   (u64 val);
    DataPoolVariable& operator=   (const DataPoolVariable& var);  // var对象复制， 描述信息复制给新的目标

    // 编译提示: warning C4800: “Marimo::DataPoolVariable::unspecified_bool_type”: 将值强制为布尔值“true”或“false”(性能警告)
    //typedef void (*unspecified_bool_type)(DataPoolVariable***);
    //                  operator unspecified_bool_type() const;

    GXBOOL            Impulse         (DataAction reason, GXSIZE_T index = 0, GXSIZE_T count = 0); // index和count只有是数组时才有效
    GXHRESULT         GetPool         (DataPool** ppDataPool) const;
    DataPool*         GetPoolUnsafe   () const;
    GXBOOL            IsSamePool      (DataPool* pDataPool) const;
    GXVOID            Free            ();
    GXBOOL            IsValid         () const;  // 返回这个 Variable 是否有效
    //GXBOOL            IsEmpty         () const;  // 返回这个 Variable 是否无效
    GXLPVOID          GetPtr          () const;  // 指针, 要注意动态数据指针会因为 NewBack 操作改变
    GXUINT            GetSize         () const;  // 字节大小, 数组是数组大小, 动态数据大小可变, 结构是结构体大小
    GXUINT            GetOffset       () const;  // 偏移, 全局变量是全局偏移, 动态数组是在buffer上的偏移 /废弃：全局变量是全局偏移, 结构体变量是结构体内偏移/
    DataPool::LPCSTR  GetName         () const;  // 获得定义名, 变量名, 数组变量名或者结构体变量名
    DataPool::LPCSTR  GetTypeName     () const;  // 类型, 变量为变量名, 数组为"Type[n]"形式, 动态数组为"Type[]"形式, 结构体为"struct Name"形式
    DataPoolTypeClass GetTypeCategory () const;  // 变量的分类
    GXDWORD           GetCaps         () const;  // 参考CAPS枚举
    clStringA         GetFullName     () const;  // （没实现）获得变量的全称,如果在结构体中，包括结构体名，如果在数组中会包含数组索引,如同DataPool::QueryByExpression()参数那样.
    clBufferBase*     GetBuffer       () const;

    // 结构体专用
    DataPoolVariable  MemberOf        (GXLPCSTR szName) const;    // 获得成员

    // 数组或动态数组专用 
    DataPoolVariable  IndexOf         (GXSIZE_T nIndex) const;    // 获得特定索引的变量
    GXSIZE_T          GetLength       () const;                   // 获得数组的成员个数, 注意与GetSize区别
    DataPoolVariable  NewBack         (GXUINT nIncrease = 1);       // 在动态数组上追加数据, 动态数组专用, 如果inc大于1，返回第一个新增变量，如果inc为0，不会新增变量，返回最后一个数据
    GXBOOL            Remove          (GXSIZE_T nIndex, GXSIZE_T nCount = 1);        // 移出动态数组指定索引的数据, 动态数组专用, index=-1时表示全部删除，此时count必须为0

    // 变量专用
    GXBOOL            ParseW          (GXLPCWSTR szString, GXUINT length); // 按照变量类型转值, length=0表示按照'\0'结尾
    GXBOOL            ParseA          (GXLPCSTR szString, GXUINT length);

    clStringW         ToStringW       () const;           // 变量按照其含义转值, 数组和结构体等同于GetTypeName()
    clStringA         ToStringA       () const;

    //GXBOOL            CopyFrom        (const DataPoolVariable& var);  // 从一个变量对象拷贝值，如果是变量，变量类型一致才能成功，如果是结构体，会遍历同层成员变量，类型与名一致才能拷贝，否则就跳过。
    // 没实现 GXBOOL            TransferFrom    (const DataPoolVariable& var);  // 从一个变量对象传送值，如果是变量，会尽量解析到目标变量，如果是结构体，按照同名变量尽量解析到目标变量.
    GXBOOL            Set             (const DataPoolVariable& var);  // 注意这个和“operator=”含义不同, 只有非结构体，非数组对象才能赋值成功
    GXBOOL            Set             (GXLPCWSTR szString);
    GXBOOL            Set             (GXLPCSTR szString);
    GXBOOL            Set             (float val);
    GXBOOL            Set             (i32 val);            // 如果变量不足32位会被截断
    GXBOOL            Set             (i64 val);
    GXBOOL            Set             (u32 val);            // 如果变量不足32位会被截断
    GXBOOL            Set             (u64 val);

    GXBOOL            Retain          (GUnknown* pUnknown); // 安全的储存GUnknown对象，设置NULL时释放GUnknown对象。
    GXBOOL            Query           (GUnknown** ppUnknown) const;

    float             ToFloat         () const;
    u32               ToInteger       () const;
    u64               ToInteger64     () const;

    GXBOOL            SetData         (GXLPCVOID lpData, GXUINT cbSize);
    GXBOOL            GetData         (GXLPVOID lpData, GXUINT cbSize) const;

    iterator          begin           () const;  // 结构体成员迭代器
    iterator          end             () const;
    element_iterator  array_begin     () const;  // 数组成员迭代器
    element_iterator  array_end       () const;
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
//#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
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
//#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
        return *(_Ty*)GetPtr();
      }

      const _Ty& operator=(const _Ty& val)
      {
        *(_Ty*)GetPtr() = val;
//#ifdef ENABLE_DATAPOOL_WATCHER
        Impulse(DATACT_Change);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
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