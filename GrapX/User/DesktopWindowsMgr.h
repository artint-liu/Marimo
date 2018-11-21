#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_
#define _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_

#define DWM_GS_DESKTOP    (-2)
#define DWM_GS_ACTIVEWND  (-1)

// DWM 用来激活/设置非活动窗口使用的结构体
struct DWM_ACTIVEWINDOWS
{
  LPGXWND lpInactiveWnd;    // 将要处于非活动的窗口
  LPGXWND lpInsertWnd;      // 插入的位置,插入到这个Wnd之后, 注意GX和Win32的窗口链是相反的
  LPGXWND lpActiveWnd;      // 将要处于活动的窗口
  LPGXWND lpGenFirst;       // 开始计算显示区域的窗口
  GRegion* prgnBefore;
  GRegion* prgnAfter;
};

enum DesktopWindowsMgrFlags
{
  GXDWM_AERO  = 0x00000001,
  GXDWM_ALPHA = 0x00000002,
};

class DesktopWindowsMgr
{
  typedef clvector<GXWindowsSurface*>  WinsSurfaceArray;
  typedef WinsSurfaceArray::iterator   WinsSurface_Iterator;
  //typedef cllist<GXWindowsSurface*>   WinsSurfaceList;
  //typedef WinsSurfaceList::iterator   WinsSurface_Iterator;
public:
  DesktopWindowsMgr(GXLPSTATION lpStation);
  virtual ~DesktopWindowsMgr();

  GXBOOL            InvalidateWndRegion (GXHWND hWnd, const GRegion* prgnUpdate, GXBOOL bWndCoord);
  GXBOOL            SendPaintMessage    ();
  GXBOOL            Render              (GrapX::GXCanvas* pCanvas);
  GXWindowsSurface* GetSurface          (GXINT nSurface);
  void              ActiveWindows       (GXINT uActiveState, DWM_ACTIVEWINDOWS* pActiveWindows);
  GXBOOL            ActiveSurface       (GXWindowsSurface* pWndSurface);

  GXHRESULT         ManageWindowSurface (GXHWND hWnd, GXUINT message);
  GXHRESULT         AllocSurface        (GXHWND hWnd);
  GXHRESULT         FreeSurface         (GXWindowsSurface* pWinsSurface);
private:

  //GXINT             FindSurface         (GXWindowsSurface* pWinsSurface);
  GXLPSTATION       m_lpStation;
  GXWindowsSurface* m_pDesktopWindows;    // 通用的窗口层
  GXWindowsSurface* m_pActiveWndSur;      // 活动的Windows表面
  WinsSurfaceArray  m_aExclusiveSurface;  // 独占的窗口层, 具有WS_EX_LAYERED标志或者最上层的窗口
  GXDWORD           m_dwFlags;
};

//GXBOOL DesktopWindowsMgr::InvalidateWndRegion(
//  GXHWND hWnd, const GRegion* prgnUpdate, GXBOOL bWndCoord);
//参数: 
// hWnd      指定需要无效的窗口, 如果为NULL, 则对所有窗口标记无效区域
// prgnUpdate  标记无效区域, 如果 hWnd 非 NULL, 则根据 prgnUpdate 和窗口区域求交来确定无效区域
//        如果 hWnd 是 NULL, 则与这个区域相交的所有窗口都会有一个交集被标记为无效区
// bWndCoord  说明 prgnUpdate 是 hWnd 坐标系还是屏幕坐标系的标志, 如果 hWnd 为 NULL 或者
//        prgnUpdate 为 NULL 则无视这个参数. bWndCoord 为 TRUE 时说明 prgnUpdate 使用的是
//        窗口坐标系.

#endif // _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_
#endif // _DEV_DISABLE_UI_CODE