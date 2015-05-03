#include "GrapX.H"
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GShader.H"
#include "Include/GTexture.H"
#include "Include/GXCanvas.H"
#include "Include/GXGraphics.H"
#include "Include/GXKernel.H"
#include "GXEffectImpl.h"

DATALAYOUT g_CanvasCommon[] =
{
  {"Color", MEMBER_OFFSET(GXCANVASCOMMCONST, colorMul), GXUB_FLOAT4, sizeof(float4)},
  {"ColorAdd", MEMBER_OFFSET(GXCANVASCOMMCONST, colorAdd), GXUB_FLOAT4, sizeof(float4)},
  {"matWVProj", MEMBER_OFFSET(GXCANVASCOMMCONST, matWVProj), GXUB_MATRIX4, sizeof(float4x4)},
  {NULL},
};

GXEffectImpl::GXEffectImpl(GXGraphics* pGraphics)
  : GXEffect()
  , m_pGraphics(pGraphics)
  , m_pShaderStub(NULL)
{
}

GXEffectImpl::~GXEffectImpl()
{
  SAFE_RELEASE(m_pShaderStub);
  m_pGraphics->UnregisterResource(this);
}


GXHRESULT GXEffectImpl::SetShaderRef(GShader* pShader)
{
  if(m_pShaderStub == NULL)
  {
    if(GXFAILED(m_pGraphics->CreateShaderStub(&m_pShaderStub)))
    {
      ASSERT(0);
    }
  }
  m_pShaderStub->SetShaderRef(pShader);
  m_pShaderStub->BindCommonUniform(g_CanvasCommon);
  return GX_OK;
}

bool GXEffectImpl::CommitUniform(GXCanvas* pCanvas, GXUINT uCommonOffset)
{
  CANVASUNIFORM CanUni;
  pCanvas->GetUniformData(&CanUni);
  clBuffer& UniformBuffer = *CanUni.pUnusualBuf;
  float4* pConstBuffer = (float4*)UniformBuffer.GetPtr();
  m_pShaderStub->CommitUniform(-1, pConstBuffer, uCommonOffset);

  GXCONST GXCANVASCOMMCONST& CommConst = CanUni.pCommon;
  m_pShaderStub->CommitUniform(0, (GXLPCVOID)&CommConst, uCommonOffset);

  return true;
}

GShader* GXEffectImpl::GetShaderUnsafe()  GXCONST
{
  return InlGetShaderUnsafe();
}

GShaderStub* GXEffectImpl::GetShaderStubUnsafe() GXCONST
{
  return m_pShaderStub;
}

GXUINT GXEffectImpl::GetHandle(GXCONST GXCHAR* pName) GXCONST
{
  if(pName == NULL || m_pShaderStub == NULL)
  {
    ASSERT(0);
    return -1;
  }
  return m_pShaderStub->GetHandleByName(pName);
}

bool GXEffectImpl::SetUniformByHandle(GXCanvas* pCanvas, GXUINT uHandle, float* fValue, GXINT nFloatCount)
{
  CANVASUNIFORM canuni;
  pCanvas->GetUniformData(&canuni);
  m_pShaderStub->SetUniformByHandle(canuni.pUnusualBuf, uHandle, fValue, nFloatCount);
  return true;
}

GXHRESULT GXEffectImpl::Invoke(GRESCRIPTDESC* pDesc)
{
  INVOKE_DESC_CHECK(pDesc);
  if(pDesc->szCmdString != NULL)
  {
    if(clstd::strcmpT(pDesc->szCmdString, "reloadshader") == 0)
    {
      SetShaderRef(NULL);
    }
  }
  return GX_OK;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXEffectImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GXEffectImpl::Release()
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

GXHRESULT GXEffectImpl::SetTextureSlot(GXLPCSTR pName, GXINT nSlot)
{
  return GX_OK;
}

GXINT GXEffectImpl::GetTextureSlot(GXLPCSTR pName)
{
  return -1;
}

GXUINT GXEffectImpl::GetConstantBufferSize()
{
  return m_pShaderStub->GetShaderUnsafe()->GetCacheSize();
}

//////////////////////////////////////////////////////////////////////////
