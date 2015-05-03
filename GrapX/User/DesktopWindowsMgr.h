#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_
#define _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_

#define DWM_GS_DESKTOP    (-2)
#define DWM_GS_ACTIVEWND  (-1)

// DWM ��������/���÷ǻ����ʹ�õĽṹ��
struct DWM_ACTIVEWINDOWS
{
  LPGXWND lpInactiveWnd;    // ��Ҫ���ڷǻ�Ĵ���
  LPGXWND lpInsertWnd;      // �����λ��,���뵽���Wnd֮��, ע��GX��Win32�Ĵ��������෴��
  LPGXWND lpActiveWnd;      // ��Ҫ���ڻ�Ĵ���
  LPGXWND lpGenFirst;       // ��ʼ������ʾ����Ĵ���
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
  GXBOOL            Render              (GXCanvas* pCanvas);
  GXWindowsSurface* GetSurface          (GXINT nSurface);
  void              ActiveWindows       (GXINT uActiveState, DWM_ACTIVEWINDOWS* pActiveWindows);
  GXBOOL            ActiveSurface       (GXWindowsSurface* pWndSurface);

  GXHRESULT         ManageWindowSurface (GXHWND hWnd, GXUINT message);
  GXHRESULT         AllocSurface        (GXHWND hWnd);
  GXHRESULT         FreeSurface         (GXWindowsSurface* pWinsSurface);
private:

  //GXINT             FindSurface         (GXWindowsSurface* pWinsSurface);
  GXLPSTATION       m_lpStation;
  GXWindowsSurface* m_pDesktopWindows;    // ͨ�õĴ��ڲ�
  GXWindowsSurface* m_pActiveWndSur;      // ���Windows����
  WinsSurfaceArray  m_aExclusiveSurface;  // ��ռ�Ĵ��ڲ�, ����WS_EX_LAYERED��־�������ϲ�Ĵ���
  GXDWORD           m_dwFlags;
};

//GXBOOL DesktopWindowsMgr::InvalidateWndRegion(
//  GXHWND hWnd, const GRegion* prgnUpdate, GXBOOL bWndCoord);
//����: 
// hWnd      ָ����Ҫ��Ч�Ĵ���, ���ΪNULL, ������д��ڱ����Ч����
// prgnUpdate  �����Ч����, ��� hWnd �� NULL, ����� prgnUpdate �ʹ�����������ȷ����Ч����
//        ��� hWnd �� NULL, ������������ཻ�����д��ڶ�����һ�����������Ϊ��Ч��
// bWndCoord  ˵�� prgnUpdate �� hWnd ����ϵ������Ļ����ϵ�ı�־, ��� hWnd Ϊ NULL ����
//        prgnUpdate Ϊ NULL �������������. bWndCoord Ϊ TRUE ʱ˵�� prgnUpdate ʹ�õ���
//        ��������ϵ.

#endif // _GRAP_X_DESKTOP_WINDOWS_MANAGER_H_
#endif // _DEV_DISABLE_UI_CODE