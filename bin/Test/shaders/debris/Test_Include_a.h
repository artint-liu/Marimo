// …¢…‰œ‡πÿ
float3 vBetaRay;                  // Rayleigh scattering coefficient
float3 vBetaRayTheta;             // Rayleigh theta scattering coefficient
float3 vBetaMie;                  // Mie scattering coefficient
float3 vBetaMieTheta;             // Mie theta scattering coefficient
float  fDirectionalityFactor;

static const float g_fRayleighConstant = 0.0596831f;  // 3/(16*PI)
static const float g_fMieConstant      = 0.0795775f;  // 1/(4*PI)

//////////////////////////////////////////////////////////////////////////
float3 BetaR(float Theta)
{
  return vBetaRayTheta * (2.0f + 0.5f * Theta * Theta);
}

float3 BetaM(float Theta)
{
  float g = fDirectionalityFactor;

  return (vBetaMieTheta * pow((1.0f - g), 2.0)) / (pow(abs(1 + g * g - 2 * g * Theta), 1.5));
}

float3 I(float Theta)
{
  return (BetaR(Theta) + BetaM(Theta)) / (vBetaRay + vBetaMie);
}

float3 E(float Theta, float SR, float SM)
{
  return exp( -(vBetaRay * SR + vBetaMie * SM));
}

float3 Lin(float Theta, float SR, float SM)
{
  return ((BetaR(Theta) + BetaM(Theta)) * (1.0f - exp(-(vBetaRay * SR + vBetaMie * SM)))) / (vBetaRay + vBetaMie);
};

//////////////////////////////////////////////////////////////////////////

float ScaleToEllipse(float Theta, float d)
{
  return d * 50.0;// * (1.1 - abs(Theta / d));
}