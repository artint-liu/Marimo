#include "GrapX.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GResource.H"
#include "GrapX/GShader.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GTexture.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GStateBlock.H"
#include "GXMaterialImpl.h"
#include "GrapX/StdMtl.h"
#include "GrapX/gxError.H"
#include "Smart/smartstream.h"
#include "smart/SmartRepository.h"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
//#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"

using namespace clstd;

// 这个 MOGenerateDeclarationCodes 生成代码要用
DATALAYOUT g_StandardMtl[] =
{
  {"g_matViewProj",             MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewProj),             GXUB_MATRIX4, sizeof(float4x4)},// ViewProjection;
  {"g_matViewProjInv",          MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewProjInv),          GXUB_MATRIX4, sizeof(float4x4)},// ViewProjectionInverse;

  {"g_matView",                 MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matView),                 GXUB_MATRIX4, sizeof(float4x4)},// View;
  {"g_matViewInv",              MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewInv),              GXUB_MATRIX4, sizeof(float4x4)},// ViewInverse;

  {"g_matProj",                 MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matProj),                 GXUB_MATRIX4, sizeof(float4x4)},// Projection;
  {"g_matProjInv",              MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matProjInv),              GXUB_MATRIX4, sizeof(float4x4)},// ProjectionInverse;

  {"g_matWorldViewProj",        MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewProj),        GXUB_MATRIX4, sizeof(float4x4)},// WorldViewProjection;
  {"g_matWorldViewProjInv",     MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewProjInv),     GXUB_MATRIX4, sizeof(float4x4)},// WorldViewProjectionInverse;

  {"g_matWorld",                MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorld),                GXUB_MATRIX4, sizeof(float4x4)},// World;
  {"g_matWorldInv",             MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldInv),             GXUB_MATRIX4, sizeof(float4x4)},// WorldInverse;

  {"g_matWorldView",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldView),            GXUB_MATRIX4, sizeof(float4x4)},// WorldView;
  {"g_matWorldViewInv",         MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewInv),         GXUB_MATRIX4, sizeof(float4x4)},// WorldViewInverse;

//{"g_fTime0_X",                MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fTime0_X),                GXUB_FLOAT, sizeof(float)},     // Time0_X;
  {"g_vTime",                   MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vTime),                   GXUB_FLOAT4, sizeof(float4)},   // vTime
  {"g_fFPS",                    MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fFPS),                    GXUB_FLOAT, sizeof(float)},     // FPS;
  {"g_fFOV",                    MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fFOV),                    GXUB_FLOAT, sizeof(float)},     // FOV;

  {"g_vViewportDim",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vViewportDim),            GXUB_FLOAT2, sizeof(float2)},   // ViewportDimensions;
  {"g_vViewportDimInv",         MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vViewportDimInv),         GXUB_FLOAT2, sizeof(float2)},   // ViewportDimensionsInverse;
  {"g_fFarClipPlane",           MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fFarClipPlane),           GXUB_FLOAT, sizeof(float)},     // FarClipPlane;
  {"g_fNearClipPlane",          MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fNearClipPlane),          GXUB_FLOAT, sizeof(float)},     // NearClipPlane;

  {"g_fMouseCoordX",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fMouseCoordX),            GXUB_FLOAT, sizeof(float)},     // MouseCoordinateX;
  {"g_fMouseCoordY",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fMouseCoordY),            GXUB_FLOAT, sizeof(float)},     // MouseCoordinateY;
  {"g_vViewDir",                MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vViewDir),                GXUB_FLOAT3, sizeof(float3)},   // ViewDirection;
  {"g_vViewPos",                MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vViewPos),                GXUB_FLOAT3, sizeof(float3)},   // ViewPosition;

//{"g_matViewProjInvTran",      MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewProjInvTran),      sizeof(float4x4)},// ViewProjectionInverseTranspose;
//{"g_matViewProjTran",         MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewProjTran),         sizeof(float4x4)},// ViewProjectionTranspose;
//{"g_matViewTran",             MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewTran),             sizeof(float4x4)},// ViewTranspose;
//{"g_matViewInvTran",          MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matViewInvTran),          sizeof(float4x4)},// ViewInverseTranspose;
//{"g_matProjTran",             MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matProjTran),             sizeof(float4x4)},// ProjectionTranspose;
//{"g_matProjInvTran",          MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matProjInvTran),          sizeof(float4x4)},// ProjectionInverseTranspose;
//{"g_matWorldViewProjTran",    MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewProjTran),    sizeof(float4x4)},// WorldViewProjectionTranspose;
//{"g_matWorldViewProjInvTran", MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewProjInvTran), sizeof(float4x4)},// WorldViewProjectionInverseTranspose;
//{"g_matWorldTran",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldTran),            sizeof(float4x4)},// WorldTranspose;
//{"g_matWorldInvTran",         MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldInvTran),         sizeof(float4x4)},// WorldInverseTranspose;
//{"g_matWorldViewTran",        MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewTran),        sizeof(float4x4)},// WorldViewTranspose;
//{"g_matWorldViewInvTran",     MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matWorldViewInvTran),     sizeof(float4x4)},// WorldViewInverseTranspose;
  
  {"g_matMainLight",            MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_matMainLight),            GXUB_MATRIX4, sizeof(float4x4)},   //
  {"g_vMainLightDir",           MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vMainLightDir),           GXUB_FLOAT3, sizeof(float3)},   //
  {"g_vLightDiffuse",           MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vLightDiffuse),           GXUB_FLOAT3, sizeof(float3)},   //
  {"g_fLightIntensity",         MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_fLightIntensity),         GXUB_FLOAT,  sizeof(float)},   //
  {"g_vLightAmbient",           MEMBER_OFFSET(STANDARDMTLUNIFORMTABLE, g_vLightAmbient),           GXUB_FLOAT3, sizeof(float3)},   //

  {NULL},
};


//ScriptedDataPool::VARIABLE_DECLARATION g_StdMtl[] = 
//{
//  {"float4x4", "g_matViewProj",         },    // ViewProjection
//  {"float4x4", "g_matViewProjInv",      },    // ViewProjectionInverse",
//  {"float4x4", "g_matView",             },    // View",
//  {"float4x4", "g_matViewInv",          },    // ViewInverse",
//  {"float4x4", "g_matProj",             },    // Projection",
//  {"float4x4", "g_matProjInv",          },    // ProjectionInverse",
//  {"float4x4", "g_matWorldViewProj",    },    // WorldViewProjection",
//  {"float4x4", "g_matWorldViewProjInv", },    // WorldViewProjectionInverse",
//  {"float4x4", "g_matWorld",            },    // World",
//  {"float4x4", "g_matWorldInv",         },    // WorldInverse",
//  {"float4x4", "g_matWorldView",        },    // WorldView",
//  {"float4x4", "g_matWorldViewInv",     },    // WorldViewInverse",
//  {"float",    "g_fTime0_X",            },    // Time0_X",
//  {"float",    "g_fFPS",                },    // FPS",
//  {"float",    "g_fFOV",                },    // FOV",
//  {"float2",   "g_vViewportDim",        },    // ViewportDimensions",
//  {"float2",   "g_vViewportDimInv",     },    // ViewportDimensionsInverse",
//  {"float",    "g_fFarClipPlane",       },    // FarClipPlane",
//  {"float",    "g_fNearClipPlane",      },    // NearClipPlane",
//  {"float",    "g_fMouseCoordX",        },    // MouseCoordinateX",
//  {"float",    "g_fMouseCoordY",        },    // MouseCoordinateY",
//  {"float3",   "g_vViewDir",            },    // ViewDirection",
//  {"float3",   "g_vViewPos",            },    // ViewPosition",
//  {"float3",   "g_vMainLightDir",       },    //
//  {"float3",   "g_vLightDiffuse",       },    //
//  {"float3",   "g_vLightAmbient",       },    // 环境光颜色
//  {NULL, NULL}
//};

/*
GXDLL char* g_szStandardMtlUniformTable =
"always struct STANDARDMTLUNIFORMTABLE\n"
"{\n"
"  float4x4 g_matViewProj;               // ViewProjection;                     \n"
"  float4x4 g_matViewProjInv;            // ViewProjectionInverse;              \n"
"                                                                               \n"
"  float4x4 g_matView;                   // View;                               \n"
"  float4x4 g_matViewInv;                // ViewInverse;                        \n"
"                                                                               \n"
"  float4x4 g_matProj;                   // Projection;                         \n"
"  float4x4 g_matProjInv;                // ProjectionInverse;                  \n"
"                                                                               \n"
"  float4x4 g_matWorldViewProj;          // WorldViewProjection;                \n"
"  float4x4 g_matWorldViewProjInv;       // WorldViewProjectionInverse;         \n"
"                                                                               \n"
"  float4x4 g_matWorld;                  // World;                              \n"
"  float4x4 g_matWorldInv;               // WorldInverse;                       \n"
"                                                                               \n"
"  float4x4 g_matWorldView;              // WorldView;                          \n"
"  float4x4 g_matWorldViewInv;           // WorldViewInverse;                   \n"
"                                                                               \n"
"  float4x4 g_matMainLight;              // 主光空间的变换矩阵                     \n"
"                                                                               \n"
"  //float4x4 g_matViewProjInvTran;      // ViewProjectionInverseTranspose;     \n"
"  //float4x4 g_matViewProjTran;         // ViewProjectionTranspose;            \n"
"  //float4x4 g_matViewTran;             // ViewTranspose;                      \n"
"  //float4x4 g_matViewInvTran;          // ViewInverseTranspose;               \n"
"  //float4x4 g_matProjTran;             // ProjectionTranspose;                \n"
"  //float4x4 g_matProjInvTran;          // ProjectionInverseTranspose;         \n"
"  //float4x4 g_matWorldViewProjTran;    // WorldViewProjectionTranspose;       \n"
"  //float4x4 g_matWorldViewProjInvTran; // WorldViewProjectionInverseTranspose;\n"
"  //float4x4 g_matWorldTran;            // WorldTranspose;                     \n"
"  //float4x4 g_matWorldInvTran;         // WorldInverseTranspose;              \n"
"  //float4x4 g_matWorldViewTran;        // WorldViewTranspose;                 \n"
"  //float4x4 g_matWorldViewInvTran;     // WorldViewInverseTranspose;          \n"
"                                                                               \n"
//"  float   g_fTime0_X;                   // Time0_X;                            \n"
"  float4  g_vTime;                      // vTime;                              \n"
"  float   g_fFPS;                       // FPS;                                \n"
"  float   g_fFOV;                       // FOV;                                \n"
"                                                                               \n"
"  float2  g_vViewportDim;               // ViewportDimensions;                 \n"
"  float2  g_vViewportDimInv;            // ViewportDimensionsInverse;          \n"
"  float   g_fFarClipPlane;              // FarClipPlane;                       \n"
"  float   g_fNearClipPlane;             // NearClipPlane;                      \n"
"                                                                               \n"
"  float   g_fMouseCoordX;               // MouseCoordinateX;                   \n"
"  float   g_fMouseCoordY;               // MouseCoordinateY;                   \n"
"                                                                               \n"
"  float3  g_vViewDir;                   // ViewDirection;                      \n"
"  float3  g_vViewPos;                   // ViewPosition;                       \n"
"  float3  g_vMainLightDir;              //                                     \n"
"  float3  g_vLightDiffuse;              //                                     \n"
"  float3  g_vLightAmbient;              // 环境光颜色                            \n"
"};"

"struct GXCANVASCOMMCONST \n"
"{                        \n"
"  float4x4  matWVProj;   \n"
"  float4    colorMul;    \n"
"  float4    colorAdd;    \n"
"};                       \n"

"STANDARDMTLUNIFORMTABLE StdMtlUniform;"
"GXCANVASCOMMCONST CanvasUniform;";
//*/

GXMaterialInstImpl::MtlStateDict GXMaterialInstImpl::s_MtlStateDict;

GXVOID GXMaterialInstImpl::InitializeMtlStateDict()
{
  s_MtlStateDict["$BLENDENABLE"]              = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, BlendEnable);
  s_MtlStateDict["BLENDENABLE_TRUE"]          = 1;
  s_MtlStateDict["BLENDENABLE_FALSE"]         = 0;
  s_MtlStateDict["BLENDENABLE_1"]             = 1;
  s_MtlStateDict["BLENDENABLE_0"]             = 0;
  s_MtlStateDict["BLENDENABLE_YES"]           = 1;
  s_MtlStateDict["BLENDENABLE_NO"]            = 0;

  //s_MtlStateDict["$ALPHATEST"]                = 

  s_MtlStateDict["$SRCBLEND"]                 = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, SrcBlend);
  s_MtlStateDict["$DESTBLEND"]                = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, DestBlend);
  s_MtlStateDict["SRCBLEND_ZERO"]             = GXBLEND_ZERO;
  s_MtlStateDict["SRCBLEND_ONE"]              = GXBLEND_ONE;
  s_MtlStateDict["SRCBLEND_SRCCOLOR"]         = GXBLEND_SRCCOLOR;
  s_MtlStateDict["SRCBLEND_INVSRCCOLOR"]      = GXBLEND_INVSRCCOLOR;
  s_MtlStateDict["SRCBLEND_SRCALPHA"]         = GXBLEND_SRCALPHA;    
  s_MtlStateDict["SRCBLEND_INVSRCALPHA"]      = GXBLEND_INVSRCALPHA;
  s_MtlStateDict["SRCBLEND_DESTALPHA"]        = GXBLEND_DESTALPHA;     
  s_MtlStateDict["SRCBLEND_INVDESTALPHA"]     = GXBLEND_INVDESTALPHA;  
  s_MtlStateDict["SRCBLEND_DESTCOLOR"]        = GXBLEND_DESTCOLOR;
  s_MtlStateDict["SRCBLEND_INVDESTCOLOR"]     = GXBLEND_INVDESTCOLOR;  
  s_MtlStateDict["SRCBLEND_SRCALPHASAT"]      = GXBLEND_SRCALPHASAT;   
  s_MtlStateDict["SRCBLEND_BOTHSRCALPHA"]     = GXBLEND_BOTHSRCALPHA;  
  s_MtlStateDict["SRCBLEND_BOTHINVSRCALPHA"]  = GXBLEND_BOTHINVSRCALPHA;
  s_MtlStateDict["SRCBLEND_BLENDFACTOR"]      = GXBLEND_BLENDFACTOR;   
  s_MtlStateDict["SRCBLEND_INVBLENDFACTOR"]   = GXBLEND_INVBLENDFACTOR;
  s_MtlStateDict["SRCBLEND_SRCCOLOR2"]        = GXBLEND_SRCCOLOR2;     
  s_MtlStateDict["SRCBLEND_INVSRCCOLOR2"]     = GXBLEND_INVSRCCOLOR2;  

  s_MtlStateDict["DESTBLEND_ZERO"]            = GXBLEND_ZERO;
  s_MtlStateDict["DESTBLEND_ONE"]             = GXBLEND_ONE;
  s_MtlStateDict["DESTBLEND_SRCCOLOR"]        = GXBLEND_SRCCOLOR;
  s_MtlStateDict["DESTBLEND_INVSRCCOLOR"]     = GXBLEND_INVSRCCOLOR;
  s_MtlStateDict["DESTBLEND_SRCALPHA"]        = GXBLEND_SRCALPHA;    
  s_MtlStateDict["DESTBLEND_INVSRCALPHA"]     = GXBLEND_INVSRCALPHA;
  s_MtlStateDict["DESTBLEND_DESTALPHA"]       = GXBLEND_DESTALPHA;     
  s_MtlStateDict["DESTBLEND_INVDESTALPHA"]    = GXBLEND_INVDESTALPHA;  
  s_MtlStateDict["DESTBLEND_DESTCOLOR"]       = GXBLEND_DESTCOLOR;
  s_MtlStateDict["DESTBLEND_INVDESTCOLOR"]    = GXBLEND_INVDESTCOLOR;  
  s_MtlStateDict["DESTBLEND_SRCALPHASAT"]     = GXBLEND_SRCALPHASAT;   
  s_MtlStateDict["DESTBLEND_BOTHSRCALPHA"]    = GXBLEND_BOTHSRCALPHA;  
  s_MtlStateDict["DESTBLEND_BOTHINVSRCALPHA"] = GXBLEND_BOTHINVSRCALPHA;
  s_MtlStateDict["DESTBLEND_BLENDFACTOR"]     = GXBLEND_BLENDFACTOR;   
  s_MtlStateDict["DESTBLEND_INVBLENDFACTOR"]  = GXBLEND_INVBLENDFACTOR;
  s_MtlStateDict["DESTBLEND_SRCCOLOR2"]       = GXBLEND_SRCCOLOR2;     
  s_MtlStateDict["DESTBLEND_INVSRCCOLOR2"]    = GXBLEND_INVSRCCOLOR2;  

  s_MtlStateDict["$BLENDOP"]                  = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, BlendOp);
  s_MtlStateDict["BLENDOP_ADD"]               = GXBLENDOP_ADD;
  s_MtlStateDict["BLENDOP_SUBTRACT"]          = GXBLENDOP_SUBTRACT;
  s_MtlStateDict["BLENDOP_REVSUBTRACT"]       = GXBLENDOP_REVSUBTRACT;
  s_MtlStateDict["BLENDOP_MIN"]               = GXBLENDOP_MIN;
  s_MtlStateDict["BLENDOP_MAX"]               = GXBLENDOP_MAX;

  s_MtlStateDict["$FILLMODE"]                 = offsetof(RENDERSTATE, RasterizerDesc) + offsetof(GXRASTERIZERDESC, FillMode);
  s_MtlStateDict["FILLMODE_POINT"]            = GXFILL_POINT;
  s_MtlStateDict["FILLMODE_WIREFRAME"]        = GXFILL_WIREFRAME;
  s_MtlStateDict["FILLMODE_SOLID"]            = GXFILL_SOLID;

  s_MtlStateDict["$CULLMODE"]                 = offsetof(RENDERSTATE, RasterizerDesc) + offsetof(GXRASTERIZERDESC, CullMode);
  s_MtlStateDict["CULLMODE_NONE"]             = GXCULL_NONE;
  s_MtlStateDict["CULLMODE_CW"]               = GXCULL_CW;
  s_MtlStateDict["CULLMODE_CCW"]              = GXCULL_CCW;

  s_MtlStateDict["$DEPTHENABLE"]              = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthEnable);
  s_MtlStateDict["DEPTHENABLE_TRUE"]          = 1;
  s_MtlStateDict["DEPTHENABLE_FALSE"]         = 0;
  s_MtlStateDict["DEPTHENABLE_1"]             = 1;
  s_MtlStateDict["DEPTHENABLE_0"]             = 0;
  s_MtlStateDict["DEPTHENABLE_YES"]           = 1;
  s_MtlStateDict["DEPTHENABLE_NO"]            = 0;

  s_MtlStateDict["$DEPTHWRITE"]               = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthWriteMask);
  s_MtlStateDict["DEPTHWRITE_TRUE"]           = 1;
  s_MtlStateDict["DEPTHWRITE_FALSE"]          = 0;
  s_MtlStateDict["DEPTHWRITE_1"]              = 1;
  s_MtlStateDict["DEPTHWRITE_0"]              = 0;
  s_MtlStateDict["DEPTHWRITE_YES"]            = 1;
  s_MtlStateDict["DEPTHWRITE_NO"]             = 0;


  s_MtlStateDict["$DEPTHFUNC"]                = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthFunc);
  s_MtlStateDict["DEPTHFUNC_NEVER"]           = GXCMP_NEVER;
  s_MtlStateDict["DEPTHFUNC_LESS"]            = GXCMP_LESS;
  s_MtlStateDict["DEPTHFUNC_EQUAL"]           = GXCMP_EQUAL;
  s_MtlStateDict["DEPTHFUNC_LESSEQUAL"]       = GXCMP_LESSEQUAL;
  s_MtlStateDict["DEPTHFUNC_GREATER"]         = GXCMP_GREATER;
  s_MtlStateDict["DEPTHFUNC_NOTEQUAL"]        = GXCMP_NOTEQUAL;
  s_MtlStateDict["DEPTHFUNC_GREATEREQUAL"]    = GXCMP_GREATEREQUAL;
  s_MtlStateDict["DEPTHFUNC_ALWAYS"]          = GXCMP_ALWAYS;
  

  s_MtlStateDict["$ADDRESSU"]                 = offsetof(GXSAMPLERDESC, AddressU);
  s_MtlStateDict["$ADDRESSV"]                 = offsetof(GXSAMPLERDESC, AddressV);
  s_MtlStateDict["$ADDRESSW"]                 = offsetof(GXSAMPLERDESC, AddressW);
  s_MtlStateDict["$MAGFILTER"]                = offsetof(GXSAMPLERDESC, MagFilter);
  s_MtlStateDict["$MINFILTER"]                = offsetof(GXSAMPLERDESC, MinFilter);
  s_MtlStateDict["$MIPFILTER"]                = offsetof(GXSAMPLERDESC, MipFilter);
  //s_MtlStateDict["$BORDERCOLOR"]            = GXSAMP_BORDERCOLOR;

  s_MtlStateDict["ADDRESSU_WRAP"]             = GXTADDRESS_WRAP;
  s_MtlStateDict["ADDRESSU_MIRROR"]           = GXTADDRESS_MIRROR;
  s_MtlStateDict["ADDRESSU_CLAMP"]            = GXTADDRESS_CLAMP;
  s_MtlStateDict["ADDRESSU_BORDER"]           = GXTADDRESS_BORDER;
  s_MtlStateDict["ADDRESSU_MIRRORONCE"]       = GXTADDRESS_MIRRORONCE;

  s_MtlStateDict["ADDRESSV_WRAP"]             = GXTADDRESS_WRAP;
  s_MtlStateDict["ADDRESSV_MIRROR"]           = GXTADDRESS_MIRROR;
  s_MtlStateDict["ADDRESSV_CLAMP"]            = GXTADDRESS_CLAMP;
  s_MtlStateDict["ADDRESSV_BORDER"]           = GXTADDRESS_BORDER;
  s_MtlStateDict["ADDRESSV_MIRRORONCE"]       = GXTADDRESS_MIRRORONCE;

  s_MtlStateDict["ADDRESSW_WRAP"]             = GXTADDRESS_WRAP;
  s_MtlStateDict["ADDRESSW_MIRROR"]           = GXTADDRESS_MIRROR;
  s_MtlStateDict["ADDRESSW_CLAMP"]            = GXTADDRESS_CLAMP;
  s_MtlStateDict["ADDRESSW_BORDER"]           = GXTADDRESS_BORDER;
  s_MtlStateDict["ADDRESSW_MIRRORONCE"]       = GXTADDRESS_MIRRORONCE;

  s_MtlStateDict["MAGFILTER_NONE"]            = GXTEXFILTER_NONE;
  s_MtlStateDict["MAGFILTER_POINT"]           = GXTEXFILTER_POINT;
  s_MtlStateDict["MAGFILTER_LINEAR"]          = GXTEXFILTER_LINEAR;
  s_MtlStateDict["MAGFILTER_ANISOTROPIC"]     = GXTEXFILTER_ANISOTROPIC;
  s_MtlStateDict["MAGFILTER_PYRAMIDALQUAD"]   = GXTEXFILTER_PYRAMIDALQUAD;
  s_MtlStateDict["MAGFILTER_GAUSSIANQUAD"]    = GXTEXFILTER_GAUSSIANQUAD;

  s_MtlStateDict["MINFILTER_NONE"]            = GXTEXFILTER_NONE;
  s_MtlStateDict["MINFILTER_POINT"]           = GXTEXFILTER_POINT;
  s_MtlStateDict["MINFILTER_LINEAR"]          = GXTEXFILTER_LINEAR;
  s_MtlStateDict["MINFILTER_ANISOTROPIC"]     = GXTEXFILTER_ANISOTROPIC;
  s_MtlStateDict["MINFILTER_PYRAMIDALQUAD"]   = GXTEXFILTER_PYRAMIDALQUAD;
  s_MtlStateDict["MINFILTER_GAUSSIANQUAD"]    = GXTEXFILTER_GAUSSIANQUAD;

  s_MtlStateDict["MIPFILTER_NONE"]            = GXTEXFILTER_NONE;
  s_MtlStateDict["MIPFILTER_POINT"]           = GXTEXFILTER_POINT;
  s_MtlStateDict["MIPFILTER_LINEAR"]          = GXTEXFILTER_LINEAR;
  s_MtlStateDict["MIPFILTER_ANISOTROPIC"]     = GXTEXFILTER_ANISOTROPIC;
  s_MtlStateDict["MIPFILTER_PYRAMIDALQUAD"]   = GXTEXFILTER_PYRAMIDALQUAD;
  s_MtlStateDict["MIPFILTER_GAUSSIANQUAD"]    = GXTEXFILTER_GAUSSIANQUAD;
}

GXVOID GXMaterialInstImpl::FinalizeMtlStateDict()
{
  s_MtlStateDict.clear();
}

GXMaterialInstImpl::GXMaterialInstImpl(GXGraphics* pGraphics)
  : GXMaterialInst()
  , m_pGraphics     (pGraphics)
  , m_pShaderStub   (NULL)
  , m_pRasterizer   (NULL)
  , m_pDepthStencil (NULL)
  , m_pBlendState   (NULL)
  , m_pSamplerState (NULL)
  , m_nRenderQueue  ((int)2000) // RenderQueue_Geometry
  , m_bSequential   (NULL)
  , m_pUniformsForReloading     (NULL)
{
  memset(m_aTextureSlot, 0, sizeof(m_aTextureSlot));
}

GXMaterialInstImpl::~GXMaterialInstImpl()
{
  SAFE_RELEASE(m_pShaderStub);
  for(int i = 0 ; i < GX_MAX_TEXTURE_STAGE; i++) {
    SAFE_RELEASE(m_aTextureSlot[i]);
  }
  SAFE_RELEASE(m_pRasterizer);
  SAFE_RELEASE(m_pDepthStencil);
  SAFE_RELEASE(m_pBlendState);
  SAFE_RELEASE(m_pSamplerState);
  SAFE_DELETE(m_pUniformsForReloading);

  m_pGraphics->UnregisterResource(this);
}

GXHRESULT GXMaterialInstImpl::SetShaderRef(GShader* pShader)
{
  if(m_pShaderStub == NULL)
  {
    if(GXFAILED(m_pGraphics->CreateShaderStub(&m_pShaderStub)))
    {
      ASSERT(0);
    }
  }

  if(pShader == NULL)
  {
    GShader* pMyShader = m_pShaderStub->GetShaderUnsafe();
    m_Buffer.Resize(pMyShader->GetCacheSize(), TRUE);
  }
  else{
    m_Buffer.Resize(pShader->GetCacheSize(), TRUE);
  }

  m_pShaderStub->SetShaderRef(pShader);
#ifdef REFACTOR_SHADER
#else
  m_pShaderStub->BindCommonUniform(g_StandardMtl);
#endif // REFACTOR_SHADER
  return GX_OK;
}

GShader* GXMaterialInstImpl::GetShaderUnsafe()  GXCONST
{
  return InlGetShaderUnsafe();
}

GShaderStub* GXMaterialInstImpl::GetShaderStubUnsafe() GXCONST
{
  return InlGetShaderStubUnsafe();
}

GXUINT GXMaterialInstImpl::GetHandle(GXLPCSTR szName) GXCONST
{
  if(szName == NULL || m_pShaderStub == NULL)
  {
    ASSERT(0);
    return -1;
  }
  return m_pShaderStub->GetHandleByName(szName);
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXMaterialInstImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GXMaterialInstImpl::Release()
{
  const GXLONG nRef = gxInterlockedDecrement(&m_nRefCount);
  if(nRef == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRef;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT GXMaterialInstImpl::Invoke(GRESCRIPTDESC* pDesc)
{
  INVOKE_DESC_CHECK(pDesc);
  if(pDesc->szCmdString != NULL)
  {
    if(clstd::strcmpT(pDesc->szCmdString, "preloadshader") == 0)
    {
      if(m_pUniformsForReloading == NULL) {
        m_pUniformsForReloading = new UniformArray;
      }
      else {
        m_pUniformsForReloading->clear();
      }

      // 收集 Uniform 的值
      GetUniformList(m_pUniformsForReloading);

      // 如果 Uniform 为空则直接删除
      if(m_pUniformsForReloading->size() == 0) {
        delete m_pUniformsForReloading;
        m_pUniformsForReloading = NULL;
      }
    }
    if(clstd::strcmpT(pDesc->szCmdString, "reloadshader") == 0)
    {
      if(m_pUniformsForReloading != NULL)
      {
        SetShaderRef(NULL);
        SetUniforms(m_pUniformsForReloading->front(), m_pUniformsForReloading->size());
        delete m_pUniformsForReloading;
        m_pUniformsForReloading = NULL;
      }
    }
  }
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetTextureSlot(GXLPCSTR pName, GXINT nSlot)
{
  return GX_OK;
}

GXBOOL GXMaterialInstImpl::IsSequential()
{
  return m_bSequential;
}

GXGraphics* GXMaterialInstImpl::GetGraphicsUnsafe()
{
  return m_pGraphics;
}

GXINT GXMaterialInstImpl::GetTextureSlot(GXLPCSTR szName)
{
  return -1;
}

GXHRESULT GXMaterialInstImpl::GetFilenameW(clStringW* pstrFilename)
{
  GXLPCWSTR szProfileDesc = m_pShaderStub->GetShaderUnsafe()->GetProfileDesc();
  if(clstd::strlenT(szProfileDesc) == 0) {
    return GX_FAIL;
  }
  GShader::ResolveProfileDescW(szProfileDesc, pstrFilename, NULL);
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::Clone(GXMaterialInst** ppCuplicateMtlInst)
{
  GXMaterialInstImpl* pNewMtlInst = NULL;
  try {
    pNewMtlInst = new GXMaterialInstImpl(m_pGraphics);
    if(pNewMtlInst == NULL) {
      CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
      return GX_FAIL;
    }

    pNewMtlInst->AddRef();

    pNewMtlInst->m_pShaderStub = m_pShaderStub;
    pNewMtlInst->m_pShaderStub->AddRef();

    for(int i = 0; i < GX_MAX_TEXTURE_STAGE; ++i)
    {
      pNewMtlInst->m_aTextureSlot[i] = m_aTextureSlot[i];
      if(pNewMtlInst->m_aTextureSlot[i] != NULL) {
        pNewMtlInst->m_aTextureSlot[i]->AddRef();
      }
    }

    pNewMtlInst->m_Buffer.Set(m_Buffer.GetPtr(), m_Buffer.GetSize());

    *ppCuplicateMtlInst = pNewMtlInst;
    return GX_OK;
  }
  catch(...)
  {
    SAFE_DELETE(pNewMtlInst);
    return GX_FAIL;
  }
}

GXHRESULT GXMaterialInstImpl::SaveFileW(GXLPCWSTR szFilename)
{
  SmartRepository Storage;
  GXHRESULT hval = SaveRepository(&Storage);
  if(GXSUCCEEDED(hval)) {
    hval = Storage.SaveW(szFilename) ? GX_OK : GX_FAIL;
  }
  return hval;
}

GXHRESULT GXMaterialInstImpl::LoadFileW(GXGraphics* pGraphics, GXLPCWSTR szFilename)
{
  SmartRepository Storage;
  GXBOOL bval = Storage.LoadW(szFilename);
  if(bval) {
    bval = LoadRepository(pGraphics, &Storage);
  }
  return bval;
}

GXHRESULT GXMaterialInstImpl::SaveRepository(SmartRepository* pStorage)
{
  CLBREAK;
  return GX_FAIL;
}

GXHRESULT GXMaterialInstImpl::LoadRepository(GXGraphics* pGraphics, SmartRepository* pStorage)
{
  CLBREAK;
  return GX_FAIL;
}

int GXMaterialInstImpl::SetSampler(GXDEFINITION* pParameters, GXSIZE_T nCount)
{
  GXHRESULT hval = GX_FAIL;
  GXUINT    handle = NULL;
  GXBOOL    bSamplerDesc = FALSE;
  GXSAMPLERDESC Desc;
  ASSERT(clstd::strcmpT(pParameters[0].szName, "SAMPLER") == 0);
  handle = GetHandle(pParameters[0].szValue);

  GXSIZE_T i = 1;
  for(; i < nCount; i++)
  {
    if(clstd::strcmpT(pParameters[i].szName, "SAMPLER") == 0) {
        break;
    }
    if(handle)
    {
      clStringA strName = "$";
      strName.Append(pParameters[i].szName);
      strName.MakeUpper();

      if(strName == "$TEXTURE") // "texture"
      {
        GTexture* pTexture = NULL;
        if(pParameters[i].szValue[0] == '<')
        {
          clStringA strTextureName = &pParameters[i].szValue[1];
          strTextureName.TrimRight('>');
          hval = m_pGraphics->CreateTexture(&pTexture, strTextureName, 0, 0, 0, GXFMT_UNKNOWN, GXRU_DEFAULT);
          if(GXFAILED(hval)) {
            CLOG_WARNING("Warning: Named texture must be created first when load this material.\n");
          }
        }
        else
        {
          hval = m_pGraphics->CreateTextureFromFileW(&pTexture, clStringW(pParameters[i].szValue));
        }

        if(GXSUCCEEDED(hval)) {
          m_pShaderStub->SetTextureByHandle(m_aTextureSlot, handle, pTexture);
        }
        SAFE_RELEASE(pTexture);
      }
      else if(strName == "$TEXTURE3D") // "texture3d"
      {
        GTexture3D* pTexture3D = NULL;
        if(pParameters[i].szValue[0] == '<')
        {
          clStringA strTextureName = &pParameters[i].szValue[1];
          strTextureName.TrimRight('>');
          hval = m_pGraphics->CreateTexture3D(&pTexture3D, strTextureName, 0, 0, 0, 0, GXFMT_UNKNOWN, GXRU_DEFAULT);
          if(GXFAILED(hval)) {
            CLOG_WARNING("Warning: Named texture must be created first when load this material.\n");
          }
        }
        else
        {
          hval = m_pGraphics->CreateTexture3DFromFileW(&pTexture3D, clStringW(pParameters[i].szValue));
        }

        if(GXSUCCEEDED(hval)) {
          m_pShaderStub->SetTextureByHandle(m_aTextureSlot, handle, pTexture3D);
        }
        SAFE_RELEASE(pTexture3D);
      }
      else if(strName == "$BORDERCOLOR")
      {
        GXDWORD dwColor = clstd::xtou(16, pParameters[i].szValue);
        Desc.BorderColor = dwColor;
      }
      else {
        MtlStateDict::iterator itName = s_MtlStateDict.find(strName);
        if(itName != s_MtlStateDict.end())
        {
          clStringA strValue = pParameters[i].szName;
          strValue.Append('_');
          strValue.Append(pParameters[i].szValue);
          strValue.MakeUpper();

          MtlStateDict::iterator itValue = s_MtlStateDict.find(strValue);

          if(itValue != s_MtlStateDict.end())
          {
            *(((GXLPBYTE)&Desc) + itName->second) = (GXBYTE)itValue->second;
            bSamplerDesc = TRUE;
          }
        }
      }
    }
  }
  if(bSamplerDesc) {
    if(m_pSamplerState == NULL)
    {
      m_pGraphics->CreateSamplerState(&m_pSamplerState);
    }
    GXUINT nStage = m_pShaderStub->GetSamplerStageByHandle(handle);
    if(GXFAILED(m_pSamplerState->SetState(nStage, &Desc))) {
      CLBREAK;
    }
  }
  return (int)i;
}

GXHRESULT GXMaterialInstImpl::SetUniforms(GXDEFINITION* pParameters, GXSIZE_T nCount)
{
  float4x4 mat;
  clStringArrayA aStrings;
  GXUINT handle = 0;
  for(GXSIZE_T i = 0; i < nCount; i++)
  {
    if(pParameters[i].szName == NULL ||
      pParameters[i].szValue == NULL ||
      pParameters[i].szName[0] == '\0' ||
      pParameters[i].szValue[0] == '\0') {
        return GX_OK;
    }
    if(clstd::strcmpT(pParameters[i].szName, "SAMPLER") == 0) {
      i += SetSampler(&pParameters[i], nCount - i);
      continue;
    }
    else {
      handle = GetHandle(pParameters[i].szName);
    }
    if(handle == 0) continue;

    const GXUniformType eType = m_pShaderStub->GetHandleType(handle);
    aStrings.clear();
    switch(eType)
    {
    case GXUB_FLOAT:
      mat._11 = (float)clstd::_xstrtof(pParameters[i].szValue);
      m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, &mat._11, 1);
      break;
    case GXUB_FLOAT2:
      ResolveString(clStringA(pParameters[i].szValue), ',', aStrings);
      if(aStrings.size() != 2)
        break;
      mat._11 = (float)clstd::_xstrtof<ch>(aStrings[0]);
      mat._12 = (float)clstd::_xstrtof<ch>(aStrings[1]);
      m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&mat, 2);
      break;
    case GXUB_FLOAT3:
      ResolveString(clStringA(pParameters[i].szValue), ',', aStrings);
      if(aStrings.size() != 3)
        break;
      mat._11 = (float)clstd::_xstrtof<ch>(aStrings[0]);
      mat._12 = (float)clstd::_xstrtof<ch>(aStrings[1]);
      mat._13 = (float)clstd::_xstrtof<ch>(aStrings[2]);
      m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&mat, 3);
      break;
    case GXUB_FLOAT4:
      ResolveString(clStringA(pParameters[i].szValue), ',', aStrings);
      if(aStrings.size() != 4)
        break;
      mat._11 = (float)clstd::_xstrtof<ch>(aStrings[0]);
      mat._12 = (float)clstd::_xstrtof<ch>(aStrings[1]);
      mat._13 = (float)clstd::_xstrtof<ch>(aStrings[2]);
      mat._14 = (float)clstd::_xstrtof<ch>(aStrings[3]);
      m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&mat, 4);
      break;
    case GXUB_MATRIX4:
      ResolveString(clStringA(pParameters[i].szValue), ',', aStrings);
      if(aStrings.size() != 16)
        break;
      for(int n = 0; n < 16; n++) {
        mat.m[n] = (float)clstd::_xstrtof<ch>(aStrings[n]);
      }
      m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&mat, 4 * 4);
      break;

    //case GXUB_SAMPLER2D:
    //  {
    //    GTexture* pTexture = NULL;
    //    GXHRESULT hval = GX_FAIL;
    //    if(pParameters[i].szDefinition[0] == '<')
    //    {
    //      clStringA strTextureName = &pParameters[i].szDefinition[1];
    //      strTextureName.TrimRight('>');
    //      hval = m_pGraphics->CreateTexture(&pTexture, strTextureName, 0, 0, 0, GXFMT_UNKNOWN, GXRU_DEFAULT);
    //      if(GXFAILED(hval)) {
    //        CLOG_WARNING("Warning: Named texture must be created first when load this material.\n");
    //      }
    //    }
    //    else
    //    {
    //      hval = m_pGraphics->CreateTextureFromFileW(&pTexture, clStringW(pParameters[i].szDefinition));
    //    }

    //    if(GXSUCCEEDED(hval)) {
    //      m_pShaderStub->SetTextureByHandle(m_aTextureSlot, handle, pTexture);
    //    }
    //    SAFE_RELEASE(pTexture);
    //  }
    //  break;
    case GXUB_SAMPLER2D:
    case GXUB_UNDEFINED:
    case GXUB_SAMPLER3D:
      CLBREAK;
      break;
    }
  }
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetRenderStates(GXDEFINITION* pParameters, GXSIZE_T nCount)
{
  RENDERSTATE RenderState;
  GXBOOL bRasterizer = FALSE;
  GXBOOL bBlend = FALSE;
  GXBOOL bDepthStencil = FALSE;

  for(GXSIZE_T i = 0; i < nCount; i++)
  {
    if(pParameters[i].szName == NULL ||
      pParameters[i].szValue == NULL ||
      pParameters[i].szName[0] == '\0' ||
      pParameters[i].szValue[0] == '\0') {
        return GX_OK;
    }
    clStringA strName = "$";
    strName += pParameters[i].szName;
    strName.MakeUpper();

    MtlStateDict::iterator itName = s_MtlStateDict.find(strName);
    if(itName != s_MtlStateDict.end())
    {
      clStringA strValue = pParameters[i].szName;
      strValue.Append('_');
      strValue.Append(pParameters[i].szValue);
      strValue.MakeUpper();
      MtlStateDict::iterator itValue = s_MtlStateDict.find(strValue);
      if(itValue != s_MtlStateDict.end())
      {
        // 这个需要按顺序判断
        if(itName->second < sizeof(RenderState.RasterizerDesc))
        {
          bRasterizer = TRUE;
        }
        else if(itName->second < sizeof(RenderState.RasterizerDesc) + sizeof(RenderState.BlendDesc))
        {
          bBlend = TRUE;
        }
        else if(itName->second < sizeof(RenderState.RasterizerDesc) + sizeof(RenderState.BlendDesc) + sizeof(RenderState.DepthStencilDesc))
        {
          bDepthStencil = TRUE;
        }

        *(((GXLPBYTE)&RenderState) + itName->second) = (GXBYTE)itValue->second;
      }
    }
    else if(strName == "$RENDERQUEUE") {
      m_nRenderQueue = MOParseRenderQueue(pParameters[i].szValue);
    }
  }
  if(bRasterizer) {
    m_pGraphics->CreateRasterizerState(&m_pRasterizer, &RenderState.RasterizerDesc);
  }

  if(bBlend) {
    m_pGraphics->CreateBlendState(&m_pBlendState, &RenderState.BlendDesc, 1);

    // 目前除了亮度叠加外其它合成方式都需要排序
    if( ! (RenderState.BlendDesc.BlendEnable && 
      RenderState.BlendDesc.SrcBlend  != GXBLEND_ONE &&
      RenderState.BlendDesc.DestBlend != GXBLEND_ONE &&
      RenderState.BlendDesc.BlendOp   != GXBLENDOP_ADD))
    {
      m_bSequential = 1;
    }
  }

  if(bDepthStencil) {
    m_pGraphics->CreateDepthStencilState(&m_pDepthStencil, &RenderState.DepthStencilDesc);
  }
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::BindData(MODataPool* pDataPool, GXLPCSTR szStruct)
{
  return m_pShaderStub->BindData(pDataPool, szStruct);
}

GXHRESULT GXMaterialInstImpl::BindDataByName(GXLPCSTR szPoolName, GXLPCSTR szStruct)
{
  MODataPool* pDataPool = NULL;
  GXHRESULT hval = MODataPool::CreateDataPool(&pDataPool, szPoolName, NULL, NULL);
  if(GXSUCCEEDED(hval))
  {
    hval = BindData(pDataPool, szStruct);
    SAFE_RELEASE(pDataPool);
  }
  return hval;
}

GXHRESULT GXMaterialInstImpl::GetUniformList(UniformArray* pUniforms)
{
  GShaderStub::UNIFORMDESC Desc;
  GXLPBYTE pData = (GXLPBYTE)m_Buffer.GetPtr();
  int idx = 0;
  while(m_pShaderStub->GetUniformByIndex(idx++, &Desc))
  {
    GXDefinition Def;
    Def.Name = Desc.Name;
    switch (Desc.eType)
    {
    case GXUB_FLOAT:
      {
        float* pValue = (float*)(pData + Desc.nOffset);
        Def.Value.Format("%f", *pValue);
      }
      break;
    case GXUB_FLOAT2:
      {
        float2* pValue = (float2*)(pData + Desc.nOffset);
        Def.Value.Format("%f,%f", pValue->x, pValue->y);
      }
      break;
    case GXUB_FLOAT3:
      {
        float3* pValue = (float3*)(pData + Desc.nOffset);
        Def.Value.Format("%f,%f,%f", pValue->x, pValue->y, pValue->z);
      }
      break;
    case GXUB_FLOAT4:
      {
        float4* pValue = (float4*)(pData + Desc.nOffset);
        Def.Value.Format("%f,%f,%f,%f", pValue->x, pValue->y, pValue->z, pValue->w);
      }
      break;
    case GXUB_MATRIX4:
      {
        float4x4* pValue = (float4x4*)(pData + Desc.nOffset);
        Def.Value.Format(
          "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", 
          pValue->_11, pValue->_12, pValue->_13, pValue->_14,
          pValue->_21, pValue->_22, pValue->_23, pValue->_24,
          pValue->_31, pValue->_32, pValue->_33, pValue->_34,
          pValue->_41, pValue->_42, pValue->_43, pValue->_44);
      }
      break;
    default:
      break;
    }
    pUniforms->push_back(Def);
  }
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetParameters(ParamType eType, GXDEFINITION* pParameters, int nCount)
{
  if(nCount <= 0) {
    nCount = INT_MAX;
  }

  switch(eType)
  {
  case PT_UNIFORMS:
    return SetUniforms(pParameters, nCount);
  case PT_RENDERSTATE:
    return SetRenderStates(pParameters, nCount);
  }
  return GX_FAIL;
}

GXHRESULT GXMaterialInstImpl::SetTextureByName(GXLPCSTR szName, GTextureBase* pTexture)
{
  GXUINT handle = GetHandle(szName);
  if(handle != 0) {
    m_pShaderStub->SetTextureByHandle(m_aTextureSlot, handle, pTexture);
    return GX_OK;
  }
  return GX_FAIL;
}

GXHRESULT GXMaterialInstImpl::SetTextureByIndex(GXUINT nIndex, GTextureBase* pTexture)
{
  if(nIndex < GX_MAX_TEXTURE_STAGE) {
    m_pShaderStub->SetTextureByIndex(m_aTextureSlot, nIndex, pTexture);
    return GX_OK;
  }
  return GX_FAIL;
}

GXHRESULT GXMaterialInstImpl::SetTextureByNameFromFileW(GXLPCSTR szName, GXLPCWSTR szFilename)
{
  GTextureBase* pTexture = NULL;
  GXHRESULT hval = m_pGraphics->CreateTextureFromFileW((GTexture**)&pTexture, szFilename);
  if(GXSUCCEEDED(hval)) {
    hval = SetTextureByName(szName, pTexture);
  }
  SAFE_RELEASE(pTexture);
  return hval;
}

GXHRESULT GXMaterialInstImpl::SetTextureByIndexFromFileW(GXUINT nIndex, GXLPCWSTR szFilename)
{
  GTextureBase* pTexture = NULL;
  GXHRESULT hval = m_pGraphics->CreateTextureFromFileExW((GTexture**)&pTexture, szFilename, 0, 0, 0, GXFMT_UNKNOWN, GXRU_DEFAULT, GXFILTER_LINEAR, GXFILTER_LINEAR);
  if(GXSUCCEEDED(hval)) {
    hval = SetTextureByIndex(nIndex, pTexture);
  }
  SAFE_RELEASE(pTexture);
  return hval;
}

GXHRESULT GXMaterialInstImpl::SetFloat1ByName(GXLPCSTR szName, float val)
{
  GXUINT handle = GetHandle(szName);
  m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&val, 1);
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetFloat2ByName(GXLPCSTR szName, const float2& vFloat2)
{
  GXUINT handle = GetHandle(szName);
  m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&vFloat2, 2);
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetFloat3ByName(GXLPCSTR szName, const float3& vFloat3)
{
  GXUINT handle = GetHandle(szName);
  m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&vFloat3, 3);
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetFloat4ByName(GXLPCSTR szName, const float4& vFloat4)
{
  GXUINT handle = GetHandle(szName);
  m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&vFloat4, 4);
  return GX_OK;
}

GXHRESULT GXMaterialInstImpl::SetMatrixByName(GXLPCSTR szName, const float4x4& mat)
{
  GXUINT handle = GetHandle(szName);
  m_pShaderStub->SetUniformByHandle(&m_Buffer, handle, (float*)&mat, 4 * 4);
  return GX_OK;
}

#ifdef REFACTOR_SHADER
GXHRESULT GXMaterialInstImpl::IntCommit(GXLPCBYTE lpCanvasUniform)
#else
GXHRESULT GXMaterialInstImpl::IntCommit(const STANDARDMTLUNIFORMTABLE* pStdUniforms)
#endif // #ifdef REFACTOR_SHADER
{
  for(int i = 0; i < GX_MAX_TEXTURE_STAGE; i++)
  {
    if(m_aTextureSlot[i] != NULL) {
      // TODO: 觉得这个应该改为 SetTextureArray 来减少循环次数
      m_pGraphics->SetTexture(m_aTextureSlot[i], i);
    }
    //else { // TODO: 这个好好想想
    //  m_pGraphics->SetTexture(NULL, i);
    //}
  }
  m_pGraphics->SetRasterizerState(m_pRasterizer);
  m_pGraphics->SetBlendState(m_pBlendState);
  m_pGraphics->SetDepthStencilState(m_pDepthStencil);
  m_pGraphics->SetSamplerState(m_pSamplerState);
#ifdef REFACTOR_SHADER
  // Update Global Uniform
  m_pShaderStub->UpdateUniform(-2, lpCanvasUniform, m_Buffer.GetPtr(), m_Buffer.GetSize()); // 提交绑定常量
  m_pShaderStub->UpdateCanvasUniform(lpCanvasUniform, m_Buffer.GetPtr(), m_Buffer.GetSize());  // 提交公共常量

  // Commit to Device
  m_pShaderStub->CommitToDevice(m_Buffer.GetPtr(), m_Buffer.GetSize());
#else
  m_pShaderStub->CommitUniform(-2, NULL, -1);               // 提交绑定常量
  m_pShaderStub->CommitUniform(-1, m_Buffer.GetPtr(), -1);  // 提交材质私有常量
  m_pShaderStub->CommitUniform(0, pStdUniforms, -1);        // 提交公共常量
#endif // #ifdef REFACTOR_SHADER
  return GX_OK;
}

int GXMaterialInstImpl::GetRenderQueue() const
{
  return m_nRenderQueue | ( m_bSequential << 16);
}

//////////////////////////////////////////////////////////////////////////
