﻿#ifndef _HIDING_CODE_
#include "GrapX.h"
#include "GrapX.Hxx"

#include "clStringSet.h"
#include "clStaticStringsDict.h"

#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataPoolIterator.h"
#include "GrapX/GXKernel.h"

#include "DataPoolImpl.h"
#include "DataPoolBuildTime.h"

#include "GXStation.h"
//using namespace clstd;

#endif // #ifndef _HIDING_CODE_
// NX16B = not cross 16-byte boundary

#ifndef VARIABLE_DATAPOOL_OBJECT
# define VARIABLE_DATAPOOL_OBJECT this
#endif

#define GSIT_Variables (m_aGSIT)
#define GSIT_Members   (m_aGSIT + m_nNumOfVar)
#define GSIT_Enums     (m_aGSIT + m_nNumOfVar + m_nNumOfMember)
#define IS_VALID_NAME(_NAME)  (_NAME != NULL && clstd::strlenT(_NAME) > 0)

#define V_WRITE(_FUNC, _TIPS) if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
#define V_READ(_FUNC, _TIPS)  if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
#define SIZEOF_PTR          sizeof(void*)
#define SIZEOF_PTR32        sizeof(GXDWORD)
#define SIZEOF_PTR64        sizeof(GXQWORD)
#define TYPE_CHANGED_FLAG   0x80000000  // 类型扩展或者缩减时的标记，记在TYPE_DESC::Cate上，用后要清除!

#ifdef _DEBUG
# define INC_DBGNUMOFSTRING ++m_nDbgNumOfString
# define INC_DBGNUMOFARRAY  ++m_nDbgNumOfArray
#else
# define INC_DBGNUMOFSTRING
# define INC_DBGNUMOFARRAY
#endif // 

#ifdef _X86
# define ASSERT_X86(x)   ASSERT(x)
# define ASSERT_X64(x)
#elif defined(_X64)
# define ASSERT_X86(x)
# define ASSERT_X64(x)   ASSERT(x)
#else
# define ASSERT_X86(x)
# define ASSERT_X64(x)
#endif // #ifdef _X86

#ifndef _HIDING_CODE_
namespace Marimo
{
  typedef DataPoolVariable Variable;

  namespace DataPoolInternal
  {
    // 预置类型
#define DATAPOOL_VECTOR2_MEMBERLIST(_N) DATAPOOL_VARIABLE_DECLARATION c_##_N##2[] = { {#_N, "x"}, {#_N, "y"}, {NULL, NULL}, }
#define DATAPOOL_VECTOR3_MEMBERLIST(_N) DATAPOOL_VARIABLE_DECLARATION c_##_N##3[] = { {#_N, "x"}, {#_N, "y"}, {#_N, "z"}, {NULL, NULL}, }
#define DATAPOOL_VECTOR4_MEMBERLIST(_N) DATAPOOL_VARIABLE_DECLARATION c_##_N##4[] = { {#_N, "x"}, {#_N, "y"}, {#_N, "z"}, {#_N, "w"}, {NULL, NULL}, }

    DATAPOOL_VECTOR2_MEMBERLIST(int);
    DATAPOOL_VECTOR3_MEMBERLIST(int);
    DATAPOOL_VECTOR4_MEMBERLIST(int);
    DATAPOOL_VECTOR2_MEMBERLIST(bool);
    DATAPOOL_VECTOR3_MEMBERLIST(bool);
    DATAPOOL_VECTOR4_MEMBERLIST(bool);
    DATAPOOL_VECTOR2_MEMBERLIST(uint);
    DATAPOOL_VECTOR3_MEMBERLIST(uint);
    DATAPOOL_VECTOR4_MEMBERLIST(uint);
    DATAPOOL_VECTOR2_MEMBERLIST(float);
    DATAPOOL_VECTOR3_MEMBERLIST(float);
    DATAPOOL_VECTOR4_MEMBERLIST(float);

#undef DATAPOOL_VECTOR2_MEMBERLIST
#undef DATAPOOL_VECTOR3_MEMBERLIST
#undef DATAPOOL_VECTOR4_MEMBERLIST


    DATAPOOL_VARIABLE_DECLARATION c_float2x2[] = { {"float", "m", 0, 4}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float3x3[] = { {"float", "m", 0, 9}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float4x4[] = { {"float", "m", 0, 16}, {NULL, NULL}, };

    DATAPOOL_TYPE_DECLARATION c_TypeDefine[] =
    {
      {DataPoolTypeClass::Float,  "float"},
      {DataPoolTypeClass::Byte,   "BYTE"},
      {DataPoolTypeClass::Word,   "WORD"},
      {DataPoolTypeClass::DWord,  "DWORD"},
      {DataPoolTypeClass::QWord,  "QWORD"},
      {DataPoolTypeClass::Byte,   "unsigned_char"},
      {DataPoolTypeClass::Word,   "unsigned_short"},
      {DataPoolTypeClass::DWord,  "unsigned_int"},
      {DataPoolTypeClass::QWord,  "unsigned_longlong"},

      {DataPoolTypeClass::SByte,  "char"},
      {DataPoolTypeClass::SWord,  "short"},
      {DataPoolTypeClass::SDWord, "int"},
      {DataPoolTypeClass::SQWord, "longlong"},

      {DataPoolTypeClass::String,  "string"},
      {DataPoolTypeClass::StringA, "stringA"},
      {DataPoolTypeClass::Object,  "object"},

      {DataPoolTypeClass::Structure, "float2"  , c_float2},
      {DataPoolTypeClass::Structure, "float3"  , c_float3},
      {DataPoolTypeClass::Structure, "float4"  , c_float4},
      {DataPoolTypeClass::Structure, "float2x2", c_float2x2},
      {DataPoolTypeClass::Structure, "float3x3", c_float3x3},
      {DataPoolTypeClass::Structure, "float4x4", c_float4x4},
      {DataPoolTypeClass::Undefine, NULL},
    };

    DATAPOOL_VARIABLE_DECLARATION c_float2x2_NX16B[] = { {"float2", "v", 0, 2}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float2x3_NX16B[] = { {"float2", "v", 0, 3}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float2x4_NX16B[] = { {"float2", "v", 0, 4}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float3x2_NX16B[] = { {"float3", "v", 0, 2}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float3x3_NX16B[] = { {"float3", "v", 0, 3}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float3x4_NX16B[] = { {"float3", "v", 0, 4}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float4x2_NX16B[] = { {"float4", "v", 0, 2}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float4x3_NX16B[] = { {"float4", "v", 0, 3}, {NULL, NULL}, };
    DATAPOOL_VARIABLE_DECLARATION c_float4x4_NX16B[] = { {"float4", "v", 0, 4}, {NULL, NULL}, };    

    DATAPOOL_TYPE_DECLARATION c_TypeDefine_NX16B[] =
    {
      {DataPoolTypeClass::SDWord, "bool"},
      {DataPoolTypeClass::Float,  "float"},
      {DataPoolTypeClass::SDWord, "int"},
      {DataPoolTypeClass::DWord,  "uint"},

      {DataPoolTypeClass::String,  "string"},
      {DataPoolTypeClass::StringA, "stringA"},
      {DataPoolTypeClass::Object,  "object"},

      {DataPoolTypeClass::Structure, "float2"  , c_float2},
      {DataPoolTypeClass::Structure, "float3"  , c_float3},
      {DataPoolTypeClass::Structure, "float4"  , c_float4},
      {DataPoolTypeClass::Structure, "float2x2", c_float2x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float2x3", c_float2x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float2x4", c_float2x4_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float3x2", c_float3x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float3x3", c_float3x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float3x4", c_float3x4_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float4x2", c_float4x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float4x3", c_float4x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "float4x4", c_float4x4_NX16B, DataPoolPack::NotCross16BoundaryShort},

      {DataPoolTypeClass::Structure, "row_float2x2", c_float2x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float2x3", c_float3x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float2x4", c_float4x2_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float3x2", c_float2x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float3x3", c_float3x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float3x4", c_float4x3_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float4x2", c_float2x4_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float4x3", c_float3x4_NX16B, DataPoolPack::NotCross16BoundaryShort},
      {DataPoolTypeClass::Structure, "row_float4x4", c_float4x4_NX16B, DataPoolPack::NotCross16BoundaryShort},
      
      {DataPoolTypeClass::Undefine, NULL},
    };

    DataPoolPack CreationFlagsToMemberPack(GXDWORD dwFlags)
    {
      // 这个判断顺序与定义掩码有关，不能随便修改
      if(IS_MARK_NX16B(dwFlags)) {
        return DataPoolPack::NotCross16Boundary;
      }
      else if((dwFlags & DataPoolCreation_Align16) == DataPoolCreation_Align16) {
        return DataPoolPack::Align16;
      }
      else if((dwFlags & DataPoolCreation_Align8) == DataPoolCreation_Align8) {
        return DataPoolPack::Align8;
      }
      else if((dwFlags & DataPoolCreation_Align4) == DataPoolCreation_Align4) {
        return DataPoolPack::Align4;
      }
      return DataPoolPack::Compact;
    }

    GXUINT NX16BArrayOffset(GXUINT nTypeSize, GXUINT nElementIndex)
    {
      return ALIGN_16(nTypeSize) * nElementIndex;
    }

    GXUINT NX16BArrayOffset(GXDWORD dwFlags, GXUINT nTypeSize, GXUINT nElementIndex)
    {
      return IS_MARK_NX16B(dwFlags)
        ? NX16BArrayOffset(nTypeSize, nElementIndex)
        : nTypeSize * nElementIndex;
    }

    GXUINT NX16BArraySize(GXUINT nTypeSize, GXUINT nElementCount)
    {
      // a[n]数组定义中，a[0]~a[n-1]其实都要16字节对齐，a[n-1]只占用实际长度
      ASSERT(nElementCount > 0);
      return (ALIGN_16(nTypeSize) * (nElementCount - 1) + nTypeSize);
    }

    GXUINT NX16BArrayLength(const DataPoolArray* pArray, const DATAPOOL_VARIABLE_DESC* pVariDesc)
    {
      return ALIGN_16(static_cast<GXUINT>(pArray->GetSize())) / ALIGN_16(pVariDesc->TypeSize());
    }

    GXUINT NX16BTypeSize(GXDWORD dwFlags, const DATAPOOL_VARIABLE_DESC* pVariDesc)
    {
      return IS_MARK_NX16B(dwFlags)
        ? ALIGN_16(pVariDesc->TypeSize()) : pVariDesc->TypeSize();
    }

    template<typename _Ty>
    void NX16BCopyArrayData(GXLPBYTE pDest, const _Ty* pSrc, GXUINT nCount)
    {
      for(GXUINT i = 0; i < nCount; i++)
      {
        *reinterpret_cast<_Ty*>(pDest) = *pSrc;
        pDest += ALIGN_16(sizeof(_Ty));
        pSrc++;
      }
    }

    void NX16BCopyArrayData(DataPoolTypeClass cls, GXLPBYTE pDest, GXLPCVOID pSrc, GXUINT nCount)
    {
      switch(cls)
      {
      case Marimo::DataPoolTypeClass::Byte:
      case Marimo::DataPoolTypeClass::SByte:
        NX16BCopyArrayData(pDest, static_cast<const u8*>(pSrc), nCount);
        break;
      case Marimo::DataPoolTypeClass::Word:
      case Marimo::DataPoolTypeClass::SWord:
        NX16BCopyArrayData(pDest, static_cast<const u16*>(pSrc), nCount);
        break;
      case Marimo::DataPoolTypeClass::DWord:
      case Marimo::DataPoolTypeClass::SDWord:
        NX16BCopyArrayData(pDest, static_cast<const u32*>(pSrc), nCount);
        break;
      case Marimo::DataPoolTypeClass::QWord:
      case Marimo::DataPoolTypeClass::SQWord:
        NX16BCopyArrayData(pDest, static_cast<const u64*>(pSrc), nCount);
        break;
      case Marimo::DataPoolTypeClass::Float:
        NX16BCopyArrayData(pDest, static_cast<const float*>(pSrc), nCount);
        break;
      case Marimo::DataPoolTypeClass::Object:
        NX16BCopyArrayData(pDest, static_cast<const size_t*>(pSrc), nCount);
        break;
      default:
        CLBREAK; // 不应改到这里
        break;
      }
    }

    GXUINT GetMemberAlignMask(DataPoolPack eMemberPack)
    {
      switch(eMemberPack)
      {
      case DataPoolPack::Align2:
        return 1;
      case DataPoolPack::Align4:
        return 3;           
      case DataPoolPack::Align8:
        return 7;
      case DataPoolPack::Align16:
        return 15;
      default:
        return 0;
      }
    }

    GXBOOL IntCreateSubPool(DataPool** ppSubPool, DataPoolImpl* pReference);

  } // namespace DataPoolInternal

#endif // #ifndef _HIDING_CODE_
  namespace Implement
  {
    //extern DATAPOOL_TYPE_DECLARATION c_InternalTypeDefine[];
    extern DataPoolVariable::VTBL* s_pPrimaryVtbl;
    extern DataPoolVariable::VTBL* s_pEnumVtbl;
    extern DataPoolVariable::VTBL* s_pFlagVtbl;
    extern DataPoolVariable::VTBL* s_pObjectVtbl;
    extern DataPoolVariable::VTBL* s_pStringVtbl;
    extern DataPoolVariable::VTBL* s_pStringAVtbl;
    extern DataPoolVariable::VTBL* s_pStructVtbl;
    extern DataPoolVariable::VTBL* s_pStaticArrayVtbl;
    extern DataPoolVariable::VTBL* s_pDynamicArrayVtbl;
    extern DataPoolVariable::VTBL* s_pStaticArrayNX16BVtbl;
    extern DataPoolVariable::VTBL* s_pDynamicArrayNX16BVtbl;

#ifndef _HIDING_CODE_
    void CopyVariables(DataPoolImpl::VARIABLE_DESC* pDestVarDesc, const DataPoolBuildTime::BTVarDescArray* pSrcVector, const clstd::STRINGSETDESC* pTable, GXINT_PTR lpBase)
    {
      int i = 0;
      for(auto it = pSrcVector->begin(); it != pSrcVector->end(); ++it, ++i)
      {
        const DataPoolBuildTime::BT_VARIABLE_DESC& sBtDesc = *it;
        DataPoolImpl::VARIABLE_DESC& sDesc = pDestVarDesc[i];
        //sDesc.TypeDesc = (GXUINT)((GXINT_PTR)&sDesc.TypeDesc - (GXINT_PTR)&m_aTypes[((BUILDTIME::BT_TYPE_DESC*)sBtDesc.GetTypeDesc())->nIndex]);
        sDesc.TypeDesc = (GXUINT)((GXINT_PTR)&sDesc.TypeDesc - ((DataPoolBuildTime::BT_TYPE_DESC*)sBtDesc.GetTypeDesc())->nTypeAddress);
        sDesc.nName     = (GXUINT)clstd::StringSetA::IndexToOffset(pTable, (int)sBtDesc.nNameIndex);
        sDesc.nOffset   = sBtDesc.nOffset;
        sDesc.nCount    = sBtDesc.nCount;
        sDesc.bDynamic  = sBtDesc.bDynamic;
        sDesc.bConst    = sBtDesc.bConst;

#ifdef DEBUG_DECL_NAME
        sDesc.Name      = (DataPool::LPCSTR)(lpBase + sDesc.nName);
        TRACE("VAR:%16s %16s %8d\n", sDesc.GetTypeDesc()->Name, sDesc.Name, sDesc.nOffset);
#endif // DEBUG_DECL_NAME
      }
    }

    GXUINT CopyHashAlgo(DATAPOOL_HASHALGO* pAlgo, const DataPoolBuildTime::HASH_ALGORITHM* pBTAlgo, u8* aBuckets, GXUINT cbBuckets)
    {
      ASSERT(pBTAlgo->nBucket < 256); // 目前m_aHashBuckets是8位的，如果出现这个断言就改成u8/u16自适应的

      if(pBTAlgo->nBucket >= (1 << 14) || pBTAlgo->nPos >= (1 << 16)) {
        pAlgo->eType = 0;
      }
      else {
        GXINT_PTR offset = (GXINT_PTR)(aBuckets + cbBuckets) - (GXINT_PTR)&pAlgo->nOffset;
        ASSERT(offset < (1 << (sizeof(pAlgo->nOffset) * 8)));
        pAlgo->eType   = pBTAlgo->eType;
        pAlgo->nBucket = pBTAlgo->nBucket;
        pAlgo->nPos    = pBTAlgo->nPos;
        pAlgo->nOffset = (u16)offset;

        ASSERT(pBTAlgo->nBucket * 2 == pBTAlgo->indices.size());
        std::for_each(pBTAlgo->indices.begin(), pBTAlgo->indices.end(), [&aBuckets, &cbBuckets](int index){
          aBuckets[cbBuckets++] = index;
        });
      }

      return cbBuckets;
    }
#endif // #ifndef _HIDING_CODE_
  } // namespace Implement

  //////////////////////////////////////////////////////////////////////////
#ifndef _HIDING_CODE_

  DataPoolTypeClass operator&(DataPoolTypeClass a, DataPoolTypeClass b)
  {
    return static_cast<DataPoolTypeClass>(static_cast<GXUINT>(a) & static_cast<GXUINT>(b));
  }

  b32 operator==(DataPoolTypeClass a, GXUINT b)
  {
    return (static_cast<GXUINT>(a) == b);
  }

  //////////////////////////////////////////////////////////////////////////

  const DATAPOOL_STRUCT_DESC* DATAPOOL_VARIABLE_DESC::GetStructDesc() const
  {
    const DATAPOOL_TYPE_DESC* pTypeDesc = GetTypeDesc();
    ASSERT(
      (pTypeDesc->Cate & DataPoolTypeClass::Enum_ClassMask) == DataPoolTypeClass::Structure ||
      (pTypeDesc->Cate & DataPoolTypeClass::Enum_ClassMask) == DataPoolTypeClass::Enumeration ||
      (pTypeDesc->Cate & DataPoolTypeClass::Enum_ClassMask) == DataPoolTypeClass::Flag);
    return static_cast<const DATAPOOL_STRUCT_DESC*>(pTypeDesc);
  }

  //////////////////////////////////////////////////////////////////////////

  DataPoolImpl::DataPoolImpl(GXLPCSTR szName)
    : m_Name           (szName ? szName : "")
    , m_nNumOfTypes    (0)
    , m_aTypes         (NULL)
    , m_nNumOfStructs  (0)
    , m_aStructs       (NULL)
    , m_cbHashBuckets  (0)
    , m_aHashAlgos     (NULL)
    , m_aHashBuckets   (NULL)
    , m_nNumOfVar      (0)
    , m_aVariables     (NULL)
    , m_nNumOfMember   (0)
    , m_aMembers       (NULL)
    , m_nNumOfEnums    (0)
    , m_aEnums         (NULL)
    , m_aGSIT          (NULL)
    , m_dwRuntimeFlags (RuntimeFlag_Fixed)
    , m_pNamesTabBegin (NULL)
    , m_pNamesTabEnd   (NULL)
  {
    //TRACE("sizeof(DataPoolImpl):%u\n", sizeof(DataPoolImpl));
  }

#endif // #ifndef _HIDING_CODE_
  DataPoolImpl::~DataPoolImpl()
  {
    // 清理池中的数据
    // 主要是清理动态数组，字符串，对象和结构体
    // 结构体会产生递归，遍历其下的数据类型
    if(m_nNumOfVar && TEST_FLAG_NOT(m_dwRuntimeFlags, RuntimeFlag_Readonly)) {
      Cleanup(m_VarBuffer.GetPtr(), m_aVariables, (int)m_nNumOfVar);
      //ASSERT(m_nDbgNumOfArray == 0);
      //ASSERT(m_nDbgNumOfString == 0);
    }

    if(m_Name.IsNotEmpty())
    {
      GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
      if(lpStation) {
        GXSTATION::NamedInterfaceDict::iterator it = lpStation->m_NamedPool.find(m_Name);
        if(it != lpStation->m_NamedPool.end()) {
          lpStation->m_NamedPool.erase(it);
        }
        else {
          CLBREAK; // 不应该啊, 肿么找不到了呢?
        }
      }
    }

#ifndef DISABLE_DATAPOOL_WATCHER
    IntCleanupWatchObj(m_FixedDict);
    for(WatchableArray::iterator it = m_WatchableArray.begin(); it != m_WatchableArray.end(); ++it)
    {
      IntCleanupWatchObj(it->second);
    }
#endif // #ifndef DISABLE_DATAPOOL_WATCHER
  }
#ifndef _HIDING_CODE_

  //////////////////////////////////////////////////////////////////////////

  GXBOOL DataPoolImpl::Initialize(LPCTYPEDECL pTypeDecl, LPCVARDECL pVarDecl, DataPoolCreation dwFlags)
  {
    if(pVarDecl == NULL) {
      return FALSE;
    }
    m_dwRuntimeFlags = dwFlags;
    //FloatVarTypeDict FloatDict; // 很多事情还没有确定下来的类型表, 浮动的
    DataPoolBuildTime sBuildTime(dwFlags);   // 构建时使用的结构

    // 创建浮动类型表
#ifdef _DEBUG
    // -- 内置类型表示是被信任的,不进行合法性检查,Debug版还是要稍微检查下的
# define CHECK_INTERNAL_TYPE TRUE
#else
# define CHECK_INTERNAL_TYPE FALSE
#endif // #ifdef _DEBUG9

    if(_CL_NOT_(sBuildTime.IntCheckTypeDecl(SELECT_INTERNAL_TYPE_TABLE(dwFlags), CHECK_INTERNAL_TYPE)))
    {
      return FALSE;
    }

    if(pTypeDecl != NULL) {
      if( ! sBuildTime.IntCheckTypeDecl(pTypeDecl, TRUE))
        return FALSE;
    }

    // 检查变量类型
    if( ! sBuildTime.CheckVarList(pVarDecl)) {
      return FALSE;
    }

    //const GXUINT nAlignSize = DataPoolInternal::CreationFlagsToAlignSize(dwFlags);
    const DataPoolPack eMemberPack = DataPoolInternal::CreationFlagsToMemberPack(dwFlags);

    GXINT nBufferSize = sBuildTime.ComputeVariableSize(pVarDecl, sBuildTime.m_aVar, eMemberPack);
    if(nBufferSize == 0) {
      CLOG_ERROR("%s: Empty data pool.\n", __FUNCTION__);
      return FALSE;
    }

    if(TEST_FLAG(sBuildTime.m_dwBuildFlags, DataPoolCreation_NoHashTable)) {
      // nothing...
    } else {
      DataPoolBuildTime::TryHash(sBuildTime.m_VarHashInfo, sBuildTime.m_aVar);
      sBuildTime.m_nNumOfBuckets += sBuildTime.m_VarHashInfo.indices.size();
    }

    // 定位各种描述表
    LocalizeTables(sBuildTime, nBufferSize);
    
    InitializeValue(0, pVarDecl);

    //m_bFixedPool = IntIsRawPool();
    return TRUE;
  }

#endif // #ifndef _HIDING_CODE_

  GXBOOL DataPoolImpl::CleanupArray(const VARIABLE_DESC* pVarDesc, GXLPVOID lpFirstElement, GXUINT nElementCount)
  {
    switch(pVarDesc->GetTypeClass())
    {
    case DataPoolTypeClass::String:
      {
        clStringW* pString = reinterpret_cast<clStringW*>(lpFirstElement);

        // 依次调用析构函数
        for(GXUINT nStringIndex = 0; nStringIndex < nElementCount; nStringIndex++)
        {
          if(pString[nStringIndex]) {
            pString[nStringIndex].~StringX();
//#ifdef _DEBUG
//            m_nDbgNumOfString--;
//#endif // #ifdef _DEBUG
          }
        }
      }
      break;

    case DataPoolTypeClass::StringA:
      {
        clStringA* pString = reinterpret_cast<clStringA*>(lpFirstElement);

        // 依次调用析构函数
        for(GXUINT nStringIndex = 0; nStringIndex < nElementCount; nStringIndex++)
        {
          if(pString[nStringIndex]) {
            pString[nStringIndex].~StringX();
//#ifdef _DEBUG
//            m_nDbgNumOfString--;
//#endif // #ifdef _DEBUG
          }
        }
      }
      break;

    case DataPoolTypeClass::Object:
      {
        GUnknown** pObjArray = reinterpret_cast<GUnknown**>(lpFirstElement);

        // 依次调用析构函数
        for(GXUINT i = 0; i < nElementCount; i++) {
          SAFE_RELEASE(pObjArray[i]);
        }
      }
      break;

    case DataPoolTypeClass::Structure:
      {
        for(GXUINT nStructIndex = 0; nStructIndex < nElementCount; nStructIndex++)
        {
          Cleanup(lpFirstElement, pVarDesc->MemberBeginPtr(), pVarDesc->MemberCount());
          lpFirstElement = (GXLPBYTE)lpFirstElement + pVarDesc->TypeSize(); // NX16B?
        }
      }
      break;
    }
    return TRUE;
  }

  GXBOOL DataPoolImpl::Cleanup(GXLPVOID lpBuffer, const DATAPOOL_VARIABLE_DESC* pVarDesc, int nVarDescCount)
  {
    GXBYTE* pData = (GXBYTE*)lpBuffer;
    for(int i = 0; i < nVarDescCount; i++)
    {
      const VARIABLE_DESC& VARDesc = reinterpret_cast<const VARIABLE_DESC&>(pVarDesc[i]);
      GXBOOL bDynamicArray = VARDesc.IsDynamicArray();
      DataPoolArray** ppBuffer = NULL;

      GXLPVOID ptr;
      int nCount = 0;

      if(bDynamicArray) // 动态字符串数组
      {
        ppBuffer = VARDesc.GetAsBufferObjPtr(pData);
        if(*ppBuffer == NULL) {
          continue;
        }
        ptr = (*ppBuffer)->GetPtr();
        nCount = (int)((*ppBuffer)->GetSize() / VARDesc.TypeSize()); // NX16B?
      }
      else // 字符串数组
      {
        ptr = VARDesc.GetAsPtr(pData);
        nCount = VARDesc.nCount;
      }

      CleanupArray(&VARDesc, ptr, nCount);
      
      if(bDynamicArray)
      {
        // ppBuffer 可能在上面的判断中已经设置，如果没设置就是基础类型动态数组。
        ASSERT(ppBuffer == NULL || ppBuffer == VARDesc.GetAsBufferObjPtr(pData));

        ppBuffer = VARDesc.GetAsBufferObjPtr(pData); // (clBuffer**)(pData + VARDesc.nOffset);
        if(*ppBuffer)
        {
          delete (*ppBuffer);
          *ppBuffer = NULL;
//#ifdef _DEBUG
//          m_nDbgNumOfArray--;
//#endif // #ifdef _DEBUG
        }
      }

    }
    return TRUE;
  }
#ifndef _HIDING_CODE_

  DataPoolImpl::LPCTD DataPoolImpl::FindType(GXLPCSTR szTypeName) const
  {
    for(GXUINT i = 0; i < m_nNumOfTypes; ++i)
    {
      if(GXSTRCMP((DataPool::LPCSTR)m_aTypes[i].GetName(), szTypeName) == 0) {
        return &m_aTypes[i];
      }
    }

    for(GXUINT i = 0; i < m_nNumOfStructs; ++i)
    {
      if(GXSTRCMP((DataPool::LPCSTR)m_aStructs[i].GetName(), szTypeName) == 0) {
        return &m_aStructs[i];
      }
    }
    return NULL;
  }

#endif // #ifndef _HIDING_CODE_

  DataPoolImpl::LPCVD DataPoolImpl::IntGetVariable(LPCVD pVdd, GXLPCSTR szName/*, int nIndex*/)
  {
    // TODO: 这里可以改为比较Name Id的方式，Name Id可以就是m_Buffer中的偏移
    LPCVD pDesc = NULL;
    int begin = 0, end;
    int nHashIndex = 0;
    if(pVdd != NULL) {

      // 只有结构体才有成员, 其他情况直接返回
      if(pVdd->GetTypeClass() != DataPoolTypeClass::Structure) {
        return NULL;
      }
      end   = pVdd->MemberCount();
      pDesc = static_cast<LPCVD>(pVdd->MemberBeginPtr());
      nHashIndex = (int)((STRUCT_DESC*)pVdd->GetTypeDesc() - m_aStructs) + 1;

      if(nHashIndex <= 0 || nHashIndex > (int)m_nNumOfStructs) {
        LPCSTR szName = (LPCSTR)pVdd->TypeName();
        CLBREAK; // 临时
        return NULL;
      }
    }
    else {
      end   = m_nNumOfVar;
      pDesc = m_aVariables;
    }

    ASSERT(pVdd == NULL ||
      pVdd->GetTypeClass() == DataPoolTypeClass::Structure ||
      pVdd->GetTypeClass() == DataPoolTypeClass::Enumeration ||
      pVdd->GetTypeClass() == DataPoolTypeClass::Flag);

    const GXUINT count = (GXUINT)end;
#ifdef _DEBUG
    LPCVD pDescBegin = pDesc;
#endif // #ifdef _DEBUG

    // Hash Find
    const HASHALGO& h = m_aHashAlgos[nHashIndex];
    if(h.eType == (u16)clstd::StaticStringsDict::HashType_Local || h.eType == (u16)clstd::StaticStringsDict::HashType_String)
    {
      STATIC_ASSERT(clstd::StaticStringsDict::HashType_Local == 1);
      clstd::StaticStringsDict::len_t _hash = h.HashString(szName);

      const u8* p = h.HashToIndex(_hash);
      ASSERT(h.nBucket >= count);
      
      // hash 索引允许两次重叠，在p[0],p[1]中
      if(count > p[0])
      {
        LPCVD pDesc2 = pDesc + (GXUINT)p[1];
        pDesc = pDesc + (GXUINT)p[0];

        if(GXSTRCMP((GXLPCSTR)pDesc->VariableName(), szName) == 0) {
          return pDesc;
        }
        else if(count > p[1] && GXSTRCMP((GXLPCSTR)pDesc2->VariableName(), szName) == 0) {
          return pDesc2;
        }
      }
    }
    else // B-Find
    {
      SortedIndexType* p = m_aGSIT + (pDesc - m_aVariables);
      while(begin <= end - 1)
      {
        int mid = (begin + end) >> 1;
        ASSERT(p[mid] < count); // 索引肯定低于总大小
        LPCVD pCurDesc = pDesc + p[mid];
        //TRACE("%s\n", pCurDesc->VariableName());
        int result = GXSTRCMP(szName, (DataPool::LPCSTR)pCurDesc->VariableName());
        if(result < 0) {
          end = mid;
        }
        else if(result > 0){
          if(begin == mid) {
            break;
          }
          begin = mid;
        }
        else {
          return pCurDesc;
        }
      }
    }

#ifdef _DEBUG
    for(int i = begin; i < (int)count; i++)
    {
      if(GXSTRCMP((DataPool::LPCSTR)pDescBegin[i].VariableName(), szName) == 0) {
        CLBREAK;
        return &pDescBegin[i];
      }
    }
#endif // #ifdef _DEBUG

    return NULL;
  }

#ifndef _HIDING_CODE_

  /*
  GXHRESULT DataPoolImpl::GetLayout(GXLPCSTR szStructName, DataLayoutArray* pLayout)
  {
    LPCVD   pVarDesc = m_aVariables;
    GXUINT  nCount = (GXUINT)m_nNumOfVar;

    if(szStructName != NULL)
    {
      //TypeDict::const_iterator itType = m_TypeDict.find(szStructName);
      //if(itType == m_TypeDict.end()) {
      //  CLBREAK;
      //  return GX_FAIL;
      //}
      LPCSD pDesc = FindType(szStructName);
      pVarDesc = reinterpret_cast<LPCVD>(pDesc->GetMembers());
      nCount   = pDesc->nMemberCount;
    }

    for(GXUINT i = 0; i < nCount; i++)
    {
      DATALAYOUT DataLayout;
      DataLayout.eType = GXUB_UNDEFINED;
      DataLayout.pName = (DataPool::LPCSTR)pVarDesc->VariableName();
      DataLayout.uOffset = pVarDesc->nOffset;
      DataLayout.uSize = pVarDesc->TypeSize();
      pLayout->push_back(DataLayout);
      pVarDesc++;
    }

    return GX_OK;
  }//*/

  GXVOID DataPoolImpl::InitializeValue(GXUINT nBaseOffset, LPCVARDECL pVarDecl)
  {
    int nVarIndex = 0;
    GXBYTE* pData = (GXBYTE*)m_VarBuffer.GetPtr();
    for(;; nVarIndex++)
    {
      // 下面两个开头故意写成不一样的, 否则不太容易区分, 太像了!
      const DATAPOOL_VARIABLE_DECLARATION& varDecl = pVarDecl[nVarIndex];
      if(varDecl.Type == NULL || varDecl.Name == NULL) {
        break;
      }
      const VARIABLE_DESC& VARDesc = m_aVariables[nVarIndex];

      GXBOOL bDynamicArray = VARDesc.IsDynamicArray();
      ASSERT(GXSTRCMPI((DataPool::LPCSTR)VARDesc.VariableName(), varDecl.Name) == 0);
      switch(VARDesc.GetTypeClass())
      {
      case DataPoolTypeClass::Structure:
        {
          int nMemberIndex;
          //int nStart = VARDesc.MemberBegin();
          int nEnd = VARDesc.MemberCount();
          auto pMembers = VARDesc.MemberBeginPtr();

          // 对于含有动态数组和字符串的结构体是不能直接赋值的
          for(nMemberIndex = 0; nMemberIndex < nEnd; nMemberIndex++)
          {
            if(pMembers[nMemberIndex].GetTypeClass() == DataPoolTypeClass::String || 
              pMembers[nMemberIndex].GetTypeClass() == DataPoolTypeClass::StringA || 
              pMembers[nMemberIndex].IsDynamicArray())
              break;
          }
          if(nMemberIndex != nEnd) {
            break;
          }
          else if(IS_MARK_NX16B(m_dwRuntimeFlags) && varDecl.Init) {
            CLOG_WARNING("NX16B 模式不支持初始化数组");
            break;
          }
        } // 这里没有 break, 如果 Struct 中没有动态数组和字符串声明, 支持初始数据.
      case DataPoolTypeClass::Byte:
      case DataPoolTypeClass::Word:
      case DataPoolTypeClass::DWord:
      case DataPoolTypeClass::Object:
      case DataPoolTypeClass::QWord:
      case DataPoolTypeClass::SByte:
      case DataPoolTypeClass::SWord:
      case DataPoolTypeClass::SDWord:
      case DataPoolTypeClass::SQWord:
      case DataPoolTypeClass::Float:
        if(varDecl.Init != NULL)
        {
          if(bDynamicArray)
          {
            ASSERT(varDecl.Count < 0);
            clBuffer* pBuffer = IntCreateArrayBuffer(&m_VarBuffer, &VARDesc, pData, -varDecl.Count);
            if(IS_MARK_NX16B(m_dwRuntimeFlags)) {
              // TODO: 不支持NX16B初始化
              DataPoolInternal::NX16BCopyArrayData(VARDesc.GetTypeClass(), static_cast<GXLPBYTE>(pBuffer->GetPtr()), varDecl.Init, -varDecl.Count);
            }
            else {
              memcpy(pBuffer->GetPtr(), varDecl.Init, pBuffer->GetSize());
            }
          }
          else
          {
            if(IS_MARK_NX16B(m_dwRuntimeFlags)) {
              // TODO: 不支持NX16B初始化
              DataPoolInternal::NX16BCopyArrayData(VARDesc.GetTypeClass(), static_cast<GXLPBYTE>(VARDesc.GetAsPtr(pData)), varDecl.Init, varDecl.Count);
            }
            else {
              memcpy(VARDesc.GetAsPtr(pData), varDecl.Init, VARDesc.GetCompactSize());
            }
          }
        }
        break;
        //case T_Struct:
        //  //if(varDecl.Init != NULL)
        //  break;
      case DataPoolTypeClass::String:
        {
          GXLPCWSTR  pStrInit = (GXLPCWSTR)varDecl.Init;
          clStringW* pStrPool = NULL;
          int nCount = varDecl.Init == NULL ? 0 : varDecl.Count;
          if(bDynamicArray)
          {
            ASSERT(VARDesc.TypeSize() == sizeof(clStringW));

            // 如果没有初始化数据, 则初始维度设置为0
            ASSERT((varDecl.Init != NULL && nCount < 0) || 
              (varDecl.Init == NULL && nCount == 0));

            pStrPool = (clStringW*)IntCreateArrayBuffer(&m_VarBuffer, &VARDesc, pData, -nCount)->GetPtr();
            nCount = -nCount;
          }
          else
          {
            pStrPool = VARDesc.GetAsStringW(pData);
          }

          // 这个写的太变态了!!
          for(int i = 0; i < nCount; i++)
          {
            clStringW& str = pStrPool[i];
            new(&str) clStringW;
            //INC_DBGNUMOFSTRING;
            if(pStrInit != NULL)
            {
              str = pStrInit;
              pStrInit += str.GetLength() + 1;
            }
          }
        } // case
        break;
      }
    }
  }
#endif // #ifndef _HIDING_CODE_

  GXDWORD DataPoolImpl::GetFlags() const
  {
    return m_dwRuntimeFlags;
  }

  GXBOOL DataPoolImpl::IsFixedPool() const
  {
    return (GXBOOL)TEST_FLAG(m_dwRuntimeFlags, DataPoolImpl::RuntimeFlag_Fixed);
  }

  DataPool::LPCSTR DataPoolImpl::GetVariableName(GXUINT nIndex) const
  {
    if(nIndex >= m_nNumOfVar) {
      return NULL;
    }

    return (DataPool::LPCSTR)m_aVariables[nIndex].VariableName();
  }

  GXLPVOID DataPoolImpl::GetRootPtr() const
  {
    return m_VarBuffer.GetPtr();
  }

  GXSIZE_T DataPoolImpl::GetRootSize() const
  {
    return m_VarBuffer.GetSize();
  }

  GXBOOL DataPoolImpl::QueryByName(GXLPCSTR szName, Variable* pVar)
  {
    VARIABLE var = {0};
    var.pBuffer = &m_VarBuffer;
    if( ! IntQuery(&var, szName, -1)) {
      pVar->Free();
      return FALSE;
    }

    if(pVar->GetPoolUnsafe() == VARIABLE_DATAPOOL_OBJECT) {
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, var.pVdd, var.pBuffer, var.AbsOffset);
    } else {
      pVar->Free();
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, VARIABLE_DATAPOOL_OBJECT, var.pVdd, var.pBuffer, var.AbsOffset);
    }
    return TRUE;
  }

  GXBOOL DataPoolImpl::QueryByExpression(GXLPCSTR szExpression, Variable* pVar)
  {
    VARIABLE var = {0};

    GXBOOL bval = IntQueryByExpression(szExpression, &var);

    if( ! bval) {
      pVar->Free();
    }
    else {
      //if( ! pVar->IsValid()) {
      //  pVar->Free();
      //}
      pVar->~DataPoolVariable();
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, VARIABLE_DATAPOOL_OBJECT, var.pVdd, var.pBuffer, var.AbsOffset);
    }
    return bval;
  }
 
  GXBOOL DataPoolImpl::FindFullName( clStringA* str, DataPool::LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset )
  {
    LPCVD pVar;
    if(pBuffer == &m_VarBuffer) {

      LPCVD pVarDescTable = m_aVariables;
      GXUINT count = m_nNumOfVar;

      while(1)
      {
        pVar = IntFindVariable(pVarDescTable, count, nOffset);

        if(str->IsNotEmpty()) {
          str->Append('.');
        }
        str->Append((DataPool::LPCSTR)pVar->VariableName());

        if(pVarDesc == pVar) {
          TRACE("FindFullName:%s\n", (DataPool::LPCSTR)*str);
          return TRUE;
        }
        else {
          TRACE("[%s]:[%s]\n", (DataPool::LPCSTR)pVar->VariableName(), (DataPool::LPCSTR)pVarDesc->VariableName());
          ASSERT(pVar->GetTypeClass() == DataPoolTypeClass::Structure);
          pVarDescTable = (LPCVD)pVar->MemberBeginPtr();
          count = pVar->MemberCount();

          if(nOffset < pVar->nOffset) { // 偏移异常,直接返回
            return FALSE;
          }

          nOffset -= pVar->nOffset;

          if(pVar->nCount > 1) {
            //const GXUINT size = IS_MARK_NX16B(m_dwRuntimeFlags)
            //  ? ALIGN_16(pVar->TypeSize()) : pVar->TypeSize();
            const GXUINT size = DataPoolInternal::NX16BTypeSize(m_dwRuntimeFlags, pVar);
            const GXUINT index = nOffset / size;
            str->AppendFormat("[%d]", index);
            nOffset -= index * size;
          }
        }
      }

      //CLNOP;
    }
    return FALSE;
  }

#ifndef DISABLE_DATAPOOL_WATCHER
  GXBOOL DataPoolImpl::IsAutoKnock()
  {
    return TEST_FLAG(m_dwRuntimeFlags, DataPoolImpl::RuntimeFlag_AutoKnock);
  }

  GXBOOL DataPoolImpl::IsKnocking(const DataPoolVariable* pVar)
  {
    return IntIsImpulsing(pVar);
  }

  GXBOOL DataPoolImpl::SetAutoKnock(GXBOOL bAutoKnock)
  {
    GXBOOL bPrevFlag = TEST_FLAG(m_dwRuntimeFlags, DataPoolImpl::RuntimeFlag_AutoKnock);
    if(bAutoKnock) {
      SET_FLAG(m_dwRuntimeFlags, DataPoolImpl::RuntimeFlag_AutoKnock);
    }
    else {
      RESET_FLAG(m_dwRuntimeFlags, DataPoolImpl::RuntimeFlag_AutoKnock);
    }
    return bPrevFlag;
  }
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

  GXINT DataPoolImpl::IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar)
  {
    GXLPCSTR  szName;         // 用来查找的名字
    GXUINT    nIndex = (GXUINT)-1;
    clStringA str;
    clStringA strName; // 用来暂存带下标时的变量名
    GXINT     result = FALSE;
    //clBufferBase* pArrayBuffer = NULL;

    ASSERT( ! pVar->IsValid());
    pVar->pBuffer = &m_VarBuffer;

    // 分解表达式
    clstd::StringCutter<clStringA> sExpression(szExpression);

    do {

      if(pVar->IsValid())
      {
        // 数组对象，没有取元素就试图访问成员变量，一定是不对的
        // 进到这里一定是取过根变量之后的事情了
        if((pVar->pVdd->IsDynamicArray() || pVar->pVdd->nCount > 1) && nIndex == (GXUINT)-1) {
          return FALSE;
        }
      }

      sExpression.Cut(str, '.');

      if(str.EndsWith(']')) // 带下标的情况
      {
        size_t pos = str.Find('[', 0);
        if(pos == clStringA::npos) {
          CLBREAK;
          result = FALSE;
          break;
        }
        clStringA strIndex = str.SubString(pos + 1, str.GetLength() - pos - 2);
        strName = str.Left(pos);
        nIndex = strIndex.ToInteger();  // GXATOI((GXLPCSTR)strIndex);
        szName = strName;
      }
      else {
        nIndex = -1;
        szName = str;
      }

      result = IntQuery(pVar, szName, nIndex);

      ASSERT(( ! result) || pVar->IsValid()); // result 和 pVar 一定同时有效或者无效
    } while(result && ( ! sExpression.IsEndOfString()));

    return result;
  }

  DataPoolImpl::LPCVD DataPoolImpl::IntFindVariable(LPCVD pVarDesc, int nCount, GXUINT nOffset)
  {
    int begin = 0;
    int end = nCount - 1;
    auto nEndOffset = pVarDesc[end].nOffset + pVarDesc[end].GetUsageSize(IS_MARK_NX16B(m_dwRuntimeFlags));
    ASSERT(nOffset <= nEndOffset);
    while(1)
    {
      const int mid = (begin + end) >> 1;
      if(pVarDesc[begin].nOffset <= nOffset && nOffset < pVarDesc[mid].nOffset) {
        end = mid;
        nEndOffset = pVarDesc[end].nOffset + pVarDesc[end].GetUsageSize(IS_MARK_NX16B(m_dwRuntimeFlags));
      }
      else if(pVarDesc[mid].nOffset <= nOffset && nOffset <= nEndOffset) {
        if(begin == mid) {
          break;
        }
        begin = mid;
      }
    }

    if(pVarDesc[end].nOffset <= nOffset) {
      return pVarDesc + end;
    }
    else if(pVarDesc[begin].nOffset <= nOffset) {
      return pVarDesc + begin;
    }

    return NULL;
  }

  template<class _TIter>
  _TIter& DataPoolImpl::first_iterator(_TIter& it)
  {
    it.pDataPool = VARIABLE_DATAPOOL_OBJECT; // 这个导致结构不能修饰为const
    it.pVarDesc  = m_aVariables;
    it.pBuffer   = &m_VarBuffer;
    it.nOffset   = 0;
    it.index     = (GXUINT)-1;
    return it;
  }

  DataPoolUtility::iterator DataPoolImpl::begin()
  {
    DataPoolUtility::iterator it;
    return first_iterator<DataPoolUtility::iterator>(it);
  }

  DataPoolUtility::iterator DataPoolImpl::end()
  {
    DataPoolUtility::iterator it;
    first_iterator(it);
    it.pVarDesc += m_nNumOfVar;
    return it;
  }

  DataPoolUtility::named_iterator DataPoolImpl::named_begin()
  {
    DataPoolUtility::named_iterator it;
    return first_iterator(it);
  }

  DataPoolUtility::named_iterator DataPoolImpl::named_end()
  {
    DataPoolUtility::named_iterator it;
    first_iterator(it);
    it.pVarDesc += m_nNumOfVar;
    return it;
  }

  GXSIZE_T DataPoolImpl::IntGetRTDescHeader(SIZELIST* pSizeList)
  {
    ASSERT(m_nNumOfTypes || m_nNumOfStructs);
    ASSERT(m_nNumOfVar);
    ASSERT(
      (m_nNumOfStructs == 0 && (m_nNumOfMember == 0 && m_nNumOfEnums == 0)) ||
      (m_nNumOfStructs != 0 && (m_nNumOfMember != 0 || m_nNumOfEnums != 0)) );
    ASSERT(m_pNamesTabEnd && m_pNamesTabBegin < m_pNamesTabEnd);  // m_pNamesTabBegin 在构建时暂时为0
    ASSERT(m_cbHashBuckets != 0 || TEST_FLAG(m_dwRuntimeFlags, DataPoolCreation_NoHashTable));

    pSizeList->cbTypes     = m_nNumOfTypes * sizeof(TYPE_DESC);
    pSizeList->cbStructs   = m_nNumOfStructs * sizeof(STRUCT_DESC);
    pSizeList->cbHashAlgo  = (m_nNumOfStructs + 1) * sizeof(HASHALGO);
    pSizeList->cbGVSIT     = (m_nNumOfVar + m_nNumOfMember + m_nNumOfEnums) * sizeof(SortedIndexType);
    pSizeList->cbVariables = m_nNumOfVar * sizeof(VARIABLE_DESC);
    pSizeList->cbMembers   = m_nNumOfMember * sizeof(VARIABLE_DESC);
    pSizeList->cbEnums     = m_nNumOfEnums * sizeof(ENUM_DESC);
    pSizeList->cbNameTable = (GXSIZE_T)m_pNamesTabEnd - (GXSIZE_T)m_pNamesTabBegin;

    return (pSizeList->cbTypes + pSizeList->cbStructs + pSizeList->cbHashAlgo + m_cbHashBuckets +
      pSizeList->cbGVSIT + pSizeList->cbVariables + pSizeList->cbMembers + pSizeList->cbEnums + pSizeList->cbNameTable);
  }

  GXSIZE_T DataPoolImpl::IntGetRTDescNames()
  {
    SIZELIST sSizeList;
    return m_Buffer.GetSize() - IntGetRTDescHeader(&sSizeList) - m_VarBuffer.GetSize();
  }

  GXUINT DataPoolImpl::IntChangePtrSize(GXUINT nSizeofPtr, VARIABLE_DESC* pVarDesc, GXUINT nCount, GXBOOL bNX16B)
  {
    // 验证是全局变量开始，或者是成员变量开始
    ASSERT(pVarDesc->nOffset == 0);

    // 只用于32位指针到64位指针或者64位指针到32位指针转换
    ASSERT(nSizeofPtr == 4 || nSizeofPtr == 8);

    GXUINT nNewOffset = 0;
    for(GXUINT i = 0; i < nCount; ++i)
    {
      VARIABLE_DESC& d = pVarDesc[i];
      const auto eCate = d.GetTypeClass();
      d.nOffset = nNewOffset;

      // 检查已经调整的标记
      // 如果没有调整过，则重新计算这个类型的大小
      // 否则步进这个类型的大小就可以
      if(TEST_FLAG_NOT(eCate, static_cast<DataPoolTypeClass>(TYPE_CHANGED_FLAG)))
      {
        TYPE_DESC* pTypeDesc = (TYPE_DESC*)d.GetTypeDesc();
        GXUINT& uCate = *(GXUINT*)&pTypeDesc->Cate;
        SET_FLAG(uCate, TYPE_CHANGED_FLAG);
        switch(eCate)
        {
        case DataPoolTypeClass::Structure:
          pTypeDesc->cbSize = IntChangePtrSize(nSizeofPtr, (VARIABLE_DESC*)d.MemberBeginPtr(), d.MemberCount(), bNX16B);
          break;

        case DataPoolTypeClass::String:
        case DataPoolTypeClass::StringA:
        case DataPoolTypeClass::Object:
          ASSERT((pTypeDesc->cbSize == 4 && nSizeofPtr == 8) ||
            (pTypeDesc->cbSize == 8 && nSizeofPtr == 4));

          pTypeDesc->cbSize = nSizeofPtr;
          break;
        }
      }

      if(d.IsDynamicArray()) {
        // 动态数组就是一个指针
        nNewOffset += nSizeofPtr; // sizeof(DataPoolArray*)
      }
      else if(bNX16B) {
        nNewOffset += d.GetNX16BSize();
      }
      else {
        nNewOffset += d.GetCompactSize();
      }
    }
    return nNewOffset;
  }

  template<class _Ty>
  void DataPoolImpl::IntClearChangePtrFlag(_Ty* pTypeDesc, GXUINT nCount)
  {
    // 如果变量没有设置修改标志，表示这个函数之前的调用有误
    //ASSERT(TEST_FLAG(pTypeDesc[i].Cate, TYPE_CHANGED_FLAG));
    for(GXUINT i = 0; i < nCount; ++i)
    {
      RESET_FLAG(*(GXUINT*)&pTypeDesc[i].Cate, TYPE_CHANGED_FLAG);
    }
  }

#ifndef DISABLE_DATAPOOL_WATCHER
  void DataPoolImpl::IntImpulse(WatchFixedDict& sDict, GXLPVOID key, DATAPOOL_IMPULSE* pImpulse)
  {
    auto it_result = sDict.find(key);
    if(it_result != sDict.end())
    {

      // 加入到推送集合里，防止自己被无限推送
      ImpulsingSet::iterator itThisImpulse = m_ImpulsingSet.insert(key).first; // FIXME: 这个如果是动态数组的话，key是偏移，可能在多个动态数组中出现重合

      for(auto it = it_result->second.begin(); it != it_result->second.end(); ++it)
      {
        pImpulse->param = it->lParam;
        switch((GXINT_PTR)it->pCallback)
        {
        case 0: // DataPoolWatcher 对象
          ((DataPoolWatcher*)it->lParam)->OnImpulse(pImpulse);
          break;
        case 1: // UI handle
          gxSendMessage((GXHWND)it->lParam, GXWM_IMPULSE, 0, (GXLPARAM)pImpulse);
          break;
        default: // 回调函数
          it->pCallback(pImpulse);
          break;
        }
      }
      m_ImpulsingSet.erase(itThisImpulse);
    }
  }

  GXBOOL DataPoolImpl::Impulse(const DataPoolVariable& var, DataAction reason, GXSIZE_T index, GXSIZE_T count)
  {
    if( ! var.IsValid()) {
      return FALSE;
    }

    const auto dwCaps = var.GetCaps();
    const auto dwExclude = DataPoolVariable::CAPS_ARRAY | DataPoolVariable::CAPS_STRUCT;

    if(TEST_FLAG(dwCaps, dwExclude) || IntIsImpulsing(&var)) {
      return FALSE;
    }

    DATAPOOL_IMPULSE sImpulse;
    sImpulse.sponsor = &var;
    sImpulse.reason  = reason;
    sImpulse.index   = (GXUINT)index;
    sImpulse.count   = (GXUINT)count;
    sImpulse.param   = NULL;

    //TRACE("impulse\n");

    WatchableArray::iterator itArray;

    if(TEST_FLAG(dwCaps, DataPoolVariable::CAPS_FIXED)) {
      IntImpulse(m_FixedDict, var.GetPtr(), &sImpulse);
    }
    else if( (itArray = m_WatchableArray.find((DataPoolArray*)var.GetBuffer()))
      != m_WatchableArray.end() )
    {
      IntImpulse(itArray->second, (GXLPVOID)var.GetOffset(), &sImpulse);
    }
    else { return FALSE; }
    return TRUE;
  }

#endif // #ifndef DISABLE_DATAPOOL_WATCHER

  GXBOOL DataPoolImpl::IntCreateUnary(clBufferBase* pBuffer, LPCVD pThisVdd, VARIABLE* pVar)
  {
    ASSERT(pThisVdd->TypeSize() != 0);

    VARIABLE::VTBL* pVtbl = pThisVdd->GetUnaryMethod();
    ASSERT(pVtbl != NULL);

    pVar->Set((VARIABLE::VTBL*)pVtbl, pThisVdd, pBuffer, pVar->AbsOffset);
    return TRUE;
  }

  GXBOOL DataPoolImpl::IntQuery(GXINOUT VARIABLE* pVar, GXLPCSTR szVariableName, GXUINT nIndex)
  {
    // 内部函数中不改变pVar->m_pDataPool的引用计数
    using namespace Implement;
    LPCVD pVarDesc = IntGetVariable(pVar->pVdd, szVariableName);

    if(pVarDesc == NULL) {
      return FALSE;
    }

    ASSERT(GXSTRCMP((DataPool::LPCSTR)pVarDesc->VariableName(), szVariableName) == 0);
    const GXUINT nMemberOffset = pVar->AbsOffset + pVarDesc->nOffset; // 后面多出用到，这里算一下

    if(pVarDesc->IsDynamicArray()) { // 动态数组
      DataPoolArray* pArrayBuffer = IntCreateArrayBuffer(pVar->pBuffer, pVarDesc, (GXBYTE*)pVar->pBuffer->GetPtr() + pVar->AbsOffset, 0);
      if(nIndex == (GXUINT)-1)
      {
        pVar->Set(IS_MARK_NX16B(m_dwRuntimeFlags)
          ? (VARIABLE::VTBL*)s_pDynamicArrayNX16BVtbl
          : (VARIABLE::VTBL*)s_pDynamicArrayVtbl, pVarDesc, pVar->pBuffer, nMemberOffset);
        return TRUE;
      }
      else if(IS_MARK_NX16B(m_dwRuntimeFlags))
      {
        if(nIndex < DataPoolInternal::NX16BArrayLength(pArrayBuffer, pVarDesc))
        {
          pVar->AbsOffset = //nIndex * ALIGN_16(pVarDesc->TypeSize());
            DataPoolInternal::NX16BArrayOffset(pVarDesc->TypeSize(), nIndex);
          return IntCreateUnary(pArrayBuffer, pVarDesc, pVar);
        }
      }
      else if(nIndex < (pArrayBuffer->GetSize() / pVarDesc->TypeSize()))
      {
        pVar->AbsOffset = nIndex * pVarDesc->TypeSize();
        return IntCreateUnary(pArrayBuffer, pVarDesc, pVar);
      }
    }
    else if(pVarDesc->nCount > 1) { // 静态数组
      if(nIndex == (GXUINT)-1)
      {
        pVar->Set(IS_MARK_NX16B(m_dwRuntimeFlags)
          ? (VARIABLE::VTBL*)s_pStaticArrayNX16BVtbl
          : (VARIABLE::VTBL*)s_pStaticArrayVtbl, pVarDesc, pVar->pBuffer, nMemberOffset);
        return TRUE;
      }
      else if(nIndex < pVarDesc->nCount)
      {
        //pVar->AbsOffset += IS_MARK_NX16B(m_dwRuntimeFlags)
        //  ? (pVarDesc->nOffset + nIndex * ALIGN_16(pVarDesc->TypeSize()))
        //  : (pVarDesc->nOffset + nIndex * pVarDesc->TypeSize());
        pVar->AbsOffset += pVarDesc->nOffset + DataPoolInternal::NX16BArrayOffset(m_dwRuntimeFlags, pVarDesc->TypeSize(), nIndex);
        return IntCreateUnary(pVar->pBuffer, pVarDesc, pVar);
      }
    }
    else {
      ASSERT(pVarDesc->nCount == 1);
      pVar->AbsOffset = nMemberOffset;
      return IntCreateUnary(pVar->pBuffer, pVarDesc, pVar);
    }
    return FALSE;
  }

#ifndef DISABLE_DATAPOOL_WATCHER
  GXBOOL DataPoolImpl::IntIsImpulsing(const DataPoolVariable* pVar) const
  {
    ImpulsingSet::const_iterator it = m_ImpulsingSet.find(pVar->GetPtr());
    return it != m_ImpulsingSet.end();
  }
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

#ifndef _HIDING_CODE_
  const clBufferBase* DataPoolImpl::IntGetEntryBuffer() const
  {
    return &m_VarBuffer;
  }

//  void DataPoolImpl::CopyVariables(VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const STRINGSETDESC* pTable, GXINT_PTR lpBase)
//  {
//    const BUILDTIME::BTVarDescArray& SrcVector = *(BUILDTIME::BTVarDescArray*)pSrcVector;
//    int i = 0;
//    for(auto it = SrcVector.begin(); it != SrcVector.end(); ++it, ++i)
//    {
//      const BUILDTIME::BT_VARIABLE_DESC& sBtDesc = *it;
//      VARIABLE_DESC& sDesc = pDestVarDesc[i];
//      //sDesc.TypeDesc = (GXUINT)((GXINT_PTR)&sDesc.TypeDesc - (GXINT_PTR)&m_aTypes[((BUILDTIME::BT_TYPE_DESC*)sBtDesc.GetTypeDesc())->nIndex]);
//      sDesc.TypeDesc = (GXUINT)((GXINT_PTR)&sDesc.TypeDesc - ((BUILDTIME::BT_TYPE_DESC*)sBtDesc.GetTypeDesc())->nTypeAddress);
//      sDesc.nName     = Implement::ConvertToNewOffsetFromOldIndex(pTable, (int)sBtDesc.nNameIndex);
//      sDesc.nOffset   = sBtDesc.nOffset;
//      sDesc.nCount    = sBtDesc.nCount;
//      sDesc.bDynamic  = sBtDesc.bDynamic;
//      sDesc.bConst    = sBtDesc.bConst;
//
//#ifdef DEBUG_DECL_NAME
//      sDesc.Name      = (DataPool::LPCSTR)(lpBase + sDesc.nName);
//      TRACE("VAR:%16s %16s %8d\n", sDesc.GetTypeDesc()->Name, sDesc.Name, sDesc.nOffset);
//#endif // DEBUG_DECL_NAME
//    }
//  }

#endif // #ifndef _HIDING_CODE_

  clsize DataPoolImpl::LocalizePtr()
  {
    GXLPBYTE ptr = (GXLPBYTE)m_Buffer.GetPtr();
    SIZELIST s;
    const GXSIZE_T nSumSize = IntGetRTDescHeader(&s);

    m_aTypes = (TYPE_DESC*)ptr;
    ptr += s.cbTypes;

    m_aStructs = (STRUCT_DESC*)ptr;
    ptr += s.cbStructs;

    m_aHashAlgos = (HASHALGO*)ptr;
    ptr += s.cbHashAlgo;

    m_aHashBuckets = (u8*)ptr;
    ptr += m_cbHashBuckets;

    m_aGSIT = (SortedIndexType*)ptr;
    ptr += s.cbGVSIT;

    m_aVariables = (VARIABLE_DESC*)(ptr);
    ptr += s.cbVariables;

    if(m_nNumOfMember) {
      m_aMembers = (VARIABLE_DESC*)ptr;
      ptr += s.cbMembers;
    }

    if(m_nNumOfEnums) {
      m_aEnums = (ENUM_DESC*)ptr;
      ptr += s.cbEnums;
    }

    m_pNamesTabBegin = (GXUINT*)ptr;
    m_pNamesTabEnd   = (GXUINT*)((GXUINT_PTR)m_pNamesTabBegin + s.cbNameTable);

    return nSumSize;
  }

#ifndef _HIDING_CODE_

  void DataPoolImpl::DbgIntDump()
  {
    TRACE("========= Types =========\n");
    for(GXUINT i = 0; i < m_nNumOfTypes; ++i) {
      TRACE("%16s %8d\n", (DataPool::LPCSTR)m_aTypes[i].GetName(), m_aTypes[i].cbSize);
    }

    TRACE("========= Structs =========\n");
    for(GXUINT i = 0; i < m_nNumOfStructs; ++i) {
      TRACE("%16s %8d\n", (DataPool::LPCSTR)m_aStructs[i].GetName(), m_aStructs[i].cbSize);
    }

    const GXBOOL bNX16B = IS_MARK_NX16B(m_dwRuntimeFlags);

    TRACE("========= Variables =========\n");
    for(GXUINT i = 0; i < m_nNumOfVar; ++i)
    {
      const auto& v = m_aVariables[i];
      if(v.nCount > 1) {
        TRACE("%16s %12s[%4d] %8d[%d]\n",
          (DataPool::LPCSTR)v.TypeName(),
          (DataPool::LPCSTR)v.VariableName(),
          v.nCount, v.nOffset, v.GetUsageSize(bNX16B));
      }
      else {
        TRACE("%16s %16s %10d[%d]\n",
          (DataPool::LPCSTR)v.TypeName(),
          (DataPool::LPCSTR)v.VariableName(),
          v.nOffset, v.GetUsageSize(bNX16B));
      }
    }

    TRACE("========= Members =========\n");
    for(GXUINT i = 0; i < m_nNumOfMember; ++i){
      const auto& v = m_aMembers[i];
      if(v.nCount > 1) {
        TRACE("%16s %12s[%4d] %8d[%d]\n",
          (DataPool::LPCSTR)v.TypeName(),
          (DataPool::LPCSTR)v.VariableName(),
          v.nCount, v.nOffset, v.GetUsageSize(bNX16B));
      }
      else {
        TRACE("%16s %16s %10d[%d]\n",
          (DataPool::LPCSTR)v.TypeName(),
          (DataPool::LPCSTR)v.VariableName(),
          v.nOffset, v.GetUsageSize(bNX16B));
      }
    }
  }

  void DataPoolImpl::LocalizeTables(BUILDTIME& bt, GXSIZE_T cbVarSpace)
  {
    // [MAIN BUFFER 结构表]:
    // #.Type desc table            类型描述表 - 1
    // #.Struct desc table          类型描述表 - 2
    // #.Hash Algorithm table
    // #.Hash buckets
    // #.GVSIT
    // #.Variable desc table        变量描述表
    // #.Struct members desc table  成员变量描述表
    // #.enum desc table            枚举描述表
    // #.Strings Offset table       字符串偏移表, 这个后面一定要接上Strings,有特殊用法
    // #.Strings                    描述表中所有字符串的字符串表
    // #.Variable Data Space        变量空间

    const GXSIZE_T cbNameTable = bt.NameSet.size() * sizeof(GXUINT);

    m_nNumOfTypes    = (GXUINT)bt.m_TypeDict.size() - (GXUINT)bt.m_nNumOfStructs;
    m_nNumOfStructs  = (GXUINT)bt.m_nNumOfStructs;
    m_nNumOfVar      = (GXUINT)bt.m_aVar.size();
    m_nNumOfMember   = (GXUINT)bt.m_aStructMember.size();
    m_nNumOfEnums    = (GXUINT)bt.m_aEnumPck.size();
    m_cbHashBuckets  = (GXUINT)bt.m_nNumOfBuckets;
    m_pNamesTabBegin = (GXUINT*)0;
    m_pNamesTabEnd   = (GXUINT*)cbNameTable;

    SIZELIST s;
    const GXSIZE_T cbHeader = IntGetRTDescHeader(&s);
    //GXSIZE_T cbTypes     = m_nNumOfTypes * sizeof(TYPE_DESC);
    //GXSIZE_T cbStructs   = m_nNumOfStructs * sizeof(STRUCT_DESC);
    //GXSIZE_T cbGVSIT     = (m_nNumOfVar + m_nNumOfMember + m_nNumOfEnums) * sizeof(SortedIndexType);
    //GXSIZE_T cbVariables = m_nNumOfVar * sizeof(VARIABLE_DESC);
    //GXSIZE_T cbMembers   = m_nNumOfMember * sizeof(VARIABLE_DESC);
    //GXSIZE_T cbEnums     = m_nNumOfEnums * sizeof(ENUM_DESC);
    //GXSIZE_T cbHeader    = cbTypes + cbStructs + cbGVSIT+ cbVariables + cbMembers + cbEnums + cbNameTable;
    m_Buffer.Resize(cbHeader + bt.NameSet.buffer_size() + cbVarSpace, FALSE);

#ifdef _DEBUG
    auto cbDbgSave = m_Buffer.GetSize();
#endif // #ifdef _DEBUG

    clstd::STRINGSETDESC* pTable = new clstd::STRINGSETDESC[bt.NameSet.size()];
    bt.NameSet.sort(pTable);

    memset((GXLPBYTE)m_Buffer.GetPtr() + cbHeader, 0, bt.NameSet.buffer_size());

    auto result = bt.NameSet.gather_offset<GXUINT>((GXUINT*)
      ((GXLPBYTE)m_Buffer.GetPtr() + cbHeader - cbNameTable), cbNameTable);
    ASSERT(result);

    GXINT_PTR lpBase = (GXINT_PTR)bt.NameSet.gather(&m_Buffer, cbHeader);
    //m_StringBase = lpBase;

    ASSERT(cbDbgSave == m_Buffer.GetSize()); // 确保GatherToBuffer不会改变Buffer的长度

    LocalizePtr();
    //ASSERT(m_StringBase == lpBase);
    //for(auto p = m_pNamesTabBegin; p != m_pNamesTabEnd; ++p)
    //{
    //  TRACE("%d %s\n", *p, (LPCSTR)m_pNamesTabEnd + (*p));
    //}

    //HASHALGO* pAlgo = &m_aHashAlgos[0];
    //DataPoolBuildTime::HASH_ALGORITHM* pBTAlgo = bt.m_VarHashInfo;
    //if(pBTAlgo->nBucket >= (1 << 14) || pBTAlgo->nPos >= (1 << 16)) {
    //  pAlgo->eType = 0;
    //}
    //else {
    //  pAlgo->eType   = pBTAlgo->eType;
    //  pAlgo->nBucket = pBTAlgo->nBucket;
    //  pAlgo->nPos    = pBTAlgo->nPos;

    //}
    GXUINT cbBuckets = 0;
    cbBuckets = Implement::CopyHashAlgo(&m_aHashAlgos[0], &bt.m_VarHashInfo, m_aHashBuckets, cbBuckets);

    // * 以下复制表的操作中均包含字符串重定位

    // 复制类型描述表
    GXUINT nTypeIndex = 0, nStructIndex = 0;
    TYPE_DESC* pType = NULL;

    for(auto it = bt.m_TypeDict.begin(); it != bt.m_TypeDict.end(); ++it) {
      BUILDTIME::BT_TYPE_DESC& sBtType = it->second;

      if(sBtType.Cate == DataPoolTypeClass::Structure) {
        STRUCT_DESC* pStruct = &m_aStructs[nStructIndex++];
        pType = pStruct;
        pStruct->nMemberCount = sBtType.nMemberCount;
        pStruct->Member = (GXUINT)((GXUINT_PTR)&m_aMembers[sBtType.nMemberIndex] - (GXUINT_PTR)&pStruct->Member);
        cbBuckets = Implement::CopyHashAlgo(&m_aHashAlgos[nStructIndex], &sBtType.HashInfo, m_aHashBuckets, cbBuckets);
      }
      else if(sBtType.Cate == DataPoolTypeClass::Enumeration || sBtType.Cate == DataPoolTypeClass::Flag) {
        STRUCT_DESC* pStruct = &m_aStructs[nStructIndex++];
        pType = pStruct;
        pStruct->nMemberCount = sBtType.nMemberCount;
        pStruct->Member = (GXUINT)((GXUINT_PTR)&m_aEnums[sBtType.nMemberIndex] - (GXUINT_PTR)&pStruct->Member);
      }
      else {
        pType = &m_aTypes[nTypeIndex++];
      }

      pType->nName        = (GXUINT)clstd::StringSetA::IndexToOffset(pTable, (int)sBtType.nNameIndex);
      pType->Cate         = sBtType.Cate;
      pType->cbSize       = sBtType.cbSize;

      sBtType.nTypeAddress = (GXINT_PTR)pType;

#ifdef DEBUG_DECL_NAME
      pType->Name = (DataPool::LPCSTR)(lpBase + pType->nName);
      TRACE("TYPE[%d]: %16s %8d\n", 0, pType->Name, pType->cbSize);
#endif // DEBUG_DECL_NAME

    }

    ASSERT(m_nNumOfTypes == nTypeIndex);
    ASSERT(m_nNumOfStructs == nStructIndex);
    ASSERT(m_cbHashBuckets == cbBuckets);

    // 复制变量和成员变量描述表
    Implement::CopyVariables(m_aVariables, &bt.m_aVar, pTable, lpBase);
    Implement::CopyVariables(m_aMembers, &bt.m_aStructMember, pTable, lpBase);

    // 复制枚举描述表
    GXUINT nIndex = 0;
    for(auto it = bt.m_aEnumPck.begin(); it != bt.m_aEnumPck.end(); ++it, ++nIndex) {
      const ENUM_DESC& sBtDesc = reinterpret_cast<ENUM_DESC&>(*it);
      ENUM_DESC& sDesc = m_aEnums[nIndex];

      sDesc.nName = (GXUINT)clstd::StringSetA::IndexToOffset(pTable, (int)sBtDesc.nName);
      sDesc.Value = sBtDesc.Value;

#ifdef DEBUG_DECL_NAME
      sDesc.Name  = (DataPool::LPCSTR)(lpBase + sDesc.nName);
#endif // DEBUG_DECL_NAME
    }

    // #.这个依赖于有效的 m_aVariables， m_aMembers 和 m_aEnums
    // #.排序依赖于描述中的 nName 成员，它会按递增排序
    // #.排序不改变m_aVariables， m_aMembers 和 m_aEnums上的顺序，
    //   而是生成有序的基于这些数组的索引，放到GSIT上
    // #.nName 在运行时使用了自定位方法，自定位化后它的值不能保证在
    //   GSIT上是递增的，所以自定位化放在后面进行
    GenGSIT();

    // 自定位化转换
    SelfLocalizable(m_aTypes,     m_nNumOfTypes,   lpBase);
    SelfLocalizable(m_aStructs,   m_nNumOfStructs, lpBase);
    SelfLocalizable(m_aVariables, m_nNumOfVar,     lpBase);
    SelfLocalizable(m_aMembers,   m_nNumOfMember,  lpBase);
    SelfLocalizable(m_aEnums,     m_nNumOfEnums,   lpBase);

    new(&m_VarBuffer) clstd::RefBuffer((GXLPBYTE)lpBase + bt.NameSet.buffer_size(), cbVarSpace);
    memset(m_VarBuffer.GetPtr(), 0, m_VarBuffer.GetSize());   // 只清除变量段的内存

    SAFE_DELETE_ARRAY(pTable);
  }

  void DataPoolImpl::GenGSIT()
  {
    SortNames<VARIABLE_DESC>(m_aVariables, GSIT_Variables, 0, m_nNumOfVar);

    for(GXUINT i = 0; i < m_nNumOfStructs; ++i) {
      const STRUCT_DESC& t = m_aStructs[i];
      
      ASSERT(
        t.Cate == DataPoolTypeClass::Structure ||
        t.Cate == DataPoolTypeClass::Flag ||
        t.Cate == DataPoolTypeClass::Enumeration);

      if(t.nMemberCount >= 1)
      {
        switch(t.Cate)
        {
        case DataPoolTypeClass::Structure:
          //SortNames<VARIABLE_DESC>(m_aMembers, GSIT_Members + t.nMemberIndex, t.nMemberIndex, t.nMemberCount);
          //TRACE("var index:%d\n", t.GetMemberIndex(m_aMembers));
          SortNames<DATAPOOL_VARIABLE_DESC>(t.GetMembers(), GSIT_Members + t.GetMemberIndex(m_aMembers), 0, t.nMemberCount);
          break;
        case DataPoolTypeClass::Flag:
        case DataPoolTypeClass::Enumeration:
          //SortNames<ENUM_DESC>(m_aEnums, GSIT_Enums + t.nMemberIndex, t.nMemberIndex, t.nMemberCount);
          //TRACE("enum index:%d\n", t.GetEnumIndex(m_aEnums));
          SortNames<DATAPOOL_ENUM_DESC>(t.GetEnumMembers(), GSIT_Enums + t.GetEnumIndex(m_aEnums), 0, t.nMemberCount);
          break;
        }
      }
    }
  }

  template<class DescT>
  void DataPoolImpl::SelfLocalizable(DescT* pDescs, int nCount, GXINT_PTR lpBase)
  {
    for(int i = 0; i < nCount; ++i)
    {
      GXUINT* pName = &pDescs[i].nName;
      ASSERT(lpBase - (GXINT_PTR)pName > 0); // 保证 lpBase 在 &nName 的后面出现

      *pName = (GXUINT)(lpBase - (GXINT_PTR)pName + *pName);

#ifdef DEBUG_DECL_NAME
      DataPool::LPCSTR szName = (DataPool::LPCSTR)((GXINT_PTR)&pDescs[i].nName + pDescs[i].nName);
      ASSERT(szName == pDescs[i].Name || pDescs[i].Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
    }
  }

  template<class DescT>
  void DataPoolImpl::SortNames( const DescT* pDescs, SortedIndexType* pDest, int nBegin, int nCount)
  {
    //ASSERT(pDest == &m_aGVSIT[nTargetTopIndex]);
    struct CONTEXT
    {
      GXUINT index;
      GXUINT name;

      b32 SortCompare(const CONTEXT& b) const
      {
        return name > b.name;
      }

      void SortSwap(CONTEXT& b)
      {
        clSwapX(index, b.index);
        clSwapX(name, b.name);
      }
    };

    CONTEXT* pContext = new CONTEXT[nCount];
    auto prev = pDescs[nBegin].nName;
    GXUINT mask = 0;
    for(int i = 0; i < nCount; ++i)
    {
      //pContext[i].index = nBegin + i; // GVSIT的索引已经加了Group的偏移
      //pContext[i].name = pDescs[i + nBegin].nName;
      pContext[i].index = i; // GVSIT的索引已经加了Group的偏移
      pContext[i].name = pDescs[i].nName;
      //m_aGVSIT[nTargetTopIndex + i] = pContext[i].index;
      pDest[i] = pContext[i].index;

      //TRACE("name:%d\n", pContext[i].name);

      mask |= pContext[i].name - prev;
      prev = pContext[i].name;
    }

    if(mask >> 31) // 检查名字是不是已经按照从大到校的顺序排序了
    {
      clstd::QuickSort(pContext, 0, nCount);
#ifdef _DEBUG
      prev = pContext->index;
#endif
      for(int i = 0; i < nCount; ++i)
      {
        //m_aGVSIT[nTargetTopIndex + i] = pContext[i].index;
        pDest[i] = pContext[i].index;
#ifdef _DEBUG
        ASSERT(pDescs[pContext[i].index].nName >= pDescs[prev].nName);
        prev = pContext[i].index;
#endif
      }
    }

    SAFE_DELETE_ARRAY(pContext);
  }

  GXBOOL DataPoolImpl::IntFindEnumFlagValue( LPCSD pTypeDesc, LPCSTR szName, EnumFlag* pOutEnumFlag ) const
  {
    const auto* p = GSIT_Enums + pTypeDesc->GetEnumIndex(m_aEnums);
    int begin = 0;
    int end = pTypeDesc->nMemberCount;
    LPCED aEnums = reinterpret_cast<LPCED>(pTypeDesc->GetEnumMembers());

    while(begin <= end - 1)
    {
      int mid = (begin + end) >> 1;
      ASSERT(p[mid] < m_nNumOfEnums); // 索引肯定低于总大小
      auto& pair = aEnums[p[mid]];
      auto result = GXSTRCMP(szName, pair.GetName());
      if(result < 0) {
        end = mid;
      }
      else if(result > 0){
        if(begin == mid) {
          break;
        }
        begin = mid;
      }
      else {
        *pOutEnumFlag = pair.Value;
        return TRUE;
      }
    }

//#if 0
//#ifdef ENABLE_DATAPOOL_WATCHER
//    for (GXUINT i = 0; i < pTypeDesc->nMemberCount; i++) {
//      TRACE("%*s %s\n", -50, aEnums[p[i]].GetName(), aEnums[p[i]].Name);
//    }
//#endif // ENABLE_DATAPOOL_WATCHER
//#endif // #ifdef _DEBUG

    return FALSE;
  }

#endif // #ifndef _HIDING_CODE_

  //////////////////////////////////////////////////////////////////////////
#ifndef DISABLE_DATAPOOL_WATCHER
  GXBOOL DataPoolImpl::IntAddToWatchDict(WatchFixedDict& sDict, GXLPVOID key, ImpulseProc pImpulseCallback, GXLPARAM lParam)
  {
    WatchFixedList sWatchList;
    auto result = sDict.insert(clmake_pair(key, sWatchList));

    WATCH_FIXED sWatch;
    sWatch.pCallback = pImpulseCallback;
    sWatch.lParam    = lParam;

    // DataPoolWatch 对象
    if((GXINT_PTR)pImpulseCallback == 0) {
      ((DataPoolWatcher*)lParam)->AddRef();
    }

    result.first->second.insert(sWatch);
    return TRUE;
  }

  GXBOOL DataPoolImpl::IntRemoveFromWatchDict(WatchFixedDict& sDict, GXLPVOID key, ImpulseProc pImpulseCallback, GXLPARAM lParam)
  {
    auto itWatchSet = sDict.find(key);
    if(itWatchSet != sDict.end())
    {
      WatchFixedList& sVarWatchSet = itWatchSet->second;
      WATCH_FIXED sWatch;
      sWatch.pCallback = pImpulseCallback;
      sWatch.lParam    = lParam;

      auto itWatch = sVarWatchSet.find(sWatch);
      if(itWatch != sVarWatchSet.end())
      {
        // DataPoolWatch 对象
        if((GXINT_PTR)itWatch->pCallback == 0) {
          DataPoolWatcher*& pWatch = *(DataPoolWatcher**)&itWatch->lParam;
          SAFE_RELEASE(pWatch);
        }
        sVarWatchSet.erase(itWatch);

        if(sVarWatchSet.empty()) {
          sDict.erase(itWatchSet);
        }
        return TRUE;
      }
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::IntWatch( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    const auto dwFlags = pVar->GetCaps();
    const auto dwBan = DataPoolVariable::CAPS_ARRAY;

    // 1.静态数组对象不能被watch
    // (1)元素不会增加或减少，不会因此产生监视事件
    // (2)元素的改变不会发生对象的监视事件
    // (3)无法通过地址来区分是数组对象还是数组第一个元素
    // 2.结构体对象不能被watch
    // (1)改变了结构体的成员函数，无法回溯到它的结构体再推送事件，或者说这样做不经济
    // (2)无法通过地址来区别是结构体还是结构体第一个成员

    if(TEST_FLAG(dwFlags, DataPoolVariable::CAPS_STRUCT)) {
      //CLBREAK;
      CLOG_ERROR("can not watch datapool struct(\"%s\").", pVar->GetName());
      return FALSE;
    }

    if(TEST_FLAG(dwFlags, DataPoolVariable::CAPS_FIXED))
    {
      if(TEST_FLAG_NOT(dwFlags, dwBan)) {
        return IntAddToWatchDict(m_FixedDict, pVar->GetPtr(), pImpulseCallback, lParam);
      }
    }
    else {
      auto it = m_WatchableArray.find((DataPoolArray*)pVar->GetBuffer());
      if(it != m_WatchableArray.end()) {
        return IntAddToWatchDict(it->second, (GXLPVOID)pVar->GetOffset(), pImpulseCallback, lParam);
      }
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::IntIgnore( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    // TODO: 这段代码和IntWatch相似，稳定后可以用模板合并
    const auto dwFlags = pVar->GetCaps();
    if(TEST_FLAG(dwFlags, DataPoolVariable::CAPS_FIXED)) {
      return IntRemoveFromWatchDict(m_FixedDict, pVar->GetPtr(), pImpulseCallback, lParam);
    }
    else {
      auto it = m_WatchableArray.find((DataPoolArray*)pVar->GetBuffer());
      if(it != m_WatchableArray.end()) {
        return IntRemoveFromWatchDict(it->second, (GXLPVOID)pVar->GetOffset(), pImpulseCallback, lParam);
      }
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Watch( GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, pImpulseCallback, lParam);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Watch( GXLPCSTR szExpression, DataPoolWatcher* pWatcher )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, NULL, (GXLPARAM)pWatcher);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Watch( GXLPCSTR szExpression, GXHWND hWnd )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, (ImpulseProc)1, (GXLPARAM)hWnd);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Watch( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    return IntWatch(pVar, pImpulseCallback, lParam);
  }

  GXBOOL DataPoolImpl::Watch( DataPoolVariable* pVar, DataPoolWatcher* pWatcher )
  {
    return IntWatch(pVar, NULL, (GXLPARAM)pWatcher);
  }

  GXBOOL DataPoolImpl::Watch( DataPoolVariable* pVar, GXHWND hWnd )
  {
    return IntWatch(pVar, (ImpulseProc)1, (GXLPARAM)hWnd);
  }

  GXBOOL DataPoolImpl::Ignore( GXLPCSTR szExpression, ImpulseProc pImpulseCallback )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, pImpulseCallback, NULL);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Ignore( GXLPCSTR szExpression, DataPoolWatcher* pWatcher )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, NULL, (GXLPARAM)pWatcher);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Ignore( GXLPCSTR szExpression, GXHWND hWnd )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, (ImpulseProc)1, (GXLPARAM)hWnd);
    }
    return FALSE;
  }

  GXBOOL DataPoolImpl::Ignore( DataPoolVariable* pVar, ImpulseProc pImpulseCallback )
  {
    return IntIgnore(pVar, pImpulseCallback, NULL);
  }

  GXBOOL DataPoolImpl::Ignore( DataPoolVariable* pVar, DataPoolWatcher* pWatcher )
  {
    return IntIgnore(pVar, NULL, (GXLPARAM)pWatcher);
  }

  GXBOOL DataPoolImpl::Ignore( DataPoolVariable* pVar, GXHWND hWnd )
  {
    return IntIgnore(pVar, (ImpulseProc)1, (GXLPARAM)hWnd);
  }

#endif // #ifndef DISABLE_DATAPOOL_WATCHER

  //////////////////////////////////////////////////////////////////////////
  //
  // 保存用的内部结构体
  //
  //typedef clvector<GXUINT>      UIntArray;
  struct BUFFER_SAVELOAD_DESC // 用于读写时临时储存的buffer描述
  {
    enum RelocalizeType : GXDWORD// 重定位表附加描述（32位）
    {
      RelocalizeType_Array   = (0 << 28),
      RelocalizeType_String  = (1 << 28),
      RelocalizeType_StringA = (2 << 28),
      RelocalizeType_Object  = (3 << 28),
      RelocalizeTypeMask     = 0xf0000000,
      RelocalizeOffsetMask   = ~RelocalizeTypeMask,
    };

    clBufferBase* pBuffer;
    //UIntArray     RelTable;   // 重定位表, 参考RelocalizeType
    // 2号重定位表，用来开发从函数收集的方法，这个成熟后去掉1号表，去掉文件记录
    GXUIntArray   RelTable;  // 重定位表, 参考RelocalizeType, 这个是平台无关的，指针按照32位计算
    //DataPoolImpl::LPCTD    pTypeDesc;
    const DATAPOOL_TYPE_DESC* pTypeDesc;

    // 模板函数原型： fn(RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc); 注意最后两个要是引用
    template<class _Fn>
    GXUINT_PTR RelocalizePtr(clBufferBase* pDestBuffer, const clBufferBase* pSourceBuffer, _Fn fn)
    {
      clsize nCopy;

      if(RelTable.empty()) {
        // 不含指针的buffer大小一定相等
        ASSERT(pDestBuffer->GetSize() == pSourceBuffer->GetSize());

        nCopy = pSourceBuffer->GetSize();
        memcpy(pDestBuffer->GetPtr(), pSourceBuffer->GetPtr(), nCopy);
        return nCopy;
      }
      GXLPBYTE pDest = (GXLPBYTE)pDestBuffer->GetPtr();
      GXLPCBYTE pSrc = (GXLPCBYTE)pSourceBuffer->GetPtr();
      GXSIZE_T nRelOffset = 0; // 这个是平台相关的，要注意64bits下的修改

      for(auto itRel = RelTable.begin(); itRel != RelTable.end(); ++itRel)
      {
        auto& offset = *itRel;
        nCopy = (offset & RelocalizeOffsetMask) - nRelOffset;
        if(nCopy)
        {
          memcpy(pDest, pSrc, nCopy);
          pDest += nCopy;
          pSrc += nCopy;
        }

        fn((RelocalizeType)(offset & RelocalizeTypeMask),
          offset & RelocalizeOffsetMask, pDest, pSrc);

        nRelOffset += (nCopy + sizeof(GXUINT));
        //nRelOffset += (nCopy + SIZEOF_PTR);
      }

      nCopy = (clsize)pSourceBuffer->GetPtr() + pSourceBuffer->GetSize() - (clsize)pSrc;
      if(nCopy) {
        memcpy(pDest, pSrc, nCopy);
      }

      return ((GXUINT_PTR)pDest - (GXUINT_PTR)pDestBuffer->GetPtr() + nCopy);
    }

    static clsize GetPtrAdjustSize(clsize nCountOfRelTab) // 指针修正尺寸
    {
      // 磁盘上指针都被转换为32位无符号整数
      return (SIZEOF_PTR - sizeof(GXUINT)) * nCountOfRelTab;
    }

    clsize GetPtrAdjustSize() const // 指针修正尺寸
    {
      return GetPtrAdjustSize(RelTable.size());
    }

    clsize GetDiskBufferSize() const
    {
      // 最终尺寸应该考虑减去把所有指针转换为4字节整数的差值
      return pBuffer->GetSize() - GetPtrAdjustSize();
    }

    void GenerateRelocalizeTable(const DATAPOOL_TYPE_DESC* pTypeDesc)
    {
      // 外部保证这个, 全局变量使用另一个重载方法
      ASSERT(pTypeDesc != NULL);

      // 使用这个缓冲上的动态数组必须匹配, 大小也肯定是类型长度的整数倍, 空的话表示这个是全局变量
      ASSERT((pBuffer->GetSize() % pTypeDesc->cbSize) == 0);

      const GXSIZE_T nCount = pBuffer->GetSize() / pTypeDesc->cbSize;

      GXUINT nBase = 0; // 基础偏移
      switch(pTypeDesc->Cate)
      {
      case DataPoolTypeClass::Structure:
        for(GXUINT i = 0; i < nCount; ++i) {
          DataPoolImpl::LPCSD pStructDesc = reinterpret_cast<DataPoolImpl::LPCSD>(pTypeDesc);
          nBase = GenerateRelocalizeTable(nBase, reinterpret_cast<DataPoolImpl::LPCVD>(pStructDesc->GetMembers()), pStructDesc->nMemberCount);
        }
        break;
      case DataPoolTypeClass::String:  GenerateRelocalizeTable(nBase, RelocalizeType_String, nCount);   break;
      case DataPoolTypeClass::StringA: GenerateRelocalizeTable(nBase, RelocalizeType_StringA, nCount);  break;
      case DataPoolTypeClass::Object:  GenerateRelocalizeTable(nBase, RelocalizeType_Object, nCount);   break;
      }
    }

    GXUINT GenerateRelocalizeTable(GXUINT nBase, RelocalizeType eRelType, GXSIZE_T nCount)
    {
      for(GXUINT n = 0; n < nCount; ++n) {
        RelTable.push_back(nBase | eRelType);
        nBase += SIZEOF_PTR32;
      }
      return nBase;
    }

    // 递归收集重定位表,平台无关，指针按照4字节计算
    GXUINT GenerateRelocalizeTable(GXUINT nBase, const DATAPOOL_VARIABLE_DESC* pVarDesc, GXSIZE_T nCount)
    {
      for(GXUINT i = 0; i < nCount; ++i)
      {
        const DATAPOOL_VARIABLE_DESC& vd = pVarDesc[i];

        if(vd.IsDynamicArray())
        {
          RelTable.push_back(nBase | RelocalizeType_Array);
          nBase += SIZEOF_PTR32;
        }
        else
        {
          ASSERT(vd.nCount >= 1);

          switch(vd.GetTypeClass())
          {
          case DataPoolTypeClass::Structure:
            for(GXUINT n = 0; n < vd.nCount; ++n) {
              nBase = GenerateRelocalizeTable(nBase, vd.MemberBeginPtr(), vd.MemberCount());
            }
            break;

          case DataPoolTypeClass::String:  nBase = GenerateRelocalizeTable(nBase, RelocalizeType_String, vd.nCount);   break;
          case DataPoolTypeClass::StringA: nBase = GenerateRelocalizeTable(nBase, RelocalizeType_StringA, vd.nCount);  break;
          case DataPoolTypeClass::Object:  nBase = GenerateRelocalizeTable(nBase, RelocalizeType_Object, vd.nCount);   break;
          default:        nBase += reinterpret_cast<const DataPoolImpl::VARIABLE_DESC&>(vd).GetCompactSize();  break;
          } // switch
        }
      } // for
      return nBase;
    }

    //void DbgCheck() // 验证文件记录和自己收集的重定位表一致性，收集算法稳定后去掉表1
    //{
    //  ASSERT(RelTable.size() == RelTable2.size());

    //  clsize count = RelTable.size();
    //  for(clsize i = 0; i < count; ++i)
    //  {
    //    ASSERT(RelTable[i] == RelTable2[i]);
    //  }
    //}
  };
  typedef clvector<BUFFER_SAVELOAD_DESC> BufDescArray;
  
  GXBOOL DataPoolImpl::Save( GXLPCWSTR szFilename )
  {
    clFile file;
    if(file.CreateAlways(szFilename)) {
      return Save(file);
    }
    CLOG_ERRORW(_CLTEXT("can not open file \"%s\"."), szFilename);
    return FALSE;
  }

#define SAVE_TRACE TRACE

  GXBOOL DataPoolImpl::Save( clFile& file )
  {
    //
    // TODO: 改为SmartRepository储存
    //
#ifdef DEBUG_DECL_NAME
    TRACE("打开名字调试模式,文件结构体中含有指针，要关闭这个宏才能保存");
    CLBREAK;
#else
    // #.[FILE_HEADER]
    // #.Variable space buffer header
    // #.Array buffer header[0]
    // #.Array buffer header[1]
    // ...
    // #.DataPool::m_Buffer - Variable space
    // #.字符串变量，字符串列表
    // #.Variable space 的重定位表 + Variable data
    // #.Array Buffer[0] 的重定位表 + data
    // #.Array Buffer[1] 的重定位表 + data
    // ...
    FILE_HEADER header;
    const GXDWORD dwFlagsMask = DataPoolCreation_NoHashTable |
      DataPoolCreation_Align4 | DataPoolCreation_Align8 |
      DataPoolCreation_Align16 | DataPoolCreation_NotCross16BytesBoundary;

    header.dwFlags          = m_dwRuntimeFlags & dwFlagsMask; // 只接受储存部分标志，因为有些标志是运行态的，文件态没有意义
    header.dwHashMagic      = clstd::HashStringT("DataPool", 8);
    header.nNumOfTypes      = m_nNumOfTypes;
    header.nNumOfStructs    = m_nNumOfStructs;
    header.nNumOfVar        = m_nNumOfVar;
    header.nNumOfMember     = m_nNumOfMember;
    header.nNumOfEnums      = m_nNumOfEnums;
    header.cbNames          = (GXUINT)IntGetRTDescNames();
    header.cbHashBuckets    = m_cbHashBuckets;
    header.cbVariableSpace  = (GXUINT)m_VarBuffer.GetSize();
    header.nNumOfStrings    = 0;
    header.cbStringSpace    = 0;
    header.nNumOfArrayBufs  = 0;
    header.nNumOfNames      = (GXUINT)(m_pNamesTabEnd - m_pNamesTabBegin);
    //header.cbArraySpace     = 0;



    clstd::StringSetW sStringVar; // 字符串变量集合
    clstd::StringSetA sStringVarA; // 字符串变量集合
    GXUINT nRelOffset = 0;  // 重定位表的开始偏移
    //UIntArray     RelocalizeTab;  
    BufDescArray  BufferTab;
    BUFFER_SAVELOAD_DESC bd;
    BUFFER_SAVELOAD_DESC* pCurrBufDesc;
    bd.pBuffer = &m_VarBuffer;
    //bd.nBeginOfRel = 0;
    bd.pTypeDesc = NULL;
    BufferTab.push_back(bd);
    pCurrBufDesc = &BufferTab.back();

#ifdef _DEBUG
    typedef clset<clBufferBase*>  BufSet;
    BufSet        sDbgBufSet;
#endif // #ifdef _DEBUG

    // 变量预处理：
    // 1.收集字符串变量集合
    // 2.收集重定位表
    auto _itBegin = begin();
    auto _itEnd = end();
    DataPoolUtility::EnumerateVariables2
      <DataPoolUtility::iterator, DataPoolUtility::element_iterator, DataPoolUtility::element_reverse_iterator>
      (_itBegin, _itEnd, 
      [&]
    (int bArray, DataPool::iterator& it, int nDepth) 
    {
      MOVariable var = it.ToVariable();
      const auto pCheckBuffer = it.pBuffer;
      if(pCheckBuffer != pCurrBufDesc->pBuffer) // 切换了Buffer
      {
        //TRACE("%08x:%08x\n", pCheckBuffer, pCurrBufDesc->pBuffer);

#ifdef _DEBUG
        // sDbgBufSet存了之前的buffer，新的buffer一定不在这个集合里
        ASSERT(sDbgBufSet.find(pCheckBuffer) == sDbgBufSet.end());
        sDbgBufSet.insert(pCheckBuffer);
#endif // #ifdef _DEBUG

        // 这个buffer结束时，累计偏移一定等于buffer大小
        ASSERT_X86(nRelOffset == pCurrBufDesc->pBuffer->GetSize());

        ++pCurrBufDesc;

        // 一定在下面“if(bArray)”里注册过，并且也是按照顺序出现的
        ASSERT(pCurrBufDesc->pBuffer == pCheckBuffer);
        nRelOffset = 0;
      }

      ASSERT(bArray || it.pVarDesc->GetTypeClass() != DataPoolTypeClass::Structure);



      if(bArray)
      {
        ASSERT(it.pVarDesc->IsDynamicArray() && it.index == (GXUINT)-1);
        auto pChildBuf = it.child_buffer();
        if(pChildBuf)
        {
          if(pChildBuf->GetSize())
          {
            ++header.nNumOfArrayBufs;
            
            bd.pBuffer = pChildBuf;
            bd.pTypeDesc = it.pVarDesc->GetTypeDesc();
            const GXINT_PTR nCurOffset = (GXINT_PTR)pCurrBufDesc - (GXINT_PTR)&BufferTab.front();
            BufferTab.push_back(bd);
            pCurrBufDesc = (BUFFER_SAVELOAD_DESC*)((GXINT_PTR)&BufferTab.front() + nCurOffset); // vector指针改变，这里更新一下指针
          }
        }
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_Array);

        //if(var.IsValid())
        //{
        //  SAVE_TRACE("0.Dyn buffer:%08x\n", it.pBuffer);
        //  SAVE_TRACE("  Dyn buffer:%08x x%d o%d %s\n", it.child_buffer(), it.child_buffer()->GetSize(), nRelOffset, it.FullNameA());
        //}
        //TRACE("*%d\n", nRelOffset);
        nRelOffset += SIZEOF_PTR32;
      }
      else if(it.pVarDesc->GetTypeClass() == DataPoolTypeClass::String)
      {
        ASSERT( ! bArray);
        sStringVar.insert(var.ToStringW());
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_String);
        nRelOffset += SIZEOF_PTR32;
        ASSERT_X86(var.GetSize() == 4);
        ++header.nNumOfStrings;
      }
      else if(it.pVarDesc->GetTypeClass() == DataPoolTypeClass::StringA)
      {
        ASSERT( ! bArray);
        sStringVarA.insert(var.ToStringA());
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_StringA);
        nRelOffset += SIZEOF_PTR32;
        ASSERT_X86(var.GetSize() == 4);
        ++header.nNumOfStrings;
      }
      else if(it.pVarDesc->GetTypeClass() == DataPoolTypeClass::Object)
      {
        ASSERT( ! bArray);
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_Object);
        nRelOffset += SIZEOF_PTR32;
        ASSERT_X86(var.GetSize() == 4);
      }
      else
      {
        ASSERT(var.IsValid());
        nRelOffset += var.GetSize();
      }

      // 累计偏移验证
      ASSERT(nRelOffset <= it.pBuffer->GetSize());
    });


    if(SIZEOF_PTR > SIZEOF_PTR32) // 运行时64位平台指针到磁盘文件32位无符号整数
    {      
      ASSERT(!BufferTab.empty()); // 至少有m_VarBuffer
      header.cbVariableSpace = (GXUINT)BufferTab.front().GetDiskBufferSize();
    }

    header.nBufHeaderOffset   = (GXUINT)(sizeof(FILE_HEADER));
    header.nDescOffset        = (GXUINT)(sizeof(FILE_HEADER) + sizeof(FILE_BUFFERHEADER) * (header.nNumOfArrayBufs + 1));
    header.nStringVarOffset   = (GXUINT)(header.nDescOffset + (m_Buffer.GetSize() - m_VarBuffer.GetSize()));
    header.cbStringSpace      = (GXUINT)(sStringVar.buffer_size() + sStringVarA.buffer_size());
    header.nBuffersOffset     = (GXUINT)(header.nStringVarOffset + header.cbStringSpace);

    header.nNumOfPtrVars      = (GXUINT)BufferTab.front().RelTable.size();


    clBuffer BufferToWrite; // 临时使用的缓冲区

    // 文件头
    V_WRITE(file.Write(&header, sizeof(FILE_HEADER)), "Failed to write file header.");


    // 数据缓冲信息头
    ASSERT(file.GetPointer() == header.nBufHeaderOffset); // 当前指针与buffer描述表开始偏移一致
    FILE_BUFFERHEADER fbh;
    for(auto it = BufferTab.begin(); it != BufferTab.end(); ++it)
    {
      fbh.nBufferSize = (GXUINT)it->GetDiskBufferSize();
      fbh.nNumOfRel   = (GXUINT)it->RelTable.size();
      fbh.nType       = it->pTypeDesc == NULL ? 0
        : header.nDescOffset + (GXUINT)((GXUINT_PTR)it->pTypeDesc - (GXUINT_PTR)m_aTypes);

      SAVE_TRACE("save buffer type:%s\n", it->pTypeDesc ? (DataPool::LPCSTR)it->pTypeDesc->GetName() : "<global>");

      //SAVE_TRACE("1.Buffer Ptr:%08x\n", it->pBuffer);
      ASSERT(fbh.nBufferSize != 0);

      V_WRITE(file.Write(&fbh, sizeof(fbh)), "Failed to write fbh.");
    }



    // 去掉变量空间的 DataPool::m_Buffer
    //DbgIntDump();
    ASSERT(file.GetPointer() == header.nDescOffset);
    if(SIZEOF_PTR > SIZEOF_PTR32)
    {
      BufferToWrite.Resize((GXUINT)m_Buffer.GetSize() - m_VarBuffer.GetSize(), FALSE);
      memcpy(BufferToWrite.GetPtr(), m_Buffer.GetPtr(), BufferToWrite.GetSize());

      // 这段儿地址计算参考[MAIN BUFFER 结构表]
      const GXUINT_PTR nDeltaVarToType = (GXUINT_PTR)m_aVariables - (GXUINT_PTR)m_aTypes;
      //const GXUINT_PTR nDeltaMemberToType = (GXUINT_PTR)m_aMembers - (GXUINT_PTR)m_aTypes;
      IntChangePtrSize(4, (VARIABLE_DESC*)((GXUINT_PTR)BufferToWrite.GetPtr() + nDeltaVarToType), m_nNumOfVar, IS_MARK_NX16B(m_dwRuntimeFlags));
      IntClearChangePtrFlag((TYPE_DESC*)BufferToWrite.GetPtr(), m_nNumOfTypes);
      IntClearChangePtrFlag((STRUCT_DESC*)((GXINT_PTR)BufferToWrite.GetPtr() + (m_nNumOfTypes * sizeof(TYPE_DESC))), m_nNumOfStructs);

      V_WRITE(file.Write(BufferToWrite.GetPtr(), (GXUINT)BufferToWrite.GetSize()), "Failed to write global buffer.");
    }
    else {
      V_WRITE(file.Write(m_Buffer.GetPtr(), (GXUINT)m_Buffer.GetSize() - header.cbVariableSpace), "Failed to write global buffer.");
    }


    
    // 字符串变量的字符串列表
    clFixedBuffer StringVarBuf;
    ASSERT(file.GetPointer() == header.nStringVarOffset); // 当前指针与字符串变量表开始偏移一致
    StringVarBuf.Resize(header.cbStringSpace, TRUE);
    sStringVar.gather(&StringVarBuf, 0);
    sStringVarA.gather(&StringVarBuf, sStringVar.buffer_size());
    V_WRITE(file.Write(StringVarBuf.GetPtr(), (GXUINT)StringVarBuf.GetSize()), "Failed to write variable string buffer.");


    // 数据缓冲的数据
    GXUINT nBufferIndex = 1;
    for(auto it = BufferTab.begin(); it != BufferTab.end(); ++it)
    {
      SAVE_TRACE("2.Buffer Ptr:%08x %zu\n", (clsize)it->pBuffer, it->pBuffer->GetSize());

      BufferToWrite.Resize(it->GetDiskBufferSize(), FALSE);

      const auto nCheck = it->RelocalizePtr(&BufferToWrite, it->pBuffer, [&]
      (BUFFER_SAVELOAD_DESC::RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc)
      {
        //SAVE_TRACE("rel offset:%d\n", (GXINT_PTR)pSrc - (GXINT_PTR)it->pBuffer->GetPtr());
        switch(type)
        {
        case BUFFER_SAVELOAD_DESC::RelocalizeType_String:
          {
            clStringW* pStr = (clStringW*)pSrc;
            if(pStr) {
              auto itSetSet = sStringVar.find(*pStr);
              ASSERT(itSetSet != sStringVar.end());
              *(GXUINT*)pDest = (GXUINT)(itSetSet->second.offset + header.nStringVarOffset);
            }
          }
          break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_StringA:
        {
          clStringA* pStr = (clStringA*)pSrc;
          if (pStr) {
            auto itSetSet = sStringVarA.find(*pStr);
            ASSERT(itSetSet != sStringVarA.end());
            *(GXUINT*)pDest = (GXUINT)(itSetSet->second.offset + header.nStringVarOffset + sStringVar.buffer_size());
          }
        }
        break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_Array:
          {
            clBufferBase** ppBuf = (clBufferBase**)pSrc;
            if((*ppBuf) && (*ppBuf)->GetSize())
            {
              ASSERT(*ppBuf == BufferTab[nBufferIndex].pBuffer);
              *(GXUINT*)pDest = nBufferIndex;
              ++nBufferIndex;
            }
            else {
              *(GXUINT*)pDest = 0; // 长度为0的buffer处理为空
            }
          }
          break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_Object:
          {
            *(GXUINT*)pDest = 0;
          }
          break;
        default:
          CLBREAK;
          break;
        }

        pDest += sizeof(GXUINT);
        pSrc += SIZEOF_PTR;
      });

      ASSERT(nCheck == BufferToWrite.GetSize());

      //TRACE("file ptr:%d\n", file.GetPointer());
      // 重定位表
      //if( ! it->RelTable.empty())
      //{
      //  V_WRITE(file.Write(&it->RelTable.front(), (GXUINT)it->RelTable.size() * sizeof(GXUINT)), "Failed to write relocalize table.");
      //}
      // buffer data
      V_WRITE(file.Write(BufferToWrite.GetPtr(), (GXUINT)BufferToWrite.GetSize()), "Failed to write data buffer.");
    }

    SAVE_TRACE("Arrays:%d Strings:%d\n", header.nNumOfArrayBufs, header.nNumOfStrings);
    SAVE_TRACE("=======================================================\n");
#endif // #ifdef DEBUG_DECL_NAME
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL DataPoolImpl::Load( clFile& file, DataPoolCreation dwFlags )
  {
    //
    // TODO: 改为SmartRepository加载
    //
    ASSERT(m_Buffer.GetSize() == 0); // 有效的DataPool对象不能执行Load方法

#ifdef DEBUG_DECL_NAME
    TRACE("打开名字调试模式,文件结构体中含有指针，要关闭这个宏才能加载");
    CLBREAK;
#else

    FILE_HEADER header;
    u32 uNumOfBytesRead;
    V_READ(file.Read(&header, sizeof(FILE_HEADER), &uNumOfBytesRead), "Can not load file header.");
    ASSERT(uNumOfBytesRead == sizeof(FILE_HEADER));

    if(header.dwHashMagic != clstd::HashStringT("DataPool", 8)) {
      CLOG_ERROR("%s : Hash magic does not match.\n", __FUNCTION__);
      return FALSE;
    }

    m_dwRuntimeFlags |= header.dwFlags; // TODO: 检查
    m_nNumOfTypes    = header.nNumOfTypes;
    m_nNumOfStructs  = header.nNumOfStructs;
    m_nNumOfVar      = header.nNumOfVar;
    m_nNumOfMember   = header.nNumOfMember;
    m_nNumOfEnums    = header.nNumOfEnums;
    m_cbHashBuckets  = header.cbHashBuckets;
    m_pNamesTabBegin = (GXUINT*)0;
    m_pNamesTabEnd   = (GXUINT*)(header.nNumOfNames * sizeof(GXUINT));



    //
    // Buffer header
    //
    typedef clvector<FILE_BUFFERHEADER> FileBufArray;
    FileBufArray BufHeaders;  // 文件记录
    BufDescArray BufferTab;   // 运行时记录
    BUFFER_SAVELOAD_DESC bd = {0};
    FILE_BUFFERHEADER BufHeader = {0};

    if(file.GetPointer() != header.nBufHeaderOffset) {
      file.SetPointer(header.nBufHeaderOffset, 0);
    }
    const int nNumOfBuffers = header.nNumOfArrayBufs + 1;
    BufferTab.insert(BufferTab.begin(), nNumOfBuffers, bd);
    BufferTab.front().pBuffer = &m_VarBuffer;

    // 读入文件记录的所有BufferHeader数据
    BufHeaders.insert(BufHeaders.begin(), nNumOfBuffers, BufHeader);
    V_READ(file.Read(&BufHeaders.front(), sizeof(FILE_BUFFERHEADER) * nNumOfBuffers, &uNumOfBytesRead), "Can not load buffer header.");
    ASSERT(uNumOfBytesRead == sizeof(FILE_BUFFERHEADER) * nNumOfBuffers);





    // 这个计算参考[MAIN BUFFER 结构表]
    SIZELIST sSizeList;
    //const GXSIZE_T cbNameTable = (GXSIZE_T)m_pNamesTabEnd - (GXSIZE_T)m_pNamesTabBegin;
    const GXSIZE_T nDescHeaderSize = IntGetRTDescHeader(&sSizeList) + header.cbNames;
    //const GXSIZE_T nDescHeaderSize = IntGetRTDescHeader(&sSizeList) + header.cbNames;
    const GXSIZE_T cbGlobalVariable = header.cbVariableSpace + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(header.nNumOfPtrVars);
    const GXSIZE_T nMainBufferSize_0 = nDescHeaderSize + cbGlobalVariable;
    GXSIZE_T nMainBufferSize = nMainBufferSize_0;




    //dwFlag = 0; // 强力调试屏蔽！！！




    //*
    if(TEST_FLAG(dwFlags, DataPoolCreation_ReadOnly))
    {
      //m_bReadOnly = 1;
      SET_FLAG(m_dwRuntimeFlags, RuntimeFlag_Readonly);
      nMainBufferSize += header.cbStringSpace;
      nMainBufferSize += sizeof(DataPoolArray) * header.nNumOfArrayBufs;

      // 索引从1开始，[0]是全局变量空间，已经计算了
      for(int i = 1; i < nNumOfBuffers; ++i)
      {
        const FILE_BUFFERHEADER& fbh = BufHeaders[i];
        nMainBufferSize += (fbh.nBufferSize + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(fbh.nNumOfRel));
      }
    }//*/

    m_Buffer.Resize(nMainBufferSize, FALSE);

    if(file.GetPointer() != header.nDescOffset) {
      file.SetPointer(header.nDescOffset, 0);
    }

    // 一次读入除了全局变量以外的数据，包括各种描述表，名字字符串列表等
    V_READ(file.Read(m_Buffer.GetPtr(), (GXUINT)nDescHeaderSize, &uNumOfBytesRead), "Can not load desc header.");
    ASSERT(uNumOfBytesRead == nDescHeaderSize);

    const clsize cbDesc = LocalizePtr();
    new(&m_VarBuffer) clstd::RefBuffer((GXLPBYTE)m_Buffer.GetPtr() + cbDesc + header.cbNames, cbGlobalVariable);

    //for(auto p = m_pNamesTabBegin; p != m_pNamesTabEnd; ++p)
    //{
    //  //TRACE("%d %s\n", *p, (LPCSTR)m_pNamesTabEnd + (*p));
    //  TRACE("%s\n", (LPCSTR)m_pNamesTabEnd + (*p));
    //}

    // 64位下扩展描述表中的指针
    if(SIZEOF_PTR > SIZEOF_PTR32)
    {
      IntChangePtrSize(8, m_aVariables, m_nNumOfVar, IS_MARK_NX16B(header.dwFlags));
      IntClearChangePtrFlag(m_aTypes, m_nNumOfTypes);
      IntClearChangePtrFlag(m_aStructs, m_nNumOfStructs);
    }




    // 字符串变量的字符串列表
    clFixedBuffer StringVarBuf;
    GXLPBYTE pStringBegin;
    if(header.cbStringSpace)
    {
      if(file.GetPointer() != header.nStringVarOffset) {
        file.SetPointer(header.nStringVarOffset, 0);
      }
      if(TEST_FLAG(dwFlags, DataPoolCreation_ReadOnly))
      {
        pStringBegin = (GXLPBYTE)m_Buffer.GetPtr() + nMainBufferSize_0;
      }
      else
      {
        StringVarBuf.Resize(header.cbStringSpace, FALSE);
        pStringBegin = (GXLPBYTE)StringVarBuf.GetPtr();
      }
      V_READ(file.Read(pStringBegin, header.cbStringSpace, &uNumOfBytesRead), "Can not load variable strings.");
      ASSERT(uNumOfBytesRead == header.cbStringSpace);
    }
    

    // 非只读模式下，在这里初始化缓冲区
    ASSERT(m_aTypes != NULL);
    if(TEST_FLAG(dwFlags, DataPoolCreation_ReadOnly))
    {
      GXLPBYTE lpBufferPtr = (GXLPBYTE)m_Buffer.GetPtr() + nMainBufferSize_0 + header.cbStringSpace;

      for(int i = 1; i < nNumOfBuffers; ++i)
      {
        const FILE_BUFFERHEADER& fbh = BufHeaders[i];
        BUFFER_SAVELOAD_DESC& bd = BufferTab[i];

        // 定位动态数组类型
        bd.pTypeDesc = (TYPE_DESC*)((GXINT_PTR)m_aTypes + (fbh.nType - header.nDescOffset));

        const clsize nBufferSize = fbh.nBufferSize + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(fbh.nNumOfRel);
        bd.pBuffer = new(lpBufferPtr) DataPoolArray((u32)nBufferSize, lpBufferPtr);
        //((DataPoolArray*)bd.pBuffer)->Resize(, FALSE);

        lpBufferPtr += (nBufferSize + sizeof(DataPoolArray));
      }
      ASSERT((GXSIZE_T)lpBufferPtr - (GXSIZE_T)m_Buffer.GetPtr() == nMainBufferSize);
    }
    else
    {
      for(int i = 1; i < nNumOfBuffers; ++i)
      {
        const FILE_BUFFERHEADER& fbh = BufHeaders[i];
        BUFFER_SAVELOAD_DESC& bd = BufferTab[i];

        // 定位动态数组类型
        bd.pTypeDesc = (TYPE_DESC*)((GXINT_PTR)m_aTypes + (fbh.nType - header.nDescOffset));

        // 分配动态数组空间，增量是8倍类型大小
        bd.pBuffer = new DataPoolArray(NULL, bd.pTypeDesc->cbSize * 8);
        ((DataPoolArray*)bd.pBuffer)->Resize(fbh.nBufferSize + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(fbh.nNumOfRel), FALSE);
      }
    }


    // 开始读取全局变量和动态数组数据
    if(file.GetPointer() != header.nBuffersOffset) {
      file.SetPointer(header.nBuffersOffset, 0);
    }

    // buffer 表中第一个就是全局变量的尺寸，肯定是相等的
    ASSERT(BufferTab.front().pBuffer->GetSize() == cbGlobalVariable);

    clBuffer BufferForRead;
    for(int i = 0; i < nNumOfBuffers; ++i)
    {
      const FILE_BUFFERHEADER& fbh = BufHeaders[i];
      BUFFER_SAVELOAD_DESC& bd = BufferTab[i];

      // 为重定位表预留空间
      if(fbh.nNumOfRel) {
        //BufferTab[i].RelTable.insert(BufferTab[i].RelTable.begin(), fbh.nNumOfRel, 0);
        bd.RelTable.reserve(fbh.nNumOfRel);
      }


      //
      // PS：重定位表既然能收集出来，为什么毛还要递归先收集，另一个循环定位？这个可以一次搞定！
      //


      // 计算buffer的类型说明,收集重定位表
      if(fbh.nType) {
        ASSERT(bd.pBuffer != NULL);
        bd.GenerateRelocalizeTable(bd.pTypeDesc);
      }
      else {
        // 全局变量
        bd.GenerateRelocalizeTable(0, m_aVariables, m_nNumOfVar);
      }

      BufferForRead.Resize(bd.pBuffer->GetSize() - bd.GetPtrAdjustSize(), FALSE);


      TRACE("load buffer type:%s\n", bd.pTypeDesc ? (DataPool::LPCSTR)bd.pTypeDesc->GetName() : "<global>");


      // 读取重定位表
      //if( ! rbd.RelTable.empty()) {
      //  file.Read(&rbd.RelTable.front(), (GXUINT)rbd.RelTable.size() * sizeof(GXUINT));
      //  //rbd.DbgCheck();
      //  // 重定位表偏移肯定都小于缓冲区
      //  ASSERT(rbd.pBuffer->GetSize() >= (rbd.RelTable.front() & BUFFER_SAVELOAD_DESC::RelocalizeOffsetMask));
      //  ASSERT(rbd.pBuffer->GetSize() >= (rbd.RelTable.back() & BUFFER_SAVELOAD_DESC::RelocalizeOffsetMask))
      //}

      V_READ(file.Read(BufferForRead.GetPtr(), (GXUINT)BufferForRead.GetSize(), &uNumOfBytesRead), "Can not load buffer data.");
      ASSERT(uNumOfBytesRead == BufferForRead.GetSize());

      const auto nCheck = bd.RelocalizePtr(bd.pBuffer, &BufferForRead, [&]
      (BUFFER_SAVELOAD_DESC::RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc)
      {
        switch (type)
        {
        case BUFFER_SAVELOAD_DESC::RelocalizeType_String:
        {
          GXLPCWSTR str = (GXLPCWSTR)((GXINT_PTR)pStringBegin + *(GXUINT*)pSrc - header.nStringVarOffset);
          if (TEST_FLAG(dwFlags, DataPoolCreation_ReadOnly))
          {
            *(GXLPCWSTR*)pDest = str;
            //INC_DBGNUMOFSTRING;
          }
          else if (str[0]) {
            new(pDest) clStringW(str);
            //INC_DBGNUMOFSTRING;
            //TRACEW(L"str:%s %s\n", str, *(clStringW*)pDest);
          }
          else {
            *(GXLPCWSTR*)pDest = NULL;
          }
        }
        break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_StringA:
        {
          GXLPCSTR str = (GXLPCSTR)((GXINT_PTR)pStringBegin + *(GXUINT*)pSrc - header.nStringVarOffset);
          if (TEST_FLAG(dwFlags, DataPoolCreation_ReadOnly))
          {
            *(GXLPCSTR*)pDest = str;
          }
          else if (str[0]) {
            new(pDest) clStringA(str);
          }
          else {
            *(GXLPCSTR*)pDest = NULL;
          }
        }
        break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_Array:
        {
          GXUINT index = *(GXUINT*)pSrc;
          ASSERT(index < BufferTab.size());
          if (index)
          {
            // 缓冲区肯定已经创建过了
            ASSERT(BufferTab[index].pBuffer != NULL);
            *(clBufferBase**)pDest = BufferTab[index].pBuffer;
            reinterpret_cast<DataPoolArray*>(BufferTab[index].pBuffer)->SetParent(bd.pBuffer);
            //INC_DBGNUMOFARRAY;
          }
          else {
            *(clBufferBase**)pDest = NULL;
          }
        }
        break;

        case BUFFER_SAVELOAD_DESC::RelocalizeType_Object:
        {
          ASSERT(*(GXUINT*)pSrc == 0);
          *(GUnknown**)pDest = NULL;
        }
        break;
        default:
          CLBREAK;
          break;
        }
        pDest += SIZEOF_PTR;
        pSrc += sizeof(GXUINT);
      });
      ASSERT(nCheck == bd.pBuffer->GetSize());
    }
# if _DEBUG
    u32 uFilePointer = file.GetPointer();
    u32 uFileSize = file.GetSize(NULL);
    ASSERT(uFileSize == uFilePointer);
# endif // #if _DEBUG

#endif // #ifdef DEBUG_DECL_NAME
    return TRUE;
  }

#ifndef _HIDING_CODE_

  GXBOOL DataPoolImpl::CreateSubPool(DataPool** ppSubPool)
  {
    return DataPoolInternal::IntCreateSubPool(ppSubPool, this);
  }
#endif // #ifndef _HIDING_CODE_

#ifndef DISABLE_DATAPOOL_WATCHER
  void DataPoolImpl::IntCleanupWatchObj(WatchFixedDict& sWatchDict)
  {
    for(WatchFixedDict::iterator it = sWatchDict.begin(); it != sWatchDict.end(); ++it)
    {
      for(WatchFixedList::iterator itList = it->second.begin(); itList != it->second.end(); ++itList)
      {
        if(itList->pCallback == 0) {
          DataPoolWatcher*& pWatcher = *(DataPoolWatcher**)&itList->lParam;
          SAFE_RELEASE(pWatcher);
        }
      }
    }
    sWatchDict.clear();
  }
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

  DataPoolArray* DataPoolImpl::IntCreateArrayBuffer(clBufferBase* pParent, LPCVD pVarDesc, GXBYTE* pBaseData, int nInitCount)
  {
    ASSERT(pVarDesc->IsDynamicArray()); // 一定是动态数组
    ASSERT(nInitCount >= 0);

    DataPoolArray** ppBuffer = pVarDesc->GetAsBufferObjPtr(pBaseData);  // 动态数组
    if(*ppBuffer == NULL && TEST_FLAG_NOT(m_dwRuntimeFlags, RuntimeFlag_Readonly))
    {
      // 这里ArrayBuffer只能使用指针形式
      //GXUINT nTypeSize = IS_MARK_NX16B(m_dwRuntimeFlags)
      //  ? ALIGN_16(pVarDesc->TypeSize()) : pVarDesc->TypeSize();
      const GXUINT nTypeSize = DataPoolInternal::NX16BTypeSize(m_dwRuntimeFlags, pVarDesc);

      *ppBuffer = new DataPoolArray(pParent, nTypeSize * 10);  // 十倍类型大小
      (*ppBuffer)->Resize(nInitCount * nTypeSize, TRUE);

#ifndef DISABLE_DATAPOOL_WATCHER
      if(pParent == &m_VarBuffer) {
        WatchFixedDict sDict;
        auto insert_result = m_WatchableArray.insert(clmake_pair(*ppBuffer, sDict));
        ASSERT(insert_result.second); // 添加的一定是全新的
      }
#endif // #ifndef DISABLE_DATAPOOL_WATCHER
//#ifdef _DEBUG
//      m_nDbgNumOfArray++;
//#endif // #ifdef _DEBUG
    }
    return *ppBuffer;
  }

#ifndef _HIDING_CODE_

  //////////////////////////////////////////////////////////////////////////
  DataPoolImpl::VARIABLE_DESC::VTBL* DataPoolImpl::VARIABLE_DESC::GetUnaryMethod() const
  {
    switch(GetTypeClass())
    {
    case DataPoolTypeClass::String:       return (VTBL*)Implement::s_pStringVtbl;
    case DataPoolTypeClass::StringA:      return (VTBL*)Implement::s_pStringAVtbl;
    case DataPoolTypeClass::Structure:    return (VTBL*)Implement::s_pStructVtbl;
    case DataPoolTypeClass::Object:       return (VTBL*)Implement::s_pObjectVtbl;
    case DataPoolTypeClass::Enumeration:  return (VTBL*)Implement::s_pEnumVtbl;
    case DataPoolTypeClass::Flag:         return (VTBL*)Implement::s_pFlagVtbl;
    default:                              return (VTBL*)Implement::s_pPrimaryVtbl;
    }
  }

  DataPoolImpl::VARIABLE_DESC::VTBL* DataPoolImpl::VARIABLE_DESC::GetMethod(GXDWORD dwFlags) const
  {
    if(IsDynamicArray()) {
      return IS_MARK_NX16B(dwFlags)
        ? reinterpret_cast<VTBL*>(Implement::s_pDynamicArrayNX16BVtbl)
        : reinterpret_cast<VTBL*>(Implement::s_pDynamicArrayVtbl);
    }
    else if(nCount > 1) {
      return IS_MARK_NX16B(dwFlags)
        ? reinterpret_cast<VTBL*>(Implement::s_pStaticArrayNX16BVtbl)
        : reinterpret_cast<VTBL*>(Implement::s_pStaticArrayVtbl);
    }
    return GetUnaryMethod();
  }
  //////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_DATAPOOL_WATCHER

  bool DataPoolImpl::WATCH_FIXED::operator<( const WATCH_FIXED& t ) const
  {
    // 这个写的好搓！
    //TRACE("(%x,%x)(%x,%x)\n", pCallback, lParam, t.pCallback, t.lParam);
    switch((GXINT_PTR)pCallback)
    {
    case 0: // DataPoolWatcher
      switch((GXINT_PTR)t.pCallback)
      {
      case 0: return lParam < t.lParam;
      default: return true;
      }
    case 1: // HWND
      switch((GXINT_PTR)t.pCallback)
      {
      case 0: return false;
      case 1: return lParam < t.lParam;
      default: return true;
      }
      // Callback
    default: return pCallback < t.pCallback;
    }
  }

#endif // #ifndef DISABLE_DATAPOOL_WATCHER

#endif // #ifndef _HIDING_CODE_
  GXUINT DataPoolImpl::GetNameId( LPCSTR szName )
  {
    DataPool::LPCSTR aNames = (DataPool::LPCSTR)m_pNamesTabEnd;
    auto* begin = m_pNamesTabBegin;
    auto* end = m_pNamesTabEnd;
    while(begin != end)
    {
      auto* mid = begin + ((end - begin) >> 1); // 注意这是二分头尾两个指针
      int r = GXSTRCMP(szName, aNames + (*mid));
      if(r == 0) {
        // 返回的是字符串在m_Buffer上的偏移
        return *mid + (GXUINT)((GXUINT_PTR)aNames + (GXUINT_PTR)m_Buffer.GetPtr());
      }
      else if(r < 0) {
        end = mid;
      }
      else {
        ASSERT(r > 0);
        if(mid == begin) {
          return 0;
        }
        begin = mid;
      }
    }
    return 0;
  }
#ifndef _HIDING_CODE_

  GXSIZE_T DATAPOOL_HASHALGO::HashString(GXLPCSTR str) const
  {
    ASSERT(eType == 1 || eType == 2);
    return (eType == (u16)clstd::StaticStringsDict::HashType_Local)
      ? clstd::StaticStringsDict::HashChar(str, GXSTRLEN(str), nPos)
      : clstd::HashStringT(str);
  }
} // namespace Marimo

#endif // #ifndef _HIDING_CODE_