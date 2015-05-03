#ifndef _MARIMO_GRAPHICS_EFFECT_PATCH3D_H_
#define _MARIMO_GRAPHICS_EFFECT_PATCH3D_H_
namespace GFX
{
  class Patch3DImpl : public Element
  {
  public:
    enum Align
    {
      TowardCamera, // ���������, UpΪ����ռ�����, ������Up��������������
      Billboard,    // ���������, UpΪ������ռ�
    };
  protected:
    static
      PROPERTY::ENUMLIST s_aEnumAlign[];
    static
      PROPERTY    s_aProperty[];

    float       m_fHalfWidth;
    float       m_fHalfHeight;
    float3      m_vUp;  // ���ϵķ�������
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