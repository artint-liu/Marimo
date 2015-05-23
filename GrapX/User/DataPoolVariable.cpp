#include "GrapX.H"
#include "GrapX.Hxx"

#include "clStringSet.h"

//#include "GrapX/GUnknown.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataPoolIterator.h"

#define Flag_ParseW             Flag_ParseT<clStringW>
#define Flag_ParseA             Flag_ParseT<clStringA>
#define Enum_ToStringW          Enum_ToStringT<clStringW>
#define Enum_ToStringA          Enum_ToStringT<clStringA>
#define Flag_ToStringW          Flag_ToStringT<clStringW>
#define Flag_ToStringA          Flag_ToStringT<clStringA>
#define Primary_ParseW          Primary_ParseT<GXWCHAR>
#define Primary_ParseA          Primary_ParseT<GXCHAR>
#define Exception_ParseW        Exception_ParseT<GXWCHAR>
#define Exception_ParseA        Exception_ParseT<GXCHAR>
#define Struct_ToStringW        Struct_ToStringT<clStringW>
#define Struct_ToStringA        Struct_ToStringT<clStringA>
#define Object_ToStringW        Object_ToStringT<clStringW>
#define Object_ToStringA        Object_ToStringT<clStringA>
#define Enum_SetAsStringW       Enum_SetAsStringT<clStringW>
#define Enum_SetAsStringA       Enum_SetAsStringT<clStringA>
#define Flag_SetAsStringW       Flag_SetAsStringT<clStringW>
#define Flag_SetAsStringA       Flag_SetAsStringT<clStringA>
#define String_SetAsStringW     String_SetAsStringT<GXLPCWSTR, clStringW>
#define String_SetAsStringA     String_SetAsStringT<GXLPCSTR, clStringW>
#define StaticArray_ToStringW   StaticArray_ToStringT<clStringW>
#define StaticArray_ToStringA   StaticArray_ToStringT<clStringA>
#define Exception_SetAsStringW  Exception_SetAsStringT<GXWCHAR>
#define Exception_SetAsStringA  Exception_SetAsStringT<GXCHAR>
#define String_ToStringW        String_ToStringT<clStringW, clStringW>
#define String_ToStringA        String_ToStringT<clStringA, clStringW>
#define StringA_ToStringW       String_ToStringT<clStringW, clStringA>
#define StringA_ToStringA       String_ToStringT<clStringA, clStringA>
#define StringA_SetAsStringW    String_SetAsStringT<GXLPCWSTR, clStringA>
#define StringA_SetAsStringA    String_SetAsStringT<GXLPCSTR, clStringA>

#ifdef ENABLE_DATAPOOL_WATCHER
#define THIS_IMPULSE_DATA_CHANGE              pThis->Impulse(DATACT_Change)
#define THIS_IMPULSE_DATA(_DATA_ACT, _INDEX)  pThis->Impulse(_DATA_ACT, _INDEX)
#else
#define THIS_IMPULSE_DATA_CHANGE
#define THIS_IMPULSE_DATA(_DATA_ACT, _INDEX)
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

using namespace clstd;

#include "DataPoolVariableVtbl.h"

namespace Marimo
{
  // 类型重定义
  typedef DataPoolVariable                          Variable;
  typedef DataPoolVariableImpl                      VarImpl;

  // 函数声明
  Variable DynamicArray_NewBack(VarImpl* pThis, GXUINT nIncrease);
  Variable Struct_GetMember(GXCONST VarImpl* pThis, GXLPCSTR szName);
  Variable Array_GetIndex(GXCONST VarImpl* pThis, int nIndex);

  DataPoolVariable::DataPoolVariable( VTBL* vtbl, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nAbsOffset )
    : m_vtbl(vtbl), m_pVdd(pVdd), m_pBuffer(pBufferBase), m_AbsOffset(nAbsOffset)
  {
    // 如果是object类型，这里可以不增加计数。原因见其它构造函数
  }

  DataPoolVariable::DataPoolVariable( VTBL* vtbl, DataPool* pDataPool, LPCVD pVdd, clBufferBase* pBufferBase, GXUINT nAbsOffset ) 
    : m_vtbl(vtbl), m_pDataPool(pDataPool), m_pVdd(pVdd), m_pBuffer(pBufferBase), m_AbsOffset(nAbsOffset)
  {
    // 如果是object类型，这里可以不增加计数。原因见其它构造函数
    if(m_pDataPool) {m_pDataPool->AddRef();}
  }

  DataPoolVariable::DataPoolVariable( const DataPoolVariable& val ) : m_vtbl(NULL), m_pDataPool(NULL), m_pVdd(NULL), m_pBuffer(NULL), m_AbsOffset(0)
  {
    *this = val; // 等号重载，内部增加了DataPool引用计数

    // 如果是object类型，这里可以不增加计数。
    // DataPool 中维护了 object 对象引用计数
    // 而 DataPoolVariable 在生命周期内维护了 DataPool 的引用计数
    // 所以这里就不必维护 object 的引用计数了
  }

  DataPoolVariable::DataPoolVariable() : m_vtbl(NULL), m_pDataPool(NULL), m_pVdd(NULL), m_pBuffer(NULL), m_AbsOffset(0)
  {
  }

  DataPoolVariable::~DataPoolVariable()
  {
    Free();
  }

  //////////////////////////////////////////////////////////////////////////
  // 这个类里面不储存成员变量，只是用来扩展访问DataPoolVariable
  // 成员变量的方法，这样使用户使用的头文件看起来更简洁
  class DataPoolVariableImpl : public DataPoolVariable
  {
  public:
    inline VTBL* InlGetVtbl() GXCONST
    {
      return m_vtbl;
    }

    inline DataPool* InlGetDataPool() GXCONST
    {
      return m_pDataPool;
    }

    inline LPCVD InlGetVDD() GXCONST
    {
      return m_pVdd;
    }

    //inline GXUINT_PTR InlStringBase() GXCONST
    //{
    //  return m_pDataPool->m_StringBase;
    //}
    inline GXBOOL InlCleanupArray(LPCVD pVarDesc, GXLPVOID lpBuffer, int nCount)
    {
      return m_pDataPool->CleanupArray(pVarDesc, lpBuffer, nCount);
    }

    inline clBufferBase* InlGetBufferObj() GXCONST
    {
      return m_pBuffer;
    }

    inline GXLPBYTE InlGetBufferPtr() GXCONST
    {
      return (GXLPBYTE)m_pBuffer->GetPtr();
    }

    inline GXBOOL IsReadOnly() GXCONST
    {
      return TEST_FLAG(m_pDataPool->m_dwRuntimeFlags, DataPool::RuntimeFlag_Readonly);
    }    

    template<typename T_LPCSTR, class _TStoString>
    inline GXBOOL SetAsString(T_LPCSTR szText)
    {
      // 这个函数是String类型专用的
      ASSERT(m_pVdd->GetTypeCategory() == T_STRING || 
        m_pVdd->GetTypeCategory() == T_STRINGA);

      // 只读模式跳过
      if(TEST_FLAG(m_pDataPool->m_dwRuntimeFlags, DataPool::RuntimeFlag_Readonly)) {
        return FALSE;
      }

      GXLPBYTE pStringData = (GXLPBYTE)GetPtr();
      if(*(void**)(pStringData) == NULL) {
        new(pStringData) _TStoString(szText);

        // 只读模式下不会新建字符串
        ASSERT(TEST_FLAG_NOT(m_pDataPool->m_dwRuntimeFlags, DataPool::RuntimeFlag_Readonly));
#ifdef _DEBUG
        m_pDataPool->m_nDbgNumOfString++;
#endif // #ifdef _DEBUG
      }
      else {
        *(_TStoString*)((GXBYTE*)pStringData) = szText;
      }
      return TRUE;
    }

//#ifdef _DEBUG
//    inline void InlIncreateStringRefCount()
//    {
//      ASSERT( ! m_pDataPool->m_bReadOnly); // 只读模式下不会新建字符串
//      m_pDataPool->m_nDbgNumOfString++;
//    }
//#endif // #ifdef _DEBUG

    inline DataPool::LPCED InlGetEnum(GXUINT nIndex) GXCONST
    {
      return m_pVdd->GetTypeDesc()->GetEnumMembers() + nIndex;
    }

    inline GXBOOL InlFindEnumFlagValue(DataPool::LPCSTR szName, DataPool::EnumFlag* pOutEnumFlag) GXCONST
    {
      GXBOOL bval = m_pDataPool->IntFindEnumFlagValue(m_pVdd->GetTypeDesc(), szName, pOutEnumFlag);

#ifdef _DEBUG
      // 使用传统函数验证二分法查找
      for(GXUINT i = 0; i < m_pVdd->GetTypeDesc()->nMemberCount; ++i)
      {
        DataPool::LPCED pair = m_pVdd->GetTypeDesc()->GetEnumMembers() + i;
          //m_pDataPool->IntGetEnum(m_pVdd->GetTypeDesc()->nMemberIndex + i);
        if(GXSTRCMP(szName, pair->GetName()) == 0)
        {
          ASSERT(bval && *pOutEnumFlag == pair->Value);
          *pOutEnumFlag = pair->Value;
          return TRUE;
        }
      }

      ASSERT( ! bval);
#endif // #ifdef _DEBUG

      return bval;
    }

    inline TypeCategory InlGetCategory() GXCONST
    {
      return (m_pVdd == NULL || m_pVdd->GetTypeDesc() == NULL)
        ? T_UNDEFINE
        : m_pVdd->GetTypeCategory();
    }

    inline GXBOOL InlIsPrimaryType() GXCONST
    {
      // 判断是否为基础类型，struct和object不认为是基础类型
      const TypeCategory cate = InlGetCategory();
      return (cate == T_BYTE || cate == T_WORD || 
        cate == T_DWORD || cate == T_QWORD ||
        cate == T_SBYTE || cate == T_SWORD ||
        cate == T_SDWORD || cate == T_SQWORD ||
        cate == T_FLOAT || cate == T_STRING ||
        cate == T_STRINGA ||
        cate == T_ENUM || cate == T_FLAG);
    }

    inline GXHRESULT InlQuery(GXLPCSTR szName, DataPoolVariable* pBase) GXCONST
    {
      ASSERT(m_pBuffer != NULL);
      DataPool::VARIABLE var = {0};
      if(GXSUCCEEDED(m_pDataPool->IntQuery(m_pBuffer, m_pVdd, szName, m_AbsOffset, &var)))
      {
        new(pBase) DataPoolVariable((VTBL*)var.vtbl, m_pDataPool, var.pVdd, var.pBuffer, var.AbsOffset);
        return GX_OK;
      }
      return GX_FAIL;
    }

    inline GXHRESULT InlSetupUnary(GXUINT nIndex, DataPoolVariable* pBase) GXCONST
    {
      //return m_pDataPool->IntCreateUnary(
      //  pBufferBase ? pBufferBase : m_pBuffer, m_pVdd, nIndex * m_pVdd->TypeSize(), pBase);
      ASSERT(m_pBuffer != NULL);
      DataPool::VARIABLE var = {0};
      //var.AbsOffset = m_pVdd->nOffset;
      var.AbsOffset = m_AbsOffset;
      if(m_pDataPool->IntCreateUnary(m_pBuffer, m_pVdd, nIndex * m_pVdd->TypeSize(), &var))
      {
        new(pBase) DataPoolVariable((VTBL*)var.vtbl, m_pDataPool, var.pVdd, var.pBuffer, var.AbsOffset);
        return GX_OK;
      }
      return GX_FAIL;
    }

    template <class _Fn>
    GXBOOL ForEachEnum(DataPool::Enum& eValue, _Fn func) const
    {
      const auto nCount = InlGetVDD()->GetTypeDesc()->nMemberCount;
      eValue = (DataPool::Enum)*(u32*)GetPtr();
      for (GXUINT i = 0; i < nCount; i++)
      {
        //auto sDesc = m_pDataPool->IntGetEnum(m_pVdd->GetTypeDesc()->nMemberIndex + i);
        auto sDesc = m_pVdd->GetTypeDesc()->GetEnumMembers() + i;
        if( ! func(sDesc, eValue)) {
          return TRUE;
        }
      }
      return FALSE;
    }

    void first_child(iterator& iter)
    {
      if(m_pVdd->GetTypeCategory() == T_STRUCT)
      {
        iter.pDataPool = m_pDataPool;
        iter.pVarDesc  = m_pVdd->MemberBeginPtr();
        iter.pBuffer   = m_pBuffer;
        iter.nOffset   = m_AbsOffset;
        iter.index     = -1;
      }
      else {
        iter.pDataPool = NULL;
        iter.pVarDesc  = NULL;
        iter.pBuffer   = NULL;
        iter.nOffset   = 0;
        iter.index     = 0;
      }
    }

    void first_element(element_iterator& iter)
    {
      iter.pDataPool = m_pDataPool;
      iter.pVarDesc  = m_pVdd;
      if(m_pVdd->IsDynamicArray())
      {
        DataPoolArray* pArrayBuffer = static_cast<DataPoolArray*>(InlGetBufferObj());
        iter.pBuffer   = pArrayBuffer;
        iter.nOffset   = 0;
      }
      else
      {
        iter.pBuffer   = m_pBuffer;
        iter.nOffset   = m_AbsOffset - m_pVdd->nOffset;
      }
      iter.index     = 0;
    }
  }; // class DataPoolVariableImpl : public DataPoolVariable

  STATIC_ASSERT(sizeof(DataPoolVariableImpl) == sizeof(DataPoolVariable)); // 确保DataPoolVariableImpl不会增加新的成员变量

  //////////////////////////////////////////////////////////////////////////
  Marimo::DataPoolUtility::iterator DataPoolVariable::begin()
  {
    iterator iter;
    reinterpret_cast<DataPoolVariableImpl*>(this)->first_child(iter);
    return iter;
  }

  Marimo::DataPoolUtility::iterator DataPoolVariable::end()
  {
    iterator iter;
    reinterpret_cast<DataPoolVariableImpl*>(this)->first_child(iter);
    if(iter.pVarDesc) {
      iter.pVarDesc += m_pVdd->MemberCount();
    }
    return iter;
  }
  
  Marimo::DataPoolUtility::element_iterator DataPoolVariable::array_begin()
  {
    element_iterator iter;
    reinterpret_cast<DataPoolVariableImpl*>(this)->first_element(iter);
    return iter;
  }

  Marimo::DataPoolUtility::element_iterator DataPoolVariable::array_end()
  {
    element_iterator iter;
    reinterpret_cast<DataPoolVariableImpl*>(this)->first_element(iter);
    iter.index = GetLength();
    return iter;
  }

  //////////////////////////////////////////////////////////////////////////


  namespace Implement
  {
    extern Variable::VTBL s_PrimaryVtbl;
    extern Variable::VTBL s_EnumVtbl;
    extern Variable::VTBL s_FlagVtbl;
    extern Variable::VTBL s_ObjectVtbl;
    extern Variable::VTBL s_StringVtbl;
    extern Variable::VTBL s_StringAVtbl;
    extern Variable::VTBL s_StructVtbl;
    extern Variable::VTBL s_StaticArrayVtbl;
    extern Variable::VTBL s_DynamicArrayVtbl;

    Variable::VTBL* s_pPrimaryVtbl      = &s_PrimaryVtbl;
    Variable::VTBL* s_pEnumVtbl         = &s_EnumVtbl;
    Variable::VTBL* s_pFlagVtbl         = &s_FlagVtbl;
    Variable::VTBL* s_pObjectVtbl       = &s_ObjectVtbl;
    Variable::VTBL* s_pStringVtbl       = &s_StringVtbl;
    Variable::VTBL* s_pStringAVtbl      = &s_StringAVtbl;
    Variable::VTBL* s_pStructVtbl       = &s_StructVtbl;
    Variable::VTBL* s_pStaticArrayVtbl  = &s_StaticArrayVtbl;
    Variable::VTBL* s_pDynamicArrayVtbl = &s_DynamicArrayVtbl;
  } // namespace Implement

  // 预置类型
  VARIABLE_DECLARATION c_float2[] = {
    {"float", "x"},
    {"float", "y"},
    {NULL, NULL},
  };

  VARIABLE_DECLARATION c_float3[] = {
    {"float", "x"},
    {"float", "y"},
    {"float", "z"},
    {NULL, NULL},
  };

  VARIABLE_DECLARATION c_float4[] = {
    {"float", "x"},
    {"float", "y"},
    {"float", "z"},
    {"float", "w"},
    {NULL, NULL},
  };

  VARIABLE_DECLARATION c_float3x3[] = {
    {"float", "m", 0, 9},
    {NULL, NULL},
  };

  VARIABLE_DECLARATION c_float4x4[] = {
    {"float", "m", 0, 16},
    {NULL, NULL},
  };

  namespace Implement
  {
    TYPE_DECLARATION c_InternalTypeDefine[] = {
      {Marimo::T_FLOAT,  "float"},
      {Marimo::T_BYTE,   "BYTE"},
      {Marimo::T_WORD,   "WORD"},
      {Marimo::T_DWORD,  "DWORD"},
      {Marimo::T_QWORD,  "QWORD"},
      {Marimo::T_BYTE,   "unsigned_char"},
      {Marimo::T_WORD,   "unsigned_short"},
      {Marimo::T_DWORD,  "unsigned_int"},
      {Marimo::T_QWORD,  "unsigned_longlong"},

      {Marimo::T_SBYTE,  "char"},
      {Marimo::T_SWORD,  "short"},
      {Marimo::T_SDWORD, "int"},
      {Marimo::T_SQWORD, "longlong"},

      {Marimo::T_STRING, "string"},
      {Marimo::T_STRINGA, "stringA"},
      {Marimo::T_OBJECT, "object"},

      {Marimo::T_STRUCT, "float2"  , c_float2},
      {Marimo::T_STRUCT, "float3"  , c_float3},
      {Marimo::T_STRUCT, "float4"  , c_float4},
      {Marimo::T_STRUCT, "float3x3", c_float3x3},
      {Marimo::T_STRUCT, "float4x4", c_float4x4},
      {T_UNDEFINE, NULL},
    };
  } // namespace Implement

  //////////////////////////////////////////////////////////////////////////
  Variable& Variable::operator=(const Variable& val)
  {
    if(m_pDataPool == val.m_pDataPool) {
      memcpy(this, &val, sizeof(Variable));
    }
    else {
      SAFE_RELEASE(m_pDataPool);
      memcpy(this, &val, sizeof(Variable));
      if(m_pDataPool) {
        m_pDataPool->AddRef();
      }
    }
    return *this;
  }

#ifdef ENABLE_DATAPOOL_WATCHER
  GXHRESULT Variable::Impulse(DataAction eAct, GXUINT nIndex)
  {
    if(m_pDataPool != NULL) {
      return m_pDataPool->ImpluseByVariable(eAct, *this, nIndex, TRUE);
    }
    return GX_OK;
  }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

  GXHRESULT Variable::GetPool(DataPool** ppDataPool) GXCONST
  {
    if(m_pDataPool != NULL)
    {
      m_pDataPool->AddRef();
      *ppDataPool = m_pDataPool;
      return GX_OK;
    }
    return GX_FAIL;
  }

  DataPool* Variable::GetPoolUnsafe() GXCONST
  {
    return m_pDataPool;
  }

  GXBOOL Variable::IsSamePool(DataPool* pDataPool) GXCONST
  {
    return m_pDataPool == pDataPool;
  }

#define IMPLEMENT_EQUAL_OPERATOR_FUNC(t, v) DataPoolVariable& DataPoolVariable::operator=(t v) { Set(v); return *this; }
#define CAST2VARPTR  reinterpret_cast<VarImpl*>
#define CAST2VARPTRC reinterpret_cast<GXCONST VarImpl*>

  IMPLEMENT_EQUAL_OPERATOR_FUNC(GXLPCWSTR, szString);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(GXLPCSTR, szString);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(float, val);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(i32, val);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(i64, val);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(u32, val);
  IMPLEMENT_EQUAL_OPERATOR_FUNC(u64, val);

  GXVOID Variable::Free()
  {
    SAFE_RELEASE(m_pDataPool);
  }

  GXBOOL Variable::IsValid() GXCONST
  {
    return m_pDataPool != NULL && m_vtbl != NULL && m_pBuffer != NULL;
  }

  GXLPVOID Variable::GetPtr() GXCONST
  {
    return (GXLPBYTE)m_pBuffer->GetPtr() + m_AbsOffset;
  }

  //GXBOOL Variable::IsFixed() GXCONST
  //{
  //  // FIXME: 没有检查空值
  //  return m_pDataPool->IntGetEntryBuffer() == m_pBuffer;
  //}

  GXDWORD Variable::GetCaps() GXCONST
  {
    //m_pVdd->IsDynamicArray()
    GXDWORD r = 0;

    if(m_pDataPool->IntGetEntryBuffer() == m_pBuffer) {
      SET_FLAG(r, CAPS_FIXED);
    }

    if(m_vtbl->NewBack == DynamicArray_NewBack) {
      SET_FLAG(r, CAPS_DYNARRAY);
      ASSERT(m_pVdd->IsDynamicArray());
    }
    else if(m_vtbl->GetIndex == Array_GetIndex) { // 动态数组和静态数组都具有 Array_GetIndex 方法
      SET_FLAG(r, CAPS_ARRAY);
      ASSERT(GetLength() > 1);
    }

    if(m_vtbl->GetMember == Struct_GetMember) {
      SET_FLAG(r, CAPS_STRUCT);
      ASSERT(m_pVdd->GetTypeDesc()->Cate == T_STRUCT);
    }

    return r;
  }

  clStringA Variable::GetFullName() GXCONST
  {
    clStringA str;
    if(m_pDataPool->FindFullName(&str, m_pVdd, m_pBuffer, m_AbsOffset)) {
      return str;
    }
    return "";
  }

  clBufferBase* Variable::GetBuffer() GXCONST
  {
    return m_pBuffer;
  }

  GXUINT Variable::GetOffset() GXCONST
  {
    //return m_vtbl->GetOffset   (this);
    return m_AbsOffset;
  }
  
  GXLPCSTR Variable::GetName() GXCONST
  {
    return m_pVdd->VariableName();
    //return m_vtbl->GetName(this);
  }
  
  GXLPCSTR Variable::GetTypeName() GXCONST
  {
    return m_pVdd->TypeName();
    //return m_vtbl->GetTypeName(this);
  }

  TypeCategory Variable::GetTypeCategory() GXCONST
  {
    return CAST2VARPTRC(this)->InlGetCategory();
  }


  GXUINT    Variable::GetSize     () GXCONST                              { return m_vtbl->GetSize     (CAST2VARPTRC(this));                    }
  Variable  Variable::MemberOf    (GXLPCSTR szName) GXCONST               { return m_vtbl->GetMember   (CAST2VARPTRC(this), szName);            }
  Variable  Variable::IndexOf     (int nIndex) GXCONST                    { return m_vtbl->GetIndex    (CAST2VARPTRC(this), nIndex);            }
  GXUINT    Variable::GetLength   () GXCONST                              { return m_vtbl->GetLength   (CAST2VARPTRC(this));                    }
  Variable  Variable::NewBack     (GXUINT nIncrease)                      { return m_vtbl->NewBack     (CAST2VARPTR (this), nIncrease);         }
  GXBOOL    Variable::Remove      (GXUINT nIndex, GXUINT nCount)          { return m_vtbl->Remove      (CAST2VARPTR (this), nIndex, nCount);    }
  clStringW Variable::ToStringW   () GXCONST                              { return m_vtbl->ToStringW   (CAST2VARPTRC(this));                    }
  clStringA Variable::ToStringA   () GXCONST                              { return m_vtbl->ToStringA   (CAST2VARPTRC(this));                    }
  GXBOOL    Variable::ParseW      (GXLPCWSTR szString, GXUINT length)     { return m_vtbl->ParseW      (CAST2VARPTR (this), szString, length);  }
  GXBOOL    Variable::ParseA      (GXLPCSTR szString, GXUINT length)      { return m_vtbl->ParseA      (CAST2VARPTR (this), szString, length);  }

  GXBOOL    Variable::Set         (GXLPCWSTR szString)                    { return m_vtbl->SetAsStringW(CAST2VARPTR (this), szString);          }
  GXBOOL    Variable::Set         (GXLPCSTR szString)                     { return m_vtbl->SetAsStringA(CAST2VARPTR (this), szString);          }
  GXBOOL    Variable::Set         (float val)                             { return m_vtbl->SetAsFloat  (CAST2VARPTR (this), val);               }
  GXBOOL    Variable::Set         (i32 val)                               { return m_vtbl->SetAsInteger(CAST2VARPTR (this), val);               }
  GXBOOL    Variable::Set         (i64 val)                               { return m_vtbl->SetAsInt64  (CAST2VARPTR (this), val);               }
  GXBOOL    Variable::Set         (u32 val)                               { return m_vtbl->SetAsInteger(CAST2VARPTR (this), val);               }
  GXBOOL    Variable::Set         (u64 val)                               { return m_vtbl->SetAsInt64  (CAST2VARPTR (this), val);               }

  float     Variable::ToFloat     () GXCONST                              { return m_vtbl->ToFloat     (CAST2VARPTRC(this));                    }
  u32       Variable::ToInteger   () GXCONST                              { return m_vtbl->ToInteger   (CAST2VARPTRC(this));                    }
  u64       Variable::ToInteger64 () GXCONST                              { return m_vtbl->ToInt64     (CAST2VARPTRC(this));                    }

  GXBOOL    Variable::Retain      (GUnknown* pUnknown)                    { return m_vtbl->Retain      (CAST2VARPTR (this), pUnknown);          }
  GXBOOL    Variable::Query       (GUnknown** ppUnknown) GXCONST          { return m_vtbl->Query       (CAST2VARPTRC(this), ppUnknown);         }

  GXBOOL    Variable::SetData     (GXLPCVOID lpData, GXUINT cbSize)       { return m_vtbl->SetData     (CAST2VARPTR (this), lpData, cbSize);    }
  GXBOOL    Variable::GetData     (GXLPVOID lpData, GXUINT cbSize) GXCONST{ return m_vtbl->GetData     (CAST2VARPTRC(this), lpData, cbSize);    }

  //////////////////////////////////////////////////////////////////////////
  //Exception_...

  // 基本类型访问异常
  template<typename _TChar>
  GXBOOL Exception_SetAsStringT(VarImpl* pThis, const _TChar* szString)
  {
    CLOG("DataPool: Can not set this variable as string.\n");
    return FALSE;
  }

  //GXBOOL Exception_SetAsStringA(VarImpl* pThis,GXLPCSTR szString)
  //{
  //  CLOG("DataPool: Can not set this variable as string.\n");
  //  return FALSE;
  //}

  template<typename _TChar>
  GXBOOL Exception_ParseT(VarImpl* pThis, const _TChar* szString, GXUINT length)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  //GXBOOL Exception_ParseA(VarImpl* pThis,GXLPCSTR szString, GXUINT length)
  //{
  //  CLOG_ERROR("%s exception\n", __FUNCTION__);
  //  return FALSE;
  //}

  float Exception_ToFloat(GXCONST VarImpl* pThis )
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return 0;
  }

  u32 Exception_ToInteger(GXCONST VarImpl* pThis )
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  u64 Exception_ToInt64(GXCONST VarImpl* pThis )
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  GXBOOL Exception_SetAsFloat(VarImpl* pThis, float val)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }
  
  GXBOOL Exception_SetAsInteger(VarImpl* pThis, u32 val)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  GXBOOL Exception_SetAsInt64(VarImpl* pThis, u64 val)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  // 结构体访问异常
  Variable Exception_GetMember(GXCONST VarImpl* pThis, GXLPCSTR szName)
  {
    CLOG_ERROR("DataPool exception: Can not get member (\"%s\") from a unary type(\"%s\").\r\n", szName, pThis->GetName());
    return Variable();
  }

  // 数组访问异常
  Variable Exception_GetIndex(GXCONST VarImpl* pThis, int nIndex)
  {
    return Variable();
  }

  Variable Exception_NewBack(VarImpl* pThis, GXUINT nIncrease)
  {
    return Variable();
  }

  GXBOOL Exception_Remove(VarImpl* pThis, GXUINT nIndex, GXUINT nCount)
  {
    return FALSE;
  }


  // 对象访问异常
  GXBOOL Exception_Retain(VarImpl* pThis, GUnknown* pUnknown)
  {
    CLOG_ERROR("DataPool exception: Can not retain \"%s\" as \"object\" type.\r\n", pThis->GetTypeName());
    return FALSE;
  }

  GXBOOL Exception_Query(GXCONST VarImpl* pThis, GUnknown** ppUnknown)
  {
    CLOG_ERROR("DataPool exception: Can not query \"%s\" as \"object\" type.\r\n", pThis->GetTypeName());
    return FALSE;
  }

  GXBOOL Exception_GetData(GXCONST VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  GXBOOL Exception_SetData(VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize)
  {
    CLOG_ERROR("%s exception\n", __FUNCTION__);
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL Object_Retain(VarImpl* pThis, GUnknown* pUnknown)
  {
    // pUnknown 不能等于 m_pDataPool 否则永远也不会释放
    ASSERT(pThis->InlGetVDD()->GetTypeCategory() == T_OBJECT);
    if( ! pThis->IsSamePool((DataPool*)pUnknown))
    {
      InlSetNewObjectT(*(GUnknown**)pThis->GetPtr(), pUnknown);
      THIS_IMPULSE_DATA_CHANGE;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL Object_Query(GXCONST VarImpl* pThis, GUnknown** ppUnknown)
  {
    ASSERT(pThis->InlGetVDD()->GetTypeCategory() == T_OBJECT);

    *ppUnknown = *((GUnknown**)pThis->GetPtr());
    if(*ppUnknown) {
      (*ppUnknown)->AddRef();
    }
    return TRUE;
  }

  GXUINT Unary_GetSize(GXCONST VarImpl* pThis)
  {
    return pThis->InlGetVDD()->TypeSize();
  }

  GXUINT StaticArray_GetSize(GXCONST VarImpl* pThis)
  {
    return pThis->InlGetVDD()->GetSize();
  }

  GXUINT DynamicArray_GetSize(GXCONST VarImpl* pThis)
  {
    return (GXUINT)pThis->InlGetBufferObj()->GetSize();
  }

  GXUINT Unary_GetLength(GXCONST VarImpl* pThis)
  {
    return 1;
  }

  Variable Array_GetIndex(GXCONST VarImpl* pThis, int nIndex)
  {
    ASSERT(( ! pThis->InlGetVDD()->IsDynamicArray()) || pThis->GetOffset() == 0);
    Variable val;
    if((GXUINT)nIndex < pThis->GetLength())
    {
      pThis->InlSetupUnary(nIndex, &val);
    }
    return val;
  }

  Variable Struct_GetMember(GXCONST VarImpl* pThis, GXLPCSTR szName)
  {
    Variable var;
    if(GXSUCCEEDED(pThis->InlQuery(szName, &var))){
      return var;
    }
    return Variable();
  }

  //////////////////////////////////////////////////////////////////////////

  clStringA Primary_ToStringA(GXCONST VarImpl* pThis)
  {
    DataPoolVariable::LPCVD pVDD = pThis->InlGetVDD();
    clStringA str = "(null)";
    if(pVDD != NULL)
    {
      //GXLPCVOID pData = pThis->GetPtr();
      switch(pThis->InlGetCategory())
      {
      case T_BYTE:
        str.Format("%u", pThis->ToInteger());
        break;
      case T_WORD:
        str.Format("%u", pThis->ToInteger());
        break;
      case T_DWORD:
        str.Format("%u", pThis->ToInteger());
        break;
      case T_QWORD:
        str.Clear();
        str.AppendUInt64(pThis->ToInteger64());
        break;
      case T_SBYTE:
        str.Format("%d", pThis->ToInteger());
        break;
      case T_SWORD:
        str.Format("%d", pThis->ToInteger());
        break;
      case T_SDWORD:
        str.Format("%d", pThis->ToInteger());
        break;
      case T_SQWORD:
        str.Clear();
        str.AppendInteger64(pThis->ToInteger64());
        break;
      case T_FLOAT:
        str.Clear();
        str.AppendFloat(pThis->ToFloat());
        break;
      default:
        CLBREAK;
      }
    }
    return str;
  }

  clStringW Primary_ToStringW(GXCONST VarImpl* pThis)
  {
    return clStringW(Primary_ToStringA(pThis));
  }

  template<typename _TChar>
  GXBOOL Primary_ParseT(VarImpl* pThis, const _TChar* szString, GXUINT length)
  {
    DataPoolVariable::LPCVD pVDD = pThis->InlGetVDD();
    if(pVDD != NULL)
    {
      if(length == 0) {
        // clstd::xtou 和 clstd::xtoi 输入-1作为长度才是表示字符串‘\0’结尾
        length = -1;
      }

      //GXLPCVOID pData = pThis->GetPtr();
      switch(pThis->InlGetCategory())
      {
      case T_BYTE:
      case T_WORD:
      case T_DWORD:
        return pThis->Set((u32)clstd::xtou(szString, 10, length));
        //return pThis->Set((u32)clstd::xtou(szString, 10, length));
        //return pThis->Set((u32)clstd::xtou(szString, 10, length));
      case T_QWORD:
        return pThis->Set((u64)clstd::_xstrtou<u64>(szString, 10, length));

      case T_SBYTE:
      case T_SWORD:
      case T_SDWORD:
        return pThis->Set((i32)clstd::xtoi(szString, 10, length));
        //return pThis->Set((i32)clstd::xtoi(szString, 10, length));
        //return pThis->Set((i32)clstd::xtoi(szString, 10, length));
      case T_SQWORD:
        return pThis->Set((i64)clstd::_xstrtoi<i64>(szString, 10, length));
      case T_FLOAT:
        ASSERT(length == -1);
        return pThis->Set((float)clstd::_xstrtof(szString));
      default:
        CLBREAK;
      }
    }
    return FALSE;
  }

  //GXBOOL Primary_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  //{
  //  return Primary_ParseT(pThis, szString, length);
  //}

  //GXBOOL Primary_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  //{
  //  return Primary_ParseT(pThis, szString, length);
  //}

  float Primary_ToFloat(GXCONST VarImpl* pThis )
  {
    return (pThis->InlGetCategory() == T_FLOAT)
      ? *(float*)pThis->GetPtr() : 0.0f;
  }

  GXBOOL Primary_SetAsFloat(VarImpl* pThis, float val)
  {
    if(pThis->InlGetCategory() == T_FLOAT) {
      *(float*)pThis->GetPtr() = val;
#ifdef ENABLE_DATAPOOL_WATCHER
      pThis->InlGetDataPool()->ImpluseByVariable(DATACT_Change, *pThis, 0, FALSE);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      return TRUE;
    }
    return FALSE;
  }

  u32 Primary_ToInteger(GXCONST VarImpl* pThis)
  {
    GXLPCVOID pData = pThis->GetPtr();
    switch(pThis->InlGetCategory())
    {
    case T_BYTE:
    case T_SBYTE:
      return (u32)*(u8*)pData;
    case T_WORD:
    case T_SWORD:
      return (u32)*(u16*)pData;
    case T_DWORD:
    case T_SDWORD:
      return *(u32*)pData;
    case T_FLOAT:
      return (s32)(*(float*)pData);
    case T_ENUM:
      return (*(DataPool::Enum*)pData);
    case T_FLAG:
      return (*(DataPool::Flag*)pData);
    default:
      CLBREAK;
    }
    return 0;
  }

  GXBOOL Primary_SetAsInteger(VarImpl* pThis, u32 val)
  {
    GXLPCVOID pData = pThis->GetPtr();
    switch(pThis->InlGetCategory())
    {
    case T_BYTE:
    case T_SBYTE:
      *(u8*)pData = (u8)val;
      break;
    case T_WORD:
    case T_SWORD:
      *(u16*)pData = (u16)val;
      break;
    case T_DWORD:
    case T_SDWORD:
      *(u32*)pData = val;
      break;
    case T_ENUM:
      *(DataPool::Enum*)pData = val;
      break;
    case T_FLAG:
      *(DataPool::Flag*)pData = val;
      break;
    default:
      CLBREAK;
      return FALSE;
    }
    THIS_IMPULSE_DATA_CHANGE;
    return TRUE;
  }

  u64 Primary_ToInt64(GXCONST VarImpl* pThis )
  {
    GXLPCVOID pData = pThis->GetPtr();
    switch(pThis->InlGetCategory())
    {
    case T_QWORD:
    case T_SQWORD:
      return *(u64*)pData;
    default:
      CLBREAK;
    }
    return 0;
  }

  GXBOOL Primary_SetAsInt64(VarImpl* pThis, u64 val)
  {
    GXLPCVOID pData = pThis->GetPtr();
    switch(pThis->InlGetCategory())
    {
    case T_QWORD:
    case T_SQWORD:
      *(u64*)pData = val;
      break;
    default:
      CLBREAK;
      return FALSE;
    }
#ifdef ENABLE_DATAPOOL_WATCHER
    pThis->InlGetDataPool()->ImpluseByVariable(DATACT_Change, *pThis, 0, FALSE);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return TRUE;
  }

  GXBOOL Primary_GetData(GXCONST VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize)
  {
    memcpy(lpData, pThis->GetPtr(), pThis->GetSize() < cbSize ? pThis->GetSize() : cbSize);
    return TRUE;
  }
  GXBOOL Primary_SetData(VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize)
  {
    memcpy(pThis->GetPtr(), lpData, pThis->GetSize() < cbSize ? pThis->GetSize() : cbSize);
    THIS_IMPULSE_DATA_CHANGE;
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  
  template<class _TString>
  GXBOOL Enum_SetAsStringT(VarImpl* pThis, typename _TString::LPCSTR szText)
  {
    GXINT n;
    DataPool::Enum* pEnum = (DataPool::Enum*)pThis->GetPtr();
    if(IsNumericT(szText, -1, &n)) {
      *pEnum = n;
      return TRUE;
    }
    else {
      clStringA str(szText);
      STATIC_ASSERT(sizeof(clStringA::TChar) == sizeof(((DataPool::LPCSTR)0)[0]));  // 防止str与sDesc.Name类型不一致而造成编码反复转换导致的性能问题
      return pThis->InlFindEnumFlagValue(str, reinterpret_cast<DataPool::EnumFlag*>(pEnum));
    }
  }

  template<class _TString>
  GXBOOL Enum_ParseT(VarImpl* pThis, typename _TString::LPCSTR szText, GXUINT length)
  {
    if(length == 0 || szText[length] == '\0') {
      return Enum_SetAsStringT<_TString>(pThis, szText);
    }
    else
    {
      _TString str(szText, length);
      return Enum_SetAsStringT<_TString>(pThis, str);
    }
  }

  //GXBOOL Enum_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  //{
  //  return Enum_SetAsStringT(pThis, szString)
  //}

  //GXBOOL Enum_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  //{
  //  CLBREAK; // TODO: 实现这个!
  //  return FALSE;
  //}

  template<class _TString>
  _TString Enum_ToStringT(GXCONST VarImpl* pThis)
  {
    _TString str;
    DataPool::Enum e;
    //auto StringBase = pThis->InlStringBase();
    GXBOOL bval = pThis->ForEachEnum(e, [&str](const DataPool::ENUM_DESC* pDesc, DataPool::Enum eValue) -> GXBOOL 
    {
      if(pDesc->Value == eValue) {
        str = (pDesc->GetName());
        return FALSE;
      }
      return TRUE;
    });

    return bval ? str : _TString(e);
  }

  //clStringW Enum_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  return Enum_ToStringT<clStringW>(pThis);
  //}

  //clStringA Enum_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  return Enum_ToStringT<clStringA>(pThis);
  //}

  //GXBOOL Enum_SetAsStringW(VarImpl* pThis, GXLPCWSTR szString)
  //{
  //  return Enum_SetAsStringT(pThis, szString);
  //}

  //GXBOOL Enum_SetAsStringA(VarImpl* pThis, GXLPCSTR szString)
  //{
  //  return Enum_SetAsStringT(pThis, szString);
  //}

  //////////////////////////////////////////////////////////////////////////

  template<class _TString>
  _TString Flag_ToStringT(GXCONST VarImpl* pThis)
  {
    const auto nCount = pThis->InlGetVDD()->GetTypeDesc()->nMemberCount;
    //const auto StringBase = pThis->InlStringBase();
    DataPool::Flag dwFlags = *(DataPool::Flag*)pThis->GetPtr();
    DataPool::Flag dwOriFlags = dwFlags;

    // 如果空值直接返回
    if(dwFlags == 0) {
      return _TString(dwFlags);
    }

    _TString str;
    for (GXUINT i = 0; i < nCount; i++)
    {
      DataPool::LPCED pDesc = pThis->InlGetEnum(i);
      if(TEST_FLAGS_ALL(dwFlags, pDesc->Value)) {
        str += pDesc->GetName();
        str += '|';
        RESET_FLAG(dwFlags, pDesc->Value);
        if( ! dwFlags) {
          break;
        }
      }
    }

    // 如果掩码没有消除所有位，则返回数值
    if(dwFlags) {
      return _TString(dwOriFlags);
    }
    else if(str.EndsWith('|')) {
      str.TrimRight('|');
    }
    return str;
  }

  GXBOOL Flag_ParseString(VarImpl* pThis, DataPool::LPCSTR szText, DataPool::Flag* pFlag)
  {
    clstd::StringCutter<clStringA> scut(szText);
    clStringA str;
    GXBOOL bval = TRUE;
    DataPool::Flag flag;
    *pFlag = 0;

    while( ! scut.IsEndOfString())
    {
      scut.Cut(str, '|');
      if(pThis->InlFindEnumFlagValue(str, &flag)) {
        *pFlag |= (flag);
      }
      else {
        bval = FALSE;
      }
    }

    return bval;
  }

  template<class _TString>
  GXBOOL Flag_SetAsStringT(VarImpl* pThis, typename _TString::LPCSTR szText)
  {
    GXINT n;
    DataPool::Flag* pFlag = (DataPool::Flag*)pThis->GetPtr();
    if(IsNumericT(szText, -1, &n)) {
      *pFlag = n;
      return TRUE;
    }
    else {
      STATIC_ASSERT(sizeof(clStringA::TChar) == sizeof(((DataPool::LPCSTR)0)[0]));  // 防止str与sDesc.Name类型不一致而造成编码反复转换导致的性能问题
      return Flag_ParseString(pThis, clStringA(szText), pFlag);
    }
  }

  template<class _TString>
  GXBOOL Flag_ParseT(VarImpl* pThis, typename _TString::LPCSTR szText, GXUINT length)
  {
    if(length == 0 || szText[length] == '\0')
    {
      return Flag_SetAsStringT<_TString>(pThis, szText);
    }
    else {
      _TString str(szText, length);
      return Flag_SetAsStringT<_TString>(pThis, str);
    }
  }

  //GXBOOL Flag_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  //{
  //  CLBREAK; // TODO: 实现这个!
  //  return FALSE;
  //}

  //GXBOOL Flag_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  //{
  //  CLBREAK; // TODO: 实现这个!
  //  return FALSE;
  //}

  //clStringW Flag_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  return Flag_ToStringT<clStringW>(pThis);
  //}

  //clStringA Flag_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  return Flag_ToStringT<clStringA>(pThis);
  //}

  //GXBOOL Flag_SetAsStringW(VarImpl* pThis, GXLPCWSTR szString)
  //{
  //  CLBREAK; // TODO: 实现这个!
  //  return FALSE;
  //}

  //GXBOOL Flag_SetAsStringA(VarImpl* pThis, GXLPCSTR szString)
  //{
  //  CLBREAK; // TODO: 实现这个!
  //  return FALSE;
  //}

  //////////////////////////////////////////////////////////////////////////

  template<class _TString>
  GXBOOL String_ParseT(VarImpl* pThis, typename _TString::LPCSTR szText, GXUINT length)
  {
    //ASSERT(length == 0); // 没实现非0长度的处理
    if(length == 0 || szText[length] == '\0') {
      pThis->Set(szText);
    }
    else {
      _TString str(szText, length);
      pThis->Set(str);
    }
    return TRUE;
  }

  //GXBOOL String_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  //{
  //  ASSERT(length == 0); // 没实现非0长度的处理
  //  if(length == 0 || szString[length] == 0) {
  //    pThis->Set(szString);
  //  }
  //  return TRUE;
  //}

  //GXBOOL String_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  //{
  //  ASSERT(length == 0); // 没实现非0长度的处理
  //  pThis->Set(szString);
  //  return TRUE;
  //}

  template<class _TRetString, class _TStoString> // 返回字符串类型和自身储存的字符串类型
  _TRetString String_ToStringT(GXCONST VarImpl* pThis)
  {
    GXLPBYTE pStringData = ((GXBYTE*)pThis->InlGetBufferPtr() + pThis->GetOffset());
    if((*(void**)pStringData) == NULL) {
      return "";
    }
    else if(pThis->IsReadOnly()) {
      return _TRetString(*(_TStoString::LPCSTR*)((GXBYTE*)pStringData));
    }
    return _TRetString(*(_TStoString*)((GXBYTE*)pStringData));
  }

  //clStringW String_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  GXLPBYTE pStringData = ((GXBYTE*)pThis->InlGetBufferPtr() + pThis->GetOffset());
  //  return *(void**)pStringData == NULL ? L"" : *(clStringW*)((GXBYTE*)pStringData);
  //}

  template<typename T_LPCSTR, class _TStoString>
  GXBOOL String_SetAsStringT(VarImpl* pThis, T_LPCSTR szString)
  {
//    GXLPBYTE pStringData = pThis->InlGetBufferPtr() + pThis->GetOffset();
//    if(*(void**)(pStringData) == NULL) {
//      new(pStringData) _TStoString(szString);
//#ifdef _DEBUG
//      pThis->InlIncreateStringRefCount();
//#endif // #ifdef _DEBUG
//    }
//    else {
//      *(_TStoString*)((GXBYTE*)pStringData) = szString;
//    }
    
    if(pThis->SetAsString<T_LPCSTR, _TStoString>(szString)) {
      THIS_IMPULSE_DATA_CHANGE;
      return TRUE;
    }
    return FALSE;
  }

  //GXBOOL String_SetAsStringW(VarImpl* pThis, GXLPCWSTR szString)
  //{
  //  GXLPBYTE pStringData = pThis->InlGetBufferPtr() + pThis->GetOffset();
  //  if(*(void**)(pStringData) == NULL) {
  //    new(pStringData) clStringW(szString);
  //    pThis->InlIncreateStringRefCount();
  //  }
  //  else {
  //    *(clStringW*)((GXBYTE*)pStringData) = szString;
  //  }

  //  THIS_IMPULSE_DATA_CHANGE
  //  return TRUE;
  //}

  //clStringA String_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  return clStringA(*(clStringW*)((GXBYTE*)pThis->InlGetBufferPtr() + pThis->GetOffset()));
  //}

  //GXBOOL String_SetAsStringA(VarImpl* pThis, GXLPCSTR szString)
  //{
  //  GXLPBYTE pStringData = pThis->InlGetBufferPtr() + pThis->GetOffset();
  //  if(*(void**)(pStringData) == NULL) {
  //    new((GXBYTE*)pStringData) clStringW;
  //    pThis->InlIncreateStringRefCount();
  //  }

  //  *(clStringW*)(pStringData) = szString;
  //  THIS_IMPULSE_DATA_CHANGE
  //  return TRUE;
  //}

  //////////////////////////////////////////////////////////////////////////

  template<class _TString>
  _TString Struct_ToStringT(GXCONST VarImpl* pThis)
  {
    static typename _TString::TChar szStruct[] = {'s','t','r','u','c','t',' ','\0'};
    _TString str = szStruct;
    str.Append(pThis->GetName());
    return str;
  }

  //clStringW Struct_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  clStringW str = L"struct ";
  //  str.Append(pThis->GetName());
  //  return str;
  //}

  //clStringA Struct_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  clStringA str = "struct ";
  //  str.Append(pThis->GetName());
  //  return str;
  //}

  //////////////////////////////////////////////////////////////////////////
  template<class _TString>
  _TString Object_ToStringT(GXCONST VarImpl* pThis)
  {
    //clStringW str = L"object ";
    static typename _TString::TChar szObject[] = {'o','b','j','e','c','t',' ','\0'};
    _TString str(szObject);
    str.Append(pThis->GetName());
    return str;
  }

  //clStringW Object_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  clStringW str = L"object ";
  //  str.Append(pThis->GetName());
  //  return str;
  //}

  //clStringA Object_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  clStringA str = "object ";
  //  str.Append(pThis->GetName());
  //  return str;
  //}

  //////////////////////////////////////////////////////////////////////////

  GXBOOL Struct_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  {
    // TODO: 暂时不支持, 将来考虑支持"键值1=数据1;键值2=数据2;..."这种类型的数据解析
    return FALSE;
  }

  GXBOOL Struct_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  {
    // TODO: 暂时不支持, 将来考虑支持"键值1=数据1;键值2=数据2;..."这种类型的数据解析
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXUINT StaticArray_GetLength(GXCONST VarImpl* pThis )
  {
    return pThis->InlGetVDD()->nCount;
  }

  template<class _TString>
  _TString StaticArray_ToStringT(GXCONST VarImpl* pThis)
  {
    static typename _TString::TChar szFmt[] = {'[','%','d',']','\0'};
    _TString str = pThis->GetTypeName();
    str.AppendFormat(szFmt, pThis->GetLength());
    return str;
  }

  //clStringW StaticArray_ToStringW(GXCONST VarImpl* pThis)
  //{
  //  clStringW str = pThis->GetTypeName();
  //  str.AppendFormat(L"[%d]", pThis->GetLength());
  //  return str;
  //}

  //clStringA StaticArray_ToStringA(GXCONST VarImpl* pThis)
  //{
  //  clStringA str = pThis->GetTypeName();
  //  str.AppendFormat("[%d]", pThis->GetLength());
  //  return str;
  //}

  GXBOOL StaticArray_ParseW(VarImpl* pThis, GXLPCWSTR szString, GXUINT length)
  {
    // TODO: 暂时不支持, 将来考虑支持"{键值1=数据1;键值2=数据2;...}..."这种类型的数据解析
    return FALSE;
  }

  GXBOOL StaticArray_ParseA(VarImpl* pThis, GXLPCSTR szString, GXUINT length)
  {
    // TODO: 暂时不支持, 将来考虑支持"{键值1=数据1;键值2=数据2;...}..."这种类型的数据解析
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXUINT DynamicArray_GetLength(GXCONST VarImpl* pThis)
  {
    DataPoolVariable::LPCVD pVdd = pThis->InlGetVDD();
    //clBuffer* pElementBuffer = pVdd->CreateAsBuffer(pThis->InlGetBufferPtr(), 0);
    clBufferBase* pBuffer = pThis->InlGetBufferObj();
    return pBuffer == NULL ? 0 : ((GXUINT)pBuffer->GetSize() / pVdd->TypeSize());
  }

  Variable DynamicArray_NewBack(VarImpl* pThis, GXUINT nIncrease)
  {
    const GXUINT nIndex = pThis->GetLength();
    Variable val;
    if(nIncrease == 0) {
      pThis->InlSetupUnary(nIndex - 1, &val);
    }
    else {
      DataPoolVariable::LPCVD pVdd = pThis->InlGetVDD();
      //ASSERT(pVdd->nOffset == pThis->InlGetOffset());
      ASSERT(pThis->GetOffset() == 0);
      DataPoolArray* pArrayBuffer = static_cast<DataPoolArray*>(pThis->InlGetBufferObj());//pThis->InlGetVDD()->CreateAsBuffer(pThis->InlGetBufferPtr(), 0);
      const GXUINT nPrevSize = (GXUINT)pArrayBuffer->GetSize();
      pArrayBuffer->Resize(nPrevSize + pVdd->TypeSize() * nIncrease, TRUE);
      pThis->InlSetupUnary(nIndex, &val);
#ifdef ENABLE_DATAPOOL_WATCHER
      pThis->InlGetDataPool()->ImpluseByVariable(DATACT_Insert, val, 0, FALSE);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    }
    return val;
  }

  GXBOOL DynamicArray_Remove(VarImpl* pThis, GXUINT nIndex, GXUINT nCount)
  {
    DataPoolVariable::LPCVD pVdd = pThis->InlGetVDD();
    const GXUINT nLength = pThis->GetLength();

    if(
      (nLength == 0) || 
      ( (nIndex != (GXUINT)-1) && 
        (nCount == 0 || nIndex >= nLength || nIndex + nCount > nLength)
      )
      )
    {
      return FALSE;
    }
    //else if((nIndex != (GXUINT)-1))
    //if(nLength == 0 || nCount == 0 ||               // 删除本身长度就是0的数组
    //  (nIndex != (GXUINT)-1 && nIndex >= nLength))  // 不合法的index
    //{ 
    //  return FALSE;
    //}

    //ASSERT(nCount == 1); // 没实现大于1的情况，这里记一下

    DataPoolArray* pArrayBuffer = static_cast<DataPoolArray*>(pThis->InlGetBufferObj());
    
    THIS_IMPULSE_DATA(DATACT_Deleting, nIndex);

    if(nIndex == (GXUINT)-1)     // 全部删除
    {
      pThis->InlCleanupArray(pVdd, pThis->GetPtr(), nLength);
      pArrayBuffer->Resize(0, FALSE);
    }
    else    // 从指定位置删除
    {
      pThis->InlCleanupArray(pVdd, (GXLPBYTE)pThis->GetPtr() + pVdd->TypeSize() * nIndex, nCount);
      pArrayBuffer->Replace(pVdd->TypeSize() * nIndex, pVdd->TypeSize() * nCount, NULL, 0);
    }
    
    THIS_IMPULSE_DATA(DATACT_Deleted, nIndex);
    return TRUE;
  }

  clStringW DynamicArray_ToStringW(GXCONST VarImpl* pThis)
  {
    return clStringW(pThis->GetTypeName());
  }

  clStringA DynamicArray_ToStringA(GXCONST VarImpl* pThis)
  {
    return pThis->GetTypeName();
  }

  template<class _TString, class _Fn>
  GXBOOL DynamicArray_ParseT(VarImpl* pThis, typename _TString::LPCSTR szText, GXUINT length, _Fn fn)
  {
    // 参考 StaticArray_Parse
    if(pThis->InlIsPrimaryType())
    {
      clstd::StringCutter<_TString> sc(szText, length);
      _TString str;

      while( ! sc.IsEndOfString())
      {
        auto newVar = pThis->NewBack();
        VarImpl var = *(VarImpl*)&newVar;
        sc.Cut(str, ',');
        if(var.InlGetVtbl() == NULL || ! fn(&var, str, 0)) {
          return FALSE;
        }
      }
      return TRUE;
    }
    else {
      // object 不支持解析
      // struct 没实现解析
      CLBREAK;
    }
    return FALSE;
  }

  GXBOOL DynamicArray_ParseW(VarImpl* pThis, GXLPCWSTR szText, GXUINT length)
  {
    return DynamicArray_ParseT<clStringW>(pThis, szText, length, 
      [](VarImpl* pVar, clStringW::LPCSTR szText, GXUINT length)->GXBOOL{
        return pVar->InlGetVtbl()->ParseW(pVar, szText, length);
    });
  }

  GXBOOL DynamicArray_ParseA(VarImpl* pThis,GXLPCSTR szText, GXUINT length)
  {
    return DynamicArray_ParseT<clStringA>(pThis, szText, length, 
      [](VarImpl* pVar, clStringA::LPCSTR szText, GXUINT length)->GXBOOL{
        return pVar->InlGetVtbl()->ParseA(pVar, szText, length);
    });
  }

  //////////////////////////////////////////////////////////////////////////

  namespace Implement
  {
    Variable::VTBL s_PrimaryVtbl2[] = {{
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Primary_ParseW          , // ParseW
      Primary_ParseA          , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Primary_ToFloat         , // ToFloat  
      Primary_ToStringW       , // GetAsStringW
      Primary_ToStringA       , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Primary_SetAsFloat      , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Primary_SetData         , // SetData     
    },{ // const
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Primary_ToFloat         , // ToFloat  
      Primary_ToStringW       , // GetAsStringW
      Primary_ToStringA       , // GetAsStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData
      Exception_SetData       , // SetData
    }};

    Variable::VTBL s_EnumVtbl2[] = {{
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Enum_ParseT<clStringW>  , // ParseW
      Enum_ParseT<clStringA>  , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Enum_ToStringW          , // GetAsStringW
      Enum_ToStringA          , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Enum_SetAsStringW       , // SetAsStringW
      Enum_SetAsStringA       , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData
      Primary_SetData         , // SetData
    },{
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Enum_ToStringW          , // GetAsStringW
      Enum_ToStringA          , // GetAsStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Exception_SetData       , // SetData     
    }};

    Variable::VTBL s_FlagVtbl2[] = {{
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Flag_ParseW             , // ParseW
      Flag_ParseA             , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Flag_ToStringW          , // GetAsStringW
      Flag_ToStringA          , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Flag_SetAsStringW       , // SetAsStringW
      Flag_SetAsStringA       , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Primary_SetData         , // SetData     
    },{
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Flag_ToStringW          , // GetAsStringW
      Flag_ToStringA          , // GetAsStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Exception_SetData       , // SetData     
    }};

    Variable::VTBL s_StringVtbl2[] = {{
      Unary_GetSize           , // GetSize      
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack      
      Exception_Remove        , // Remove       
      String_ParseT<clStringW>, // Parse        
      String_ParseT<clStringA>, // Parse        
      Exception_ToInteger     , // ToInteger 
      Exception_ToInt64       , // ToInt64   
      Exception_ToFloat       , // ToFloat   
      String_ToStringW        , // ToStringW 
      String_ToStringA        , // ToStringA 
      Exception_SetAsInteger  , // SetAsInteger 
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      String_SetAsStringW     , // SetAsStringW 
      String_SetAsStringA     , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData      
      Exception_SetData       , // SetData      
    },{
      Unary_GetSize           , // GetSize      
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack      
      Exception_Remove        , // Remove       
      Exception_ParseW        , // Parse        
      Exception_ParseA        , // Parse        
      Exception_ToInteger     , // ToInteger 
      Exception_ToInt64       , // ToInt64   
      Exception_ToFloat       , // ToFloat   
      String_ToStringW        , // ToStringW 
      String_ToStringA        , // ToStringA 
      Exception_SetAsInteger  , // SetAsInteger 
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      Exception_SetAsStringW  , // SetAsStringW 
      Exception_SetAsStringA  , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    }};

    Variable::VTBL s_StructVtbl2[] = {{
      Unary_GetSize           , // GetSize
      Struct_GetMember        , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Struct_ParseW           , // ParseW
      Struct_ParseA           , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Struct_ToStringW        , // ToStringW
      Struct_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      Exception_SetAsStringW  , // SetAsStringW 
      Exception_SetAsStringA  , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    },{
      Unary_GetSize           , // GetSize
      Struct_GetMember        , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Struct_ToStringW        , // ToStringW
      Struct_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      Exception_SetAsStringW  , // SetAsStringW 
      Exception_SetAsStringA  , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    }};

    Variable::VTBL s_ObjectVtbl2[] = {{
      Unary_GetSize           , // GetSize
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Object_ToStringW        , // ToStringW
      Object_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Object_Retain           ,
      Object_Query            ,
      Primary_GetData         , // GetData
      Primary_SetData         , // SetData
    },{
      Unary_GetSize           , // GetSize
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Object_ToStringW        , // ToStringW
      Object_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Object_Query            ,
      Primary_GetData         , // GetData
      Exception_SetData       , // SetData
    }};

    Variable::VTBL s_StaticArrayVtbl2[] = {{
      StaticArray_GetSize     , // GetSize     
      Exception_GetMember     , // GetMember   
      Array_GetIndex          , // GetIndex    
      StaticArray_GetLength   , // GetLength   
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      StaticArray_ParseW      , // ParseW
      StaticArray_ParseA      , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      StaticArray_ToStringW   , // ToStringW
      StaticArray_ToStringA   , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64  
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    },{
      StaticArray_GetSize     , // GetSize     
      Exception_GetMember     , // GetMember   
      Array_GetIndex          , // GetIndex    
      StaticArray_GetLength   , // GetLength   
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      StaticArray_ToStringW   , // ToStringW
      StaticArray_ToStringA   , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64  
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    }};

    Variable::VTBL s_DynamicArrayVtbl2[] = {{
      DynamicArray_GetSize    , // GetSize
      Exception_GetMember     , // GetMember
      Array_GetIndex          , // GetIndex
      DynamicArray_GetLength  , // GetLength
      DynamicArray_NewBack    , // NewBack
      DynamicArray_Remove     , // Remove
      DynamicArray_ParseW     , // ParseW
      DynamicArray_ParseA     , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      DynamicArray_ToStringW  , // ToStringW
      DynamicArray_ToStringA  , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    },{
      DynamicArray_GetSize    , // GetSize
      Exception_GetMember     , // GetMember
      Array_GetIndex          , // GetIndex
      DynamicArray_GetLength  , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      DynamicArray_ToStringW  , // ToStringW
      DynamicArray_ToStringA  , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData
      Exception_SetData       , // SetData
    }};
//////////////////////////////////////////////////////////////////////////
    Variable::VTBL s_PrimaryVtbl = {
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Primary_ParseW          , // ParseW
      Primary_ParseA          , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Primary_ToFloat         , // ToFloat  
      Primary_ToStringW       , // GetAsStringW
      Primary_ToStringA       , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Primary_SetAsFloat      , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Primary_SetData         , // SetData     
    };

    Variable::VTBL s_EnumVtbl = {
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Enum_ParseT<clStringW>  , // ParseW
      Enum_ParseT<clStringA>  , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Enum_ToStringW          , // GetAsStringW
      Enum_ToStringA          , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Enum_SetAsStringW       , // SetAsStringW
      Enum_SetAsStringA       , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Primary_SetData         , // SetData     
    };

    Variable::VTBL s_FlagVtbl = {
      Unary_GetSize           , // GetSize     
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      Flag_ParseW             , // ParseW
      Flag_ParseA             , // ParseA
      Primary_ToInteger       , // ToInteger
      Primary_ToInt64         , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      Flag_ToStringW          , // GetAsStringW
      Flag_ToStringA          , // GetAsStringA
      Primary_SetAsInteger    , // SetAsInteger
      Primary_SetAsInt64      , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat  
      Flag_SetAsStringW       , // SetAsStringW
      Flag_SetAsStringA       , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Primary_GetData         , // GetData     
      Primary_SetData         , // SetData     
    };

    // 这个与 s_PrimaryVtbl 完全一致，所以合并了
    //Variable::VTBL s_DynamicPrimaryVtbl = {
    //  Unary_GetSize           , // GetSize      
    //  Exception_GetMember     , // GetMember    
    //  Exception_GetIndex      , // GetIndex     
    //  Unary_GetLength         , // GetLength    
    //  Exception_NewBack       , // NewBack     
    //  Exception_Remove        , // Remove      
    //  Primary_ParseW          , // ParseW
    //  Primary_ParseA          , // ParseA
    //  Primary_ToStringW       , // ToStringW
    //  Exception_SetAsStringW  , // SetAsStringW 
    //  Primary_ToStringA       , // ToStringA 
    //  Exception_SetAsStringA  , // SetAsStringA 
    //  Primary_ToFloat         , // ToFloat   
    //  Primary_SetAsFloat      , // SetAsFloat   
    //  Primary_ToInteger       , // ToInteger 
    //  Primary_SetAsInteger    , // SetAsInteger 
    //  Primary_ToInt64         , // ToInt64   
    //  Primary_SetAsInt64      , // SetAsInt64
    //  Exception_Retain        ,
    //  Exception_Query         ,
    //  Primary_GetData         , // GetData      
    //  Primary_SetData         , // SetData      
    //};

    Variable::VTBL s_StringVtbl = {
      Unary_GetSize           , // GetSize      
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack      
      Exception_Remove        , // Remove       
      String_ParseT<clStringW>, // Parse        
      String_ParseT<clStringA>, // Parse        
      Exception_ToInteger     , // ToInteger 
      Exception_ToInt64       , // ToInt64   
      Exception_ToFloat       , // ToFloat   
      String_ToStringW        , // ToStringW 
      String_ToStringA        , // ToStringA 
      Exception_SetAsInteger  , // SetAsInteger 
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      String_SetAsStringW     , // SetAsStringW 
      String_SetAsStringA     , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData      
      Exception_SetData       , // SetData      
    };

    Variable::VTBL s_StringAVtbl = {
      Unary_GetSize           , // GetSize      
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack      
      Exception_Remove        , // Remove       
      String_ParseT<clStringW>, // Parse        
      String_ParseT<clStringA>, // Parse        
      Exception_ToInteger     , // ToInteger 
      Exception_ToInt64       , // ToInt64   
      Exception_ToFloat       , // ToFloat   
      StringA_ToStringW       , // ToStringW 
      StringA_ToStringA       , // ToStringA 
      Exception_SetAsInteger  , // SetAsInteger 
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      StringA_SetAsStringW    , // SetAsStringW 
      StringA_SetAsStringA    , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData      
      Exception_SetData       , // SetData      
    };
    Variable::VTBL s_StructVtbl = {
      Unary_GetSize           , // GetSize      
      Struct_GetMember        , // GetMember    
      Exception_GetIndex      , // GetIndex     
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Struct_ParseW           , // ParseW
      Struct_ParseA           , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Struct_ToStringW        , // ToStringW
      Struct_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat   
      Exception_SetAsStringW  , // SetAsStringW 
      Exception_SetAsStringA  , // SetAsStringA 
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData      
      Exception_SetData       , // SetData      
    };

    //Variable::VTBL s_DynamicStructVtbl = {
    //  Unary_GetSize             , // GetSize     
    //  Struct_GetMember          , // GetMember   
    //  Exception_GetIndex        , // GetIndex    
    //  Unary_GetLength           , // GetLength   
    //  0                         , // NewBack     
    //  0                         , // Remove      
    //  Struct_ParseW             , // ParseW
    //  Struct_ParseA             , // ParseA
    //  Struct_ToStringW          , // ToStringW
    //  Exception_SetAsStringW    , // SetAsStringW
    //  Struct_ToStringA          , // ToStringA
    //  Exception_SetAsStringA    , // SetAsStringA
    //  0                         , // ToFloat  
    //  0                         , // SetAsFloat  
    //  0                         , // ToInteger
    //  0                         , // SetAsInteger
    //  0                         , // ToInt64  
    //  0                         , // SetAsInt64  
    //  Exception_Retain          ,
    //  Exception_Query           ,
    //  0                         , // GetData     
    //  0                         , // SetData     
    //};
    Variable::VTBL s_ObjectVtbl = {
      Unary_GetSize           , // GetSize
      Exception_GetMember     , // GetMember
      Exception_GetIndex      , // GetIndex
      Unary_GetLength         , // GetLength
      Exception_NewBack       , // NewBack
      Exception_Remove        , // Remove
      Exception_ParseW        , // ParseW
      Exception_ParseA        , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64
      Exception_ToFloat       , // ToFloat
      Object_ToStringW        , // ToStringW
      Object_ToStringA        , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64
      Exception_SetAsFloat    , // SetAsFloat
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Object_Retain           ,
      Object_Query            ,
      Primary_GetData         , // GetData
      Primary_SetData         , // SetData
    };

    //Variable::VTBL s_DynamicObjectVtbl = {
    //  Unary_GetSize       , // GetSize
    //  Exception_GetMember , // GetMember
    //  Exception_GetIndex  , // GetIndex
    //  Unary_GetLength     , // GetLength
    //  0                   , // NewBack
    //  0                   , // Remove
    //  Primary_ParseW      , // ParseW
    //  Primary_ParseA      , // ParseA
    //  Object_ToStringW    , // ToStringW
    //  Exception_SetAsString   , // SetAsStringW
    //  Object_ToStringA    , // ToStringA
    //  Base_SetAsStringA   , // SetAsStringA
    //  Primary_ToFloat     , // ToFloat
    //  Primary_SetAsFloat  , // SetAsFloat
    //  Primary_ToInteger   , // ToInteger
    //  Primary_SetAsInteger, // SetAsInteger
    //  Primary_ToInt64     , // ToInt64
    //  Primary_SetAsInt64  , // SetAsInt64
    //  Object_Retain       ,
    //  Object_Query        ,
    //  Primary_GetData     , // GetData
    //  Primary_SetData     , // SetData
    //};

    Variable::VTBL s_StaticArrayVtbl = {
      StaticArray_GetSize     , // GetSize     
      Exception_GetMember     , // GetMember   
      Array_GetIndex          , // GetIndex    
      StaticArray_GetLength   , // GetLength   
      Exception_NewBack       , // NewBack     
      Exception_Remove        , // Remove      
      StaticArray_ParseW      , // ParseW
      StaticArray_ParseA      , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      StaticArray_ToStringW   , // ToStringW
      StaticArray_ToStringA   , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64  
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData     
      Exception_SetData       , // SetData     
    };

    Variable::VTBL s_DynamicArrayVtbl = {
      DynamicArray_GetSize    , // GetSize     
      Exception_GetMember     , // GetMember   
      Array_GetIndex          , // GetIndex    
      DynamicArray_GetLength  , // GetLength   
      DynamicArray_NewBack    , // NewBack     
      DynamicArray_Remove     , // Remove      
      DynamicArray_ParseW     , // ParseW
      DynamicArray_ParseA     , // ParseA
      Exception_ToInteger     , // ToInteger
      Exception_ToInt64       , // ToInt64  
      Exception_ToFloat       , // ToFloat  
      DynamicArray_ToStringW  , // ToStringW
      DynamicArray_ToStringA  , // ToStringA
      Exception_SetAsInteger  , // SetAsInteger
      Exception_SetAsInt64    , // SetAsInt64  
      Exception_SetAsFloat    , // SetAsFloat  
      Exception_SetAsStringW  , // SetAsStringW
      Exception_SetAsStringA  , // SetAsStringA
      Exception_Retain        ,
      Exception_Query         ,
      Exception_GetData       , // GetData     
      Exception_SetData       , // SetData     
    };
  } // namespace Implement
} // namespace Marimo
