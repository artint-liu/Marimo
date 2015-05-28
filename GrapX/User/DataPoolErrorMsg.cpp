#include "GrapX.H"
#include "GrapX.Hxx"

#include "clTextLines.h"
#include "Smart/smartstream.h"
#include "Smart/SmartStock.h"

#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataPoolIterator.h"
#include "GrapX/GXKernel.H"

#include "DataPoolErrorMsg.h"
#include "DataPoolImpl.h"

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
    FILE_SECTION* fs = new FILE_SECTION;
    fs->nBaseLine = 0;
    m_Sources.push_back(fs);
    m_SourcesTable.push_back(NULL); // ����ʹ��0���Id
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
    // ����״̬
    // ����Ѿ��ڳ���״̬��������
    // ����״̬���ή��������״̬
    // ����״̬���ή����������ߴ���״̬
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