#ifndef _CLSTD_TEXT_LINES_H_
#define _CLSTD_TEXT_LINES_H_

// �ı��кż��㹤��
// ��Ҫ���ڷ����ı��������ݣ��ṩ�У���ת����ƫ�����ʹ�ƫ����ת��Ϊ����

namespace clstd
{
  template<typename _TChar>
  class TextLines
  {
    typedef clvector<clsize> LinesArray;
    const _TChar* m_pText;
    clsize        m_nLength;
    LinesArray    m_aLines;
    
    // ��¼��һ��offset�����У���Ϊ�ٶ�PosFromOffset�ܸ߸������Ե���offset�ķ�ʽ���õ�
    // ʵ�ʲ���90�����ı�14��β���ʹ�õ������ַ�����ͨ���ַ�����ʱ��Ĳ��΢����΢
    //clsize        m_nPrevLine;

  protected:
    clsize IntGenerate(const _TChar* pText, clsize length)
    {
      clsize i = 0;
      m_aLines.push_back(i);
      for(; i < length; ++i) {
        if(pText[i] == '\n') {
          m_aLines.push_back(i);
        }
      }
      m_aLines.push_back(i);
      return m_aLines.size();
    }

    b32 PosFromOffset(clsize nOffset, clsize start, clsize end, int* nLine, int* nRow) const
    {
      //while(start != end && start != end - 1) {
      while(start < end - 1)
      {
        const clsize mid = (start + end) >> 1;;

        if(m_aLines[start] <= nOffset && nOffset < m_aLines[mid]) {
          end = mid;
        }
        else if(m_aLines[mid] <= nOffset && nOffset < m_aLines[end]) {
          start = mid;
        }
        else if(nOffset >= m_nLength) {
          // ��������ı����ȣ��к���Ϊ���һ��
          *nLine = (int)m_aLines.size() - 1;
          *nRow = 0;
          return FALSE;
        }
        else {
          *nLine = 0;
          *nRow = 0;
          return FALSE;
        }
      } 

      //m_nPrevLine = start;
      *nLine = (int)(start + 1);
      *nRow  = (int)(nOffset - m_aLines[start]);
      return TRUE;
    }
  public:
    TextLines() : m_pText(NULL), m_nLength(0) {}
    TextLines(const _TChar* pText, clsize length)
      : m_pText(pText)
      , m_nLength(length) { Generate(pText, length); }

    clsize Generate(const _TChar* pText, clsize length)
    {
      m_pText = pText;
      m_nLength = length;
      m_aLines.clear();

      return IntGenerate(pText, length);
    }

    clsize OffsetFromPos(int nLine, int nRow) // nLine �� nRow ���Ǵ�1��ʼ�ģ������õ�ƫ�Ʋ����ڷ���(clsize)-1
    {
      clsize nBase = nLine < 1 ? 0 : m_aLines[nLine - 1];
      
      if((clsize)nLine < m_aLines.size() && (clsize)nRow < m_aLines[nLine] - nBase) {
        return nRow + nBase;
      }

      return (clsize)-1;
    }

    b32 PosFromOffset(clsize nOffset, int* nLine, int* nRow) const
    {
      return PosFromOffset(nOffset, 0, m_aLines.size() - 1, nLine, nRow);
    }

    b32 PosFromPtr(const _TChar* pText, int* nLine, int* nRow) const
    {
      return PosFromOffset(pText - m_pText, nLine, nRow);
    }

    b32 IsPtrIn(const _TChar* ptr) const
    {
      return ptr >= m_pText && ptr < (m_pText + m_nLength);
    }

    const _TChar* GetPtr() const
    {
      return m_pText;
    }

    clsize GetLength() const
    {
      return m_nLength;
    }
  };

  typedef TextLines<wch> TextLinesW;
  typedef TextLines< ch> TextLinesA;

} // namespace clstd

#endif // _CLSTD_TEXT_LINES_H_