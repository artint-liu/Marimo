#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include <GrapX.H>
#include "User/GrapX.Hxx"

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GPrimitive.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GShader.H"
#include "GrapX/GXKernel.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.H"

// 私有头文件
#include "GPrimitiveImpl_d3d9.h"
//#define _GXGRAPHICS_INLINE_PRIMITIVE_D3D9_
#include "Canvas/GXResourceMgr.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"

namespace D3D9
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
  
  GPrimImpl::GPrimImpl(GXGraphics* pGraphics)
    : m_pGraphicsImpl   ((GXGraphicsImpl*)pGraphics)
    , m_pVertexBuffer   (NULL)
    , m_uElementSize    (0)
    , m_uElementCount   (0)
    , m_pLockedVertex   (NULL)
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

  GXBOOL GPrimImpl::RestoreVertices()
  {
    if(m_pVertices != NULL) {
      GXLPVOID lpLocked = NULL;\
      m_pVertexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD|D3DLOCK_DONOTWAIT);
      memcpy(lpLocked, m_pVertices, m_uElementCount * m_uElementSize);
      m_pVertexBuffer->Unlock();
      return TRUE;
    }
    return FALSE;
  }

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

  GXHRESULT GPrimitiveVImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD dwUsage;
    D3DPOOL ePool;
    ConvertPrimResUsageToNative(m_dwResUsage, &dwUsage, &ePool);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      {
        SAFE_RELEASE(m_pVertexBuffer);
        return GX_OK;
      }
      break;
    case RC_ResetDevice:
      {
        LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

        if(GXSUCCEEDED(lpd3dDevice->CreateVertexBuffer(
          m_uElementCount * m_uElementSize, dwUsage, NULL, ePool, &m_pVertexBuffer, NULL)))
        {
          RestoreVertices();
          return GX_OK;
        }
      }
      break;
    case RC_ResizeDevice:
      break;
    }
    return GX_FAIL;
  }

  GXBOOL GPrimitiveVImpl::InitPrimitive(GXLPCVOID pVertInitData, GXUINT uElementCount, GXUINT uElementSize, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage)
  {
    m_uElementCount = uElementCount;
    m_uElementSize  = uElementSize;
    m_dwResUsage    = ResUsage;

    // 创建顶点声明
    CreateVertexDeclaration(pVertexDecl);
    ASSERT(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM) == 0);
    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_ResetDevice;

    //if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    if(Invoke(&Desc) == GX_OK)
    {
      // 这个应该在 Invoke ResetDevice 之后创建, 否则会有冗余的内存复制
      if( ! (TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD) || 
        TEST_FLAG(ResUsage, GXRU_FREQUENTLYWRITE)))
      {
        m_pVertices = new GXBYTE[m_uElementCount * m_uElementSize];
      }

      // Update Init Data
      if(pVertInitData != NULL)
      {
        GXLPVOID pLockedVert = Lock(0, 0);
        if(pLockedVert != NULL)
        {
          memcpy(pLockedVert, pVertInitData, uElementCount * uElementSize);
          Unlock();
        }
        else{
          ASSERT(0);
        }
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
    if(m_pLockedVertex != NULL)
      return m_pLockedVertex;
    if(IsDiscardable() != FALSE)
    {
      m_pVertexBuffer->Lock(uElementOffsetToLock * m_uElementSize, 
        uElementOffsetToLock * m_uElementSize, &m_pLockedVertex, dwFlags);
      return m_pLockedVertex;
    }
    return NULL;
  }

  GXBOOL GPrimitiveVImpl::Unlock()
  {
    if(IsDiscardable() != FALSE)
    {
      if(m_pVertices != NULL) {
        // TODO: 按照锁定的区域复制
        memcpy(m_pVertices, m_pLockedVertex, m_uElementCount * m_uElementSize);
      }

      m_pVertexBuffer->Unlock();
      m_pLockedVertex = NULL;
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
    return InlGetSafeObjectT<GVertexDeclaration>(ppDeclaration, m_pVertexDecl);
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
    if(eRes == GPrimitive::ResourceAll || eRes == GPrimitive::ResourceVertices) {
      return RestoreVertices();
    }
    return FALSE;
  }
  
  //////////////////////////////////////////////////////////////////////////
  GPrimitiveVIImpl::GPrimitiveVIImpl(GXGraphics* pGraphics)
    : GPrimitiveVI    ()
    , GPrimImpl       (pGraphics)
    , m_pIndexBuffer   (NULL)
    , m_uIndexCount    (0)
    , m_pLockedIndex   (NULL)
    , m_pIndices       (NULL)
  {
  }

  GPrimitiveVIImpl::~GPrimitiveVIImpl()
  {
    INVOKE_LOST_DEVICE;
    SAFE_RELEASE(m_pVertexDecl);
    SAFE_DELETE_ARRAY(m_pIndices);
    SAFE_DELETE_ARRAY(m_pVertices);
    m_pGraphicsImpl->UnregisterResource(this);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GPrimitiveVIImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GPrimitiveVIImpl::Release()
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
        LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

        DWORD dwUsage;
        D3DPOOL ePool;
        ConvertPrimResUsageToNative(m_dwResUsage, &dwUsage, &ePool);

        if(GXSUCCEEDED(lpd3dDevice->CreateVertexBuffer(
          m_uElementCount * m_uElementSize, dwUsage, NULL, ePool, &m_pVertexBuffer, NULL)) &&
          GXSUCCEEDED(lpd3dDevice->CreateIndexBuffer(
          m_uIndexCount * sizeof(GXWORD), dwUsage, D3DFMT_INDEX16, ePool, &m_pIndexBuffer, NULL)) ) {

            RestoreVertices();
            RestoreIndices();

            return GX_OK;
        }
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


    //if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    if(Invoke(&Desc) == GX_OK)
    {
      // 这个应该在 Invok ResetDevice 之后创建, 否则会有冗余的内存复制
      if( ! (TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD) || 
        TEST_FLAG(ResUsage, GXRU_FREQUENTLYWRITE)))
      {
        m_pIndices = new GXBYTE[m_uIndexCount * sizeof(GXWORD)];
        m_pVertices = new GXBYTE[m_uElementCount * m_uElementSize];
      }
      
      // Update Init Data
      if(pVertInitData != NULL && pIndexInitData != NULL)
      {
        GXLPVOID pLockedVert = NULL;
        GXWORD* pLockedIdx = NULL;

        memcpy(m_pVertices, pVertInitData, m_uElementCount * m_uElementSize);
        memcpy(m_pIndices, pIndexInitData, m_uIndexCount * sizeof(GXWORD));

        if(Lock(0, 0, 0, 0, &pLockedVert, &pLockedIdx))
        {
          memcpy(pLockedVert, pVertInitData, uVertexCount * uVertexSize);
          memcpy(pLockedIdx, pIndexInitData, uIndexCount * sizeof(GXWORD));
          Unlock();
        }
        else{
          ASSERT(0);
        }
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

    if(IsDiscardable() != FALSE)
    {
      if(m_pLockedVertex == NULL)
        m_pVertexBuffer->Lock(uElementOffsetToLock * m_uElementSize,
        uElementCountToLock * m_uElementSize, &m_pLockedVertex, dwFlags);

      if(m_pLockedIndex == NULL)
        m_pIndexBuffer->Lock(uIndexOffsetToLock * sizeof(GXWORD), 
        uIndexLengthToLock * sizeof(GXWORD), (void**)&m_pLockedIndex, dwFlags);
      *ppVertexData = m_pLockedVertex;
      *ppIndexData  = m_pLockedIndex;

      return ((*ppIndexData) != NULL && (*ppVertexData) != NULL);
    }
    return FALSE;
  }


  GXBOOL GPrimitiveVIImpl::Unlock()
  {
    if(IsDiscardable() != FALSE)
    {
      if(m_pLockedVertex != NULL)
      {
        if(m_pVertices != NULL) {
          // TODO: 按照锁定的区域复制
          memcpy(m_pVertices, m_pLockedVertex, m_uElementCount * m_uElementSize);
        }
        m_pVertexBuffer->Unlock();
        m_pLockedVertex = NULL;
      }
      if(m_pLockedIndex != NULL)
      {
        if(m_pIndices != NULL) {
          // TODO: 按照锁定的区域复制
          memcpy(m_pIndices, m_pLockedIndex, m_uIndexCount * sizeof(GXWORD));
        }
        m_pIndexBuffer->Unlock();
        m_pLockedIndex  = NULL;
      }
    }
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
    //if(m_pVertexDecl == NULL)
    //  return GX_FAIL;
    //m_pVertexDecl->AddRef();
    //*ppDeclaration = m_pVertexDecl;
    //return GX_OK;
    return InlGetSafeObjectT<GVertexDeclaration>(ppDeclaration, m_pVertexDecl);
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
    switch(eRes)
    {
    case GPrimitive::ResourceIndices:
      return RestoreIndices();
    case GPrimitive::ResourceVertices:
      return RestoreVertices();
    case GPrimitive::ResourceAll:
      {
        const GXBOOL bval = RestoreIndices();
        return (RestoreVertices() && bval);
      }
    }
    return FALSE;
  }

  GXBOOL GPrimitiveVIImpl::RestoreIndices()
  {
    if(m_pIndices != NULL) {
      GXLPVOID lpLocked = NULL;
      m_pIndexBuffer->Lock(0, 0, &lpLocked, D3DLOCK_DISCARD|D3DLOCK_DONOTWAIT);
      memcpy(lpLocked, m_pIndices, m_uIndexCount * sizeof(GXWORD));
      m_pIndexBuffer->Unlock();
      return TRUE;
    }
    return FALSE;
  }
}
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)