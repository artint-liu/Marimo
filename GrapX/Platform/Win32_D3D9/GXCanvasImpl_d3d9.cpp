#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_EFFECT_D3D9_
#define _GXGRAPHICS_INLINE_SHADER_D3D9_
#define _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D9_
#define _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_
#define _GXGRAPHICS_INLINE_SET_BLEND_STATE_
#define _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_
#define _GXGRAPHICS_INLINE_SET_RENDER_STATE_
#define _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_

// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GPrimitive.H"
#include "GrapX/GStateBlock.H"
#include "GrapX/GRegion.H"
#include "GrapX/GTexture.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXFont.H"
#include "GrapX/GXImage.H"
#include "GrapX/GShader.H"
#include "GrapX/GXKernel.H"

// ƽ̨���
#include "GrapX/Platform.h"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Canvas/GXEffectImpl.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
#include "Platform/Win32_D3D9/GTextureImpl_d3d9.H"
#include "Platform/Win32_D3D9/GStateBlock_d3d9.H"

// ˽��ͷ�ļ�
#include <clUtility.h>
#include <GrapX/VertexDecl.H>
#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.H"
#include "GrapX/GXUser.H"
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>
#include <GrapX/gxDevice.H>
//#include <User/GXWindow.h>
#include <Utility/hlsl/FXCommRegister.H>
#include <GrapX/GCamera.h>


#define TRACE_BATCH

//////////////////////////////////////////////////////////////////////////
namespace D3D9
{
#include "Canvas/GXCanvas_Text.inl"
#define D3D9_CANVAS_IMPL
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/GXCanvasImpl.inl"
#include "Canvas/GXCanvasCoreImpl.inl"

  GXBOOL GXCanvasImpl::SetEffectUniformByName1f (const GXCHAR* pName, const float fValue)
  {
    if(!((m_uBatchCount + 1) < m_uBatchSize))
      Flush();
    GXUINT uHandle = m_CallState.pEffectImpl->GetHandle(pName);
    m_aBatch[m_uBatchCount].eFunc = CF_SetUniform1f;
    m_aBatch[m_uBatchCount].Handle = uHandle;
    m_aBatch[m_uBatchCount].PosF.x = fValue;
    m_aBatch[m_uBatchCount].PosF.y = 0;
    m_aBatch[m_uBatchCount].PosF.z = 0;
    m_aBatch[m_uBatchCount].PosF.w = 1;
    m_uBatchCount++;
    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniformByName2f (const GXCHAR* pName, const float2* vValue)
  {
    if(!((m_uBatchCount + 1) < m_uBatchSize))
      Flush();
    GXUINT uHandle = m_CallState.pEffectImpl->GetHandle(pName);
    m_aBatch[m_uBatchCount].eFunc = CF_SetUniform2f;
    m_aBatch[m_uBatchCount].Handle = uHandle;
    m_aBatch[m_uBatchCount].PosF.x = vValue->x;
    m_aBatch[m_uBatchCount].PosF.y = vValue->y;
    m_aBatch[m_uBatchCount].PosF.z = 0;
    m_aBatch[m_uBatchCount].PosF.w = 1;
    m_uBatchCount++;
    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniformByName3f (const GXCHAR* pName, const float3* fValue)
  {
    if(!((m_uBatchCount + 1) < m_uBatchSize))
      Flush();
    GXUINT uHandle = m_CallState.pEffectImpl->GetHandle(pName);
    m_aBatch[m_uBatchCount].eFunc = CF_SetUniform3f;
    m_aBatch[m_uBatchCount].Handle = uHandle;
    m_aBatch[m_uBatchCount].PosF.x = fValue->x;
    m_aBatch[m_uBatchCount].PosF.y = fValue->y;
    m_aBatch[m_uBatchCount].PosF.z = fValue->z;
    m_aBatch[m_uBatchCount].PosF.w = 1;
    m_uBatchCount++;

    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniformByName4f (const GXCHAR* pName, const float4* fValue)
  {
    if(!((m_uBatchCount + 1) < m_uBatchSize))
      Flush();
    GXUINT uHandle = m_CallState.pEffectImpl->GetHandle(pName);
    m_aBatch[m_uBatchCount].eFunc = CF_SetUniform4f;
    m_aBatch[m_uBatchCount].Handle = uHandle;
    m_aBatch[m_uBatchCount].PosF.x = fValue->x;
    m_aBatch[m_uBatchCount].PosF.y = fValue->y;
    m_aBatch[m_uBatchCount].PosF.z = fValue->z;
    m_aBatch[m_uBatchCount].PosF.w = fValue->w;
    m_uBatchCount++;
    return TRUE;
  }
  
  GXBOOL GXCanvasImpl::SetEffectUniformByName4x4(const GXCHAR* pName, const float4x4* pValue)
  {
    if(!((m_uBatchCount + 1) < m_uBatchSize))
      Flush();
    GXUINT uHandle = m_CallState.pEffectImpl->GetHandle(pName);
    float4x4* pMat = new float4x4;
    m_aBatch[m_uBatchCount].eFunc = CF_SetUniform4x4f;
    m_aBatch[m_uBatchCount].Handle = uHandle;
    m_aBatch[m_uBatchCount].comm.lParam = (GXLPARAM)pMat;
    *pMat = *pValue;
    m_uBatchCount++;
    return TRUE;
  }

  GXBOOL GXCanvasImpl::SetEffectUniform1f(const GXINT nIndex, const float fValue)
  {
    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniform2f(const GXINT nIndex, const float2* vValue)
  {
    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniform3f(const GXINT nIndex, const float3* fValue)
  {
    return TRUE;
  }
  GXBOOL GXCanvasImpl::SetEffectUniform4f(const GXINT nIndex, const float4* fValue)
  {
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
} // namespace D3D9

#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
