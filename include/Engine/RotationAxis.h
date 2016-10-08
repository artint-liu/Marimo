#ifndef _EDITOR_UTILITY_ROTATION_AXIS_H_
#define _EDITOR_UTILITY_ROTATION_AXIS_H_

namespace EditorUtility
{
  class GAMEENGINE_API RotationAxis : public TransformAxis
  {
  public:
    enum AxisHitTest{
      HT_NONE,
      HT_AXIS_X,  // 绕x轴旋转
      HT_AXIS_Y,
      HT_AXIS_Z,
      //HT_PLANE_YZ,
      //HT_PLANE_ZX,
      //HT_PLANE_XY,
    };
  protected:
    AxisHitTest m_eHit;
    //AxisHitTest m_eHighlight;
    GVGeometry* m_pHighlight;
    GVNode*     m_pBind;
    int         m_nSegments;
    GXBOOL      m_bTracking;
    TransformAxisFunction* m_pCallback;
  protected:
    RotationAxis() 
      : m_eHit(HT_NONE)
      //, m_eHighlight(HT_NONE)
      , m_pHighlight(NULL)
      , m_pBind(NULL)
      , m_nSegments(0)
      , m_bTracking(FALSE)
      , m_pCallback(NULL)
    {
    }
    virtual ~RotationAxis();
    GXBOOL  InitializeRefPlane  (GXGraphics* pGraphics, int nSegments);
    GVNode* Hit                 (GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT);
    float   ProjectLength       (GXCanvas3D* pCanvas, const float3& vPos, const float3& vDir, GXLPCPOINT ptBegin, GXLPCPOINT ptEnd);
    void    AlignNodeTransform  ();    
    void    SetArcY             (float fStart, float fStop);
    void    SetHighlight        (GXBOOL bHighlight);
    void    AlignBinder         ();
    void    InvokeRotation      (const float3& vRotation);
  public:
    GVNode* BindNode    (GVNode* pNode);
    //GXBOOL  HitTest     (GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT);
    GXBOOL  HitTest     (GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit);
    GXBOOL  Track       (GXCanvas3D* pCanvas, GXLPCPOINT ptHit);
    GXVOID  SetFunction (TransformAxisFunction* pFunction);
    //GXBOOL  MoveDist    (GXCanvas3D* pCanvas, GXLPCPOINT ptBegin, GXLPCPOINT ptEnd, float3* vDist);
    inline AxisHitTest  GetHitTest() const;
    inline void         ClearHitTest();
    virtual GXBOOL      Update        (const GVSCENEUPDATE& sContext);
  public:
    static GXHRESULT CreateAxis(GXGraphics* pGraphics, RotationAxis** ppAxis);
  };

  inline RotationAxis::AxisHitTest RotationAxis::GetHitTest() const
  {
    return m_eHit;
  }

  inline void RotationAxis::ClearHitTest()
  {
    m_eHit = HT_NONE;
  }

} // namespace EditorUtility
#endif // _EDITOR_UTILITY_ROTATION_AXIS_H_