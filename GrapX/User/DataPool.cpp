#include "GrapX.H"
#include "GrapX.Hxx"
#include "clStringSet.h"
#include "clPathFile.h"
//#include "GrapX/GUnknown.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolIterator.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"

#include "DataPoolVariableVtbl.h"
using namespace clstd;

// TODO:
// [C]1.�޷��������ǣ���unsigned_int�� Ҫ��Ϊ ��unsigned int��
// [C]2.IsFixedӦ��ʹ�ø����׵ķ����ж�
// [C]3.�ݲ�֧��object����
// [C]4.��SetAsXXXXXX����һ�������۵�����
// 5.DataPoolVariable�����ع�Ϊָ��
// [F]6.��̬����Ҫ�������һ��class ArrayBuffer : public clBufferָ�룬Ҫ���Ǹ�Ϊʵ�壬���ٿ��ܵ��ڴ���Ƭ
// 7.����֧��typedef�ؼ���
// [C]8.��̬����ȥ����#����ʽ�ļ�¼
// 9.֧��const��ʽ
// [C]10.�����кţ�����ʱ�﷨���
// 11.���ȫ���硰var[139].left��
// [C]12.�����ṹ�Ƿ�Ҳ����Ӷ�ַ�������ݷ���һ���ڴ����
// [C]13.�����ӿ� iterator
// [C]14.���� struct A{ A a; } ��������������
// [C]15.Remove��Ҫ����һ���ӿڿ���ɾ�����ɳ�Ա��ɾ����Ա���б���������object��string�Ͷ�̬����
// 16.��̬����Ŀǰֻ������
// 17.���Ӳο����ͣ��൱��ָ��ָ����Ч�ı���
// [C]18.saveʱ��ָ���ض�λ��������load�ض�λ�ϲ�����װΪ��׼����
// [C]19.64λ��������
// [C]20.��������Ϊ��������������ʵ��
// 21.clStringA ��֧��
// 22.�����֧��

#define GSIT_Variables (m_aGSIT)
#define GSIT_Members   (m_aGSIT + m_nNumOfVar)
#define GSIT_Enums     (m_aGSIT + m_nNumOfVar + m_nNumOfMember)

#define V_WRITE(_FUNC, _TIPS) if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
#define V_READ(_FUNC, _TIPS)  if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
#define SIZEOF_PTR          sizeof(void*)
#define SIZEOF_PTR32        sizeof(GXDWORD)
#define SIZEOF_PTR64        sizeof(GXQWORD)
#define TYPE_CHANGED_FLAG   0x80000000  // ������չ��������ʱ�ı�ǣ�����TYPE_DESC::Cate�ϣ��ú�Ҫ���!

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

//namespace std
//{
//  template<> struct less<Marimo::DataPool::WATCH_FIXED>
//  {
//    bool operator()(const Marimo::DataPool::WATCH_FIXED& a, const Marimo::DataPool::WATCH_FIXED& b) const
//    {
//      return _Str.GetHash(); 
//    }
//  };
//}
//
namespace Marimo
{
//#ifdef _X86
//  STATIC_ASSERT(sizeof(DataPool::TYPE_DESC) == 20);
//#else _X64
//  STATIC_ASSERT(sizeof(DataPool::TYPE_DESC) == 24);
//#endif

  typedef DataPoolVariable              Variable;
  typedef const DataPool::VARIABLE_DESC DPVDD;

  //
  // �����õ��ڲ��ṹ��
  //
  typedef clvector<GXUINT>      UIntArray;
  struct BUFFER_SAVELOAD_DESC // ���ڶ�дʱ��ʱ�����buffer����
  {
    enum RelocalizeType // �ض�λ����������32λ��
    {
      RelocalizeType_Array   = (0 << 28),
      RelocalizeType_String  = (1 << 28),
      RelocalizeType_StringA = (2 << 28),
      RelocalizeType_Object  = (3 << 28),
      RelocalizeTypeMask     = 0xf0000000,
      RelocalizeOffsetMask   = ~RelocalizeTypeMask,
    };

    clBufferBase* pBuffer;
    //UIntArray     RelTable;   // �ض�λ��, �ο�RelocalizeType
    // 2���ض�λ�����������Ӻ����ռ��ķ�������������ȥ��1�ű�ȥ���ļ���¼
    UIntArray     RelTable;  // �ض�λ��, �ο�RelocalizeType, �����ƽ̨�޹صģ�ָ�밴��32λ����
    DataPool::LPCTD    pTypeDesc;

    // ģ�庯��ԭ�ͣ� fn(RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc); ע���������Ҫ������
    template<class _Fn>
    GXUINT_PTR RelocalizePtr(clBufferBase* pDestBuffer, const clBufferBase* pSourceBuffer, _Fn fn)
    {
      clsize nCopy;

      if(RelTable.empty()) {
        // ����ָ���buffer��Сһ�����
        ASSERT(pDestBuffer->GetSize() == pSourceBuffer->GetSize());

        nCopy = pSourceBuffer->GetSize();
        memcpy(pDestBuffer->GetPtr(), pSourceBuffer->GetPtr(), nCopy);
        return nCopy;
      }
      GXLPBYTE pDest = (GXLPBYTE)pDestBuffer->GetPtr();
      GXLPCBYTE pSrc = (GXLPCBYTE)pSourceBuffer->GetPtr();
      GXSIZE_T nRelOffset = 0; // �����ƽ̨��صģ�Ҫע��64bits�µ��޸�

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

    static clsize GetPtrAdjustSize(clsize nCountOfRelTab) // ָ�������ߴ�
    {
      // ������ָ�붼��ת��Ϊ32λ�޷�������
      return (SIZEOF_PTR - sizeof(GXUINT)) * nCountOfRelTab;
    }

    clsize GetPtrAdjustSize() const // ָ�������ߴ�
    {
      return GetPtrAdjustSize(RelTable.size());
    }

    clsize GetDiskBufferSize() const
    {
      // ���ճߴ�Ӧ�ÿ��Ǽ�ȥ������ָ��ת��Ϊ4�ֽ������Ĳ�ֵ
      return pBuffer->GetSize() - GetPtrAdjustSize();
    }

    void GenerateRelocalizeTable(DataPool::LPCTD pTypeDesc)
    {
      // �ⲿ��֤���, ȫ�ֱ���ʹ����һ�����ط���
      ASSERT(pTypeDesc != NULL);

      // ʹ����������ϵĶ�̬�������ƥ��, ��СҲ�϶������ͳ��ȵ�������, �յĻ���ʾ�����ȫ�ֱ���
      ASSERT((pBuffer->GetSize() % pTypeDesc->cbSize) == 0);

      const GXUINT nCount = pBuffer->GetSize() / pTypeDesc->cbSize;

      GXUINT nBase = 0; // ����ƫ��
      switch(pTypeDesc->Cate)
      {
      case T_STRUCT:
        for(GXUINT i = 0; i < nCount; ++i) {
          nBase = GenerateRelocalizeTable(nBase, pTypeDesc->GetMembers(), pTypeDesc->nMemberCount);
        }
        break;
      case T_STRING:  GenerateRelocalizeTable(nBase, RelocalizeType_String, nCount);   break;
      case T_STRINGA: GenerateRelocalizeTable(nBase, RelocalizeType_StringA, nCount);  break;
      case T_OBJECT:  GenerateRelocalizeTable(nBase, RelocalizeType_Object, nCount);   break;
      }
    }

    GXUINT GenerateRelocalizeTable(GXUINT nBase, RelocalizeType eRelType, GXUINT nCount)
    {
      for(GXUINT n = 0; n < nCount; ++n) {
        RelTable.push_back(nBase | eRelType);
        nBase += SIZEOF_PTR32;
      }
      return nBase;
    }

    // �����ռ��ض�λ��,ƽ̨�޹أ�ָ�밴��4�ֽڼ���
    GXUINT GenerateRelocalizeTable(GXUINT nBase, DataPool::LPCVD pVarDesc, GXUINT nCount)
    {
      for(GXUINT i = 0; i < nCount; ++i)
      {
        const DataPool::VARIABLE_DESC& vd = pVarDesc[i];
        if(vd.IsDynamicArray())
        {
          RelTable.push_back(nBase | RelocalizeType_Array);
          nBase += SIZEOF_PTR32;
        }
        else
        {
          ASSERT(vd.nCount >= 1);

          switch(vd.GetTypeCategory())
          {
          case T_STRUCT:
            for(GXUINT n = 0; n < vd.nCount; ++n) {
              nBase = GenerateRelocalizeTable(nBase, vd.MemberBeginPtr(), vd.MemberCount());
            }
            break;

          case T_STRING:  nBase = GenerateRelocalizeTable(nBase, RelocalizeType_String, vd.nCount);   break;
          case T_STRINGA: nBase = GenerateRelocalizeTable(nBase, RelocalizeType_StringA, vd.nCount);  break;
          case T_OBJECT:  nBase = GenerateRelocalizeTable(nBase, RelocalizeType_Object, vd.nCount);   break;
          default:        nBase += vd.GetSize();  break;
          } // switch
        }
      } // for
      return nBase;
    }

    //void DbgCheck() // ��֤�ļ���¼���Լ��ռ����ض�λ��һ���ԣ��ռ��㷨�ȶ���ȥ����1
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

  namespace Implement
  {
    extern TYPE_DECLARATION c_InternalTypeDefine[];
    extern DataPoolVariable::VTBL* s_pPrimaryVtbl;
    extern DataPoolVariable::VTBL* s_pEnumVtbl;
    extern DataPoolVariable::VTBL* s_pFlagVtbl;
    extern DataPoolVariable::VTBL* s_pObjectVtbl;
    extern DataPoolVariable::VTBL* s_pStringVtbl;
    extern DataPoolVariable::VTBL* s_pStringAVtbl;
    extern DataPoolVariable::VTBL* s_pStructVtbl;
    extern DataPoolVariable::VTBL* s_pStaticArrayVtbl;
    //extern DataPoolVariable::VTBL* s_pDynamicPrimaryVtbl;
    //extern DataPoolVariable::VTBL* s_pDynamicObjectVtbl;
    //extern DataPoolVariable::VTBL* s_pDynamicStructVtbl;
    extern DataPoolVariable::VTBL* s_pDynamicArrayVtbl;
  } // namespace Implement

#ifdef ENABLE_OLD_DATA_ACTION
#ifdef ENABLE_DATAPOOL_WATCHER
  class DataPoolUIWatcher : public DataPoolWatcher
  {
  private:
    typedef clvector<GXHWND> WndHandleArray;
    WndHandleArray m_aHandles;
  public:
    GXHRESULT AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    clStringA DataPoolUIWatcher::GetClassName()
    {
      return STR_DATAPOOL_WATCHER_UI;
    }

    GXHRESULT DataPoolUIWatcher::RegisterPrivate(GXLPVOID pIndentify)
    {
      if( ! gxIsWindow((GXHWND)pIndentify)) {
        return GX_FAIL;
      }
      WndHandleArray::iterator it = 
        std::find(m_aHandles.begin(), m_aHandles.end(), pIndentify);

      if(it == m_aHandles.end())
      {
        m_aHandles.push_back((GXHWND)pIndentify);
      }
      return GX_OK;
    }

    GXHRESULT DataPoolUIWatcher::UnregisterPrivate(GXLPVOID pIndentify)
    {
      WndHandleArray::iterator it = 
        std::find(m_aHandles.begin(), m_aHandles.end(), pIndentify);

      if(it != m_aHandles.end())
      {
        m_aHandles.erase(it);
        return GX_OK;
      }
      return GX_FAIL;
    }

    GXHRESULT DataPoolUIWatcher::OnKnock(KNOCKACTION* pKnock)
    {
      for(WndHandleArray::iterator it = m_aHandles.begin();
        it != m_aHandles.end(); ++it) {
          gxSendMessage(*it, GXWM_IMPULSE, 0, (GXLPARAM)pKnock);
      }
      return GX_OK;
    }
  }; // class DataPoolUIMonitor
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
#endif // #ifdef ENABLE_OLD_DATA_ACTION

  GXBOOL DataPool::Initialize(LPCTYPEDECL pTypeDecl, LPCVARDECL pVarDecl)
  {
    if(pVarDecl == NULL) {
      return FALSE;
    }
    //FloatVarTypeDict FloatDict; // �ܶ����黹û��ȷ�����������ͱ�, ������
    DataPoolBuildTime sBuildTime;   // ����ʱʹ�õĽṹ

    // �����������ͱ�
    // -- �������ͱ�ʾ�Ǳ����ε�,�����кϷ��Լ��,Debug�滹��Ҫ��΢����µ�
#ifdef _DEBUG
    if( ! sBuildTime.IntCheckTypeDecl(Implement::c_InternalTypeDefine, TRUE))
      return FALSE;
#else
    if( ! sBuildTime.IntCheckTypeDecl(Implement::c_InternalTypeDefine, FALSE))
      return FALSE;
#endif // #ifdef _DEBUG9
    if(pTypeDecl != NULL) {
      if( ! sBuildTime.IntCheckTypeDecl(pTypeDecl, TRUE))
        return FALSE;
    }

    // ����������
    if( ! sBuildTime.CheckVarList(pVarDecl)) {
      return FALSE;
    }

    GXINT nBufferSize = sBuildTime.CalculateVarSize(pVarDecl, sBuildTime.m_aVar);
    if(nBufferSize == 0) {
      CLOG_ERROR("%s: Empty data pool.\n", __FUNCTION__);
      return FALSE;
    }

    // ��λ����������
    LocalizeTables(sBuildTime, nBufferSize);
    
    InitializeValue(0, pVarDecl);

    //m_bFixedPool = IntIsRawPool();
    return TRUE;
  }


  GXBOOL DataPool::CleanupArray(LPCVD pVarDesc, GXLPVOID lpFirstElement, int nElementCount)
  {
    switch(pVarDesc->GetTypeCategory())
    {
    case T_STRING:
      {
        clStringW* pString = reinterpret_cast<clStringW*>(lpFirstElement);

        // ���ε�����������
        for(int nStringIndex = 0; nStringIndex < nElementCount; nStringIndex++)
        {
          if(pString[nStringIndex]) {
            pString[nStringIndex].~clStringX();
#ifdef _DEBUG
            m_nDbgNumOfString--;
#endif // #ifdef _DEBUG
          }
        }
      }
      break;

    case T_STRINGA:
      {
        clStringA* pString = reinterpret_cast<clStringA*>(lpFirstElement);

        // ���ε�����������
        for(int nStringIndex = 0; nStringIndex < nElementCount; nStringIndex++)
        {
          if(pString[nStringIndex]) {
            pString[nStringIndex].~clStringX();
#ifdef _DEBUG
            m_nDbgNumOfString--;
#endif // #ifdef _DEBUG
          }
        }
      }
      break;

    case T_OBJECT:
      {
        GUnknown** pObjArray = reinterpret_cast<GUnknown**>(lpFirstElement);

        // ���ε�����������
        for(int i = 0; i < nElementCount; i++) {
          SAFE_RELEASE(pObjArray[i]);
        }
      }
      break;

    case T_STRUCT:
      {
        for(int nStructIndex = 0; nStructIndex < nElementCount; nStructIndex++)
        {
          Cleanup(lpFirstElement, pVarDesc->MemberBeginPtr(), pVarDesc->MemberCount());
          lpFirstElement = (GXLPBYTE)lpFirstElement + pVarDesc->TypeSize();
        }
        //if(bDynamicArray) {
        //  SAFE_DELETE(*ppBuffer);
        //}
      }
      break;
    }
    return TRUE;
  }

  GXBOOL DataPool::Cleanup(GXLPVOID lpBuffer, LPCVD pVarDesc, int nVarDescCount)
  {
    GXBYTE* pData = (GXBYTE*)lpBuffer;
    for(int i = 0; i < nVarDescCount; i++)
    {
      const VARIABLE_DESC& VARDesc = pVarDesc[i];
      GXBOOL bDynamicArray = VARDesc.IsDynamicArray();
      DataPoolArray** ppBuffer = NULL;

      GXLPVOID ptr;
      int nCount = 0;

      if(bDynamicArray) // ��̬�ַ�������
      {
        ppBuffer = VARDesc.GetAsBufferPtr(pData);
        if(*ppBuffer == NULL) {
          continue;
        }
        ptr = (*ppBuffer)->GetPtr();
        nCount = (int)((*ppBuffer)->GetSize() / VARDesc.TypeSize());
      }
      else // �ַ�������
      {
        ptr = VARDesc.GetAsPtr(pData);
        nCount = VARDesc.nCount;
      }

      CleanupArray(&VARDesc, ptr, nCount);
      
      if(bDynamicArray)
      {
        // ppBuffer ������������ж����Ѿ����ã����û���þ��ǻ������Ͷ�̬���顣
        ASSERT(ppBuffer == NULL || ppBuffer == VARDesc.GetAsBufferPtr(pData));

        ppBuffer = VARDesc.GetAsBufferPtr(pData); // (clBuffer**)(pData + VARDesc.nOffset);
        if(*ppBuffer)
        {
          delete (*ppBuffer);
          *ppBuffer = NULL;
#ifdef _DEBUG
          m_nDbgNumOfArray--;
#endif // #ifdef _DEBUG
        }
      }

    }
    return TRUE;
  }

  DataPool::LPCVD DataPool::IntGetVariable(LPCVD pVdd, GXLPCSTR szName/*, int nIndex*/)
  {
    // TODO: ������Ը�Ϊ�Ƚ�Name Id�ķ�ʽ��Name Id���Ծ���m_Buffer�е�ƫ��
    LPCVD pDesc = NULL;
    int begin = 0, end;
    if(pVdd != NULL) {

      // ֻ�нṹ����г�Ա, �������ֱ�ӷ���
      if(pVdd->GetTypeCategory() != T_STRUCT) {
        return NULL;
      }
      end   = pVdd->MemberCount();
      pDesc = pVdd->MemberBeginPtr();
      //ASSERT(nEnd <= (int)m_nNumOfMember);
    }
    else {
      end   = m_nNumOfVar;
      pDesc = m_aVariables;
    }

    SortedIndexType* p = m_aGSIT + (pDesc - m_aVariables);

#ifdef _DEBUG
    GXUINT count = (GXUINT)end;
    //for(int i = begin; i < end; ++i)
    //{
    //  TRACE("%2d %*s %s\n", i, -32, pDesc[i].VariableName(), pDesc[p[i]].VariableName());
    //}
#endif // #ifdef _DEBUG
    //*
    while(begin <= end - 1)
    {
      int mid = (begin + end) >> 1;
      ASSERT(p[mid] < count); // �����϶������ܴ�С
      LPCVD pCurDesc = pDesc + p[mid];
      //TRACE("%s\n", pCurDesc->VariableName());
      int result = GXSTRCMP(szName, pCurDesc->VariableName());
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
    }//*/
#ifdef _DEBUG
    for(int i = begin; i < (int)count; i++)
    {
      if(GXSTRCMP(pDesc[i].VariableName(), szName) == 0) {
        CLBREAK;
        return &pDesc[i];
      }
    }
#endif // #ifdef _DEBUG
    return NULL;
  }

  GXVOID DataPool::InitializeValue(GXUINT nBaseOffset, LPCVARDECL pVarDecl)
  {
    int nVarIndex = 0;
    GXBYTE* pData = (GXBYTE*)m_VarBuffer.GetPtr();
    for(;; nVarIndex++)
    {
      // ����������ͷ����д�ɲ�һ����, ����̫��������, ̫����!
      const VARIABLE_DECLARATION& varDecl = pVarDecl[nVarIndex];
      if(varDecl.Type == NULL || varDecl.Name == NULL) {
        break;
      }
      const VARIABLE_DESC& VARDesc = m_aVariables[nVarIndex];

      GXBOOL bDynamicArray = VARDesc.IsDynamicArray();
      ASSERT(GXSTRCMPI(VARDesc.VariableName(), varDecl.Name) == 0);
      switch(VARDesc.GetTypeCategory())
      {
      case T_STRUCT:
        {
          int nMemberIndex;
          //int nStart = VARDesc.MemberBegin();
          int nEnd = VARDesc.MemberCount();
          auto pMembers = VARDesc.MemberBeginPtr();

          // ���ں��ж�̬������ַ����Ľṹ���ǲ���ֱ�Ӹ�ֵ��
          for(nMemberIndex = 0; nMemberIndex < nEnd; nMemberIndex++)
          {
            if(pMembers[nMemberIndex].GetTypeCategory() == T_STRING || 
              pMembers[nMemberIndex].GetTypeCategory() == T_STRINGA || 
              pMembers[nMemberIndex].IsDynamicArray())
              break;
          }
          if(nMemberIndex != nEnd)
            break;
        } // ����û�� break, ��� Struct ��û�ж�̬������ַ�������, ֧�ֳ�ʼ����.
      case T_BYTE:
      case T_WORD:
      case T_DWORD:
      case T_OBJECT:
      case T_QWORD:
      case T_SBYTE:
      case T_SWORD:
      case T_SDWORD:
      case T_SQWORD:
      case T_FLOAT:
        if(varDecl.Init != NULL)
        {
          if(bDynamicArray)
          {
            ASSERT(varDecl.Count < 0);
            clBuffer* pBuffer = VARDesc.CreateAsBuffer(this, &m_VarBuffer, pData, varDecl.Init == NULL ? 0 : -varDecl.Count);
            memcpy(pBuffer->GetPtr(), varDecl.Init, pBuffer->GetSize());
          }
          else
          {
            memcpy(VARDesc.GetAsPtr(pData), varDecl.Init, VARDesc.GetSize());
          }
        }
        break;
        //case T_Struct:
        //  //if(varDecl.Init != NULL)
        //  break;
      case T_STRING:
        {
          GXLPCWSTR  pStrInit = (GXLPCWSTR)varDecl.Init;
          clStringW* pStrPool = NULL;
          int nCount = varDecl.Init == NULL ? 0 : varDecl.Count;
          if(bDynamicArray)
          {
            ASSERT(VARDesc.TypeSize() == sizeof(clStringW));

            // ���û�г�ʼ������, ���ʼά������Ϊ0
            ASSERT((varDecl.Init != NULL && nCount < 0) || 
              (varDecl.Init == NULL && nCount == 0));

            pStrPool = (clStringW*)VARDesc.CreateAsBuffer(this, &m_VarBuffer, pData, -nCount)->GetPtr();
            nCount = -nCount;
          }
          else
          {
            pStrPool = VARDesc.GetAsStringW(pData);
          }

          // ���д��̫��̬��!!
          for(int i = 0; i < nCount; i++)
          {
            clStringW& str = pStrPool[i];
            new(&str) clStringW;
            INC_DBGNUMOFSTRING;
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

  GXBOOL DataPool::IsIllegalName(GXLPCSTR szName)
  {
    int i = 0;
    while(szName[i] != '\0')
    {
      if( ! (szName[i] == '_' || 
        (szName[i] >= 'a' && szName[i] <= 'z') ||
        (szName[i] >= 'A' && szName[i] <= 'Z') ||
        (szName[i] >= '0' && szName[i] <= '9' && i > 0)) ) {
          return FALSE;
      }
      i++;
    }
    return i > 0;
  }

  DataPool::DataPool(GXLPCSTR szName)
    : m_Name        (szName ? szName : "")
    //, m_pBuffer     (NULL)
    , m_nNumOfTypes (0)
    , m_aTypes      (NULL)
    , m_nNumOfVar   (0)
    , m_aVariables  (NULL)
    , m_nNumOfMember(0)
    , m_aMembers    (NULL)
    , m_nNumOfEnums (0)
    , m_aEnums      (NULL)
    //, m_StringBase  (0)
    , m_aGSIT       (NULL)
    //, m_bFixedPool  (1)
    //, m_bReadOnly   (0)
#ifdef ENABLE_DATAPOOL_WATCHER
    //, m_bAutoKnock  (1)
    , m_dwRuntimeFlags     (RuntimeFlag_Fixed)
#else
    , m_dwRuntimeFlags     (RuntimeFlag_AutoKnock | RuntimeFlag_Fixed)
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
#ifdef _DEBUG
    , m_nDbgNumOfArray (0)
    , m_nDbgNumOfString(0)
#endif // #ifdef _DEBUG
  {
  }

  DataPool::~DataPool()
  {
    // ������е�����
    // ��Ҫ������̬���飬�ַ���������ͽṹ��
    // �ṹ�������ݹ飬�������µ���������
    if(m_nNumOfVar && TEST_FLAG_NOT(m_dwRuntimeFlags, RuntimeFlag_Readonly)) {
      Cleanup(m_VarBuffer.GetPtr(), m_aVariables, (int)m_nNumOfVar);
      ASSERT(m_nDbgNumOfArray == 0);
      ASSERT(m_nDbgNumOfString == 0);
    }

#ifdef DATAPOOLCOMPILER_PROJECT
#else
    if(m_Name.IsNotEmpty())
    {
      GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
      GXSTATION::NamedInterfaceDict::iterator it = lpStation->m_NamedPool.find(m_Name);
      if(it != lpStation->m_NamedPool.end()) {
        lpStation->m_NamedPool.erase(it);
      }
      else {
        CLBREAK; // ��Ӧ�ð�, ��ô�Ҳ�������?
      }
    }
#endif // #ifdef DATAPOOLCOMPILER_PROJECT

#ifdef ENABLE_OLD_DATA_ACTION
#ifdef ENABLE_DATAPOOL_WATCHER
    // �ͷ����еļ�����
    for(WatcherArray::iterator it = m_aWatchers.begin();
      it != m_aWatchers.end(); ++it) {
        SAFE_RELEASE(*it);
    }
    m_aWatchers.clear();
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
#else
    for(WatchFixedDict::iterator it = m_FixedDict.begin(); it != m_FixedDict.end(); ++it)
    {
      for(WatchFixedList::iterator itList = it->second.begin(); itList != it->second.end(); ++itList)
      {
        if(itList->pCallback == 0) {
          DataPoolWatcher*& pWatcher = *(DataPoolWatcher**)&itList->lParam;
          SAFE_RELEASE(pWatcher);
        }
      }
    }
    m_FixedDict.clear();
#endif // #ifdef ENABLE_OLD_DATA_ACTION
    //SAFE_DELETE(m_pBuffer);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
#ifdef DATAPOOLCOMPILER_PROJECT
  GXHRESULT DataPool::AddRef()
  {
    return ++m_nRefCount;
  }

  GXHRESULT DataPool::Release()
  {
    GXLONG nRefCount = --m_nRefCount;
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return m_uRefCount;
  }
#else
  GXHRESULT DataPool::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT DataPool::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return m_uRefCount;
  }
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXLPCSTR DataPool::GetVariableName(GXUINT nIndex) const
  {
    if(nIndex >= m_nNumOfVar) {
      return NULL;
    }

    return (m_aVariables[nIndex].VariableName());
  }

  const DataPool::TYPE_DESC* DataPool::FindType(GXLPCSTR szTypeName) const
  {
    for(GXUINT i = 0; i < m_nNumOfTypes; ++i)
    {
      if(GXSTRCMP(m_aTypes[i].GetName(), szTypeName) == 0) {
        return &m_aTypes[i];
      }
    }
    return NULL;
  }

#ifdef DATAPOOLCOMPILER_PROJECT
#else
  GXHRESULT DataPool::GetLayout(GXLPCSTR szStructName, DataLayoutArray* pLayout)
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
      LPCTD pDesc = FindType(szStructName);
      pVarDesc = pDesc->GetMembers(); //&m_aMembers[pDesc->nMemberIndex];
      nCount   = pDesc->nMemberCount;
    }

    for(GXUINT i = 0; i < nCount; i++)
    {
      DATALAYOUT DataLayout;
      DataLayout.eType = GXUB_UNDEFINED;
      DataLayout.pName = pVarDesc->VariableName();
      DataLayout.uOffset = pVarDesc->nOffset;
      DataLayout.uSize = pVarDesc->TypeSize();
      pLayout->push_back(DataLayout);
      pVarDesc++;
    }

    return GX_OK;
  }
#endif // #ifdef DATAPOOLCOMPILER_PROJECT

  GXBOOL DataPool::IsFixedPool()
  {
    return (GXBOOL)TEST_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_Fixed);
  }

#ifdef ENABLE_DATAPOOL_WATCHER
  GXBOOL DataPool::IsAutoKnock()
  {
    return TEST_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_AutoKnock);
  }

  GXBOOL DataPool::IsKnocking(const DataPoolVariable* pVar)
  {
    return IntIsKnocking(pVar);
  }

  GXBOOL DataPool::SetAutoKnock(GXBOOL bAutoKnock)
  {
    GXBOOL bPrevFlag = TEST_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_AutoKnock);
    if(bAutoKnock) {
      SET_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_AutoKnock);
    }
    else {
      RESET_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_AutoKnock);
    }
    return bPrevFlag;
  }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

  GXLPVOID DataPool::GetFixedDataPtr()
  {
    return TEST_FLAG(m_dwRuntimeFlags, DataPool::RuntimeFlag_Fixed) ? m_VarBuffer.GetPtr() : NULL;
  }

  GXBOOL DataPool::QueryByName(GXLPCSTR szName, Variable* pVar)
  {
    VARIABLE var = {0};
    var.pBuffer = &m_VarBuffer;
    if( ! IntQuery(&var, szName, -1)) {
      pVar->Free();
      return FALSE;
    }

    if(pVar->GetPoolUnsafe() == this) {
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, var.pVdd, var.pBuffer, var.AbsOffset);
    } else {
      pVar->Free();
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, this, var.pVdd, var.pBuffer, var.AbsOffset);
    }
    return TRUE;
  }

  // ����ֵ�Ǳ��������buffer��������������1
  GXINT DataPool::IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar)
  {
    GXLPCSTR  szName;         // �������ҵ�����
    GXUINT    nIndex = (GXUINT)-1;
    clStringA str;
    clStringA strName; // �����ݴ���±�ʱ�ı�����
    GXINT     result = FALSE;
    //clBufferBase* pArrayBuffer = NULL;

    ASSERT( ! pVar->IsValid());
    pVar->pBuffer = &m_VarBuffer;

    // �ֽ���ʽ
    clstd::StringCutter<clStringA> sExpression(szExpression);

    do {

      if(pVar->IsValid())
      {
        // �������û��ȡԪ�ؾ���ͼ���ʳ�Ա������һ���ǲ��Ե�
        // ��������һ����ȡ��������֮���������
        if((pVar->pVdd->IsDynamicArray() || pVar->pVdd->nCount > 1) && nIndex == (GXUINT)-1) {
          return FALSE;
        }
      }

      sExpression.Cut(str, '.');

      if(str.EndsWith(']')) // ���±�����
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

      /*
      if( ! pVar->IsValid()) {
        result = IntQuery(&m_VarBuffer, NULL, szName, 0, pVar, &pArrayBuffer);
      }
      else {
        result = IntQuery(pVar->pBuffer, pVar->pVdd, szName, pVar->AbsOffset, pVar, &pArrayBuffer);
      }

      // ��������
      if(result && nIndex != (GXUINT)-1) {
        // ��ξ���ʵ����DataPoolVariable::GetLength()
        const GXBOOL bDynamic = pVar->pVdd->IsDynamicArray();
        if( ! bDynamic && nIndex < pVar->pVdd->nCount)
        {
          pVar->AbsOffset += nIndex * pVar->pVdd->TypeSize();
          IntCreateUnary(pVar->pBuffer, pVar->pVdd, pVar);
        }
        else if(bDynamic && nIndex < (pArrayBuffer->GetSize() / pVar->pVdd->TypeSize()))
        {
          pVar->AbsOffset = nIndex * pVar->pVdd->TypeSize();
          IntCreateUnary(pArrayBuffer, pVar->pVdd, pVar);
        }
        else {
          result = FALSE;
        }
      }
      /*/

      result = IntQuery(pVar, szName, nIndex);

      //*/
      ASSERT(( ! result) || pVar->IsValid()); // result �� pVar һ��ͬʱ��Ч������Ч
      //if(( ! result) || ( ! pVar->IsValid())) {
      //  result = 0;
      //  break;
      //}
    } while(result && ( ! sExpression.IsEndOfString()));

    return result;
  }

  GXBOOL DataPool::QueryByExpression(GXLPCSTR szExpression, Variable* pVar)
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
      new(pVar) DataPoolVariable((DataPoolVariable::VTBL*)var.vtbl, this, var.pVdd, var.pBuffer, var.AbsOffset);
    }
    return bval;
  }

  DataPool::LPCVD DataPool::IntFindVariable(LPCVD pVarDesc, int nCount, GXUINT nOffset)
  {
    int begin = 0;
    int end = nCount - 1;
    auto nEndOffset = pVarDesc[end].nOffset + pVarDesc[end].GetUsageSize();
    while(1)
    {
      const int mid = (begin + end) >> 1;
      if(pVarDesc[begin].nOffset <= nOffset && nOffset < pVarDesc[mid].nOffset) {
        end = mid;
        nEndOffset = pVarDesc[end].nOffset + pVarDesc[end].GetUsageSize();
      }
      else if(pVarDesc[mid].nOffset <= nOffset && nOffset <= nEndOffset) {
        if(begin == mid) {
          break;
        }
        begin = mid;
      }
    }

    if(pVarDesc[end].nOffset >= nOffset) {
      return pVarDesc + end;
    }
    else if(pVarDesc[begin].nOffset >= nOffset) {
      return pVarDesc + begin;
    }

    return NULL;
  }

  GXBOOL DataPool::FindFullName( clStringA* str, LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset )
  {
    LPCVD pVar;
    if(pBuffer == &m_VarBuffer) {
      pVar = IntFindVariable(m_aVariables, m_nNumOfVar, nOffset);
      if(pVarDesc == pVar) {
        *str = pVar->VariableName();
        return TRUE;
      }
      //CLNOP;
    }
    return FALSE;
  }

#ifdef ENABLE_DATAPOOL_WATCHER
  int DataPool::FindWatcher(DataPoolWatcher* pWatcher)
  {
    int nIndex = 0;
    for(WatcherArray::iterator it = m_aWatchers.begin();
      it != m_aWatchers.end(); ++it, ++nIndex)
    {
      if(*it == pWatcher) {
        return nIndex;
      }
    }
    return -1;
  }

#ifdef ENABLE_OLD_DATA_ACTION
  int DataPool::FindWatcherByName(GXLPCSTR szClassName)
  {
    int nIndex = 0;
    for(WatcherArray::iterator it = m_aWatchers.begin();
      it != m_aWatchers.end(); ++it, ++nIndex)
    {
      if((*it)->GetClassName() == szClassName) {
        return nIndex;
      }
    }
    return -1;
  }

  GXHRESULT DataPool::CreateWatcher(GXLPCSTR szClassName)
  {
    if(FindWatcherByName(szClassName) >= 0) {
      // �ظ�����
      return GX_FAIL;
    }
    // TODO: ...

    DataPoolWatcher* pWatcher = NULL;
    if(clStringA(szClassName) == STR_DATAPOOL_WATCHER_UI)
    {
      pWatcher = new DataPoolUIWatcher;
    }
    if( ! InlCheckNewAndIncReference(pWatcher)) {
      return GX_FAIL;
    }
    m_aWatchers.push_back(pWatcher);
    return (GXHRESULT)(m_aWatchers.size() - 1);
  }

  GXHRESULT DataPool::RemoveWatcherByName(GXLPCSTR szClassName)
  {
    const int nIndex = FindWatcherByName(szClassName);
    if(nIndex < 0) {
      return GX_FAIL;
    }
    WatcherArray::iterator it = m_aWatchers.begin() + nIndex;
    SAFE_RELEASE(*it);
    m_aWatchers.erase(it);
    return GX_OK;
  }

  GXHRESULT DataPool::AddWatcher(DataPoolWatcher* pWatcher)
  {
    if(FindWatcher(pWatcher) >= 0) {
      return GX_FAIL;
    }

    GXHRESULT hval = pWatcher->AddRef();
    if(GXSUCCEEDED(hval))
    {
      m_aWatchers.push_back(pWatcher);
    }
    return hval;
  }

  GXHRESULT DataPool::RemoveWatcher(DataPoolWatcher* pWatcher)
  {
    int nIndex = FindWatcher(pWatcher);
    if(nIndex < 0 || nIndex >= (int)m_aWatchers.size()) {
      return GX_FAIL;
    }

    m_aWatchers.erase(m_aWatchers.begin() + nIndex);
    return pWatcher->Release();
  }

  GXHRESULT DataPool::RegisterIdentify(GXLPCSTR szClassName, GXLPVOID pIndentify)
  {
    if(szClassName == NULL) {
      return GX_FAIL;
    }

    int nIndex = FindWatcherByName(szClassName);
    if(nIndex < 0) {
      nIndex = CreateWatcher(szClassName);
      if(nIndex < 0) {
        return GX_FAIL;
      }
    }

    return m_aWatchers[nIndex]->RegisterPrivate(pIndentify);
  }

  GXHRESULT DataPool::UnregisterIdentify(GXLPCSTR szClassName, GXLPVOID pIndentify)
  {
    if(szClassName == NULL) {
      return GX_FAIL;
    }

    const int nIndex = FindWatcherByName(szClassName);
    if(nIndex < 0) {
      return GX_FAIL;
    }

    return m_aWatchers[nIndex]->UnregisterPrivate(pIndentify);
  }
#endif // #ifdef ENABLE_OLD_DATA_ACTION

  GXHRESULT DataPool::ImpluseByVariable(DataAction eType, const DataPoolVariable& var, GXUINT nIndex, GXBOOL bForce)
  {
    if( ! var.IsValid() ||
      ( ! bForce && TEST_FLAG_NOT(m_dwRuntimeFlags, RuntimeFlag_AutoKnock))) {
      return GX_FAIL;
    }
    //GXDWORD dwSaveAK = m_bAutoKnock;
    //m_bAutoKnock = 0;

    if(IntIsKnocking(&var)) {
      return GX_FAIL;
    }

    //KNOCKACTION ka;
    ////InlSetZeroT(ka);
    //ka.pSponsor  = &var;
    //ka.pDataPool = this;
    //ka.Name      = var.GetName();
    //ka.Action    = eType;
    //ka.ptr       = var.GetPtr();
    auto it_result = m_FixedDict.find(var.GetPtr());
    if(it_result != m_FixedDict.end())
    {
      DATAPOOL_IMPULSE sImpulse;
      sImpulse.sponsor = &var;
      sImpulse.reason = eType;
      sImpulse.index = nIndex;
      sImpulse.count = 1;
      sImpulse.param = NULL;
      for(auto it = it_result->second.begin(); it != it_result->second.end(); ++it)
      {
        sImpulse.param = it->lParam;
        switch((GXINT_PTR)it->pCallback)
        {
        case 0:
          ((DataPoolWatcher*)it->lParam)->OnImpulse(&sImpulse);
          break;
        case 1:
          gxSendMessage((GXHWND)it->lParam, GXWM_IMPULSE, 0, (GXLPARAM)&sImpulse);
          break;
        default:
          it->pCallback(&sImpulse);
          break;
        }
      }
    }

    //m_setKnocking
    //m_setKnocking
    //m_setKnocking
    //m_setKnocking
    //m_setKnocking
    //m_setKnocking
    //m_setKnocking

    //// FIXME: ���д�ģ����ܲ���
    //if(TEST_FLAG_NOT(var.GetCaps(), DataPoolVariable::CAPS_FIXED)) {
    //  //ka.Index = var.InlGetOffset() / var.InlGetVDD()->TypeSize();
    //  ka.Index = nIndex;
    //}
    //else {
    //  ka.Index = 0;
    //}

    //m_setKnocking.insert(var.GetPtr());
    //for(WatcherArray::iterator it = m_aWatchers.begin();
    //  it != m_aWatchers.end(); ++it)
    //{
    //  (*it)->OnKnock(&ka);
    //}

    //KnockingSet::iterator it = m_setKnocking.find(var.GetPtr());
    //m_setKnocking.erase(it);
    //m_bAutoKnock = dwSaveAK;
    return GX_OK;
  }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

  GXBOOL DataPool::IntCreateUnary(clBufferBase* pBuffer, LPCVD pThisVdd, VARIABLE* pVar)
  {
    ASSERT(pThisVdd->TypeSize() != 0);

    VARIABLE::VTBL* pVtbl = pThisVdd->GetUnaryMethod();
    ASSERT(pVtbl != NULL);

    pVar->Set((VARIABLE::VTBL*)pVtbl, pThisVdd, pBuffer, pVar->AbsOffset);
    return TRUE;
  }

  GXBOOL DataPool::IntQuery(GXINOUT VARIABLE* pVar, GXLPCSTR szVariableName, GXUINT nIndex)
  {
    // �ڲ������в��ı�pVar->m_pDataPool�����ü���
    using namespace Implement;
    LPCVD pVarDesc = IntGetVariable(pVar->pVdd, szVariableName);
    const GXUINT nMemberOffset = pVar->AbsOffset + pVarDesc->nOffset; // �������õ���������һ��

    if(pVarDesc == NULL) {
      return FALSE;
    }

    if(pVarDesc->IsDynamicArray()) { // ��̬����
      clBuffer* pArrayBuffer = pVarDesc->CreateAsBuffer(this, pVar->pBuffer, (GXBYTE*)pVar->pBuffer->GetPtr() + pVar->AbsOffset, 0);
      if(nIndex == (GXUINT)-1)
      {
        pVar->Set((VARIABLE::VTBL*)s_pDynamicArrayVtbl, pVarDesc, pVar->pBuffer, nMemberOffset);
        return TRUE;
      }
      else if(nIndex < (pArrayBuffer->GetSize() / pVarDesc->TypeSize()))
      {
        pVar->AbsOffset = nIndex * pVarDesc->TypeSize();
        return IntCreateUnary(pArrayBuffer, pVarDesc, pVar);
      }
    }
    else if(pVarDesc->nCount > 1) { // ��̬����
      if(nIndex == (GXUINT)-1)
      {
        pVar->Set((VARIABLE::VTBL*)s_pStaticArrayVtbl, pVarDesc, pVar->pBuffer, nMemberOffset);
        return TRUE;
      }
      else if(nIndex < pVarDesc->nCount)
      {
        pVar->AbsOffset += (pVarDesc->nOffset + nIndex * pVarDesc->TypeSize());
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

  //GXBOOL DataPool::IntQuery(clBufferBase* pBufferBase, DPVDD* pParentVdd, GXLPCSTR szVariable, int nOffsetAdd, VARIABLE* pVar, clBufferBase** ppArrayBuffer)
  //{
  //  // �ڲ������в��ı�pVar->m_pDataPool�����ü���
  //  using namespace Implement;
  //  GXBYTE* pDataBuffer = (GXBYTE*)pBufferBase->GetPtr();
  //  DPVDD* pVdd = IntGetVariable(pParentVdd, szVariable);
  //  const GXBOOL bRootBuf = ((GXLPCVOID)pBufferBase == (GXLPCVOID)&m_VarBuffer);
  //  ASSERT(bRootBuf || ( ! bRootBuf && pParentVdd != NULL));
  //  //ASSERT( ! pVar->IsValid());
  //  if(pVdd == NULL) {
  //    return FALSE;
  //  }

  //  if(pVdd->IsDynamicArray()) {
  //    clBuffer* pElementBuffer = pVdd->CreateAsBuffer(this, pBufferBase, (GXBYTE*)pBufferBase->GetPtr() + nOffsetAdd, 0);
  //    //pVar->Set((VARIABLE::VTBL*)s_pDynamicArrayVtbl, pVdd, pElementBuffer, 0);
  //    pVar->Set((VARIABLE::VTBL*)s_pDynamicArrayVtbl, pVdd, pBufferBase, pVdd->nOffset + nOffsetAdd);
  //    *ppArrayBuffer = pElementBuffer;
  //    //return TRUE;
  //  }
  //  else if(pVdd->nCount > 1) {
  //    pVar->Set((VARIABLE::VTBL*)s_pStaticArrayVtbl, pVdd, pBufferBase, pVdd->nOffset + nOffsetAdd);
  //    //return pBufferBase;
  //  }
  //  else {
  //    ASSERT(pVdd->nCount == 1);
  //    pVar->AbsOffset = pVdd->nOffset + nOffsetAdd;
  //    return IntCreateUnary(pBufferBase, pVdd, pVar);
  //    //return pBufferBase;
  //  }
  //  return TRUE;
  //}

#ifdef ENABLE_DATAPOOL_WATCHER
  GXBOOL DataPool::IntIsKnocking(const DataPoolVariable* pVar) const
  {
    KnockingSet::const_iterator it = m_setKnocking.find(pVar->GetPtr());
    return it != m_setKnocking.end();
  }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

#define IS_VALID_NAME(_NAME)  (_NAME != NULL && clstd::strlenT(_NAME) > 0)

  GXHRESULT DataPool::FindDataPool(DataPool** ppDataPool, GXLPCSTR szName)
  {
#ifdef DATAPOOLCOMPILER_PROJECT
#else
    if(IS_VALID_NAME(szName))
    {
      GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
      GXSTATION::NamedInterfaceDict::iterator it = lpStation->m_NamedPool.find(szName);

      // �еĻ�ֱ���������ü���Ȼ�󷵻�
      if(it != lpStation->m_NamedPool.end()) {
        *ppDataPool = static_cast<DataPool*>(it->second);
        return it->second->AddRef();
      }
    }
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
    return GX_FAIL;
  }

  GXHRESULT DataPool::FindVariable(DataPool** ppDataPool, DataPoolVariable* pVar, GXLPCSTR szGlobalExpession)
  {
    if(ppDataPool == NULL && pVar == NULL) {
      return GX_FAIL;
    }

    clStringA strNamespace;
    clStringA strVariable;
    clStringA(szGlobalExpession).DivideBy('.', strNamespace, strVariable);

    DataPool* pDataPool = NULL;
    Variable  Var;
    GXHRESULT hval = MODataPool::FindDataPool(&pDataPool, strNamespace);
    if(GXSUCCEEDED(hval) && pDataPool->QueryByExpression(strVariable, &Var))
    {
      if(ppDataPool) {
        *ppDataPool = pDataPool;
        pDataPool = NULL;
      }
      else {
        SAFE_RELEASE(pDataPool);
      }

      if(pVar) {
        *pVar = Var;
      }
      return hval;
    }

    SAFE_RELEASE(pDataPool);
    return GX_FAIL;
  }

  GXHRESULT DataPool::CreateFromResolver(DataPool** ppDataPool, GXLPCSTR szName, DataPoolCompiler* pResolver)
  {    
    if(pResolver) {
      DataPoolCompiler::MANIFEST sManifest;
      if(GXSUCCEEDED(pResolver->GetManifest(&sManifest)) &&
         GXSUCCEEDED(CreateDataPool(ppDataPool, szName, sManifest.pTypes, sManifest.pVariables)))
      {
        if(sManifest.pImportFiles)
        {
          for(auto it = sManifest.pImportFiles->begin(); it != sManifest.pImportFiles->end(); ++it)
          {
            // ����Ҳ��������
            (*ppDataPool)->ImportDataFromFileW((GXLPCWSTR)*it);
          }
        }
        return GX_OK;
      }
      return GX_FAIL;
    }
    else {
      return CreateDataPool(ppDataPool, szName, NULL, NULL);
    }
    return GX_FAIL;
  }

  GXHRESULT DataPool::CompileFromMemory(DataPool** ppDataPool, GXLPCSTR szName, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength)
  {
    DataPoolCompiler* pResolver = NULL;
    GXHRESULT hval = szDefinitionCodes == NULL ? GX_OK :
      DataPoolCompiler::CreateFromMemory(&pResolver, pInclude, szDefinitionCodes, nCodeLength);
    if(GXSUCCEEDED(hval))
    {
      hval = CreateFromResolver(ppDataPool, szName, pResolver);
      SAFE_RELEASE(pResolver);
    }
    return hval;
  }

  const clBufferBase* DataPool::IntGetEntryBuffer() const
  {
    return &m_VarBuffer;
  }

  class DefaultDataPoolInclude : public DataPoolInclude
  {
  public:
    GXHRESULT Open(IncludeType eIncludeType, GXLPCWSTR pFileName, GXLPVOID lpParentData, GXLPCVOID *ppData, GXUINT *pBytes)
    {
      //clStringW str = pFileName;
      //clpathfile::RemoveFileSpecW(str);
      //clpathfile::CombinePathW(str, str, pFileName);
      clstd::File file;
      if(file.OpenExistingW(pFileName) && file.MapToBuffer((CLBYTE**)ppData, 0, 0, pBytes)) {
        return GX_OK;
      }
      return GX_FAIL;
    }

    GXHRESULT Close(GXLPCVOID pData)
    {
      const CLBYTE* ptr = (const CLBYTE*)pData;
      SAFE_DELETE_ARRAY(ptr);
      return GX_OK;
    }
  };

  GXHRESULT DataPool::CompileFromFileW(DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, GXLPCWSTR szFilename, DataPoolInclude* pInclude)
  {
    clstd::File file;
    GXHRESULT hval = GX_FAIL;
    clStringW strFilenameW = szFilename;
    clpathfile::MakeFullPath(strFilenameW);
    if(file.OpenExistingW(strFilenameW))
    {
      clBuffer* pBuffer;
      DefaultDataPoolInclude IncludeImpl;
      if(file.MapToBuffer(&pBuffer)) {
        // TODO: ������ļ�����Ҫ���BOM����ת��ΪUnicode��ʽ
        clStringA strDefine;
        clStringA strFilenameA = strFilenameW;
        strDefine.Format("#FILE %s\n#LINE 1\n", (clStringA::LPCSTR)strFilenameA);
        pBuffer->Insert(0, (GXLPCSTR)strDefine, strDefine.GetLength());

        hval = CompileFromMemory(ppDataPool, szName, pInclude ? pInclude : &IncludeImpl, (GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize());
        delete pBuffer;
        pBuffer = NULL;
      }
    }
    else {
      hval = GX_E_OPEN_FAILED;
    }
    return hval;
  }

  //DataPool::LPCENUMDESC DataPool::IntGetEnum( GXUINT nPackIndex ) const
  //{
  //  // ************************************************************************
  //  // �����ʼд���˷���enum desc������
  //  // ����ʹ��Name�Զ�λ����Release�淵��ֵ�������Ż����ᵼ��Name�Զ�λ����Ч��ַ
  //  // Ϊ�˱�ָ֤���ȶ����ĳ��˷���ָ��
  //  //
  //  return &m_aEnums[nPackIndex];
  //  //return m_aEnumPck[nPackIndex];
  //}

  inline GXUINT ConvertToNewOffsetFromOldIndex(const STRINGSETDESC* pTable, int nOldIndex)
  {
    return (GXUINT)pTable[nOldIndex].offset;
  }

  void DataPool::CopyVariables(VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const STRINGSETDESC* pTable, GXINT_PTR lpBase)
  {
    const BUILDTIME::BTVarDescArray& SrcVector = *(BUILDTIME::BTVarDescArray*)pSrcVector;
    int i = 0;
    for(auto it = SrcVector.begin(); it != SrcVector.end(); ++it, ++i)
    {
      const BUILDTIME::BT_VARIABLE_DESC& sBtDesc = *it;
      VARIABLE_DESC& sDesc = pDestVarDesc[i];
      sDesc.TypeDesc = (GXUINT)((GXINT_PTR)&sDesc.TypeDesc - (GXINT_PTR)&m_aTypes[((BUILDTIME::BT_TYPE_DESC*)sBtDesc.GetTypeDesc())->nIndex]);
      sDesc.nName     = ConvertToNewOffsetFromOldIndex(pTable, (int)sBtDesc.nName);
      sDesc.nOffset   = sBtDesc.nOffset;
      sDesc.nCount    = sBtDesc.nCount;
      sDesc.bDynamic  = sBtDesc.bDynamic;
      sDesc.bConst    = sBtDesc.bConst;

#ifdef DEBUG_DECL_NAME
      sDesc.Name      = (DataPool::LPCSTR)(lpBase + sDesc.nName);
      //TRACE("VAR:%16s %16s %8d\n", sDesc.GetTypeDesc()->Name, sDesc.Name, sDesc.nOffset);
#endif // DEBUG_DECL_NAME
    }
  }

  clsize DataPool::LocalizePtr()
  {
    GXLPBYTE ptr = (GXLPBYTE)m_Buffer.GetPtr();

    auto cbTypes     = m_nNumOfTypes * sizeof(TYPE_DESC);
    auto cbGVSIT     = (m_nNumOfVar + m_nNumOfMember + m_nNumOfEnums) * sizeof(SortedIndexType);
    auto cbVariables = m_nNumOfVar * sizeof(VARIABLE_DESC);
    auto cbMembers   = m_nNumOfMember * sizeof(VARIABLE_DESC);
    auto cbEnums     = m_nNumOfEnums * sizeof(ENUM_DESC);

    m_aTypes      = (TYPE_DESC*)ptr;
    m_aGSIT       = (SortedIndexType*)(ptr + cbTypes);
    m_aVariables  = (VARIABLE_DESC*)(ptr + cbTypes + cbGVSIT);

    if(m_nNumOfMember) {
      m_aMembers = (VARIABLE_DESC*)(ptr + cbTypes + cbGVSIT + cbVariables);
    }

    if(m_nNumOfEnums) {
      m_aEnums = (ENUM_DESC*)(ptr + cbTypes + cbGVSIT + cbVariables + cbMembers);
    }

    return cbTypes + cbGVSIT + cbVariables + cbMembers + cbEnums;
  }

  void DataPool::DbgIntDump()
  {
    TRACE("========= Types =========\n");
    for(GXUINT i = 0; i < m_nNumOfTypes; ++i) {
      TRACE("%16s %8d\n", m_aTypes[i].GetName(), m_aTypes[i].cbSize);
    }

    TRACE("========= Variables =========\n");
    for(GXUINT i = 0; i < m_nNumOfVar; ++i)
    {
      const auto& v = m_aVariables[i];
      if(v.nCount > 1) {
        TRACE("%16s %12s[%4d] %8d[%d]\n", v.TypeName(), v.VariableName(), v.nCount, v.nOffset, v.GetUsageSize());
      }
      else {
        TRACE("%16s %16s %10d[%d]\n", v.TypeName(), v.VariableName(), v.nOffset, v.GetUsageSize());
      }
    }

    TRACE("========= Members =========\n");
    for(GXUINT i = 0; i < m_nNumOfMember; ++i){
      const auto& v = m_aMembers[i];
      if(v.nCount > 1) {
        TRACE("%16s %12s[%4d] %8d[%d]\n", v.TypeName(), v.VariableName(), v.nCount, v.nOffset, v.GetUsageSize());
      }
      else {
        TRACE("%16s %16s %10d[%d]\n", v.TypeName(), v.VariableName(), v.nOffset, v.GetUsageSize());
      }
    }
  }

  void DataPool::LocalizeTables(BUILDTIME& bt, GXSIZE_T cbVarSpace)
  {
    // [MAIN BUFFER �ṹ��]:
    // #.Type desc table            ����������
    // #.GVSIT
    // #.Variable desc table        ����������
    // #.Struct members desc table  ��Ա����������
    // #.enum desc table            ö��������
    // #.Strings                    �������������ַ������ַ�����
    // #.Variable Data Space        �����ռ�

    m_nNumOfTypes  = (GXUINT)bt.m_TypeDict.size();
    m_nNumOfVar    = (GXUINT)bt.m_aVar.size();
    m_nNumOfMember = (GXUINT)bt.m_aStructMember.size();
    m_nNumOfEnums  = (GXUINT)bt.m_aEnumPck.size();

    auto cbTypes     = m_nNumOfTypes * sizeof(TYPE_DESC);
    auto cbGVSIT     = (m_nNumOfVar + m_nNumOfMember + m_nNumOfEnums) * sizeof(SortedIndexType);
    auto cbVariables = m_nNumOfVar * sizeof(VARIABLE_DESC);
    auto cbMembers   = m_nNumOfMember * sizeof(VARIABLE_DESC);
    auto cbEnums     = m_nNumOfEnums * sizeof(ENUM_DESC);
    auto cbHeader    = cbTypes + cbGVSIT+ cbVariables + cbMembers + cbEnums;
    m_Buffer.Resize(cbHeader + bt.NameSet.buffer_size() + cbVarSpace, FALSE);

#ifdef _DEBUG
    auto cbDbgSave = m_Buffer.GetSize();
#endif // #ifdef _DEBUG

    STRINGSETDESC* pTable = new STRINGSETDESC[bt.NameSet.size()];
    bt.NameSet.sort(pTable);

    memset((GXLPBYTE)m_Buffer.GetPtr() + cbHeader, 0, bt.NameSet.buffer_size());

    GXINT_PTR lpBase = (GXINT_PTR)bt.NameSet.GatherToBuffer(&m_Buffer, cbHeader);
    //m_StringBase = lpBase;

    ASSERT(cbDbgSave == m_Buffer.GetSize()); // ȷ��GatherToBuffer����ı�Buffer�ĳ���

    LocalizePtr();
    //ASSERT(m_StringBase == lpBase);


    // * ���¸��Ʊ�Ĳ����о������ַ����ض�λ

    // ��������������
    GXUINT nIndex = 0;
    for(auto it = bt.m_TypeDict.begin(); it != bt.m_TypeDict.end(); ++it, ++nIndex) {
      BUILDTIME::BT_TYPE_DESC& sBtType = it->second;
      TYPE_DESC& sType = m_aTypes[nIndex];
      sType.nName        = ConvertToNewOffsetFromOldIndex(pTable, (int)sBtType.nName);
      sType.Cate         = sBtType.Cate;
      sType.cbSize       = sBtType.cbSize;
      //sType.nMemberIndex = sBtType.nMemberIndex;
      sType.nMemberCount = sBtType.nMemberCount;

      if(sType.Cate == T_STRUCT) {
        sType.Member = (GXUINT)((GXUINT_PTR)&m_aMembers[sBtType.Member] - (GXUINT_PTR)&sType.Member);
      }
      else if(sType.Cate == T_ENUM || sType.Cate == T_FLAG) {
        sType.Member = (GXUINT)((GXUINT_PTR)&m_aEnums[sBtType.Member] - (GXUINT_PTR)&sType.Member);
      }

#ifdef DEBUG_DECL_NAME
      sType.Name         = (DataPool::LPCSTR)(lpBase + sType.nName);
      TRACE("TYPE[%d]: %16s %8d\n", nIndex, sType.Name, sType.cbSize);
#endif // DEBUG_DECL_NAME

      sBtType.nIndex = nIndex;
    }

    // ���Ʊ����ͳ�Ա����������
    CopyVariables(m_aVariables, &bt.m_aVar, pTable, lpBase);
    CopyVariables(m_aMembers, &bt.m_aStructMember, pTable, lpBase);

    // ����ö��������
    nIndex = 0;
    for(auto it = bt.m_aEnumPck.begin(); it != bt.m_aEnumPck.end(); ++it, ++nIndex) {
      const ENUM_DESC& sBtDesc = *it;
      ENUM_DESC& sDesc = m_aEnums[nIndex];

      sDesc.nName = ConvertToNewOffsetFromOldIndex(pTable, (int)sBtDesc.nName);
      sDesc.Value = sBtDesc.Value;

#ifdef DEBUG_DECL_NAME
      sDesc.Name  = (DataPool::LPCSTR)(lpBase + sDesc.nName);
#endif // DEBUG_DECL_NAME
    }

    // #.�����������Ч�� m_aVariables�� m_aMembers �� m_aEnums
    // #.���������������е� nName ��Ա�����ᰴ��������
    // #.���򲻸ı�m_aVariables�� m_aMembers �� m_aEnums�ϵ�˳��
    //   ������������Ļ�����Щ������������ŵ�GSIT��
    // #.nName ������ʱʹ�����Զ�λ�������Զ�λ��������ֵ���ܱ�֤��
    //   GSIT���ǵ����ģ������Զ�λ�����ں������
    GenGSIT();

    // �Զ�λ��ת��
    SelfLocalizable(m_aTypes,     m_nNumOfTypes,  lpBase);
    SelfLocalizable(m_aVariables, m_nNumOfVar,    lpBase);
    SelfLocalizable(m_aMembers,   m_nNumOfMember, lpBase);
    SelfLocalizable(m_aEnums,     m_nNumOfEnums,  lpBase);

    new(&m_VarBuffer) clstd::RefBuffer((GXLPBYTE)lpBase + bt.NameSet.buffer_size(), cbVarSpace);
    memset(m_VarBuffer.GetPtr(), 0, m_VarBuffer.GetSize());   // ֻ��������ε��ڴ�

    SAFE_DELETE_ARRAY(pTable);
  }

  void DataPool::GenGSIT()
  {
    SortNames<VARIABLE_DESC>(m_aVariables, GSIT_Variables, 0, m_nNumOfVar);

    for(GXUINT i = 0; i < m_nNumOfTypes; ++i) {
      const TYPE_DESC& t = m_aTypes[i];

      if(t.nMemberCount >= 1)
      {
        switch(t.Cate)
        {
        case T_STRUCT:
          //SortNames<VARIABLE_DESC>(m_aMembers, GSIT_Members + t.nMemberIndex, t.nMemberIndex, t.nMemberCount);
          //TRACE("var index:%d\n", t.GetMemberIndex(m_aMembers));
          SortNames<VARIABLE_DESC>(t.GetMembers(), GSIT_Members + t.GetMemberIndex(m_aMembers), 0, t.nMemberCount);
          break;
        case T_FLAG:
        case T_ENUM:
          //SortNames<ENUM_DESC>(m_aEnums, GSIT_Enums + t.nMemberIndex, t.nMemberIndex, t.nMemberCount);
          //TRACE("enum index:%d\n", t.GetEnumIndex(m_aEnums));
          SortNames<ENUM_DESC>(t.GetEnumMembers(), GSIT_Enums + t.GetEnumIndex(m_aEnums), 0, t.nMemberCount);
          break;
        }
      }
    }
  }

  template<class DescT>
  void DataPool::SelfLocalizable(DescT* pDescs, int nCount, GXINT_PTR lpBase)
  {
    for(int i = 0; i < nCount; ++i)
    {
      GXUINT* pName = &pDescs[i].nName;
      ASSERT(lpBase - (GXINT_PTR)pName > 0); // ��֤ lpBase �� &nName �ĺ������

      *pName = (GXUINT)(lpBase - (GXINT_PTR)pName + *pName);

#ifdef DEBUG_DECL_NAME
      DataPool::LPCSTR szName = (DataPool::LPCSTR)((GXINT_PTR)&pDescs[i].nName + pDescs[i].nName);
      ASSERT(szName == pDescs[i].Name || pDescs[i].Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
    }
  }

  template<class DescT>
  void DataPool::SortNames( const DescT* pDescs, SortedIndexType* pDest, int nBegin, int nCount)
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
      //pContext[i].index = nBegin + i; // GVSIT�������Ѿ�����Group��ƫ��
      //pContext[i].name = pDescs[i + nBegin].nName;
      pContext[i].index = i; // GVSIT�������Ѿ�����Group��ƫ��
      pContext[i].name = pDescs[i].nName;
      //m_aGVSIT[nTargetTopIndex + i] = pContext[i].index;
      pDest[i] = pContext[i].index;

      //TRACE("name:%d\n", pContext[i].name);

      mask |= pContext[i].name - prev;
      prev = pContext[i].name;
    }

    if(mask >> 31) // ��������ǲ����Ѿ����մӴ�У��˳��������
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

  GXBOOL DataPool::IntFindEnumFlagValue( LPCTD pTypeDesc, LPCSTR szName, EnumFlag* pOutEnumFlag ) GXCONST
  {
    const auto* p = GSIT_Enums + pTypeDesc->GetEnumIndex(m_aEnums);
    int begin = 0;
    int end = pTypeDesc->nMemberCount;
    LPCED aEnums = pTypeDesc->GetEnumMembers();

    while(begin <= end - 1)
    {
      int mid = (begin + end) >> 1;
      ASSERT(p[mid] < m_nNumOfEnums); // �����϶������ܴ�С
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

#if 0
#ifdef ENABLE_DATAPOOL_WATCHER
    for (GXUINT i = 0; i < pTypeDesc->nMemberCount; i++) {
      TRACE("%*s %s\n", -50, aEnums[p[i]].GetName(), aEnums[p[i]].Name);
    }
#endif // ENABLE_DATAPOOL_WATCHER
#endif // #ifdef _DEBUG

    return FALSE;
  }

  template<class _TIter>
  _TIter& DataPool::first_iterator(_TIter& it)
  {
    it.pDataPool = this; // ������½ṹ��������Ϊconst
    it.pVarDesc  = m_aVariables;
    it.pBuffer   = &m_VarBuffer;
    it.nOffset   = 0;
    it.index     = (GXUINT)-1;
    return it;
  }

  DataPoolUtility::iterator DataPool::begin()
  {
    DataPoolUtility::iterator it;
    return first_iterator<DataPoolUtility::iterator>(it);
  }

  DataPoolUtility::iterator DataPool::end()
  {
    DataPoolUtility::iterator it;
    first_iterator(it);
    it.pVarDesc += m_nNumOfVar;
    return it;
  }

  DataPoolUtility::named_iterator DataPool::named_begin()
  {
    DataPoolUtility::named_iterator it;
    return first_iterator(it);
  }

  DataPoolUtility::named_iterator DataPool::named_end()
  {
    DataPoolUtility::named_iterator it;
    first_iterator(it);
    it.pVarDesc += m_nNumOfVar;
    return it;
  }

  GXBOOL DataPool::SaveW( GXLPCWSTR szFilename )
  {
    clFile file;
    if(file.CreateAlwaysW(szFilename)) {
      return Save(file);
    }
    CLOG_ERRORW(L"can not open file \"%s\".", szFilename);
    return FALSE;
  }

#define SAVE_TRACE TRACE

  GXBOOL DataPool::Save( clFile& file )
  {
    //
    // TODO: ��ΪSmartRepository����
    //
#ifdef DEBUG_DECL_NAME
    TRACE("�����ֵ���ģʽ,�ļ��ṹ���к���ָ�룬Ҫ�ر��������ܱ���");
    CLBREAK;
#else
    // #.[FILE_HEADER]
    // #.Variable space buffer header
    // #.Array buffer header[0]
    // #.Array buffer header[1]
    // ...
    // #.DataPool::m_Buffer - Variable space
    // #.�ַ����������ַ����б�
    // #.Variable space ���ض�λ�� + Variable data
    // #.Array Buffer[0] ���ض�λ�� + data
    // #.Array Buffer[1] ���ض�λ�� + data
    // ...
    FILE_HEADER header;
    header.dwFlags          = 0;
    header.nNumOfTypes      = m_nNumOfTypes;
    header.nNumOfVar        = m_nNumOfVar;
    header.nNumOfMember     = m_nNumOfMember;
    header.nNumOfEnums      = m_nNumOfEnums;
    header.cbDescTabNames   = (GXUINT)IntGetRTDescNames();
    header.cbVariableSpace  = (GXUINT)m_VarBuffer.GetSize();
    header.nNumOfStrings    = 0;
    header.cbStringSpace    = 0;
    header.nNumOfArrayBufs  = 0;
    //header.cbArraySpace     = 0;



    StringSetW sStringVar; // �ַ�����������
    StringSetA sStringVarA; // �ַ�����������
    GXUINT nRelOffset = 0;  // �ض�λ��Ŀ�ʼƫ��
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

    // ����Ԥ����
    // 1.�ռ��ַ�����������
    // 2.�ռ��ض�λ��
    auto _itBegin = begin();
    auto _itEnd = end();
    DataPoolUtility::EnumerateVariables2
      <DataPoolUtility::iterator, DataPoolUtility::element_iterator, DataPoolUtility::element_reverse_iterator>
      (_itBegin, _itEnd, 
#ifdef _DEBUG
      [&sStringVar, &sStringVarA, &header, &nRelOffset, &BufferTab, &pCurrBufDesc, &sDbgBufSet, &bd]
#else
      [&sStringVar, &sStringVarA, &header, &nRelOffset, &BufferTab, &pCurrBufDesc, &bd]
#endif // #ifdef _DEBUG
    (int bArray, DataPool::iterator& it, int nDepth) 
    {
      MOVariable var = it.ToVariable();
      const auto pCheckBuffer = it.pBuffer;
      if(pCheckBuffer != pCurrBufDesc->pBuffer) // �л���Buffer
      {
        //TRACE("%08x:%08x\n", pCheckBuffer, pCurrBufDesc->pBuffer);

#ifdef _DEBUG
        // sDbgBufSet����֮ǰ��buffer���µ�bufferһ���������������
        ASSERT(sDbgBufSet.find(pCheckBuffer) == sDbgBufSet.end());
        sDbgBufSet.insert(pCheckBuffer);
#endif // #ifdef _DEBUG

        // ���buffer����ʱ���ۼ�ƫ��һ������buffer��С
        ASSERT_X86(nRelOffset == pCurrBufDesc->pBuffer->GetSize());

        ++pCurrBufDesc;

        // һ�������桰if(bArray)����ע���������Ҳ�ǰ���˳����ֵ�
        ASSERT(pCurrBufDesc->pBuffer == pCheckBuffer);

        //bd.pBuffer = pCheckBuffer;
        //BufferTab.push_back(bd);
        //pCurrBufDesc = &BufferTab.back();
        nRelOffset = 0;
      }

      ASSERT(bArray || it.pVarDesc->GetTypeCategory() != T_STRUCT);



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
            pCurrBufDesc = (BUFFER_SAVELOAD_DESC*)((GXINT_PTR)&BufferTab.front() + nCurOffset); // vectorָ��ı䣬�������һ��ָ��
          }
          //pCurrBufDesc->pTypeDesc = it.pVarDesc->GetTypeDesc();
          //else {
          //  pCurrBufDesc->ZeroArray.push_back(nRelOffset);
          //}
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
      else if(it.pVarDesc->GetTypeCategory() == T_STRING)
      {
        ASSERT( ! bArray);
        sStringVar.insert(var.ToStringW());
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_String);
        nRelOffset += SIZEOF_PTR32;
        ASSERT_X86(var.GetSize() == 4);
        ++header.nNumOfStrings;
      }
      else if(it.pVarDesc->GetTypeCategory() == T_STRINGA)
      {
        ASSERT( ! bArray);
        sStringVarA.insert(var.ToStringA());
        pCurrBufDesc->RelTable.push_back(nRelOffset | BUFFER_SAVELOAD_DESC::RelocalizeType_StringA);
        nRelOffset += SIZEOF_PTR32;
        ASSERT_X86(var.GetSize() == 4);
        ++header.nNumOfStrings;
      }
      else if(it.pVarDesc->GetTypeCategory() == T_OBJECT)
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

      // �ۼ�ƫ����֤
      ASSERT(nRelOffset <= it.pBuffer->GetSize());
    });


    if(SIZEOF_PTR > SIZEOF_PTR32) // ����ʱ64λƽָ̨�뵽�����ļ�32λ�޷�������
    {      
      ASSERT( ! BufferTab.empty()) // ������m_VarBuffer
      header.cbVariableSpace = (GXUINT)BufferTab.front().GetDiskBufferSize();
    }

    //header.nRelocalizeOffset  = sizeof(FILE_HEADER);
    //header.nRelocalizeCount   = RelocalizeTab.size();
    header.nBufHeaderOffset   = (GXUINT)(sizeof(FILE_HEADER));
    header.nDescOffset        = (GXUINT)(sizeof(FILE_HEADER) + sizeof(FILE_BUFFERHEADER) * (header.nNumOfArrayBufs + 1));
    header.nStringVarOffset   = (GXUINT)(header.nDescOffset + (m_Buffer.GetSize() - m_VarBuffer.GetSize()));
    header.nBuffersOffset     = (GXUINT)header.nStringVarOffset + sStringVar.buffer_size();

    header.nNumOfPtrVars      = (GXUINT)BufferTab.front().RelTable.size();
    header.cbStringSpace      = (GXUINT)sStringVar.buffer_size();


    clBuffer BufferToWrite; // ��ʱʹ�õĻ�����

    // �ļ�ͷ
    V_WRITE(file.Write(&header, sizeof(FILE_HEADER)), "Failed to write file header.");


    // ���ݻ�����Ϣͷ
    ASSERT(file.GetPointer() == header.nBufHeaderOffset); // ��ǰָ����buffer������ʼƫ��һ��
    FILE_BUFFERHEADER fbh;
    for(auto it = BufferTab.begin(); it != BufferTab.end(); ++it)
    {
      fbh.nBufferSize = (GXUINT)it->GetDiskBufferSize();
      fbh.nNumOfRel   = (GXUINT)it->RelTable.size();
      fbh.nType       = it->pTypeDesc == NULL ? 0
        : header.nDescOffset + (GXUINT)((GXUINT_PTR)it->pTypeDesc - (GXUINT_PTR)m_aTypes);

      SAVE_TRACE("save buffer type:%s\n", it->pTypeDesc ? it->pTypeDesc->GetName() : "<global>");

      //SAVE_TRACE("1.Buffer Ptr:%08x\n", it->pBuffer);
      ASSERT(fbh.nBufferSize != 0);

      V_WRITE(file.Write(&fbh, sizeof(fbh)), "Failed to write fbh.");
    }



    // ȥ�������ռ�� DataPool::m_Buffer
    //DbgIntDump();
    ASSERT(file.GetPointer() == header.nDescOffset);
    if(SIZEOF_PTR > SIZEOF_PTR32)
    {
      BufferToWrite.Resize((GXUINT)m_Buffer.GetSize() - m_VarBuffer.GetSize(), FALSE);
      memcpy(BufferToWrite.GetPtr(), m_Buffer.GetPtr(), BufferToWrite.GetSize());

      // ��ζ���ַ����ο�[MAIN BUFFER �ṹ��]
      const GXUINT_PTR nDeltaVarToType = (GXUINT_PTR)m_aVariables - (GXUINT_PTR)m_aTypes;
      const GXUINT_PTR nDeltaMemberToType = (GXUINT_PTR)m_aMembers - (GXUINT_PTR)m_aTypes;
      IntChangePtrSize(4, (VARIABLE_DESC*)((GXUINT_PTR)BufferToWrite.GetPtr() + nDeltaVarToType), m_nNumOfVar);
      IntClearChangePtrFlag((TYPE_DESC*)BufferToWrite.GetPtr(), m_nNumOfTypes);

      V_WRITE(file.Write(BufferToWrite.GetPtr(), (GXUINT)BufferToWrite.GetSize()), "Failed to write global buffer.");
    }
    else {
      V_WRITE(file.Write(m_Buffer.GetPtr(), (GXUINT)m_Buffer.GetSize() - header.cbVariableSpace), "Failed to write global buffer.");
    }


    
    // �ַ����������ַ����б�
    clFixedBuffer StringVarBuf;
    ASSERT(file.GetPointer() == header.nStringVarOffset); // ��ǰָ�����ַ���������ʼƫ��һ��
    StringVarBuf.Resize(sStringVar.buffer_size(), TRUE);
    sStringVar.GatherToBuffer(&StringVarBuf, 0);
    V_WRITE(file.Write(StringVarBuf.GetPtr(), (GXUINT)StringVarBuf.GetSize()), "Failed to write variable string buffer.");


    // ���ݻ��������
    GXUINT nBufferIndex = 1;
    for(auto it = BufferTab.begin(); it != BufferTab.end(); ++it)
    {
      SAVE_TRACE("2.Buffer Ptr:%08x %d\n", it->pBuffer, it->pBuffer->GetSize());

      BufferToWrite.Resize(it->GetDiskBufferSize(), FALSE);

      const auto nCheck = it->RelocalizePtr(&BufferToWrite, it->pBuffer, [this, &sStringVar, header, &BufferTab, &nBufferIndex]
      (BUFFER_SAVELOAD_DESC::RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc)
      {
        //SAVE_TRACE("rel offset:%d\n", (GXINT_PTR)pSrc - (GXINT_PTR)it->pBuffer->GetPtr());
        switch(type)
        {
        case BUFFER_SAVELOAD_DESC::RelocalizeType_String:
          {
            clStringW* pStr = (clStringW*)pSrc;
            //SAVE_TRACE("str:%s\n", *pStr);
            if(pStr)
            {
              auto itSetSet = sStringVar.find(*pStr);
              ASSERT(itSetSet != sStringVar.end());
              *(GXUINT*)pDest = (GXUINT)(itSetSet->second.offset + header.nStringVarOffset);
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
              *(GXUINT*)pDest = 0; // ����Ϊ0��buffer����Ϊ��
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
      // �ض�λ��
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

  GXBOOL DataPool::Load( clFile& file, GXDWORD dwFlag )
  {
    //
    // TODO: ��ΪSmartRepository����
    //
    ASSERT(m_Buffer.GetSize() == 0); // ��Ч��DataPool������ִ��Load����

#ifdef DEBUG_DECL_NAME
    TRACE("�����ֵ���ģʽ,�ļ��ṹ���к���ָ�룬Ҫ�ر��������ܼ���");
    CLBREAK;
#else

    FILE_HEADER header;
    V_READ(file.Read(&header, sizeof(FILE_HEADER)), "Can not load file header.");

    m_nNumOfTypes  = header.nNumOfTypes;
    m_nNumOfVar    = header.nNumOfVar;
    m_nNumOfMember = header.nNumOfMember;
    m_nNumOfEnums  = header.nNumOfEnums;



    //
    // Buffer header
    //
    typedef clvector<FILE_BUFFERHEADER> FileBufArray;
    FileBufArray BufHeaders;  // �ļ���¼
    BufDescArray BufferTab;   // ����ʱ��¼
    BUFFER_SAVELOAD_DESC bd = {0};
    FILE_BUFFERHEADER BufHeader = {0};

    if(file.GetPointer() != header.nBufHeaderOffset) {
      file.SetPointer(header.nBufHeaderOffset, 0);
    }
    const int nNumOfBuffers = header.nNumOfArrayBufs + 1;
    BufferTab.insert(BufferTab.begin(), nNumOfBuffers, bd);
    BufferTab.front().pBuffer = &m_VarBuffer;

    // �����ļ���¼������BufferHeader����
    BufHeaders.insert(BufHeaders.begin(), nNumOfBuffers, BufHeader);
    V_READ(file.Read(&BufHeaders.front(), sizeof(FILE_BUFFERHEADER) * nNumOfBuffers), "Can not load buffer header.");





    // �������ο�[MAIN BUFFER �ṹ��]
    const GXSIZE_T nDescHeaderSize = IntGetRTDescHeader() + header.cbDescTabNames;
    const GXSIZE_T cbGlobalVariable = header.cbVariableSpace + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(header.nNumOfPtrVars);
    const GXSIZE_T nMainBufferSize_0 = nDescHeaderSize + cbGlobalVariable;
    GXSIZE_T nMainBufferSize = nMainBufferSize_0;




    //dwFlag = 0; // ǿ���������Σ�����




    //*
    if(TEST_FLAG(dwFlag, DataPoolLoad_ReadOnly))
    {
      //m_bReadOnly = 1;
      SET_FLAG(m_dwRuntimeFlags, RuntimeFlag_Readonly);
      nMainBufferSize += header.cbStringSpace;
      nMainBufferSize += sizeof(DataPoolArray) * header.nNumOfArrayBufs;

      // ������1��ʼ��[0]��ȫ�ֱ����ռ䣬�Ѿ�������
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

    // һ�ζ������ȫ�ֱ�����������ݣ��������������������ַ����б��
    V_READ(file.Read(m_Buffer.GetPtr(), (GXUINT)nDescHeaderSize), "Can not load desc header.");

    const clsize cbDesc = LocalizePtr();
    new(&m_VarBuffer) clstd::RefBuffer((GXLPBYTE)m_Buffer.GetPtr() + cbDesc + header.cbDescTabNames, cbGlobalVariable);


    // 64λ����չ�������е�ָ��
    if(SIZEOF_PTR > SIZEOF_PTR32)
    {
      IntChangePtrSize(8, m_aVariables, m_nNumOfVar);
      IntClearChangePtrFlag(m_aTypes, m_nNumOfTypes);
    }




    // �ַ����������ַ����б�
    clFixedBuffer StringVarBuf;
    GXLPBYTE pStringBegin;
    if(header.cbStringSpace)
    {
      if(file.GetPointer() != header.nStringVarOffset) {
        file.SetPointer(header.nStringVarOffset, 0);
      }
      if(TEST_FLAG(dwFlag, DataPoolLoad_ReadOnly))
      {
        pStringBegin = (GXLPBYTE)m_Buffer.GetPtr() + nMainBufferSize_0;
      }
      else
      {
        StringVarBuf.Resize(header.cbStringSpace, FALSE);
        pStringBegin = (GXLPBYTE)StringVarBuf.GetPtr();
      }
      V_READ(file.Read(pStringBegin, header.cbStringSpace), "Can not load variable strings.");
    }
    

    // ��ֻ��ģʽ�£��������ʼ��������
    ASSERT(m_aTypes != NULL);
    if(TEST_FLAG(dwFlag, DataPoolLoad_ReadOnly))
    {
      GXLPBYTE lpBufferPtr = (GXLPBYTE)m_Buffer.GetPtr() + nMainBufferSize_0 + header.cbStringSpace;

      for(int i = 1; i < nNumOfBuffers; ++i)
      {
        const FILE_BUFFERHEADER& fbh = BufHeaders[i];
        BUFFER_SAVELOAD_DESC& bd = BufferTab[i];

        // ��λ��̬��������
        bd.pTypeDesc = (TYPE_DESC*)((GXINT_PTR)m_aTypes + (fbh.nType - header.nDescOffset));

        const clsize nBufferSize = fbh.nBufferSize + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(fbh.nNumOfRel);
        bd.pBuffer = new(lpBufferPtr) DataPoolArray(lpBufferPtr, nBufferSize);
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

        // ��λ��̬��������
        bd.pTypeDesc = (TYPE_DESC*)((GXINT_PTR)m_aTypes + (fbh.nType - header.nDescOffset));

        // ���䶯̬����ռ䣬������8�����ʹ�С
        bd.pBuffer = new DataPoolArray(bd.pTypeDesc->cbSize * 8);
        ((DataPoolArray*)bd.pBuffer)->Resize(fbh.nBufferSize + BUFFER_SAVELOAD_DESC::GetPtrAdjustSize(fbh.nNumOfRel), FALSE);
      }
    }


    // ��ʼ��ȡȫ�ֱ����Ͷ�̬��������
    if(file.GetPointer() != header.nBuffersOffset) {
      file.SetPointer(header.nBuffersOffset, 0);
    }

    // buffer ���е�һ������ȫ�ֱ����ĳߴ磬�϶�����ȵ�
    ASSERT(BufferTab.front().pBuffer->GetSize() == cbGlobalVariable);

    clBuffer BufferForRead;
    for(int i = 0; i < nNumOfBuffers; ++i)
    {
      const FILE_BUFFERHEADER& fbh = BufHeaders[i];
      BUFFER_SAVELOAD_DESC& bd = BufferTab[i];

      // Ϊ�ض�λ��Ԥ���ռ�
      if(fbh.nNumOfRel) {
        //BufferTab[i].RelTable.insert(BufferTab[i].RelTable.begin(), fbh.nNumOfRel, 0);
        bd.RelTable.reserve(fbh.nNumOfRel);
      }


      //
      // PS���ض�λ���Ȼ���ռ�������Ϊʲôë��Ҫ�ݹ����ռ�����һ��ѭ����λ���������һ�θ㶨��
      //


      // ����buffer������˵��,�ռ��ض�λ��
      if(fbh.nType) {
        ASSERT(bd.pBuffer != NULL);
        bd.GenerateRelocalizeTable(bd.pTypeDesc);
      }
      else {
        // ȫ�ֱ���
        bd.GenerateRelocalizeTable(0, m_aVariables, m_nNumOfVar);
      }

      BufferForRead.Resize(bd.pBuffer->GetSize() - bd.GetPtrAdjustSize(), FALSE);


      TRACE("load buffer type:%s\n", bd.pTypeDesc ? bd.pTypeDesc->GetName() : "<global>");


      // ��ȡ�ض�λ��
      //if( ! rbd.RelTable.empty()) {
      //  file.Read(&rbd.RelTable.front(), (GXUINT)rbd.RelTable.size() * sizeof(GXUINT));
      //  //rbd.DbgCheck();
      //  // �ض�λ��ƫ�ƿ϶���С�ڻ�����
      //  ASSERT(rbd.pBuffer->GetSize() >= (rbd.RelTable.front() & BUFFER_SAVELOAD_DESC::RelocalizeOffsetMask));
      //  ASSERT(rbd.pBuffer->GetSize() >= (rbd.RelTable.back() & BUFFER_SAVELOAD_DESC::RelocalizeOffsetMask))
      //}

      V_READ(file.Read(BufferForRead.GetPtr(), (GXUINT)BufferForRead.GetSize()), "Can not load buffer data.");

      const auto nCheck = bd.RelocalizePtr(bd.pBuffer, &BufferForRead, [&pStringBegin, &BufferTab, &header, &dwFlag, this]
      (BUFFER_SAVELOAD_DESC::RelocalizeType type, GXUINT nOffset, GXLPBYTE& pDest, GXLPCBYTE& pSrc)
      {
        switch(type)
        {
        case BUFFER_SAVELOAD_DESC::RelocalizeType_String:
          {
            GXLPCWSTR str = (GXLPCWSTR)((GXINT_PTR)pStringBegin + *(GXUINT*)pSrc - header.nStringVarOffset);
            if(TEST_FLAG(dwFlag, DataPoolLoad_ReadOnly))
            {
              *(GXLPCWSTR*)pDest = str;
              INC_DBGNUMOFSTRING;
            }
            else if(str[0]) {
              new(pDest) clStringW(str);
              INC_DBGNUMOFSTRING;
              //TRACEW(L"str:%s %s\n", str, *(clStringW*)pDest);
            }
            else {
              *(GXLPCWSTR*)pDest = NULL;
            }
          }
          break;
        case BUFFER_SAVELOAD_DESC::RelocalizeType_Array:
          {
            GXUINT index = *(GXUINT*)pSrc;
            ASSERT(index < BufferTab.size());
            if(index)
            {
              // �������϶��Ѿ���������
              ASSERT(BufferTab[index].pBuffer != NULL);
              *(clBufferBase**)pDest = BufferTab[index].pBuffer;
              INC_DBGNUMOFARRAY;
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
#endif // #ifdef DEBUG_DECL_NAME
    return TRUE;
  }

  GXSIZE_T DataPool::IntGetRTDescHeader()
  {
    auto cbTypes     = m_nNumOfTypes * sizeof(TYPE_DESC);
    auto cbGVSIT     = (m_nNumOfVar + m_nNumOfMember + m_nNumOfEnums) * sizeof(SortedIndexType);
    auto cbVariables = m_nNumOfVar * sizeof(VARIABLE_DESC);
    auto cbMembers   = m_nNumOfMember * sizeof(VARIABLE_DESC);
    auto cbEnums     = m_nNumOfEnums * sizeof(ENUM_DESC);
    return (cbTypes + cbGVSIT + cbVariables + cbMembers + cbEnums);
  }

  GXSIZE_T DataPool::IntGetRTDescNames()
  {
    return m_Buffer.GetSize() - IntGetRTDescHeader() - m_VarBuffer.GetSize();
  }

  GXUINT DataPool::IntChangePtrSize(GXUINT nSizeofPtr, VARIABLE_DESC* pVarDesc, GXUINT nCount)
  {
    // ��֤��ȫ�ֱ�����ʼ�������ǳ�Ա������ʼ
    ASSERT(pVarDesc->nOffset == 0);

    // ֻ����32λָ�뵽64λָ�����64λָ�뵽32λָ��ת��
    ASSERT(nSizeofPtr == 4 || nSizeofPtr == 8);
    
    GXUINT nNewOffset = 0;
    for(GXUINT i = 0; i < nCount; ++i)
    {
      VARIABLE_DESC& d = pVarDesc[i];
      const auto eCate = d.GetTypeCategory();
      d.nOffset = nNewOffset;

      if(d.IsDynamicArray()) {
        // ��̬�������һ��ָ��
        nNewOffset += nSizeofPtr; // sizeof(DataPoolArray*)
      }
      else
      {
        // ����Ѿ������ı��
        // ���û�е������������¼���������͵Ĵ�С
        // ���򲽽�������͵Ĵ�С�Ϳ���
        if(TEST_FLAG_NOT(eCate, TYPE_CHANGED_FLAG))
        {
          TYPE_DESC* pTypeDesc = (TYPE_DESC*)d.GetTypeDesc();
          GXUINT& uCate = *(GXUINT*)&pTypeDesc->Cate;
          SET_FLAG(uCate, TYPE_CHANGED_FLAG);
          switch(eCate)
          {
          case T_STRUCT:
            //pTypeDesc->cbSize = IntChangePtrSize(nSizeofPtr, pMembers, &pMembers[d.MemberBegin()], d.MemberCount());
            pTypeDesc->cbSize = IntChangePtrSize(nSizeofPtr, (VARIABLE_DESC*)d.MemberBeginPtr(), d.MemberCount());
            break;

          case T_STRING:
          case T_STRINGA:
          case T_OBJECT:
            ASSERT((pTypeDesc->cbSize == 4 && nSizeofPtr == 8) ||
              (pTypeDesc->cbSize == 8 && nSizeofPtr == 4));

            pTypeDesc->cbSize = nSizeofPtr;
            break;
          }
        }
        nNewOffset += d.GetSize();
      } // if(d.IsDynamicArray())
    }
    return nNewOffset;
  }

  void DataPool::IntClearChangePtrFlag( TYPE_DESC* pTypeDesc, GXUINT nCount )
  {
    // �������û�������޸ı�־����ʾ�������֮ǰ�ĵ�������
    //ASSERT(TEST_FLAG(pTypeDesc[i].Cate, TYPE_CHANGED_FLAG));
    for(GXUINT i = 0; i < nCount; ++i)
    {
      RESET_FLAG(*(GXUINT*)&pTypeDesc[i].Cate, TYPE_CHANGED_FLAG);
    }
  }

  //bool operator<(const WATCH_FIXED& a, const WATCH_FIXED& b)
  //{
  //  return pCallback < t.pCallback;
  //}


  GXBOOL DataPool::IntWatch( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    GXBOOL bret = FALSE;
    const auto dwFlags = pVar->GetCaps();
    const auto dwBan = DataPoolVariable::CAPS_STRUCT | DataPoolVariable::CAPS_ARRAY | DataPoolVariable::CAPS_DYNARRAY;

    // 1.�� watch ������ fixed ����
    // 2.�ṹ�壬��������� wacth
    // 3.Object����Ҳ���ܱ� watch
    if(TEST_FLAG(dwFlags, DataPoolVariable::CAPS_FIXED) &&
      TEST_FLAG_NOT(dwFlags, dwBan) && pVar->GetTypeCategory() != T_OBJECT)
    {
      WatchFixedList sWatchList;
      auto result = m_FixedDict.insert(clmake_pair(pVar->GetPtr(), sWatchList));
      bret = TRUE;

      WATCH_FIXED sWatch;
      sWatch.pCallback = pImpulseCallback;
      sWatch.lParam    = lParam;

      // DataPoolWatch ����
      if((GXINT_PTR)pImpulseCallback == 0) {
        ((DataPoolWatcher*)lParam)->AddRef();
      }

      result.first->second.insert(sWatch);
    }
    return bret;
  }

  GXBOOL DataPool::IntIgnore( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    auto itWatchSet = m_FixedDict.find(pVar->GetPtr());
    if(itWatchSet != m_FixedDict.end())
    {
      WatchFixedList& sVarWatchSet = itWatchSet->second;
      WATCH_FIXED sWatch;
      sWatch.pCallback = pImpulseCallback;
      sWatch.lParam    = lParam;

      auto itWatch = sVarWatchSet.find(sWatch);
      if(itWatch != sVarWatchSet.end())
      {
        // DataPoolWatch ����
        if((GXINT_PTR)itWatch->pCallback == 0) {
          DataPoolWatcher*& pWatch = *(DataPoolWatcher**)&itWatch->lParam;
          SAFE_RELEASE(pWatch);
        }
        sVarWatchSet.erase(itWatch);
      }

      if(sVarWatchSet.empty()) {
        m_FixedDict.erase(itWatchSet);
      }
    }
    return FALSE;
  }

  GXBOOL DataPool::Watch( GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, pImpulseCallback, lParam);
    }
    return FALSE;
  }

  GXBOOL DataPool::Watch( GXLPCSTR szExpression, DataPoolWatcher* pWatcher )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, NULL, (GXLPARAM)pWatcher);
    }
    return FALSE;
  }

  GXBOOL DataPool::Watch( GXLPCSTR szExpression, GXHWND hWnd )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntWatch(&var, (ImpulseProc)1, (GXLPARAM)hWnd);
    }
    return FALSE;
  }

  GXBOOL DataPool::Watch( DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam )
  {
    return IntWatch(pVar, pImpulseCallback, lParam);
  }

  GXBOOL DataPool::Watch( DataPoolVariable* pVar, DataPoolWatcher* pWatcher )
  {
    return IntWatch(pVar, NULL, (GXLPARAM)pWatcher);
  }

  GXBOOL DataPool::Watch( DataPoolVariable* pVar, GXHWND hWnd )
  {
    return IntWatch(pVar, (ImpulseProc)1, (GXLPARAM)hWnd);
  }

  GXBOOL DataPool::Ignore( GXLPCSTR szExpression, ImpulseProc pImpulseCallback )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, pImpulseCallback, NULL);
    }
    return FALSE;
  }

  GXBOOL DataPool::Ignore( GXLPCSTR szExpression, DataPoolWatcher* pWatcher )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, NULL, (GXLPARAM)pWatcher);
    }
    return FALSE;
  }

  GXBOOL DataPool::Ignore( GXLPCSTR szExpression, GXHWND hWnd )
  {
    DataPoolVariable var;
    if(QueryByExpression(szExpression, &var)) {
      return IntIgnore(&var, (ImpulseProc)1, (GXLPARAM)hWnd);
    }
    return FALSE;
  }

  GXBOOL DataPool::Ignore( DataPoolVariable* pVar, ImpulseProc pImpulseCallback )
  {
    return IntIgnore(pVar, pImpulseCallback, NULL);
  }

  GXBOOL DataPool::Ignore( DataPoolVariable* pVar, DataPoolWatcher* pWatcher )
  {
    return IntIgnore(pVar, NULL, (GXLPARAM)pWatcher);
  }

  GXBOOL DataPool::Ignore( DataPoolVariable* pVar, GXHWND hWnd )
  {
    return IntIgnore(pVar, (ImpulseProc)1, (GXLPARAM)hWnd);
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  DataPool::VARIABLE_DESC::VTBL* DataPool::VARIABLE_DESC::GetUnaryMethod() const
  {
    switch(GetTypeCategory())
    {
    case T_STRING:  return (VTBL*)Implement::s_pStringVtbl;
    case T_STRINGA: return (VTBL*)Implement::s_pStringAVtbl;
    case T_STRUCT:  return (VTBL*)Implement::s_pStructVtbl;
    case T_OBJECT:  return (VTBL*)Implement::s_pObjectVtbl;
    case T_ENUM:    return (VTBL*)Implement::s_pEnumVtbl;
    case T_FLAG:    return (VTBL*)Implement::s_pFlagVtbl;
    default:        return (VTBL*)Implement::s_pPrimaryVtbl;
    }
  }

  DataPool::VARIABLE_DESC::VTBL* DataPool::VARIABLE_DESC::GetMethod() const
  {
    if(IsDynamicArray()) {
      return (VTBL*)Implement::s_pDynamicArrayVtbl;
    }
    else if(nCount > 1) {
      return (VTBL*)Implement::s_pStaticArrayVtbl;
    }
    return GetUnaryMethod();
  }


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  bool DataPool::WATCH_FIXED::operator<( const WATCH_FIXED& t ) const
  {
    // ���д�ĺô꣡
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
      case 0: false;
      case 1: return lParam < t.lParam;
      default: return true;
      }
    // Callback
    default: return pCallback < t.pCallback;
    }
  }

} // namespace Marimo
