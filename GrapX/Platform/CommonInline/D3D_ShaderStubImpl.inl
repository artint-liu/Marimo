﻿//////////////////////////////////////////////////////////////////////////
#ifdef REFACTOR_GRAPX_SHADER
GShader* GShaderStubImpl::GetShaderUnsafe() const
{
  return m_pShaderImpl;
}

GXUINT GShaderStubImpl::GetHandleByName(const GXCHAR* pName) const
{
  if(pName == NULL || m_pShaderImpl == NULL)
  {
    ASSERT(0);
    return -1;
  }
  return m_pShaderImpl->GetHandle(pName);
}

GXUINT GShaderStubImpl::GetHandleByIndex(GXUINT nIndex) const
{
  ASSERT(m_aCommonUniforms.size() == m_pShaderImpl->GetConstantDescTable().size());
  if(nIndex >= m_aCommonUniforms.size()) {
    return 0;
  }
  return m_aCommonUniforms[nIndex].pConstDesc->dwHandle;
}

GXUniformType GShaderStubImpl::GetHandleType(GXUINT handle) const
{
  return m_pShaderImpl->GetHandleType(handle);
}

GXUINT GShaderStubImpl::GetSamplerStageByHandle(GXUINT handle) const
{
  return m_pShaderImpl->GetStageByHandle(handle);
}

GXBOOL GShaderStubImpl::SetUniformByHandle(clBufferBase* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount)
{
  // 应该验证是非绑定变量
  return IntSetUniform(&(m_pShaderImpl->GetConstantDescTable().front()),
    uHandle, fValue, nFloatCount, pUnusualUnifom ? (float4*)pUnusualUnifom->GetPtr() : NULL);
}

#ifdef REFACTOR_SHADER
GXBOOL GShaderStubImpl::CommitToDevice(GXLPVOID lpUniform, GXSIZE_T cbSize)
{
  return m_pShaderImpl->CommitToDevice(lpUniform, cbSize);
}
#endif // #ifdef REFACTOR_SHADER
#endif // #ifdef REFACTOR_GRAPX_SHADER