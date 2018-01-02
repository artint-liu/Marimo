#include <GrapX.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ArithmeticExpression.h"
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "ExpressionSample.h"

GXLPCSTR aOperStack_HasError[] = {NULL,};

GXLPCSTR aOperStack_008[] = {
  "[*] [b] [c]",
  "[+] [a] [b*c]",
  NULL, };

GXLPCSTR aOperStack_009[] = {
  "[+] [a] [b]",
  "[*] [(a+b)] [c]",
  NULL, };

GXLPCSTR aOperStack_010[] = {
  "[+] [a] [b]",
  "[+] [a+b] [c]",
  "[*] [d] [e]",
  "[/] [d*e] [f]",
  "[+] [a+b+c] [d*e/f]",
  NULL, };

GXLPCSTR aOperStack_Return0100[] = {
  "[.] [a] [x]",
  "[.] [a] [y]",
  "[,] [a.x] [a.y]",
  "[.] [f] [z]",
  "[,] [a.x,a.y] [f.z]",
  "[F] [mix] [a.x,a.y,f.z]",
 NULL, };

GXLPCSTR aOperStack_016[] = {
  "[,] [0] [Theta]",
  "[F] [max] [0,Theta]",
  "[,] [max(0,Theta)] [10.0]",
  "[F] [pow] [max(0,Theta),10.0]",
  "[,] [pow(max(0,Theta),10.0)] [SR]",
  "[,] [pow(max(0,Theta),10.0),SR] [SM]",
  "[F] [Lin] [pow(max(0,Theta),10.0),SR,SM]",
  "[=] [L] [Lin(pow(max(0,Theta),10.0),SR,SM)]",
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
  "[F] [I] [Theta]",
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
  "[F] [texture2DGrad] [river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady]",
  "[.] [texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady)] [xyz]",
  "[*] [groundColor] [texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz]",
  "[,] [n] [vVec]",
  "[F] [dot] [n,vVec]",
  "[.] [river] [depth]",
  "[*] [dot(n,vVec)] [river.depth]",
  "[*] [dot(n,vVec)*river.depth] [riverDepth]",
  "[/] [groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz] [(dot(n,vVec)*river.depth*riverDepth)]",
  "[+] [riverShallowColor] [groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz/(dot(n,vVec)*river.depth*riverDepth)]",
  "[=] [riverColor] [riverShallowColor+groundColor*texture2DGrad(river.bed.patternTex,normal.xy*river.depth/10.0+p.xy/(river.bed.length/2.0),gradx,grady).xyz/(dot(n,vVec)*river.depth*riverDepth)]",
  NULL, };

GXLPCSTR aOperStack_348[] = {
  // a?b:c?f:g
  "[:] [f] [g]",
  "[?] [c] [f:g]",
  "[:] [b] [c?f:g]",
  "[?] [a] [b:c?f:g]",
  NULL, };

GXLPCSTR aOperStack_349[] = {
  //a?b?d:e:c
  "[:] [d] [e]",
  "[?] [b] [d:e]",
  "[:] [b?d:e] [c]",
  "[?] [a] [b?d:e:c]",
  NULL, };

GXLPCSTR aOperStack_350[] = {
  //a?b?d:e:c?f:g
  "[:] [d] [e]",
  "[?] [b] [d:e]",
  "[:] [f] [g]",
  "[?] [c] [f:g]",
  "[:] [b?d:e] [c?f:g]",
  "[?] [a] [b?d:e:c?f:g]",
  NULL, };

GXLPCSTR aOperStack_360[] = {
  "[>] [sumWeight] [0.0]",
  "[F] [sqrt] [sumWeight]",
  "[/] [sumWeightedSlopes] [sqrt(sumWeight)]",
  "[.] [river] [wave]",
  "[.] [river.wave] [patternTex]",
  "[.] [p] [xy]",
  "[.] [river] [wave]",
  "[.] [river.wave] [length]",
  "[/] [p.xy] [river.wave.length]",
  "[,] [river.wave.patternTex] [p.xy/river.wave.length]",
  "[,] [river.wave.patternTex,p.xy/river.wave.length] [gradx]",
  "[,] [river.wave.patternTex,p.xy/river.wave.length,gradx] [grady]",
  "[F] [texture2DGrad] [river.wave.patternTex,p.xy/river.wave.length,gradx,grady]",
  "[.] [texture2DGrad(river.wave.patternTex,p.xy/river.wave.length,gradx,grady)] [xy]",
  "[:] [sumWeightedSlopes/sqrt(sumWeight)] [texture2DGrad(river.wave.patternTex,p.xy/river.wave.length,gradx,grady).xy]",
  "[?] [sumWeight>0.0] [sumWeightedSlopes/sqrt(sumWeight):texture2DGrad(river.wave.patternTex,p.xy/river.wave.length,gradx,grady).xy]",
  NULL, };

GXLPCSTR aOperStack_361[] = {
  "[<] [rmu] [0.0]",
  "[>] [delta] [0.0]",
  "[&&] [rmu<0.0] [delta>0.0]",
  "[,] [1.0] [0.0]",
  "[,] [1.0,0.0] [0.0]",
  "[F] [float] [RES_MU]",
  "[/] [0.5] [float(RES_MU)]",
  "[-] [0.5] [0.5/float(RES_MU)]",
  "[,] [1.0,0.0,0.0] [0.5-0.5/float(RES_MU)]",
  "[F] [float4] [1.0,0.0,0.0,0.5-0.5/float(RES_MU)]",
  "[-] [] [1.0]",
  "[*] [H] [H]",
  "[,] [-1.0] [H*H]",
  "[,] [-1.0,H*H] [H]",
  "[F] [float] [RES_MU]",
  "[/] [0.5] [float(RES_MU)]",
  "[+] [0.5] [0.5/float(RES_MU)]",
  "[,] [-1.0,H*H,H] [0.5+0.5/float(RES_MU)]",
  "[F] [float4] [-1.0,H*H,H,0.5+0.5/float(RES_MU)]",
  "[:] [float4(1.0,0.0,0.0,0.5-0.5/float(RES_MU))] [float4(-1.0,H*H,H,0.5+0.5/float(RES_MU))]",
  "[?] [rmu<0.0&&delta>0.0] [float4(1.0,0.0,0.0,0.5-0.5/float(RES_MU)):float4(-1.0,H*H,H,0.5+0.5/float(RES_MU))]",
  "[=] [cst] [rmu<0.0&&delta>0.0?float4(1.0,0.0,0.0,0.5-0.5/float(RES_MU)):float4(-1.0,H*H,H,0.5+0.5/float(RES_MU))]",
  NULL, };


GXLPCSTR aOperStack_362[] = {
  "[.] [viewdir] [z]",
  "[>] [viewdir.z] [0.0]",
  "[,] [0.5] [0.5]",
  "[F] [float2] [0.5,0.5]",
  "[.] [viewdir] [xy]",
  "[*] [viewdir.xy] [4.0]",
  "[+] [float2(0.5,0.5)] [viewdir.xy*4.0]",
  "[,] [glareSampler] [float2(0.5,0.5)+viewdir.xy*4.0]",
  "[F] [tex2D] [glareSampler,float2(0.5,0.5)+viewdir.xy*4.0]",
  "[.] [tex2D(glareSampler,float2(0.5,0.5)+viewdir.xy*4.0)] [rgb]",
  "[,] [0.0] [0.0]",
  "[,] [0.0,0.0] [0.0]",
  "[F] [float3] [0.0,0.0,0.0]",
  "[:] [tex2D(glareSampler,float2(0.5,0.5)+viewdir.xy*4.0).rgb] [float3(0.0,0.0,0.0)]",
  "[?] [viewdir.z>0.0] [tex2D(glareSampler,float2(0.5,0.5)+viewdir.xy*4.0).rgb:float3(0.0,0.0,0.0)]",
  "[=] [data] [viewdir.z>0.0?tex2D(glareSampler,float2(0.5,0.5)+viewdir.xy*4.0).rgb:float3(0.0,0.0,0.0)]",
  NULL, };

GXLPCSTR aOperStack_if001[] = {
  "[==] [a] [b]",
  "[+] [c] [d]",
  "[>] [c+d] [e]",
  "[&&] [a==b] [c+d>e]",
  "[=] [b] [c]",
  "[if] [a==b&&c+d>e] [b=c]",
  NULL, };

GXLPCSTR aOperStack_if004[] = {
  "[>] [a] [b]",
  "[=] [c] [a]",
  "[if] [a>b] [c=a]",
  "[<] [a] [b]",
  "[=] [c] [b]",
  "[if] [a<b] [c=b]",
  NULL, };

GXLPCSTR aOperStack_if006[] = {
  "[==] [a] [b]",
  "[+] [c] [d]",
  "[>] [c+d] [e]",
  "[&&] [a==b] [c+d>e]",
  "[=] [b] [c]",
  "[=] [a] [b]",
  "[else] [] [a=b]",
  "[if] [a==b&&c+d>e] [b=c]",
  NULL, };

GXLPCSTR aOperStack_if007[] = {
  "[==] [a] [b]",
  "[<] [c] [d]",
  "[&&] [a==b] [c<d]",
  "[=] [b] [c]",
  "[*] [4] [a]",
  "[=] [a] [4*a]",
  "[=] [a] [b]",
  "[else] [] [a=b]",
  "[if] [a==b&&c<d] [b=c;a=4*a]",
  NULL, };

GXLPCSTR aOperStack_if008[] = {
  "[==] [a] [b]",
  "[=] [r] [c]",
  "[>] [a] [b]",
  "[=] [r] [a]",
  "[=] [r] [b]",
  "[else] [] [r=b]",
  "[elif] [a>b] [r=a]",
  "[if] [a==b] [r=c]",
  NULL, };

GXLPCSTR aOperStack_if009[] = {
  "[==] [a] [b]",
  "[=] [b] [c]",
  "[*] [4] [a]",
  "[=] [a] [4*a]",
  "[>] [a] [b]",
  "[=] [a] [b]",
  "[=] [a] [c]",
  "[else] [] [a=c]",
  "[elif] [a>b] [a=b]",
  "[if] [a==b] [b=c;a=4*a]",
  NULL, };

//////////////////////////////////////////////////////////////////////////

GXLPCSTR aOperStack_OC001[] = {
  "[=] [i] [4]",
  "[=] [n] [10]",
  "[,] [i=4] [n=10]",
  NULL, };

GXLPCSTR aOperStack_OC003[] = {
  "[--] [n] []",
  "[-] [n--] [i]",
  NULL, };

GXLPCSTR aOperStack_OC004[] = {
  "[--] [] [i]",
  "[-] [n] [--i]",
  NULL, };


GXLPCSTR aOperStack_OC005[] = {
  "[--] [n] []",
  "[--] [] [i]",
  "[-] [n--] [--i]",
  NULL, };

#define _I_ __FILE__,__LINE__

SAMPLE_EXPRESSION samplesOpercode[] = {
  {0, _I_, "i=4, n=10",  7, aOperStack_OC001}, // 下面的例子就是这里的初始值
  {0, _I_, "n--i",       3},// err
  {0, _I_, "n- -i",      4},
  {0, _I_, "n---i",      4, aOperStack_OC003},// n=9, i=4, 结果6
  {0, _I_, "n-- -i",     4, aOperStack_OC003},// n=9, i=4, 结果6
  {0, _I_, "n- --i",     4, aOperStack_OC004},// n=10, i=3, 结果7
  {0, _I_, "n--- --i",   5, aOperStack_OC005},// n=9, i=3, 结果7
  {0, _I_, "n--- - --i", 6},// n=9, i=3, 结果13
  {0, _I_, ";", 1},
  {0, _I_, ";;", 2},
  {NULL},};

SAMPLE_EXPRESSION samplesNumeric[] = {
  // 整数
  {0, _I_, "10",  1},
  {0, _I_, "2e3", 1},
  //"-10",
  //"-2e3",

  // 十六进制
  {0, _I_, "0x123456", 1},

  // 八进制
  {0, _I_, "02314",  1},
  {0, _I_, "-02314", 2},

  // float
  {0, _I_, "1.5",           1},
  {0, _I_, "1.5f",          1},
  {0, _I_, "1.",            1},
  {0, _I_, ".5",            1},
  {0, _I_, ".5f",           1},
  {0, _I_, "1e3",           1},
  {0, _I_, "1e-3",          1},
  {0, _I_, "1.5e3",         1},
  {0, _I_, "1.5e-3",        1},
  {0, _I_, "1e3f",          1},
  {0, _I_, "1e-3f",         1},
  {0, _I_, "1.5e3f",        1},
  {0, _I_, "1.5e-3f",       1},
  {0, _I_, "1.e3f",         1},
  {0, _I_, "1.e-3f",        1},
  {0, _I_, ".5e3f",         1},
  {0, _I_, ".5e-3f",        1},

  // 负数float
  {0, _I_, "-1.5",          2},
  {0, _I_, "-1.5f",         2},
  {0, _I_, "-1.",           2},
  {0, _I_, "-.5",           2},
  {0, _I_, "-.5f",          2},
  {0, _I_, "-1e3",          2},
  {0, _I_, "-1e-3",         2},
  {0, _I_, "-1.5e3",        2},
  {0, _I_, "-1.5e-3",       2},
  {0, _I_, "-1e3f",         2},
  {0, _I_, "-1e-3f",        2},
  {0, _I_, "-1.5e3f",       2},
  {0, _I_, "-1.5e-3f",      2},
  {0, _I_, "-1.e3f",        2},
  {0, _I_, "-1.e-3f",       2},
  {0, _I_, "-.5e3f",        2},
  {0, _I_, "-.5e-3f",       2},
  {0, _I_, NULL, 0},};

SAMPLE_EXPRESSION samplesSimpleExpression[] = {
  // 基本表达式
  {0, _I_, "output.color", 3},

  // 数学表达式
  {0, _I_, "a+b*c", 5, aOperStack_008},
  {0, _I_, "(a+b)*c", 7, aOperStack_009},
  {0, _I_, "a+b+c+d*e/f", 11, aOperStack_010},
  {0, _I_, "k*((a*b)+c+d*e)", 15},

  // 三元操作
  {0, _I_, "a?b:c", 5},
  {0, _I_, "a>b?b:c", 7},
  {0, _I_, "a?b:c?f:g", 9, aOperStack_348},
  {0, _I_, "a?b?d:e:c", 9, aOperStack_349},
  {0, _I_, "a?b?d:e:c?f:g", 13, aOperStack_350},

  // 赋值
  {0, _I_, "a=b+c", 5},
  {0, _I_, "a=b=c=d", 7},
  {0, _I_, "float a", 2},
  {0, _I_, "float a=b+c", 6},
  {0, _I_, "float3 a=b=c=d", 8},
  {0, _I_, "float a,b,c,d,e", 10},
  {0, _I_, "float a, b = c", 6},
  {0, _I_, "(a)=a+b;", 0},
  {0, _I_, "(a=a+b);", 0},
  {0, _I_, "a=a+b;b=a-c*d;c=a*d;", 20},
  {0, "(a=a+b);(b=a-c*d);c=a*d;", 24},

  // 定义
  {0, _I_, "float2 texcoord : TEXCOORD0, pos : POSITION0", 8, NULL},
  {0, _I_, "float2 texcoord : TEXCOORD0 = float2(0,0), pos : POSITION0 = float2(320, 240)", 0, NULL},
  {0, _I_, "float freqs, time, frame, fps", 8, NULL},
  {0, _I_, "float freqs[8], time[4]", 10},
  {0, _I_, "float freqs[16]", 5},
  {0, _I_, "float freqs[16][8]", 8},
  //{0, _I_, "float freqs[] = {12, 3, 5}", 12},
  {0, _I_, NULL,  0},};

SAMPLE_EXPRESSION samplesIfExpression[] = {
  {0, _I_, "if(a == b && c + d > e)", 12, aOperStack_HasError}, // error
  {0, _I_, "if(a == b && c + d > e) b=c;", 16, aOperStack_if001},
  {0, _I_, "if(a == b && c + d > e) b=c; a=4*a;", 22},
  {0, _I_, "if(a == b && c + d > e) { b=c; a=4*a; }", 24},

  {0, _I_, "if(a > b) c = a; if(a < b) c = b;", 20, aOperStack_if004},

  {0, _I_, "if(a > b) else a = b;", 11, aOperStack_HasError}, // error
  {0, _I_, "if(a == b && c + d > e) b=c; else a = b;", 21, aOperStack_if006},
  {0, _I_, "if(a == b && c + d > e) b=c; a=4*a; else a = b;", 27, aOperStack_HasError}, // error
  {0, _I_, "if(a == b && c < d) { b=c; a=4*a; } else a = b;", 27, aOperStack_if007},

  {0, _I_, "if(a == b) else if(a > b) a = b;", 17, aOperStack_HasError}, // error
  {0, _I_, "if(a == b) r=c; else if(a > b) r = a; else r = b;", 26, aOperStack_if008},
  {0, _I_, "if(a == b) b=c; a=4*a; else if(a > b) a = b else a = b;", 31, aOperStack_HasError}, // error
  {0, _I_, "if(a == b) { b=c; a=4*a; } else if(a > b) a = b; else a = c;", 34, aOperStack_if009},
  {0, _I_, "while(a - b * c)", 8, aOperStack_HasError},
  //{0, "switch(a - b * c)", 8},
  {0, _I_, NULL,  0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
};

//////////////////////////////////////////////////////////////////////////


GXLPCSTR aOperStack_FOR000[] = {
  "[for_2] [] []",
  "[for_1] [] []",
  NULL};

GXLPCSTR aOperStack_FOR001[] = {
  "[=] [inti] [0]",
  "[<] [i] [10]",
  "[++] [i] []",
  "[*] [b] [c]",
  "[+] [a] [b*c]",
  "[=] [n] [a+b*c]",
  "[for_2] [i<10] [i++]",
  "[for_1] [inti=0] [n=a+b*c]",
  NULL};

GXLPCSTR aOperStack_FOR002[] = {
  "[=] [inti] [0]",
  "[<] [i] [10]",
  "[++] [i] []",
  "[*] [b] [c]",
  "[+] [a] [b*c]",
  "[=] [n] [a+b*c]",
  "[for_2] [i<10] [i++]",
  "[for_1] [inti=0] [n=a+b*c]",
  NULL};

//GXLPCSTR* aOperStack_FOR003 = aOperStack_FOR002;

GXLPCSTR aOperStack_FOR004[] = {
  "[=] [inti] [0]",
  "[=] [intn] [10]",
  "[,] [inti=0] [intn=10]",
  "[<] [i] [10]",
  "[>=] [n] [0]",
  "[,] [i<10] [n>=0]",
  "[++] [i] []",
  "[--] [] [n]",
  "[,] [i++] [--n]",
  "[*] [b] [c]",
  "[+] [a] [b*c]",
  "[+] [a+b*c] [n]",
  "[=] [r] [a+b*c+n]",
  "[<] [n] [5]",
  "[F] [out] [n]",
  "[if] [n<5] [out(n)]",
  "[for_2] [i<10,n>=0] [i++,--n]",
  "[for_1] [inti=0,intn=10] [r=a+b*c+n;if(n<5){out(n);}]",
  NULL};


SAMPLE_EXPRESSION samplesForExpression[] = {
  //{0, ";;;;;",    0},
  {0, _I_, "for(;;);", 0, aOperStack_FOR000},
  {0, _I_, "for(int i = 0; i < 10; i++) n = a+b*c;", 0, aOperStack_FOR001},
  {0, _I_, "for(int i = 0; i < 10; i++) n = a+b*c;", 0, aOperStack_FOR002},
  {0, _I_, "for(int i = 0; i < 10; i++) {n = a+b*c;}", 0, aOperStack_FOR002},
  {0, _I_, "for(int i = 0, int n = 10; i < 10, n >= 0; i++, --n) {r = a+b*c + n; if(n < 5) { out(n);} }", 0, aOperStack_FOR004},
  {0, _I_, NULL,  0},
};

SAMPLE_EXPRESSION samplesExpression[] = {
  {0, _I_, "(Output.LdotN*shadowFactor)+Output.Ambient+Output.Specular*shadowFactor", 17},
  {0, _I_, "Input.Normal = (Input.Normal - 0.5) * 2.0", 13},
  {0, _I_, "Input.Position.xyz += SwingGrass(Input.Position.xyz, Input.Texcoord.y - 0.1875)", 22},
  {0, _I_, "float  spec = max(0, dot(normalize(Input.vViewDir), normalize(vRLight)))", 22},
  {0, _I_, "const float fAdjustFactor = 255.0 / 256.0 * 2.0", 9},
  {0, _I_, "float SR = ( 1.05f - pow( V.y, 0.3f ) ) * 1000", 17},
  {0, _I_, "float3 L = Lin( pow(max(0, Theta), 10.0), SR, SM )", 21, aOperStack_016},
  {0, _I_, "Output.color.rgb = L * g_vLightDiffuse * g_fSunIntensity", 11, aOperStack_017},
  {0, _I_, "Output.I.rgb = (1.0f - Output.E.rgb) * I( Theta ) * g_vLightDiffuse.xyz * g_fSunIntensity", 26, aOperStack_018},
  {0, _I_, "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient + Output.Specular * shadowFactor)", 26},
  {0, _I_, "float4 c = Output.Diffuse * (Output.LdotN + Output.Ambient);", 17},
  {0, _I_, "float4 Diffuse = tex2D(MainSampler, Input.TexUV) * Input.Color", 15},
  {0, _I_, "Output.uvSM.x = (Output.uvSM.x + Output.uvSM.w) * 0.5", 21},
  {0, _I_, "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)", 25},
  {0, _I_, "vec2 slopesVal = texture2DGrad(river.wave.patternTex, rCoord / river.wave.length, gradx, grady).xy", 25},
  {0, _I_, "vec3 fn = vec3(textureTile(fragmentNormalSampler, uv).xy * 2.0 - 1.0, 0.0)", 20},
  {0, _I_, "slopes = texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy", 26},
  {0, _I_, "riverColor = riverShallowColor + groundColor * texture2DGrad(river.bed.patternTex, normal.xy * river.depth / 10.0 + p.xy / (river.bed.length / 2.0), gradx, grady).xyz / (dot(n, vVec) * river.depth * riverDepth)", 59, aOperStack_033},
  {0, _I_, "vec2 v = abs(st * river.gridSize / river.screenSize - floor(st * river.gridSize / river.screenSize) - 0.5)", 30},
  {0, _I_, "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)", 33},
  {0, _I_, "float3 inscatter = skyRadiance(WCP + origin, d, WSD, extinction, 0.0)", 17},
  {0, _I_, "float4 c = Output.Diffuse * ((Output.LdotN * shadowFactor) + Output.Ambient)", 20},
  {0, _I_, "float blend = clamp((d - deformation.blending.x) / deformation.blending.y, 0.0, 1.0)", 25},
  {0, _I_, "groundColor = treeBrdf(q, d, lcc, v, fn, WSD, vec3(0.0, 0.0, 1.0), reflectance, sunL, skyE)", 31},
  {0, _I_, "data.r += mod(dot(floor(deformation.offset.xy / deformation.offset.z + 0.5), vec2(1.0)), 2.0)", 33},
  {0, _I_, "float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU))", 42},
  {0, _I_, "float uMuS = 0.5 / float(RES_MU_S) + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(RES_MU_S))", 50},
  {0, _I_, "return tex3D(table, float3((uNu + uMuS      ) / float(RES_NU), uMu, uR)) * (1.0 - lerp) +  tex3D(table, float3((uNu + uMuS + 1.0) / float(RES_NU), uMu, uR)) * lerp", 56},
  {0, _I_, "float2 y = a01s / (2.3193*abs(a01) + sqrt(1.52*a01sq + 4.0)) * float2(1.0, exp(-d/H*(d/(2.0*r)+mu)))", 47},
  {0, _I_, "m_fHue = m_fSaturation = m_fValue = 0.0",    7},

  {0, _I_, "float4 cst = rmu < 0.0 && delta > 0.0 ? float4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(RES_MU)) : float4(-1.0, H * H, H, 0.5 + 0.5 / float(RES_MU))", 49, aOperStack_361},
  {0, _I_, "float3 data = viewdir.z > 0.0 ? tex2D(glareSampler, float2(0.5,0.5) + viewdir.xy * 4.0).rgb : float3(0.0,0.0,0.0)", 37, aOperStack_362},
  {0, _I_, NULL,  0},
  {0, _I_, "return mix(a.x, a.y, f.z);", 0, aOperStack_Return0100},
  {0, _I_, "return sumWeight > 0.0 ? sumWeightedSlopes / sqrt(sumWeight) : texture2DGrad(river.wave.patternTex, p.xy / river.wave.length, gradx, grady).xy", 36, aOperStack_360},
  {0, _I_, "return 1.5 * 1.0 / (4.0 * M_PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*mu, -3.0/2.0) * (1.0 + mu * mu) / (2.0 + mieG*mieG)", 56},
  {0, _I_, "return float4(  c.xyz * Input.E.xyz  + Input.I.xyz, Output.Diffuse.w)", 25},
  {0, _I_, "return (vBetaMieTheta * pow((1.0f - g), 2.0)) / (pow(abs(1 + g * g - 2 * g * Theta), 1.5))", 37},
  {0, _I_, "return vBetaRayTheta * (2.0f + 0.5f * Theta * Theta)", 12},
  {0, _I_, "return (BetaR(Theta) + BetaM(Theta)) / (vBetaRay + vBetaMie)", 18},
  {0, _I_, "return exp( -(vBetaRay * SR + vBetaMie * SM))", 14},
  {0, _I_, "return ((BetaR(Theta) + BetaM(Theta)) * (1.0f - exp(-(vBetaRay * SR + vBetaMie * SM)))) / (vBetaRay + vBetaMie)", 38},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, "",    0},
  {0, _I_, NULL,  0},
};
