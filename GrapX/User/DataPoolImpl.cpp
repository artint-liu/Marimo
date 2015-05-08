#include "GrapX.H"
#include "GrapX.Hxx"
#include "clStringSet.h"
#include "clTextLines.h"
#include "Smart/smartstream.h"
#include "Smart/SmartStock.h"

//#include "GrapX/GUnknown.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/GXKernel.H"

#include "DataPoolVariableVtbl.h"
#include "DataPoolImpl.h"
using namespace clstd;

#define GSIT_Variables (m_aGVSIT)
#define GSIT_Members   (m_aGVSIT + m_nNumOfVar)
#define GSIT_Enums     (m_aGVSIT + m_nNumOfVar + m_nNumOfMember)
#define IS_VALID_NAME(_NAME)  (_NAME != NULL && clstd::strlenT(_NAME) > 0)

#define ERROR_CODE_CANT_OPEN_FILE             1002
#define ERROR_CODE_CANT_PARSE_DATA            6000
#define ERROR_CODE_STRUCT_NOT_EXIST_IN_STRUCT 6001
#define ERROR_CODE_STRUCT_NOT_EXIST           6002
#define ERROR_CODE_OUTOF_STATIC_ARRAY         6003
#define ERROR_CODE_NOT_ARRAY                  6004
#define ERROR_CODE_NOT_STRUCT                 6005
#define ERROR_CODE_NOT_FOUND_VAR              6006

namespace Marimo
{
  class DataPoolImpl : public DataPool
  {
  protected:
    typedef SmartStockW::Section Section;
    typedef clstd::TextLinesW clTextLinesW;

    struct IMPORT
    {
      //GXLPCWSTR         szFilename;
      //clTextLinesW*     tl;
      typedef DataPoolErrorMsg<GXWCHAR> DataPoolErrorMsgW;
      SmartStockW       ss;
      DataPoolErrorMsgW ErrorMsg;
    };

    struct VAR_COUNT // 导入数据时查找variable并标记使用的结构体
    {
      MOVariable var;
      GXUINT     nCount;
    };

  public:
    DataPoolImpl(GXLPCSTR szName) : DataPool(szName) {}
    GXHRESULT ImportDataFromFileW( GXLPCWSTR szFilename);
  protected:
    void IntImportSections(IMPORT& import, Section sectParent, MOVariable* varParent);
    void IntImportKeys    (IMPORT& import, Section sect, MOVariable* var);
  };


  GXHRESULT DataPool::CreateDataPool(DataPool** ppDataPool, GXLPCSTR szName, const TYPE_DECLARATION* pTypeDecl, const VARIABLE_DECLARATION* pVarDecl)
  {
#ifdef DATAPOOLCOMPILER_PROJECT
    GXHRESULT hval = GX_OK;

    // 查找同名的 DataPool

    hval = FindDataPool(ppDataPool, szName);
    if(GXSUCCEEDED(hval)) {
      return hval;
    }

    DataPoolImpl* pDataPoolImpl = new DataPoolImpl(szName);
    if( ! InlCheckNewAndIncReference(pDataPoolImpl)) {
      return GX_FAIL;
    }

    // 初始化
    if( ! pDataPoolImpl->Initialize(pTypeDecl, pVarDecl)) {
      pDataPoolImpl->Release();
      pDataPoolImpl = NULL;
      hval = GX_FAIL;
    }
    else {
      hval = GX_OK;
    }

    *ppDataPool = pDataPoolImpl;
    return hval;
#else
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

    DataPoolImpl* pDataPoolImpl = new DataPoolImpl(szName);
    if( ! InlCheckNewAndIncReference(pDataPoolImpl)) {
      return GX_FAIL;
    }

    // 初始化
    if( ! pDataPoolImpl->Initialize(pTypeDecl, pVarDecl)) {
      pDataPoolImpl->Release();
      pDataPoolImpl = NULL;
      hval = GX_FAIL;
    }
    else {
      hval = GX_OK;
    }

    // 注册
    if(lpStation != NULL && pDataPoolImpl != NULL)
    {
      ASSERT(pDataPoolImpl->m_Name.IsNotEmpty());
      lpStation->m_NamedPool[pDataPoolImpl->m_Name] = pDataPoolImpl;
    }

    *ppDataPool = pDataPoolImpl;
    return hval;
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
  }

  GXHRESULT DataPool::CreateFromFileW(DataPool** ppDataPool, GXLPCSTR szName, GXLPCWSTR szFilename, GXDWORD dwFlag)
  {
    ASSERT(szName == NULL);  // 暂时这个不支持命名方式
    GXHRESULT hval = GX_OK;

    DataPool* pDataPool = new DataPoolImpl(szName);
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


  GXHRESULT DataPoolImpl::ImportDataFromFileW( GXLPCWSTR szFilename )
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

  void DataPoolImpl::IntImportSections(
    IMPORT&     import,
    Section     sectParent,
    MOVariable* varParent)
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
          itVar = sVarDict.insert(std::make_pair(strVarName, vc)).first;
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

  void DataPoolImpl::IntImportKeys(
    IMPORT&     import,
    Section     sect,
    MOVariable* var)
  {
    SmartStockW::PARAMETER param;
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

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

#define STD_ERROR_HEAD_EXT    "%s(%d): error(%c%d): "
#define STD_WARNING_HEAD_EXT  "%s(%d): warning(%c%d): "

GXWCHAR STD_ERROR_HEAD_EXTW[]  = {'%','s','(','%','d',')',':',' ','e','r','r','o','r','(','%','c','%','d',')',':',' ','\0'};
GXWCHAR STD_WARNING_HEAD_EXTW[]= {'%','s','(','%','d',')',':',' ','w','a','r','n','i','n','g','(','%','c','%','d',')',':',' ','\0'};
GXWCHAR STD_ERROR_HEADW[]  = {'e','r','r','o','r','(','%','c','%','d',')',':',' ','\0'};
GXWCHAR STD_WARNING_HEADW[]= {'w','a','r','n','i','n','g','(','%','c','%','d',')',':',' ','\0'};

#define _DPEM_TEMPL  template<typename _TChar>
#define _DPEM_CLS    DataPoolErrorMsg<_TChar>

  _DPEM_TEMPL
  _DPEM_CLS::DataPoolErrorMsg()
    : m_cSign('I')
    , m_CompileCode(2)
    , m_bSilent(FALSE)
  {
    FILE_SECTION* fs = new FILE_SECTION;
    fs->nBaseLine = 0;
    m_Sources.push_back(fs);
    m_SourcesTable.push_back(NULL); // 避免使用0这个Id
    m_SourcesTable.push_back(fs);
  }

  _DPEM_TEMPL
  _DPEM_CLS::~DataPoolErrorMsg()
  {
    //std::for_each(m_SourcesTable.begin(), m_SourcesTable.end(), [](FILE_SECTION*& pfs) {
    //  SAFE_DELETE(pfs);
    //});
    for(auto it = m_SourcesTable.begin(); it != m_SourcesTable.end(); ++it) {
      //, [](FILE_SECTION*& pfs) {
      SAFE_DELETE(*it);
    }
  }


  _DPEM_TEMPL
  void _DPEM_CLS::SetMessageSign(GXWCHAR cSign)
  {
    m_cSign = cSign;
  }

  _DPEM_TEMPL
  GXBOOL _DPEM_CLS::LoadErrorMessageW(GXLPCWSTR szErrorFile)
  {
    SmartStockW ss;
    if(ss.LoadW(szErrorFile))
    {
      SmartStockW::Section sectRoot = ss.Open(NULL);
      if(sectRoot)
      {
        SmartStockW::PARAMETER param;
        if(sectRoot->FirstKey(param))
        {
          do {
            m_ErrorMsg[clstd::xtou(param.KeyName())] = param.ToString();
          } while (param.NextKey());
        }
        ss.CloseSection(sectRoot);
      }
    }
    return FALSE;
  }

  //_DPEM_TEMPL
  //void _DPEM_CLS::WriteErrorW( GXBOOL bError, T_LPCSTR pSourcePtr, GXLPCWSTR message, ... )
  //{
  //  UpdateResult(bError);
  //  clStringW str;
  //  if(pSourcePtr) {
  //    str.Format(bError ? STD_ERROR_HEADW : STD_WARNING_HEADW, m_Sources.back()->strFilename, LineFromPtr(pSourcePtr), m_cSign, 0);
  //  }

  //  va_list  arglist;
  //  va_start(arglist, message);

  //  str.VarFormat(message, arglist);
  //  str.Append(L"\r\n");
  //  if(bError) {
  //    CLOG_ERRORW(str);
  //  }
  //  else {
  //    CLOG_WARNINGW(str);
  //  }
  //}

  _DPEM_TEMPL
  void _DPEM_CLS::WriteErrorW(GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, ...)
  {
    UpdateResult(bError);
    clStringW str;
    if(nOffset != (GXSIZE_T)-1) {
      if(nOffset) {
        str.Format(bError ? STD_ERROR_HEAD_EXTW : STD_WARNING_HEAD_EXTW, (GXLPCWSTR)m_Sources.back()->strFilename, LineFromOffset(nOffset), m_cSign, nCode);
      }
      else {
        str.Format(bError ? STD_ERROR_HEADW : STD_WARNING_HEADW, m_cSign, nCode);
      }
    }

    auto it = m_ErrorMsg.find(nCode);
    if(it == m_ErrorMsg.end()) {
      str.AppendFormat(L"Missing Compiler error message.\r\n");
    }
    else {

      try
      {
        va_list  arglist;
        va_start(arglist, nCode);

        str.VarFormat(it->second, arglist);
        str.Append(L"\r\n");
      }
      catch(...)
      {
        str.Append(L"\r\n");
      }
    }

    if(bError) {
      CLOG_ERRORW(str);
    }
    else if( ! m_bSilent){
      CLOG_WARNINGW(str);
    }
  }

  _DPEM_TEMPL
  void _DPEM_CLS::WriteErrorA( GXBOOL bError, T_LPCSTR pSourcePtr, GXLPCSTR message, ... )
  {
    UpdateResult(bError);
    clStringA str;
    if(pSourcePtr) {
      str.Format(bError ? STD_ERROR_HEAD_EXT : STD_WARNING_HEAD_EXT, (GXLPCSTR)clStringA(m_Sources.back()->strFilename), LineFromPtr(pSourcePtr), m_cSign, 0);
    }

    va_list  arglist;
    va_start(arglist, message);

    str.VarFormat(message, arglist);
    str.Append("\r\n");
    if(bError) {
      CLOG_ERROR(str);
    }
    else {
      CLOG_WARNING(str);
    }
  }

  _DPEM_TEMPL
  GXHRESULT _DPEM_CLS::UpdateResult( GXBOOL bError )
  {
    // 更新状态
    // 如果已经在出错状态，不更新
    // 警告状态，会降级到错误状态
    // 正常状态，会降级到警告或者错误状态
    int nNewCode = bError ? 0 : 1;
    m_CompileCode = clMin(nNewCode, m_CompileCode);
    return m_CompileCode;
  }

  _DPEM_TEMPL
  int _DPEM_CLS::LineFromPtr(T_LPCSTR ptr) const
  {
    int nLine, nRow;
    FILE_SECTION* pfs;
    pfs = m_Sources.back();
    pfs->tl.PosFromPtr(ptr, &nLine, &nRow);
    return pfs->nBaseLine + nLine;
  }

  _DPEM_TEMPL
  int _DPEM_CLS::LineFromOffset(GXSIZE_T nOffset, GXUINT idFile) const
  {
    int nLine, nRow;
    FILE_SECTION* pfs;
    if(idFile == 0) {
      pfs = m_Sources.back();
    }
    else if(idFile < m_SourcesTable.size()) {
      pfs = m_SourcesTable[idFile];
    }
    else {
      return 0;
    }
    pfs->tl.PosFromOffset(nOffset, &nLine, &nRow);
    return pfs->nBaseLine + nLine;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::SetCurrentFilenameW( GXLPCWSTR szFilename )
  {
    m_Sources.back()->strFilename = szFilename;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::SetCurrentTopLine( GXINT nTopLine )
  {
    m_Sources.back()->nBaseLine = nTopLine;
  }

  _DPEM_TEMPL
  GXLPCWSTR _DPEM_CLS::GetFilenameW(GXUINT idFile) const
  {
    if(idFile == 0) {
      return m_Sources.back()->strFilename;
    }
    else if(idFile < m_SourcesTable.size()) {
      return m_SourcesTable[idFile]->strFilename;
    }
    return L"";
  }

  _DPEM_TEMPL
  GXUINT _DPEM_CLS::GetCurrentFileId() const
  {
    return (GXUINT)m_SourcesTable.size() - 1;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::PushFile( GXLPCWSTR szFilename, GXINT nTopLine /*= 0*/ )
  {
    FILE_SECTION* fs = new FILE_SECTION;
    fs->strFilename = szFilename;
    fs->nBaseLine = nTopLine;
    m_Sources.push_back(fs);
    m_SourcesTable.push_back(fs);
  }

  _DPEM_TEMPL
  void _DPEM_CLS::PopFile()
  {
    m_Sources.pop_back();
  }

  _DPEM_TEMPL
  int _DPEM_CLS::GetErrorLevel() const
  {
    return m_CompileCode;
  }

  _DPEM_TEMPL
  GXSIZE_T _DPEM_CLS::GenerateCurLines( T_LPCSTR pText, clsize length )
  {
    return m_Sources.back()->tl.Generate(pText, length);
  }

  _DPEM_TEMPL
  GXBOOL _DPEM_CLS::CheckIncludeLoop( GXLPCWSTR szFilename) const
  {
    for(auto it = m_Sources.rbegin(); it != m_Sources.rend(); ++it)
    {
      if((*it)->strFilename == szFilename) {
        return FALSE;
      }
    }
    return TRUE;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::SetSilentMode( GXBOOL bSilent )
  {
    m_bSilent = bSilent;
  }

  template class DataPoolErrorMsg<GXWCHAR>;
  template class DataPoolErrorMsg<GXCHAR>;

} // namespace Marimo
