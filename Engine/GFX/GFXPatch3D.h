#ifndef _MARIMO_GRAPHICS_EFFECT_PATCH3D_H_
#define _MARIMO_GRAPHICS_EFFECT_PATCH3D_H_
namespace GFX
{
  class Patch3DImpl : public Element
  {
  public:
    enum Align
    {
      TowardCamera, // 朝着摄像机, Up为世界空间坐标, 面沿着Up方向尽量面对摄像机
      Billboard,    // 对着摄像机, Up为摄像机空间
    };
  protected:
    static
      PROPERTY::ENUMLIST s_aEnumAlign[];
    static
      PROPERTY    s_aProperty[];

    float       m_fHalfWidth;
    float       m_fHalfHeight;
    float3      m_vUp;  // 向上的方向向量
    Align       m_eAlign;
  protected:
    void CalcAxis(const GVSCENEUPDATE& sContext, float3& vTop, float3& vRight);

  public:
    Patch3DImpl(GXGraphics* pGraphics);

    virtual GXBOOL Initialize   (GXGraphics* pGraphics);
    virtual GXBOOL SolveParams  (GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount);
    virtual GXUINT MakeParams   (GXDefinition* aDefines, GXUINT nArrayCount);
    virtual GXBOOL Update       (const GVSCENEUPDATE& sContext);
  };
} // namespace GFX
#endif // _MARIMO_GRAPHICS_EFFECT_PATCH3D_H_