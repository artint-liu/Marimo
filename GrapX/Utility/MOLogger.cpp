#include "GrapX.h"
#include "User/GrapX.Hxx"

//#include "GrapX/GUnknown.H"
#include "GrapX/GXKernel.H"
#include "GrapX/MOLogger.H"
#include "clfifo.h"

class FileLoggerImpl : public ILogger
{
  friend GXHRESULT GXDLLAPI MOCreateFileLoggerW(ILogger**ppLogger, GXLPCWSTR szOutputFile, GXBOOL bUnicode);

private:
  clStringW     m_strFilename;
  clFile        m_file;
  clstd::Locker m_locker;
  u32           m_bUnicode : 1;

private:
  FileLoggerImpl(GXLPCWSTR szFilename, GXBOOL bUnicode)
    : m_strFilename (szFilename)
    , m_bUnicode    (bUnicode)
  {
  }

  virtual ~FileLoggerImpl()
  {
    OutputW(L"[End of log file]\r\n");
    m_file.Close();
  }

  GXBOOL Initialize()
  {
    return m_file.CreateAlwaysW(m_strFilename);
  }

public:
  GXHRESULT AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  template<typename _Ty>
  inline u32 OutputT(const _Ty* szText, u32 length, const _Ty* szEnd = NULL, u32 endLen = 0)
  {
    if(length == 0) {
      return 0;
    }

    clstd::ScopedLocker locker(m_locker);

    if(length == (u32)-1) {
      length = GXSTRLEN(szText);
    }

    m_file.Write(szText, length * sizeof(_Ty));

    if(endLen && szText[length - 1] != (_Ty)'\n') {
      m_file.Write(szEnd, endLen * sizeof(_Ty));
    }

    return length;
  }

  virtual u32 OutputW(GXLPCWSTR szString)
  {
    if(m_bUnicode) {
      return OutputT(szString, GXSTRLEN(szString)/*, L"\r\n", 2*/);
    }
    else {
      clStringA str(szString);
      return OutputT<ch>(str, str.GetLength());
    }
  }

  virtual u32 OutputA(GXLPCSTR szString)
  {
    if(m_bUnicode)
    {
      clStringW str(szString);
      return OutputT<wch>(str, str.GetLength());
    }
    else {
      return OutputT(szString, GXSTRLEN(szString)/*, "\r\n", 2*/);
    }
  }

  virtual u32 OutputFormatA(GXLPCSTR szFormat, ...)
  {
    va_list  arglist;
    va_start(arglist, szFormat);

    clStringA str;
    str.VarFormat(szFormat, arglist);

    if(m_bUnicode) {
      clStringW strW(str, str.GetLength());
      return OutputT<wch>(strW, strW.GetLength());
    }
    else {
      return OutputT<ch>(str, str.GetLength());
    }
  }

  virtual u32 OutputFormatW(GXLPCWSTR szFormat, ...)
  {
    va_list  arglist;
    va_start(arglist, szFormat);

    clStringW str;
    str.VarFormat(szFormat, arglist);

    if(m_bUnicode) {
      return OutputT<wch>(str, str.GetLength());
    }
    else {
      clStringA strA(str, str.GetLength());
      return OutputT<ch>(strA, strA.GetLength());
    }
  }
};

//////////////////////////////////////////////////////////////////////////

class StreamLoggerImpl : public IStreamLogger
{
  friend GXHRESULT GXDLLAPI MOCreateMemoryStreamLoggerW(IStreamLogger**ppLogger, GXSIZE_T nBufferSize, GXBOOL bUnicode);
private:
  //clstd::Locker       m_locker;
  //clstd::FixedBuffer  m_buffer;
  LogStreamProc     m_pStreamProc;
  GXLPARAM          m_lParam;
  GXDWORD           m_idThread;
  clstd::fifo       m_fifo;
  GXDWORD           m_bUnicode;
private:
  StreamLoggerImpl(GXBOOL bUnicode)
    : m_bUnicode    (bUnicode)
    , m_pStreamProc (NULL)
    , m_lParam      (0)
    , m_idThread    (0)
  {
  }

  virtual ~StreamLoggerImpl()
  {
  }

  GXBOOL Initialize(GXSIZE_T nBufferSize)
  {
    return m_fifo.Initialize(nBufferSize, TRUE);
  }

public:
  virtual GXHRESULT AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  virtual GXHRESULT Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  virtual u32 OutputW(GXLPCWSTR szString)
  {
    u32 nRet = 0;
    if(m_bUnicode) {
      nRet = m_fifo.put((const u8*)szString, GXSTRLEN(szString) * sizeof(wch));
    }
    else {
      clStringA str(szString);
      nRet = m_fifo.put((const u8*)(const ch*)str, str.GetLength() * sizeof(ch));
    }
    CallStreamProc();
    return nRet;
  }

  virtual u32 OutputA(GXLPCSTR szString)
  {
    u32 nRet = 0;
    if(m_bUnicode) {
      clStringW str(szString);
      nRet = m_fifo.put((const u8*)(const wch*)str, str.GetLength() * sizeof(wch));
    }
    else {
      nRet = m_fifo.put((const u8*)szString, GXSTRLEN(szString) * sizeof(ch));
    }
    CallStreamProc();
    return nRet;
  }

  virtual u32 OutputFormatA(GXLPCSTR szFormat, ...)
  {
    va_list  arglist;
    va_start(arglist, szFormat);

    u32 nRet = 0;
    clStringA str;
    str.VarFormat(szFormat, arglist);

    if(m_bUnicode) {
      clStringW strW(str, str.GetLength());
      nRet = m_fifo.put((const u8*)(const wch*)strW, strW.GetLength());
    }
    else {
      nRet = m_fifo.put((const u8*)(const ch*)str, str.GetLength());
    }
    CallStreamProc();
    return nRet;
  }

  virtual u32 OutputFormatW (GXLPCWSTR szFormat, ...)
  {
    va_list  arglist;
    va_start(arglist, szFormat);

    u32 nRet = 0;
    clStringW str;
    str.VarFormat(szFormat, arglist);
    if(m_bUnicode) {
      nRet = m_fifo.put((const u8*)(const wch*)str, str.GetLength());
    }
    else {
      clStringA strA(str, str.GetLength());
      nRet = m_fifo.put((const u8*)(const ch*)strA, strA.GetLength());
    }
    CallStreamProc();
    return nRet;
  }

  virtual GXLPVOID SetStreamProc(GXDWORD idThread, LogStreamProc pNewProc, GXLPARAM lParam)
  {
    GXLPVOID pPrevProc = m_pStreamProc;
    m_pStreamProc = pNewProc;
    m_lParam = lParam;
    m_idThread = idThread;
    CallStreamProc();
    return pPrevProc;
  }

  void CallStreamProc()
  {
    if(m_pStreamProc && m_fifo.size())
    {
      u8 buffer[2048];

      if(m_idThread != 0 && m_idThread != gxGetCurrentThreadId()) {
        return;
      }

      while (true)
      {
        const u32 len = m_fifo.get(buffer, sizeof(buffer) - 4); // gcc 下 wchar_t 有可能是4字节
        if(len == 0) {
          break;
        }
        buffer[len] = '\0';
        buffer[len + 1] = '\0';
        buffer[len + 2] = '\0';
        buffer[len + 3] = '\0';
        m_pStreamProc(buffer, m_bUnicode, m_lParam);
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////////
GXHRESULT GXDLLAPI MOCreateFileLoggerW(ILogger**ppLogger, GXLPCWSTR szOutputFile, GXBOOL bUnicode)
{
  FileLoggerImpl* pLogger = new FileLoggerImpl(szOutputFile, bUnicode);
  if( ! InlCheckNewAndIncReference(pLogger)) {
    return GX_FAIL;
  }

  if( ! pLogger->Initialize()) {
    SAFE_RELEASE(pLogger);
    return GX_FAIL;
  }

  *ppLogger = pLogger;
  return GX_OK;     
}

GXHRESULT GXDLLAPI MOCreateMemoryStreamLoggerW(IStreamLogger**ppLogger, GXSIZE_T nBufferSize, GXBOOL bUnicode)
{
  StreamLoggerImpl* pLogger = new StreamLoggerImpl(bUnicode);
  if( ! InlCheckNewAndIncReference(pLogger)) {
    return GX_FAIL;
  }

  if( ! pLogger->Initialize(nBufferSize)) {
    SAFE_RELEASE(pLogger);
    return GX_FAIL;
  }

  *ppLogger = pLogger;
  return GX_OK;     
}

//////////////////////////////////////////////////////////////////////////
GXVOID GXDLLAPI MOLogW( GXLPCWSTR szFormat, ... )
{
  va_list  arglist;
  va_start(arglist, szFormat);

  clStringW str;
  str.VarFormat(szFormat, arglist);

  GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->m_pLogger->OutputW(str);
}

//////////////////////////////////////////////////////////////////////////
GXVOID GXDLLAPI MOLogA( GXLPCSTR szFormat, ... )
{
  va_list  arglist;
  va_start(arglist, szFormat);

  clStringA str;
  str.VarFormat(szFormat, arglist);

  GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->m_pLogger->OutputA(str);
}

//////////////////////////////////////////////////////////////////////////
GXVOID GXDLLAPI MOLogOutputW( GXLPCWSTR szText )
{
  GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->m_pLogger->OutputW(szText);
}

//////////////////////////////////////////////////////////////////////////
GXVOID GXDLLAPI MOLogOutputA( GXLPCSTR szText )
{
  GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->m_pLogger->OutputA(szText);
}