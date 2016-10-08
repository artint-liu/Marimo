#ifndef _STANDARD_MATERIAL_DEFINE_H_
#define _STANDARD_MATERIAL_DEFINE_H_

#ifdef REFACTOR_SHADER
struct STD_CANVAS_UNIFORM
{
  GXINT_PTR id_matViewProj;             // ViewProjection;
  GXINT_PTR id_matViewProjInv;          // ViewProjectionInverse;

  GXINT_PTR id_matView;                 // View;
  GXINT_PTR id_matViewInv;              // ViewInverse;

  GXINT_PTR id_matProj;                 // Projection;
  GXINT_PTR id_matProjInv;              // ProjectionInverse;

  GXINT_PTR id_matWorldViewProj;        // WorldViewProjection;
  GXINT_PTR id_matWorldViewProjInv;     // WorldViewProjectionInverse;

  GXINT_PTR id_matWorld;                // World;
  GXINT_PTR id_matWorldInv;             // WorldInverse;

  GXINT_PTR id_matWorldView;            // WorldView;
  GXINT_PTR id_matWorldViewInv;         // WorldViewInverse;

  GXINT_PTR id_matMainLight;            // 主光空间的变换矩阵

  GXINT_PTR id_vTime;                    // x: sin(vTime.z), y: cos(vTime.z), z: (TickCount % 10000) * 1e-3f * 2 * PI, w:TickCount * 1e-3f
  GXINT_PTR id_fFPS;                     // FPS;
  GXINT_PTR id_fFOV;                     // FOV;

  GXINT_PTR id_vViewportDim;             // ViewportDimensions;
  GXINT_PTR id_vViewportDimInv;          // ViewportDimensionsInverse;
  GXINT_PTR id_fFarClipPlane;            // FarClipPlane;
  GXINT_PTR id_fNearClipPlane;           // NearClipPlane;

  GXINT_PTR id_fMouseCoordX;             // MouseCoordinateX;
  GXINT_PTR id_fMouseCoordY;             // MouseCoordinateY;

  GXINT_PTR id_vViewDir;                 // ViewDirection;
  GXINT_PTR id_vViewPos;                 // ViewPosition;

  // TODO: 光源相关的不要放在引擎里，要放在客户端，因为客户端已经具备了自定义通用常量的能力
  GXINT_PTR id_vMainLightDir;            // TODO: 移走！
  GXINT_PTR id_vLightDiffuse;            // TODO: 移走！
  GXINT_PTR id_fLightIntensity;          // TODO: 移走！ 
  GXINT_PTR id_vLightAmbient;            // TODO: 移走！ 环境光颜色
};
#endif

struct STANDARDMTLUNIFORMTABLE
{
  float4x4 g_matViewProj;             // ViewProjection;
  float4x4 g_matViewProjInv;          // ViewProjectionInverse;

  float4x4 g_matView;                 // View;
  float4x4 g_matViewInv;              // ViewInverse;

  float4x4 g_matProj;                 // Projection;
  float4x4 g_matProjInv;              // ProjectionInverse;

  float4x4 g_matWorldViewProj;        // WorldViewProjection;
  float4x4 g_matWorldViewProjInv;     // WorldViewProjectionInverse;

  float4x4 g_matWorld;                // World;
  float4x4 g_matWorldInv;             // WorldInverse;

  float4x4 g_matWorldView;            // WorldView;
  float4x4 g_matWorldViewInv;         // WorldViewInverse;
  
  float4x4 g_matMainLight;            // 主光空间的变换矩阵

  //float4x4 g_matViewProjInvTran;      // ViewProjectionInverseTranspose;
  //float4x4 g_matViewProjTran;         // ViewProjectionTranspose;
  //float4x4 g_matViewTran;             // ViewTranspose;
  //float4x4 g_matViewInvTran;          // ViewInverseTranspose;
  //float4x4 g_matProjTran;             // ProjectionTranspose;
  //float4x4 g_matProjInvTran;          // ProjectionInverseTranspose;
  //float4x4 g_matWorldViewProjTran;    // WorldViewProjectionTranspose;
  //float4x4 g_matWorldViewProjInvTran; // WorldViewProjectionInverseTranspose;
  //float4x4 g_matWorldTran;            // WorldTranspose;
  //float4x4 g_matWorldInvTran;         // WorldInverseTranspose;
  //float4x4 g_matWorldViewTran;        // WorldViewTranspose;
  //float4x4 g_matWorldViewInvTran;     // WorldViewInverseTranspose;

  //float   g_fTime0_X;                 // Time0_X;
  float4  g_vTime;                    // x: sin(vTime.z), y: cos(vTime.z), z: (TickCount % 10000) * 1e-3f * 2 * PI, w:TickCount * 1e-3f
  float   g_fFPS;                     // FPS;
  float   g_fFOV;                     // FOV;

  float2  g_vViewportDim;             // ViewportDimensions;
  float2  g_vViewportDimInv;          // ViewportDimensionsInverse;
  float   g_fFarClipPlane;            // FarClipPlane;
  float   g_fNearClipPlane;           // NearClipPlane;

  float   g_fMouseCoordX;             // MouseCoordinateX;
  float   g_fMouseCoordY;             // MouseCoordinateY;

  float3  g_vViewDir;                 // ViewDirection;
  float3  g_vViewPos;                 // ViewPosition;
  float3  g_vMainLightDir;            //
  float3  g_vLightDiffuse;            //
  float   g_fLightIntensity;
  float3  g_vLightAmbient;            // 环境光颜色
};

#endif // _STANDARD_MATERIAL_DEFINE_H_