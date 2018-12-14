#include "GrapX.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GResource.h"
#include "GrapX/GShader.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GStateBlock.h"
#include "GXMaterialImpl.h"
#include "GrapX/StdMtl.h"
#include "GrapX/gxError.h"
#include "Smart/smartstream.h"
#include "smart/SmartRepository.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

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
namespace GrapX
{
  MaterialImpl::MtlStateDict MaterialImpl::s_MtlStateDict;
  GXVOID MaterialImpl::InitializeMtlStateDict()
  {
    s_MtlStateDict["$BLENDENABLE"]      = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, BlendEnable);
    s_MtlStateDict["BLENDENABLE_TRUE"]  = 1;
    s_MtlStateDict["BLENDENABLE_FALSE"] = 0;
    s_MtlStateDict["BLENDENABLE_1"]     = 1;
    s_MtlStateDict["BLENDENABLE_0"]     = 0;
    s_MtlStateDict["BLENDENABLE_YES"]   = 1;
    s_MtlStateDict["BLENDENABLE_NO"]    = 0;

    //s_MtlStateDict["$ALPHATEST"]      = 

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

    s_MtlStateDict["$BLENDOP"]            = offsetof(RENDERSTATE, BlendDesc) + offsetof(GXBLENDDESC, BlendOp);
    s_MtlStateDict["BLENDOP_ADD"]         = GXBLENDOP_ADD;
    s_MtlStateDict["BLENDOP_SUBTRACT"]    = GXBLENDOP_SUBTRACT;
    s_MtlStateDict["BLENDOP_REVSUBTRACT"] = GXBLENDOP_REVSUBTRACT;
    s_MtlStateDict["BLENDOP_MIN"]         = GXBLENDOP_MIN;
    s_MtlStateDict["BLENDOP_MAX"]         = GXBLENDOP_MAX;

    s_MtlStateDict["$FILLMODE"]           = offsetof(RENDERSTATE, RasterizerDesc) + offsetof(GXRASTERIZERDESC, FillMode);
    s_MtlStateDict["FILLMODE_POINT"]      = GXFILL_POINT;
    s_MtlStateDict["FILLMODE_WIREFRAME"]  = GXFILL_WIREFRAME;
    s_MtlStateDict["FILLMODE_SOLID"]      = GXFILL_SOLID;

    s_MtlStateDict["$CULLMODE"]           = offsetof(RENDERSTATE, RasterizerDesc) + offsetof(GXRASTERIZERDESC, CullMode);
    s_MtlStateDict["CULLMODE_NONE"]       = GXCULL_NONE;
    s_MtlStateDict["CULLMODE_CW"]         = GXCULL_CW;
    s_MtlStateDict["CULLMODE_CCW"]        = GXCULL_CCW;

    s_MtlStateDict["$DEPTHENABLE"]      = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthEnable);
    s_MtlStateDict["DEPTHENABLE_TRUE"]  = 1;
    s_MtlStateDict["DEPTHENABLE_FALSE"] = 0;
    s_MtlStateDict["DEPTHENABLE_1"]     = 1;
    s_MtlStateDict["DEPTHENABLE_0"]     = 0;
    s_MtlStateDict["DEPTHENABLE_YES"]   = 1;
    s_MtlStateDict["DEPTHENABLE_NO"]    = 0;

    s_MtlStateDict["$DEPTHWRITE"]       = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthWriteMask);
    s_MtlStateDict["DEPTHWRITE_TRUE"]   = 1;
    s_MtlStateDict["DEPTHWRITE_FALSE"]  = 0;
    s_MtlStateDict["DEPTHWRITE_1"]      = 1;
    s_MtlStateDict["DEPTHWRITE_0"]      = 0;
    s_MtlStateDict["DEPTHWRITE_YES"]    = 1;
    s_MtlStateDict["DEPTHWRITE_NO"]     = 0;


    s_MtlStateDict["$DEPTHFUNC"]              = offsetof(RENDERSTATE, DepthStencilDesc) + offsetof(GXDEPTHSTENCILDESC, DepthFunc);
    s_MtlStateDict["DEPTHFUNC_NEVER"]         = GXCMP_NEVER;
    s_MtlStateDict["DEPTHFUNC_LESS"]          = GXCMP_LESS;
    s_MtlStateDict["DEPTHFUNC_EQUAL"]         = GXCMP_EQUAL;
    s_MtlStateDict["DEPTHFUNC_LESSEQUAL"]     = GXCMP_LESSEQUAL;
    s_MtlStateDict["DEPTHFUNC_GREATER"]       = GXCMP_GREATER;
    s_MtlStateDict["DEPTHFUNC_NOTEQUAL"]      = GXCMP_NOTEQUAL;
    s_MtlStateDict["DEPTHFUNC_GREATEREQUAL"]  = GXCMP_GREATEREQUAL;
    s_MtlStateDict["DEPTHFUNC_ALWAYS"]        = GXCMP_ALWAYS;


    s_MtlStateDict["$ADDRESSU"]   = offsetof(GXSAMPLERDESC, AddressU);
    s_MtlStateDict["$ADDRESSV"]   = offsetof(GXSAMPLERDESC, AddressV);
    s_MtlStateDict["$ADDRESSW"]   = offsetof(GXSAMPLERDESC, AddressW);
    s_MtlStateDict["$MAGFILTER"]  = offsetof(GXSAMPLERDESC, MagFilter);
    s_MtlStateDict["$MINFILTER"]  = offsetof(GXSAMPLERDESC, MinFilter);
    s_MtlStateDict["$MIPFILTER"]  = offsetof(GXSAMPLERDESC, MipFilter);
    //s_MtlStateDict["$BORDERCOLOR"]            = GXSAMP_BORDERCOLOR;

    s_MtlStateDict["ADDRESSU_WRAP"]       = GXTADDRESS_WRAP;
    s_MtlStateDict["ADDRESSU_MIRROR"]     = GXTADDRESS_MIRROR;
    s_MtlStateDict["ADDRESSU_CLAMP"]      = GXTADDRESS_CLAMP;
    s_MtlStateDict["ADDRESSU_BORDER"]     = GXTADDRESS_BORDER;
    s_MtlStateDict["ADDRESSU_MIRRORONCE"] = GXTADDRESS_MIRRORONCE;

    s_MtlStateDict["ADDRESSV_WRAP"]       = GXTADDRESS_WRAP;
    s_MtlStateDict["ADDRESSV_MIRROR"]     = GXTADDRESS_MIRROR;
    s_MtlStateDict["ADDRESSV_CLAMP"]      = GXTADDRESS_CLAMP;
    s_MtlStateDict["ADDRESSV_BORDER"]     = GXTADDRESS_BORDER;
    s_MtlStateDict["ADDRESSV_MIRRORONCE"] = GXTADDRESS_MIRRORONCE;

    s_MtlStateDict["ADDRESSW_WRAP"]       = GXTADDRESS_WRAP;
    s_MtlStateDict["ADDRESSW_MIRROR"]     = GXTADDRESS_MIRROR;
    s_MtlStateDict["ADDRESSW_CLAMP"]      = GXTADDRESS_CLAMP;
    s_MtlStateDict["ADDRESSW_BORDER"]     = GXTADDRESS_BORDER;
    s_MtlStateDict["ADDRESSW_MIRRORONCE"] = GXTADDRESS_MIRRORONCE;

    s_MtlStateDict["MAGFILTER_NONE"]          = GXTEXFILTER_NONE;
    s_MtlStateDict["MAGFILTER_POINT"]         = GXTEXFILTER_POINT;
    s_MtlStateDict["MAGFILTER_LINEAR"]        = GXTEXFILTER_LINEAR;
    s_MtlStateDict["MAGFILTER_ANISOTROPIC"]   = GXTEXFILTER_ANISOTROPIC;
    s_MtlStateDict["MAGFILTER_PYRAMIDALQUAD"] = GXTEXFILTER_PYRAMIDALQUAD;
    s_MtlStateDict["MAGFILTER_GAUSSIANQUAD"]  = GXTEXFILTER_GAUSSIANQUAD;

    s_MtlStateDict["MINFILTER_NONE"]          = GXTEXFILTER_NONE;
    s_MtlStateDict["MINFILTER_POINT"]         = GXTEXFILTER_POINT;
    s_MtlStateDict["MINFILTER_LINEAR"]        = GXTEXFILTER_LINEAR;
    s_MtlStateDict["MINFILTER_ANISOTROPIC"]   = GXTEXFILTER_ANISOTROPIC;
    s_MtlStateDict["MINFILTER_PYRAMIDALQUAD"] = GXTEXFILTER_PYRAMIDALQUAD;
    s_MtlStateDict["MINFILTER_GAUSSIANQUAD"]  = GXTEXFILTER_GAUSSIANQUAD;

    s_MtlStateDict["MIPFILTER_NONE"]          = GXTEXFILTER_NONE;
    s_MtlStateDict["MIPFILTER_POINT"]         = GXTEXFILTER_POINT;
    s_MtlStateDict["MIPFILTER_LINEAR"]        = GXTEXFILTER_LINEAR;
    s_MtlStateDict["MIPFILTER_ANISOTROPIC"]   = GXTEXFILTER_ANISOTROPIC;
    s_MtlStateDict["MIPFILTER_PYRAMIDALQUAD"] = GXTEXFILTER_PYRAMIDALQUAD;
    s_MtlStateDict["MIPFILTER_GAUSSIANQUAD"]  = GXTEXFILTER_GAUSSIANQUAD;
  }

  GXVOID MaterialImpl::FinalizeMtlStateDict()
  {
    s_MtlStateDict.clear();
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MaterialImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT MaterialImpl::Release()
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

  MaterialImpl::~MaterialImpl()
  {
  }

  MaterialImpl::MaterialImpl(Graphics* pGraphics, Shader* pShader)
    : m_pGraphics(pGraphics)
    , m_pShader(pShader)
  {
  }

  GXHRESULT MaterialImpl::IntCommit(GXLPCBYTE lpCanvasUniform)
  {
    CLBREAK;
    return GX_FAIL;
  }

  GXHRESULT MaterialImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    return GX_OK;
  }

  Graphics* MaterialImpl::GetGraphicsUnsafe() const
  {
    return m_pGraphics;
  }

  Marimo::DataPoolVariable MaterialImpl::GetUniform(GXLPCSTR szName)
  {
    Marimo::DataPoolVariable var;
    return var;
  }

  GXBOOL MaterialImpl::SetTexture(GXUINT nSlot, Texture* pTexture)
  {
    return FALSE;
  }

  GXBOOL MaterialImpl::SetTexture(GXLPCSTR szSamplerName, Texture* pTexture)
  {
    return FALSE;
  }

  int MaterialImpl::GetRenderQueue() const
  {
    CLBREAK;
    return -1;
  }

  GXHRESULT MaterialImpl::GetFilename(clStringW* pstrFilename)
  {
    CLBREAK;
    return GX_FAIL;
  }

  GXBOOL MaterialImpl::InitMaterial()
  {
    CLBREAK;
    return FALSE;
  }

  GXHRESULT MaterialImpl::BindDataByName(GXLPCSTR szPoolName, GXLPCSTR szStruct)
  {
    CLBREAK;
    return GX_FAIL;
  }

  GXHRESULT MaterialImpl::SetParameters(ParamType eType, GXDEFINITION* pParameters, int nCount)
  {
    CLBREAK;
    return GX_FAIL;
  }
} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
