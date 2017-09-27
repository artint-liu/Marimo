#include "GrapX.H"
//#include "clTree.H"
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GShader.H"
#include "GrapX/GXKernel.H"

//#include "3d/gvNode.h"
//#include "3d/gvMesh.h"
//#include "3d/gvGeometry.h"
#include "GrapX/GrapVR.H"
#include "Engine.h"

#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"

#include "Engine/SkyQuad.h"

namespace Scene
{
  namespace Internal
  {
    static GXLPCSTR s_szDataPoolDecl =
    "float   fSunTime;              \n"
    "float   fTheta;                \n"
    "float   fPhi;                  \n"
    "float   fIntensity;            \n"
    "float3  vDirection;            \n"
    "float   fTurbidity;            \n"
    "float3  vColor;                \n" // 貌似没必要放这里
    "//Scattering coefficients.     \n"
    "float3  vBetaRay;              \n"
    "float3  vBetaRayTheta;         \n"
    "float3  vBetaMie;              \n"
    "float3  vBetaMieTheta;         \n"
    "                               \n"
    "float   fRayFactor;            \n"
    "float   fMieFactor;            \n"
    "                               \n"
    "//float   fTurbidity;            \n"
    "float   fDirectionalityFactor; \n";

    //--------------------------------------------------------------------------------------
    // Default constructor.
    //--------------------------------------------------------------------------------------
    CSun::CSun(float fTheta, float fPhi, float fIntensity ) 
      : m_pDataPool(NULL)
#ifdef _ENABLE_DATAPOOL
#else
      , m_fIntensity( fIntensity )
      , m_fTurbidity( 2.0f )
#endif // #ifdef _ENABLE_DATAPOOL
    {
      SetPosition( fTheta, fPhi );
    }

    //--------------------------------------------------------------------------------------
    // Default destructor.
    //--------------------------------------------------------------------------------------
    CSun::~CSun()
    {
#ifdef ENABLE_DATAPOOL_WATCHER
      CLBREAK;
      //m_pDataPool->RemoveWatcher(this);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      SAFE_RELEASE(m_pDataPool);
    }

    //--------------------------------------------------------------------------------------
    // Get sun light direction.
    //--------------------------------------------------------------------------------------
    CFloat3& CSun::GetDirection()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarDirection;
#else
      return m_vDirection;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get sun light intensity.
    //--------------------------------------------------------------------------------------
    float CSun::GetIntensity()
    {
#ifdef _ENABLE_DATAPOOL
      return m_fVarIntensity;
#else
      return (float&)m_fIntensity;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get sun light intensity.
    //--------------------------------------------------------------------------------------
    CFloat3& CSun::GetColor()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarColor;
#else
      return (CFloat3&)m_vColor;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Set sun position.
    //--------------------------------------------------------------------------------------
    void CSun::SetPosition( float fTheta, float fPhi )
    {
#ifdef _ENABLE_DATAPOOL
      if( ! m_fVarTheta.IsValid() || ! m_fVarPhi.IsValid()) {
        return;
      }

      m_fVarTheta = fTheta;
      m_fVarPhi = fPhi;

      float fCosTheta = cosf(m_fVarTheta);
      float fSinTheta = sinf(m_fVarTheta);
      float fCosPhi	  = cosf(m_fVarPhi);
      float fSinPhi	  = sinf(m_fVarPhi);

      if(m_vVarDirection.IsValid())
      {
        m_vVarDirection = float3(fSinTheta * fCosPhi, fCosTheta, fSinTheta * fSinPhi);
        ((float3&)m_vVarDirection).normalize();
      }
#else
      m_fTheta = fTheta;
      m_fPhi = fPhi;

      float fCosTheta = cosf( m_fTheta );
      float fSinTheta = sinf( m_fTheta );
      float fCosPhi	= cosf( m_fPhi );
      float fSinPhi	= sinf( m_fPhi );

      m_vDirection = float3( fSinTheta * fCosPhi,
        fCosTheta, fSinTheta * fSinPhi );
      m_vDirection.normalize();
#endif // #ifdef _ENABLE_DATAPOOL

      //D3DXVec3Normalize( &m_vDirection, &m_vDirection );

      ComputeAttenuation();
    }

    //--------------------------------------------------------------------------------------
    // Set sun position based on time.
    //--------------------------------------------------------------------------------------
    void CSun::SetPosition( float fTime )
    {
//      float t = fTime + 0.170f * sinf( (4.0f * CL_PI * (JULIANDATE - 80.0f)) / 373.0f )
//        - 0.129f * sinf( (2.0f * CL_PI * (JULIANDATE - 8.0f )) / 355.0f )
//        + ( 12 * ( MERIDIAN - LONGITUDE ) ) / CL_PI;
//
#ifdef _ENABLE_DATAPOOL
      m_fVarSunTime = fTime;
#endif // #ifdef _ENABLE_DATAPOOL
//
//      float fDelta = 0.4093f * sinf( (2.0f * CL_PI * (JULIANDATE - 81.0f)) / 368.0f );
//
//      float fSinLat = sinf( LATITUDE );
//      float fCosLat = cosf( LATITUDE );
//      float fSinDelta = sinf( fDelta );
//      float fCosDelta = cosf( fDelta );
//      float fSinT = sinf( ( CL_PI * t ) / 12.0f );
//      float fCosT = cosf( ( CL_PI * t ) / 12.0f );
//
//      float fTheta = CL_PI / 2.0f - asinf( fSinLat * fSinDelta - fCosLat * fCosDelta * fCosT );
//      float fPhi = atanf( (              - fCosDelta * fSinT                    ) /
//        ( fCosLat * fSinDelta - fSinLat * fCosDelta * fCosT ) );
//
//      SetPosition( fTheta, fPhi );
    }

    //--------------------------------------------------------------------------------------
    // Set sun light intensity.
    //--------------------------------------------------------------------------------------
    void CSun::SetIntensity( float fIntensity )
    {
#ifdef _ENABLE_DATAPOOL
      if(m_fVarIntensity.IsValid())
      {
        m_fVarIntensity = fIntensity;
      }
#else
      m_fIntensity = fIntensity;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Set turbidity.
    //--------------------------------------------------------------------------------------
    void CSun::SetTurbidity( float fTurbidity )
    {
#ifdef _ENABLE_DATAPOOL
      if(m_fVarTurbidity.IsValid())
      {
        m_fVarTurbidity = fTurbidity;
      }
#else
      m_fTurbidity = fTurbidity;
#endif // #ifdef _ENABLE_DATAPOOL

      ComputeAttenuation();
    }

    //--------------------------------------------------------------------------------------
    // Calculate the sun color based on sun position.
    //
    // This function is from the source available on 
    // http://www.cs.utah.edu/vissim/papers/sunsky/
    // Used in the paper "A Practical Analytic Model for Daylight"
    // by  A. J. Preetham, Peter Shirley, Brian Smits
    //--------------------------------------------------------------------------------------
    void CSun::ComputeAttenuation()
    {
      // m_fVarTurbidity
      // m_fVarTheta
#ifdef _ENABLE_DATAPOOL
      float fBeta = 0.04608365822050f * m_fVarTurbidity - 0.04586025928522f;
      float fTauR, fTauA, fTau[3];
      float m = 1.0f/(cos(m_fVarTheta) + 0.15f * pow(93.885f-m_fVarTheta/CL_PI*180.0f,-1.253f));  // Relative Optical Mass
#else
      float fBeta = 0.04608365822050f * m_fTurbidity - 0.04586025928522f;
      float fTauR, fTauA, fTau[3];
      float m = 1.0f/(cos(m_fTheta) + 0.15f*pow(93.885f-m_fTheta/CL_PI*180.0f,-1.253f));  // Relative Optical Mass
#endif // #ifdef _ENABLE_DATAPOOL

      int i;
      float fLambda[3]; 
      fLambda[0] = 0.65f;	// red (in um.)
      fLambda[1] = 0.57f;	// green (in um.)
      fLambda[2] = 0.475f;	// blue (in um.)


      for(i = 0; i < 3; i++)
      {
        // Rayleigh Scattering
        // Results agree with the graph (pg 115, MI) */
        // lambda in um.
        fTauR = exp( -m * 0.008735f * pow(fLambda[i], float(-4.08f)));

        // Aerosal (water + dust) attenuation
        // beta - amount of aerosols present 
        // alpha - ratio of small to large particle sizes. (0:4,usually 1.3)
        // Results agree with the graph (pg 121, MI) 
        const float fAlpha = 1.3f;
        fTauA = exp(-m * fBeta * pow(fLambda[i], -fAlpha));  // lambda should be in um


        fTau[i] = fTauR * fTauA; 

      }

#ifdef _ENABLE_DATAPOOL
      if(m_vVarColor.IsValid())
      {
        m_vVarColor = float3( fTau[0], fTau[1], fTau[2] );
      }
#else
      m_vColor = float3( fTau[0], fTau[1], fTau[2] );
#endif // #ifdef _ENABLE_DATAPOOL
    }

    void CSun::SetDataPool(MODataPool* pDataPool)
    {
      if(m_pDataPool) {
#ifdef ENABLE_DATAPOOL_WATCHER
        CLBREAK;
        //m_pDataPool->RemoveWatcher(this);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      }
      
      InlSetNewObjectT(m_pDataPool, pDataPool);

      if(m_pDataPool) {
#ifdef ENABLE_DATAPOOL_WATCHER
        CLBREAK;
        //m_pDataPool->AddWatcher(this);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      }

#ifdef _ENABLE_DATAPOOL
      //m_fVarIntensity = fIntensity;
      //m_fVarTurbidity = 2.0f;

      m_pDataPool->QueryByName("fSunTime",   &m_fVarSunTime);
      m_pDataPool->QueryByName("fTheta",     &m_fVarTheta);
      m_pDataPool->QueryByName("fPhi",       &m_fVarPhi);
      m_pDataPool->QueryByName("fIntensity", &m_fVarIntensity);
      m_pDataPool->QueryByName("vDirection", &m_vVarDirection);
      m_pDataPool->QueryByName("fTurbidity", &m_fVarTurbidity);
      m_pDataPool->QueryByName("vColor",     &m_vVarColor);
      ASSERT(m_fVarTheta.IsValid() && m_fVarPhi.IsValid() && m_fVarIntensity.IsValid() &&
        m_vVarDirection.IsValid() && m_fVarTurbidity.IsValid() && m_vVarColor.IsValid());
#endif // #ifdef _ENABLE_DATAPOOL
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT CSun::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT CSun::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0) {
        this->~CSun();
        return GX_OK;
      }
      return nRefCount;
    }

    clStringA CSun::GetClassName()
    {
      return "Scene.Sky.Sun";
    }

    GXHRESULT CSun::RegisterPrivate(GXLPVOID pIndentify)
    {
      return GX_FAIL;
    }

    GXHRESULT CSun::UnregisterPrivate(GXLPVOID pIndentify)
    {
      return GX_FAIL;
    }

    GXVOID CSun::OnImpulse(Marimo::LPCDATAIMPULSE pImpulse)
    {
      CLBREAK;
      if(GXSTRCMP(pImpulse->sponsor->GetName(), "fSunTime") == 0)
      {
        float fTime = m_fVarSunTime;
        float t = fTime + 0.170f * sinf( (4.0f * CL_PI * (JULIANDATE - 80.0f)) / 373.0f )
          - 0.129f * sinf( (2.0f * CL_PI * (JULIANDATE - 8.0f )) / 355.0f )
          + ( 12 * ( MERIDIAN - LONGITUDE ) ) / CL_PI;

        float fDelta = 0.4093f * sinf( (2.0f * CL_PI * (JULIANDATE - 81.0f)) / 368.0f );

        float fSinLat = sinf( LATITUDE );
        float fCosLat = cosf( LATITUDE );
        float fSinDelta = sinf( fDelta );
        float fCosDelta = cosf( fDelta );
        float fSinT = sinf( ( CL_PI * t ) / 12.0f );
        float fCosT = cosf( ( CL_PI * t ) / 12.0f );

        float fTheta = CL_PI / 2.0f - asinf( fSinLat * fSinDelta - fCosLat * fCosDelta * fCosT );
        float fPhi = atanf((-fCosDelta * fSinT) / (fCosLat * fSinDelta - fSinLat * fCosDelta * fCosT));

        SetPosition( fTheta, fPhi );
      }
      else if(GXSTRCMP(pImpulse->sponsor->GetName(), "fTurbidity") == 0)
      {
        ComputeAttenuation();
      }
      //return GX_OK;
    }


    //--------------------------------------------------------------------------------------
    // Default constructor.
    //--------------------------------------------------------------------------------------
    CAtmosphere::CAtmosphere()
      : m_pDataPool(NULL)
#ifdef _ENABLE_DATAPOOL
#else
      , m_fRayFactor(1000.0f)
      , m_fMieFactor(0.7f)
      , m_fTurbidity(2.0f)
      , m_fDirectionalityFactor(0.6f)
#endif // #ifdef _ENABLE_DATAPOOL
    {
      CalculateRayleighCoeff();
      CalculateMieCoeff();
    }

    //--------------------------------------------------------------------------------------
    // Default destructor.
    //--------------------------------------------------------------------------------------
    CAtmosphere::~CAtmosphere()
    {
#ifdef ENABLE_DATAPOOL_WATCHER
      CLBREAK;
      //m_pDataPool->RemoveWatcher(this);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      SAFE_RELEASE(m_pDataPool);
    }

    //--------------------------------------------------------------------------------------
    // Get the Beta Rayleigh coefficient.
    //--------------------------------------------------------------------------------------
    CFloat3& CAtmosphere::GetBetaRay()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarBetaRay;
#else
      return m_vBetaRay;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get the Beta Rayleigh theta coefficient.
    //--------------------------------------------------------------------------------------
    CFloat3& CAtmosphere::GetBetaRayTheta()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarBetaRayTheta;
#else
      return m_vBetaRayTheta;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get the Beta Mie coefficient.
    //--------------------------------------------------------------------------------------
    CFloat3& CAtmosphere::GetBetaMie()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarBetaMie;
#else
      return m_vBetaMie;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get the directionality factor.
    //--------------------------------------------------------------------------------------
    float CAtmosphere::GetDirectionalityFactor()
    {
#ifdef _ENABLE_DATAPOOL
      return m_fVarDirectionalityFactor;
#else
      return m_fDirectionalityFactor;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Set Rayleigh factor.
    //--------------------------------------------------------------------------------------
    void CAtmosphere::SetRayFactor( float fRayFactor )
    {
#ifdef _ENABLE_DATAPOOL      
      if(m_fVarRayFactor.IsValid())
      {
        m_fVarRayFactor = fRayFactor;
      }
#else
      m_fRayFactor = fRayFactor;
#endif // m_fRayFactor = fRayFactor;

      CalculateRayleighCoeff();
      CalculateMieCoeff();
    }

    //--------------------------------------------------------------------------------------
    // Set Mie factor.
    //--------------------------------------------------------------------------------------
    void CAtmosphere::SetMieFactor( float fMieFactor )
    {
#ifdef _ENABLE_DATAPOOL
      m_fVarMieFactor = fMieFactor;
#else
      m_fMieFactor = fMieFactor;
#endif // #ifdef _ENABLE_DATAPOOL

      CalculateRayleighCoeff();
      CalculateMieCoeff();
    }

    //--------------------------------------------------------------------------------------
    // Set turbidity.
    //--------------------------------------------------------------------------------------
    void CAtmosphere::SetTurbidity( float fTurbidity )
    {
#ifdef _ENABLE_DATAPOOL
      m_fVarTurbidity = fTurbidity;
#else
      m_fTurbidity = fTurbidity;
#endif // #ifdef _ENABLE_DATAPOOL

      CalculateRayleighCoeff();
      CalculateMieCoeff();

      //CSceneManager::GetInstancePtr()->GetSun()->SetTurbidity( fTurbidity );
    }

    //--------------------------------------------------------------------------------------
    // Set directionality factor.
    //--------------------------------------------------------------------------------------
    void CAtmosphere::SetDirectionalityFactor( float fDirectionalityFactor )
    {
#ifdef _ENABLE_DATAPOOL
      m_fVarDirectionalityFactor = fDirectionalityFactor;
#else
      m_fDirectionalityFactor = fDirectionalityFactor;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Get the Beta Mie theta coefficient.
    //--------------------------------------------------------------------------------------
    CFloat3& CAtmosphere::GetBetaMieTheta()
    {
#ifdef _ENABLE_DATAPOOL
      return (CFloat3&)m_vVarBetaMieTheta;
#else
      return m_vBetaMieTheta;
#endif // #ifdef _ENABLE_DATAPOOL
    }

    //--------------------------------------------------------------------------------------
    // Calculate the Rayleigh scattering coefficient
    // for the wavelength: Red(650), Green(570) and Blue(475)
    //--------------------------------------------------------------------------------------
    void CAtmosphere::CalculateRayleighCoeff()
    {
      // 由下列值计算
      // m_fVarRayFactor

      const float n  = 1.00029f;		//Refraction index for air
      const float N  = 2.545e25f;		//Molecules per unit volume
      const float pn = 0.035f;		//Depolarization factor

#ifdef _ENABLE_DATAPOOL
      if( ! m_fVarRayFactor.IsValid()) {
        return;
      }
      float fRayleighFactor = m_fVarRayFactor * ( pow( CL_PI, 2.0f ) * pow( n*n - 1.0f, 2.0f ) * ( 6 + 3 * pn ) ) /
        (N * ( 6 - 7 * pn ));

      if(m_vVarBetaRayTheta.IsValid())
      {
        ((float3&)m_vVarBetaRayTheta).x = fRayleighFactor / ( 2.0f * pow( 650.0e-9f, 4.0f ) );
        ((float3&)m_vVarBetaRayTheta).y = fRayleighFactor / ( 2.0f * pow( 570.0e-9f, 4.0f ) );
        ((float3&)m_vVarBetaRayTheta).z = fRayleighFactor / ( 2.0f * pow( 475.0e-9f, 4.0f ) );
      }

      if(m_vVarBetaRay.IsValid())
      {
        ((float3&)m_vVarBetaRay).x = 8.0f * fRayleighFactor / ( 3.0f * pow( 650.0e-9f, 4.0f ) );
        ((float3&)m_vVarBetaRay).y = 8.0f * fRayleighFactor / ( 3.0f * pow( 570.0e-9f, 4.0f ) );
        ((float3&)m_vVarBetaRay).z = 8.0f * fRayleighFactor / ( 3.0f * pow( 475.0e-9f, 4.0f ) );
      }
#else
      float fRayleighFactor = m_fRayFactor * ( pow( CL_PI, 2.0f ) * pow( n*n - 1.0f, 2.0f ) * ( 6 + 3 * pn ) ) /
        (	                   N                          * ( 6 - 7 * pn ) );

      m_vBetaRayTheta.x = fRayleighFactor / ( 2.0f * pow( 650.0e-9f, 4.0f ) );
      m_vBetaRayTheta.y = fRayleighFactor / ( 2.0f * pow( 570.0e-9f, 4.0f ) );
      m_vBetaRayTheta.z = fRayleighFactor / ( 2.0f * pow( 475.0e-9f, 4.0f ) );

      m_vBetaRay.x = 8.0f * fRayleighFactor / ( 3.0f * pow( 650.0e-9f, 4.0f ) );
      m_vBetaRay.y = 8.0f * fRayleighFactor / ( 3.0f * pow( 570.0e-9f, 4.0f ) );
      m_vBetaRay.z = 8.0f * fRayleighFactor / ( 3.0f * pow( 475.0e-9f, 4.0f ) );
#endif
    }

    //--------------------------------------------------------------------------------------
    // Calculate the Mie scattering coefficient
    // for the wavelength: Red(650), Green(570) and Blue(475)
    //--------------------------------------------------------------------------------------
    void CAtmosphere::CalculateMieCoeff()
    {
      // 由下列值计算:
      // m_fVarTurbidity
      // m_fVarMieFactor

      float K[3] = { 0.685f, 0.682f, 0.670f };
#ifdef _ENABLE_DATAPOOL
      if( ! m_fVarTurbidity.IsValid() || ! m_fVarMieFactor.IsValid()){
        return;
      }
      float c = ( 0.6544f * m_fVarTurbidity - 0.6510f ) * 1e-16f;	//Concentration factor
      float fMieFactor = m_fVarMieFactor * 0.434f * c * 4.0f * CL_PI * CL_PI;
      if(m_vVarBetaMieTheta.IsValid())
      {
        ((float3&)m_vVarBetaMieTheta).set(
          fMieFactor / ( 2.0f * pow(650e-9f, 2.0f)),
          fMieFactor / ( 2.0f * pow(570e-9f, 2.0f)),
          fMieFactor / ( 2.0f * pow(475e-9f, 2.0f)));
      }

      if(m_vVarBetaMie.IsValid())
      {
        ((float3&)m_vVarBetaMie).set(
          K[0] * fMieFactor / pow(650e-9f, 2.0f),
          K[1] * fMieFactor / pow(570e-9f, 2.0f),
          K[2] * fMieFactor / pow(475e-9f, 2.0f));
      }
#else
      float c = ( 0.6544f * m_fTurbidity - 0.6510f ) * 1e-16f;	//Concentration factor

      float fMieFactor = m_fMieFactor * 0.434f * c * 4.0f * CL_PI * CL_PI;

      m_vBetaMieTheta.x = fMieFactor / ( 2.0f * pow( 650e-9f, 2.0f ) );
      m_vBetaMieTheta.y = fMieFactor / ( 2.0f * pow( 570e-9f, 2.0f ) );
      m_vBetaMieTheta.z = fMieFactor / ( 2.0f * pow( 475e-9f, 2.0f ) );

      m_vBetaMie.x = K[0] * fMieFactor / pow( 650e-9f, 2.0f );
      m_vBetaMie.y = K[1] * fMieFactor / pow( 570e-9f, 2.0f );
      m_vBetaMie.z = K[2] * fMieFactor / pow( 475e-9f, 2.0f );
#endif // #ifdef _ENABLE_DATAPOOL
    }

    void CAtmosphere::SetDataPool(MODataPool* pDataPool)
    {
#ifdef ENABLE_DATAPOOL_WATCHER
      if(m_pDataPool) {
        CLBREAK;
        //m_pDataPool->RemoveWatcher(this);
      }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

      InlSetNewObjectT(m_pDataPool, pDataPool);

#ifdef ENABLE_DATAPOOL_WATCHER
      if(m_pDataPool) {
        CLBREAK;
        //m_pDataPool->AddWatcher(this);
      }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

#ifdef _ENABLE_DATAPOOL
      m_pDataPool->QueryByName("vBetaRay",              &m_vVarBetaRay);
      m_pDataPool->QueryByName("vBetaRayTheta",         &m_vVarBetaRayTheta);
      m_pDataPool->QueryByName("vBetaMie",              &m_vVarBetaMie);
      m_pDataPool->QueryByName("vBetaMieTheta",         &m_vVarBetaMieTheta);
      m_pDataPool->QueryByName("fRayFactor",            &m_fVarRayFactor);
      m_pDataPool->QueryByName("fMieFactor",            &m_fVarMieFactor);
      m_pDataPool->QueryByName("fTurbidity",            &m_fVarTurbidity);
      m_pDataPool->QueryByName("fDirectionalityFactor", &m_fVarDirectionalityFactor);

      ASSERT(
      m_vVarBetaRay.IsValid() && m_vVarBetaRayTheta.IsValid() && m_vVarBetaMie.IsValid() && 
      m_vVarBetaMieTheta.IsValid() && m_fVarRayFactor.IsValid() && m_fVarMieFactor.IsValid() && 
      m_fVarTurbidity.IsValid() && m_fVarDirectionalityFactor.IsValid());
#endif // #ifdef _ENABLE_DATAPOOL

    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT CAtmosphere::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT CAtmosphere::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0) {
        this->~CAtmosphere();
        return GX_OK;
      }
      return nRefCount;
    }

    clStringA CAtmosphere::GetClassName()
    {
      return "Scene.Sky.Atmpsphere";
    }

    GXHRESULT CAtmosphere::RegisterPrivate(GXLPVOID pIndentify)
    {
      return GX_FAIL;
    }

    GXHRESULT CAtmosphere::UnregisterPrivate(GXLPVOID pIndentify)
    {
      return GX_FAIL;
    }

    //GXHRESULT CAtmosphere::OnKnock(Marimo::KNOCKACTION* pKnock)
    GXVOID CAtmosphere::OnImpulse(Marimo::LPCDATAIMPULSE pImpulse)
    {
#ifdef ENABLE_DATAPOOL_WATCHER
      CLBREAK;
      //if(ON_KNOCKVAR(pKnock, m_fVarRayFactor))
      //{
      //  CalculateRayleighCoeff();
      //}
      //else if(ON_KNOCKVAR(pKnock, m_fVarTurbidity))
      //{
      //  CalculateMieCoeff();
      //}
      //else if(ON_KNOCKVAR(pKnock, m_fVarMieFactor))
      //{
      //  CalculateMieCoeff();
      //}
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      //return GX_OK;
    }

  } // namespace Internal

  CFloat3& SkyQuad::GetDirection()
  {
    return m_Sun.GetDirection();
  }

  float SkyQuad::GetIntensity()
  {
    return m_Sun.GetIntensity();
  }

  MOVarFloat SkyQuad::GetIntensityVar()
  {
    return m_Sun.m_fVarIntensity;
  }

  CFloat3& SkyQuad::GetColor()
  {
    return m_Sun.GetColor();
  }

  CFloat3& SkyQuad::GetBetaRay()
  {
    return m_Atmosphere.GetBetaRay();
  }

  CFloat3& SkyQuad::GetBetaRayTheta()
  {
    return m_Atmosphere.GetBetaRayTheta();
  }

  CFloat3& SkyQuad::GetBetaMie()
  {
    return m_Atmosphere.GetBetaMie();
  }

  CFloat3& SkyQuad::GetBetaMieTheta()
  {
    return m_Atmosphere.GetBetaMieTheta();
  }

  float SkyQuad::GetDirectionalityFactor()
  {
    return m_Atmosphere.GetDirectionalityFactor();
  }

  SkyQuad::~SkyQuad()
  {
    SAFE_RELEASE(m_pDataPool);
  }

  GXHRESULT SkyQuad::Create(GXGraphics* pGraphics, SkyQuad** ppSkyQuad)
  {
    GXHRESULT hval = GX_OK;
    SkyQuad* pSky = new SkyQuad;
    if( ! InlCheckNewAndIncReference(pSky)) {
      return GX_FAIL;
    }

    pSky->SetName("sky quad");

    if( ! pSky->InitializeAsQuadPlane(pGraphics, float3(0,0,0.9f), float3(0,0,-1), float3::AxisY,
      float2(2,2), 64, 64, GXVF_NORMAL | GXVF_TEXCOORD))  // 这个格数不能降低, 看看 Shader 里是否能解决?
    {
      hval = GX_FAIL;
    }

    if(GXSUCCEEDED(hval))
    {
      hval = MODataPool::CompileFromMemory(&pSky->m_pDataPool, 
        "AtmosphereScatteringPool", NULL, Internal::s_szDataPoolDecl);
      if(GXSUCCEEDED(hval))
      {
#ifdef ENABLE_DATAPOOL_WATCHER
        pSky->m_pDataPool->SetAutoKnock(TRUE);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
        pSky->m_Sun.SetDataPool(pSky->m_pDataPool);
        pSky->m_Atmosphere.SetDataPool(pSky->m_pDataPool);
      }
    }

    if(GXSUCCEEDED(hval))
    {
      pSky->m_Sun.SetPosition(9.7f);
      pSky->m_Sun.SetIntensity(3.8f);
      pSky->m_Sun.SetTurbidity(1.7f);

      pSky->m_Atmosphere.SetDirectionalityFactor(0.6f);
      pSky->m_Atmosphere.SetTurbidity(1.7f);
      pSky->m_Atmosphere.SetMieFactor(0.125f);
      pSky->m_Atmosphere.SetRayFactor(607.0f);
    }

    if(GXSUCCEEDED(hval))
    {
      GXMaterialInst* pSkyMtl = NULL;
      hval = pGraphics->CreateMaterialFromFileW(&pSkyMtl, L"shaders\\szCraft_SkyAtmoScatt.txt", MLT_REFERENCE);
      if(GXSUCCEEDED(hval))
      {
        pSkyMtl->SetFloat3ByName("v3LightPos", float3(0.01f,1.0f,0.01f));
        pSkyMtl->SetFloat1ByName("g_fSunIntensity", pSky->GetIntensity());

        hval = pSky->SetMaterialInst(pSkyMtl, NODEMTL_IGNOREVERT);
        pSky->CombineFlags(GVNF_NOCLIP);
        SAFE_RELEASE(pSkyMtl);
      }
    }

    if(GXFAILED(hval))
    {
      pSky->Release();
      pSky = NULL;
    }

    *ppSkyQuad = pSky;
    return hval;
  }
} // namespace Scene