#if defined(__cplusplus) 
#define DECLARE_VERTEX_COMM_REGISTER(TYPE, NAME, IDX)  const unsigned long FXCR_##V_##NAME##_ = IDX
#define DECLARE_PIXEL_COMM_REGISTER(TYPE, NAME, IDX)  const unsigned long FXCR_##P_##NAME##_ = IDX
#define FXVCOMMREGIDX(NAME)  FXCR_##V_##NAME##_
#define FXPCOMMREGIDX(NAME)  FXCR_##P_##NAME##_

#define FXVCOMMREG(NAME)  (m_aVertexShaderRegister[FXVCOMMREGIDX(NAME)])
#define FXPCOMMREG(NAME)  (m_aPixelShaderRegister[FXPCOMMREGIDX(NAME)])
#else
#define DECLARE_VERTEX_COMM_REGISTER(TYPE, NAME, IDX)  TYPE NAME : register(c##IDX)
#define DECLARE_PIXEL_COMM_REGISTER(TYPE, NAME, IDX)  TYPE NAME : register(c##IDX)
#endif

// Vertex Shader Register
DECLARE_VERTEX_COMM_REGISTER(float4x4,  matWVProj, 0);

// Pixel Shader Register
DECLARE_PIXEL_COMM_REGISTER(float4, Color,          0);
DECLARE_PIXEL_COMM_REGISTER(float4, ColorAdd,       1);
DECLARE_PIXEL_COMM_REGISTER(float2, HalfTexelKernel,2);
DECLARE_PIXEL_COMM_REGISTER(float2, TexelKernel,    3);

//struct _XXOO
//{
//  float4x4  matWVProj;
//  float4    crColor;
//  float4    crColorAdd;
//  float2    HalfTexelKernel;
//  float2    TexelKernel;
//};
//
