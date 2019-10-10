// GrapX 3D 绘图对象
// 与GXCanvas不同的是, GXCanvas在绘图时锁定, 用完释放. GXCanvas3D 需要长期持有, 完全不用时才能释放

#ifndef _IMPLEMENT_GRAP_X_CANVAS_3D_H_
#define _IMPLEMENT_GRAP_X_CANVAS_3D_H_

//#define MRT_SUPPORT_COUNT 4
namespace GrapX
{
  class Canvas3DCommImpl : public Canvas3D
  {
  protected:
    Canvas3DCommImpl() : Canvas3D(2, RESTYPE_CANVAS3D) {}
  };
}

//float Canvas3DImpl::GetAspect() const
//{
//  return (float)m_Viewport.regn.w / (float)m_Viewport.regn.h;
//}

//////////////////////////////////////////////////////////////////////////
#endif // _IMPLEMENT_GRAP_X_CANVAS_3D_H_