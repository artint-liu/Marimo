#include "GrapX.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GResource.h"
#include "GrapX/GShader.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/StdMtl.h"
#include "GrapX/gxError.h"
#include "Smart/smartstream.h"
#include "smart/SmartRepository.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "clTokens.h"
#include "GXMaterialImpl.h"
#include "clStringAttach.h"

using namespace clstd;
GXBOOL Parse(GXBLENDDESC& desc, TokensA& tokens, TokensA::iterator& iter);
GXBOOL Parse(GXRASTERIZERDESC& desc, TokensA& tokens, TokensA::iterator& iter);
GXBOOL Parse(GXDEPTHSTENCILDESC& desc, TokensA& tokens, TokensA::iterator& iter);


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
    SAFE_RELEASE(m_pDataPool);
    SAFE_RELEASE(m_pRasterizer);
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pBlendState);
  }

  MaterialImpl::MaterialImpl(Graphics* pGraphics, Shader* pShader)
    : m_pGraphics(pGraphics)
    , m_pShader(pShader)
    , m_pDataPool(NULL)
    , m_nRenderQueue(DEFAULT_RENDER_QUEUE)
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

  void MaterialImpl::SetDepthStencilState(DepthStencilState* pState)
  {
    if(pState)
    {
      InlSetNewObjectAlwaysT(m_pDepthStencil, pState);
    }
  }

  void MaterialImpl::SetRasterizerState(RasterizerState* pState)
  {
    if (pState)
    {
      InlSetNewObjectAlwaysT(m_pRasterizer, pState);
    }
  }

  void MaterialImpl::SetBlendState(BlendState* pState)
  {
    if (pState)
    {
      InlSetNewObjectAlwaysT(m_pBlendState, pState);
    }
  }

  GXBOOL MaterialImpl::SetState(GXLPCSTR szStateCommand)
  {
    TokensA tokens(szStateCommand, strlenT(szStateCommand));
    GXBLENDDESC sBlendDesc;
    GXRASTERIZERDESC sRasterizerDesc;
    GXDEPTHSTENCILDESC sDepthStencilDesc(TRUE, FALSE);
    GXBOOL bBlend = FALSE, bRasterizer = FALSE, bDepthStencil = FALSE;

    auto iter = tokens.begin();

    while(iter != tokens.end())
    {
      if (::Parse(sBlendDesc, tokens, iter)) {
        bBlend= TRUE;
      }

      if(iter == tokens.end()) {
        break;
      }

      if (::Parse(sDepthStencilDesc, tokens, iter)) {
        bDepthStencil = TRUE;
      }

      if (iter == tokens.end()) {
        break;
      }

      if (::Parse(sRasterizerDesc, tokens, iter)) {
        bRasterizer = TRUE;
      }
    }

    // 根据修改标记更换渲染状态
    if(bRasterizer) {
      SAFE_RELEASE(m_pRasterizer);
      m_pGraphics->CreateRasterizerState(&m_pRasterizer, &sRasterizerDesc);
    }

    if(bDepthStencil) {
      SAFE_RELEASE(m_pDepthStencil);
      m_pGraphics->CreateDepthStencilState(&m_pDepthStencil, &sDepthStencilDesc);
    }

    if(bBlend) {
      SAFE_RELEASE(m_pBlendState);
      m_pGraphics->CreateBlendState(&m_pBlendState, &sBlendDesc, 1);
    }

    return TRUE;
  }

  Marimo::DataPoolVariable MaterialImpl::GetUniform(GXLPCSTR szName)
  {
    Marimo::DataPoolVariable var;
    if(m_pDataPool) {
      m_pDataPool->QueryByName(szName, &var);
    }
    return var;
  }

  GXBOOL MaterialImpl::SetTexture(GXUINT nSlot, Texture* pTexture)
  {
    if (m_aTextures.size() > nSlot)
    {
      m_aTextures[nSlot].texture = pTexture;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL MaterialImpl::SetTexture(GXLPCSTR szSamplerName, Texture* pTexture)
  {
    const Shader::BINDRESOURCE_DESC* pDesc = m_pShader->FindBindResource(szSamplerName);
    if (pDesc && m_aTextures.size() > pDesc->slot)
    {
      m_aTextures[pDesc->slot].texture = pTexture;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL MaterialImpl::GetTexture(GXLPCSTR szSamplerName, Texture** ppTexture)
  {
    const Shader::BINDRESOURCE_DESC* pDesc = m_pShader->FindBindResource(szSamplerName);
    if (pDesc && m_aTextures.size() > pDesc->slot)
    {
      *ppTexture = m_aTextures[pDesc->slot].texture;
      SAFE_ADDREF(*ppTexture);
      return TRUE;
    }
    return FALSE;
  }

  int MaterialImpl::SetRenderQueue(int nRenderQueue)
  {
    int nPrevRenderQueue = m_nRenderQueue;
    if (nRenderQueue >= 0 && nRenderQueue <= RenderQueue_Max) {
      m_nRenderQueue = nRenderQueue;
    }
    return nPrevRenderQueue;
  }

  int MaterialImpl::GetRenderQueue() const
  {
    return m_nRenderQueue;
  }

  GXBOOL MaterialImpl::GetFilename(clStringW* pstrFilename)
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL MaterialImpl::InitMaterial()
  {
    Marimo::DATAPOOL_MANIFEST manifest;
    m_pShader->GetDataPoolDeclaration(&manifest);
    GXHRESULT hr = Marimo::DataPool::CreateDataPool(&m_pDataPool, NULL, manifest.pTypes, manifest.pVariables, Marimo::DataPoolCreation_NotCross16BytesBoundary);

    GXINT nMaxSlot = 0;
    for (GXUINT n = 0;; n++)
    {
      const Shader::BINDRESOURCE_DESC* pDesc = m_pShader->GetBindResource(n);
      if (pDesc == NULL) {
        break;
      }
      else if (pDesc->type == Shader::BindType::Sampler) {
        nMaxSlot = clMax(nMaxSlot, pDesc->slot + 1);
      }
    }

    GXRASTERIZERDESC sRaterizerDesc;
    m_pGraphics->CreateRasterizerState(&m_pRasterizer, &sRaterizerDesc);

    GXDEPTHSTENCILDESC sDepthStencilDesc(TRUE, FALSE);
    m_pGraphics->CreateDepthStencilState(&m_pDepthStencil, &sDepthStencilDesc);

    GXBLENDDESC sBlendDesc;
    m_pGraphics->CreateBlendState(&m_pBlendState, &sBlendDesc, 1);

    if (nMaxSlot) {
      GXSamplerDesc sSamplerDesc(GXTADDRESS_WRAP, 0, GXTEXFILTER_LINEAR);
      SamplerState* pSamplerState = NULL;
      m_pGraphics->CreateSamplerState(&pSamplerState, &sSamplerDesc);

      m_aTextures.resize(nMaxSlot);
      m_aSamplerStates.assign(nMaxSlot, ObjectT<SamplerState>(pSamplerState));

      ch buffer[128];
      clStringAttachA strName(buffer, sizeof(buffer));
      for (GXUINT index = 0;; index++)
      {
        auto desc = m_pShader->GetBindResource(index);
        if (desc == NULL) {
          break;
        }
        
        if (desc->type == Shader::BindType::Texture)
        {
          strName.Clear();
          strName.Append(desc->name).Append("_TexelSize");
          Marimo::DataPoolVariable var = GetUniform(strName.CStr());
          if (var.IsValid() && clstd::strcmpT(var.GetTypeName(), "float4") == 0)
          {
            m_aTextures[desc->slot].TexelSize = var.CastTo<MOVarFloat4>();
          }
        }
      }

      SAFE_RELEASE(pSamplerState);
    }

    return TRUE;
  }

  GXBOOL MaterialImpl::SetFloat(GXLPCSTR szName, float value)
  {
    Marimo::DataPoolVariable var = GetUniform(szName);
    if (var.IsValid())
    {
      return var.Set(value);
    }
    return FALSE;
  }

  float  MaterialImpl::GetFloat(GXLPCSTR szName)
  {
    Marimo::DataPoolVariable var = GetUniform(szName);
    if (var.IsValid())
    {
      return var.ToFloat();
    }
    return 0;
  }

  GXBOOL MaterialImpl::SetVector(GXLPCSTR szName, float4* pVector)
  {
    Marimo::DataPoolVariable var = GetUniform(szName);
    if (var.IsValid())
    {
      MOVarFloat4 var4 = var.CastTo<MOVarFloat4>();
      var4 = *pVector;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL MaterialImpl::GetVector(float4* pOut, GXLPCSTR szName)
  {
    Marimo::DataPoolVariable var = GetUniform(szName);
    if (var.IsValid())
    {
      MOVarFloat4 var4 = var.CastTo<MOVarFloat4>();
      *pOut = (float4&)var;
      return TRUE;
    }
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

  Marimo::DataPool* MaterialImpl::GetDataPoolUnsafe() const
  {
    return m_pDataPool;
  }

  GXBOOL MaterialImpl::Commit()
  {
    GXUINT slot = 0;
    for (auto it = m_aTextures.begin(); it != m_aTextures.end(); ++it, slot++)
    {
      m_pGraphics->SetTexture(it->texture, slot);
      if(it->TexelSize.IsValid() && (it->texture != NULL)) {
        GXSIZE size;
        it->texture->GetDimension(&size);
        it->TexelSize->set(1.0f / size.cx, 1.0f / size.cy, (float)size.cx, (float)size.cy);
      }
    }

    m_pGraphics->SetBlendState(m_pBlendState);
    m_pGraphics->SetRasterizerState(m_pRasterizer);
    m_pGraphics->SetDepthStencilState(m_pDepthStencil);
    if(_CL_NOT_(m_aSamplerStates.empty())) {
      m_pGraphics->SetSamplerState(0, (GXUINT)m_aSamplerStates.size(), &m_aSamplerStates.front());
    }

    return TRUE;
  }

} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
#define CMP_STR(_STR, _ENUM) if (strncmpT(str, _STR, len) == 0) { return _ENUM; }
#define CHECK_END(_ITER) if(_ITER == tokens.end()) { return FALSE; }
GXBlend GXDLLAPI MOStringToBlend(GXLPCSTR str, size_t len)
{
  CMP_STR("zero", GXBLEND_ZERO)
  else CMP_STR("one", GXBLEND_ONE)
  else CMP_STR("srccolor", GXBLEND_SRCCOLOR)
  else CMP_STR("oneminussrccolor", GXBLEND_INVSRCCOLOR)
  else CMP_STR("srcalpha", GXBLEND_SRCALPHA)
  else CMP_STR("oneminussrcalpha", GXBLEND_INVSRCALPHA)
  else CMP_STR("destalpha", GXBLEND_DESTALPHA)
  else CMP_STR("oneminusdestalpha", GXBLEND_INVDESTALPHA)
  else CMP_STR("destcolor", GXBLEND_DESTCOLOR)
  else CMP_STR("oneminusdestcolor", GXBLEND_INVDESTCOLOR)
  //else CMP_STR("one", GXBLEND_SRCALPHASAT);
  //else CMP_STR("one", GXBLEND_BOTHSRCALPHA);
  //else CMP_STR("one", GXBLEND_BOTHINVSRCALPHA);
  //else CMP_STR("one", GXBLEND_BLENDFACTOR);
  //else CMP_STR("one", GXBLEND_INVBLENDFACTOR);
  //else CMP_STR("one", GXBLEND_SRCCOLOR2);
  //else CMP_STR("one", GXBLEND_INVSRCCOLOR2);
  return GXBLEND_FORCE_DWORD;
}


GXBlendOp GXDLLAPI MOStringToBlendOp(GXLPCSTR str, size_t len)
{
  CMP_STR("add", GXBLENDOP_ADD)
  else CMP_STR("subtract", GXBLENDOP_SUBTRACT)
  else CMP_STR("revsubtract", GXBLENDOP_REVSUBTRACT)
  else CMP_STR("min", GXBLENDOP_MIN)
  else CMP_STR("max", GXBLENDOP_MAX)
  return GXBLENDOP_FORCE_DWORD;
}

GXStencilOp GXDLLAPI MOStringToStencilOp(GXLPCSTR str, size_t len)
{
  CMP_STR("keep", GXSTENCILOP_KEEP)
  else CMP_STR("zero", GXSTENCILOP_ZERO)
  else CMP_STR("replace", GXSTENCILOP_REPLACE)
  else CMP_STR("incrsat", GXSTENCILOP_INCRSAT)
  else CMP_STR("decrsat", GXSTENCILOP_DECRSAT)
  else CMP_STR("invert", GXSTENCILOP_INVERT)
  else CMP_STR("incr", GXSTENCILOP_INCR)
  else CMP_STR("decr", GXSTENCILOP_DECR)
  return GXSTENCILOP_FORCE_DWORD;
}

GXCullMode GXDLLAPI MOStringToCullMode(GXLPCSTR str, size_t len)
{
  CMP_STR("none", GXCULL_NONE)
  else CMP_STR("cw", GXCULL_CW)
  else CMP_STR("ccw", GXCULL_CCW)
  return GXCULL_FORCE_DWORD;
}

GXCompareFunc GXDLLAPI MOStringToCompareFunc(GXLPCSTR str, size_t len)
{
  CMP_STR("never", GXCMP_NEVER)
  else CMP_STR("less", GXCMP_LESS)
  else CMP_STR("equal", GXCMP_EQUAL)
  else CMP_STR("lessequal", GXCMP_LESSEQUAL)
  else CMP_STR("greater", GXCMP_GREATER)
  else CMP_STR("notequal", GXCMP_NOTEQUAL)
  else CMP_STR("greaterequal", GXCMP_GREATEREQUAL)
  else CMP_STR("always", GXCMP_ALWAYS)
  return GXCMP_FORCE_DWORD;
}

GXCHAR szBlend[] = "blend";
GXCHAR szColorMask[] = "colormask";
GXCHAR szOff[] = "off";

GXBOOL GXBLENDDESC::Parse(GXLPCSTR szLine)
{
  TokensA tokens(szLine, strlenT(szLine));
  TokensA::iterator iter = tokens.begin();
  return ::Parse(*this, tokens, iter);
}

// 解析成功返回TRUE
// iter步进到下一个解析位置
GXBOOL Parse(GXBLENDDESC& desc, clstd::TokensA& tokens, clstd::TokensA::iterator& iter)
{
  if (iter == szBlend)
  {
    ++iter;
    CHECK_END(iter);

    if (iter == szOff)
    {
      desc.BlendEnable = FALSE;
      desc.SrcBlend    = GXBLEND_ONE;
      desc.DestBlend   = GXBLEND_ZERO;
      ++iter;
      return TRUE;
    }

    clstd::TokensA::iterator iterSecond = iter + 1;
    CHECK_END(iterSecond);

    GXBlend _SrcBlend = MOStringToBlend(iter.marker, iter.length);
    GXBlend _DestBlend = MOStringToBlend(iterSecond.marker, iterSecond.length);
    if (_SrcBlend == GXBLEND_FORCE_DWORD || _DestBlend == GXBLEND_FORCE_DWORD) {
      iter = iterSecond + 1;
      return FALSE;
    }

    desc.SrcBlend = _SrcBlend;
    desc.DestBlend = _DestBlend;
    desc.BlendEnable = _CL_NOT_(_SrcBlend == GXBLEND_ONE && _DestBlend == GXBLEND_ZERO);
    iter = iterSecond + 1;
    return TRUE;
  }
  //else if (iter == szColorMask)
  //{
  //  ++iter;
  //  if (iter == tokens.end()) {
  //    return FALSE;
  //  }

  //  iter.
  //}
  return FALSE;
}

GXBOOL GXDEPTHSTENCILDESC::Parse(GXLPCSTR szLine)
{
  clstd::TokensA tokens(szLine, clstd::strlenT(szLine));
  clstd::TokensA::iterator iter = tokens.begin();
  return ::Parse(*this, tokens, iter);
}

GXBOOL Parse(GXDEPTHSTENCILDESC& desc, clstd::TokensA& tokens, clstd::TokensA::iterator& iter)
{
  CHECK_END(iter);
  if (iter == "zwrite")
  {
    ++iter;
    CHECK_END(iter);
    if (iter == "off") {
      desc.DepthEnable = FALSE;
      ++iter;
      return TRUE;
    }
    else if (iter == "on") {
      desc.DepthEnable = TRUE;
      ++iter;
      return TRUE;
    }
    ++iter;
  }
  return FALSE;
}

GXBOOL Parse(GXRASTERIZERDESC& desc, clstd::TokensA& tokens, clstd::TokensA::iterator& iter)
{
  CHECK_END(iter);
  if (iter == "cull")
  {
    ++iter;
    CHECK_END(iter);
    GXCullMode cull = MOStringToCullMode(iter.marker, iter.length);
    if(cull != GXCULL_FORCE_DWORD)
    {
      desc.CullMode = cull;
    }
    ++iter;
    return TRUE;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
