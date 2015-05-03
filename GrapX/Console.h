#ifndef _GRAP_X_COLSOLE_H_
#define _GRAP_X_COLSOLE_H_

class roundbuffer // 这个目前和clfifo的代码很相似
{
protected:
  u8*       m_pBuffer;
  u32       m_uCapacity;
  u32       m_uBegin;
  u32       m_uEnd;
  clstd::Locker* m_pLocker;
  u32 m_bOverwrite : 1;
public:
  roundbuffer();
  virtual ~roundbuffer();

  b32   Initialize(u32 size, b32 bOverwrite); // 暂时不支持可覆盖的写入
  u32   Write     (u8* pData, u32 uLength);
  u32   Read      (u8* pData, u32 uLength);
  u8*   GetPtr    ();
  void  Lock      ();
  void  Unlock    ();
  b32   TryLock   ();
};

class TextRoundBuffer : public roundbuffer
{
public:
  struct LINEDESC
  {
    clBuffer*     pBuffer;
    int           nNumLines;
    clvector<int> aPos; // nNumLines + 1
    LINEDESC()
      : nNumLines(0)
    {
      pBuffer = new clBuffer;
    }
    ~LINEDESC()
    {
      SAFE_DELETE(pBuffer);
      nNumLines = 0;
      aPos.clear();
    }
  };
  enum LineOffset
  {
    LO_Begin,
    LO_End,
  };
private:
  u32 ReverseFindNewLine(u32 uStart);
  u32 FindNewLine       (u32 uStart);
  u32 Copy              (u8* pData, u32 uBegin, u32 uEnd); // 内部函数, uBegin和uEnd 不能超出范围
public:
  u32 WriteW    (GXLPCWSTR szText, u32 length);
  u32 GetLines  (u32 uLineCount, LineOffset eOffset, LINEDESC* pLineDesc);
};


class GXConsole
{
public:
  GXConsole();
public:
  virtual GXLRESULT Render(GXCanvas* pCanvas);
  virtual GXLRESULT Scroll(int nLine);
  virtual GXLRESULT Show(GXBOOL bShow);
  virtual GXLRESULT WriteW(GXLPCWSTR szFormat, ...);
  virtual GXLRESULT WriteA(GXLPCSTR szFormat, ...);
public:
  GXLRESULT Initialize(GXGraphics* pGraphics);
  GXLRESULT Finalize();
protected:
private:
  GXGraphics*       m_pGraphics;
  GXFont*           m_pFont;
  const int         s_nFontHeight;
  TextRoundBuffer*  m_pBuffer;
private:
  void DrawLines(GXCanvas* pCanvas, int y, int yInc, const TextRoundBuffer::LINEDESC* pLineDesc);
};





#endif // _GRAP_X_COLSOLE_H_