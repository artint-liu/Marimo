#ifndef _EDITOR_UTILITY_TRANSFORM_AXIS_H_
#define _EDITOR_UTILITY_TRANSFORM_AXIS_H_

namespace EditorUtility
{
  class TransformAxisFunction;

  class TransformAxis : public GVGeometry
  {
  public:
    virtual GVNode* BindNode    (GVNode* pNode) = 0;
    virtual GXBOOL  HitTest     (GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit) = 0;
    virtual GXBOOL  Track       (GXCanvas3D* pCanvas, GXLPCPOINT ptHit) = 0;
    virtual GXVOID  SetFunction (TransformAxisFunction* pFunction) = 0;
    static  GXColor32 GetAxisColor(char cAxis);
  };

  class TransformAxisFunction
  {
  public:
    virtual GXVOID Scale      (GVNode* pBinder, const float3& v) = 0;
    virtual GXVOID RotateR    (GVNode* pBinder, const float3& v) = 0;
    virtual GXVOID Translate  (GVNode* pBinder, GVNode::ESpace eSpace, const float3& v) = 0;
  };
} // namespace EditorUtility

#endif // _EDITOR_UTILITY_TRANSFORM_AXIS_H_