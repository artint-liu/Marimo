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
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"

#ifdef ENABLE_GRAPHICS_API_DX11
namespace D3D11
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  GPrimImpl::GPrimImpl(GXGraphics* pGraphics)
    : m_pGraphicsImpl   ((GXGraphicsImpl*)pGraphics)
    , m_pVertexBuffer   (NULL)
    , m_uElementSize    (0)
    , m_uElementCount   (0)
    , m_pVertMappedRes  (NULL)
    //, m_pLockedVertex   (NULL)
    , m_dwResUsage      (NULL)
    , m_pVertexDecl     (NULL)
    , m_pVertices       (NULL)
  {
  }

  GXBOOL GPrimImpl::CreateVertexDeclaration(LPCGXVERTEXELEMENT pVertexDecl)
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
    //else if(m_uRefCount == 1)
    //{
    //  return m_pGraphicsImpl->OldUnregisterResource(this);
    //}
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
      //SAFE_RELEASE(m_pIndexBuffer);
      SAFE_RELEASE(m_pVertexBuffer);
      return GX_OK;
    }
    break;
    case RC_ResetDevice:
    {
      ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      HRESULT hr = E_FAIL;

      D3D11_BUFFER_DESC bd;
      D3D11_SUBRESOURCE_DATA InitData;

      InlSetZeroT(bd);
      GrapXToDX11::PrimitiveDescFromResUsage(m_dwResUsage, &bd);

      bd.ByteWidth = m_uElementCount * m_uElementSize;
      bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

      InlSetZeroT(InitData);
      InitData.pSysMem = (const void *)pDesc->lParam;

      hr = pd3dDevice->CreateBuffer(&bd, m_pVertices == NULL ? NULL : &InitData, &m_pVertexBuffer);
      if(FAILED(hr))
        return hr;

      //bd.ByteWidth = m_uIndexCount * sizeof(GXWORD);
      //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

      //InlSetZeroT(InitData);
      //InitData.pSysMem = m_pIndices;

      //hr = pd3dDevice->CreateBuffer(&bd, m_pIndices == NULL ? NULL : &InitData, &m_pIndexBuffer);
      if(FAILED(hr))
        return hr;

      return S_OK;
      //    DWORD dwUsage;
      //    D3DPOOL ePool;
      //    ConvertPrimResUsageToNative(m_dwResUsage, &dwUsage, &ePool);

      //    if(GXSUCCEEDED(lpd3dDevice->CreateVertexBuffer(
      //      m_uElementCount * m_uElementSize, dwUsage, NULL, ePool, &m_pVertexBuffer, NULL)) &&
      //      GXSUCCEEDED(lpd3dDevice->CreateIndexBuffer(
      //      m_uIndexCount * sizeof(GXWORD), dwUsage, D3DFMT_INDEX16, ePool, &m_pIndexBuffer, NULL)) ) {

      //        RestoreVertices();

      //        if(m_pIndices != NULL) {
      //          GXLPVOID lpLocked = NULL;
      //          m_pIndexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD|D3DLOCK_DONOTWAIT);
      //          memcpy(lpLocked, m_pIndices, m_uIndexCount * sizeof(GXWORD));
      //          m_pIndexBuffer->Unlock();
      //        }

      //        return GX_OK;
      //    }
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
    CreateVertexDeclaration(pVertexDecl);
    ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);

    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;
    Desc.lParam = (GXLPARAM)pVertInitData;

    if(Invoke(&Desc) == GX_OK)
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
    //  m_pVertexBuffer->Lock(uElementOffsetToLock * m_uElementSize, 
    //    uElementOffsetToLock * m_uElementSize, &m_pLockedVertex, dwFlags);
    //  return m_pLockedVertex;
    //}
    //D3D11_MAPPED_SUBRESOURCE d11MS;
    m_pVertMappedRes = new D3D11_MAPPED_SUBRESOURCE;
    m_pGraphicsImpl->m_pImmediateContext->Map(m_pVertexBuffer, 0, D3D11_MAP_READ_WRITE, 0, m_pVertMappedRes);
    //m_pLockedVertex = 

    return m_pVertMappedRes->pData;
  }

  GXBOOL GPrimitiveVImpl::Unlock()
  {
    //if(IsDiscardable() != FALSE)
    //{
    //  if(m_pVertices != NULL) {
    //    // TODO: 按照锁定的区域复制
    //    memcpy(m_pVertices, m_pLockedVertex, m_uElementCount * m_uElementSize);
    //  }

    //  m_pVertexBuffer->Unlock();
    //  m_pLockedVertex = NULL;
    //}
    if(m_pVertMappedRes != NULL)
    {
      m_pGraphicsImpl->m_pImmediateContext->Unmap(m_pVertexBuffer, 0);
      delete m_pVertMappedRes;
      m_pVertMappedRes = NULL;
    }
    else {
      return FALSE;
    }

    return GX_OK;
  }
  //GPrimitive::Type GPrimitiveVImpl::GetType()
  //{
  //  return VertexOnly;
  //}

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
    , m_pIndexBuffer   (NULL)
    , m_uIndexCount    (0)
    //, m_pLockedIndex   (NULL)
    , m_pIndexMappedRes(NULL)
    , m_pIndices       (NULL)
  {
  }

  GPrimitiveVIImpl::~GPrimitiveVIImpl()
  {
    //OnDeviceEvent(DE_LostDevice);
    INVOKE_LOST_DEVICE;
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pVertexBuffer);

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
        SAFE_RELEASE(m_pIndexBuffer);
        SAFE_RELEASE(m_pVertexBuffer);
        return GX_OK;
      }
      break;
    case RC_ResetDevice:
      {
        ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
        HRESULT hr = E_FAIL;

        D3D11_BUFFER_DESC bd;
        D3D11_SUBRESOURCE_DATA InitData;

        InlSetZeroT(bd);
        GrapXToDX11::PrimitiveDescFromResUsage(m_dwResUsage, &bd);

        bd.ByteWidth      = m_uElementCount * m_uElementSize;
        bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;

        InlSetZeroT(InitData);
        InitData.pSysMem = m_pVertices;

        hr = pd3dDevice->CreateBuffer(&bd, m_pVertices == NULL ? NULL : &InitData, &m_pVertexBuffer);
        if( FAILED( hr ) )
          return hr;

        bd.ByteWidth      = m_uIndexCount * sizeof(GXWORD);
        bd.BindFlags      = D3D11_BIND_INDEX_BUFFER;

        InlSetZeroT(InitData);
        InitData.pSysMem = m_pIndices;

        hr = pd3dDevice->CreateBuffer(&bd, m_pIndices == NULL ? NULL : &InitData, &m_pIndexBuffer);
        if( FAILED( hr ) )
          return hr;

        return S_OK;
        //    DWORD dwUsage;
        //    D3DPOOL ePool;
        //    ConvertPrimResUsageToNative(m_dwResUsage, &dwUsage, &ePool);

        //    if(GXSUCCEEDED(lpd3dDevice->CreateVertexBuffer(
        //      m_uElementCount * m_uElementSize, dwUsage, NULL, ePool, &m_pVertexBuffer, NULL)) &&
        //      GXSUCCEEDED(lpd3dDevice->CreateIndexBuffer(
        //      m_uIndexCount * sizeof(GXWORD), dwUsage, D3DFMT_INDEX16, ePool, &m_pIndexBuffer, NULL)) ) {

        //        RestoreVertices();

        //        if(m_pIndices != NULL) {
        //          GXLPVOID lpLocked = NULL;
        //          m_pIndexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD|D3DLOCK_DONOTWAIT);
        //          memcpy(lpLocked, m_pIndices, m_uIndexCount * sizeof(GXWORD));
        //          m_pIndexBuffer->Unlock();
        //        }

        //        return GX_OK;
        //    }
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
    CreateVertexDeclaration(pVertexDecl);
    ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);

    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;
    GXHRESULT hval = Invoke(&Desc);

    if(GXSUCCEEDED(hval))
    {
      // 这个应该在 Invoke 之后创建, 否则会有冗余的内存复制
      if( ! (TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD) || 
        TEST_FLAG(ResUsage, GXRU_FREQUENTLYWRITE)))
      {
        m_pIndices = new GXBYTE[m_uIndexCount * sizeof(GXWORD)];
        m_pVertices = new GXBYTE[m_uElementCount * m_uElementSize];
      }

      return TRUE;
    }

    // 有可能其中一个创建成功了.
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pVertexBuffer);
    return FALSE;
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
    if(ppVertexData == NULL || ppIndexData == NULL || m_pIndexBuffer == NULL)
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
      m_pGraphicsImpl->m_pImmediateContext->Map(m_pVertexBuffer, 0, eMap, 0, m_pVertMappedRes);
      *ppVertexData = m_pVertMappedRes->pData;
    }

    m_pIndexMappedRes = new D3D11_MAPPED_SUBRESOURCE;
    if(m_pIndexMappedRes != NULL) {
      m_pGraphicsImpl->m_pImmediateContext->Map(m_pIndexBuffer, 0, eMap, 0, m_pIndexMappedRes);
      *ppIndexData = (GXWORD*)m_pIndexMappedRes->pData;
    }

    return TRUE;

    //if(IsDiscardable() != FALSE)
    //{
    //  if(m_pLockedVertex == NULL)
    //    m_pVertexBuffer->Lock(uElementOffsetToLock * m_uElementSize,
    //    uElementCountToLock * m_uElementSize, &m_pLockedVertex, dwFlags);

    //  if(m_pLockedIndex == NULL)
    //    m_pIndexBuffer->Lock(uIndexOffsetToLock * sizeof(GXWORD), 
    //    uIndexLengthToLock * sizeof(GXWORD), (void**)&m_pLockedIndex, dwFlags);
    //  *ppVertexData = m_pLockedVertex;
    //  *ppIndexData  = m_pLockedIndex;

    //  return ((*ppIndexData) != NULL && (*ppVertexData) != NULL);
    //}
    //return FALSE;
  }


  GXBOOL GPrimitiveVIImpl::Unlock()
  {
    ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
    if(m_pVertMappedRes != NULL)
    {
      pImmediateContext->Unmap(m_pVertexBuffer, 0);
      delete m_pVertMappedRes;
      m_pVertMappedRes = NULL;
    }
    else {
      ASSERT(0);
      return FALSE;
    }

    if(m_pIndexMappedRes != NULL)
    {
      pImmediateContext->Unmap(m_pIndexBuffer, 0);
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
    //    m_pVertexBuffer->Unlock();
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
  //GPrimitive::Type GPrimitiveVIImpl::GetType()
  //{
  //  return Indexed;
  //}

  GXUINT GPrimitiveVIImpl::GetIndexCount()
  {
    return m_uIndexCount;
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