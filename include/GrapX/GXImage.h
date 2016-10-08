#ifndef _GRAPH_X_IMAGE_H_
#define _GRAPH_X_IMAGE_H_

class GTexture;
class GRegion;

enum HelperState
{
  HS_Create,    // 创建Helper对象
  HS_Release,   // 释放
  HS_Hold,      // 保存主纹理对象内容
  HS_Fetch,     // 将Helper内容更新到主纹理
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
  GXSTDINTERFACE(GXHRESULT GetTexture       (GTexture** ppTexture) const);              // 这个你懂的,用完要释放!

  // =====================================================================================
  GXSTDINTERFACE(GTexture* GetTextureUnsafe ());                          // 用于快速取纹理对象, 这个接口不会增加纹理的引用计数
  GXSTDINTERFACE(GXBOOL    SaveToFileW      (GXLPCWSTR pszFileName, GXLPCSTR pszDestFormat));  // 调试用的函数, 可以将纹理保存到一个图像文件中, 格式可以为"BMP","PNG"或"DDS"等, 忽略大小写

};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_X_CANVAS_H_