#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// ȫ��ͷ�ļ�
#include <GrapX.h>
#include "User/GrapX.Hxx"

// ��׼�ӿ�
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

// ƽ̨���
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"

// ˽��ͷ�ļ�
#include "GPrimitiveImpl_d3d9.h"
//#define _GXGRAPHICS_INLINE_PRIMITIVE_D3D9_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"

namespace D3D9
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
  
  //GPrimImpl::GPrimImpl(GXGraphics* pGraphics)
  //  : m_pGraphicsImpl   ((GXGraphicsImpl*)pGraphics)
  //  , m_pVertexBuffer   (NULL)
  //  , m_uVertexSize    (0)
  //  , m_uVertexCount   (0)
  //  , m_pLockedVertex   (NULL)
  //  , m_dwResUsage      (NULL)
  //  , m_pVertexDecl     (NULL)
  //  , m_pVertices       (NULL)
  //{
  //}

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
      //OnDeviceEvent(DE_LostDevice);
      delete this;
      return GX_OK;
    }
    //else if(m_uRefCount == 1)
    //{
    //  return m_pGraphicsImpl->OldUnregisterResource(this);
    //}
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GPrimitiveVertexOnlyImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD dwUsage;
    D3DPOOL ePool;
    //ConvertPrimResUsageToNative(m_dwResUsage, &dwUsage, &ePool);
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
      if(m_eResUsage != GXResUsage_SystemMem)
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
    //m_uVertexCount = uElementCount;
    //m_uElementSize  = uElementSize;
    //m_dwResUsage    = ResUsage;

    // ������������
    CreateVertexDeclaration(pVertexDecl);
    //ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);
    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;

    //if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    if(Invoke(&Desc) == GX_OK)
    {
      // ���Ӧ���� Invoke ResetDevice ֮�󴴽�, �������������ڴ渴��
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

  //GXBOOL GPrimitiveVertexOnlyImpl::EnableDiscard(GXBOOL bDiscard)
  //{
  //  return FALSE;
  //}

  //GXBOOL GPrimitiveVertexOnlyImpl::IsDiscardable()
  //{
  //  return TRUE;
  //}

  GXLPVOID GPrimitiveVertexOnlyImpl::MapVertexBuffer(GXResMap eMap)
  {
    if(m_pLockedVertex != NULL) {
      return NULL;
    }

    //if(IsDiscardable() != FALSE)
    //{
    //  m_pVertexBuffer->Lock(0, m_uVertexCount * m_uVertexSize, &m_pLockedVertex, D3DLOCK_DISCARD);
    //  return m_pLockedVertex;
    //}
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
    //if(IsDiscardable() != FALSE)
    //{
    //  if(m_pVertices != NULL) {
    //    memcpy(m_pVertices, m_pLockedVertex, m_uVertexCount * m_uVertexSize);
    //  }

    //  m_pVertexBuffer->Unlock();
    //  m_pLockedVertex = NULL;
    //}
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
  //GPrimitive::Type GPrimitiveVImpl::GetType()
  //{
  //  return VertexOnly;
  //}
  
  //GXLPVOID GPrimitiveVertexOnlyImpl::GetVerticesBuffer()
  //{
  //  return m_pVertices;
  //}

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

  //GXBOOL GPrimitiveVertexOnlyImpl::UpdateResouce( ResEnum eRes )
  //{
  //  if(eRes == GPrimitive::ResourceAll || eRes == GPrimitive::ResourceVertices) {
  //    return RestoreVertices();
  //  }
  //  return FALSE;
  //}
  
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
    //if(uVertexCount == 0 || uVertexSize == 0 || uIndexCount == 0)
    //  return FALSE;

    //m_uVertexCount = uVertexCount;
    //m_uVertexSize  = uVertexSize;
    //m_uIndexCount   = uIndexCount;
    //m_dwResUsage    = ResUsage;

    // ������������
    //CreateVertexDeclaration(pVertexDecl);
    //ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);
    GPrimitiveVertexOnlyImpl::InitPrimitive(pVertexDecl, pVertInitData);

    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;


    //if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    if(Invoke(&Desc) == GX_OK)
    {
      // ���Ӧ���� Invok ResetDevice ֮�󴴽�, �������������ڴ渴��
      m_pIndexBuffer = new GXBYTE[m_uIndexCount * m_uIndexSize];
      
      // Update Init Data
      if(pIndexInitData != NULL)
      {
        memcpy(m_pIndexBuffer, pIndexInitData, m_uIndexCount * m_uIndexSize);
        RestoreIndices();

        //VIndex* pLockedIdx = (VIndex*)MapIndexBuffer(GXResMap_Write);

        ////memcpy(m_pVertices, pVertInitData, m_uVertexCount * m_uVertexSize);
        ////memcpy(m_pIndices, pIndexInitData, m_uIndexCount * sizeof(GXWORD));
        //if(pLockedIdx)
        //{
        //  //memcpy(pLockedVert, pVertInitData, uVertexCount * uVertexSize);
        //  memcpy(pLockedIdx, pIndexInitData, m_uIndexCount * m_uIndexSize);
        //  UnmapIndexBuffer(pLockedIdx);
        //}
        //else{
        //  ASSERT(0);
        //}
      }

      return TRUE;
    }

    // �п�������һ�������ɹ���.
    SAFE_RELEASE(m_pD3D9IndexBuffer);
    SAFE_RELEASE(m_pD3D9VertexBuffer);
    return FALSE;
  }

  //GXBOOL GPrimitiveVertexIndexImpl::EnableDiscard(GXBOOL bDiscard)
  //{
  //  return FALSE;
  //}

  //GXBOOL GPrimitiveVertexIndexImpl::IsDiscardable()
  //{
  //  return TRUE;
  //}

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
  //GPrimitive::Type GPrimitiveVIImpl::GetType()
  //{
  //  return Indexed;
  //}

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

  //GXBOOL GPrimitiveVertexIndexImpl::UpdateResouce( ResEnum eRes )
  //{
  //  switch(eRes)
  //  {
  //  case GPrimitive::ResourceIndices:
  //    return RestoreIndices();
  //  case GPrimitive::ResourceVertices:
  //    return RestoreVertices();
  //  case GPrimitive::ResourceAll:
  //    {
  //      const GXBOOL bval = RestoreIndices();
  //      return (RestoreVertices() && bval);
  //    }
  //  }
  //  return FALSE;
  //}

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
