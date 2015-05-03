#ifndef _EDITOR_UTILITY_AXIS_H_
#define _EDITOR_UTILITY_AXIS_H_

namespace EditorUtility
{
  class GAMEENGINE_API TranslationAxis : public TransformAxis
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
    };
  protected:
    AxisHitTest m_eHit;
    GVGeometry* m_pHighlight;
    AxisHitTest m_eHighlight;
    GXBOOL      m_bTracking;
    GVNode*     m_pBind;
    ESpace      m_eSpace;
    TransformAxisFunction* m_pCallback;
  protected:
    TranslationAxis() 
      : m_eHit(HT_NONE)
      , m_pBind(NULL)
      , m_eHighlight(HT_NONE)
      , m_pHighlight(NULL)
      , m_bTracking(FALSE)
      , m_eSpace(S_RELATIVE)
      , m_pCallback(NULL)
      //, m_eSpace(S_ABSOLUTE)
    {
    }
    virtual ~TranslationAxis();
    GXBOOL  InitializeRefPlane  (GXGraphics* pGraphics);
    GVNode* Hit                 (GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT);
    void    SetHighlight        (GXBOOL bHighlight);
    void    AlignBinder         ();
    void    InvokeTranslation   (const float3& vPos);
  public:
    GVNode* BindNode    (GVNode* pNode);
    GXBOOL  HitTest     (GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit);
    GXBOOL  Track       (GXCanvas3D* pCanvas, GXLPCPOINT ptHit);
    GXVOID  SetFunction (TransformAxisFunction* pFunction);
    void    SetSpace    (ESpace eSpace);
    inline AxisHitTest  GetHitTest() const;
    inline void         ClearHitTest();
    virtual GXBOOL      Update        (const GVSCENEUPDATE& sContext);
  public:
    static GXHRESULT CreateAxis(GXGraphics* pGraphics, TranslationAxis** ppAxis);
  };

  inline TranslationAxis::AxisHitTest TranslationAxis::GetHitTest() const
  {
    return m_eHit;
  }

  inline void TranslationAxis::ClearHitTest()
  {
    m_eHit = HT_NONE;
  }

} // namespace EditorUtility
#endif // _EDITOR_UTILITY_AXIS_H_