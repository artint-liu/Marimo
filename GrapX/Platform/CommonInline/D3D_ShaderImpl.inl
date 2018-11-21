#ifdef REFACTOR_GRAPX_SHADER
GrapX::Graphics* GShaderImpl::GetGraphicsUnsafe() const
{
  return m_pGraphicsImpl;
}

GXLPCWSTR GShaderImpl::GetProfileDesc() const
{
  return m_strProfileDesc;
}
#endif // #ifdef REFACTOR_GRAPX_SHADER
//////////////////////////////////////////////////////////////////////////
