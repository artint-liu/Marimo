#ifndef _EDITOR_UTILITY_SCALING_AXIS_H_
#define _EDITOR_UTILITY_SCALING_AXIS_H_

namespace EditorUtility
{
  class GAMEENGINE_API ScalingAxis : public TransformAxis
  {
  public:
    enum AxisHitTest{
      HT_NONE,
      HT_AXIS_X,
      HT_AXIS_Y,
      HT_AXIS_Z,
      HT_PLANE_YZ,
      HT_PLANE_ZX,
      HT_PLANE_XY,
      HT_ENTIRETY,
    };
  protected:
    AxisHitTest m_eHit;
    GVGeometry* m_pHighlight;
    AxisHitTest m_eHighlight;
    GXBOOL      m_bTracking;
    GVNode*     m_pBind;
    float3      m_vTrackedScaling;
    TransformAxisFunction* m_pCallback;
  protected:
    ScalingAxis() 
      : m_eHit(HT_NONE)
      , m_pBind(NULL)
      , m_eHighlight(HT_NONE)
      , m_pHighlight(NULL)
      , m_bTracking(FALSE)
      , m_vTrackedScaling(1.0f)
      , m_pCallback(NULL)
      //, m_eSpace(S_ABSOLUTE)
    {
    }
    virtual ~ScalingAxis();
    GXBOOL  InitializeRefPlane  (GXGraphics* pGraphics);
    GVNode* Hit                 (GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT);
    void    SetHighlight        (GXBOOL bHighlight);
    void    AlignBinder         ();
    void    InvokeScaling       (float3& vScaling);
  public:
    GVNode* BindNode    (GVNode* pNode);
    GXBOOL  HitTest     (GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit);
    GXBOOL  Track       (GXCanvas3D* pCanvas, GXLPCPOINT ptHit);
    GXVOID  SetFunction (TransformAxisFunction* pFunction);
    inline AxisHitTest  GetHitTest() const;
    inline void         ClearHitTest();
    virtual GXBOOL      Update        (const GVSCENEUPDATE& sContext);
  public:
    static GXHRESULT CreateAxis(GXGraphics* pGraphics, ScalingAxis** ppAxis);
  };

  inline ScalingAxis::AxisHitTest ScalingAxis::GetHitTest() const
  {
    return m_eHit;
  }

  inline void ScalingAxis::ClearHitTest()
  {
    m_eHit = HT_NONE;
  }

} // namespace EditorUtility
#endif // _EDITOR_UTILITY_SCALING_AXIS_H_