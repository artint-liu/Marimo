#ifndef _MARIMO_GRAPHICS_EFFECT_PARTICLES_H_
#define _MARIMO_GRAPHICS_EFFECT_PARTICLES_H_
namespace GFX
{
  class ParticlesImpl : public Element
  {
  public:
    enum Align
    {
      TowardCamera, // 朝着摄像机, Up为世界空间坐标, 面沿着Up方向尽量面对摄像机
      Billboard,    // 对着摄像机, Up为摄像机空间
    };
  protected:
    struct PARTICLE
    {
      float3  Pos;
      float3  Velocity;
      GXINT   nElapse;
      GXINT   nLifeTime;
      float   fWidthH;    // Half Width
      float   fHeightH;   // Half Height
    };
    typedef clvector<PARTICLE> ParticleArray;
  protected:
    static
      PROPERTY    s_aProperty[];
    Rand          m_Rand;
    float3        m_vGravity;
    ParticleArray m_aParticles;
    RANGEF        m_Speed;
    float         m_fHalfWidth;
    float         m_fHalfHeight;
    float         m_fExtentRange;         // 粒子尺寸变化
    float         m_fAspectRange;         // 粒子尺寸横纵比率变化
    GXINT         m_nElapse;              // 发射器经过的时间
    GXINT         m_nLifeTime;            // 发射器的周期,-1表示不会停止
    RANGEI        m_ParticleLifeTime;
    Align         m_eAlign;
    CommState     m_eState;

  protected:
    void NewOne   (PARTICLE& p);
    void CalcAxis (const GVSCENEUPDATE& sContext, float3& vTop, float3& vRight);

  public:
    ParticlesImpl(GXGraphics* pGraphics);

    virtual GXINT  Execute      (GXLPCSTR szCmd, GXWPARAM wParam, GXLPARAM lParam);

    virtual GXBOOL Initialize   (GXGraphics* pGraphics);
    virtual GXBOOL SolveParams  (GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount);
    virtual GXUINT MakeParams   (GXDefinition* aDefines, GXUINT nArrayCount);
    virtual GXBOOL Update       (const GVSCENEUPDATE& sContext);
  };
} // namespace GFX
#endif // _MARIMO_GRAPHICS_EFFECT_PARTICLES_H_