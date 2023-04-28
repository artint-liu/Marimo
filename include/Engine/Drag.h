#ifndef _EDITOR_UTILITY_DRAG_H_
#define _EDITOR_UTILITY_DRAG_H_

namespace EditorUtility
{
  // TODO:干掉这个！
  //struct DRAG
  //{
  //  GXDWORD	dwStartTime;	//	Start dragging time
  //  GXBOOL  bDrag;			  //	Drag flag
  //  GXPOINT Hit;

  //  DRAG()
  //    : dwStartTime(0)
  //    , bDrag(false)
  //  {
  //    Hit.x		= 0;
  //    Hit.y		= 0;
  //  }
  //  void ButtonDown(GXPOINT point)
  //  {
  //    bDrag = TRUE;
  //    dwStartTime = gxGetTickCount();

  //    Hit.x = point.x;
  //    Hit.y = point.y;
  //  }

  //  void ButtonUp()
  //  {
  //    bDrag = FALSE;
  //  }

  //  inline GXBOOL IsDrag() const
  //  {
  //    return bDrag;
  //  }
  //};

  // Rect Handles
  // 0  1  2
  // 3     4
  // 5  6  7

  // 由一个RECT生成操作这个RECT的八个句柄区域
  void GAMEENGINE_API GenerateRectHandles (GXLPCRECT lprc, int nHandleSize, GXLPREGN rgHandles);

  // 点RectHandles上的悬浮测试, 没有则返回-1
  int  GAMEENGINE_API HitTestRectHandles  (GXLPCREGN rgHandles, GXLPCPOINT pt);

  //// 如果点在RectHandles上悬浮
  //int  GAMEENGINE_API DragRectHandles     ();

  //////////////////////////////////////////////////////////////////////////

  // 回调形式的拖动处理
  // ptCursor 是进入拖动状态时鼠标的位置
  // ptOrigin 是用户定义的需要拖动物体的位置
  // ptDelta 是拖动产生的偏移，这个偏移是相对于ptCursor的，不是相对上一次回调的位置
  typedef GXBOOL (GXCALLBACK *DragProc)(GXLPCPOINT ptDelta, GXLPCPOINT ptOrigin, GXLPARAM lParam);
  GXLRESULT GAMEENGINE_API TrackDragAction(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin, DragProc lpDragFunc, GXLPARAM lParam);

  // 接口形式的拖动处理
  class GAMEENGINE_DLL CDrag
  {
  public:
    GXLRESULT Track(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin);
    virtual GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin) = NULL;  // ptAbsoluteDelta是当前Cursor位置与Track开始时Cursor位置之差
  };

  // Lambda表达式的拖动
  template<class _Fn>
  GXLRESULT TrackDragAction(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin, _Fn DragFunc)
  {
    class DragImpl : public CDrag {
      _Fn func;
      GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin) {
        return func(ptAbsoluteDelta, ptOrigin);
      }
    public:
      DragImpl(_Fn f) : func(f) {}
    }drag(DragFunc);
    return drag.Track(ptCursor, ptOrigin);
  };
} // namespace EditorUtility

#endif // #ifndef _EDITOR_UTILITY_DRAG_H_