#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

// ȫ��ͷ�ļ�
#include <GrapX.h>

// ��׼�ӿ�
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"

// ƽ̨���
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// ˽��ͷ�ļ�
//#include "Platform/Win32_D3D11/GStateBlock_D3D11.h"
#define _GXGRAPHICS_INLINE_RENDERSTATE_
#include "Canvas/GXResourceMgr.h"
#include "GVertexDeclImpl_d3d11.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
//#include "User/GrapX.Hxx"
#include "GrapX/gxError.h"

namespace D3D11   
{
  GVertexDeclImpl::GVertexDeclImpl( GXGraphicsImpl* pGraphics ) 
    : GVertexDeclaration()
    , m_pGraphics(pGraphics)
    , m_NumDescs(0)
    , m_nStride(NULL)
  {
  }

  GVertexDeclImpl::~GVertexDeclImpl()
  {
    //SAFE_RELEASE(m_pDecl);
    SAFE_DELETE_ARRAY(m_pVertexElement);
    m_pGraphics->UnregisterResource(this);
  }

  GXHRESULT GVertexDeclImpl::Initialize(LPCGXVERTEXELEMENT lpVertexElement)
  {
    //LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
    //ID3D11Device* const pd3dDevice = m_pGraphics->D3DGetDevice();
    // Ĭ�ϵĶ�������
    //clBuffer* pVertDeclBuf;
    GrapXToDX11::VertexLayoutFromVertexDecl(lpVertexElement, &m_aDescs);

    m_NumDescs = (UINT)m_aDescs.size();
    if(m_aDescs.back().SemanticName.IsEmpty()) {
      m_NumDescs--;
    }

    // ����ԭ�е�����
    const GXUINT nCount = MOGetDeclCount(lpVertexElement) + 1;
    m_pVertexElement = new GXVERTEXELEMENT[nCount];
    memcpy(m_pVertexElement, lpVertexElement, nCount * sizeof(GXVERTEXELEMENT));

    //pd3dDevice->CreateInputLayout(&aDesc.front(), uSize, )
    //ConvertVertexDeclToNative(lpVertexElement, &pVertDeclBuf);
    //HRESULT hval = lpd3dDevice->CreateVertexDeclaration((const D3DVERTEXELEMENT9*)pVertDeclBuf->GetPtr(), &m_pDecl);
    //if(GXFAILED(hval))
    //{
    //  ASSERT(0);
    //}
    //SAFE_DELETE(pVertDeclBuf);

    m_nStride = MOGetDeclVertexSize(lpVertexElement);
    //return hval;
    return GX_OK;       
  }

  GXHRESULT GVertexDeclImpl::Activate()
  {
    //ID3D11Device* lpd3dDevice = m_pGraphics->D3DGetDevice();
    //return lpd3dDevice->SetVertexDeclaration(m_pDecl);
    //m_pGraphics->m_pImmediateContext->IASetInputLayout(m_pDecl);
    ASSERT(0);
    return GX_OK;
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
} // namespace D3D11

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)