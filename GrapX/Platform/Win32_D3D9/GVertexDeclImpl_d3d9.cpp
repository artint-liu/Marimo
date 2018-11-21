#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXKernel.h"
//#include "Include/DataPool.h"
//#include "Include/DataPoolVariable.h"
//#include <vector>

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"

// 私有头文件
//#include <Driver/Shader/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
//#include "include/GXCanvas3D.h"
#include "GVertexDeclImpl_d3d9.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.h"
//#include "clstd/clPathFile.h"

namespace D3D9
{
  GVertexDeclImpl::GVertexDeclImpl(GraphicsImpl* pGraphics)
    : GVertexDeclaration()
    , m_pGraphics       (pGraphics)
    , m_nStride         (NULL)
    , m_pVertexElement  (NULL)
  {
  }

  GVertexDeclImpl::~GVertexDeclImpl()
  {
    TRACE("this:%x id:%d\n", this, gxGetCurrentThreadId());
    SAFE_RELEASE(m_pDecl);
    m_pGraphics->UnregisterResource(this);
    SAFE_DELETE_ARRAY(m_pVertexElement);
  }

  GXHRESULT GVertexDeclImpl::Initialize(LPCGXVERTEXELEMENT lpVertexElement)
  {
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
    // 默认的顶点声明
    clBuffer* pVertDeclBuf;
    ConvertVertexDeclToNative(lpVertexElement, &pVertDeclBuf);
    HRESULT hval = lpd3dDevice->CreateVertexDeclaration((const D3DVERTEXELEMENT9*)pVertDeclBuf->GetPtr(), &m_pDecl);
    if(GXFAILED(hval))
    {
      ASSERT(0);
    }
    SAFE_DELETE(pVertDeclBuf);

    // 复制原有的声明
    const GXUINT nCount = MOGetDeclCount(lpVertexElement) + 1;
    m_pVertexElement = new GXVERTEXELEMENT[nCount];
    memcpy(m_pVertexElement, lpVertexElement, nCount * sizeof(GXVERTEXELEMENT));

    m_nStride = MOGetDeclVertexSize(lpVertexElement);
    return hval;
  }

  GXHRESULT GVertexDeclImpl::Activate()
  {
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
    return lpd3dDevice->SetVertexDeclaration(m_pDecl);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GVertexDeclImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GVertexDeclImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXUINT GVertexDeclImpl::GetStride()
  {
    return m_nStride;       
  }

  GXINT GVertexDeclImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return MOGetDeclOffset(m_pVertexElement, Usage, UsageIndex, lpDesc);
  }

  GXLPCVERTEXELEMENT GVertexDeclImpl::GetVertexElement()
  {
    return m_pVertexElement;
  }

} // namespace D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
