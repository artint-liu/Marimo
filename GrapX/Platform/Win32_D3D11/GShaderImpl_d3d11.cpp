#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_

// ȫ��ͷ�ļ�
#include <GrapX.h>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXKernel.h"

// ƽ̨���
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GShaderImpl_D3D11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_D3D11.h"


// ˽��ͷ�ļ�
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "Platform/Win32_D3D11/GXCanvasImpl_D3D11.h"
#include "clPathFile.h"

//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)
#define PS_HANDLE_SHIFT 16
namespace D3D11
{
  //////////////////////////////////////////////////////////////////////////

#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/D3D_ShaderImpl.inl"
#include "Platform/CommonInline/X_ShaderImpl.inl"

  //////////////////////////////////////////////////////////////////////////
  STATIC_ASSERT(D3D_INCLUDE_LOCAL == c_D3D_INCLUDE_LOCAL);
  STATIC_ASSERT(D3D_INCLUDE_SYSTEM == c_D3D_INCLUDE_SYSTEM);

  //////////////////////////////////////////////////////////////////////////
  GShaderImpl::GShaderImpl(GXGraphics* pGraphics)
    : GShader         ()
    , m_pGraphicsImpl  ((GXGraphicsImpl*)pGraphics)
    , m_dwFlag        (NULL)
    , m_pVertexShader (NULL)
    , m_pPixelShader  (NULL)
    , m_pVertexBuf    (NULL)
    , m_pvct          (NULL)
    , m_ppct          (NULL)
    , m_cbCacheSize    (0)
    , m_cbPixelTopIndex  (0)
  {
    memset(&m_VertexShaderConstTabDesc, 0, sizeof(m_VertexShaderConstTabDesc));
    memset(&m_PixelShaderConstTabDesc, 0, sizeof(m_PixelShaderConstTabDesc));
    AddRef();
  }

  GShaderImpl::~GShaderImpl()
  {
    CleanUp();
  }

  GXHRESULT GShaderImpl::CleanUp()
  {
    m_cbCacheSize = 0;
    m_aConstDesc.clear();

    SAFE_DELETE(m_pVertexBuf);
    SAFE_RELEASE(m_ppct);
    SAFE_RELEASE(m_pvct);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pVertexShader);

    for(BufPairArray::iterator it = m_aBufPairs.begin();
      it != m_aBufPairs.end(); ++it)
    {
      SAFE_RELEASE(it->pD3D11ResBufer);
      SAFE_DELETE(it->pUserBuffer);
    }
    m_dwFlag = NULL;
    return GX_OK;
  }

  GXINT GShaderImpl::UpdateConstTabDesc(ID3D11ShaderReflection* pct, D3D11_SHADER_DESC* pctd, GXUINT uHandleShift)
  {
    UINT i = 0;
    GXINT nCacheSize = 0;
    GXDWORD dwTopIndex = (GXDWORD)m_aConstDesc.size() + 1;

    pct->GetDesc(pctd);

    // Const Buffer ѭ��
    for(UINT ncb = 0; ncb < pctd->ConstantBuffers; ncb++)
    {
      D3D11_SHADER_BUFFER_DESC SdrBufDesc;
      ID3D11ShaderReflectionConstantBuffer* pConstBuf = pct->GetConstantBufferByIndex(ncb);

      pConstBuf->GetDesc(&SdrBufDesc);
      ASSERT(SdrBufDesc.Type == D3D_CT_CBUFFER);
      nCacheSize += SdrBufDesc.Size;
      int nCBArrayIdx = CreateShaderConstBuffer(SdrBufDesc.Size, ncb, uHandleShift != 0);

      for(UINT n = 0; n < SdrBufDesc.Variables; n++, i++)
      {
        D3D11_SHADER_VARIABLE_DESC SdrVarDesc;
        D3D11_SHADER_TYPE_DESC SdrTypeDesc;
        ID3D11ShaderReflectionVariable* pSdrVar = pConstBuf->GetVariableByIndex(n);
        if(pSdrVar != NULL)
        {
          GXCONSTDESC cd;
          ID3D11ShaderReflectionType* pSdrType = pSdrVar->GetType();
          pSdrVar->GetDesc(&SdrVarDesc);
          pSdrType->GetDesc(&SdrTypeDesc);

          cd.strName      = SdrVarDesc.Name;
          cd.dwNameID     = cd.strName.GetHash();
          cd.nConstBuf    = ncb;
          cd.nCBArrayIdx  = nCBArrayIdx;
          cd.dwHandle     = (i + dwTopIndex) << uHandleShift;
          cd.StartOffset  = SdrVarDesc.StartOffset;    // Offset in constant buffer's backing store
          cd.Size         = SdrVarDesc.Size;           // Size of variable (in bytes)
          cd.uFlags       = SdrVarDesc.uFlags;         // Variable flags
          cd.StartTexture = SdrVarDesc.StartTexture;   // First texture index (or -1 if no textures used)
          cd.TextureSize  = SdrVarDesc.TextureSize;    // Number of texture slots possibly used.
          cd.StartSampler = SdrVarDesc.StartSampler;   // First sampler index (or -1 if no textures used)
          cd.SamplerSize  = SdrVarDesc.SamplerSize;    // Number of sampler slots possibly used.
          cd.Class        = SdrTypeDesc.Class;          // Variable class (e.g. object, matrix, etc.)
          cd.Type         = SdrTypeDesc.Type;           // Variable type (e.g. float, sampler, etc.)
          cd.Rows         = SdrTypeDesc.Rows;           // Number of rows (for matrices, 1 for other numeric, 0 if not applicable)
          cd.Columns      = SdrTypeDesc.Columns;        // Number of columns (for vectors & matrices, 1 for other numeric, 0 if not applicable)
          cd.Elements     = SdrTypeDesc.Elements;       // Number of elements (0 if not an array)
          cd.Members      = SdrTypeDesc.Members;        // Number of members (0 if not a structure)
          cd.MemberOffset = SdrTypeDesc.Offset;         // Offset from the start of structure (0 if not a structure member)
          m_aConstDesc.push_back(cd);
        }
        //__asm nop
        //SAFE_RELEASE(pSdrVar);
      }

      //SAFE_RELEASE(pConstBuf);
    }
    for(UINT nrb = 0; nrb < pctd->BoundResources; nrb++)
    {
      D3D11_SHADER_INPUT_BIND_DESC InputBindDesc;
      HRESULT hval = pct->GetResourceBindingDesc(nrb, &InputBindDesc);
      //__asm nop
    }

    return nCacheSize;
  }
  
  GXHRESULT GShaderImpl::LoadFromMemory(const clBufferBase* pVertexBuf, const clBufferBase* pPixelBuf)
  {
    ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXHRESULT hval = GX_OK;

    // �����ɵĶ���
    CleanUp();


    if(pVertexBuf == NULL || pPixelBuf == NULL) {
      return GX_FAIL;
    }

    hval = pd3dDevice->CreateVertexShader(pVertexBuf->GetPtr(), pVertexBuf->GetSize(), NULL, &m_pVertexShader);
    if(SUCCEEDED(hval))
    {
      hval = D3DReflect( pVertexBuf->GetPtr(), pVertexBuf->GetSize(), 
        IID_ID3D11ShaderReflection, (void**)&m_pvct);

      if(SUCCEEDED(hval)) {
        m_dwFlag |= GXSHADERCAP_VERTEX;

        // ���︴��һ�� VertexBuffer, ע�ⲻ�� VertexShader
        ASSERT(m_pVertexBuf == NULL);
        m_pVertexBuf = new clBuffer;
        m_pVertexBuf->Append(pVertexBuf->GetPtr(), pVertexBuf->GetSize());
      }
    }

    hval = pd3dDevice->CreatePixelShader(pPixelBuf->GetPtr(), pPixelBuf->GetSize(), NULL, &m_pPixelShader);
    if(SUCCEEDED(hval))
    {
      hval = D3DReflect( pPixelBuf->GetPtr(), pPixelBuf->GetSize(), 
        IID_ID3D11ShaderReflection, (void**)&m_ppct);
      if(SUCCEEDED(hval))
        m_dwFlag |= GXSHADERCAP_PIXEL;
    }

    m_cbPixelTopIndex = UpdateConstTabDesc(m_pvct, &m_VertexShaderConstTabDesc, 0);
    m_cbCacheSize = UpdateConstTabDesc(m_ppct, &m_PixelShaderConstTabDesc, PS_HANDLE_SHIFT) + m_cbPixelTopIndex;
    ////BuildMapper();
    return hval;
  }
  
  GXHRESULT GShaderImpl::CompileShader(clBuffer* pBuffer, LPD3DINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled)
  {
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    ID3DBlob* pShader;

    LPCSTR szShaderFile = NULL;
    LPCSTR szFunctionName = NULL;
    LPCSTR szProfile = NULL;
    switch(eCompiled)
    {
    case CompiledComponentPixelShder:
      szFunctionName = "compose_ps_main";
      szProfile      = "ps_4_0";
      break;
    case CompiledPixelShder:
      szFunctionName = "ps_main";
      szProfile      = "ps_4_0";
      break;
    case CompiledComponentVertexShder:
      szFunctionName = "compose_vs_main";
      szProfile      = "vs_4_0";
      break;
    case CompiledVertexShder:
      szFunctionName = "vs_main";
      szProfile      = "vs_4_0";
      break;
    default:
      return GX_FAIL;
    }

    // D3DCompile
    HRESULT hval = D3DCompile((LPCSTR)pBuffer->GetPtr(), pBuffer->GetSize(), 
      szShaderFile, (D3D10_SHADER_MACRO*)pMacros, pInclude, szFunctionName, szProfile, dwShaderFlags, 0, &pShader, &pErrorBlob);

    if( FAILED(hval) )
    {
      if( pErrorBlob != NULL ) {
        TRACE("Shader compiled error:\n>%s\n", (char*)pErrorBlob->GetBufferPointer() );
      }
      SAFE_RELEASE(pErrorBlob);
      return hval;
    }

    pBuffer->Resize(0, FALSE);
    if(pShader) {
      pBuffer->Append(pShader->GetBufferPointer(), pShader->GetBufferSize());
    }

    SAFE_RELEASE(pShader);
    SAFE_RELEASE(pErrorBlob);
    return S_OK;
  }

  GXHRESULT GShaderImpl::CompileShader(clBuffer* pIntermediateCode, GXLPCSTR szSourceCode, size_t nSourceLen, LPD3DINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled)
  {
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    ID3DBlob* pShader;

    LPCSTR szShaderFile = NULL;
    LPCSTR szFunctionName = NULL;
    LPCSTR szProfile = NULL;
    switch(eCompiled)
    {
    case CompiledComponentPixelShder:
      szFunctionName = "compose_ps_main";
      szProfile = "ps_4_0";
      break;
    case CompiledPixelShder:
      szFunctionName = "ps_main";
      szProfile = "ps_4_0";
      break;
    case CompiledComponentVertexShder:
      szFunctionName = "compose_vs_main";
      szProfile = "vs_4_0";
      break;
    case CompiledVertexShder:
      szFunctionName = "vs_main";
      szProfile = "vs_4_0";
      break;
    default:
      return GX_FAIL;
    }

    HRESULT hval = D3DCompile(szSourceCode, nSourceLen,
      szShaderFile, (D3D10_SHADER_MACRO*)pMacros, pInclude, szFunctionName, szProfile, dwShaderFlags, 0, &pShader, &pErrorBlob);

    if(FAILED(hval))
    {
      if(pErrorBlob != NULL) {
        TRACE("Shader compiled error:\n>%s\n", (char*)pErrorBlob->GetBufferPointer());
      }
      SAFE_RELEASE(pErrorBlob);
      return hval;
    }

    pIntermediateCode->Resize(0, FALSE);
    if(pShader) {
      pIntermediateCode->Append(pShader->GetBufferPointer(), pShader->GetBufferSize());
    }

    SAFE_RELEASE(pShader);
    SAFE_RELEASE(pErrorBlob);
    return S_OK;
  }

  UINT GShaderImpl::CreateShaderConstBuffer(UINT cbSize, UINT nIdx, GXBOOL bPS)
  {
    ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXSDRBUFFERPAIR Pair;
    D3D11_BUFFER_DESC bd;
    InlSetZeroT(bd);

    bd.Usage     = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = cbSize;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = pd3dDevice->CreateBuffer(&bd, NULL, &Pair.pD3D11ResBufer);
    if(FAILED(hr))
    {
      ASSERT(0);
      return -1;
    }
    Pair.nIdx        = nIdx;
    Pair.pUserBuffer = new GXBYTE[cbSize];
    Pair.BufferSize  = cbSize;
    Pair.bNeedUpdate = FALSE;
    Pair.bPS         = bPS;

    m_aBufPairs.push_back(Pair);
    return (UINT)m_aBufPairs.size() - 1;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GShaderImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GShaderImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    
    if(nRefCount == 0)
    {
      //OnDeviceEvent(DE_LostDevice);

      m_pGraphicsImpl->UnregisterResource(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXHRESULT GShaderImpl::Activate()
  {
    ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
    pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
    pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
    return GX_OK;
  }

  GXBOOL GShaderImpl::CheckUpdateConstBuf()
  {
    ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
    for(BufPairArray::iterator it = m_aBufPairs.begin();
      it != m_aBufPairs.end(); ++it)
    {
      if(it->bNeedUpdate) {
        it->bNeedUpdate = FALSE;
        pImmediateContext->UpdateSubresource(it->pD3D11ResBufer, 0, NULL, it->pUserBuffer, 0, 0 );
      }
      if(it->bPS) {
        pImmediateContext->PSSetConstantBuffers(it->nIdx, 1, &it->pD3D11ResBufer);
      }
      else {
        pImmediateContext->VSSetConstantBuffers(it->nIdx, 1, &it->pD3D11ResBufer);
      }
    }
    return TRUE;
  }

  const GShaderImpl::ConstantDescArray& GShaderImpl::GetConstantDescTable() const
  {
    return m_aConstDesc;
  }

  GXUINT GShaderImpl::GetHandle(GXLPCSTR pName) const
  {
    GXDWORD dwHashId = clStringA(pName).GetHash();
    GXUINT uHandle = 0;
    GXINT nFind = 0;
    for(GShaderImpl::ConstantDescArray::const_iterator it = m_aConstDesc.begin();
      it != m_aConstDesc.end(); ++it)
    {
      if(it->dwNameID == dwHashId)
      {
        ASSERT(it->strName == pName);

        uHandle |= it->dwHandle;
        if(++nFind == 2)
          return uHandle;
      }
    }
    return uHandle;
  }

  GXUniformType GShaderImpl::GetHandleType(GXUINT handle) const
  {
    ASSERT(0); // FIXME: ûʵ��
    //m_aConstDesc[handle & 0xffff].Class
    return GXUB_UNDEFINED;
  }
  
  GXUINT GShaderImpl::GetStageByHandle(GXUINT handle) const
  {
    CLBREAK; // FIXME: ûʵ��
    return 0;
  }

  void GShaderImpl::PutInResourceMgr()
  {
    SET_FLAG(m_dwFlag, ShaderFlag_PutInResourceManager);
  }

#ifdef REFACTOR_SHADER
  GXBOOL GShaderImpl::CommitToDevice( GXLPVOID lpUniform, GXUINT cbSize )
  {
    CLBREAK;
    return FALSE;
  }
#endif // #ifdef REFACTOR_SHADER
  //////////////////////////////////////////////////////////////////////////
} // namespace D3D11

GXHRESULT GXDLLAPI MOCompileHLSL(GXLPCWSTR szShaderDesc, GXLPCWSTR szResourceDir, GXLPCSTR szPlatformSect, MOSHADERBUFFERS* pBuffers)
{
  MOSHADER_ELEMENT_SOURCE SrcComponent;
  GXHRESULT hval = GShader::Load(szShaderDesc, szResourceDir, szPlatformSect, &SrcComponent, NULL);
  if(GXFAILED(hval)) {
    return hval;
  }

  GXDefinitionArray aMacros;
  MOSHADERBUFFERS ShaderSources;
  clBuffer*& pVBuf = ShaderSources.pVertexShader; // ��������һ����������,�����Ķ�
  clBuffer*& pPBuf = ShaderSources.pPixelShader;
  GXDEFINITION* pShaderMacro = NULL;
  pVBuf = NULL;
  pPBuf = NULL;
  hval = GX_OK;

  if( ! GShader::ComposeSource(&SrcComponent, GXPLATFORM_WIN32_DIRECT3D11, &ShaderSources, &aMacros)) {
    // LOG: ���Shader Sourceʧ��
    hval = GX_FAIL;
    goto FUNC_RET;
  }

  const GXBOOL bCompiledVS = ((GXLPBYTE)pVBuf->GetPtr())[0] == '\0';
  const GXBOOL bCompiledPS = ((GXLPBYTE)pPBuf->GetPtr())[0] == '\0';

  if(bCompiledVS || bCompiledPS) {
    // LOG: VS/PS���붼ΪԴ������ܱ���
    hval = GX_FAIL;
    goto FUNC_RET;
  }

  // ��������ʱ��Ҫ�ĺ�
  if( ! aMacros.empty())
  {
    int i = 0;
    pShaderMacro = new GXDEFINITION[aMacros.size() + 1];

    // �����Ϊ�˰Ѻ�����Ľ�β��Ϊ NULL, clString �޷�������һ��.
    for(GXDefinitionArray::iterator it = aMacros.begin();
      it != aMacros.end(); ++it, ++i) {
        pShaderMacro[i].szName = it->Name;
        pShaderMacro[i].szValue = it->Value;
    }
    pShaderMacro[i].szName = NULL;
    pShaderMacro[i].szValue = NULL;
  }

  const GXBOOL bVSComposing = SrcComponent.strVSComposer.IsNotEmpty();
  const GXBOOL bPSComposing = SrcComponent.strPSComposer.IsNotEmpty();

  D3D11::IHLSLInclude* pInclude = new D3D11::IHLSLInclude(NULL, clStringA(szResourceDir));

  if(GXFAILED(hval = D3D11::GShaderImpl::CompileShader(pVBuf, pInclude, pShaderMacro, bVSComposing 
    ? D3D11::GShaderImpl::CompiledComponentVertexShder : D3D11::GShaderImpl::CompiledVertexShder))) {
    goto FUNC_RET;
  }

  if(GXFAILED(hval = D3D11::GShaderImpl::CompileShader(pPBuf, pInclude, pShaderMacro, bPSComposing 
    ? D3D11::GShaderImpl::CompiledComponentPixelShder : D3D11::GShaderImpl::CompiledPixelShder))) {
    goto FUNC_RET;
  }

  SAFE_DELETE(pInclude);


  if(pBuffers->pPixelShader == NULL) {
    pBuffers->pPixelShader = new clBuffer(8);
  }
  else { pBuffers->pPixelShader->Resize(0, FALSE); }

  if(pBuffers->pVertexShader == NULL) {
    pBuffers->pVertexShader = new clBuffer(8);
  }
  else { pBuffers->pVertexShader->Resize(0, FALSE); }


  //D3D_DISASM_ENABLE_COLOR_CODE            Enable the output of color codes. 
  //D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS  Enable the output of default values. 
  //D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING Enable instruction numbering. 
  //D3D_DISASM_ENABLE_INSTRUCTION_CYCLE     No effect. 

  ID3DBlob* pDisassembly = NULL;
  const UINT Flags = D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS | D3D_DISASM_DISABLE_DEBUG_INFO;
  //const UINT Flags = D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS;
  // TODO: �жϷ���ֵ
  D3DDisassemble(pVBuf->GetPtr(), pVBuf->GetSize(), Flags, NULL, &pDisassembly);
  TRACE("%s\n\n\n", pDisassembly->GetBufferPointer());
  pBuffers->pVertexShader->Append(pDisassembly->GetBufferPointer(), pDisassembly->GetBufferSize());
  SAFE_RELEASE(pDisassembly);

  D3DDisassemble(pPBuf->GetPtr(), pPBuf->GetSize(), Flags, NULL, &pDisassembly);
  TRACE("%s\n\n\n", pDisassembly->GetBufferPointer());
  pBuffers->pPixelShader->Append(pDisassembly->GetBufferPointer(), pDisassembly->GetBufferSize());
  SAFE_RELEASE(pDisassembly);


FUNC_RET:
  SAFE_DELETE_ARRAY(pShaderMacro);
  SAFE_DELETE(ShaderSources.pVertexShader);
  SAFE_DELETE(ShaderSources.pPixelShader);
  //SAFE_RELEASE(pShader11);
  return hval;
}


//SetVertexShaderConstantB
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)