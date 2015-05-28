#ifndef _MARIMO_DATAPOOL_ERROR_MESSAGE_H_
#define _MARIMO_DATAPOOL_ERROR_MESSAGE_H_

namespace Marimo
{
  template<typename _TChar>
  class DataPoolErrorMsg
  {
    struct FILE_SECTION
    {
      typedef clstd::TextLines<_TChar>          TextLines;
      TextLines tl;
      clStringW strFilename;  // ���ڱ���ʱ��ʾ���ļ���
      GXINT     nBaseLine;    // ����ʱ��ʾ����ʼ�к�
    };

    typedef clvector<FILE_SECTION*>           FileSectionTable;
    typedef cllist<FILE_SECTION*>             FileSectionList;
    typedef clmap<GXUINT, clStringW>          ErrorMsgDict;
    typedef const _TChar*                     T_LPCSTR;

    GXWCHAR               m_cSign;        // �������ǰ���ǩ������"error(C1200)"����"error(I1200)"
    FileSectionTable      m_SourcesTable;
    FileSectionList       m_Sources;      // ������ļ�����Ϣ
    ErrorMsgDict          m_ErrorMsg;
    GXBOOL                m_bSilent;      // ��Ĭģʽ, ����ʾ�κξ���
    int                   m_CompileCode;  // 0:ʧ��; 1:�ɹ����Ǻ��о���; 2:�ɹ����κ���Ϣ

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
    int       LineFromOffset      (GXSIZE_T nOffset, GXUINT idFile = 0) const;
    void      PushFile            (GXLPCWSTR szFilename, GXINT nTopLine = 0);
    void      PopFile             ();
    void      WriteErrorW         (GXBOOL bError, GXSIZE_T nOffset, GXUINT nCode, ...);
    //void      WriteErrorW         (GXBOOL bError, T_LPCSTR pSourcePtr, GXLPCWSTR message, ...);
    void      WriteErrorA         (GXBOOL bError, T_LPCSTR pSourcePtr, GXLPCSTR message, ...);
    int       GetErrorLevel       () const;
    GXBOOL    CheckIncludeLoop    (GXLPCWSTR szFilename) const;
    void      SetSilentMode       (GXBOOL bSilent);
  };

} // namespace Marimo

#endif // #ifndef _MARIMO_DATAPOOL_ERROR_MESSAGE_H_