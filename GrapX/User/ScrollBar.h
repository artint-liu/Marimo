#ifndef _SCROLLBAR_
#define _SCROLLBAR_

#define GX_SCROLLBAR_BUTTON_WIDTH    g_SystemMetrics[GXSM_CXVSCROLL]
#define GX_SCROLLBAR_BUTTON_HEIGHT    g_SystemMetrics[GXSM_CYHSCROLL]
#define GX_SCROLLBAR_MINPAGE_HALF_SIZE  3      
#define GX_SCROLLBAR_MINPAGE_SIZE    (GX_SCROLLBAR_MINPAGE_HALF_SIZE + GX_SCROLLBAR_MINPAGE_HALF_SIZE)


//class GXDLLAPI CScrollBar : public CGXFrame
//{
//  GXSCROLLBAR    m_pScrollBar;
//public:
//  GXHRESULT Paint();
//
//  CScrollBar();
//  CScrollBar(GXDWORD dwExStyle,GXLPWNDCLASSEX lpWndClassEx,GXLPCWSTR lpWindowName,GXDWORD dwStyle,
//    int x,int y,int nWidth,int nHeight,HGXWND hWndParent,HGXMENU hMenu,HINSTANCE hInstance,GXLPVOID lpParam);
//  ~CScrollBar();
//};

typedef struct __tagGXSCROLLBARDRWAINGINFO
{
  GXINT nPageSize;      // »¬¿éµÄÏñËØ³ß´ç
  GXINT nTotalPage;      // Õû¸ö¿Õ°×ÇøÓòµÄÏñËØ³ß´ç
  GXINT nTrackPos;
}GXSCROLLBARDRAWINGINFO, *LPGXSCROLLBARDRAWINGINFO;

GXVOID GXGetScrollBarDrawingInfo(LPGXSCROLLBARDRAWINGINFO, LPGXSCROLLBAR, GXUINT);

#define GXPSB_NORMAL  0x00000000L
#define GXPSB_HOT    0x00000001L
GXBOOL GXPaintHScrollBar(GXWndCanvas*, LPGXSCROLLBAR, GXRECT* ,GXDWORD);
GXBOOL GXPaintVScrollBar(GXWndCanvas*, LPGXSCROLLBAR, GXRECT* ,GXDWORD);

#endif // _SCROLLBAR_