#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.h>
#include "User/GrapX.Hxx"

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GShaderImpl_D3D11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_d3d11.h"

// 私有头文件
#include "GPrimitiveImpl_d3d11.h"
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"

#ifdef ENABLE_GRAPHICS_API_DX11
namespace D3D11
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  GPrimImpl::GPrimImpl(GXGraphics* pGraphics)
    : m_pGraphicsImpl   ((GXGraphicsImpl*)pGraphics)
    , m_pD3D11VertexBuffer   (NULL)
    , m_uElementSize    (0)
    , m_uElementCount   (0)
    , m_pVertMappedRes  (NULL)
    , m_dwResUsage      (NULL)
    , m_pVertexDecl     (NULL)
    , m_pVertices       (NULL)
  {
  }

  GXBOOL GPrimImpl::IntCreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl)
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

  HRESULT GPrimImpl::IntCreateBuffer(ID3D11Buffer** ppD3D11Buffer, GXUINT nSize, GXUINT nBindFlags, GXLPCVOID pInitData)
  {
    ASSERT(m_uElementCount > 0 && m_uElementSize > 0);
    ASSERT(*ppD3D11Buffer == NULL);

    D3D11_BUFFER_DESC bd;
    InlSetZeroT(bd);

    GrapXToDX11::PrimitiveDescFromResUsage(m_dwResUsage, &bd);

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

    return (hr);
  }

  LPCGXVERTEXELEMENT GPrimImpl::GetVertexDeclUnsafe()
  {
    return NULL;//m_pVertexDecl->get;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GPrimitiveVImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GPrimitiveVImpl::Release()
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

  GPrimitiveVImpl::GPrimitiveVImpl(GXGraphics* pGraphics)
    : GPrimitiveV  ()
    , GPrimImpl    (pGraphics)
  {
  }

  GPrimitiveVImpl::~GPrimitiveVImpl()
  {
    INVOKE_LOST_DEVICE;
    SAFE_RELEASE(m_pVertexDecl);
    SAFE_DELETE_ARRAY(m_pVertices);
    m_pGraphicsImpl->UnregisterResource(this);
  }


  GXHRESULT GPrimitiveVImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
    {
      SAFE_RELEASE(m_pD3D11VertexBuffer);
      return GX_OK;
    }
    break;
    case RC_ResetDevice:
    {
      const GXUINT nVerticesSize = m_uElementCount * m_uElementSize;
      SAFE_RELEASE(m_pD3D11VertexBuffer);

      if(m_pVertices)
      {
        return IntCreateBuffer(&m_pD3D11VertexBuffer, nVerticesSize, D3D11_BIND_VERTEX_BUFFER, m_pVertices);
      }
      else if(pDesc->lParam)
      {
        return IntCreateBuffer(&m_pD3D11VertexBuffer, nVerticesSize, D3D11_BIND_VERTEX_BUFFER, (GXLPCVOID)pDesc->lParam);
      }
      else
      {
        return IntCreateBuffer(&m_pD3D11VertexBuffer, nVerticesSize, D3D11_BIND_VERTEX_BUFFER, NULL);
      }

      return S_OK;
    }
    break;
    case RC_ResizeDevice:
      break;
    }
    return GX_FAIL;
  }

  GXBOOL GPrimitiveVImpl::InitPrimitive(GXLPCVOID pVertInitData, GXUINT uElementCount, GXUINT uElementSize, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage)
  {
    //ASSERT(pVertInitData == NULL); // TODO: 稍后支持初始化数据
    m_uElementCount = uElementCount;
    m_uElementSize  = uElementSize;
    m_dwResUsage    = ResUsage;

    // 创建顶点声明
    IntCreateVertexDeclaration(pVertexDecl);
    ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);
    ASSERT(m_pD3D11VertexBuffer == NULL);

    HRESULT hr = IntCreateBuffer(&m_pD3D11VertexBuffer, m_uElementCount * m_uElementSize, D3D11_BIND_VERTEX_BUFFER, pVertInitData);

    if(SUCCEEDED(hr))
    {
      // 这个应该在 OnDeviceEvent 之后创建, 否则会有冗余的内存复制
      if( ! (TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD) || 
        TEST_FLAG(ResUsage, GXRU_FREQUENTLYWRITE)))
      {
        m_pVertices = new GXBYTE[m_uElementCount * m_uElementSize];
      }

      return TRUE;
    }

    return FALSE;
  }

  GXBOOL GPrimitiveVImpl::EnableDiscard(GXBOOL bDiscard)
  {
    return FALSE;
  }

  GXBOOL GPrimitiveVImpl::IsDiscardable()
  {
    return TRUE;
  }

  GXLPVOID GPrimitiveVImpl::Lock(GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXDWORD dwFlags/* = (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)*/)
  {
    if(m_pVertMappedRes != NULL) {
      return m_pVertMappedRes->pData;
    }
    //if(m_pLockedVertex != NULL)
    //  return m_pLockedVertex;
    //if(IsDiscardable() != FALSE)
    //{
    //  m_pD3D11VertexBuffer->Lock(uElementOffsetToLock * m_uElementSize, 
    //    uElementOffsetToLock * m_uElementSize, &m_pLockedVertex, dwFlags);
    //  return m_pLockedVertex;
    //}
    //D3D11_MAPPED_SUBRESOURCE d11MS;
    m_pVertMappedRes = new D3D11_MAPPED_SUBRESOURCE;
    m_pGraphicsImpl->m_pImmediateContext->Map(m_pD3D11VertexBuffer, 0, D3D11_MAP_READ_WRITE, 0, m_pVertMappedRes);
    //m_pLockedVertex = 

    return m_pVertMappedRes->pData;
  }

  GXBOOL GPrimitiveVImpl::Unlock()
  {
    if(m_pVertMappedRes != NULL)
    {
      m_pGraphicsImpl->m_pImmediateContext->Unmap(m_pD3D11VertexBuffer, 0);
      delete m_pVertMappedRes;
      m_pVertMappedRes = NULL;
    }
    else {
      return FALSE;
    }

    return GX_OK;
  }

  GXLPVOID GPrimitiveVImpl::GetVerticesBuffer()
  {
    return m_pVertices;
  }

  GXUINT GPrimitiveVImpl::GetVerticesCount()
  {
    return m_uElementCount;
  }

  GXUINT GPrimitiveVImpl::GetVertexStride()
  {
    return m_uElementSize;
  }

  GXUINT GPrimitiveVImpl::GetIndicesCount()
  {
    return 0;
  }

  GXHRESULT GPrimitiveVImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    if(m_pVertexDecl == NULL)
      return GX_FAIL;
    m_pVertexDecl->AddRef();
    *ppDeclaration = m_pVertexDecl;
    return GX_OK;
  }

  GXGraphics* GPrimitiveVImpl::GetGraphicsUnsafe()
  {
    return static_cast<GXGraphics*>(m_pGraphicsImpl);
  }

  GXINT GPrimitiveVImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
  }

  GXBOOL GPrimitiveVImpl::UpdateResouce( ResEnum eRes )
  {
    CLBREAK;
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  GPrimitiveVIImpl::GPrimitiveVIImpl(GXGraphics* pGraphics)
    : GPrimitiveVI    ()
    , GPrimImpl       (pGraphics)
    , m_pD3D11IndexBuffer   (NULL)
    , m_uIndexCount    (0)
    , m_pIndexMappedRes(NULL)
    , m_pIndices       (NULL)
  {
  }

  GPrimitiveVIImpl::~GPrimitiveVIImpl()
  {
    INVOKE_LOST_DEVICE;
    SAFE_RELEASE(m_pD3D11IndexBuffer);
    SAFE_RELEASE(m_pD3D11VertexBuffer);

    SAFE_RELEASE(m_pVertexDecl);
    SAFE_DELETE_ARRAY(m_pIndices);
    SAFE_DELETE_ARRAY(m_pVertices);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GPrimitiveVIImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GPrimitiveVIImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    ASSERT((m_uRefCount & 0x80000000) == 0);
    if(nRefCount == 0)
    {
      m_pGraphicsImpl->UnregisterResource(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GPrimitiveVIImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      {
        SAFE_RELEASE(m_pD3D11IndexBuffer);
        SAFE_RELEASE(m_pD3D11VertexBuffer);
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

  GXBOOL GPrimitiveVIImpl::InitPrimitive(GXLPCVOID pVertInitData, 
    GXUINT uVertexCount, GXUINT uVertexSize, GXLPCVOID pIndexInitData, GXUINT uIndexCount, 
    LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage)
  {
    ASSERT(pIndexInitData == NULL && pVertInitData == NULL); // TODO: 稍后支持初始化数据
    if(uVertexCount == 0 || uVertexSize == 0 || uIndexCount == 0)
      return FALSE;

    m_uElementCount = uVertexCount;
    m_uElementSize  = uVertexSize;
    m_uIndexCount   = uIndexCount;
    m_dwResUsage    = ResUsage;

    // 创建顶点声明
    IntCreateVertexDeclaration(pVertexDecl);
    ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);

    ASSERT(m_pD3D11VertexBuffer == NULL && m_pD3D11IndexBuffer == NULL);

    HRESULT hr = IntCreateBuffer(&m_pD3D11VertexBuffer, m_uElementCount * m_uElementSize, D3D11_BIND_VERTEX_BUFFER, pVertInitData);
    if(FAILED(hr)) {
      return FALSE;
    }
    
    hr = IntCreateBuffer(&m_pD3D11IndexBuffer, m_uIndexCount * sizeof(VIndex), D3D11_BIND_INDEX_BUFFER, pIndexInitData);
    if(FAILED(hr)) {
      return FALSE;
    }
    

    // 这个应该在 Invoke 之后创建, 否则会有冗余的内存复制
    if( ! (TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD) || 
      TEST_FLAG(ResUsage, GXRU_FREQUENTLYWRITE)))
    {
      m_pIndices = new GXBYTE[m_uIndexCount * sizeof(GXWORD)];
      m_pVertices = new GXBYTE[m_uElementCount * m_uElementSize];
    }

    return TRUE;
  }

  GXBOOL GPrimitiveVIImpl::EnableDiscard(GXBOOL bDiscard)
  {
    return FALSE;
  }

  GXBOOL GPrimitiveVIImpl::IsDiscardable()
  {
    return TRUE;
  }

  GXBOOL GPrimitiveVIImpl::Lock(GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
    GXLPVOID* ppVertexData, GXWORD** ppIndexData, GXDWORD dwFlags/* = (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)*/)
  {
    if(ppVertexData == NULL || ppIndexData == NULL || m_pD3D11IndexBuffer == NULL)
      return FALSE;

    *ppIndexData = NULL;
    *ppVertexData = NULL;

    if(m_pVertMappedRes != NULL || m_pIndexMappedRes != NULL) {
      if(m_pVertMappedRes != NULL) {
        *ppVertexData = m_pVertMappedRes->pData;
      }
      if(m_pIndexMappedRes != NULL) {
        *ppIndexData = (GXWORD*)m_pIndexMappedRes->pData;
      }
      return TRUE;
    }
    D3D11_MAP eMap = GrapXToDX11::PrimitiveMapFromResUsage(m_dwResUsage);

    m_pVertMappedRes = new D3D11_MAPPED_SUBRESOURCE;
    if(m_pVertMappedRes != NULL) {
      m_pGraphicsImpl->m_pImmediateContext->Map(m_pD3D11VertexBuffer, 0, eMap, 0, m_pVertMappedRes);
      *ppVertexData = m_pVertMappedRes->pData;
    }

    m_pIndexMappedRes = new D3D11_MAPPED_SUBRESOURCE;
    if(m_pIndexMappedRes != NULL) {
      m_pGraphicsImpl->m_pImmediateContext->Map(m_pD3D11IndexBuffer, 0, eMap, 0, m_pIndexMappedRes);
      *ppIndexData = (GXWORD*)m_pIndexMappedRes->pData;
    }

    return TRUE;
  }


  GXBOOL GPrimitiveVIImpl::Unlock()
  {
    ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
    if(m_pVertMappedRes != NULL)
    {
      pImmediateContext->Unmap(m_pD3D11VertexBuffer, 0);
      delete m_pVertMappedRes;
      m_pVertMappedRes = NULL;
    }
    else {
      ASSERT(0);
      return FALSE;
    }

    if(m_pIndexMappedRes != NULL)
    {
      pImmediateContext->Unmap(m_pD3D11IndexBuffer, 0);
      delete m_pIndexMappedRes;
      m_pIndexMappedRes = NULL;
    }
    else {
      ASSERT(0);
      return FALSE;
    }
    //if(IsDiscardable() != FALSE)
    //{
    //  if(m_pLockedVertex != NULL)
    //  {
    //    if(m_pVertices != NULL) {
    //      // TODO: 按照锁定的区域复制
    //      memcpy(m_pVertices, m_pLockedVertex, m_uElementCount * m_uElementSize);
    //    }
    //    m_pD3D11VertexBuffer->Unlock();
    //    m_pLockedVertex = NULL;
    //  }
    //  if(m_pLockedIndex != NULL)
    //  {
    //    if(m_pIndices != NULL) {
    //      // TODO: 按照锁定的区域复制
    //      memcpy(m_pIndices, m_pLockedIndex, m_uIndexCount * sizeof(GXWORD));
    //    }
    //    m_pIndexBuffer->Unlock();
    //    m_pLockedIndex  = NULL;
    //  }
    //}
    return GX_OK;
  }

  GXLPVOID GPrimitiveVIImpl::GetVerticesBuffer()
  {
    return m_pVertices;
  }

  GXUINT GPrimitiveVIImpl::GetVerticesCount()
  {
    return m_uElementCount;
  }

  GXUINT GPrimitiveVIImpl::GetVertexStride()
  {
    return m_uElementSize;
  }

  GXLPVOID GPrimitiveVIImpl::GetIndicesBuffer()
  {
    return m_pIndices;
  }

  GXUINT GPrimitiveVIImpl::GetIndicesCount()
  {
    return m_uIndexCount;
  }

  GXHRESULT GPrimitiveVIImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    if(m_pVertexDecl == NULL)
      return GX_FAIL;
    m_pVertexDecl->AddRef();
    *ppDeclaration = m_pVertexDecl;
    return GX_OK;
  }
  GXGraphics* GPrimitiveVIImpl::GetGraphicsUnsafe()
  {
    return static_cast<GXGraphics*>(m_pGraphicsImpl);
  }

  GXINT GPrimitiveVIImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
  {
    return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
  }

  GXBOOL GPrimitiveVIImpl::UpdateResouce( ResEnum eRes )
  {
    CLBREAK;
    return TRUE;
  }
} // namespace D3D11
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)