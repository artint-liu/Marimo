#include <clstd.h>
#include <clString.h>
#include <Smart/SmartStream.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"

ExpressionParser expp;

char* szExpTest[] = {
  "i=4, n=10",   // 下面的例子就是这里的初始值
  "n--i",       // err
  "n---i",      // n=9, i=4, 结果6
  "n-- -i",     // n=9, i=4, 结果6
  "n- --i",     // n=10, i=3, 结果7
  "n--- --i",   // n=9, i=3, 结果7
  "n--- - --i", // n=9, i=3, 结果13
  
  // 整数
  "10",
  "2e3",
  "-10",
  "-2e3",

  // 十六进制
  "0x123456",

  // 八进制
  "02314",
  "-02314",

  // float
  "1.5",
  "1.5f",
  "1.",
  ".5",
  ".5f",
  "1e3",
  "1e-3",
  "1.5e3",
  "1.5e-3",
  "1e3f",
  "1e-3f",
  "1.5e3f",
  "1.5e-3f",
  "1.e3f",
  "1.e-3f",
  ".5e3f",
  ".5e-3f",

  // 负数float
  "-1.5",
  "-1.5f",
  "-1.",
  "-.5",
  "-.5f",
  "-1e3",
  "-1e-3",
  "-1.5e3",
  "-1.5e-3",
  "-1e3f",
  "-1e-3f",
  "-1.5e3f",
  "-1.5e-3f",
  "-1.e3f",
  "-1.e-3f",
  "-.5e3f",
  "-.5e-3f",

  "Input.Normal = (Input.Normal - 0.5) * 2.0",
  "Input.Position.xyz += SwingGrass(Input.Position.xyz, Input.Texcoord.y - 0.1875)",
  "float  spec = max(0, dot(normalize(Input.vViewDir), normalize(vRLight)))",
  "const float fAdjustFactor = 255.0 / 256.0 * 2.0",
  "float SR = ( 1.05f - pow( V.y, 0.3f ) ) * 1000",
  "float3 L = Lin( pow(max(0, Theta), 10.0), SR, SM )",
  "Output.color.rgb = L * g_vLightDiffuse * g_fSunIntensity",
  "Output.I.rgb = (1.0f - Output.E.rgb) * I( Theta ) * g_vLightDiffuse.xyz * g_fSunIntensity",
  "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient + Output.Specular * shadowFactor)",
  "float4 c = Output.Diffuse * (Output.LdotN + Output.Ambient);",
  "  return float4(  c.xyz * Input.E.xyz  + Input.I.xyz, Output.Diffuse.w)",
  "return (vBetaMieTheta * pow((1.0f - g), 2.0)) / (pow(abs(1 + g * g - 2 * g * Theta), 1.5))",
  "return vBetaRayTheta * (2.0f + 0.5f * Theta * Theta)",
  "return (BetaR(Theta) + BetaM(Theta)) / (vBetaRay + vBetaMie)",
  "return exp( -(vBetaRay * SR + vBetaMie * SM))",
  "return ((BetaR(Theta) + BetaM(Theta)) * (1.0f - exp(-(vBetaRay * SR + vBetaMie * SM)))) / (vBetaRay + vBetaMie)",
  "float4 Diffuse = tex2D(MainSampler, Input.TexUV) * Input.Color",
  "Output.uvSM.x = (Output.uvSM.x + Output.uvSM.w) * 0.5",
  "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)",
  "vec2 slopesVal = texture2DGrad(river.wave.patternTex, rCoord / river.wave.length, gradx, grady).xy",
  "return sumWeight > 0.0 ? sumWeightedSlopes / sqrt(sumWeight) : texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy",
  "vec3 fn = vec3(textureTile(fragmentNormalSampler, uv).xy * 2.0 - 1.0, 0.0)",
  "slopes = texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy",
  "riverColor = riverShallowColor + groundColor * texture2DGrad(river.bed.patternTex, normal.xy * river.depth / 10.0 + p.xy / (river.bed.length / 2.0), gradx, grady).xyz / (dot(n, vVec) * river.depth * riverDepth)",
  "vec2 v = abs(st * river.gridSize / river.screenSize - floor(st * river.gridSize / river.screenSize) - 0.5)",
  "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)",
  "float3 inscatter = skyRadiance(WCP + origin, d, WSD, extinction, 0.0)",
  "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient)",
  "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)",
  "groundColor = treeBrdf(q, d, lcc, v, fn, WSD, vec3(0.0, 0.0, 1.0), reflectance, sunL, skyE)",
  "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)",
  "float4 cst = rmu < 0.0 && delta > 0.0 ? float4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(RES_MU)) : float4(-1.0, H * H, H, 0.5 + 0.5 / float(RES_MU))",
  "float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU))",
  "float uMuS = 0.5 / float(RES_MU_S) + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(RES_MU_S))",
  "    return tex3D(table, float3((uNu + uMuS      ) / float(RES_NU), uMu, uR)) * (1.0 - lerp) +  tex3D(table, float3((uNu + uMuS + 1.0) / float(RES_NU), uMu, uR)) * lerp",
  "float2 y = a01s / (2.3193*abs(a01) + sqrt(1.52*a01sq + 4.0)) * float2(1.0, exp(-d/H*(d/(2.0*r)+mu)))",
  "return 1.5 * 1.0 / (4.0 * M_PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*mu, -3.0/2.0) * (1.0 + mu * mu) / (2.0 + mieG*mieG)",
  "float3 data = viewdir.z > 0.0 ? tex2D(glareSampler, float2(0.5,0.5) + viewdir.xy * 4.0).rgb : float3(0.0,0.0,0.0)",
  NULL,
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  NULL,
};

void TestExpressionParser()
{
  for(int i = 0; szExpTest[i] != 0; i++)
  {
    expp.Attach(szExpTest[i], strlen(szExpTest[i]));
    int nCount = 0;
    for(SmartStreamA::iterator it = expp.begin();
      it != expp.end(); ++it)
    {
      TRACE("|%s|  ", it.ToString());
      nCount++;
    }
    TRACE("(%d)\n\n", nCount);
  }
}