#ifndef _GRAP_X_FONT_H_
#define _GRAP_X_FONT_H_

class GTexture;
class GXFont : public GResource
{
public:
  typedef struct __tagCHARDESC  // 字符描述
  {
    GTexture*   pTex;  // 所在的纹理
    REGN        rgSrc;  // 所在纹理的区域
    GXPOINT     ptDest;
    GXINT       nAdvWidth;
  }CHARDESC, *LPCHARDESC;

public:
  GXFont() : GResource(1, RESTYPE_FONT){}
  GXSTDINTERFACE(GXBOOL   CreateFont             (GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXCONST GXCHAR *pFileName));
  GXSTDINTERFACE(GXBOOL   GetDescW               (GXLPLOGFONTW lpLogFont) const);
  GXSTDINTERFACE(GXBOOL   GetDescA               (GXLPLOGFONTA lpLogFont) const);
  GXSTDINTERFACE(GXLPVOID GetTexture             (GXUINT idx) const);
  GXSTDINTERFACE(GXBOOL   QueryCharDescFromCache (GXWCHAR ch, LPCHARDESC pCC));
  GXSTDINTERFACE(GXBOOL   QueryCharDesc          (GXWCHAR ch, LPCHARDESC pCC));
  GXSTDINTERFACE(GXINT    QueryCharWidth         (GXWCHAR ch));
  GXSTDINTERFACE(GXLONG   GetMetricsHeight       () const);    // 带有段落的高度
  GXSTDINTERFACE(GXLONG   GetWidth               () const);    // 创建字体的宽度
  GXSTDINTERFACE(GXLONG   GetHeight              () const);    // 创建字体的高度
  GXSTDINTERFACE(GXBOOL   GetMetricW             (GXLPTEXTMETRICW lptm) const);
};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAP_X_FONT_H_