#ifndef _GRAP_X_WINDOWS_SURFACE_H_
#define _GRAP_X_WINDOWS_SURFACE_H_

//////////////////////////////////////////////////////////////////////////
//
// 引用的最底层头文件
//
////#include <Include/GUnknown.h>
//#include <vector>

//////////////////////////////////////////////////////////////////////////
//
// 类声明
//
class GXImage;
class GTexture;
class GRegion;
enum RGNCOMPLEX;

// GXWindowsSurface::SetExclusiveWnd 标志
#define SEW_DONOTBLT    0x00000001

//////////////////////////////////////////////////////////////////////////
//
// 类实现
//
class GXWindowsSurface : public GUnknown
{
  friend class DesktopWindowsMgr;
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef();
  virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE


  GXWindowsSurface  (GXLPSTATION lpStation, GXHWND hWnd);
  ~GXWindowsSurface  ();

  RGNCOMPLEX      InvalidateRegion    (const GRegion* pRegion);
  RGNCOMPLEX      InvalidateRect      (GXRECT* lpRect);
  RGNCOMPLEX      ValidateBlank       ();
  GXBOOL          Scroll              (int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate);
  GXBOOL          BitBltRect          (GXWindowsSurface* pSrcSurface, int xDest, int yDest, LPGXCRECT lprcSource);
  GXBOOL          BitBltRegion        (GXWindowsSurface* pSrcSurface, int xDest, int yDest, GRegion* lprgnSource);
  int             GenerateWindowsRgn  (GXBOOL bDelay); // 收集这个Surface上所有窗口的并集
  GXHWND          GetExclusiveWnd     ();
  GXHWND          SetExclusiveWnd     (GXHWND hWnd, GXDWORD dwFlags);

  GXBOOL          SaveToFileW         (GXLPCWSTR lpFilename, GXLPCSTR lpFormat);

  GXLPSTATION     m_lpStation;
  GXImage*        m_pRenderTar;       // 原点对应 Windows 的原点
  GTexture*       m_pDepthStencil;    // D3DFMT_D24S8
  GXRECT          rcScrUpdate;        // GX屏幕空间的坐标，原点对应 Station 屏幕原点

  GXHWND          m_hExclusiveWnd;    // 只有独占Surface的窗口才有这个值
  GRegion*        m_prgnWindows;      // TODO: 以后窗口区域储存在这个里面, 不再把整张m_pRenderTar画出来了
  GRegion*        m_prgnUpdate;
private:
  GXDWORD         m_bGenerateWindowsRgn : 1;
  GXDWORD         m_bLayered : 1;


#ifdef ENABLE_DYNMAIC_EFFECT
  CKinematicGrid    *  pKGrid;
#endif
};


#endif // _GRAP_X_WINDOWS_SURFACE_H_