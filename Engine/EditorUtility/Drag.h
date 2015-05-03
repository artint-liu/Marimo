#ifndef _EDITOR_UTILITY_DRAG_H_
#define _EDITOR_UTILITY_DRAG_H_

namespace EditorUtility
{
  // TODO:�ɵ������
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

  // ��һ��RECT���ɲ������RECT�İ˸��������
  void GAMEENGINE_API GenerateRectHandles (GXLPCRECT lprc, int nHandleSize, GXLPREGN rgHandles);

  // ��RectHandles�ϵ���������, û���򷵻�-1
  int  GAMEENGINE_API HitTestRectHandles  (GXLPCREGN rgHandles, GXLPCPOINT pt);

  //// �������RectHandles������
  //int  GAMEENGINE_API DragRectHandles     ();

  //////////////////////////////////////////////////////////////////////////

  // �ص���ʽ���϶�����
  // ptCursor �ǽ����϶�״̬ʱ����λ��
  // ptOrigin ���û��������Ҫ�϶������λ��
  // ptDelta ���϶�������ƫ�ƣ����ƫ���������ptCursor�ģ����������һ�λص���λ��
  typedef GXBOOL (GXCALLBACK *DragProc)(GXLPCPOINT ptDelta, GXLPCPOINT ptOrigin, GXLPARAM lParam);
  GXLRESULT GAMEENGINE_API TrackDragAction(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin, DragProc lpDragFunc, GXLPARAM lParam);

  // �ӿ���ʽ���϶�����
  class GAMEENGINE_DLL CDrag
  {
  public:
    GXLRESULT Track(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin);
    virtual GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin) = NULL;  // ptAbsoluteDelta�ǵ�ǰCursorλ����Track��ʼʱCursorλ��֮��
  };

  // Lambda���ʽ���϶�
  template<class _Fn>
  GXLRESULT TrackDragAction(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin, _Fn DragFunc)
  {
    class DragImpl : public CDrag {
      _Fn func;
      GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin) {
        return func(ptAbsoluteDelta, ptOrigin);
      }
    public:
      DragImpl(_Fn f) : func(f){}
    }drag(DragFunc);
    return drag.Track(ptOrigin, ptOrigin);
  }
} // namespace EditorUtility

#endif // #ifndef _EDITOR_UTILITY_DRAG_H_