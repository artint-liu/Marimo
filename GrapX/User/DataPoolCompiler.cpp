#include "GrapX.H"
#include "GrapX.Hxx"

#include "clPathFile.h"
#include "clTextLines.h"
#include "clMathExpressionParser.h"
#include "Smart/smartstream.h"

#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/GXKernel.H"

//#include <exception>

//#include "DataPoolVariableVtbl.h"
#include "DataPoolErrorMsg.h"
#include "DataPoolImpl.h"
using namespace clstd;

#define E_1000_NOT_FOUND_SEMICOLON          1000
#define E_1001_SYNTAX_ERROR                 1001
#define E_1002_CANT_OPEN_FILE               1002
#define E_1003_COMPILE_FAILED               1003
#define E_1010__DEFINE_DIFFERENT_TYPE       1010
#define E_1011_TYPE_NOT_DEFINED             1011
#define E_1014_ARRAY_NEED_TMPL_ARG          1014
#define E_1015_BAD_OF_ARRAY_TMPL_AT_END     1015
#define E_1016_ILLEGAL_NUM_OF_ARRAY         1016
#define E_1017_NEED_END_OF_SQUARE_BRACKETS  1017

#define E_1020_NEED_BEGIN_OF_BRACE          1020
#define E_1021_NEED_END_OF_BRACE            1021

#define E_1050_ILLEGAL_END                  1050

#define E_1100_ALWAYS_MODIFIES_STRUCT_ONLY  1100
#define E_1101_CANT_USE_AS_STRUCT_NAME      1101
#define E_1102_STRUCT_ALREADY_DEFINED       1102

#define E_1200_CANT_USE_AS_VAR_NAME         1200
#define E_1201_INVALID_TYPE                 1201
#define E_1202_ALREADY_DEFINED              1202

#define E_1400_CANT_DECL_AS_ENUM_TYPE       1400
#define E_1401_CANT_USE_AS_ENUM_NAME        1401
#define E_1402_ENUM_ALREADY_DEFINED         1402
#define E_1403_CANT_PARSE_EXPRESSION        1403

#define E_1500_LOOP_INCLUDE                 1500


#define E_1900_SHARP_SIGN_MUST_BE_FIRST     1900
#define E_1901_BAD_SHARP_PRAGMA_FMT         1901
#define E_1902_UNKNOWN_PROCESS_PRAGMA_CMD   1902
#define E_1903_UNKNOWN_PROCESS_CMD          1903
#define E_1904_PROCESS_NEED_NEWLINE_AT_END  1904

#define LOG_NOT_FOUND_SEMICOLON(_IT)  m_ErrorMsg.WriteErrorW(TRUE, _IT, E_1000_NOT_FOUND_SEMICOLON)
#define LOG_SYNTAX_ERROR(_IT, _SYM)   m_ErrorMsg.WriteErrorW(TRUE, _IT, E_1001_SYNTAX_ERROR, _SYM)
namespace Marimo
{
  namespace Implement
  {
    extern TYPE_DECLARATION c_InternalTypeDefine[];
  } // namespace Implement

  typedef SmartStreamA::iterator StreamIter;

  class DataPoolResolverImpl : public DataPoolCompiler
  {
    friend class DataPoolCompiler;
    friend u32 CALLBACK TriggerProc(StreamIter& it, clsize uRemain, u32_ptr lParam);

    enum StringType
    {
      ST_VarType    = 0x00010000,
      ST_StructType = 0x00020000,
      ST_EnumType   = 0x00040000,
      ST_AnyType    = ST_VarType | ST_StructType | ST_EnumType,

      ST_VarName    = 1,
      ST_EnumName   = 2,
    };

    struct DEFINE
    {
      StringType type;
      GXUINT     SourceFileId;
      GXSIZE_T   SourceOffset;
    };

    typedef clvector<VARIABLE_DECLARATION>        VarDeclArray;
    typedef clvector<ENUM_DECLARATION>            EnumPairArray;
    typedef clvector<VarDeclArray*>               MembersArray;
    typedef clvector<EnumPairArray*>              EnumDeclArray;
    typedef clvector<TYPE_DECLARATION>            TypeDeclArray;
    typedef clhash_map<clStringA, StringType>     StringDict;
    typedef clhash_map<clStringA, DEFINE>         NameDict;
    typedef clhash_map<clStringA, DataPool::Enum> EnumDict;
    typedef DataPoolErrorMsg<GXCHAR>              DataPoolErrorMsgA;
    //typedef clset<clStringA>                  TypeDeclArray;

  private:
    //clStringA             m_strFilename;
    //int                   m_nBaseLine;
    //StringDict            m_StringsDict;
    NameDict              m_NameDict;
    EnumDict              m_EnumDict;
    TypeDeclArray         m_aTypes;
    MembersArray          m_aStructMember;
    EnumDeclArray         m_aEnums;
    VarDeclArray          m_aVariables;         // 不在结构中的变量    
    DataPoolInclude*      m_pInclude;
    clStringListW         m_ImportFiles;
    GXBOOL                m_bParsingExpression; // TriggerProc部分功能在表达式解析时要跳过
    clStringHashSetA      m_VarNameSet;         // 全局变量集合，用来检查重名
    DataPoolErrorMsgA     m_ErrorMsg;
    // 自动设置表达式解析状态的类
    class EPSection
    {
      DataPoolResolverImpl* m_pCompiler;
    public:
      EPSection(DataPoolResolverImpl* pCompiler) : m_pCompiler(pCompiler){ m_pCompiler->m_bParsingExpression = TRUE; }
      ~EPSection() { m_pCompiler->m_bParsingExpression = FALSE; }
    };

  protected:
    TypeCategory  GetVarCateByName  (GXLPCSTR szTypeName);
    GXLPCSTR      AddString         (StringType eType, clStringHashSetA* pVarNameSet, GXLPCSTR szString, GXSIZE_T nSourceOffset, GXBOOL* result);
    GXLPCSTR      CheckType         (GXLPCSTR szTypeName, GXSIZE_T nSourceOffset, GXBOOL* result);
    GXBOOL        ParseStruct       (SmartStreamA& ss, StreamIter& it, GXBOOL bAlways);
    GXBOOL        ParseEnum         (SmartStreamA& ss, StreamIter& it, GXBOOL bFlag);
    GXBOOL        ParseEnumItem     (SmartStreamA& ss, StreamIter& it, const StreamIter&, ENUM_DECLARATION& sEnum);
    GXBOOL        ParseVariable     (SmartStreamA& ss, StreamIter& it, clStringHashSetA& sNameSet, VARIABLE_DECLARATION* pVarDecl);
    GXBOOL        ParseAssignment   (SmartStreamA& ss, StreamIter& it, VARIABLE_DECLARATION* pVarDecl);
    u64           ParseInitValue    (const clStringA& strValue) const;
    GXBOOL        FindValueFromEnum (const StreamIter& it, GXINT* nValue);
    GXBOOL        MainCompile       (DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength);
    GXBOOL        IntCompile        (DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength);
    GXHRESULT     GetCompileResult  ();
    GXBOOL        CheckIncludeLoop  (GXLPCWSTR szFilename, GXSIZE_T nSourceOffset);
    

    GXINT         MathExpressionParser(const cllist<StreamIter>& expression, GXINT* nValue);

    template<typename _Ty>
    void      AllocInitPtr    (VARIABLE_DECLARATION* pVarDecl, GXINT aInitArrayCount);
    template<typename _Ty>
    GXBOOL    ParseInitList   (_Ty* aArray, const clStringArrayA& aInit);
    DataPoolResolverImpl();
    virtual   ~DataPoolResolverImpl();
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT AddRef  ();
    GXHRESULT Release ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT GetManifest(MANIFEST* pManifest) const;
  };

  //////////////////////////////////////////////////////////////////////////

  DataPoolResolverImpl::DataPoolResolverImpl()
    : m_pInclude(NULL)
    , m_bParsingExpression(FALSE)
  {
  }

  GXHRESULT DataPoolResolverImpl::GetManifest(MANIFEST* pManifest) const
  {
    if(m_aTypes.empty() || m_aVariables.empty()) {
      return GX_FAIL;
    }
    pManifest->pTypes = &m_aTypes.front();
    pManifest->pVariables = &m_aVariables.front();
    pManifest->pImportFiles = &m_ImportFiles;
    return GX_OK;
  }

  //template<class SmartStreamT, typename _Ty>
  //GXBOOL IsHeadOfLine(SmartStreamT* pStream, _Ty* pCurrent)
  //{
  //  auto* ptr = pStream->GetStreamPtr();
  //  auto* p = pCurrent - 1;
  //  while(p >= ptr) {
  //    if(*p == '\n') {
  //      return TRUE;
  //    }
  //    else if(TEST_FLAG_NOT(pStream->GetCharSemantic(*p), SmartStreamT::M_GAP)) {
  //      return FALSE;
  //    }
  //    --p;
  //  }
  //  return TRUE; // 到文档开头了
  //}

  template<class _Iter>
  GXBOOL IsDifferentLines(const _Iter& first, const _Iter& second) {
    ASSERT(first.pContainer == second.pContainer);
    if(second == second.pContainer->end()) {
      return TRUE;
    }
    auto* p = first.marker;
    while(p < second.marker) {
      if(*p == '\n') {
        return TRUE;
      }
      p++;
    }
    return FALSE;
  }

  GXBOOL IsNumeric(const StreamIter& it, GXINT* pInteger)
  {
    return IsNumericT(it.marker, it.length, pInteger);
  }

  u32 CALLBACK TriggerProc(StreamIter& it, clsize uRemain, u32_ptr lParam)
  {
    u32 i = 2; // "//" 和 "/*"开始都是两个字节

    if(it.marker[0] == '\n') {
      ++it;
      return 0 ;
    }
    else if(it.marker[0] == '/' && it.marker[1] == '/') // 处理单行注释“//...”
    {
      SmartStreamUtility::ExtendToNewLine(it, 2, uRemain);
      ++it;
      return 0;
    }
    else if(it.marker[0] == '/' && it.marker[1] == '*') // 处理块注释“/*...*/”
    {
      //--uRemain;
      //auto c0 = it.marker[i];
      //while(i < uRemain)
      //{
      //  auto c1 = it.marker[i + 1];
      //  if(c0 == '*' && c1 == '/') {
      //    it.length = i + 2;
      //    ++it;
      //    return 0;
      //  }
      //  c0 = c1;
      //  ++i;
      //}
      SmartStreamUtility::ExtendToCStyleBlockComment(it, 2, uRemain);
      ++it;
      return 0;
    }
    else if(it.marker[0] == '#') {
      auto pThis = (DataPoolResolverImpl*)lParam;

      // 表达式解析时不会处理'#'，使错误信息更准确
      if(pThis->m_bParsingExpression) {
        return 0;
      }
      if( ! SmartStreamUtility::IsHeadOfLine(it.pContainer, it.marker)) {
        pThis->m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1900_SHARP_SIGN_MUST_BE_FIRST);
        SmartStreamUtility::ExtendToNewLine(it, 1, uRemain);
        ++it;
        return 0;
      }
      ASSERT(it.length == 1);
      ++it;

      static const GXCHAR szFileDefine[] = "FILE";
      static const GXCHAR szLineDefine[] = "LINE";
      static const GXCHAR szInclude[] = "include";
      static const GXCHAR szPragma[] = "pragma";
      static const GXCHAR szPragma_import[] = "import";

      // 处理文件名设定标志
      if(it.BeginsWith(szFileDefine)) { // # FILE
        it.length = GXSTRLEN(szFileDefine);
        ++it;
        SmartStreamUtility::ExtendToNewLine(it, 0, uRemain);
        clStringW str = (GXLPCSTR)it.ToString();

        // 去掉设定标记，换行，空格
        str.TrimLeft(0x20);
        str.TrimRight(L" \n");
        pThis->m_ErrorMsg.SetCurrentFilenameW(str);
      }
      else if(it.BeginsWith(szLineDefine)) { // # LINE
        it.length = GXSTRLEN(szFileDefine);
        ++it;
        SmartStreamUtility::ExtendToNewLine(it, 0, uRemain);
        auto  str = it.ToString();
        //auto* pThis = ((DataPoolResolverImpl*)lParam);
        //str.Remove(0, (sizeof(szFileDefine) / sizeof(szFileDefine[0]) - 1));
        str.TrimLeft(0x20);
        int nCurLine = pThis->m_ErrorMsg.LineFromPtr(it.marker);
        pThis->m_ErrorMsg.SetCurrentTopLine(str.ToInteger() - nCurLine - 1);
      }
      else if(it == szInclude) { // # include
        ++it;

        //DataPoolResolverImpl* pResolver = (DataPoolResolverImpl*)lParam;
        GXLPCVOID pData = NULL;
        GXUINT nBytes = 0;

        // 根据当前文件路径计算include文件路径
        clStringW str = pThis->m_ErrorMsg.GetFilenameW();
        clpathfile::RemoveFileSpecW(str);
        clpathfile::CombinePathW(str, str, clStringW(it.ToString()));

        if(pThis->CheckIncludeLoop(str, it.offset()) && GXSUCCEEDED(pThis->m_pInclude->Open(
          DataPoolInclude::IncludeType_Local, str, NULL, &pData, &nBytes)))
        {
            pThis->m_ErrorMsg.PushFile(str);

            if( ! pThis->IntCompile(NULL, (GXLPCSTR)pData, (GXSIZE_T)nBytes)) {
              pThis->m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1003_COMPILE_FAILED, (GXLPCWSTR)str);
            }

            pThis->m_ErrorMsg.PopFile();
        }
        else {
          pThis->m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1002_CANT_OPEN_FILE, (GXLPCWSTR)str);
        }
        pThis->m_pInclude->Close(pData);
      }
      else if(it.BeginsWith(szPragma)) { // # pragma
        ++it;
        if(it == szPragma_import)
        {
          StreamIter itImport = it;
          ++it;
          clvector<StreamIter> argv;
          argv.reserve(3);
          SmartStreamUtility::Get(it, it.pContainer->end(), ")", 3, argv);

          if(argv.size() == 3 && argv[0] != "(" || argv[2] != ")") {
            pThis->m_ErrorMsg.WriteErrorW(FALSE, it.offset(), E_1901_BAD_SHARP_PRAGMA_FMT);
          }
          else {
            // 导入数据文件
            
            clStringW str = pThis->m_ErrorMsg.GetFilenameW();
            clpathfile::RemoveFileSpecW(str);
            clpathfile::CombinePathW(str, str, clStringW(argv[1].ToString()));

            pThis->m_ImportFiles.push_back(str);
            //TRACEW(L"Import file: %s\n", pThis->m_ImportFiles.back());
          }

          it = itImport; // 还原it用来重定位行尾
        }
        else
        {
          clStringW strPragma = szPragma;
          clStringW strIter = (GXLPCSTR)it.ToString();
          pThis->m_ErrorMsg.WriteErrorW(FALSE, it.offset(), E_1902_UNKNOWN_PROCESS_PRAGMA_CMD, (GXLPCWSTR)strPragma, (GXLPCWSTR)strIter);
        }
        SmartStreamUtility::ExtendToNewLine(it, it.length, uRemain);
      }
      else {
        clStringW strIter = (GXLPCSTR)it.ToString();
        pThis->m_ErrorMsg.WriteErrorW(FALSE, it.offset(), E_1903_UNKNOWN_PROCESS_CMD, (GXLPCWSTR)strIter);
        SmartStreamUtility::ExtendToNewLine(it, it.length, uRemain);
      }
      auto itPrev = it;
      ++it;

      if( ! IsDifferentLines(itPrev, it)) {
        //CLOG_ERROR("预处理命令后应直接带有换行标记.\n");
        pThis->m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1904_PROCESS_NEED_NEWLINE_AT_END);
      }
    }
    else {
      CLBREAK; // 只处理 “//” 和 “/*...*/” 两种注释
    }
    return 0;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
#ifdef DATAPOOLCOMPILER_PROJECT
  GXHRESULT DataPoolResolverImpl::AddRef()
  {
    return ++m_nRefCount;
  }

  GXHRESULT DataPoolResolverImpl::Release()
  {
    const GXLONG nRefCount = --m_nRefCount;
    if(nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#else
  GXHRESULT DataPoolResolverImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT DataPoolResolverImpl::Release()
  {
    const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  DataPoolResolverImpl::~DataPoolResolverImpl()
  {
    for(auto it = m_aVariables.begin();
      it != m_aVariables.end(); ++it) {
        free(it->Init);
    }
    m_aVariables.clear();

    for(auto it = m_aStructMember.begin();
      it != m_aStructMember.end(); ++it) {
        SAFE_DELETE(*it);
    }
    m_aStructMember.clear();

    for(auto it = m_aEnums.begin(); it != m_aEnums.end(); ++it)
    {
      SAFE_DELETE(*it);
    }
    m_aEnums.clear();
  }

  TypeCategory DataPoolResolverImpl::GetVarCateByName(GXLPCSTR szTypeName)
  {
    for(int i = 0; Implement::c_InternalTypeDefine[i].Cate != T_UNDEFINE; i++)
    {
      if(GXSTRCMP(Implement::c_InternalTypeDefine[i].Name, szTypeName) == 0) {
        return Implement::c_InternalTypeDefine[i].Cate;
      }
    }

    for(TypeDeclArray::iterator it = m_aTypes.begin();
      it != m_aTypes.end(); ++it) {
        if(GXSTRCMP(it->Name, szTypeName) == 0) {
          return it->Cate;
        }
    }
    return T_UNDEFINE;
  }

  GXLPCSTR DataPoolResolverImpl::AddString(StringType eType, clStringHashSetA* pVarNameSet, GXLPCSTR szString, GXSIZE_T nSourceOffset, GXBOOL* result)
  {
    auto it = m_NameDict.find(szString);

    if(it != m_NameDict.end()) {
      DEFINE& d = it->second;
      GXUINT nErrorCode = 0;
      if(d.type != eType) {
        // 被定义为不同的类型
        nErrorCode = E_1010__DEFINE_DIFFERENT_TYPE;
      }
      else if(eType == ST_StructType) {
        nErrorCode = E_1102_STRUCT_ALREADY_DEFINED;
      }
      else if(eType == ST_VarName)
      {
        ASSERT(pVarNameSet != NULL); // 这个一定要传入变量集合

        // 全局变量/成员变量重名检查
        if(pVarNameSet->find(szString) != pVarNameSet->end()) {
          nErrorCode = E_1202_ALREADY_DEFINED;
        }
        else {
          pVarNameSet->insert(szString);
        }
      }
      else if(eType == ST_EnumType && eType == ST_EnumName) {
        nErrorCode = E_1402_ENUM_ALREADY_DEFINED;
      }

      if(nErrorCode != 0) {
        clStringW strString = szString;
        m_ErrorMsg.WriteErrorW(TRUE, nSourceOffset, nErrorCode, (GXLPCWSTR)strString,
          m_ErrorMsg.GetFilenameW(d.SourceFileId), 
          m_ErrorMsg.LineFromOffset(d.SourceOffset, d.SourceFileId));
        *result = FALSE;
      }
    }
    else {
      DEFINE def;
      def.type         = eType;
      def.SourceFileId = m_ErrorMsg.GetCurrentFileId();
      def.SourceOffset = nSourceOffset;
      m_NameDict[szString] = def;
      it = m_NameDict.insert(clmake_pair(szString, def)).first;

      if(eType == ST_VarName) {
        pVarNameSet->insert(szString);
      }
    }
    return it->first;
  }

  GXLPCSTR DataPoolResolverImpl::CheckType(GXLPCSTR szTypeName, GXSIZE_T nSourceOffset, GXBOOL* result)
  {
    auto it = m_NameDict.find(szTypeName);

    if(it == m_NameDict.end()) {
      clStringW strTypename = szTypeName;
      m_ErrorMsg.WriteErrorW(TRUE, nSourceOffset, E_1011_TYPE_NOT_DEFINED, (GXLPCWSTR)strTypename);
      *result = FALSE;
      return NULL;
    }
    else if( ! (it->second.type & ST_AnyType)) {
      // "%s" 不是有效的变量类型.
      clStringW strTypename = szTypeName;
      m_ErrorMsg.WriteErrorW(TRUE, nSourceOffset, E_1201_INVALID_TYPE, (GXLPCWSTR)strTypename);
      *result = FALSE;
      return NULL;
    }
    return it->first;
  }

  u64 DataPoolResolverImpl::ParseInitValue(const clStringA& strValue) const
  {
    // TODO: 检查字符串作为数字的合法性
    // TODO: 表达式解析?
    // FIXME: 64/32位,有符号/无符号混乱

    if(strValue.BeginsWith("0x") || strValue.BeginsWith("0X")) {
      clStringA str = strValue.Right(strValue.GetLength() - 2);      
      return str.ToInteger(16);
    }
    else if(strValue[0] == '0') {
      return strValue.ToInteger(8);
    }
    return strValue.ToInteger();
  }

  GXBOOL DataPoolResolverImpl::MainCompile(DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength)
  {
    // 填充内置数据类型
    DEFINE def;
    def.SourceFileId = 0;
    def.SourceOffset = 0;
    for(int i = 0; Implement::c_InternalTypeDefine[i].Name != NULL; ++i)
    {
      def.type = ST_VarType;
      m_NameDict[Implement::c_InternalTypeDefine[i].Name] = def;
    }

    // 加载编译信息资源
    m_ErrorMsg.LoadErrorMessageW(L"dpcmsg.txt");
    m_ErrorMsg.SetMessageSign('C');

    GXBOOL bval = IntCompile(pInclude, szDefinitionCodes, nCodeLength);
    if(bval)
    {
      TYPE_DECLARATION TypeDecl;
      VARIABLE_DECLARATION VarDeclEnd;
      InlSetZeroT(TypeDecl);
      InlSetZeroT(VarDeclEnd);
      m_aTypes.push_back(TypeDecl);
      m_aVariables.push_back(VarDeclEnd);
    }
    return bval && m_ErrorMsg.GetErrorLevel();
  }

  GXBOOL DataPoolResolverImpl::IntCompile(DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength)
  {
    SmartStreamA ss;
    u32 SemiTable[128];
    GXBOOL result = TRUE;

    const clsize nCodeLen = nCodeLength ? nCodeLength : clstd::strlenT(szDefinitionCodes);
    ss.Initialize(szDefinitionCodes, (u32)nCodeLen);
    ss.GetCharSemantic(SemiTable, 0, 128);
    SemiTable['/'] |= SmartStreamA::M_CALLBACK;
    SemiTable['#'] |= SmartStreamA::M_CALLBACK;
    ss.SetCharSemantic(SemiTable, 0, 128);
    ss.SetTriggerCallBack(TriggerProc, (u32_ptr)this);
    ss.SetFlags(SmartStreamA::F_SYMBOLBREAK);
    //m_Sources.back().tl.Generate(szDefinitionCodes, nCodeLen);
    m_ErrorMsg.GenerateCurLines(szDefinitionCodes, nCodeLen);

    if(pInclude) {
      m_pInclude = pInclude;
    }

    //TRACE("\n============================\n");
    StreamIter it = ss.begin();
    auto nPrevOffset = it.offset();
    for(;it != ss.end(); ++it)
    {
      if(it == ";") {
        continue;
      }
      else if(it == "always")
      {
        nPrevOffset = it.offset();
        ++it;
        if(it != "struct")
        {
          result = FALSE;
          // "\"always\" 关键字只能用来修饰结构体声明"
          m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1100_ALWAYS_MODIFIES_STRUCT_ONLY);
        }
        if( ! ParseStruct(ss, it, TRUE)) {
          result = FALSE;
          break;
        }
      }
      else if(it == "struct")
      {
        if( ! ParseStruct(ss, it, FALSE)) {
          result = FALSE;
          break;
        }
      }
      else if(it == "enum")
      {
        ParseEnum(ss, it, false);
      }
      else if(it == "flag")
      {
        ParseEnum(ss, it, true);
      }
      else {
        VARIABLE_DECLARATION VarDecl;

        if(it == "const") {
          nPrevOffset = it.offset();
          ++it;
          VarDecl.Flags = VarDeclFlag_Const;
        }
        else {
          VarDecl.Flags = 0;
        }

        if(ParseVariable(ss, it, m_VarNameSet, &VarDecl)) {
          m_aVariables.push_back(VarDecl);
        }
        else {
          result = FALSE;
          break;
        }
      }
      //TRACE(it.ToString());
    }
    return result;
  }

  template<typename _Ty>
  GXBOOL DataPoolResolverImpl::ParseInitList(_Ty* aArray, const clStringArrayA& aInit)
  {
    int i = 0;
    for(clStringArrayA::const_iterator it = aInit.begin();
      it != aInit.end(); ++it, ++i) {
        aArray[i] = (_Ty)ParseInitValue(*it);
    }
    return TRUE;
  }

  template<typename _Ty>
  void DataPoolResolverImpl::AllocInitPtr(VARIABLE_DECLARATION* pVarDecl, GXINT aInitArrayCount)
  {
    pVarDecl->Init = malloc(sizeof(_Ty) * abs(pVarDecl->Count));
    if(pVarDecl->Count > 0 && pVarDecl->Count > aInitArrayCount) {
      memset(pVarDecl->Init, 0, pVarDecl->Count * sizeof(_Ty));
    }
  }

  GXBOOL DataPoolResolverImpl::ParseAssignment(SmartStreamA& ss, StreamIter& it, VARIABLE_DECLARATION* pVarDecl)
  {
    clStringArrayA aInit;
    if(it == '{') {
      StreamIter itBegin;
      StreamIter itEnd;

      if( ! SmartStreamUtility::FindPair(it, itBegin, itEnd, '{', '}')) {
        // "缺少 \'}\'."
        m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1021_NEED_END_OF_BRACE);
        return FALSE;
      }

      while(++it != itEnd) {
        aInit.push_back(it.ToString());
        ++it;
        if(it == itEnd) {
          break;
        }
        if(it != ',') {
          //CLOG_ERROR("error: missing expected \',\' in array.\n");
          m_ErrorMsg.WriteErrorA(TRUE, it.marker, "数组中缺少 \',\' 分割.");
          return FALSE;
        }
      }
    }
    else {
      aInit.push_back(it.ToString());
    }

    ASSERT(pVarDecl->Count != 0);

    if(pVarDecl->Count > 0) {
      if(aInit.size() > (GXUINT)pVarDecl->Count) {
        m_ErrorMsg.WriteErrorA(TRUE, NULL, "赋值项太多了");
        return FALSE;
      }
    }
    else {
      pVarDecl->Count = -(GXINT)aInit.size();
    }

    TypeCategory eCate = GetVarCateByName(pVarDecl->Type);
    switch(eCate)
    {
    case T_UNDEFINE:
      m_ErrorMsg.WriteErrorA(TRUE, it.marker, "错误的类型名(%s)", pVarDecl->Type);
      return FALSE;
    
    case T_OBJECT:
      CLOG_ERROR("error: unsupport type.\n");
      return FALSE;

    case T_STRUCT:
    case T_STRUCTALWAYS:
      m_ErrorMsg.WriteErrorA(TRUE, it.marker, "\"object\"类型(%s)不支持赋值", pVarDecl->Name);
      return FALSE;

    case T_BYTE:
    case T_SBYTE:
      AllocInitPtr<u8>(pVarDecl, (GXINT)aInit.size());
      ParseInitList((u8*)pVarDecl->Init, aInit);
      break;

    case T_WORD:
    case T_SWORD:
      AllocInitPtr<u16>(pVarDecl, (GXINT)aInit.size());
      ParseInitList((u16*)pVarDecl->Init, aInit);
      break;

    case T_DWORD:
    case T_SDWORD:
      AllocInitPtr<u32>(pVarDecl, (GXINT)aInit.size());
      ParseInitList((u32*)pVarDecl->Init, aInit);
      break;

    case T_SQWORD:
    case T_QWORD:
      AllocInitPtr<u64>(pVarDecl, (GXINT)aInit.size());
      ParseInitList((u64*)pVarDecl->Init, aInit);
      break;

    case T_FLOAT:
      {
        int i = 0;
        AllocInitPtr<float>(pVarDecl, (GXINT)aInit.size());
        float* aFloats = (float*)pVarDecl->Init;

        for(clStringArrayA::iterator it = aInit.begin();
          it != aInit.end(); ++it, ++i) {
            aFloats[i] = (float)it->ToFloat();
        }
      }
      break;

    case T_STRING:
      {
        // 字符串列表格式:"str1\0str2\0str3" 维度由pVarDecl->Count指定
        GXSIZE_T nSize = 0; // WCHAR
        for(clStringArrayA::iterator it = aInit.begin();
          it != aInit.end(); ++it) {
            nSize += (it->GetLength() + 1);
        }

        pVarDecl->Init = malloc(sizeof(GXWCHAR) * nSize);// new GXWCHAR[nSize];
        GXWCHAR* pDest = (GXWCHAR*)pVarDecl->Init;
        for(clStringArrayA::iterator it = aInit.begin();
          it != aInit.end(); ++it) {
            clStringW str = (GXLPCSTR)*it; // ANSI to UNICODE
            GXSTRCPY(pDest, (const GXWCHAR*)str);
            pDest += str.GetLength() + 1;
        }
      }
      //CLOG_ERROR("error: It is not support initializing string.\n");
      break;
    }
    
    return TRUE;
  }

  GXBOOL DataPoolResolverImpl::ParseVariable(SmartStreamA& ss, StreamIter& it, clStringHashSetA& sNameSet, VARIABLE_DECLARATION* pVarDecl)
  {
    GXBOOL bSigned = TRUE;
    GXBOOL result = TRUE;
    auto nPrevOffset = it.offset(); // 用来记住出错前的指针位置
    GXBOOL bTmplArray = FALSE;
    EPSection eps(this);

    pVarDecl->Type = NULL;
    pVarDecl->Name = NULL;
    pVarDecl->Init = NULL;
    pVarDecl->Count = 1;
    pVarDecl->Flags = 0;

    enum {
      STEP_TmplBegin, // "<"
      STEP_TmplEnd,   // ">"
      STEP_Sign,
      STEP_Type,
      STEP_Name,
      STEP_Count,
      STEP_Assign,
      STEP_End,
    }Step = STEP_Sign;


    if(it == "array") // “array<Type>” 模板声明
    {
      ++it;
      Step = STEP_TmplBegin;
      bTmplArray = TRUE;
      pVarDecl->Count = -1;
    }


    while(it != ss.end())
    {
      switch(Step)
      {
      case STEP_TmplBegin:
        Step = STEP_Sign;
        if(it != "<") {
          m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1014_ARRAY_NEED_TMPL_ARG);
          result = FALSE;
        }
        break;

      case STEP_Sign:
        Step = STEP_Type;
        if(it == "unsigned") {
          bSigned = FALSE;
          break;
        }
        else {
          continue;
        }
        CLBREAK;
        break; // 不应该走到这里

      case STEP_Type:
        if(bSigned) {
          pVarDecl->Type = CheckType(it.ToString(), it.offset(), &result);
        }
        else {
          clStringA str = "unsigned_";
          str += it.ToString();
          pVarDecl->Type = CheckType(str, it.offset(), &result);
        }
        Step = bTmplArray ? STEP_TmplEnd : STEP_Name;
        break;

      case STEP_TmplEnd:
        Step = STEP_Name;
        if(it != ">") {
          m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1015_BAD_OF_ARRAY_TMPL_AT_END);
          result = FALSE;
        }
        break;

      case STEP_Name:
        pVarDecl->Name = AddString(ST_VarName, &sNameSet, it.ToString(), it.offset(), &result);
        if( ! DataPool::IsIllegalName(pVarDecl->Name)) {
          clStringW strVarName = pVarDecl->Name;
          m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1200_CANT_USE_AS_VAR_NAME, (GXLPCWSTR)strVarName);
          result = FALSE;
        }
        Step = bTmplArray ? STEP_End : STEP_Count;
        break;

      case STEP_Count:
        Step = STEP_Assign;
        if(it != "[") {
          continue;
        }

        nPrevOffset = it.offset();
        ++it;

        if(it == "]") {
          pVarDecl->Count = -1;
        }
        else {
          pVarDecl->Count = atoi(it.ToString());

          if(pVarDecl->Count <= 0) {
            //CLOG_ERROR(STD_ERROR_HEAD"can not use negative or zero count.\n", m_strFilename, LineFromPtr(pPrevPtr));
            // 数组不能指定小于等于0的长度
            m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1016_ILLEGAL_NUM_OF_ARRAY);
            result = FALSE;
          }

          nPrevOffset = it.offset();
          ++it;

          if(it != "]") {
            //CLOG_ERROR(STD_ERROR_HEAD"missing expected \']\'.\n", m_strFilename, LineFromPtr(pPrevPtr));
            //CLBREAK;
            // "缺少\']\'符号"
            m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1017_NEED_END_OF_SQUARE_BRACKETS);
            result = FALSE;
          }
        }
        break;

      case STEP_Assign:
        Step = STEP_End;
        if(it != '=' && it != '{') {
          continue;
        }

        nPrevOffset = it.offset();
        ++it;

        if( ! ParseAssignment(ss, it, pVarDecl)) {
          return FALSE;
        }
        break;

      case STEP_End:
        if(it != ";") {
          LOG_NOT_FOUND_SEMICOLON(nPrevOffset);
          it = ss.find(it, 2, ";", "}");
          result = FALSE;
        }
        return result;
      }

      nPrevOffset = it.offset();
      ++it;
    }

    return result;
  }

  GXBOOL DataPoolResolverImpl::ParseEnumItem(SmartStreamA& ss, StreamIter& it, const StreamIter& itEnd, ENUM_DECLARATION& sEnum)
  {
    enum
    {
      STEP_Name,
      STEP_EqualOperator,
      STEP_Value,
      STEP_End,
    }Step = STEP_Name;

    auto nPrevOffset = it.offset(); // 用来记住出错前的偏移
    GXBOOL result = TRUE;

    while(it != ss.end())
    {
      switch(Step)
      {
      case STEP_Name: // 枚举值
        sEnum.Name = AddString(ST_EnumName, NULL, it.ToString(), it.offset(), &result);

        if( ! DataPool::IsIllegalName(sEnum.Name)) {
          // \"%s\" 不能作为枚举名字使用
          m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1401_CANT_USE_AS_ENUM_NAME, sEnum.Name);
          result = FALSE;
        }

        Step = STEP_EqualOperator;
        break;

      case STEP_EqualOperator: // 等号
        if(it == itEnd || it == ",") {
          return result;
        }
        else if(it == "=") {
          Step = STEP_Value;
        }
        else {
          clStringW str = (GXLPCSTR)it.ToString();
          LOG_SYNTAX_ERROR(it.offset(), (GXLPCWSTR)str);
          result = FALSE;
        }
        break;

      case STEP_Value: // 解析值
        {
          Step = STEP_End;
          GXBOOL bPositive = TRUE;
          if(it == "+" || it == "-") {
            if(it.marker[0] == '-') { bPositive = FALSE; }
            ++it;
          }

          if(IsNumeric(it, &sEnum.Value))
          {
            // 根据之前的符号转换为负值
            sEnum.Value = bPositive ? sEnum.Value : -sEnum.Value;
          }
          else{
            cllist<StreamIter> listIters;
            nPrevOffset = it.offset();
            SmartStreamUtility::Get(it, itEnd, 0, listIters, [](const StreamIter& it)->GXBOOL {
              if(it == "," || it == "}") {
                return FALSE;
              }
              return TRUE;
            });

            if(listIters.empty()) {
              // 无法解析表达式
              m_ErrorMsg.WriteErrorW(TRUE, nPrevOffset, E_1403_CANT_PARSE_EXPRESSION);
              result = FALSE;
            }
            else{
              result = MathExpressionParser(listIters, &sEnum.Value);
            }

            nPrevOffset = it.offset();
            continue;
          }
        }
        break;

      case STEP_End:
        if(it != itEnd && it != ",") {
          clStringW str = (GXLPCSTR)it.ToString();
          m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1050_ILLEGAL_END, (GXLPCWSTR)str);
          result = FALSE;
        }
        return result;
      }

      nPrevOffset = it.offset();
      ++it;
    }
    return result;
  }

  GXBOOL DataPoolResolverImpl::ParseStruct(SmartStreamA& ss, StreamIter& it, GXBOOL bAlways)
  {
    VarDeclArray* pStruct = new VarDeclArray;
    TYPE_DECLARATION TypeDecl;
    StreamIter itBegin;
    StreamIter itEnd;
    GXBOOL result = TRUE;
    ASSERT(it == "struct");
    ++it;
    TypeDecl.Cate = bAlways ? T_STRUCTALWAYS : T_STRUCT;
    TypeDecl.Name = AddString(ST_StructType, NULL, it.ToString(), it.offset(), &result);
    
    if( ! DataPool::IsIllegalName(TypeDecl.Name)) {
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1101_CANT_USE_AS_STRUCT_NAME, TypeDecl.Name);      // "%s" 不能作为结构体名使用
      result = FALSE;
    }

    ++it;
    if(it != "{") {
      // struct 定义错误
      //CLOG_ERROR(STD_ERROR_HEAD"缺少期望的\'{\'符号。\n", m_strFilename, LineFromPtr(it.marker));
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1020_NEED_BEGIN_OF_BRACE);
      return FALSE;
    }
    
    if( ! SmartStreamUtility::FindPair(it, itBegin, itEnd, "{", "}")) {
      // 花括号不匹配
      //CLOG_ERROR(STD_ERROR_HEAD"找不到对应的\'}\'。\n", m_strFilename, LineFromPtr(it.marker));
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1021_NEED_END_OF_BRACE);
      return FALSE;
    }

    VARIABLE_DECLARATION VarMember;
    clStringHashSetA sVarNameSet;
    while(++it != itEnd && it != ss.end())
    {
      if(ParseVariable(ss, it, sVarNameSet, &VarMember)) {
        pStruct->push_back(VarMember);
      }
      else {
        result = FALSE; // 错误不返回，继续报其他错误
      }
    }
    ++it;
    if(it != ";") {
      LOG_NOT_FOUND_SEMICOLON(it.offset());
      result = FALSE;
    }

    // 增加结尾
    VARIABLE_DECLARATION VarDeclEnd;
    InlSetZeroT(VarDeclEnd);
    pStruct->push_back(VarDeclEnd);

    // 结尾 it 应该为 ";"
    TypeDecl.as.Struct = &(pStruct->front()); // 这个填入后 TypeStorage.aVariable 就不能再追加数据了
    m_aStructMember.push_back(pStruct);
    m_aTypes.push_back(TypeDecl);
    return result;
  }

  GXBOOL DataPoolResolverImpl::ParseEnum(SmartStreamA& ss, StreamIter& it, GXBOOL bFlag)
  {
    EnumPairArray* pEnum = new EnumPairArray;
    TYPE_DECLARATION TypeDecl;
    StreamIter itBegin;
    StreamIter itEnd;
    GXBOOL result = TRUE;
    ASSERT(it == "enum" || it == "flag");

    // 预先加入列表，避免按照值查找时找不到
    m_aEnums.push_back(pEnum);

    ++it;
    TypeDecl.Cate = bFlag ? T_FLAG : T_ENUM;
    TypeDecl.Name = AddString(ST_EnumType, NULL, it.ToString(), it.offset(), &result);

    if( ! DataPool::IsIllegalName(TypeDecl.Name)) {
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1400_CANT_DECL_AS_ENUM_TYPE, TypeDecl.Name);
      result = FALSE;
    }

    ++it;
    if(it != "{") {
      // enum 定义错误
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1020_NEED_BEGIN_OF_BRACE);
      return FALSE;
    }

    if( ! SmartStreamUtility::FindPair(it, itBegin, itEnd, "{", "}")) {
      // 花括号不匹配
      m_ErrorMsg.WriteErrorW(TRUE, it.offset(), E_1021_NEED_END_OF_BRACE);
      return FALSE;
    }

    ENUM_DECLARATION sEnum = {0}; // 如果没有设置初始值，枚举从0开始

    while(++it != itEnd && it != ss.end())
    {
      if(ParseEnumItem(ss, it, itEnd, sEnum)) {

        ASSERT(m_EnumDict.find(sEnum.Name) == m_EnumDict.end());
        m_EnumDict[sEnum.Name] = sEnum.Value;

        pEnum->push_back(sEnum);
      }
      else {
        result = FALSE; // 错误不返回，继续报其他错误
      }

      if(it == itEnd) {
        break;
      }

      ++sEnum.Value;
    }

    ++it;
    if(it != ";") {
      // 缺少";"
      LOG_NOT_FOUND_SEMICOLON(it.offset());
      CLBREAK;
      result = FALSE;
    }

    // 增加结尾
    ENUM_DECLARATION sEnumEnd;
    InlSetZeroT(sEnumEnd);
    pEnum->push_back(sEnumEnd);

    // 结尾 it 应该为 ";"
    TypeDecl.as.Enum = &(pEnum->front()); // 这个填入后 TypeStorage.aVariable 就不能再追加数据了    
    m_aTypes.push_back(TypeDecl);
    return result;
  }


  GXBOOL DataPoolResolverImpl::FindValueFromEnum(const StreamIter& it, GXINT* nValue)
  {
    auto str = it.ToString();
    auto itEnum = m_EnumDict.find(str);

    if(itEnum == m_EnumDict.end()) {
      return FALSE;
    }

    *nValue = itEnum->second;
    return TRUE;
  }


  GXHRESULT DataPoolResolverImpl::GetCompileResult()
  {
    static GXHRESULT aResultTab[] = {GX_FAIL, 1, GX_OK};
    return aResultTab[m_ErrorMsg.GetErrorLevel()];
  }

  GXBOOL DataPoolResolverImpl::MathExpressionParser( const cllist<StreamIter>& expression, GXINT* nValue )
  {
    ASSERT( ! expression.empty()); // 外面保证这个

    clstd::MathExpressionParser<StreamIter> m;

    if( ! m.ParseExpression(expression)) {
      // 无法解析表达式
      m_ErrorMsg.WriteErrorW(TRUE, expression.front().offset(), E_1403_CANT_PARSE_EXPRESSION);
    }

    return m.CalculateValue<GXINT>(nValue, [this](const StreamIter& it, GXINT* value)->GXBOOL{
      GXBOOL bval = IsNumeric(it, value);
      if( ! bval) {
        return FindValueFromEnum(it, value);
      }
      return bval;
    });
  }

  GXBOOL DataPoolResolverImpl::CheckIncludeLoop( GXLPCWSTR szFilename, GXSIZE_T nSourceOffset )
  {
    const GXBOOL bval = m_ErrorMsg.CheckIncludeLoop(szFilename);
    if( ! bval) {
      m_ErrorMsg.WriteErrorW(TRUE, nSourceOffset, E_1500_LOOP_INCLUDE, szFilename);
    }
    return bval;
  }

  GXHRESULT DataPoolCompiler::CreateFromMemory(DataPoolCompiler** ppResolver, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength)
  {
    DataPoolResolverImpl* pResolver = new DataPoolResolverImpl;
    if( ! InlCheckNewAndIncReference(pResolver)) {
      return GX_FAIL;
    }

    GXHRESULT hval = GX_OK;
    if( ! pResolver->MainCompile(pInclude, szDefinitionCodes, nCodeLength)) {
      pResolver->Release();
      pResolver = NULL;
      hval = GX_FAIL;
    }

    *ppResolver = pResolver;
    return hval;
  }

  //////////////////////////////////////////////////////////////////////////


} // namespace Marimo