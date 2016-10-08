//GXSAMPLERSTAGE GSamplerStateImpl::s_DefaultSamplerState;

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GSamplerStateImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GSamplerStateImpl::Release()
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


GXBOOL GSamplerStateImpl::InitializeStatic()
{
  //IntSetSamplerToDefault(&s_DefaultSamplerState);
  return TRUE;
}



//GXHRESULT GSamplerStateImpl::SetState(int nSampler, GXSAMPLERDESC* pSamplerDesc)
//{
//  return GX_OK;
//}
