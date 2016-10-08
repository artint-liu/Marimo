#ifndef _MARIMO_LOGGER_H_
#define _MARIMO_LOGGER_H_

// TODO: 修改相关的头文件引用
class ILogger : public GUnknown
{
public:
  GXSTDINTERFACE(GXHRESULT  AddRef        ());
  GXSTDINTERFACE(GXHRESULT  Release       ());
  GXSTDINTERFACE(size_t     OutputW       (GXLPCWSTR szString));
  GXSTDINTERFACE(size_t     OutputA       (GXLPCSTR szString));
  GXSTDINTERFACE(size_t     OutputFormatA (GXLPCSTR szFormat, ...));
  GXSTDINTERFACE(size_t     OutputFormatW (GXLPCWSTR szFormat, ...));
};


class IStreamLogger : public ILogger
{
public:
  typedef GXINT (GXCALLBACK *LogStreamProc)(GXLPVOID szText, GXBOOL bUnicode, GXLPARAM lParam);
public:
  GXSTDINTERFACE(GXHRESULT  AddRef        ());
  GXSTDINTERFACE(GXHRESULT  Release       ());
  GXSTDINTERFACE(size_t     OutputW       (GXLPCWSTR szString));
  GXSTDINTERFACE(size_t     OutputA       (GXLPCSTR szString));
  GXSTDINTERFACE(size_t     OutputFormatA (GXLPCSTR szFormat, ...));
  GXSTDINTERFACE(size_t     OutputFormatW (GXLPCWSTR szFormat, ...));
  GXSTDINTERFACE(GXLPVOID   SetStreamProc (GXDWORD idThread, LogStreamProc pNewProc, GXLPARAM lParam));
};

GXHRESULT GXDLLAPI MOCreateFileLoggerW(ILogger**ppLogger, GXLPCWSTR szOutputFile, GXBOOL bUnicode);
GXHRESULT GXDLLAPI MOCreateMemoryStreamLoggerW(IStreamLogger**ppLogger, GXSIZE_T nBufferSize, GXBOOL bUnicode);

#endif // #ifdef _MARIMO_LOGGER_H_