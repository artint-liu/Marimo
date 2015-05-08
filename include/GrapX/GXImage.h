#ifndef _GRAPH_X_IMAGE_H_
#define _GRAPH_X_IMAGE_H_

class GTexture;
class GRegion;

enum HelperState
{
  HS_Create,    // ����Helper����
  HS_Release,   // �ͷ�
  HS_Hold,      // �����������������
  HS_Fetch,     // ��Helper���ݸ��µ�������
};

class GXImage : public GResource
{
public:
  GXImage() : GResource(1, RESTYPE_IMAGE){}

  GXSTDINTERFACE(GXImage*  Clone            () const);
  GXSTDINTERFACE(GXBOOL    GetDesc          (GXBITMAP*lpBitmap) const);
  GXSTDINTERFACE(GXINT     GetWidth         () const);
  GXSTDINTERFACE(GXINT     GetHeight        () const);
  GXSTDINTERFACE(void      GetDimension     (GXINT* pWidth, GXINT* pHeight) const);
  GXSTDINTERFACE(GXHRESULT SetHelperState   (HelperState eState, GXLPARAM lParam));
  GXSTDINTERFACE(GXBOOL    BitBltRegion     (GXImage* pSource, int xDest, int yDest, GRegion* lprgnSource));
  GXSTDINTERFACE(GXBOOL    Scroll           (int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate));
  GXSTDINTERFACE(GXHRESULT GetTexture       (GTexture** ppTexture) const);              // ����㶮��,����Ҫ�ͷ�!

  // =====================================================================================
  GXSTDINTERFACE(GTexture* GetTextureUnsafe ());                          // ���ڿ���ȡ�������, ����ӿڲ���������������ü���
  GXSTDINTERFACE(GXBOOL    SaveToFileW      (GXLPCWSTR pszFileName, GXLPCSTR pszDestFormat));  // �����õĺ���, ���Խ������浽һ��ͼ���ļ���, ��ʽ����Ϊ"BMP","PNG"��"DDS"��, ���Դ�Сд

};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_X_CANVAS_H_