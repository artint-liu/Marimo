#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

// 全局头文件
#include <GrapX.h>

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// 私有头文件
//#include "Platform/Win32_D3D11/GStateBlock_D3D11.h"
#define _GXGRAPHICS_INLINE_RENDERSTATE_
#include "Canvas/GXResourceMgr.h"
#include "GVertexDeclImpl_d3d11.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
//#include "User/GrapX.Hxx"
#include "GrapX/gxError.h"

namespace GrapX
{
  namespace D3D11
  {
    VertexDeclImpl::VertexDeclImpl(GraphicsImpl* pGraphics)
      : GVertexDeclaration()
      , m_pGraphics(pGraphics)
      , m_NumDescs(0)
      , m_nStride(NULL)
    {
    }

    VertexDeclImpl::~VertexDeclImpl()
    {
      //SAFE_RELEASE(m_pDecl);
      SAFE_DELETE_ARRAY(m_pVertexElement);
      m_pGraphics->UnregisterResource(this);
    }

    GXHRESULT VertexDeclImpl::Initialize(LPCGXVERTEXELEMENT lpVertexElement, const clStringA& strSketchName)
    {
      // 默认的顶点声明
      //clBuffer* pVertDeclBuf;
      GrapXToDX11::VertexLayoutFromVertexDecl(lpVertexElement, &m_aDescs);

      m_NumDescs = (UINT)m_aDescs.size();
      if(m_aDescs.back().SemanticName.IsEmpty()) {
        m_NumDescs--;
      }
      m_strSketchName = strSketchName;

      // 复制原有的声明
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

    GXHRESULT VertexDeclImpl::Activate()
    {
      //ID3D11Device* lpd3dDevice = m_pGraphics->D3DGetDevice();
      //return lpd3dDevice->SetVertexDeclaration(m_pDecl);
      //m_pGraphics->m_pImmediateContext->IASetInputLayout(m_pDecl);
      ASSERT(0);
      return GX_OK;
    }

    const clStringA& VertexDeclImpl::GetSketchName() const
    {
      return m_strSketchName;
    }

    const GrapXToDX11::GXD3D11InputElementDescArray& VertexDeclImpl::GetVertexLayoutDescArray() const
    {
      ASSERT(m_aDescs.size() == m_NumDescs + 1);
      return m_aDescs;
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT VertexDeclImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT VertexDeclImpl::Release()
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

    GXUINT VertexDeclImpl::GetStride()
    {
      return m_nStride;
    }

    GXINT VertexDeclImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
    {
      return MOGetDeclOffset(m_pVertexElement, Usage, UsageIndex, lpDesc);
    }

    GXLPCVERTEXELEMENT VertexDeclImpl::GetVertexElement()
    {
      return m_pVertexElement;
    }
  } // namespace D3D11
} // namespace GrapX

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)