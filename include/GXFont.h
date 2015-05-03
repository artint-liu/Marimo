#ifndef _GRAP_X_FONT_H_
#define _GRAP_X_FONT_H_

class GTexture;
class GXFont : public GResource
{
public:
  typedef struct __tagCHARDESC  // �ַ�����
  {
    GTexture*   pTex;  // ���ڵ�����
    REGN        rgSrc;  // �������������
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
  GXSTDINTERFACE(GXLONG   GetMetricsHeight       () const);    // ���ж���ĸ߶�
  GXSTDINTERFACE(GXLONG   GetWidth               () const);    // ��������Ŀ��
  GXSTDINTERFACE(GXLONG   GetHeight              () const);    // ��������ĸ߶�
  GXSTDINTERFACE(GXBOOL   GetMetricW             (GXLPTEXTMETRICW lptm) const);
};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAP_X_FONT_H_