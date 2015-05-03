#ifndef _IMPLEMENT_GX_FREE_TYPE_FONT_H_
#define _IMPLEMENT_GX_FREE_TYPE_FONT_H_

//#include <hash_map>

class GTexture;
class GXGraphics;
namespace D3D9{
  class GXGraphicsImpl;
};
namespace D3D11{
  class GXGraphicsImpl;
};
namespace WOGL{
  class GXGraphicsImpl;
};
namespace GLES2{
  class GXGraphicsImpl;
};

class _GFTFont : public GXFont
{
  friend class D3D9::GXGraphicsImpl;
  friend class D3D11::GXGraphicsImpl;
  friend class WOGL::GXGraphicsImpl;
  friend class GLES2::GXGraphicsImpl;
private:
  typedef clvector<GTexture*>          GTextureArray;
  GXGraphics*     m_pGraphics;
  FT_Library      m_Library;
  FT_Face         m_Face;
  //GXCHAR*         m_pFaceName;
  clStringA       m_strFaceName;    // ����������ɹ���Ϊ��
  GXULONG         m_nWidth;
  GXULONG         m_nHeight;
  GTextureArray   m_aFontTex;
  GXPOINT         m_ptPen;
  GXLOGFONTA      m_LogFont;

  typedef struct __tagFTFONTDESC
  {
    unsigned char  idxTex;    // �������������
    unsigned short xGrid;     // ��ģ��λ�� ��������
    unsigned short yGrid;     // ��ģ��λ��
         short xPos;          // ��Χ���Ŀ�ʼλ��
         short yPos;
    unsigned short nWidth;    // ����ַ��Ŀ��
    unsigned short nHeight;   // �߶�
    unsigned short nAdvWidth; // ���ӿ��
  }FTFONTDESC, *LPFTFONTDESC;

  typedef clhash_map<short, FTFONTDESC>  CharToFontDesc;
  CharToFontDesc*        m_pmapCode2FMatrix;

  _GFTFont  (GXGraphics* pGraphics, GXCONST GXLPLOGFONTA lpLogFont);
  virtual ~_GFTFont();

  inline GXVOID FontDescToCharDesc    (const FTFONTDESC &fm, LPCHARDESC lpCD);
  GXBOOL    _CreateTexture            ();
  GXVOID    UpdateTexBuffer           (GXUINT idxTex, LPGXREGN prgDest, unsigned char* pBuffer);
public:
  virtual GXBOOL    CreateFont        (GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXCONST GXCHAR *pFileName);
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef            ();
  virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXHRESULT Invoke            (GRESCRIPTDESC* pDesc);

  virtual GXBOOL    GetDescW          (GXLPLOGFONTW lpLogFont) const;
  virtual GXBOOL    GetDescA          (GXLPLOGFONTA lpLogFont) const;

  virtual GXLPVOID  GetTexture        (GXUINT idx) const;
  virtual GXINT     QueryCharWidth    (GXWCHAR ch);
  virtual GXBOOL    QueryCharDescFromCache  (GXWCHAR ch, LPCHARDESC pCC);
  virtual GXBOOL    QueryCharDesc     (GXWCHAR ch, LPCHARDESC pCC);
  virtual GXLONG    GetMetricsHeight  () const;    // ���ж���ĸ߶�
  virtual GXLONG    GetWidth          () const;    // ��������Ŀ��
  virtual GXLONG    GetHeight         () const;    // ��������ĸ߶�
  virtual GXBOOL    GetMetricW        (GXLPTEXTMETRICW lptm) const;

public:
  friend GXLRESULT GXDLLAPI GXCreateFreeTypeFontIndirectW(GXLPLOGFONTW lpLogFont, GXFont **ppFont);
  friend GXLRESULT GXDLLAPI GXCreateFreeTypeFontIndirectA(GXLPLOGFONTA lpLogFont, GXFont **ppFont);
  friend GXLRESULT GXDLLAPI GXCreateFreeTypeFontW(GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXLPCWSTR pFileName, GXFont **ppFont);
  friend GXLRESULT GXDLLAPI GXCreateFreeTypeFontA(GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXLPCSTR pFileName, GXFont **ppFont);
};


#endif // _IMPLEMENT_GX_FREE_TYPE_FONT_H_