#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GShader.H"
#include "GrapX/GTexture.H"
#include "GrapX/GXKernel.H"

// 平台相关
#include "GrapX/Platform.h"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"


// 私有头文件
#include <GrapX/VertexDecl.H>
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.H"
#include "clPathFile.H"
#include "Smart/smartstream.h"
#include "GDI/GXShaderMgr.h"
//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)
#define PS_HANDLE_SHIFT 16

using namespace clstd;

namespace D3D9
{
  //////////////////////////////////////////////////////////////////////////

#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/D3D_ShaderImpl.inl"
#include "Platform/CommonInline/X_ShaderImpl.inl"

  //////////////////////////////////////////////////////////////////////////
  STATIC_ASSERT(D3DXINC_LOCAL == c_D3D_INCLUDE_LOCAL);
  STATIC_ASSERT(D3DXINC_SYSTEM == c_D3D_INCLUDE_SYSTEM);
  //////////////////////////////////////////////////////////////////////////

  GShaderImpl::GShaderImpl(GXGraphics* pGraphics)
    : GShader           ()
    , m_pGraphicsImpl   ((GXGraphicsImpl*)pGraphics)
    , m_dwFlag          (NULL)
    , m_pVertexShader   (NULL)
    , m_pPixelShader    (NULL)
    , m_pvct            (NULL)
    , m_ppct            (NULL)
    , m_cbCacheSize     (0)
    , m_cbPixelTopIndex (0)
  {    
    memset(&m_VertexShaderConstTabDesc, 0, sizeof(m_VertexShaderConstTabDesc));
    memset(&m_PixelShaderConstTabDesc, 0, sizeof(m_PixelShaderConstTabDesc));
    AddRef();
  }

  GShaderImpl::~GShaderImpl()
  {
    if( ! m_pVertexShader && ! m_pPixelShader && ! m_dwFlag) {
      // 走到这里应该是创建失败了，都应该是空的
      ASSERT( ! m_pvct && ! m_ppct);
      return;
    }
    m_pGraphicsImpl->UnregisterResource(this);
    CleanUp();
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GShaderImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GShaderImpl::Release()
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

  GXHRESULT GShaderImpl::CleanUp()
  {
    m_cbCacheSize = 0;

    Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();
    ASSERT(pConstNameObj != NULL);

    for(ConstantDescArray::iterator it = m_aConstDesc.begin();
      it != m_aConstDesc.end(); ++it) {
        pConstNameObj->RemoveName(this, it->Name);
    }
    m_aConstDesc.clear();

    SAFE_RELEASE(m_ppct);
    SAFE_RELEASE(m_pvct);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pVertexShader);
    m_dwFlag = NULL;
    return GX_OK;
  }

  GXHRESULT GShaderImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    if(pDesc->szCmdString != NULL)
    {
      if(clstd::strcmpT(pDesc->szCmdString, "reloadshader") == 0)
      {
        MOSHADER_ELEMENT_SOURCE ses;
        GXHRESULT hval = GShader::Load(m_strProfileDesc, m_pGraphicsImpl->m_strResourceDir, 
          m_pGraphicsImpl->InlGetPlatformStringA(), &ses, NULL);
        if(GXFAILED(hval)) {
          return hval;
        }

        m_pGraphicsImpl->IntAttachComposerSdrDesc(&ses);

        if(GXFAILED(LoadFromFile(&ses)))
        {
          ses.strPreVS.Clear();
          ses.strVS = "shaders\\ErrorShader.txt";
          //ses.strVSExtra.Clear();
          ses.strPS = "shaders\\ErrorShader.txt";
          ses.strVSComposer.Clear();
          ses.strPSComposer.Clear();
          //ses.aPSComponent.clear();
          ses.strMacros.Clear();
          LoadFromFile(&ses);
        }
        else {
          CLOGW(L"Reload shader: %s\n", m_strProfileDesc);
        }
      }
    }
    else {
      if(pDesc->dwCmdCode == RCC_Shader)
      {
        GXLPCSTR szName = (GXLPCSTR)pDesc->lParam;
        ASSERT(pDesc->wParam > 0); // CanvasUniform肯定大于0
        for(ConstantDescArray::iterator it = m_aConstDesc.begin();
          it != m_aConstDesc.end(); ++it)
        {
          if(GXSTRCMP(szName, it->Name) == 0)
          {
            it->nCanvasUniform = pDesc->wParam;
            break;
          }
        }
      }
    }
    return GX_OK;
  }

  GXINT GShaderImpl::UpdateConstTabDesc(LPD3DXCONSTANTTABLE pct, LPD3DXCONSTANTTABLE_DESC pctd, GXUINT uHandleShift)
  {
    GXINT nCacheSize = 0;
    GXDWORD dwTopIndex = (GXDWORD)m_aConstDesc.size() + 1;
    Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();
    pct->GetDesc(pctd);

    GXD3DXCONSTDESC cd;
    for(GXUINT i = 0; i < pctd->Constants; i++)
    {
      GXUINT uCount;
      D3DXHANDLE hConst = pct->GetConstant(NULL, i);
      pct->GetConstantDesc(hConst, &cd, &uCount);

      //// 把RegisterIndex +1,便于Pixel与Vertex的Index打包
      //cd.RegisterIndex = cd.RegisterIndex;
#ifdef _DEBUG
      if(cd.RegisterSet == D3DXRS_SAMPLER)
      {
        ASSERT(cd.Bytes == 4);
      }
      else if(cd.RegisterSet == D3DXRS_FLOAT4 ||
        cd.RegisterSet == D3DXRS_INT4)
      {
        // Shader被优化后有可能Rows和RegisterCount不相等
        //ASSERT(cd.Rows == cd.RegisterCount);
        ASSERT(cd.Bytes == cd.Rows * cd.Columns * sizeof(float));
      }
      else
        ASSERT(0);
#endif // _DEBUG
      // Sampler不算在常量寄存器内
      // 寄存器的大小按照出现的最大寄存器索引+占用的寄存器数量决定,
      // 因为寄存器可能是跳跃的,不一定是连续使用的.
      if(cd.RegisterSet != D3DXRS_SAMPLER) {
        nCacheSize = clMax(nCacheSize, 
          (GXINT)((cd.RegisterIndex + cd.RegisterCount) * sizeof(float4)));  // 根据上面的断言调整,保证这个计算不会覆盖
      }

      cd.dwNameID = clStringA(cd.Name).GetHash();
      cd.dwHandle = (i + dwTopIndex) << uHandleShift;
      cd.nCanvasUniform = pConstNameObj->AddName(this, cd.Name, cd.Bytes);
      m_aConstDesc.push_back(cd);
    }
    return nCacheSize;
  }
  
  GXHRESULT GShaderImpl::LoadFromMemory(const clBufferBase* pVertexBuf, const clBufferBase* pPixelBuf)
  {
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXHRESULT hr = GX_OK;

    // 清理旧的对象
    CleanUp();

    if(pVertexBuf == NULL || pPixelBuf == NULL) {
      return GX_FAIL;
    }

    hr = lpd3dDevice->CreateVertexShader((GXDWORD*)pVertexBuf->GetPtr(),&m_pVertexShader);
    if(SUCCEEDED(hr))
    {
      D3DXGetShaderConstantTable((GXDWORD*)pVertexBuf->GetPtr(), &m_pvct);
      m_dwFlag |= GXSHADERCAP_VERTEX;
    }
    else {
      return hr;
    }

    hr = lpd3dDevice->CreatePixelShader((GXDWORD*)pPixelBuf->GetPtr(),&m_pPixelShader);
    if(SUCCEEDED(hr))
    {
      D3DXGetShaderConstantTable((GXDWORD*)pPixelBuf->GetPtr(), &m_ppct);
      m_dwFlag |= GXSHADERCAP_PIXEL;
    }
    else {
      return hr;
    }

    m_cbPixelTopIndex = UpdateConstTabDesc(m_pvct, &m_VertexShaderConstTabDesc, 0);
    m_cbCacheSize = UpdateConstTabDesc(m_ppct, &m_PixelShaderConstTabDesc, PS_HANDLE_SHIFT) + m_cbPixelTopIndex;
    return hr;
  }

  GXHRESULT GShaderImpl::CompileShader(clBuffer* pBuffer, LPD3DXINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled)
  {
    LPD3DXBUFFER pShader = NULL;
    LPD3DXBUFFER pErrorBuf = NULL;
    GXHRESULT hval = GX_OK;

    LPCSTR szFunctionName = NULL;
    LPCSTR szProfile = NULL;

    switch(eCompiled)
    {
    case CompiledComponentPixelShder:
      szFunctionName = "compose_ps_main";
      szProfile = "ps_3_0";
      break;
    case CompiledPixelShder:
      szFunctionName = "ps_main";
      szProfile = "ps_3_0";
      break;
    case CompiledComponentVertexShder:
      szFunctionName = "compose_vs_main";
      szProfile = "vs_3_0";
      break;
    case CompiledVertexShder:
      szFunctionName = "vs_main";
      szProfile = "vs_3_0";
      break;
    default:
      return GX_FAIL;
    }

    if(FAILED(D3DXCompileShader((LPCSTR)pBuffer->GetPtr(), (UINT)pBuffer->GetSize(), 
      (D3DXMACRO*)pMacros, pInclude, szFunctionName, szProfile, NULL, &pShader, &pErrorBuf, NULL)))
    {
      LPCSTR szErrorString = (LPCSTR)pErrorBuf->GetBufferPointer();
      CLOG_ERROR("Shader compiled error:\n>%s\n", szErrorString);
      hval = GX_FAIL;
    }
#if 0
    // Test D3DXDisassembleShader
    LPD3DXBUFFER pAsmBuffer = NULL;
    GXHRESULT hval2 = D3DXDisassembleShader((DWORD*)pShader->GetBufferPointer(),
      FALSE, NULL, &pAsmBuffer);
    if(GXSUCCEEDED(hval2) && pAsmBuffer)
    {
      LPCSTR szAsmString = (LPCSTR)pAsmBuffer->GetBufferPointer();
      TRACE(szAsmString);
    }
    SAFE_RELEASE(pAsmBuffer);
#endif // #if

    pBuffer->Resize(0, FALSE);
    if(pShader) {
      pBuffer->Append(pShader->GetBufferPointer(), pShader->GetBufferSize());
    }

    SAFE_RELEASE(pErrorBuf);
    SAFE_RELEASE(pShader);
    return hval;
  }

  GXHRESULT GShaderImpl::Activate()
  {
    LPDIRECT3DDEVICE9 const lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    if(m_dwFlag & GXSHADERCAP_VERTEX)
      V(lpd3dDevice->SetVertexShader(m_pVertexShader));
    if(m_dwFlag & GXSHADERCAP_PIXEL)
      V(lpd3dDevice->SetPixelShader(m_pPixelShader));
    return GX_OK;
  }

  const GShaderImpl::ConstantDescArray& GShaderImpl::GetConstantDescTable() GXCONST
  {
    return m_aConstDesc;
  }

  GXUINT GShaderImpl::GetHandle(GXLPCSTR pName) GXCONST
  {
    GXDWORD dwHashId = clStringA(pName).GetHash();
    GXUINT uHandle = 0;
    GXINT nFind = 0;
    for(GShaderImpl::ConstantDescArray::const_iterator it = m_aConstDesc.begin();
      it != m_aConstDesc.end(); ++it)
    {
      if(it->dwNameID == dwHashId)
      {
        ASSERT(clstd::strcmpT(it->Name, pName) == 0); // 粗线这个说明 Hash 冲突

        uHandle |= it->dwHandle;
        if(++nFind == 2)
          return uHandle;
      }
    }
    return uHandle;
  }

  GXUniformType GShaderImpl::GetHandleType(GXUINT handle) GXCONST
  {
    const int nID = ((handle & 0xffff) != 0 ? (handle & 0xffff) : (handle >> 16));
    const GXD3DXCONSTDESC& Desc = m_aConstDesc[nID - 1];
    if(Desc.Type == D3DXPT_FLOAT)
    {
      if(Desc.Class == D3DXPC_SCALAR)
      {
        ASSERT(Desc.Columns == 1);
        return GXUB_FLOAT;
      }
      else if(Desc.Class == D3DXPC_VECTOR)
      {
        if(Desc.Columns == 2) {
          return GXUB_FLOAT2;
        }
        else if(Desc.Columns == 3) {
          return GXUB_FLOAT3;
        }
        else if(Desc.Columns == 4) {
          return GXUB_FLOAT4;
        }
        else {
          ASSERT(0);
        }
      }
      else if(Desc.Class == D3DXPC_MATRIX_COLUMNS)
      {
        return GXUB_MATRIX4;
      }
      else ASSERT(0);
    }
    else if(Desc.Type == D3DXPT_SAMPLER2D)
    {
      ASSERT(Desc.Class == D3DXPC_OBJECT);
      return GXUB_SAMPLER2D;
    }
    else if(Desc.Type == D3DXPT_SAMPLER3D)
    {
      ASSERT(Desc.Class == D3DXPC_OBJECT);
      return GXUB_SAMPLER3D;
    }
    else
    {
      // 不支持的类型转换
      CLBREAK;
    }
    ASSERT(0);
    return GXUB_UNDEFINED;
  }

  GXUINT GShaderImpl::GetStageByHandle(GXUINT handle) GXCONST
  {
    //ASSERT(GetHandleType(handle) == GXUB_SAMPLER2D);
    ASSERT((handle & 0xffff) == 0);
    const int nID = GXHIWORD(handle);
    const GXD3DXCONSTDESC& Desc = m_aConstDesc[nID - 1];
    ASSERT(Desc.Type == D3DXPT_SAMPLER2D || Desc.Type == D3DXPT_SAMPLER3D);
    return Desc.RegisterIndex;
  }
#ifdef REFACTOR_SHADER
  GXBOOL GShaderImpl::CommitToDevice(GXLPVOID lpUniform, GXSIZE_T cbSize)
  {
    if(cbSize != m_cbCacheSize) {
      return FALSE;
    }
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    const UINT nVector4fCount = m_cbPixelTopIndex / sizeof(float4);
    lpd3dDevice->SetVertexShaderConstantF(0, (const float*)lpUniform, nVector4fCount);
    lpd3dDevice->SetPixelShaderConstantF(0, (const float*)(((LPBYTE)lpUniform) + m_cbPixelTopIndex), (m_cbCacheSize - m_cbPixelTopIndex) / sizeof(float4));
    return TRUE;
  }
#endif // #ifdef REFACTOR_SHADER
} // namespace D3D9

#include <Smart/smartstream.h>
#include <Smart/SmartProfile.h>

GXVOID GShader::ResolveProfileDescW(GXLPCWSTR szProfileDesc, clStringW* pstrFilename, clStringA* pstrMacros)
{
  if(pstrFilename == NULL && pstrMacros == NULL) {
    return;
  }
  
  clStringW strFilename = szProfileDesc;
  if(strFilename.IsEmpty()) {
    return;
  }

  const int pos = (int)strFilename.Find('|');
  if(pos > 0) {
    // 如果是复合描述: "文件名|宏"
    if(pstrMacros != NULL) {
      *pstrMacros = strFilename.Right(strFilename.GetLength() - pos - 1);
    }
    if(pstrFilename != NULL) {
      *pstrFilename = strFilename.Left(pos);
    }
  }
  else if(pstrFilename != NULL) {
    // 单纯文件名
    *pstrFilename = strFilename;
  }
  return;
}

GXHRESULT GShader::Load(GXLPCWSTR szShaderDesc, GXLPCWSTR szResourceDir, GXLPCSTR szPlatformSect, MOSHADER_ELEMENT_SOURCE* pElement, GXOUT MTLFILEPARAMDESC* pMtlParam)
{
  SmartProfileA sp;
  clStringA strSect = clStringA("shader\\") + szPlatformSect;
  clStringW strFilename;
  clStringA strMacros;

  GShader::ResolveProfileDescW(szShaderDesc, &strFilename, &strMacros);
  if(strFilename.IsEmpty()) {
    return FALSE;
  }
  
  if( ! IsFullPath(strFilename)) {
    clpathfile::CombinePathW(strFilename, szResourceDir, strFilename);
  }

  if( ! sp.LoadW(strFilename)) {
    CLOG_WARNING(__FUNCTION__": Can not open file(%s).\n", clStringA(strFilename));
    return GX_E_OPEN_FAILED;
  }

  clStringA strResourceDirA(szResourceDir);
  MOSHADER_ELEMENT_SOURCE& ses = *pElement;
  GShader::LoadElementSource(&sp, strSect, pElement, pMtlParam == NULL ? NULL : &pMtlParam->aBindPool);
  ses.strMacros = strMacros;

  if(pElement->strPreVS.IsNotEmpty() && ! IsFullPath(ses.strPreVS))
    clpathfile::CombinePathA(ses.strPreVS, strResourceDirA, ses.strPreVS);
  
  if(ses.strVS.IsNotEmpty() && ! IsFullPath(ses.strVS))
    clpathfile::CombinePathA(ses.strVS, strResourceDirA, ses.strVS);
  
  //if(ses.strVSExtra.IsNotEmpty() && ! IsFullPath(ses.strVSExtra))
  //  clpathfile::CombinePathA(ses.strVSExtra, strResourceDirA, ses.strVSExtra);
  
  if(ses.strPS.IsNotEmpty() && ! IsFullPath(ses.strPS))
    clpathfile::CombinePathA(ses.strPS, strResourceDirA, ses.strPS);
  
  if(ses.strVSComposer.IsNotEmpty() && ! IsFullPath(ses.strVSComposer))
    clpathfile::CombinePathA(ses.strVSComposer, strResourceDirA, ses.strVSComposer);
  
  if(ses.strPSComposer.IsNotEmpty() && ! IsFullPath(ses.strPSComposer))
    clpathfile::CombinePathA(ses.strPSComposer, strResourceDirA, ses.strPSComposer);

  //for(clStringArrayA::iterator it = ses.aPSComponent.begin();
  //  it != ses.aPSComponent.end(); ++it) {
  //    if(it->IsNotEmpty() && ! IsFullPath(*it)) {
  //      clpathfile::CombinePathA(*it, strResourceDirA, *it);
  //    }
  //}

  if(pMtlParam != NULL)
  {
    GShader::LoadUniformSet(&sp, strSect, &pMtlParam->aUniforms);
    GShader::LoadStateSet(&sp, strSect, &pMtlParam->aStates);
  }

  return GX_OK;
}

GXBOOL GShader::LoadElementSource(SmartProfileA* pSmart, GXLPCSTR szSection, MOSHADER_ELEMENT_SOURCE* pElement, clStringArrayA* aDataPool)
{
  typedef SmartProfileA::HANDLE sHANDLE;
  typedef SmartProfileA::VALUE sVALUE;

  //clStringA strSect = clStringA("shader\\") + szPlatformSect;
  //clStringA strComponentSect = clStringA(szSection) + "\\PixelComponent";


  sHANDLE handle = pSmart->OpenSection(szSection);
  //sHANDLE hPixelCpn = pSmart->OpenSection(strComponentSect);

  if(handle == NULL) {
    return GX_ERROR_HANDLE;
  }
  //MOSHADER_ELEMENT_SOURCE ses;
  //clStringArrayA aPixelComponentKeys;
  pElement->bExtComposer  = pSmart->FindKeyAsBoolean(handle, "ExternalComposer", FALSE);
  pElement->strPreVS      = pSmart->FindKeyAsString(handle, "PreVertexShader", "");
  pElement->strVS         = pSmart->FindKeyAsString(handle, "VertexShader", "");

  pElement->strPS         = pSmart->FindKeyAsString(handle, "PixelShader", "");
  pElement->strVSComposer = pSmart->FindKeyAsString(handle, "VertexShaderComposer", "");
  pElement->strPSComposer = pSmart->FindKeyAsString(handle, "PixelShaderComposer", "");

  if(aDataPool != NULL)
  {
    clStringA strDataPool = pSmart->FindKeyAsString(handle, "DataPool", "");
    if(strDataPool.IsNotEmpty())
    {
      if(strDataPool.Find(';', 0)) {
        aDataPool->push_back(strDataPool);
      }
      else {
        ResolveString(strDataPool, ';', *aDataPool);
      }
    }
  }
  //pElement->strMacros     = strMacros;

  //ConvertToAbsolutePathA(pElement->strPreVS);
  //ConvertToAbsolutePathA(pElement->strVS);
  //ConvertToAbsolutePathA(pElement->strVSExtra);
  //ConvertToAbsolutePathA(pElement->strPS);
  //ConvertToAbsolutePathA(pElement->strVSComposer);
  //ConvertToAbsolutePathA(pElement->strPSComposer);
  //clStringA strResourceDir = GetResourceFilePathW();
  //if(clpathfile::IsRelativeA(ses.strPreVS)) {
  //  clpathfile::CombinePathA(ses.strPreVS, strResourceDir, ses.strPreVS);
  //}

  //clStringA strPixelComponent = pSmart->FindKeyAsString(handle, "PixelComponent", "");
  //ResolveString(strPixelComponent, ';', aPixelComponentKeys);
  //for(clStringArrayA::iterator it = aPixelComponentKeys.begin();
  //  it != aPixelComponentKeys.end(); ++it)
  //{
  //  clStringA strPSComponent = pSmart->FindKeyAsString(handle, *it, "");
  //  if( ! strPSComponent.IsEmpty()) {
  //    //ConvertToAbsolutePathA(strPSComponent);
  //    pElement->aPSComponent.push_back(strPSComponent);
  //  }
  //}
  //pSmart->CloseHandle(hPixelCpn);
  pSmart->FindClose(handle);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
inline GXBOOL IntLoadShaderComponent(const clStringA& strFilename, clFile& File, GXBOOL bCompiled, clBuffer** ppComponentBuf, GXLPCSTR szFailedText)
{
  if(strFilename.IsNotEmpty())
  {
    if(File.OpenExistingA(strFilename)) {
      if(File.MapToBuffer(ppComponentBuf)) {
        // TODO: 应该优化,避免插入的内存复制
        if( ! bCompiled)  // 不是编译格式的话就插入 #line 宏
        {
          clStringA strFileDesc;
          ASSERT( ! clpathfile::IsRelativeA(strFilename));
          strFileDesc.Format("\r\n#line 1 \"%s\"\r\n", strFilename);
          (*ppComponentBuf)->Insert(0, (CLLPVOID)strFileDesc.GetBuffer(), strFileDesc.GetLength());
        }
        return TRUE;
      }
    }
    else {
      CLOG_WARNING(szFailedText, strFilename);
      return FALSE;
    }
  }
  return FALSE;
}

GXBOOL GShader::ComposeSource(MOSHADER_ELEMENT_SOURCE* pSrcComponent, GXDWORD dwPlatfomCode, GXOUT MOSHADERBUFFERS* pSources, GXOUT GXDefinitionArray* aMacros)
{
  clFile  File;
  clBuffer*& pVertexBuffer = pSources->pVertexShader;
  clBuffer*& pPixelBuffer  = pSources->pPixelShader;
  clBuffer* pComponentBuf = NULL;
  clBuffer* pDeclCodesBuf = NULL;
  GXBOOL result = TRUE;
  //GXHRESULT hr = GX_OK;
  //clvector<GXDefinition> aMacros; // 这个要放在这里, 避免在其他的作用域中析构

  clsize pos;
  // 只有扩展名为"HLSL"才认为是源代码
  pos = clpathfile::FindExtensionA(pSrcComponent->strVS);
  const GXBOOL bCompiledVS = (pos <= 0 || GXSTRCMPI(&pSrcComponent->strVS[(int)pos], ".hlsl") != 0);
  pos = clpathfile::FindExtensionA(pSrcComponent->strPS);
  const GXBOOL bCompiledPS = (pos <= 0 || GXSTRCMPI(&pSrcComponent->strPS[(int)pos], ".hlsl") != 0);

  // TODO: 二进制数据和源码数据可能会有混合错误的问题...
  if( ! IntLoadShaderComponent(pSrcComponent->strVS, File, bCompiledVS, &pVertexBuffer, "Can't load VertexShader(%s).\n")) {
    result = FALSE;
  }

  // 如果文件名一致则跳过磁盘IO直接复制一份
  if(GXSTRCMPI<ch>(pSrcComponent->strVS, pSrcComponent->strPS) == 0) 
  {
    pPixelBuffer = new clBuffer;
    pPixelBuffer->Append(pVertexBuffer->GetPtr(), pVertexBuffer->GetSize());
  }
  else if( ! IntLoadShaderComponent(pSrcComponent->strPS, File, bCompiledPS, &pPixelBuffer, "Can't load PixelShader(%s).\n")) {
    result = FALSE;
  }
  GXDEFINITION* pShaderMacro = NULL;
  int nCodeLenWithoutSwitcherMacro = 0; // pDeclCodesBuf 不带宏开关时的长度
  if( ! (bCompiledPS && bCompiledVS))
  {
    extern DATALAYOUT g_StandardMtl[];
    MOGenerateDeclarationCodes(g_StandardMtl, dwPlatfomCode, &pDeclCodesBuf);
    if(pDeclCodesBuf)
    {
      clStringA strSwitcherMacro;
      nCodeLenWithoutSwitcherMacro = (int)pDeclCodesBuf->GetSize();
      strSwitcherMacro = "#define _COMPOSING_SHADER\r\n";
      pDeclCodesBuf->Append(strSwitcherMacro.GetBuffer(), strSwitcherMacro.GetLength());
    }

    if(aMacros != NULL && pSrcComponent->strMacros.IsNotEmpty())
    {
      int i = 0;
      ResolverMacroStringToD3DMacro(pSrcComponent->strMacros, *aMacros);
    }
  }

  // 编译的PS/VS只能在VertexShader和PixelShader里设定
  // 组件内的指定都认为是源代码, 并且只有在VertexShader和PixelShader为源代码时才加载.
  if(result)
  {
    if( ! bCompiledVS)
    {
      if(IntLoadShaderComponent(pSrcComponent->strPreVS, File, FALSE, &pComponentBuf, "Can't load PreVertexShader(%s).\n")) {
        pVertexBuffer->Insert(0, pComponentBuf->GetPtr(), pComponentBuf->GetSize());
        SAFE_DELETE(pComponentBuf);
      }

      //if(IntLoadShaderComponent(pSrcComponent->strVSExtra, File, FALSE, &pComponentBuf, "Can't load VertexShaderExtra(%s).\n")) {
      //  pVertexBuffer->Insert(0, pComponentBuf->GetPtr(), pComponentBuf->GetSize());
      //  SAFE_DELETE(pComponentBuf);
      //}

      if(IntLoadShaderComponent(pSrcComponent->strVSComposer, File, FALSE, &pComponentBuf, "Can't load VertexShaderComposer(%s).\n")) {
        pVertexBuffer->Append(pComponentBuf->GetPtr(), pComponentBuf->GetSize());
        SAFE_DELETE(pComponentBuf);
      }
    }

    if( ! bCompiledPS)
    {
      //for(clStringArrayA::iterator it = pSrcComponent->aPSComponent.begin();
      //  it != pSrcComponent->aPSComponent.end(); ++it)
      //{
      //  if(IntLoadShaderComponent(*it, File, FALSE, &pComponentBuf, "Can't load PixelShaderComposer(%s).\n")) {
      //    pPixelBuffer->Append(pComponentBuf->GetPtr(), pComponentBuf->GetSize());
      //    SAFE_DELETE(pComponentBuf);
      //  }
      //}

      if(IntLoadShaderComponent(pSrcComponent->strPSComposer, File, FALSE, &pComponentBuf, "Can't load PixelShaderComposer(%s).\n")) {
        pPixelBuffer->Append(pComponentBuf->GetPtr(), pComponentBuf->GetSize());
        SAFE_DELETE(pComponentBuf);
      }
    }

    // --------------

    if( ! bCompiledVS)
    {
      const GXBOOL bComposing = pSrcComponent->strVSComposer.IsNotEmpty();
      if(pDeclCodesBuf) {
        pVertexBuffer->Insert(0, pDeclCodesBuf->GetPtr(), bComposing ? pDeclCodesBuf->GetSize() : nCodeLenWithoutSwitcherMacro);
      }
    }

    if( ! bCompiledPS)
    {
      const GXBOOL bComposing = pSrcComponent->strPSComposer.IsNotEmpty();
      if(pDeclCodesBuf) {
        pPixelBuffer->Insert(0, pDeclCodesBuf->GetPtr(), bComposing ? pDeclCodesBuf->GetSize() : nCodeLenWithoutSwitcherMacro);
      }
    }
  }

  SAFE_DELETE(pDeclCodesBuf);
  return result;
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GShader::LoadUniformSet(SmartProfileA* pSmart, GXLPCSTR szSection, ParamArray* aUniforms)
{
  typedef SmartProfileA::HANDLE sHANDLE;
  typedef SmartProfileA::VALUE sVALUE;

  clStringA strSection = clStringA(szSection) + "\\parameter";

  //for(int i = 0; i < 2; i++)
  {
    sHANDLE hParam = pSmart->OpenSection(strSection);
    if(hParam != NULL)
    {
      sVALUE val;
      sHANDLE hKey = pSmart->FindFirstKey(hParam, val);
      if(hKey != NULL) {
        do {
          GXDefinition Def;
          Def.Name = val.KeyName();
          Def.Value = val.ToString();
          aUniforms->push_back(Def);
        } while (pSmart->FindNextKey(hKey, val));
      }

      //sHANDLE hSect = pSmart->FindFirstSection(hParam, FALSE, NULL, NULL);
      sHANDLE hSect = pSmart->FindFirstSection(NULL, FALSE, strSection, NULL);
      GXDefinition Def;

      if(hSect != NULL)
      {
        do {
          sHANDLE hKeySampler = pSmart->FindFirstKey(hSect, val);
          if(hKeySampler != NULL)
          {
            Def.Name = "SAMPLER";
            Def.Value = pSmart->GetSectionName(hSect);
            aUniforms->push_back(Def);
            do {
              Def.Name = val.KeyName();
              Def.Value = val.ToString();
              aUniforms->push_back(Def);
            } while(pSmart->FindNextKey(hKeySampler, val));
          }
          Def.Name = "SAMPLER";
          Def.Value = "";
          aUniforms->push_back(Def);
          pSmart->CloseHandle(hKeySampler);
        } while (pSmart->FindNextSection(hSect));
        pSmart->CloseHandle(hSect);
      }
      pSmart->CloseHandle(hKey);
      pSmart->CloseHandle(hParam);
    }
  }
  return TRUE;
}

GXBOOL GShader::LoadStateSet(SmartProfileA* pSmart, GXLPCSTR szSection, ParamArray* aStates)
{
  typedef SmartProfileA::HANDLE sHANDLE;
  typedef SmartProfileA::VALUE sVALUE;

  for(int i = 0; i < 2; i++)
  {
    sHANDLE hParam = pSmart->OpenSection(clStringA(szSection) + "\\state");
    if(hParam != NULL)
    {
      sVALUE val;
      sHANDLE hKey = pSmart->FindFirstKey(hParam, val);
      if(hKey != NULL) {
        do {
          GXDefinition Def;
          Def.Name = val.KeyName();
          Def.Value = val.ToString();
          aStates->push_back(Def);
        } while (pSmart->FindNextKey(hKey, val));
      }

      pSmart->CloseHandle(hKey);
      pSmart->CloseHandle(hParam);
    }
  }
  return TRUE;
}

//SetVertexShaderConstantB
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)