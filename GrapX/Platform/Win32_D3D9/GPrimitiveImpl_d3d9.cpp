#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.h>
#include "User/GrapX.Hxx"

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"

// 私有头文件
#include "GPrimitiveImpl_d3d9.h"
//#define _GXGRAPHICS_INLINE_PRIMITIVE_D3D9_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"

namespace D3D9
{
  GXBOOL GPrimitiveVertexOnlyImpl::CreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl)
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

  GXBOOL GPrimitiveVertexOnlyImpl::RestoreVertices()
  {
    if(m_pVertexBuffer != NULL) {
      GXLPVOID lpLocked = NULL;
      m_pD3D9VertexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT);
      memcpy(lpLocked, m_pVertexBuffer, m_uVertexCount * m_uVertexSize);
      m_pD3D9VertexBuffer->Unlock();
      return TRUE;
    }
    return FALSE;
  }

  GPrimitiveVertexOnlyImpl::GPrimitiveVertexOnlyImpl(GXGraphics* pGraphics, GXResUsage eUsage, GXUINT nVertexCount, GXUINT nVertexStride)
    : GPrimitive      (0, RESTYPE_PRIMITIVE)
    , m_pGraphicsImpl (static_cast<GXGraphicsImpl*>(pGraphics))
    , m_pD3D9VertexBuffer (NULL)
    , m_eResUsage     (eUsage)
    , m_uVertexCount  (nVertexCount)
    , m_uVertexSize   (nVertexStride)
    , m_pLockedVertex (NULL)
    , m_pVertexBuffer (NULL)
    , m_pVertexDecl   (NULL)
  {
  }

  GPrimitiveVertexOnlyImpl::~GPrimitiveVertexOnlyImpl()
  {
    if(m_pLockedVertex) {
      UnmapVertexBuffer(m_pLockedVertex);
    }

    if(m_pD3D9VertexBuffer) {
      m_pGraphicsImpl->UnregisterResource(this);
    }

    INVOKE_LOST_DEVICE;
    SAFE_RELEASE(m_pVertexDecl);
    SAFE_DELETE_ARRAY(m_pVertexBuffer);
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

  GXHRESULT GPrimitiveVertexOnlyImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD dwUsage;
    D3DPOOL ePool;
    ConvertPrimResUsageToNative(&dwUsage, &ePool, m_eResUsage);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
    {
      SAFE_RELEASE(m_pD3D9VertexBuffer);
      return GX_OK;
    }
    break;
    case RC_ResetDevice:
    {
      if(m_eResUsage != GXResUsage::SystemMem)
      {
        LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

        if(SUCCEEDED(lpd3dDevice->CreateVertexBuffer(
          m_uVertexCount * m_uVertexSize, dwUsage, NULL, ePool, &m_pD3D9VertexBuffer, NULL)))
        {
          RestoreVertices();
          return GX_OK;
        }
      }
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
    CreateVertexDeclaration(pVertexDecl);
    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;

    if(m_uVertexSize == 0) {
      m_uVertexSize = m_pVertexDecl->GetStride();
    }


    if(Invoke(&Desc) == GX_OK)
    {
      // 这个应该在 Invoke ResetDevice 之后创建, 否则会有冗余的内存复制
      m_pVertexBuffer = new GXBYTE[m_uVertexCount * m_uVertexSize];

      // Update Init Data
      if(pVertInitData != NULL)
      {
        memcpy(m_pVertexBuffer, pVertInitData, m_uVertexCount * m_uVertexSize);
        RestoreVertices();
      }
      return TRUE;
    }

    return FALSE;
  }

  GXLPVOID GPrimitiveVertexOnlyImpl::MapVertexBuffer(GXResMap eMap)
  {
    if(m_pLockedVertex != NULL) {
      return NULL;
    }

    HRESULT hr = S_OK;

    switch(m_eResUsage)
    {
    case GXResUsage_Default:
      break;
    case GXResUsage_Write:
    case GXResUsage_Read:
    case GXResUsage_ReadWrite:
      hr = m_pD3D9VertexBuffer->Lock(0, m_uVertexCount * m_uVertexSize, &m_pLockedVertex, D3DLOCK_DISCARD);
      if(SUCCEEDED(hr)) {
        return m_pVertexBuffer;
      }
      break;
    case GXResUsage_SystemMem:
      return m_pVertexBuffer;
    }

    return NULL;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::UnmapVertexBuffer(GXLPVOID lpMappedBuffer)
  {
    if(lpMappedBuffer != m_pVertexBuffer) {
      return FALSE;
    }

    memcpy(m_pLockedVertex, m_pVertexBuffer, m_uVertexCount * m_uVertexSize);
    m_pD3D9VertexBuffer->Unlock();
    m_pLockedVertex = NULL;;

    return TRUE;
  }

  GXLPVOID GPrimitiveVertexOnlyImpl::MapIndexBuffer(GXResMap eMap)
  {
    return NULL;
  }

  GXBOOL GPrimitiveVertexOnlyImpl::UnmapIndexBuffer(GXLPVOID lpMappedBuffer)
  {
    return FALSE;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetVertexCount()
  {
    return m_uVertexCount;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetVertexStride()
  {
    return m_uVertexSize;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetIndexCount()
  {
    return 0;
  }

  GXUINT GPrimitiveVertexOnlyImpl::GetIndexStride()
  {
    return 0;
  }

  GXHRESULT GPrimitiveVertexOnlyImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    return InlGetSafeObjectT<GVertexDeclaration>(ppDeclaration, m_pVertexDecl);
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
    , m_pD3D9IndexBuffer   (NULL)
    , m_uIndexCount    (nIndexCount)
    , m_uIndexSize     (nIndexStride)
    , m_pLockedIndex   (NULL)
    , m_pIndexBuffer   (NULL)
  {
  }

  GPrimitiveVertexIndexImpl::~GPrimitiveVertexIndexImpl()
  {
    if(m_pLockedIndex) {
      UnmapVertexBuffer(m_pLockedIndex);
    }

    INVOKE_LOST_DEVICE;
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
    //ASSERT((m_uRefCount & 0x80000000) == 0);
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
        SAFE_RELEASE(m_pD3D9IndexBuffer);
        SAFE_RELEASE(m_pD3D9VertexBuffer);
        return GX_OK;
      }
      break;
    case RC_ResetDevice:
      {
        LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

        DWORD dwUsage;
        D3DPOOL ePool;
        ConvertPrimResUsageToNative(&dwUsage, &ePool, m_eResUsage);

        if(SUCCEEDED(lpd3dDevice->CreateVertexBuffer(m_uVertexCount * m_uVertexSize, dwUsage, NULL, ePool, &m_pD3D9VertexBuffer, NULL)))
        {
          RestoreVertices();
          if(SUCCEEDED(lpd3dDevice->CreateIndexBuffer(m_uIndexCount * m_uIndexSize, dwUsage, m_uIndexSize == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, ePool, &m_pD3D9IndexBuffer, NULL)))
          {
            RestoreIndices();
            return GX_OK;
          }
        }
      }
      break;
    case RC_ResizeDevice:
      break;
    }
    return GX_FAIL;
  }
  GXBOOL GPrimitiveVertexIndexImpl::InitPrimitive(LPCGXVERTEXELEMENT pVertexDecl, GXLPCVOID pVertInitData, GXLPCVOID pIndexInitData)
  {
    GPrimitiveVertexOnlyImpl::InitPrimitive(pVertexDecl, pVertInitData);

    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;


    if(Invoke(&Desc) == GX_OK)
    {
      // 这个应该在 Invok ResetDevice 之后创建, 否则会有冗余的内存复制
      m_pIndexBuffer = new GXBYTE[m_uIndexCount * m_uIndexSize];
      
      // Update Init Data
      if(pIndexInitData != NULL)
      {
        memcpy(m_pIndexBuffer, pIndexInitData, m_uIndexCount * m_uIndexSize);
        RestoreIndices();
      }

      return TRUE;
    }

    // 有可能其中一个创建成功了.
    SAFE_RELEASE(m_pD3D9IndexBuffer);
    SAFE_RELEASE(m_pD3D9VertexBuffer);
    return FALSE;
  }

  GXLPVOID GPrimitiveVertexIndexImpl::MapIndexBuffer(GXResMap eMap)
  {
    if(m_pLockedIndex != NULL) {
      return NULL;
    }

    HRESULT hr = S_OK;

    switch(m_eResUsage)
    {
    case GXResUsage_Default:
      break;
    case GXResUsage_Write:
    case GXResUsage_Read:
    case GXResUsage_ReadWrite:
      hr = m_pD3D9IndexBuffer->Lock(0, m_uIndexCount * m_uIndexSize, &m_pLockedIndex, D3DLOCK_DISCARD);
      if(SUCCEEDED(hr)) {
        return m_pIndexBuffer;
      }
      break;
    case GXResUsage_SystemMem:
      return m_pIndexBuffer;
    }

    return NULL;
  }


  GXBOOL GPrimitiveVertexIndexImpl::UnmapIndexBuffer(GXLPVOID lpMappedBuffer)
  {
    if(lpMappedBuffer != m_pIndexBuffer) {
      return FALSE;
    }

    memcpy(m_pLockedIndex, m_pIndexBuffer, m_uIndexCount * m_uIndexSize);
    m_pD3D9IndexBuffer->Unlock();
    m_pLockedIndex = NULL;

    return TRUE;
  }

  GXUINT GPrimitiveVertexIndexImpl::GetIndexCount()
  {
    return m_uIndexCount;
  }
  
  GXUINT GPrimitiveVertexIndexImpl::GetIndexStride()
  {
    return m_uIndexSize;
  }

  GXGraphics* GPrimitiveVertexIndexImpl::GetGraphicsUnsafe()
  {
    return static_cast<GXGraphics*>(m_pGraphicsImpl);
  }

  GXINT GPrimitiveVertexIndexImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
  }

  GXBOOL GPrimitiveVertexIndexImpl::RestoreIndices()
  {
    if(m_pIndexBuffer != NULL) {
      GXLPVOID lpLocked = NULL;
      m_pD3D9IndexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT);
      memcpy(lpLocked, m_pIndexBuffer, m_uIndexCount * m_uIndexSize);
      m_pD3D9IndexBuffer->Unlock();
      return TRUE;
    }
    return FALSE;
  }
}
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
