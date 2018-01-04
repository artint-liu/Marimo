#include "GrapX.H"
#include "GrapX.Hxx"

#include "clTextLines.h"
#include "Smart/smartstream.h"
#include <clTokens.h>
#include "clStock.h"

#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataPoolIterator.h"
#include "GrapX/GXKernel.H"

#include "DataPoolErrorMsg.h"
//#include "DataPoolImpl.h"

namespace Marimo
{
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
    //FILE_SECTION* fs = new FILE_SECTION;
    //fs->nBaseLine = 0;
    //m_Sources.push_back(fs);
    m_SourcesTable.push_back(NULL); // 避免使用0这个Id
    //m_SourcesTable.push_back(fs);
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
    clstd::StockW ss;
    if(ss.LoadFromFile(szErrorFile))
    {
      clstd::StockW::Section sectRoot = ss.OpenSection(NULL);
      if(sectRoot)
      {
        clstd::StockW::ATTRIBUTE param;
        if(sectRoot.FirstKey(param))
        {
          do {
            m_ErrorMsg[clstd::xtou(param.KeyName())] = param.ToString();
          } while (param.NextKey());
        }
        //ss.CloseSection(sectRoot);
      }
    }
    return FALSE;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::WriteMessageW(GXBOOL bError, GXLPCWSTR szMessage)
  {
    // 这里不能用CLOG_*
    if(bError) {
      _cl_traceW(szMessage);
    }
    else if( ! m_bSilent){
      _cl_traceW(szMessage);
    }
  }

  _DPEM_TEMPL
  void _DPEM_CLS::VarWriteErrorW(GXBOOL bError, T_LPCSTR ptr, GXUINT nCode, va_list arglist)
  {
    UpdateResult(bError);
    clStringW str;
    while((GXSIZE_T)ptr != (GXSIZE_T)-1) {
      if(ptr) {
        const FILE_SECTION* pfs = SectionFromPtr(ptr);
        if(pfs) {
          str.Format(bError ? STD_ERROR_HEAD_EXTW : STD_WARNING_HEAD_EXTW, (GXLPCWSTR)pfs->strFilename, 
            LineFromOffset((GXSIZE_T)ptr - (GXSIZE_T)pfs->tl.GetPtr(), pfs), m_cSign, nCode);
          break;
        }
      }
      str.Format(bError ? STD_ERROR_HEADW : STD_WARNING_HEADW, m_cSign, nCode);
      break;
    }

    auto it = m_ErrorMsg.find(nCode);
    if(it == m_ErrorMsg.end()) {
      str.AppendFormat(L"Missing Compiler error message.\r\n");
    }
    else {
      try {
        str.VarFormat(it->second, arglist);
        str.Append(L"\r\n");
      }
      catch(...)
      {
        str.Append(L"\r\n");
      }
    }

    WriteMessageW(bError, str);
  }

  _DPEM_TEMPL
  void _DPEM_CLS::VarWriteErrorW(GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, va_list arglist)
  {
    VarWriteErrorW(bError, m_Sources.back()->tl.GetPtr() + nOffset, nCode, arglist);
  }

  _DPEM_TEMPL
  void _DPEM_CLS::WriteErrorW(GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, ...)
  {
    if(m_Sources.empty()) {
      //CLOG_ERROR();
    }
    va_list  arglist;
    va_start(arglist, nCode);
    VarWriteErrorW(bError, m_Sources.back()->tl.GetPtr() + nOffset, nCode, arglist);
    va_end(arglist);
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
  int _DPEM_CLS::LineFromOffset(GXSIZE_T nOffset, const FILE_SECTION* pfs) const
  {
    ASSERT(pfs); // 外部检查这个不为空

    int nLine, nRow;
    pfs->tl.PosFromOffset(nOffset, &nLine, &nRow);
    return pfs->nBaseLine + nLine;
  }

  _DPEM_TEMPL
  int _DPEM_CLS::LineFromOffset(GXSIZE_T nOffset, GXUINT idFile) const
  {
    //int nLine, nRow;
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
    return LineFromOffset(nOffset, pfs);
    //pfs->tl.PosFromOffset(nOffset, &nLine, &nRow);
    //return pfs->nBaseLine + nLine;
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
  GXLPCWSTR _DPEM_CLS::GetFilePathW(GXUINT idFile) const
  {
    if(idFile == 0) {
      return m_Sources.back()->strPath;
    }
    else if(idFile < m_SourcesTable.size()) {
      return m_SourcesTable[idFile]->strPath;
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
    fs->strPath = szFilename;
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

  _DPEM_TEMPL
  const typename _DPEM_CLS::FILE_SECTION* _DPEM_CLS::SectionFromPtr(T_LPCSTR ptr) const
  {
    for(auto it = m_Sources.rbegin(); it != m_Sources.rend(); ++it)
    {
      const FILE_SECTION* pSect = *it;
      if(pSect->tl.IsPtrIn(ptr)) {
        return pSect;
      }
    }
    return NULL;
  }

  _DPEM_TEMPL
  void _DPEM_CLS::Destroy(_DPEM_CLS* pErrorMsg)
  {
    delete pErrorMsg;
  }

  _DPEM_TEMPL
  _DPEM_CLS* _DPEM_CLS::Create()
  {
    return new DataPoolErrorMsg;
  }

  template class DataPoolErrorMsg<GXWCHAR>;
  template class DataPoolErrorMsg<GXCHAR>;

} // namespace Marimo