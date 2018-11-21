#if defined(ENABLE_GRAPHICS_API_DX9)
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"

// 私有头文件
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"
#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.h"
#include "clPathFile.h"
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

  GShaderImpl::GShaderImpl(Graphics* pGraphics)
    : GShader           ()
    , m_pGraphicsImpl   ((GraphicsImpl*)pGraphics)
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
  }

  GShaderImpl::~GShaderImpl()
  {
    if( ! m_pVertexShader && ! m_pPixelShader && TEST_FLAG_NOT(m_dwFlag, GXSHADERCAP_MASK)) {
      // 走到这里应该是创建失败了，都应该是空的
      ASSERT( ! m_pvct && ! m_ppct);
      return;
    }

    if(TEST_FLAG(m_dwFlag, ShaderFlag_PutInResourceManager))
    {
      m_pGraphicsImpl->UnregisterResource(this);
    }
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
    RESET_FLAG(m_dwFlag, GXSHADERCAP_MASK);
    //m_dwFlag = NULL;
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

    hr = lpd3dDevice->CreateVertexShader((DWORD*)pVertexBuf->GetPtr(),&m_pVertexShader);
    if(SUCCEEDED(hr))
    {
      D3DXGetShaderConstantTable((DWORD*)pVertexBuf->GetPtr(), &m_pvct);
      m_dwFlag |= GXSHADERCAP_VERTEX;
    }
    else {
      return hr;
    }

    hr = lpd3dDevice->CreatePixelShader((DWORD*)pPixelBuf->GetPtr(),&m_pPixelShader);
    if(SUCCEEDED(hr))
    {
      D3DXGetShaderConstantTable((DWORD*)pPixelBuf->GetPtr(), &m_ppct);
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
  
  GXHRESULT GShaderImpl::CompileShader(clBuffer* pIntermediateCode, GXLPCSTR szSourceCode, size_t nSourceLen, LPD3DXINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled)
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

    if(FAILED(D3DXCompileShader((LPCSTR)szSourceCode, (UINT)nSourceLen,
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

    pIntermediateCode->Resize(0, FALSE);
    if(pShader) {
      pIntermediateCode->Append(pShader->GetBufferPointer(), pShader->GetBufferSize());
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
        ASSERT(clstd::strcmpT(it->Name, pName) == 0); // 粗线这个说明 Hash 冲突

        uHandle |= it->dwHandle;
        if(++nFind == 2)
          return uHandle;
      }
    }
    return uHandle;
  }

  GXUniformType GShaderImpl::GetHandleType(GXUINT handle) const
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

  GXUINT GShaderImpl::GetStageByHandle(GXUINT handle) const
  {
    //ASSERT(GetHandleType(handle) == GXUB_SAMPLER2D);
    ASSERT((handle & 0xffff) == 0);
    const int nID = GXHIWORD(handle);
    const GXD3DXCONSTDESC& Desc = m_aConstDesc[nID - 1];
    ASSERT(Desc.Type == D3DXPT_SAMPLER2D || Desc.Type == D3DXPT_SAMPLER3D);
    return Desc.RegisterIndex;
  }

  void GShaderImpl::PutInResourceMgr()
  {
    SET_FLAG(m_dwFlag, ShaderFlag_PutInResourceManager);
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
//SetVertexShaderConstantB
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #if defined(ENABLE_GRAPHICS_API_DX9)