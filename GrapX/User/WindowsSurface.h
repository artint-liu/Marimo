#ifndef _GRAP_X_WINDOWS_SURFACE_H_
#define _GRAP_X_WINDOWS_SURFACE_H_

//////////////////////////////////////////////////////////////////////////
//
// ���õ���ײ�ͷ�ļ�
//
////#include <Include/GUnknown.H>
//#include <vector>

//////////////////////////////////////////////////////////////////////////
//
// ������
//
class GXImage;
class GTexture;
class GRegion;
enum RGNCOMPLEX;

// GXWindowsSurface::SetExclusiveWnd ��־
#define SEW_DONOTBLT    0x00000001

//////////////////////////////////////////////////////////////////////////
//
// ��ʵ��
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
  int             GenerateWindowsRgn  (GXBOOL bDelay); // �ռ����Surface�����д��ڵĲ���
  GXHWND          GetExclusiveWnd     ();
  GXHWND          SetExclusiveWnd     (GXHWND hWnd, GXDWORD dwFlags);

  GXBOOL          SaveToFileW         (GXLPCWSTR lpFilename, GXLPCSTR lpFormat);

  GXLPSTATION     m_lpStation;
  GXImage*        m_pRenderTar;       // ԭ���Ӧ Windows ��ԭ��
  GTexture*       m_pDepthStencil;    // D3DFMT_D24S8
  GXRECT          rcScrUpdate;        // GX��Ļ�ռ�����꣬ԭ���Ӧ Station ��Ļԭ��

  GXHWND          m_hExclusiveWnd;    // ֻ�ж�ռSurface�Ĵ��ڲ������ֵ
  GRegion*        m_prgnWindows;      // TODO: �Ժ󴰿����򴢴����������, ���ٰ�����m_pRenderTar��������
  GRegion*        m_prgnUpdate;
private:
  GXDWORD         m_bGenerateWindowsRgn : 1;
  GXDWORD         m_bLayered : 1;


#ifdef ENABLE_DYNMAIC_EFFECT
  CKinematicGrid    *  pKGrid;
#endif
};


#endif // _GRAP_X_WINDOWS_SURFACE_H_