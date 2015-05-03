#ifndef _IMPLEMENT_GRAPH_X_IMAGE_H_
#define _IMPLEMENT_GRAPH_X_IMAGE_H_

#define GXIMAGEFLAG_NOTSAMEWITHTEXTURE  0x00000001  // ������ͬ�ߴ�, ����֧��NonPow2������´���

class GXGraphics;
class GXImageImpl : public GXImage
{
private:
  GXGraphics*         m_pGraphics;
  GTexture*           m_pNativeTex;
  GTexture*           m_pHelperTex;

  GXINT               m_nWidth;  // ʵ�ʳߴ�, ���ܻ���ʱ�ߴ���Ļ����ֵ
  GXINT               m_nHeight;
  GXFormat            m_eFormat;
  GXDWORD             m_dwFlags;

protected:
  virtual ~GXImageImpl();
public:
  GXImageImpl (GXGraphics* pGraphics);
  GXImageImpl (GXGraphics* pGraphics, GXLONG nWidth, GXLONG nHeight, GXFormat eFormat);

  GXBOOL Initialize(GTexture*pTexture);
  GXBOOL Initialize(GXBOOL bRenderable, LPGXIMAGEINFOX pSrcFile, GXLPVOID lpBits);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef            ();
  virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT Invoke            (GRESCRIPTDESC* pDesc);
  virtual GXImage*  Clone             () const;
  virtual GXBOOL    GetDesc           (GXBITMAP*lpBitmap) GXCONST;
  virtual GXINT     GetWidth          () GXCONST;
  virtual GXINT     GetHeight         () GXCONST;
  virtual void      GetDimension      (GXINT* pWidth, GXINT* pHeight) GXCONST;
  virtual GXHRESULT SetHelperState    (HelperState eState, GXLPARAM lParam);
  virtual GXBOOL    BitBltRegion      (GXImage* pSource, int xDest, int yDest, GRegion* lprgnSource);
  virtual GXBOOL    Scroll            (int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate);
  virtual GXHRESULT GetTexture        (GTexture** ppTexture) GXCONST;

  virtual GTexture* GetTextureUnsafe  ();
  virtual GXBOOL    SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat);
private:
  void UpdateDimension();
  void UpdateFlags();
public:

};

#endif // _IMPLEMENT_GRAPH_X_IMAGE_H_