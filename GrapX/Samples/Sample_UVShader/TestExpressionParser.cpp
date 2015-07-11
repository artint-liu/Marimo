#include <GrapX.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

GXLPCSTR aOperStack_016[] = {
  "[,] [0] [Theta]",
  "[<func call>] [max] [0,Theta]",
  "[,] [max(0,Theta)] [10.0]",
  "[<func call>] [pow] [max(0,Theta),10.0]",
  "[,] [pow(max(0,Theta),10.0)] [SR]",
  "[,] [pow(max(0,Theta),10.0),SR] [SM]",
  "[<func call>] [Lin] [pow(max(0,Theta),10.0),SR,SM]",
  "[=] [float3L] [Lin(pow(max(0,Theta),10.0),SR,SM)]",
  NULL, };

GXLPCSTR aOperStack_017[] = {
  "[.] [Output] [color]",
  "[.] [Output.color] [rgb]",
  "[*] [L] [g_vLightDiffuse]",
  "[*] [L*g_vLightDiffuse] [g_fSunIntensity]",
  "[=] [Output.color.rgb] [L*g_vLightDiffuse*g_fSunIntensity]",
  NULL, };

GXLPCSTR aOperStack_018[] = {
  "[.] [Output] [I]",
  "[.] [Output.I] [rgb]",
  "[.] [Output] [E]",
  "[.] [Output.E] [rgb]",
  "[-] [1.0f] [Output.E.rgb]",
  "[<func call>] [I] [Theta]",
  "[*] [(1.0f-Output.E.rgb)] [I(Theta)]",
  "[.] [g_vLightDiffuse] [xyz]",
  "[*] [(1.0f-Output.E.rgb)*I(Theta)] [g_vLightDiffuse.xyz]",
  "[*] [(1.0f-Output.E.rgb)*I(Theta)*g_vLightDiffuse.xyz] [g_fSunIntensity]",
  "[=] [Output.I.rgb] [(1.0f-Output.E.rgb)*I(Theta)*g_vLightDiffuse.xyz*g_fSunIntensity]",
  NULL};

GXLPCSTR aOperStack_033[] = {
  "[.] [river] [bed]",
  "[.] [river.bed] [patternTex]",
  "[.] [normal] [xy]",
  "[.] [river] [depth]",
  "[*] [normal.xy] [river.depth]",
  "[/] [normal.xy*river.depth] [10.0]",
  "[.] [p] [xy]",
  "[.] [river] [bed]",
  "[.] [river.bed] [length]",
  "[/] [river.bed.length] [2.0]",
  "[/] [p.xy] [(river.bed.length/2.0)]",
  "[+] [normal.xy*river.depth/10.0] [p.xy/(river.bed.length/2.0)]",
  "[,] [river.bed.patternTex] [normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0)]",
  "[,] [river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0)] [gradx]",
  "[,] [river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx] [grady]",
  "[<func call>] [texture2DGrad] [river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady]",
  "[.] [texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady)] [xyz]",
  "[*] [groundColor] [texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz]",
  "[,] [n] [vVec]",
  "[<func call>] [dot] [n,vVec]",
  "[.] [river] [depth]",
  "[*] [dot(n,vVec)] [river.depth]",
  "[*] [dot(n,vVec)*river.depth] [riverDepth]",
  "[/] [groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz] [(dot(n,vVec)*river.depth*riverDepth)]",
  "[+] [riverShallowColor] [groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz/(dot(n,vVec)*river.depth*riverDepth)]",
  "[=] [riverColor] [riverShallowColor+groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz/(dot(n,vVec)*river.depth*riverDepth)]",
  NULL, };


SAMPLE_EXPRESSION samples[] = {
  {0, "i=4, n=10",  7}, // 下面的例子就是这里的初始值
  {0, "n--i",       3},// err
  {0, "n---i",      4},// n=9, i=4, 结果6
  {0, "n-- -i",     4},// n=9, i=4, 结果6
  {0, "n- --i",     4},// n=10, i=3, 结果7
  {0, "n--- --i",   5},// n=9, i=3, 结果7
  {0, "n--- - --i", 6},// n=9, i=3, 结果13
  
  //// 整数
  //{0, "10",  1},
  //{0, "2e3", 1},
  ////"-10",
  ////"-2e3",

  //// 十六进制
  //{0, "0x123456", 1},

  //// 八进制
  //{0, "02314",  1},
  //{0, "-02314", 2},

  //// float
  //{0, "1.5",           1},
  //{0, "1.5f",          1},
  //{0, "1.",            1},
  //{0, ".5",            1},
  //{0, ".5f",           1},
  //{0, "1e3",           1},
  //{0, "1e-3",          1},
  //{0, "1.5e3",         1},
  //{0, "1.5e-3",        1},
  //{0, "1e3f",          1},
  //{0, "1e-3f",         1},
  //{0, "1.5e3f",        1},
  //{0, "1.5e-3f",       1},
  //{0, "1.e3f",         1},
  //{0, "1.e-3f",        1},
  //{0, ".5e3f",         1},
  //{0, ".5e-3f",        1},

  //// 负数float
  //{0, "-1.5",          2},
  //{0, "-1.5f",         2},
  //{0, "-1.",           2},
  //{0, "-.5",           2},
  //{0, "-.5f",          2},
  //{0, "-1e3",          2},
  //{0, "-1e-3",         2},
  //{0, "-1.5e3",        2},
  //{0, "-1.5e-3",       2},
  //{0, "-1e3f",         2},
  //{0, "-1e-3f",        2},
  //{0, "-1.5e3f",       2},
  //{0, "-1.5e-3f",      2},
  //{0, "-1.e3f",        2},
  //{0, "-1.e-3f",       2},
  //{0, "-.5e3f",        2},
  //{0, "-.5e-3f",       2},
  //{0, NULL, 0},
  {0, "output.color", 3},
  {0, "a+b+c+d*e/f", 11},
  {0, "k*((a*b)+c+d*e)", 15},
  {0, "(Output.LdotN*shadowFactor)+Output.Ambient+Output.Specular*shadowFactor", 17},
  {0, "Input.Normal = (Input.Normal - 0.5) * 2.0", 13},
  {0, "Input.Position.xyz += SwingGrass(Input.Position.xyz, Input.Texcoord.y - 0.1875)", 22},
  {0, "float  spec = max(0, dot(normalize(Input.vViewDir), normalize(vRLight)))", 22},
  {0, "const float fAdjustFactor = 255.0 / 256.0 * 2.0", 9},
  {0, "float SR = ( 1.05f - pow( V.y, 0.3f ) ) * 1000", 17},
  {0, "float3 L = Lin( pow(max(0, Theta), 10.0), SR, SM )", 21, aOperStack_016},
  {0, "Output.color.rgb = L * g_vLightDiffuse * g_fSunIntensity", 11, aOperStack_017},
  {0, "Output.I.rgb = (1.0f - Output.E.rgb) * I( Theta ) * g_vLightDiffuse.xyz * g_fSunIntensity", 26, aOperStack_018},
  {0, "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient + Output.Specular * shadowFactor)", 26},
  {0, "float4 c = Output.Diffuse * (Output.LdotN + Output.Ambient);", 17},
  {0, "  return float4(  c.xyz * Input.E.xyz  + Input.I.xyz, Output.Diffuse.w)", 25},
  {0, "return (vBetaMieTheta * pow((1.0f - g), 2.0)) / (pow(abs(1 + g * g - 2 * g * Theta), 1.5))", 37},
  {0, "return vBetaRayTheta * (2.0f + 0.5f * Theta * Theta)", 12},
  {0, "return (BetaR(Theta) + BetaM(Theta)) / (vBetaRay + vBetaMie)", 18},
  {0, "return exp( -(vBetaRay * SR + vBetaMie * SM))", 14},
  {0, "return ((BetaR(Theta) + BetaM(Theta)) * (1.0f - exp(-(vBetaRay * SR + vBetaMie * SM)))) / (vBetaRay + vBetaMie)", 38},
  {0, "float4 Diffuse = tex2D(MainSampler, Input.TexUV) * Input.Color", 15},
  {0, "Output.uvSM.x = (Output.uvSM.x + Output.uvSM.w) * 0.5", 21},
  {0, "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)", 25},
  {0, "vec2 slopesVal = texture2DGrad(river.wave.patternTex, rCoord / river.wave.length, gradx, grady).xy", 25},
  {0, "vec3 fn = vec3(textureTile(fragmentNormalSampler, uv).xy * 2.0 - 1.0, 0.0)", 20},
  {0, "slopes = texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy", 26},
  {0, "riverColor = riverShallowColor + groundColor * texture2DGrad(river.bed.patternTex, normal.xy * river.depth / 10.0 + p.xy / (river.bed.length / 2.0), gradx, grady).xyz / (dot(n, vVec) * river.depth * riverDepth)", 59, aOperStack_033},
  {0, "vec2 v = abs(st * river.gridSize / river.screenSize - floor(st * river.gridSize / river.screenSize) - 0.5)", 30},
  {0, "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)", 33},
  {0, "float3 inscatter = skyRadiance(WCP + origin, d, WSD, extinction, 0.0)", 17},
  {0, "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient)", 20},
  {0, "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)", 25},
  {0, "groundColor = treeBrdf(q, d, lcc, v, fn, WSD, vec3(0.0, 0.0, 1.0), reflectance, sunL, skyE)", 31},
  {0, "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)", 33},
  {0, "float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU))", 42},
  {0, "float uMuS = 0.5 / float(RES_MU_S) + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(RES_MU_S))", 50},
  {0, "    return tex3D(table, float3((uNu + uMuS      ) / float(RES_NU), uMu, uR)) * (1.0 - lerp) +  tex3D(table, float3((uNu + uMuS + 1.0) / float(RES_NU), uMu, uR)) * lerp", 56},
  {0, "float2 y = a01s / (2.3193*abs(a01) + sqrt(1.52*a01sq + 4.0)) * float2(1.0, exp(-d/H*(d/(2.0*r)+mu)))", 47},
  {0, "return 1.5 * 1.0 / (4.0 * M_PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*mu, -3.0/2.0) * (1.0 + mu * mu) / (2.0 + mieG*mieG)", 56},
  {0, "m_fHue = m_fSaturation = m_fValue = 0.0",    7},

  //{"a?b:c", 5},
  //{"a?b:c?f:g", 9},
  //{"a?b?d:e:c", 9},
  //{"a?b?d:e:c?f:g", 14},
  //{"return sumWeight > 0.0 ? sumWeightedSlopes / sqrt(sumWeight) : texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy", 36},
  //{"float4 cst = rmu < 0.0 && delta > 0.0 ? float4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(RES_MU)) : float4(-1.0, H * H, H, 0.5 + 0.5 / float(RES_MU))", 49},
  //{"float3 data = viewdir.z > 0.0 ? tex2D(glareSampler, float2(0.5,0.5) + viewdir.xy * 4.0).rgb : float3(0.0,0.0,0.0)", 37},
  {0, NULL,  0},
  {0, "",    0},
  {0, "",    0},
  {0, "",    0},
  {0, "",    0},
  {0, "",    0},
  {0, "",    0},
  {0, "",    0},
  {0, NULL,  0},
};
