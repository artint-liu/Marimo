#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.h>
#include "User/GrapX.Hxx"

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_d3d11.h"

// 私有头文件
#include "GPrimitiveImpl_d3d11.h"
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"

#ifdef ENABLE_GRAPHICS_API_DX11
namespace D3D11
{
  GXBOOL GPrimitiveVertexOnlyImpl::IntCreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl)
  {
    GXBOOL bval = FALSE;
    SAFE_RELEASE(m_pVertexDecl);

    if(pVertexDecl != NULL) {
      GVertexDeclaration* pVertDeclObj = NULL;
      bval = GXSUCCEEDED(m_pGraphicsImpl->CreateVertexDeclaration(&pVertDeclObj, pVertexDecl));
      if(bval) {
        m_pVertexDecl = static_cast<GVertexDeclImpl*>(pVertDeclObj);
      }
    }    
    return bval;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::IntCreateBuffer(ID3D11Buffer** ppD3D11Buffer, GXUINT nSize, GXUINT nBindFlags, GXLPCVOID pInitData)
  {
    ASSERT(*ppD3D11Buffer == NULL);

    D3D11_BUFFER_DESC bd;
    InlSetZeroT(bd);

    GrapXToDX11::PrimitiveDescFromResUsage(&bd, m_eUsage);

    bd.ByteWidth = nSize;
    bd.BindFlags = nBindFlags;

    ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    HRESULT hr = S_OK;

    if(pInitData) {
      D3D11_SUBRESOURCE_DATA InitData;
      InlSetZeroT(InitData);
      InitData.pSysMem = pInitData;
      hr = pd3dDevice->CreateBuffer(&bd, &InitData, ppD3D11Buffer);
    }
    else {
      hr = pd3dDevice->CreateBuffer(&bd, NULL, ppD3D11Buffer);
    }

    return SUCCEEDED(hr);
  }

  LPCGXVERTEXELEMENT GPrimitiveVertexOnlyImpl::GetVertexDeclUnsafe()
  {
    return NULL;//m_pVertexDecl->get;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GPrimitiveVertexOnlyImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GPrimitiveVertexOnlyImpl::Release()
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

  GPrimitiveVertexOnlyImpl::GPrimitiveVertexOnlyImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride)
    : GPrimitive  (0, RESTYPE_PRIMITIVE)
    , m_pGraphicsImpl(static_cast<GXGraphicsImpl*>(pGraphics))
    , m_pD3D11VertexBuffer(NULL)
    , m_uVertexCount(nVertexCount)
    , m_uVertexStride(nVertexStride)
    , m_pVertexBuffer(NULL)
    , m_pVertexDecl(NULL)
    , m_eUsage(eUsage)
  {
    InlSetZeroT(m_sVertexMapped);
  }

  GPrimitiveVertexOnlyImpl::~GPrimitiveVertexOnlyImpl()
  {
    if(m_sVertexMapped.pData) {
      UnmapVertexBuffer(m_sVertexMapped.pData);
    }

    if(m_pD3D11VertexBuffer) {
      m_pGraphicsImpl->UnregisterResource(this);
    }

    SAFE_RELEASE(m_pVertexDecl);
    SAFE_RELEASE(m_pD3D11VertexBuffer);
    SAFE_DELETE_ARRAY(m_pVertexBuffer);
  }


  GXHRESULT GPrimitiveVertexOnlyImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
    {
      return GX_OK;
    }
    break;
    case RC_ResetDevice:
    {
      return S_OK;
    }
    break;
    case RC_ResizeDevice:
      break;
    }
    return GX_FAIL;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData)
  {
    // 创建顶点声明
    IntCreateVertexDeclaration(pVertexDecl);

    GXBOOL result = TRUE;
    if(m_uVertexStride == 0) {
      m_uVertexStride = m_pVertexDecl->GetStride();
    }

    if(m_eUsage != GXResUsage::GXResUsage_SystemMem)
    {
      result = IntCreateBuffer(&m_pD3D11VertexBuffer, m_uVertexCount * m_uVertexStride, D3D11_BIND_VERTEX_BUFFER, pVertInitData);
    }

    if(result)
    {
      if(m_eUsage == GXResUsage::GXResUsage_Read || m_eUsage == GXResUsage::GXResUsage_ReadWrite || m_eUsage == GXResUsage::GXResUsage_SystemMem)
      {
        m_pVertexBuffer = new GXBYTE[m_uVertexCount * m_uVertexStride];
        if(pVertInitData) {
          memcpy(m_pVertexBuffer, pVertInitData, m_uVertexCount * m_uVertexStride);
        }
      }
      return TRUE;
    }

    return FALSE;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::EnableDiscard(GXBOOL bDiscard)
  {
    return FALSE;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::IsDiscardable()
  {
    return TRUE;
  }

  GXLPVOID GPrimitiveVertexOnlyImpl::IntMapBuffer(GXResMap eMap, ID3D11Buffer* pD3D11Buffer, D3D11_MAPPED_SUBRESOURCE& rMappedDesc, GXLPBYTE pMemBuffer)
  {
    // 不能嵌套调用Map/Unmap
    if(rMappedDesc.pData) {
      return NULL;
    }

    if(m_eUsage == GXResUsage::GXResUsage_Default)
    {
    }
    else if(m_eUsage == GXResUsage::GXResUsage_Write)
    {
      if(eMap == GXResMap::GXResMap_Write) {
        if(SUCCEEDED(m_pGraphicsImpl->m_pImmediateContext->Map(pD3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rMappedDesc))) {
          return rMappedDesc.pData;
        }
      }
    }
    else if(m_eUsage == GXResUsage::GXResUsage_Read)
    {
      if(eMap == GXResMap::GXResMap_Read) {
        return pMemBuffer;
      }
    }
    else if(m_eUsage == GXResUsage::GXResUsage_ReadWrite)
    {
      if(eMap == GXResMap::GXResMap_Read) {
        return pMemBuffer;
      }
      else if(eMap == GXResMap::GXResMap_Write) {
        if(SUCCEEDED(m_pGraphicsImpl->m_pImmediateContext->Map(pD3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rMappedDesc))) {
          return rMappedDesc.pData;
        }
      }
      else if(eMap == GXResMap::GXResMap_ReadWrite) {
        if(SUCCEEDED(m_pGraphicsImpl->m_pImmediateContext->Map(pD3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rMappedDesc))) {
          return pMemBuffer;
        }
      }
    }
    else if(m_eUsage == GXResUsage::GXResUsage_SystemMem)
    {
      return pMemBuffer;
    }
    return NULL;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::IntUnmapBuffer(GXLPVOID lpMappedBuffer, ID3D11Buffer* pD3D11Buffer, D3D11_MAPPED_SUBRESOURCE& rMappedDesc, GXLPBYTE pMemBuffer)
  {
    if(lpMappedBuffer != rMappedDesc.pData && lpMappedBuffer == pMemBuffer) {
      return FALSE;
    }

    if(lpMappedBuffer == pMemBuffer) {
      memcpy(rMappedDesc.pData, pMemBuffer, rMappedDesc.RowPitch);
    }

    m_pGraphicsImpl->m_pImmediateContext->Unmap(pD3D11Buffer, 0);
    InlSetZeroT(rMappedDesc);
    return TRUE;
  }


  GXLPVOID GPrimitiveVertexOnlyImpl::MapVertexBuffer(GXResMap eMap)
  {
    return IntMapBuffer(eMap, m_pD3D11VertexBuffer, m_sVertexMapped, m_pVertexBuffer);
  }

  GXBOOL GPrimitiveVertexOnlyImpl::UnmapVertexBuffer(GXLPVOID lpMappedBuffer)
  {
    return IntUnmapBuffer(lpMappedBuffer, m_pD3D11VertexBuffer, m_sVertexMapped, m_pVertexBuffer);
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetVertexCount()
  {
    return m_uVertexCount;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetVertexStride()
  {
    return m_uVertexStride;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetIndexCount()
  {
    return 0;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetIndexStride()
  {
    return 0;
  }

  GXLPVOID GPrimitiveVertexOnlyImpl::MapIndexBuffer(GXResMap eResMap)
  {
    return NULL;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::UnmapIndexBuffer(GXLPVOID lpMappedBuffer)
  {
    return FALSE;
  }

  GXHRESULT GPrimitiveVertexOnlyImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    if(m_pVertexDecl == NULL)
      return GX_FAIL;
    m_pVertexDecl->AddRef();
    *ppDeclaration = m_pVertexDecl;
    return GX_OK;
  }

  GXGraphics* GPrimitiveVertexOnlyImpl::GetGraphicsUnsafe()
  {
    return static_cast<GXGraphics*>(m_pGraphicsImpl);
  }

  GXINT GPrimitiveVertexOnlyImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
  }

  //////////////////////////////////////////////////////////////////////////

  GPrimitiveVertexIndexImpl::GPrimitiveVertexIndexImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride, GXUINT nIndexCount, GXUINT nIndexStride)
    : GPrimitiveVertexOnlyImpl(pGraphics, eUsage, nVertexCount, nVertexStride)
    , m_pD3D11IndexBuffer (NULL)
    , m_uIndexCount       (nIndexCount)
    , m_uIndexStride      (nIndexStride)
    , m_pIndexBuffer      (NULL)
  {
    InlSetZeroT(m_sIndexMapped);
  }

  GPrimitiveVertexIndexImpl::~GPrimitiveVertexIndexImpl()
  {
    if(m_sIndexMapped.pData) {
      UnmapIndexBuffer(m_sIndexMapped.pData);
    }

    SAFE_RELEASE(m_pD3D11IndexBuffer);
    SAFE_DELETE_ARRAY(m_pIndexBuffer);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GPrimitiveVertexIndexImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GPrimitiveVertexIndexImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    ASSERT((m_uRefCount & 0x80000000) == 0);
    if(nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GPrimitiveVertexIndexImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      {
        return GX_OK;
      }
      break;
    case RC_ResetDevice:
      {        
        return S_OK;
      }
      break;
    case RC_ResizeDevice:
      break;
    }
    return GX_FAIL;
  }

  GXBOOL GPrimitiveVertexIndexImpl::InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData, GXLPCVOID pIndexInitData)
  {
    // 创建顶点声明
    GXBOOL result = GPrimitiveVertexOnlyImpl::InitPrimitive(pVertexDecl, pVertInitData);

    if(_CL_NOT_(result)) {
      return result;
    }

    if(m_eUsage != GXResUsage::GXResUsage_SystemMem)
    {
      result = IntCreateBuffer(&m_pD3D11IndexBuffer, m_uIndexCount * m_uIndexStride, D3D11_BIND_INDEX_BUFFER, pIndexInitData);
    }

    if(result)
    {
      if(m_eUsage == GXResUsage::GXResUsage_Read || m_eUsage == GXResUsage::GXResUsage_ReadWrite || m_eUsage == GXResUsage::GXResUsage_SystemMem)
      {
        m_pIndexBuffer = new GXBYTE[m_uIndexCount * m_uIndexStride];
        if(pIndexInitData)
        {
          memcpy(m_pIndexBuffer, pIndexInitData, m_uIndexCount * m_uIndexStride);
        }
      }
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL GPrimitiveVertexIndexImpl::EnableDiscard(GXBOOL bDiscard)
  {
    return FALSE;
  }

  GXBOOL GPrimitiveVertexIndexImpl::IsDiscardable()
  {
    return TRUE;
  }

  GXUINT GPrimitiveVertexIndexImpl::GetVertexCount()
  {
    return m_uVertexCount;
  }

  GXUINT GPrimitiveVertexIndexImpl::GetVertexStride()
  {
    return m_uVertexStride;
  }

  GXLPVOID GPrimitiveVertexIndexImpl::MapIndexBuffer(GXResMap eMap)
  {
    return IntMapBuffer(eMap, m_pD3D11IndexBuffer, m_sIndexMapped, m_pIndexBuffer);
  }

  GXBOOL GPrimitiveVertexIndexImpl::UnmapIndexBuffer(GXLPVOID lpMappedBuffer)
  {
    return IntUnmapBuffer(lpMappedBuffer, m_pD3D11IndexBuffer, m_sIndexMapped, m_pIndexBuffer);
  }

  GXUINT GPrimitiveVertexIndexImpl::GetIndexCount()
  {
    return m_uIndexCount;
  }

  GXUINT GPrimitiveVertexIndexImpl::GetIndexStride()
  {
    return m_uIndexStride;
  }

  GXHRESULT GPrimitiveVertexIndexImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    if(m_pVertexDecl == NULL)
      return GX_FAIL;
    m_pVertexDecl->AddRef();
    *ppDeclaration = m_pVertexDecl;
    return GX_OK;
  }

  GXGraphics* GPrimitiveVertexIndexImpl::GetGraphicsUnsafe()
  {
    return static_cast<GXGraphics*>(m_pGraphicsImpl);
  }

  GXINT GPrimitiveVertexIndexImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
  }

} // namespace D3D11
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)