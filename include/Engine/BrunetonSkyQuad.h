#ifndef _BRUNETON_SKY_QUAD_H_
#define _BRUNETON_SKY_QUAD_H_

namespace Scene
{
//  namespace Internal
//  {
//    const float JULIANDATE = 150;
//    const float LONGITUDE = 0.0f;
//    const float LATITUDE = 0.0f;
//    const float MERIDIAN = 0.0f;
//
//    class CSun : public Marimo::DataPoolMonitor
//    {
//    public:
//
//      CSun(float fTheta = 0.0f, float fPhi = 0.0f, float fIntensity = 1.0f);
//      ~CSun();
//
//      CFloat3&  GetDirection  ();
//      float     GetIntensity  ();
//      CFloat3&  GetColor      ();
//
//      void  SetPosition     (float fTheta, float fPhi);
//      void  SetPosition     (float fTime);
//      void  SetIntensity    (float fIntensity);
//      void  SetTurbidity    (float fTurbidity);
//
//      void ComputeAttenuation ();
//      void SetDataPool        (MODataPool* pDataPool);
//
//      GXHRESULT AddRef              ();
//      GXHRESULT Release             ();
//      clStringA GetClassName        ();
//      GXHRESULT RegisterIdentify    (GXLPVOID pIndentify);
//      GXHRESULT UnregisterIdentify  (GXLPVOID pIndentify);
//      GXHRESULT OnKnock             (Marimo::KNOCKACTION* pKnock);
//
//    private:
//      MODataPool* m_pDataPool;
//#ifdef _ENABLE_DATAPOOL
//      MOVarFloat  m_fVarSunTime;
//      MOVarFloat  m_fVarTheta;
//      MOVarFloat  m_fVarPhi;
//      MOVarFloat  m_fVarIntensity;
//      MOVarFloat3 m_vVarDirection;
//      MOVarFloat  m_fVarTurbidity;
//      MOVarFloat3 m_vVarColor;
//#else
//      float   m_fTheta;
//      float   m_fPhi;
//      float   m_fIntensity;
//      float3  m_vDirection;
//      float   m_fTurbidity;
//      float3  m_vColor;
//#endif // #ifdef _ENABLE_DATAPOOL
//    }; // class CSun
//
//    class CAtmosphere : public Marimo::DataPoolMonitor
//    {
//      friend class SkyQuad;
//    private:
//      void CalculateRayleighCoeff();
//      void CalculateMieCoeff();
//
//    public:
//      CAtmosphere();
//      ~CAtmosphere();
//      CFloat3&  GetBetaRay              ();
//      CFloat3&  GetBetaRayTheta         ();
//      CFloat3&  GetBetaMie              ();
//      CFloat3&  GetBetaMieTheta         ();
//      float     GetDirectionalityFactor ();
//
//      void      SetRayFactor            (float fRayFactor);
//      void      SetMieFactor            (float fMieFactor);
//      void      SetTurbidity            (float fTurbidity);
//      void      SetDirectionalityFactor (float fDirectionalityFactor);
//      void      SetDataPool             (MODataPool* pDataPool);
//
//      GXHRESULT AddRef                  ();
//      GXHRESULT Release                 ();
//      clStringA GetClassName            ();
//      GXHRESULT RegisterIdentify        (GXLPVOID pIndentify);
//      GXHRESULT UnregisterIdentify      (GXLPVOID pIndentify);
//      GXHRESULT OnKnock                 (Marimo::KNOCKACTION* pKnock);
//    private:
//      MODataPool* m_pDataPool;
//      //Scattering coefficients.
//#ifdef _ENABLE_DATAPOOL
//      MOVarFloat3  m_vVarBetaRay;
//      MOVarFloat3  m_vVarBetaRayTheta;
//      MOVarFloat3  m_vVarBetaMie;
//      MOVarFloat3  m_vVarBetaMieTheta;
//
//      MOVarFloat   m_fVarRayFactor;
//      MOVarFloat   m_fVarMieFactor;
//
//      MOVarFloat   m_fVarTurbidity;
//      MOVarFloat   m_fVarDirectionalityFactor;
//#else
//      float3  m_vBetaRay;
//      float3  m_vBetaRayTheta;
//      float3  m_vBetaMie;
//      float3  m_vBetaMieTheta;
//
//      float   m_fRayFactor;
//      float   m_fMieFactor;
//
//      float   m_fTurbidity;
//      float   m_fDirectionalityFactor;
//#endif // #ifdef _ENABLE_DATAPOOL
//
//    }; // class CAtmosphere
//  } // Internal

  class GAMEENGINE_API BrunetonSkyQuad : public GVGeometry
  {
    //Internal::CAtmosphere m_Atmosphere;
    //Internal::CSun        m_Sun;
    //MODataPool*           m_pDataPool;
  public:
    BrunetonSkyQuad();// : m_pDataPool(NULL){ m_Sun.AddRef(); }
    virtual ~BrunetonSkyQuad();
    //CFloat3&  GetDirection            ();
    //float     GetIntensity            ();
    //CFloat3&  GetColor                ();
    //CFloat3&  GetBetaRay              ();
    //CFloat3&  GetBetaRayTheta         ();
    //CFloat3&  GetBetaMie              ();
    //CFloat3&  GetBetaMieTheta         ();
    //float     GetDirectionalityFactor ();
  public:
    static GXHRESULT Create(GXGraphics* pGraphics, BrunetonSkyQuad** ppSkyQuad);
  };
} // namespace Scene

#endif // _SKY_QUAD_H_