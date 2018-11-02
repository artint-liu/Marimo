#ifndef _IMPLEMENT_GRAPH_X_IMAGE_H_
#define _IMPLEMENT_GRAPH_X_IMAGE_H_

#if 0

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
  virtual GXHRESULT AddRef            () override;
  virtual GXHRESULT Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT Invoke            (GRESCRIPTDESC* pDesc) override;
  virtual GXImage*  Clone             () const override;
  virtual GXBOOL    GetDesc           (GXBITMAP*lpBitmap) const override;
  //virtual GXINT     GetWidth          () const override;
  //virtual GXINT     GetHeight         () const override;
  virtual GXSIZE    GetDimension      (GXSIZE* pDimension) const override;
  //virtual GXHRESULT SetHelperState    (HelperState eState, GXLPARAM lParam);
  virtual GXBOOL    BitBltRegion      (GXImage* pSource, int xDest, int yDest, GRegion* lprgnSource) override;
  virtual GXBOOL    Scroll            (int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate) override;
  virtual GXHRESULT GetTexture        (GTexture** ppTexture) const override;

  virtual GTexture* GetTextureUnsafe  () override;
  virtual GXBOOL    SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat) override;
private:
  void UpdateDimension();
  void UpdateFlags();
public:

};

#endif // 0
#endif // _IMPLEMENT_GRAPH_X_IMAGE_H_