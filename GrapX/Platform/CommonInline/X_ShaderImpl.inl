const int c_D3D_INCLUDE_LOCAL  = 0;
const int c_D3D_INCLUDE_SYSTEM = 1;

template<class _ID3DIncludeT, typename _IncludeTypeT>
class IHLSLIncludeT : public _ID3DIncludeT
{
private:
  typedef clhash_map<clStringA, clBuffer*> IncDict;
  Graphics* m_pGraphics;
  clStringA   m_strBaseDir;
  IncDict     m_IncFiles;

public:
  IHLSLIncludeT(Graphics* pGraphics, GXLPCSTR szBaseDir)
    : m_pGraphics(pGraphics)
    , m_strBaseDir(szBaseDir != NULL ? szBaseDir : "")
  {
  }

  virtual ~IHLSLIncludeT()
  {
    for(IncDict::iterator it = m_IncFiles.begin();
      it != m_IncFiles.end(); ++it)
    {
      clBuffer* pBuffer = it->second;
      delete pBuffer;
    }
    m_IncFiles.clear();
  }

  STDMETHOD(Open)(_IncludeTypeT IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
  {
    if(IncludeType == c_D3D_INCLUDE_LOCAL || IncludeType == c_D3D_INCLUDE_SYSTEM)
    {
      clFile file;
      clStringA strFullPath;

      if(clpathfile::IsRelative(pFileName))
      {
        if(m_pGraphics != NULL) {
          strFullPath = pFileName;
          m_pGraphics->ConvertToAbsolutePathA(strFullPath);
        }
        else if(m_strBaseDir.IsNotEmpty()) {
          clpathfile::CombinePath(strFullPath, m_strBaseDir, pFileName);
        }
        else {
          strFullPath = pFileName;
        }
      }
      else {
        strFullPath = pFileName;
      }

      // 查找 Include 是否已经存在
      IncDict::iterator it = m_IncFiles.find(strFullPath);
      if(it != m_IncFiles.end()) {
        *ppData = it->second->GetPtr();
        *pBytes = (UINT)it->second->GetSize();
        return S_OK;
      }

      // 打开并映射文件
      if(file.OpenExisting(strFullPath))
      {
        clBuffer* pBuffer = NULL;
        if(file.GetSize(NULL) == 0)
        {
          pBuffer = new clBuffer(128);
        }
        else if( ! file.MapToBuffer(&pBuffer))
        {
          return E_FAIL;
        }

        clStringA strLineInfo;
        strLineInfo.Format("#line 1 \"%s\"\r\n", strFullPath);
        pBuffer->Insert(0, strLineInfo.GetBuffer(), strLineInfo.GetLength());

        *ppData = pBuffer->GetPtr();
        *pBytes = (UINT)pBuffer->GetSize();
        m_IncFiles[strFullPath] = pBuffer;
        return S_OK;
      }
    }
    return E_FAIL;
  }

  STDMETHOD(Close)(LPCVOID pData)
  {
    return S_OK;
  }
};

#ifdef _WIN32_DIRECT3D_9_H_
typedef IHLSLIncludeT<ID3DXInclude, D3DXINCLUDE_TYPE> IHLSLInclude;
#elif defined(_WIN32_DIRECT3D_11_H_)
typedef IHLSLIncludeT<ID3D10Include, D3D_INCLUDE_TYPE> IHLSLInclude;
#endif // #ifdef _WIN32_DIRECT3D_9_H_

#ifdef REFACTOR_GRAPX_SHADER
GXHRESULT GShaderImpl::LoadFromFile(MOSHADER_ELEMENT_SOURCE* pSdrElementSrc)
{
  GXDefinitionArray aMacros;
  MOSHADERBUFFERS ShaderBuffers;
  ShaderBuffers.pVertexShader = NULL;
  ShaderBuffers.pPixelShader  = NULL;
  GXDEFINITION* pShaderMacro = NULL;
  IHLSLInclude* pInclude = NULL;
  GXHRESULT hr = GX_OK;

  GXPlaformIdentity pltid;
  m_pGraphicsImpl->GetPlatformID(&pltid);
  ASSERT(pltid == GXPLATFORM_WIN32_DIRECT3D9 || pltid == GXPLATFORM_WIN32_DIRECT3D11);

  if(GShader::ComposeSource(pSdrElementSrc, pltid, &ShaderBuffers, &aMacros))
  {
    clBuffer* const pVertexBuffer = ShaderBuffers.pVertexShader;
    clBuffer* const pPixelBuffer  = ShaderBuffers.pPixelShader;
    const GXBOOL bCompiledVS = ((GXLPBYTE)pVertexBuffer->GetPtr())[0] == '\0';
    const GXBOOL bCompiledPS = ((GXLPBYTE)pPixelBuffer->GetPtr() )[0] == '\0';


    if( ! (bCompiledPS && bCompiledVS))
    {
      if( ! aMacros.empty())
      {
        int i = 0;
        pShaderMacro = new GXDEFINITION[aMacros.size() + 1];

        // 这个是为了把宏数组的结尾填为 NULL, clString 无法做到这一点.
        for(GXDefinitionArray::iterator it = aMacros.begin();
          it != aMacros.end(); ++it, ++i) {
            pShaderMacro[i].szName = it->Name;
            pShaderMacro[i].szValue = it->Value;
        }
        pShaderMacro[i].szName = NULL;
        pShaderMacro[i].szValue = NULL;
      }

      pInclude = new IHLSLInclude(m_pGraphicsImpl, NULL);
    }

    // 注意: 输入的pVertexBuffer和pPixelBuffer会改变
    if( ! bCompiledVS)
    {
      const GXBOOL bComposing = pSdrElementSrc->strVSComposer.IsNotEmpty();
      hr = CompileShader(pVertexBuffer, pInclude, pShaderMacro, bComposing ? CompiledComponentVertexShder : CompiledVertexShder);
      if(GXFAILED(hr)) {
        goto FUNC_RET;
      }
    }

    if( ! bCompiledPS)
    {
      const GXBOOL bComposing = pSdrElementSrc->strPSComposer.IsNotEmpty();
      hr = CompileShader(pPixelBuffer, pInclude, pShaderMacro, bComposing ? CompiledComponentPixelShder : CompiledPixelShder);
      if(GXFAILED(hr)) {
        goto FUNC_RET;
      }
    }

    hr = LoadFromMemory(pVertexBuffer, pPixelBuffer);
  }
  else {
    hr = GX_FAIL;
  }

FUNC_RET:
  SAFE_DELETE(pInclude);
  SAFE_DELETE_ARRAY(pShaderMacro);
  SAFE_DELETE(ShaderBuffers.pVertexShader);
  SAFE_DELETE(ShaderBuffers.pPixelShader);

  return hr;
}
#endif // #ifdef REFACTOR_GRAPX_SHADER