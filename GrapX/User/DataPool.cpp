#include "GrapX.H"
#include "GrapX.Hxx"

#include "clPathFile.h"
#include "clTextLines.h"
#include "Smart/smartstream.h"
#include "Smart/SmartStock.h"

#include "GrapX/DataPool.H"
#include "GrapX/DataPoolIterator.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"

#include "DataPoolErrorMsg.h"
#include "DataPoolImpl.h"
using namespace clstd;

// TODO:
// [C]1.无符号类型是：“unsigned_int” 要改为 “unsigned int”
// [C]2.IsFixed应该使用更简易的方法判断
// [C]3.暂不支持object类型
// [C]4.“SetAsXXXXXX”想一个更美观的名字
// 5.DataPoolVariable考虑重构为指针
// [F]6.动态数组要额外分配一个class ArrayBuffer : public clBuffer指针，要考虑改为实体，减少可能的内存碎片
// 7.考虑支持typedef关键字
// [C]8.动态数组去掉‘#’方式的记录
// 9.支持const方式
// [C]10.编译行号，编译时语法检查
// 11.获得全称如“var[139].left”
// [C]12.描述结构是否也能添加定址，和数据放在一个内存块里
// [C]13.遍历接口 iterator
// [C]14.消除 struct A{ A a; } 这种自引用问题
// [C]15.Remove需要增加一个接口可以删除若干成员，删除成员还有遍历，清理object，string和动态数组
// 16.动态数组目前只增不减
// 17.增加参考类型，相当于指针指向有效的变量
// [C]18.save时的指针重定位，功能与load重定位合并，封装为标准函数
// [C]19.64位加载问题
// [C]20.迭代器分为具名和匿名两种实现
// 21.clStringA 的支持
// 22.多编码支持

//#define GSIT_Variables (m_aGSIT)
//#define GSIT_Members   (m_aGSIT + m_nNumOfVar)
//#define GSIT_Enums     (m_aGSIT + m_nNumOfVar + m_nNumOfMember)
//
//#define V_WRITE(_FUNC, _TIPS) if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
//#define V_READ(_FUNC, _TIPS)  if( ! (_FUNC)) { CLOG_ERROR(_TIPS); return FALSE; }
//#define SIZEOF_PTR          sizeof(void*)
//#define SIZEOF_PTR32        sizeof(GXDWORD)
//#define SIZEOF_PTR64        sizeof(GXQWORD)
//#define TYPE_CHANGED_FLAG   0x80000000  // 类型扩展或者缩减时的标记，记在TYPE_DESC::Cate上，用后要清除!
//
//#ifdef _DEBUG
//# define INC_DBGNUMOFSTRING ++m_nDbgNumOfString
//# define INC_DBGNUMOFARRAY  ++m_nDbgNumOfArray
//#else
//# define INC_DBGNUMOFSTRING
//# define INC_DBGNUMOFARRAY
//#endif // 
//
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

namespace Marimo
{
  typedef DataPoolVariable              Variable;
  //typedef const DataPool::VARIABLE_DESC DPVDD;

  


//#ifdef ENABLE_OLD_DATA_ACTION
//#ifdef ENABLE_DATAPOOL_WATCHER
//  class DataPoolUIWatcher : public DataPoolWatcher
//  {
//  private:
//    typedef clvector<GXHWND> WndHandleArray;
//    WndHandleArray m_aHandles;
//  public:
//    GXHRESULT AddRef()
//    {
//      return gxInterlockedIncrement(&m_nRefCount);
//    }
//
//    GXHRESULT Release()
//    {
//      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
//      if(nRefCount == 0)
//      {
//        delete this;
//        return GX_OK;
//      }
//      return nRefCount;
//    }
//
//    clStringA DataPoolUIWatcher::GetClassName()
//    {
//      return STR_DATAPOOL_WATCHER_UI;
//    }
//
//    GXHRESULT DataPoolUIWatcher::RegisterPrivate(GXLPVOID pIndentify)
//    {
//      if( ! gxIsWindow((GXHWND)pIndentify)) {
//        return GX_FAIL;
//      }
//      WndHandleArray::iterator it = 
//        std::find(m_aHandles.begin(), m_aHandles.end(), pIndentify);
//
//      if(it == m_aHandles.end())
//      {
//        m_aHandles.push_back((GXHWND)pIndentify);
//      }
//      return GX_OK;
//    }
//
//    GXHRESULT DataPoolUIWatcher::UnregisterPrivate(GXLPVOID pIndentify)
//    {
//      WndHandleArray::iterator it = 
//        std::find(m_aHandles.begin(), m_aHandles.end(), pIndentify);
//
//      if(it != m_aHandles.end())
//      {
//        m_aHandles.erase(it);
//        return GX_OK;
//      }
//      return GX_FAIL;
//    }
//
//    GXHRESULT DataPoolUIWatcher::OnKnock(KNOCKACTION* pKnock)
//    {
//      for(WndHandleArray::iterator it = m_aHandles.begin();
//        it != m_aHandles.end(); ++it) {
//          gxSendMessage(*it, GXWM_IMPULSE, 0, (GXLPARAM)pKnock);
//      }
//      return GX_OK;
//    }
//  }; // class DataPoolUIMonitor
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
//#endif // #ifdef ENABLE_OLD_DATA_ACTION

  //////////////////////////////////////////////////////////////////////////

  class DefaultDataPoolInclude : public DataPoolInclude
  {
  public:
    GXHRESULT Open(IncludeType eIncludeType, GXLPCWSTR pFileName, GXLPVOID lpParentData, GXLPCVOID *ppData, GXUINT *pBytes)
    {
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
  //////////////////////////////////////////////////////////////////////////
  //
  // DataPoolImpl 未来可能会有不同功能多种实现，放在不同域名下
  // 这个模板中的调用方法如果放在DataPoolImpl中，那么所有引用DataPoolImpl头文件
  // 的代码都要包含SmartStream,SmartStockW,TextLinesW,DataPoolErrorMsg这几个头文件
  // 而这又不是必须的，所以本着精简头文件依赖的原则，把这部分提取成了模板。
  //
  template<class _TDataPoolImpl>
  class DataPoolObject : public _TDataPoolImpl
  {
  protected:
    typedef clstd::SmartStockW::Section Section;
    typedef clstd::TextLinesW clTextLinesW;

    enum ErrorCode
    {
      ERROR_CODE_CANT_OPEN_FILE             = 1002,
      ERROR_CODE_CANT_PARSE_DATA            = 6000,
      ERROR_CODE_STRUCT_NOT_EXIST_IN_STRUCT = 6001,
      ERROR_CODE_STRUCT_NOT_EXIST           = 6002,
      ERROR_CODE_OUTOF_STATIC_ARRAY         = 6003,
      ERROR_CODE_NOT_ARRAY                  = 6004,
      ERROR_CODE_NOT_STRUCT                 = 6005,
      ERROR_CODE_NOT_FOUND_VAR              = 6006,
    };

  public:
    struct IMPORT
    {
      typedef DataPoolErrorMsg<GXWCHAR> DataPoolErrorMsgW;
      clstd::SmartStockW ss;
      DataPoolErrorMsgW  ErrorMsg;
    };

  public:
    DataPoolObject(GXLPCSTR szName) : _TDataPoolImpl(szName) {}
    virtual ~DataPoolObject() {}

    virtual GXHRESULT ImportDataFromFileW (GXLPCWSTR szFilename)
    {
      CLOGW(L"Import data from \"%s\".\n", szFilename);
      IMPORT import;
      import.ErrorMsg.LoadErrorMessageW(L"dpcmsg.txt");
      if(import.ss.LoadW(szFilename)) {
        clsize length;
        const GXWCHAR* pText = import.ss.GetText(&length);
        //clstd::TextLinesW tl(pText, length);
        auto sectRoot = import.ss.Open(NULL);
        import.ErrorMsg.SetCurrentFilenameW(szFilename);
        import.ErrorMsg.GenerateCurLines(pText, length);
        import.ErrorMsg.SetMessageSign('I');
        //import.ErrorMsg.SetSilentMode(TRUE);  // Debug!!
        //import.szFilename = szFilename;
        //import.tl = &tl;

        IntImportSections(import, sectRoot, NULL);

        import.ss.CloseSection(sectRoot);
        return GX_OK;
      }
      else {
        import.ErrorMsg.WriteErrorW(TRUE, 0, ERROR_CODE_CANT_OPEN_FILE, szFilename);
      }
      return GX_FAIL;
    }

  protected:
    void IntImportSections(IMPORT& import, Section sectParent, MOVariable* varParent)
    {
      typedef clhash_map<clStringA, VAR_COUNT> VarDict;
      GXBOOL bval = TRUE;
      auto sect = import.ss.OpenChild(sectParent, NULL);
      MOVariable var;
      VarDict sVarDict;

      if(sect)
      {
        do {
          clStringA strVarName = (GXLPCWSTR)sect->SectionName();
          //TRACE("sect name(%d):%s\n", nDbg++, strVarName);

          auto itVar = sVarDict.find(strVarName);
          if(itVar != sVarDict.end()) {
            var = itVar->second.var;
          }
          else 
          {
            if(varParent) {
              var = varParent->MemberOf(strVarName);
              if( ! var.IsValid()) {
                //nLine = import.ErrorMsg.LineFromPtr(sect->itSectionName.marker);
                //CLOG_WARNINGW(L"%s(%d): %s下面不存在名为\"%s\"的结构体\n", import.ErrorMsg.GetCurrentFilenameW(), nLine, clStringW(varParent->GetName()), clStringW(strVarName));
                clStringW strParent = varParent->GetName();
                clStringW strVarNameW = (GXLPCSTR)strVarName;
                import.ErrorMsg.WriteErrorW(FALSE, sect->itSectionName.offset(), ERROR_CODE_STRUCT_NOT_EXIST_IN_STRUCT, (GXLPCWSTR)strParent, (GXLPCWSTR)strVarNameW);
                continue;
              }
            }
            else {
              bval = QueryByName(strVarName, &var);
              if( ! bval) {
                //nLine = import.ErrorMsg.LineFromPtr(sect->itSectionName.marker);
                //CLOG_WARNINGW(L"%s(%d): 不存在名为\"%s\"的结构体\n", import.ErrorMsg.GetCurrentFilenameW(), nLine, clStringW(strVarName));
                clStringW strVarNameW = (GXLPCSTR)strVarName;
                import.ErrorMsg.WriteErrorW(FALSE, sect->itSectionName.offset(), ERROR_CODE_STRUCT_NOT_EXIST, (GXLPCWSTR)strVarNameW);
                continue;
              }
            }

            VAR_COUNT vc;
            vc.var = var;
            vc.nCount = 0;
            itVar = sVarDict.insert(clmake_pair(strVarName, vc)).first;
          }

          GXDWORD dwCaps = var.GetCaps();

          // 动态数组追加数据
          // 静态数组检查导入数据是否超长
          // 一元变量检查是否已经导入过数据
          MOVariable varNew;
          if(TEST_FLAG(dwCaps, MOVariable::CAPS_DYNARRAY))
          {
            varNew = var.NewBack();
          }
          else if(var.GetLength() > 1)
          {
            if(itVar->second.nCount >= var.GetLength()) {
              //import.tl->PosFromPtr(sect->itSectionName.marker, &nLine, &nRow);
              //CLOG_WARNINGW(L"%s(%d): 静态数组\"%s\"导入数据已经超过了它的最大容量(%d).\n", import.szFilename, nLine, clStringW(strVarName), var.GetLength());
              clStringW strVarNameW = (GXLPCSTR)strVarName;
              import.ErrorMsg.WriteErrorW(FALSE, sect->itSectionName.offset(), ERROR_CODE_OUTOF_STATIC_ARRAY, (GXLPCWSTR)strVarNameW, var.GetLength());
              continue;
            }

            varNew = var.IndexOf(itVar->second.nCount);
          }
          else
          {
            if(itVar->second.nCount > 0) {
              //import.tl->PosFromPtr(sect->itSectionName.marker, &nLine, &nRow);
              //CLOG_WARNINGW(L"%s(%d): \"%s\"变量声明为数组才可以重复导入数据.\n", import.szFilename, nLine, clStringW(strVarName));
              clStringW strVarNameW = (GXLPCSTR)strVarName;
              import.ErrorMsg.WriteErrorW(FALSE, sect->itSectionName.offset(), ERROR_CODE_NOT_ARRAY, (GXLPCWSTR)strVarNameW);
              continue;
            }
            varNew = var;
          }

          // 结构体属性检查
          if(TEST_FLAG_NOT(varNew.GetCaps(), MOVariable::CAPS_STRUCT)) {
            clStringW strVarNameW = (GXLPCSTR)strVarName;
            import.ErrorMsg.WriteErrorW(FALSE, sect->itSectionName.offset(), ERROR_CODE_NOT_STRUCT, (GXLPCWSTR)strVarNameW);
            continue;
          }

          IntImportSections(import, sect, &varNew);
          itVar->second.nCount++;

        } while(sect->NextSection(NULL));
      }

      //if(varParent) {
      IntImportKeys(import, sectParent, varParent);
      //}

      import.ss.CloseSection(sect);

      //for(auto it = sVarDict.begin(); it != sVarDict.end(); ++it)
      //{
      //  CLOG("import %s[%d]\n", it->first, it->second.nCount);
      //}
    }

    void IntImportKeys(IMPORT& import, Section sect, MOVariable* var)
    {
      clstd::SmartStockW::PARAMETER param;
      clStringW strValue;
      clStringW strKey;
      if(sect->FirstKey(param))
      {
        do {
          param.KeyName(strKey);

          MOVariable varMember;
          if(var) {
            varMember = var->MemberOf(clStringA(strKey));
          }
          else {
            QueryByName(clStringA(strKey), &varMember);
          }

          if(varMember.IsValid())
          {
            param.ToString(strValue);
            if( ! varMember.ParseW(strValue, 0))
            {
              // variable 无法解析字符串
              import.ErrorMsg.WriteErrorW(FALSE, param.itKey.offset(), ERROR_CODE_CANT_PARSE_DATA, (GXLPCWSTR)strKey, (GXLPCWSTR)strValue);
            }
          }
          else
          {
            // 没找到对应的variable
            import.ErrorMsg.WriteErrorW(FALSE, param.itKey.offset(), ERROR_CODE_NOT_FOUND_VAR, (GXLPCWSTR)strKey, (GXLPCWSTR)param.ToString(strValue));
          }
        } while (param.NextKey());
      }
    }
  };

  //GXHRESULT DataPoolImportImpl::ImportDataFromFileW( GXLPCWSTR szFilename )
  //void DataPoolImportImpl::IntImportSections(
  //  IMPORT&     import,
  //  Section     sectParent,
  //  MOVariable* varParent)
  //void DataPoolImportImpl::IntImportKeys(
  //  IMPORT&     import,
  //  Section     sect,
  //  MOVariable* var)
  


  //class DataPoolObject : public DataPoolImpl, public DataPoolImportImpl
  //{
  //};


  //////////////////////////////////////////////////////////////////////////

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




#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
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
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE




#define IS_VALID_NAME(_NAME)  (_NAME != NULL && clstd::strlenT(_NAME) > 0)

  GXHRESULT DataPool::FindDataPool(DataPool** ppDataPool, GXLPCSTR szName)
  {
    if(IS_VALID_NAME(szName))
    {
      GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
      GXSTATION::NamedInterfaceDict::iterator it = lpStation->m_NamedPool.find(szName);

      // 有的话直接增加引用计数然后返回
      if(it != lpStation->m_NamedPool.end()) {
        *ppDataPool = static_cast<DataPool*>(it->second);
        return it->second->AddRef();
      }
    }
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
            // 出错也继续导入
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
        // TODO: 这个从文件加载要检查BOM，并转换为Unicode格式
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

  GXHRESULT DataPool::CreateFromFileW(DataPool** ppDataPool, GXLPCSTR szName, GXLPCWSTR szFilename, GXDWORD dwFlag)
  {
    ASSERT(szName == NULL);  // 暂时这个不支持命名方式
    GXHRESULT hval = GX_OK;

    DataPool* pDataPool = new DataPoolObject<DataPoolImpl>(szName);
    if( ! InlCheckNewAndIncReference(pDataPool)) {
      return GX_FAIL;
    }

    clFile file;

    if( ! file.OpenExistingW(szFilename)) {
      hval = GX_E_OPEN_FAILED;
    }
    else {
      if( ! pDataPool->Load(file, dwFlag)) {
        hval = GX_E_OPEN_FAILED;
      }
    }

    if(GXSUCCEEDED(hval)) {
      *ppDataPool = pDataPool;
    }
    return hval;
  }

  GXHRESULT DataPool::CreateDataPool(DataPool** ppDataPool, GXLPCSTR szName, const TYPE_DECLARATION* pTypeDecl, const VARIABLE_DECLARATION* pVarDecl)
  {
    GXLPSTATION lpStation = NULL;
    GXHRESULT hval = GX_OK;

    // 查找同名的 DataPool

    hval = FindDataPool(ppDataPool, szName);
    if(GXSUCCEEDED(hval)) {
      return hval;
    }
    else if(IS_VALID_NAME(szName)) {
      lpStation = GXSTATION_PTR(GXUIGetStation());
    }

    DataPoolObject<DataPoolImpl>* pDataPoolObj = new DataPoolObject<DataPoolImpl>(szName);
    if( ! InlCheckNewAndIncReference(pDataPoolObj)) {
      return GX_FAIL;
    }

    // 初始化
    if( ! pDataPoolObj->Initialize(pTypeDecl, pVarDecl)) {
      pDataPoolObj->Release();
      pDataPoolObj = NULL;
      hval = GX_FAIL;
    }
    else {
      hval = GX_OK;
    }

    // 注册
    if(lpStation != NULL && pDataPoolObj != NULL)
    {
      ASSERT(pDataPoolObj->m_Name.IsNotEmpty());
      lpStation->m_NamedPool[pDataPoolObj->m_Name] = pDataPoolObj;
    }

    *ppDataPool = pDataPoolObj;
    return hval;
  }

  //DataPool::LPCENUMDESC DataPool::IntGetEnum( GXUINT nPackIndex ) const
  //{
  //  // ************************************************************************
  //  // 这里最开始写成了返回enum desc的引用
  //  // 但是使用Name自定位后发现Release版返回值经过了优化，会导致Name自定位到无效地址
  //  // 为了保证指针稳定，改成了返回指针
  //  //
  //  return &m_aEnums[nPackIndex];
  //  //return m_aEnumPck[nPackIndex];
  //}







  Marimo::DataPoolVariable DataPool::operator[]( GXLPCSTR szExpression )
  {
    DataPoolVariable var;
    QueryByExpression(szExpression, &var);
    return var;
  }


  //////////////////////////////////////////////////////////////////////////



  //////////////////////////////////////////////////////////////////////////




  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////




} // namespace Marimo
