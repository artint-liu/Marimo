#ifndef _MARIMO_GRAPHICS_EFFECT_X_H_
#define _MARIMO_GRAPHICS_EFFECT_X_H_

namespace GFX
{
  typedef clstd::_vector2<int> int2;
  typedef clstd::_vector3<int> int3;

  struct RANGEI
  {
    int _min;
    int _max;
    RANGEI(int a, int b)
      : _min(a), _max(b) {}
  };

  struct RANGEF
  {
    float _min;
    float _max;
    RANGEF(float a, float b)
      : _min(a), _max(b) {}
  };

  class Rand : public clstd::Rand
  {
  public:
    float3& RandVector(float3& vDest)
    {
      return vDest.set(randf2(), randf2(), randf2());
    }
    int RandRangeI(const RANGEI& r)
    {
      if(r._max == r._min) {
        return (rand() % r._min) + 1;
      }
      return (rand() % (r._max - r._min)) + r._min;
    }

    float RandRangeF(const RANGEF& r)
    {
      return (randf() * (r._max - r._min)) + r._min;
    }
  };

  enum CommState
  {
    Play,
    Stop,
    Pause,
    Step,
    Freeze,
    //UpdateOnce = 0x8000,  // ��Ҫ����һ�β���ת����ȷ״̬
  };

  enum TypeId
  {
    Empty,
    Patch3D,
    Particles,
  };

  struct PROPERTY
  {
    enum Type
    {
      T_UNKNOWN,
      T_UINT,
      T_INT,
      T_INT2,
      T_INT3,
      T_FLOAT,
      T_FLOAT2,
      T_FLOAT3,
      T_NMFLOAT2,   // mormalized float2
      T_NMFLOAT3,   // mormalized float3
      T_RANGEF,
      T_RANGEI,
      T_STRINGW,
      T_STRINGA,
      T_HALFFLOAT,  // ����Ϊfloat�����0.5
      T_USER,       // �û����Դ����
#ifdef _UNICODE
      T_STRING = T_STRINGW,
#else
      T_STRING = T_STRINGA,
#endif // #ifdef _UNICODE
    };

    struct ENUMLIST
    {
      GXLPCSTR szName;
      GXINT    nEnum;
    };

    union
    {
      INT_PTR  General;
      Type     eType;
      ENUMLIST*aEnum;
    };
    GXLPCSTR szName;
    GXUINT   Offset;

    template<typename _Ty>
    inline _Ty& ToVar(GXLPCVOID pBase) const
    {
      return *(_Ty*)((GXLPBYTE)pBase + Offset);
    }

    GAMEENGINE_DLL size_t SizeOf();
  };

  class Element : public GVMesh // �����Ժ�Ҫ��GVNode�̳�
  {
  public:
    const static GXDWORD ClassCode = GXMAKEFOURCC('V','G','F','X');
    typedef clvector<GXDefinition>  ParamArray;
  protected:
    struct COMMONPARAMDESC
    {
      clStringW strTexture;
      clStringW strMaterial;
      float3    vPos;
    };
  protected:
    TypeId      m_eTypeId;
    COMMONPARAMDESC m_sCommParam;

  protected:
    GXBOOL  ImportCommonParam (const GXDEFINITION& sDef, COMMONPARAMDESC& sCommParam);
    GXUINT  ExportCommonParam (GXDefinition* aDef, GXUINT nCount, COMMONPARAMDESC& sCommParam);
    GXBOOL  CreateCommonRes   (GXGraphics* pGraphics, COMMONPARAMDESC& sCommParam);

  public:
    Element(GXGraphics* pGraphics, TypeId eTypeId)
      : GVMesh(pGraphics, ClassCode)
      , m_eTypeId(eTypeId)
    {
    }

    virtual GXINT   Execute       (GXLPCSTR szCmd, GXWPARAM wParam, GXLPARAM lParam);
    GAMEENGINE_DLL
    static  GXBOOL  ImportParam   (GXLPVOID pBasePtr, const GXDEFINITION& sDefine, const PROPERTY* aProps);
    GAMEENGINE_DLL 
    static  GXBOOL  ExportParam   (GXLPCVOID pBasePtr, GXDefinition& sDefine, const PROPERTY& sProps);


    GXSTDINTERFACE(GXBOOL Initialize  (GXGraphics* pGraphics));
    GXSTDINTERFACE(GXBOOL SolveParams (GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount));
    GXSTDINTERFACE(GXUINT MakeParams  (GXDefinition* aDefines, GXUINT nArrayCount));
    GXSTDINTERFACE(GXBOOL Update      (const GVSCENEUPDATE& sContext));
    inline TypeId GetGFXType();
  };

  // Element::SolveParams
  //  ����ӿ�Ҫ���ж��������������������.һ��Elementӵ�����ɲ���,�ӿ��ڲ���Ҫ��֤,��update֮ǰ��ͬ���߳�������ı���һһ���򼸸�
  //  ��������Э���ò����䶯�����Ĺ����ԸĶ�.�ڲ����Ϸ�������±�֤����ȷ������һ����������.

  inline TypeId Element::GetGFXType()
  {
    return m_eTypeId;
  }

  class ElementJar : public Element
  {
  public:
    ElementJar(GXGraphics* pGraphics);

    virtual GXBOOL Initialize  (GXGraphics* pGraphics);
    virtual GXBOOL SolveParams (GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount);
    virtual GXUINT MakeParams  (GXDefinition* aDefines, GXUINT nArrayCount);
    virtual GXBOOL Update      (const GVSCENEUPDATE& sContext);
  };




  GXHRESULT GAMEENGINE_API Create         (Element** ppElement, GXGraphics* pGraphics, TypeId eId, GXDEFINITION* aDefines, GXSIZE_T nCount);
  GXHRESULT GAMEENGINE_API CreateFromFileA(Element** ppElement, GXGraphics* pGraphics, GXLPCSTR szFilename);
  GXHRESULT GAMEENGINE_API CreateFromFileW(Element** ppElement, GXGraphics* pGraphics, GXLPCWSTR szFilename);
} // namespace GFX

//typedef GFX::Element 
#endif // #ifndef _MARIMO_GRAPHICS_EFFECT_X_H_