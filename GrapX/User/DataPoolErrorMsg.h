#ifndef _MARIMO_DATAPOOL_ERROR_MESSAGE_H_
#define _MARIMO_DATAPOOL_ERROR_MESSAGE_H_

namespace Marimo
{
  template<typename _TChar>
  class GXDLL DataPoolErrorMsg
  {
    struct FILE_SECTION
    {
      typedef clstd::TextLines<_TChar>          TextLines;
      TextLines tl;
      clStringW strFilename;  // 用于报错时提示的文件名
      GXINT     nBaseLine;    // 报错时提示的起始行号
    };

    typedef clvector<FILE_SECTION*>           FileSectionTable;
    typedef cllist<FILE_SECTION*>             FileSectionList;
    typedef clmap<GXUINT, clStringW>          ErrorMsgDict;
    typedef const _TChar*                     T_LPCSTR;

    GXWCHAR               m_cSign;        // 错误号码前面的签名，如"error(C1200)"或者"error(I1200)"
    FileSectionTable      m_SourcesTable;
    FileSectionList       m_Sources;      // 编译的文件名信息
    ErrorMsgDict          m_ErrorMsg;
    GXBOOL                m_bSilent;      // 静默模式, 不提示任何警告
    int                   m_CompileCode;  // 0:失败; 1:成功但是含有警告; 2:成功无任何信息

  protected:
    GXHRESULT UpdateResult  (GXBOOL bError);
  public:
    DataPoolErrorMsg();
    virtual ~DataPoolErrorMsg();
    GXBOOL    LoadErrorMessageW   (GXLPCWSTR szErrorFile);
    void      SetMessageSign      (GXWCHAR cSign);
    void      SetCurrentFilenameW (GXLPCWSTR szFilename);
    void      SetCurrentTopLine   (GXINT nTopLine);
    GXSIZE_T  GenerateCurLines    (T_LPCSTR pText, clsize length);
    GXLPCWSTR GetFilenameW        (GXUINT idFile = 0) const;
    GXUINT    GetCurrentFileId    () const;
    int       LineFromPtr         (T_LPCSTR ptr) const;
    int       LineFromOffset      (GXSIZE_T nOffset, const FILE_SECTION* pfs) const;
    int       LineFromOffset      (GXSIZE_T nOffset, GXUINT idFile = 0) const;
    void      PushFile            (GXLPCWSTR szFilename, GXINT nTopLine = 0);
    void      PopFile             ();

    void      WriteMessageW       (GXBOOL bError, GXLPCWSTR szMessage);
    void      WriteErrorW         (GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, ...);
    void      VarWriteErrorW      (GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, va_list arglist); // 从当前文件的偏移确定位置
    void      VarWriteErrorW      (GXBOOL bError, T_LPCSTR ptr, GXUINT nCode, va_list arglist);     // 从指针确定源文件位置，这个支持多个源文件的行号查询
    void      WriteErrorA         (GXBOOL bError, T_LPCSTR pSourcePtr, GXLPCSTR message, ...);
    int       GetErrorLevel       () const;   // 参考 m_CompileCode
    GXBOOL    CheckIncludeLoop    (GXLPCWSTR szFilename) const;
    void      SetSilentMode       (GXBOOL bSilent);

    const FILE_SECTION* SectionFromPtr(T_LPCSTR ptr) const; // 从指针找到来自哪一个文件
  };

} // namespace Marimo

#endif // #ifndef _MARIMO_DATAPOOL_ERROR_MESSAGE_H_