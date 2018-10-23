//====== DRAG LIST CONTROL ====================================================

#ifndef NODRAGLIST

typedef struct tagGXDRAGLISTINFO {
  GXUINT uNotification;
  GXHWND hWnd;
  GXPOINT ptCursor;
} GXDRAGLISTINFO, *GXLPDRAGLISTINFO;

#define GXDL_BEGINDRAG            (GXWM_USER+133)
#define GXDL_DRAGGING             (GXWM_USER+134)
#define GXDL_DROPPED              (GXWM_USER+135)
#define GXDL_CANCELDRAG           (GXWM_USER+136)

#define GXDL_CURSORSET            0
#define GXDL_STOPCURSOR           1
#define GXDL_COPYCURSOR           2
#define GXDL_MOVECURSOR           3

#define GXDRAGLISTMSGSTRING       _CLTEXT("commctrl_DragListMsg")

GXBOOL GXDLLAPI gxMakeDragList(GXHWND hLB);
void GXDLLAPI gxDrawInsert(GXHWND handParent, GXHWND hLB, int nItem);

int GXDLLAPI gxLBItemFromPt(GXHWND hLB, GXPOINT pt, GXBOOL bAutoScroll);

#endif