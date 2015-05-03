#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GXGraphics.H"
#include "Include/GXCanvas.H"
#include "Include/GShader.H"
#include "Include/GTexture.H"
#include "Include/GXKernel.H"
//#include "Include/DataPool.H"
//#include "Include/DataPoolVariable.H"
//#include <vector>

// ƽ̨���
#include "Platform/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"

// ˽��ͷ�ļ�
//#include <Driver/Shader/VertexDecl.H>
#include "Canvas/GXResourceMgr.h"
//#include "include/GXCanvas3D.h"
#include "GVertexDeclImpl_d3d9.H"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
//#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.H"
//#include "clstd/clPathFile.H"

namespace D3D9
{
  GVertexDeclImpl::GVertexDeclImpl(GXGraphicsImpl* pGraphics)
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
    // Ĭ�ϵĶ�������
    clBuffer* pVertDeclBuf;
    ConvertVertexDeclToNative(lpVertexElement, &pVertDeclBuf);
    HRESULT hval = lpd3dDevice->CreateVertexDeclaration((const D3DVERTEXELEMENT9*)pVertDeclBuf->GetPtr(), &m_pDecl);
    if(GXFAILED(hval))
    {
      ASSERT(0);
    }
    SAFE_DELETE(pVertDeclBuf);

    // ����ԭ�е�����
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