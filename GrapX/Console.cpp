#include "GrapX.h"
//#include "GrapX/GUnknown.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GResource.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXFont.H"
#include "Console.h"
#include "clfifo.h"

roundbuffer::roundbuffer()
  : m_pBuffer(NULL)
  , m_uCapacity(0)
  , m_pLocker(NULL)
  , m_bOverwrite(0)
  , m_uBegin(0)
  , m_uEnd(0)
{
}

roundbuffer::~roundbuffer()
{
  SAFE_DELETE_ARRAY(m_pBuffer);
  SAFE_DELETE(m_pLocker);
}

b32 roundbuffer::Initialize(u32 size, b32 bOverwrite)
{
  ASSERT(bOverwrite == 0);
  if (size & (size - 1)) {   
    if(size > 0x80000000) {
      return FALSE;
    }
    size = clstd::RoundupPowOfTwo(size);   
  }
  m_uCapacity = size;
  
  m_pBuffer = new u8[m_uCapacity];
  m_pLocker = new clstd::Locker;

  if(m_pBuffer == NULL || m_pLocker == NULL) {
    SAFE_DELETE_ARRAY(m_pBuffer);
    SAFE_DELETE(m_pLocker);
    return FALSE;
  }

  return TRUE;
}

u32 roundbuffer::Write(u8* pData, u32 uLength)
{
  u32 l;
  ASSERT(uLength < m_uCapacity);

  uLength = clMin(uLength, m_uCapacity - m_uEnd + m_uBegin);

  if(m_pLocker) {
    m_pLocker->Lock();
  }

  // first put the data starting from fifo->in to buffer end
  l = clMin(uLength, m_uCapacity - (m_uEnd & (m_uCapacity - 1)));   
  memcpy(m_pBuffer + (m_uEnd & (m_uCapacity - 1)), pData, l);   

  // then put the rest (if any) at the beginning of the buffer
  memcpy(m_pBuffer, pData + l, uLength - l);

  if(m_pLocker) {
    m_pLocker->Unlock();
  }

  m_uEnd += uLength;  
  return uLength;   
}

u32 roundbuffer::Read(u8* pData, u32 uLength)
{
  unsigned int l;   
  uLength = clMin(uLength, m_uEnd - m_uBegin);

  if(m_pLocker) {
    m_pLocker->Lock();
  }

  // first get the data from fifo->out until the end of the buffer
  l = clMin(uLength, m_uCapacity - (m_uBegin & (m_uCapacity - 1)));   
  memcpy(pData, m_pBuffer + (m_uBegin & (m_uCapacity - 1)), l);   

  // then get the rest (if any) from the beginning of the buffer
  memcpy(pData + l, m_pBuffer, uLength - l);   

  if(m_pLocker) {
    m_pLocker->Unlock();
  }

  m_uBegin += uLength;
  return uLength;   
}

void roundbuffer::Lock()
{
  m_pLocker->Lock();
}

void roundbuffer::Unlock()
{
  m_pLocker->Unlock();
}

b32 roundbuffer::TryLock()
{
  return m_pLocker->TryLock();
}
//////////////////////////////////////////////////////////////////////////
u32 TextRoundBuffer::ReverseFindNewLine(u32 uStart)
{
  // TODO: 超过4G时可能会有问题
  for(u32 i = uStart; i > m_uBegin; i -= sizeof(GXWCHAR))
  {
    GXWCHAR& ch = *(GXWCHAR*)&m_pBuffer[i & (m_uCapacity - 1)];
    if(ch == L'\n') {
      return i;
    }
  }
  return m_uBegin;
}

u32 TextRoundBuffer::FindNewLine(u32 uStart)
{
  // TODO: 超过4G时可能会有问题
  for(u32 i = uStart; i < m_uEnd; i += sizeof(GXWCHAR))
  {
    GXWCHAR& ch = *(GXWCHAR*)&m_pBuffer[i & (m_uCapacity - 1)];
    if(ch == L'\n') {
      return i + sizeof(GXWCHAR);
    }
  }
  return m_uEnd;
}

u32 TextRoundBuffer::WriteW(GXLPCWSTR szText, u32 length)
{
  length *= sizeof(GXWCHAR);
  if(m_uEnd - m_uBegin + length > m_uCapacity)
  {
    m_uBegin += length;
    m_uBegin = FindNewLine(m_uBegin);
  }
  return Write((u8*)szText, length);
}

u32 TextRoundBuffer::Copy(u8* pData, u32 uBegin, u32 uEnd)
{
  unsigned int l;
  ASSERT(uBegin >= m_uBegin && uEnd <= m_uEnd);
  u32 uLength = uEnd - uBegin;

  //if(m_pLocker) {
  //  m_pLocker->Lock();
  //}

  // first get the data from fifo->out until the end of the buffer
  l = clMin(uLength, m_uCapacity - (uBegin & (m_uCapacity - 1)));   
  memcpy(pData, m_pBuffer + (uBegin & (m_uCapacity - 1)), l);   

  // then get the rest (if any) from the beginning of the buffer
  memcpy(pData + l, m_pBuffer, uLength - l);   

  //if(m_pLocker) {
  //  m_pLocker->Unlock();
  //}

  return uLength;   
}

u32 TextRoundBuffer::GetLines(u32 uLineCount, LineOffset eOffset, LINEDESC* pLineDesc)
{
  pLineDesc->nNumLines = 0;
  pLineDesc->aPos.clear();
  u32 uStart;
  if(eOffset == LO_End)
  {
    uStart = m_uEnd;

    for(u32 i = 0; i < uLineCount; i++)
    {
      pLineDesc->nNumLines++;
      pLineDesc->aPos.push_back(uStart + sizeof(GXWCHAR));
      uStart -= sizeof(GXWCHAR);

      if((uStart = ReverseFindNewLine(uStart)) == m_uBegin) {
        break;
      }
    }
  }
  else if(eOffset == LO_Begin)
  {
    // FIXME:这个分支还不对
    uStart = m_uBegin;
    for(u32 i = 0; i < uLineCount; i++)
    {
      pLineDesc->aPos.push_back(uStart);
      if((uStart = FindNewLine(uStart)) == m_uEnd) {
        break;
      }
      pLineDesc->nNumLines++;
    }
  }
  else
    return 0;
  pLineDesc->aPos.push_back(uStart);

  const u32 uBegin = clMin(pLineDesc->aPos.front(), pLineDesc->aPos.back());
  const u32 uEnd = clMax(pLineDesc->aPos.front(), pLineDesc->aPos.back());

  for(clvector<int>::iterator it = pLineDesc->aPos.begin();
    it != pLineDesc->aPos.end(); ++it)
  {
    *it = *it - uStart;
  }
  pLineDesc->pBuffer->Resize((uEnd - uBegin), FALSE);
  return Copy((u8*)pLineDesc->pBuffer->GetPtr(), uBegin, uEnd - sizeof(GXWCHAR));
}
//////////////////////////////////////////////////////////////////////////
GXConsole::GXConsole()
  : m_pBuffer(NULL)
  , s_nFontHeight(16)
  , m_pFont(NULL)
{

}
GXLRESULT GXConsole::Initialize(GXGraphics* pGraphics)
{
  m_pGraphics = pGraphics;
  m_pBuffer = new TextRoundBuffer;
  m_pBuffer->Initialize(65536, FALSE);
  return 0;
}

GXLRESULT GXConsole::Finalize()
{
  SAFE_DELETE(m_pBuffer);
  SAFE_RELEASE(m_pFont);
  return 0;
}

void GXConsole::DrawLines(GXCanvas* pCanvas, int y, int yInc, const TextRoundBuffer::LINEDESC* pLineDesc)
{
  GXLPCWSTR lpText = (GXLPCWSTR)pLineDesc->pBuffer->GetPtr();
  int nTop = y;
  int nBack = (int)pLineDesc->aPos.size() - 1;
  for(int i = 0; i < pLineDesc->nNumLines; i++)
  {
    int nLength = abs(pLineDesc->aPos[nBack - (i + 1)] - pLineDesc->aPos[nBack - i]) / sizeof(GXWCHAR) - 1;
    pCanvas->TextOutW(m_pFont, 0, nTop, &lpText[pLineDesc->aPos[nBack - i] / sizeof(GXWCHAR)], 
      nLength, 0xffffffff);
      nTop += yInc;
  }
}

GXLRESULT GXConsole::Show(GXBOOL bShow)
{
  if(m_pFont == NULL) {
    m_pFont = m_pGraphics->CreateFontA(0, s_nFontHeight, "fonts/wqy-microhei.ttc");
  }
  return GX_OK;
}

GXLRESULT GXConsole::Render(GXCanvas* pCanvas)
{
  GXSIZE size;
 
  pCanvas->GetTargetDimension(&size);
  size.cy >>= 1;
  pCanvas->FillRectangle(0, 0, size.cx, size.cy, 0xa0000000);

  TextRoundBuffer::LINEDESC LineDesc;
  m_pBuffer->GetLines(size.cy / s_nFontHeight, TextRoundBuffer::LO_End, &LineDesc);

  DrawLines(pCanvas, size.cy - s_nFontHeight * LineDesc.nNumLines, s_nFontHeight, &LineDesc);
  return 0;
}

GXLRESULT GXConsole::Scroll(int nLine)
{
  return 0;
}

GXLRESULT GXConsole::WriteW(GXLPCWSTR szFormat, ...)
{
  clStringW str;
  va_list    arglist;
  va_start(arglist, szFormat);
  size_t length = str.VarFormat(szFormat, arglist);
  m_pBuffer->WriteW(str, (u32)str.GetLength());
  return length;
}

GXLRESULT GXConsole::WriteA(GXLPCSTR szFormat, ...)
{
  clStringA TextA;
  clStringW TextW;
  va_list    arglist;
  va_start(arglist, szFormat);
  size_t length = TextA.VarFormat(szFormat, arglist);

  TextW = TextA;
  m_pBuffer->WriteW(TextW, (u32)TextW.GetLength());
  return length;
}
