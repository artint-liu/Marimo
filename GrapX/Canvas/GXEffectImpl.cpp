#include "GrapX.h"
#include "GrapX/GResource.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXKernel.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GXEffectImpl.h"

//DATALAYOUT g_CanvasCommon[] =
//{
//  {"Color", MEMBER_OFFSET(GXCANVASCOMMCONST, colorMul), GXUB_FLOAT4, sizeof(float4)},
//  {"ColorAdd", MEMBER_OFFSET(GXCANVASCOMMCONST, colorAdd), GXUB_FLOAT4, sizeof(float4)},
//  {"matWVProj", MEMBER_OFFSET(GXCANVASCOMMCONST, matWVProj), GXUB_MATRIX4, sizeof(float4x4)},
//  {NULL},
//};
namespace GrapX
{
  EffectImpl::EffectImpl(Graphics* pGraphics, Shader* pShader)
    : m_pGraphics(pGraphics)
    , m_pShader(pShader)
    , m_pDataPool(NULL)
  {
    if(pShader) {
      pShader->AddRef();
    }
  }

  EffectImpl::~EffectImpl()
  {
    SAFE_RELEASE(m_pShader);
    SAFE_RELEASE(m_pDataPool);
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT EffectImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT EffectImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);

    if(nRefCount == 0)
    {
      //OnDeviceEvent(DE_LostDevice);

      if(m_pDataPool)
      {
        m_pGraphics->UnregisterResource(this);
      }
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT EffectImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    return GX_OK;
  }

  GXBOOL EffectImpl::SetTexture(GXUINT nSlot, Texture* pTexture)
  {
    if(nSlot >= m_aTextures.size()) {
      return FALSE;
    }
    m_aTextures[nSlot] = pTexture;
    return TRUE;
  }

  GXBOOL EffectImpl::SetTexture(GXLPCSTR szSamplerName, Texture* pTexture)
  {
    const Shader::BINDRESOURCE_DESC* pDesc = m_pShader->FindBindResource(szSamplerName);
    if(pDesc && pDesc->type == Shader::BindType::Sampler)
    {
      ASSERT(pDesc->slot < (int)m_aTextures.size());
      m_aTextures[pDesc->slot] = pTexture;
      return TRUE;
    }
    return FALSE;
  }

  void EffectImpl::BindTextureSlot(GXLPCSTR szTextureName, int nSlot)
  {
  }

  Graphics* EffectImpl::GetGraphicsUnsafe() const
  {
    return m_pGraphics;
  }


  Shader* EffectImpl::GetShaderUnsafe() const
  {
    return m_pShader;
  }

  Marimo::DataPool* EffectImpl::GetDataPoolUnsafe() const
  {
    return m_pDataPool;
  }

  Marimo::DataPoolVariable EffectImpl::GetUniform(GXLPCSTR szName)
  {
    Marimo::DataPoolVariable var;
    m_pDataPool->QueryByName(szName, &var);
    return var;
  }

  GXHRESULT EffectImpl::Clone(Effect** ppNewEffect)
  {
    return m_pGraphics->CreateEffect(ppNewEffect, m_pShader);
  }

  GXBOOL EffectImpl::InitEffect()
  {
    Marimo::DATAPOOL_MANIFEST manifest;
    m_pShader->GetDataPoolDeclaration(&manifest);
    GXHRESULT hr = Marimo::DataPool::CreateDataPool(&m_pDataPool, NULL, manifest.pTypes, manifest.pVariables, Marimo::DataPoolCreation_NotCross16BytesBoundary);

    GXINT nMaxSlot = 0;
    for(GXUINT n = 0;; n++)
    {
      const Shader::BINDRESOURCE_DESC* pDesc = m_pShader->GetBindResource(n);
      if(pDesc == NULL) {
        break;
      }
      else if(pDesc->type == Shader::BindType::Sampler) {
        nMaxSlot = clMax(nMaxSlot, pDesc->slot + 1);
      }
    }

    if(nMaxSlot) {
      m_aTextures.assign(nMaxSlot, NULL);
    }

    return GXSUCCEEDED(hr);
  }

  GXBOOL EffectImpl::Commit()
  {
    GXUINT slot = 0;
    for(auto it = m_aTextures.begin(); it != m_aTextures.end(); ++it, slot++)
    {
      m_pGraphics->SetTexture(*it, slot);
    }
    return TRUE;
  }

  //GXBOOL EffectImpl::CommitUniform()
  //{
  //  return m_pShader->CommitConstantBuffer(m_pDataPool);
  //}

  GXUINT EffectImpl::GetHandle(const GXCHAR* pName) const
  {
    CLBREAK;
    return 0;
  }
} // namespace GrapX
//////////////////////////////////////////////////////////////////////////
