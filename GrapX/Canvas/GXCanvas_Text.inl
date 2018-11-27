//// 全局头文件
//#include <GrapX.h>
//#include <User/GrapX.Hxx>// 标准接口
//
//// 标准接口
//#include <Include/GResource.h>
//#include <Include/GXCanvas.h>
//#include <Include/GXFont.h>
//
//// 私有头文件
//#include <clstd/Utility.h>
//#include <clstd/floatx.h>
//#include <Driver/Shader/VertexDecl.h>
//#include <Canvas\GXCanvasImpl.h>

// 参考亚洲语言断行规则
// http://msdn.microsoft.com/zh-cn/goglobal/bb688158.aspx

//extern CD3DGraphics *  g_pGraphics;
//static 
//  BEGIN_RENDERSTATE_BLOCK(s_TextRSEx)
//  RENDERSTATE_BLOCK(D3DRS_SEPARATEALPHABLENDENABLE, TRUE)
//  RENDERSTATE_BLOCK(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA)
//  RENDERSTATE_BLOCK(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA)
//  END_RENDERSTATE_BLOCK
//
//static
//  BEGIN_RENDERSTATE_BLOCK(s_NormalRSEx)
//  RENDERSTATE_BLOCK(D3DRS_ALPHABLENDENABLE, TRUE)
//  RENDERSTATE_BLOCK(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA)
//  RENDERSTATE_BLOCK(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA)
//  RENDERSTATE_BLOCK(D3DRS_SEPARATEALPHABLENDENABLE, FALSE)
//  END_RENDERSTATE_BLOCK
//
//static
//  BEGIN_RENDERSTATE_BLOCK(s_FTTextRSEx)
//  RENDERSTATE_BLOCK(D3DRS_ALPHABLENDENABLE, TRUE)
//  RENDERSTATE_BLOCK(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA)
//  RENDERSTATE_BLOCK(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA)
//  RENDERSTATE_BLOCK(D3DRS_SEPARATEALPHABLENDENABLE, TRUE)
//  RENDERSTATE_BLOCK(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE)
//  RENDERSTATE_BLOCK(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE)
//  END_RENDERSTATE_BLOCK
//////////////////////////////////////////////////////////////////////////
#define JPN_BEGIN  0x3041  // (含) 其中不全是日文
#define JPN_END    0x3129
#define CHS_BEGIN  0x4E00  // (含)
#define CHS_END    0x9FA5  // (含)
#define KOR_BEGIN  0xAC00
#define KOR_END    0xD7A3

#define CHAR_SPACE  0x20

// 中文后继字符 - 简体中文
static GXWCHAR chMustBeBack[] =
{
  0x0021, // !
  0x0029, // )
  0x002C, // ,
  0x002E, // .
  0x003A, // :
  0x003B, // ;
  0x003F, // ?
  0x005D, // ]
  0x007D, // }
  0x00A8, //
  0x00B7, // ·

  0x02C7, //
  0x02C9, // 
  0x2014, // —
  0x2016, // ‖
  0x2019, // 
  0x201D, // 
  0x2026, // 
  0x2236, // ∶
  0x3001, // 、
  0x3002, // 。
  0x3003, // 〃
  0x3005, // 々
  0x3009, // 〉
  0x300B, // 》
  0x300D, // 」
  0x300F, // 』
  0x3011, // 】
  0x3015, // 〕
  0x3017, // 〗

  0xFF01, // ！
  0xFF02, // ＂
  0xFF07, // ＇
  0xFF09, // ）
  0xFF0C, // ，
  0xFF0E, // ．
  0xFF1A, // ：
  0xFF1B, // ；
  0xFF1F, // ？
  0xFF3D, // ］
  0xFF40, // ｀
  0xFF5C, // ｜
  0xFF5D, // ｝
  0xFF5E, // ～
};
#define IS_CHS(CH)  (CH >= CHS_BEGIN && CH <= CHS_END)
#define IS_ENG(CH)  ((CH >= L'A' && CH <= L'Z') || (CH >= L'a' && CH <= L'z'))

GXBOOL CanBreak(GXWCHAR pch, GXWCHAR ch);

//////////////////////////////////////////////////////////////////////////

  GXINT CanvasImpl::TextOutDirect(const INTMEASURESTRING* p, GXLPPOINT pptPosition)
  {
    ASSERT(p->pFont != NULL);
    GXFont::CHARDESC CharDesc;
    //INTMEASURESTRING m;

    GXPOINT  ptPen;
    GXINT    i = 0;
    GXINT    nTabIdx = 0;

    //m.pFont     = pFTFont;
    //m.lpString  = lpString;
    //m.cString   = cbString;
    //m.uFormat   = uFormat;
    //m.nTabPositions = 0;
    //m.lpnTabStopPositions = NULL;

    ptPen.x = pptPosition->x;
    ptPen.y = pptPosition->y;

    while(i < p->cString)
    {
      if((p->uFormat & GXDT_EXPANDTABS) && p->lpString[i] == L'\t')
      {
        const GXLONG nTabWidth = p->pFont->GetWidth() << 2;
        //ptPen.x = ((ptPen.x - pptPosition->x - m_xOrigin + nTabWidth) / nTabWidth)
        //  * nTabWidth + m_xOrigin + pptPosition->x;
        ptPen.x = LocalizeTabPos(p, ptPen.x - pptPosition->x - m_xOrigin, nTabWidth, &nTabIdx)
          + m_xOrigin + pptPosition->x;
      }
      else
      {
        if((p->uFormat & GXDT_NOPREFIX) || p->lpString[i] != L'&')
        {
          p->pFont->QueryCharDesc(p->lpString[i], &CharDesc);
          DrawTexture(CharDesc.pTex, ptPen.x + CharDesc.ptDest.x, ptPen.y + CharDesc.ptDest.y, &CharDesc.rgSrc);
          ptPen.x += CharDesc.nAdvWidth;
        }
        else
        {
          i++;
          if(i >= p->cString) {
            break;
          }
          GXREGN rgDest;
          p->pFont->QueryCharDesc(p->lpString[i], &CharDesc);
          DrawTexture(CharDesc.pTex, ptPen.x + CharDesc.ptDest.x, ptPen.y + CharDesc.ptDest.y, &CharDesc.rgSrc);
          rgDest.left   = ptPen.x + CharDesc.ptDest.x;
          rgDest.width  = CharDesc.rgSrc.width;
          p->pFont->QueryCharDesc(L'_', &CharDesc);
          rgDest.top    = ptPen.y + CharDesc.ptDest.y;
          rgDest.height = CharDesc.rgSrc.height;
          DrawTexture(CharDesc.pTex, &rgDest, &CharDesc.rgSrc);
          ptPen.x += rgDest.width;
        }
      }

      if(ptPen.x + m_CallState.xOrigin>= m_CallState.rcClip.right) {
        break;
      }

      i++;
    }
    return ptPen.x;
  }

  // 定位Tab位置
  GXINT CanvasImpl::LocalizeTabPos(const INTMEASURESTRING* p, int nCurPos, int nDefaultTabWidth, int* pLastIndex)
  {
    if(p->nTabPositions != 0 && p->lpnTabStopPositions != NULL) {
      for(int i = *pLastIndex; i < p->nTabPositions; i++) {
        if(nCurPos < p->lpnTabStopPositions[i]) {
          *pLastIndex = i + 1;
          return p->lpnTabStopPositions[i];
        }
      }
    }
    return (nCurPos + nDefaultTabWidth) / nDefaultTabWidth * nDefaultTabWidth;
  }

  //////////////////////////////////////////////////////////////////////////
  // 获取单行文本的宽度
  // 输入参数：cbString 必须是一个确切的字符串长度
  // 返回值：字符串的宽度
  GXINT CanvasImpl::MeasureStringWidth_SL(const INTMEASURESTRING* p)
  {
    ASSERT(p->pFont != NULL);
    GXINT  i = 0;
    GXINT  nWidth = 0;
    GXINT  nTabIdx = 0;

    while(i < p->cString)
    {
      if((p->uFormat & GXDT_EXPANDTABS) && p->lpString[i] == L'\t')
      {
        const GXLONG nTabWidth = p->pFont->GetWidth() << 3;
        //nWidth = ((nWidth + nTabWidth) / nTabWidth) * nTabWidth;
        nWidth = LocalizeTabPos(p, nWidth, nTabWidth, &nTabIdx);
      }
      else
      {
        if((p->uFormat & GXDT_NOPREFIX) == 0 && p->lpString[i] == L'&')
        {
          i++;
          if(i >= p->cString)
            break;
        }
        nWidth += p->pFont->QueryCharWidth(p->lpString[i]);
      }
      i++;
    }
    return nWidth;
  }
  //////////////////////////////////////////////////////////////////////////
  // 获取字符串宽度，如果遇到回车换行或者字符串结尾'\0'函数返回
  // 输入参数：cbString 是字符串长度，扫描过程会依照字符串结尾'\0'或者回车换行符号和字符串长度这四个条件判定，
  // 满足任意条件后都会返回。
  // 输出：pEnd 返回字符串结尾的索引，这个值应该指向'\0''\r''\n'或者等于cbString
  // 返回值：这段长度字符串的宽度
  GXINT CanvasImpl::MeasureStringWidth_RN(const INTMEASURESTRING* p, GXINT* pEnd)
  {
    ASSERT(p->pFont != NULL);
    GXINT  i = 0;
    GXINT  nWidth = 0;
    GXINT  nTabIdx = 0;

    while(i != p->cString)
    {
      GXWCHAR ch = p->lpString[i];
      if(ch == L'\0' || ch == L'\r' || ch == L'\n')
        break;
      if((p->uFormat & GXDT_EXPANDTABS) && ch == L'\t')
      {
        const GXLONG nTabWidth = p->pFont->GetWidth() << 3;
        nWidth = LocalizeTabPos(p, nWidth, nTabWidth, &nTabIdx);
        //nWidth = ((nWidth + nTabWidth) / nTabWidth) * nTabWidth;
      }
      else
      {
        if((p->uFormat & GXDT_NOPREFIX) == 0 && ch == L'&')
        {
          i++;
          if(i == p->cString)
            break;
        }
        nWidth += p->pFont->QueryCharWidth(ch);
      }
      i++;
    }
    *pEnd = i;
    return nWidth;
  }
  //////////////////////////////////////////////////////////////////////////
  GXBOOL CanBreak(GXWCHAR pch, GXWCHAR ch)
  {
    return (ch == CHAR_SPACE || (!IS_CHS(pch) && !IS_ENG(pch)) || (IS_CHS(pch) && ch != 0x0CFF/*chinese ','*/ && ch != 0x0230/*chinese '.'*/));
  }

  GXINT CanvasImpl::MeasureStringWidth_WB(const INTMEASURESTRING* p, GXINT nWidthLimit, GXINT* pEnd)
  {
    ASSERT(p->pFont != NULL);
    struct CONTENT
    {
      GXINT i;
      GXINT nWidth;
    }Break, Curr;

    Break.i = 0;
    Break.nWidth = 0;
    Curr.i = 0;
    Curr.nWidth = 0;
    //GXINT  i = 0;
    //GXINT  nWidth = 0;
    //GXINT  iCanBreak = 0;
    //GXINT  i
    GXWCHAR ch, pch = L'\0';
    GXINT nTabIdx = 0;

    while(Curr.i != p->cString)
    {
      ch = p->lpString[Curr.i];
      if(ch == L'\0' || ch == L'\r' || ch == L'\n')
        goto RET_IMMT;
      else if(CanBreak(pch, ch))
      {
        Break.i = Curr.i;
        Break.nWidth = Curr.nWidth;
      }
      if((p->uFormat & GXDT_EXPANDTABS) && ch == L'\t')
      {
        const GXLONG nTabWidth = p->pFont->GetWidth() << 3;
        //Curr.nWidth = ((Curr.nWidth + nTabWidth) / nTabWidth) * nTabWidth;
        Curr.nWidth = LocalizeTabPos(p, Curr.nWidth, nTabWidth, &nTabIdx);
      }
      else
      {
        if((p->uFormat & GXDT_NOPREFIX) == 0 && ch == L'&')
        {
          Curr.i++;
          if(Curr.i == p->cString)
            goto RET_CURR;
        }
        GXINT nCharWidth = p->pFont->QueryCharWidth(ch);
        Curr.nWidth += nCharWidth;
        if(Curr.nWidth > nWidthLimit)
        {
          // 这里 Curr.nWidth 已经超出宽度，但是，一般情况下不会返回这个值
          // 除非之前的扫描没有找到可断行位置。
          //Curr.nWidth -= nCharWidth;
          //if(Curr.nWidth > nWidthLimit)  // ??
          //  Curr.i--;
          break;
        }
      }
      pch = ch;
      Curr.i++;
    }
    if(Break.i > 0 && Curr.nWidth > nWidthLimit)
    {
      if(p->lpString[Break.i] == CHAR_SPACE)
        *pEnd = Break.i;
      else
        *pEnd = Break.i - 1;
      return Break.nWidth;
    }
RET_CURR:
    if(p->lpString[Curr.i + 1] == CHAR_SPACE && Curr.i != p->cString)
      Curr.i++;
RET_IMMT:
    *pEnd = Curr.i;
    return Curr.nWidth;
  }

  //////////////////////////////////////////////////////////////////////////
  //
  //GXDT_SINGLELINE  Displays text on a single line only. Carriage returns and linefeeds do not break the line.
  //  GXDT_BOTTOM  Justifies the text to the bottom of the rectangle. This value must be combined with GXDT_SINGLELINE.
  //  GXDT_TOP    Top-justifies text (single line only).
  //  GXDT_VCENTER  Centers text vertically (single line only).

  //GXDT_NOCLIP    Draws without clipping. DrawText is somewhat faster when GXDT_NOCLIP is used.
  //GXDT_CALCRECT  Determines the width and height of the rectangle. If there are multiple lines of text, 
  //        DrawText uses the width of the rectangle pointed to by the lpRect parameter and extends 
  //        the base of the rectangle to bound the last line of text. If there is only one line of text, 
  //        DrawText modifies the right side of the rectangle so that it bounds the last character in the 
  //        line. In either case, DrawText returns the height of the formatted text but does not draw the text.

  //GXDT_CENTER    Centers text horizontally in the rectangle.
  //GXDT_LEFT    Aligns text to the left.
  //GXDT_RIGHT    Aligns text to the right.

  //GXDT_WORDBREAK  Breaks words. Lines are automatically broken between words if a word would extend past the 
  //        edge of the rectangle specified by the lpRect parameter. A carriage return-linefeed sequence 
  //        also breaks the line.
  //
  //GXDT_EXPANDTABS  Expands tab characters. The default number of characters per tab is eight.
  //GXDT_NOPREFIX  Turns off processing of prefix characters. Normally, DrawText interprets the mnemonic-prefix 
  //        character & as a directive to underscore the character that follows, and the mnemonic-prefix 
  //        characters && as a directive to print a single &. By specifying GXDT_NOPREFIX, this processing 
  //        is turned off.
  //GXDT_TABSTOP  Sets tab stops. Bits 15-8 (high-order byte of the low-order word) of the uFormat parameter 
  //        specify the number of characters for each tab. The default number of characters per tab is eight.
  //
  //(暂不支持)GXDT_EDITCONTROL  Duplicates the text-displaying characteristics of a multiline edit control. Specifically, the average character width is calculated in the same manner as for an edit control, and the function does not display a partially visible last line.
  //(暂不支持)GXDT_END_ELLIPSIS or GXDT_PATH_ELLIPSIS  Replaces part of the given string with ellipses, if necessary, so that the result fits in the specified  rectangle. The given string is not modified unless the GXDT_MODIFYSTRING flag is specified.You can specify GXDT_END_ELLIPSIS to replace characters at the end of the string, or GXDT_PATH_ELLIPSIS to replace characters in the middle of the string. If the string contains backslash (\) characters, GXDT_PATH_ELLIPSIS preserves as much as possible of the text after the last backslash.
  //(暂不支持)GXDT_EXTERNALLEADING  Includes the font external leading in line height. Normally, external leading is not included in the height of a line of text.
  //(暂不支持)GXDT_MODIFYSTRING  Modifies the given string to match the displayed text. This flag has no effect unless the GXDT_END_ELLIPSIS or GXDT_PATH_ELLIPSIS flag is specified.
  //(暂不支持)GXDT_RTLREADING  Layout in right to left reading order for bi-directional text when the font selected into the hdc is a Hebrew or Arabic font. The default reading order for all text is left to right.
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  GXINT CanvasImpl::DrawText_SingleLine(const INTMEASURESTRING* p, GXLPRECT lpRect)
  {
    ASSERT(p->uFormat & GXDT_SINGLELINE);
    ASSERT(lpRect != NULL);
    // 支持的格式：
    // GXDT_BOTTOM
    // GXDT_TOP
    // GXDT_VCENTER

    // GXDT_CENTER
    // GXDT_LEFT
    // GXDT_RIGHT

    // GXDT_EXPANDTABS
    // GXDT_NOPREFIX

    // GXDT_NOCLIP
    // GXDT_CALCRECT - 这个参数无视 GXDT_NOCLIP 最后结果会与 lpRect(left top) 偏移

    GXSIZE sizeExtent;
    //GXINT nStrLength = (p->nCount == -1) ? GXSTRLEN(p->lpString) : p->nCount;
    INTMEASURESTRING m = *p;
    m.cString = (p->cString == -1) ? (GXINT)GXSTRLEN(p->lpString) : p->cString;

    // 只计算字符串尺寸
    if(p->uFormat & GXDT_CALCRECT)
    {
      //sizeExtent.cx = MeasureStringWidth_SL(p->pFont, p->lpString, nStrLength, p->uFormat);
      sizeExtent.cx = MeasureStringWidth_SL(&m);
      sizeExtent.cy = p->pFont->GetMetricsHeight();
      lpRect->right  = lpRect->left + sizeExtent.cx;
      lpRect->bottom = lpRect->top  + sizeExtent.cy;
      return sizeExtent.cy;
    }

    // 如果设置裁剪失败则返回
    if((p->uFormat & GXDT_NOCLIP) == 0 && SetParametersInfo(CPI_SETTEXTCLIP, 0, lpRect) == 0) {
      return 0L;
    }

    //INTMEASURESTRING m = *p;
    sizeExtent.cy = p->pFont->GetMetricsHeight();

    // 文字只显示在左上角, 则直接绘制文字。
    if((p->uFormat & (GXDT_BOTTOM | GXDT_VCENTER | GXDT_CENTER | GXDT_RIGHT)) == 0)
    {
      m.uFormat = p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX);
      //TextOutDirect(p->pFont, (GXLPPOINT)&lpRect->left, p->lpString, m.cString, p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX));
      TextOutDirect(&m, (GXLPPOINT)&lpRect->left);

      if((p->uFormat & GXDT_NOCLIP) == 0) {
        SetParametersInfo(CPI_SETTEXTCLIP, 0, NULL);
      }

      //lpRect->bottom = lpRect->top + sizeExtent.cy;
      return p->pFont->GetMetricsHeight();
    }

    // 其他格式...
    GXPOINT ptPos = {lpRect->left, lpRect->top};

    //sizeExtent.cx = MeasureStringWidth_SL(p->pFont, p->lpString, nStrLength, p->uFormat);
    sizeExtent.cx = MeasureStringWidth_SL(&m);

    if(p->uFormat & GXDT_VCENTER) {
      ptPos.y = (lpRect->bottom + lpRect->top - sizeExtent.cy) >> 1;
    }
    else if(p->uFormat & GXDT_BOTTOM) {
      ptPos.y = lpRect->bottom - sizeExtent.cy;
    }

    if(p->uFormat & GXDT_CENTER) {
      ptPos.x = (lpRect->right + lpRect->left - sizeExtent.cx) >> 1;
    }
    else if(p->uFormat & GXDT_RIGHT) {
      ptPos.x = lpRect->right - sizeExtent.cx;
    }

    m.uFormat = p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX);
    //TextOutDirect(p->pFont, &ptPos, p->lpString, m.cString, p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX));
    TextOutDirect(&m, &ptPos);

    if((p->uFormat & GXDT_NOCLIP) == 0) {
      SetParametersInfo(CPI_SETTEXTCLIP, 0, NULL);
    }

    //lpRect->right = lpRect->left + (lpRect->right - ptPos.x);
    //lpRect->bottom = lpRect->top + sizeExtent.cy;
    return sizeExtent.cy;
  }
  //////////////////////////////////////////////////////////////////////////
  GXINT CanvasImpl::DrawText_Normal(const INTMEASURESTRING* p, GXLPRECT lpRect)
  {
    // GXDT_EXPANDTABS
    // GXDT_NOPREFIX

    // GXDT_NOCLIP
    // GXDT_CALCRECT - 这个参数无视 GXDT_NOCLIP 最后结果会与 lpRect(left top) 偏移
    GXUINT  nStart = 0;
    //GXINT  nStringCount = p->cString;
    GXINT   nEnd;
    GXPOINT ptPos = {lpRect->left, lpRect->top};
    GXINT   nMaxWidth = 0;
    INTMEASURESTRING m = *p;
    INTMEASURESTRING m2 = *p;
    //m.cString = p->cString;

    // 如果设置裁剪失败则返回
    if((p->uFormat & (GXDT_NOCLIP | GXDT_CALCRECT)) == 0 && SetParametersInfo(CPI_SETTEXTCLIP, 0, lpRect) == 0) {
      return 0L;
    }

    while(1) 
    {
      //GXINT nStrWidth = MeasureStringWidth_RN(p->pFont, &p->lpString[nStart], nStringCount, p->uFormat, &nEnd);
      m.lpString = &p->lpString[nStart];
      GXINT nStrWidth = MeasureStringWidth_RN(&m, &nEnd);
      
      if(p->uFormat & GXDT_CENTER) {
        ptPos.x = (lpRect->right + lpRect->left - nStrWidth) >> 1;
      }
      else if(p->uFormat & GXDT_RIGHT) {
        ptPos.x = lpRect->right - nStrWidth;
      }

      if(p->uFormat & GXDT_CALCRECT) {
        nMaxWidth = clMax(nMaxWidth, nStrWidth);
      } else {
        //TextOutDirect(p->pFont, &ptPos, &p->lpString[nStart], nEnd, p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX));
        m2.lpString = &p->lpString[nStart];
        m2.cString = nEnd;
        m2.uFormat = p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX);

        TextOutDirect(&m2, &ptPos);
      }

      ptPos.y += p->pFont->GetMetricsHeight();
      
      if(p->lpString[nStart + nEnd] == '\0' ||
        (m.cString > 0 && m.cString - (nEnd + 1) <= 0) ) {
          break;
      }

      m.cString -= (nEnd + 1);
      nStart += (nEnd + 1);
    };
    
    if(p->uFormat & GXDT_CALCRECT)
    {
      lpRect->right  = lpRect->left + nMaxWidth;
      lpRect->bottom = lpRect->top  + ptPos.y;
    }

    if((p->uFormat & (GXDT_NOCLIP | GXDT_CALCRECT)) == 0) {
      SetParametersInfo(CPI_SETTEXTCLIP, 0, NULL);
    }
    return ptPos.y;
  }
  //////////////////////////////////////////////////////////////////////////
  // 备注：目前 WordBreak 方式与Win32的不完全一样，已知的差异是在狭长区域
  //     Win32的DrawText仍然不会分割把后继字符与之前的字符断行。而我写的会
  //     把后继字符另起新行
  GXINT CanvasImpl::DrawText_WordBreak(const INTMEASURESTRING* p, GXLPRECT lpRect)
  {
    // GXDT_EXPANDTABS
    // GXDT_NOPREFIX

    // GXDT_NOCLIP
    // GXDT_CALCRECT - 这个参数无视 GXDT_NOCLIP 最后结果会与 lpRect(left top) 偏移
    GXUINT  nStart = 0;
    //GXINT  nStringCount = p->cString;
    GXINT   nEnd;
    GXPOINT ptPos = {lpRect->left, lpRect->top};
    GXINT   nMaxWidth = 0;
    INTMEASURESTRING m = *p;
    INTMEASURESTRING m2 = *p;
    //m.cString = p->cString;

    // 如果设置裁剪失败则返回
    if((p->uFormat & (GXDT_NOCLIP | GXDT_CALCRECT)) == 0 && SetParametersInfo(CPI_SETTEXTCLIP, 0, lpRect) == 0) {
      return 0L;
    }

    while(1) 
    {
      //GXINT nStrWidth = MeasureStringWidth_WB(p->pFont, &p->lpString[nStart], nStringCount, p->uFormat, lpRect->right - lpRect->left, &nEnd);
      m.lpString = &p->lpString[nStart]; // TODO: 不用重新赋值，直接使用循环后面的“+=”
      GXINT nStrWidth = MeasureStringWidth_WB(&m, lpRect->right - lpRect->left, &nEnd);

      if(p->uFormat & GXDT_CENTER) {
        ptPos.x = (lpRect->right + lpRect->left - nStrWidth) >> 1;
      }
      else if(p->uFormat & GXDT_RIGHT) {
        ptPos.x = lpRect->right - nStrWidth;
      }

      GXBOOL bBreakLoop = p->lpString[nStart + nEnd] == '\0' ||
        (m.cString > 0 && m.cString - (nEnd + 1) <= 0);

      if(p->uFormat & GXDT_CALCRECT) {
        nMaxWidth = clMax(nMaxWidth, nStrWidth);
      } else {
        //TextOutDirect(p->pFont, &ptPos, &p->lpString[nStart], nEnd + (1 ^ bBreakLoop), p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX));
        m2.lpString = &p->lpString[nStart];
        m2.cString  = nEnd + (1 ^ bBreakLoop);
        m2.uFormat  = p->uFormat & (GXDT_EXPANDTABS | GXDT_NOPREFIX);
        TextOutDirect(&m2, &ptPos);
      }

      ptPos.y += p->pFont->GetMetricsHeight();
       
      if(bBreakLoop) {
        break;
      }

      m.cString -= (nEnd + 1);
      nStart += (nEnd + 1);
    };

    if(p->uFormat & GXDT_CALCRECT)
    {
      lpRect->right  = lpRect->left + nMaxWidth;
      lpRect->bottom = lpRect->top  + ptPos.y;
    }

    if((p->uFormat & (GXDT_NOCLIP | GXDT_CALCRECT)) == 0)
      SetParametersInfo(CPI_SETTEXTCLIP, 0, NULL);

    return ptPos.y;
  }
  //////////////////////////////////////////////////////////////////////////
  GXINT CanvasImpl::DrawText(GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText)
  {
    clStringW str;
    if(nCount == -1)
      str.Append(lpString);
    else if(nCount > 0)
      str.Append(lpString, nCount);
    else
      return FALSE;

    return DrawText(pFTFont, str, (int)str.GetLength(), lpRect, uFormat, crText);
  }

  GXINT CanvasImpl::DrawText(GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText)
  {
    GXINT nRet;
    GXINT eLastMode = 0;
    if((uFormat & GXDT_CALCRECT) == 0)
    {
      ASSERT(pFTFont != NULL);
      ASSERT(TEST_FLAG(uFormat, GXDT_NOCLIP) || lpRect->right > lpRect->left);
      ASSERT(TEST_FLAG(uFormat, GXDT_NOCLIP) || lpRect->bottom > lpRect->top);

      SetParametersInfo(CPI_SETTEXTURECOLOR, crText, NULL);
      SetParametersInfo(CPI_SETCOLORADDITIVE, 0x00ffffff, NULL);
      eLastMode = SetCompositingMode(CM_SourceOver);
    }
    INTMEASURESTRING m;
    m.pFont    = pFTFont;
    m.lpString = lpString;
    m.cString  = nCount;
    m.uFormat  = uFormat;
    m.nTabPositions = 0;
    m.lpnTabStopPositions = NULL;
       
    
    if(uFormat & GXDT_SINGLELINE) {
      nRet = DrawText_SingleLine(&m, lpRect);
    }
    else if(uFormat & GXDT_WORDBREAK) {
      nRet = DrawText_WordBreak(&m, lpRect);
    }
    else {
      nRet = DrawText_Normal(&m, lpRect);
    }

    if((uFormat & GXDT_CALCRECT) == 0)
    {
      SetParametersInfo(CPI_SETTEXTURECOLOR, 0xffffffff, NULL);
      SetParametersInfo(CPI_SETCOLORADDITIVE, 0, NULL);

      if(eLastMode != CM_SourceOver)
        SetCompositingMode((CompositingMode)eLastMode);
    }
    return nRet;
  }


  //////////////////////////////////////////////////////////////////////////
  GXBOOL CanvasImpl::TextOut(GXFont* pFTFont, GXINT nXStart,GXINT nYStart,GXLPCSTR lpString,GXINT cbString, GXCOLORREF crText)
  {
    clStringW str;
    if(cbString == -1) {
      str.Append(lpString);
    }
    else if(cbString > 0) { // 指定有限长度
      str.Append(lpString, cbString);
    }
    else {
      return FALSE;
    }

    return TextOut(pFTFont, nXStart, nYStart, str, (int)str.GetLength(), crText);
  }

  GXBOOL CanvasImpl::TextOut(GXFont* pFTFont, GXINT nXStart,GXINT nYStart,GXLPCWSTR lpString,GXINT cbString, GXCOLORREF crText)
  {
    GXPOINT ptPos = {nXStart, nYStart};
    SetParametersInfo(CPI_SETTEXTURECOLOR, crText, NULL);
    SetParametersInfo(CPI_SETCOLORADDITIVE, 0x00ffffff, NULL);
    INTMEASURESTRING m;
    m.pFont = pFTFont;
    m.lpString = lpString;
    m.cString = cbString;
    m.uFormat = GXDT_NOPREFIX;
    m.nTabPositions = 0;
    m.lpnTabStopPositions = NULL;
    //TextOutDirect(pFTFont, &ptPos, lpString, cbString, GXDT_NOPREFIX);
    TextOutDirect(&m, &ptPos);
    SetParametersInfo(CPI_SETTEXTURECOLOR, 0xffffffff, NULL);
    SetParametersInfo(CPI_SETCOLORADDITIVE, 0, NULL);
    return (GXBOOL)pFTFont->GetMetricsHeight();
  }

  GXLONG CanvasImpl::TabbedTextOut(GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText)
  {
    clStringW str;
    if(nCount == -1) {
      str.Append(lpString);
    }
    else if(nCount > 0) { // 指定有限长度
      str.Append(lpString, nCount);
    }
    else {
      return FALSE;
    }

    return TabbedTextOut(pFTFont, x, y, str, (int)str.GetLength(), nTabPositions, lpTabStopPositions, crText);
  }

  GXLONG CanvasImpl::TabbedTextOut(GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText)
  {
    GXRECT rect(x, y, 0, 0);
    //GXPOINT ptPos = {x, y};
    INTMEASURESTRING m;
    m.pFont = pFTFont;
    m.lpString = lpString;
    m.cString = nCount;
    m.uFormat = GXDT_NOPREFIX | GXDT_NOCLIP | GXDT_SINGLELINE | GXDT_EXPANDTABS;
    m.nTabPositions = nTabPositions;
    m.lpnTabStopPositions = lpTabStopPositions;

    if((crText & 0xff000000) == 0)
    {
      m.uFormat |= GXDT_CALCRECT;
      DrawText_SingleLine(&m, &rect);
    }
    else
    {
      SetParametersInfo(CPI_SETTEXTURECOLOR, crText, NULL);
      SetParametersInfo(CPI_SETCOLORADDITIVE, 0x00ffffff, NULL);
      //TextOutDirect(pFTFont, &ptPos, lpString, cbString, GXDT_NOPREFIX);
      //TextOutDirect(&m, &ptPos);
      DrawText_SingleLine(&m, &rect);

      SetParametersInfo(CPI_SETTEXTURECOLOR, 0xffffffff, NULL);
      SetParametersInfo(CPI_SETCOLORADDITIVE, 0, NULL);
    }
    //return (GXBOOL)pFTFont->GetMetricsHeight();
    return GXMAKELPARAM(rect.right, rect.bottom);
  }
