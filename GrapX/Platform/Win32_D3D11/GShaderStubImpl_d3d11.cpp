#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

//#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GShaderImpl_D3D11.h"
#include "Platform/Win32_D3D11/GShaderStubImpl_D3D11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_D3D11.h"


// 私有头文件
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "Platform/Win32_D3D11/GXCanvasImpl_D3D11.h"
#include "clPathFile.h"

//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)

using namespace clstd;

//#define PS_HANDLE_SHIFT 16
namespace D3D11
{
#include "Platform/CommonInline/D3D_ShaderStubImpl.inl"

  GShaderStubImpl::GShaderStubImpl(GXGraphics* pGraphics)
    : GShaderStub()
    , m_pGraphicsImpl((GXGraphicsImpl*)pGraphics)
    , m_pShaderImpl(NULL)
  {
  }

  GXHRESULT GShaderStubImpl::SetShaderRef(GShader* pShader)
  {
    // 第二次调用时, 如果pShader为NULL则表示更新链接表
    if(m_pShaderImpl == NULL && pShader == NULL)
      return GX_FAIL;
    else if(pShader != NULL && m_pShaderImpl != pShader)
    {
      SAFE_RELEASE(m_pShaderImpl);
      m_pShaderImpl = (GShaderImpl*)pShader;
      m_pShaderImpl->AddRef();
    }

    m_aCommonUniforms.clear();

    COMMONUNIFORM cu;
    const GShaderImpl::ConstantDescArray& aConstDescTab
      = m_pShaderImpl->GetConstantDescTable();
    for(GShaderImpl::ConstantDescArray::const_iterator it = aConstDescTab.begin();
      it != aConstDescTab.end(); ++it)
    {
      //  if(it->Type == D3DXPT_SAMPLER2D)
      //    continue;
      if( it->Type == D3D_SVT_TEXTURE || it->Type == D3D_SVT_TEXTURE1D ||
        it->Type == D3D_SVT_TEXTURE2D || it->Type == D3D_SVT_TEXTURE3D ||
        it->Type == D3D_SVT_TEXTURECUBE || it->Type == D3D_SVT_SAMPLER)
      {
        ASSERT(0);  // 不确定是不是有这些类型,增加一个断言判断.
      }
      cu.pConstDesc = &*it;
      cu.cbSize     = 0;
      cu.nOffsetOf  = -1;
      cu.nBinderIdx = -1;

      m_aCommonUniforms.push_back(cu);
    }
    return GX_OK;
  }

  GXHRESULT GShaderStubImpl::BindData(MODataPool* pDataPool, GXLPCSTR szStruct)
  {
    return GX_OK;
  }

  GXHRESULT GShaderStubImpl::FindDataPoolByName(GXLPCSTR szName, MODataPool** ppDataPool)
  {
    return GX_OK;
  }

  GXHRESULT GShaderStubImpl::BindCommonUniform(LPCDATALAYOUT lpUniformDef)
  {
    if(FindUniformDef(lpUniformDef) >= 0)
    {
      return GX_OK;
    }
    BINDERSECTDESC bsd;
    bsd.lpDefine = lpUniformDef;
    bsd.nTopIndex = -1;
    m_aBinderSectDesc.push_back(bsd);
    GXSHORT nBinderIdx = (GXSHORT)m_aBinderSectDesc.size() - 1;

    for(CommonUniformArray::iterator it = m_aCommonUniforms.begin();
      it != m_aCommonUniforms.end(); ++it)
    {
      COMMONUNIFORM& cu = *it;
      for(int i = 0;; i++)
      {
        if(lpUniformDef[i].pName == NULL)
          break;
        if(cu.pConstDesc->strName == lpUniformDef[i].pName)
        {
          ASSERT(lpUniformDef[i].uSize == cu.pConstDesc->Size);
          cu.cbSize     = lpUniformDef[i].uSize;
          cu.nOffsetOf  = lpUniformDef[i].uOffset;
          cu.nBinderIdx = nBinderIdx;
          break;
        }
      }
    }
    IntSortByBinder();
    IntGenerateBinderTopIndex();
    return GX_OK;
  }

  GXINT GShaderStubImpl::FindUniformDef(LPCDATALAYOUT lpUniformDef)
  {
    GXINT nIdx = 0;
    for(BinderSectDescArray::iterator it = m_aBinderSectDesc.begin();
      it != m_aBinderSectDesc.end(); ++it, ++nIdx) {
        if(lpUniformDef == it->lpDefine) {
          return nIdx;
        }
    }
    return -1;
  }

  GXBOOL GShaderStubImpl::CommitUniform(int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset)
  {
    // nDefIdx == -1 时 lpData 是对应的 unusual Buffer
    // nDefIdx >= 0 时 lpData 是对应的 Common Struct

    const GXBYTE* pConstBufferBytes = (const GXBYTE*)lpData;
    //LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    if(nDefIdx == -1)
    {
      for(CommonUniformArray::iterator it = m_aCommonUniforms.begin();
        it != m_aCommonUniforms.end(); ++it)
      {
        COMMONUNIFORM& cu = *it;
        if(cu.nBinderIdx != -1) {
          break;
        }
        if((uCommonOffset != -1 && uCommonOffset != cu.nOffsetOf))
          continue;

        const GXUINT uVertex = GXLOWORD(cu.pConstDesc->dwHandle);
        const GXBYTE* pData = (uVertex == 0)
          ? &pConstBufferBytes[cu.pConstDesc->StartOffset + m_pShaderImpl->GetPixelIndexOffset()]
        : &pConstBufferBytes[cu.pConstDesc->StartOffset];
        IntSetUniform(&m_pShaderImpl->GetConstantDescTable().front(),
          cu.pConstDesc->dwHandle, (const float*)pData, 
          cu.pConstDesc->Size >> 2);
      }
    }
    else
    {
      const int nTopIdx = m_aBinderSectDesc[nDefIdx].nTopIndex;
      if(nTopIdx < 0) {
        // 所有uniform均没有绑定
        return FALSE;
      }
      for(CommonUniformArray::iterator it = m_aCommonUniforms.begin() + nTopIdx;
        it != m_aCommonUniforms.end(); ++it)
      {
        COMMONUNIFORM& cu = *it;
        if(cu.nBinderIdx != nDefIdx) {
          break;
        }
        if((uCommonOffset != -1 && uCommonOffset != cu.nOffsetOf))
          continue;
        const GXBYTE* pValue = (((GXLPBYTE)pConstBufferBytes) + cu.nOffsetOf);
        IntSetUniform(&m_pShaderImpl->GetConstantDescTable().front(),
          cu.pConstDesc->dwHandle, (const float*)pValue, cu.pConstDesc->Size >> 2);
      }
    }

    return true;
  }



  GXBOOL GShaderStubImpl::IntIsTextureHandle(GXUINT uHandle)
  {
    //const GXUINT uVertex = GXLOWORD(uHandle);  // 肯定为空
    //const GXUINT uPixel  = GXHIWORD(uHandle);

    //const GXLPCD3DXCONSTDESC lpConstDesc = 
    //  &m_pShaderImpl->GetConstantDescTable().front();

    //return(uVertex == 0 && lpConstDesc[uPixel - 1].RegisterSet == D3DXRS_SAMPLER);
    return FALSE;
  }

  GXBOOL GShaderStubImpl::IntSetUniform(GXLPCCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom)
  {
    // 如果没有绑定的变量,应该提供pUnusualUnifom参数回写到缓冲区中
    // nFloatCount 就是float的数量,不是float4的数量
    const GXUINT uVertex = GXLOWORD(dwHandle);
    const GXUINT uPixel  = GXHIWORD(dwHandle);

    ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXBYTE*       const pData      = (GXBYTE*)pUnusualUnifom;
    GShaderImpl::BufPairArray& aBufferPairs = m_pShaderImpl->GetPairs();
    if(uVertex != NULL)
    {
      const GXCONSTDESC& ShaderConst = lpConstDesc[uVertex - 1];
      ASSERT((ShaderConst.Size >> 2) == nFloatCount);
      //ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2)); 有可能出现, RegisterCount是优化后的数量
      //ASSERT(UniformBuffer.GetSize() > ShaderConst.RegisterIndex * sizeof(float4));
      //  V(pd3dDevice->SetVertexShaderConstantF(
      //    ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));
      GXSDRBUFFERPAIR& Pair = aBufferPairs[ShaderConst.nCBArrayIdx];

      Pair.bNeedUpdate = TRUE;

      if(ShaderConst.Size == sizeof(float4))
        *(float4*)&Pair.pUserBuffer[ShaderConst.StartOffset] = *(float4*)fValue;
      else
        memcpy(&Pair.pUserBuffer[ShaderConst.StartOffset], fValue, ShaderConst.Size);

      if(pUnusualUnifom != NULL)
      {
        if(ShaderConst.Size == sizeof(float4))
          *(float4*)&pData[ShaderConst.StartOffset] = *(float4*)fValue;
        else
          memcpy(&pData[ShaderConst.StartOffset], fValue, ShaderConst.Size);
      }
    }
    if(uPixel != NULL)
    {
      const GXCONSTDESC& ShaderConst = lpConstDesc[uPixel - 1];
      const INT nPixelBufferIndex = m_pShaderImpl->GetPixelIndexOffset() + ShaderConst.StartOffset;
      ASSERT((ShaderConst.Size >> 2) == nFloatCount);
      //ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2));
      //ASSERT(UniformBuffer.GetSize() >= nPixelBufferIndex * sizeof(float4));

      //  V(pd3dDevice->SetPixelShaderConstantF(
      //    ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));
      GXSDRBUFFERPAIR& Pair = aBufferPairs[ShaderConst.nCBArrayIdx];


      Pair.bNeedUpdate = TRUE;

      if(ShaderConst.Size == sizeof(float4))
        *(float4*)&Pair.pUserBuffer[ShaderConst.StartOffset] = *(float4*)fValue;
      else
        memcpy(&Pair.pUserBuffer[ShaderConst.StartOffset], fValue, ShaderConst.Size);

      if(pUnusualUnifom != NULL)
      {
        if(ShaderConst.Size == sizeof(float4))
          *(float4*)&pData[nPixelBufferIndex] = *(float4*)fValue;
        else
          memcpy(&pData[nPixelBufferIndex], fValue, ShaderConst.Size);
      }
    }
    return true;  
  }

  GXBOOL GShaderStubImpl::GetUniformByIndex(GXUINT nIndex, UNIFORMDESC* pDesc) const
  {
    return TRUE;
  }

  GXBOOL GShaderStubImpl::SetTextureByHandle(GTextureBase** pTextureArray, GXUINT uHandle, GTextureBase* pTexture)
  {
    CLBREAK;
    return TRUE;
  }

  GXBOOL GShaderStubImpl::SetTextureByIndex(GTextureBase** pTextureArray, GXUINT nIndex, GTextureBase* pTexture)
  {
    CLBREAK;
    return TRUE;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GShaderStubImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GShaderStubImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    //if(m_uRefCount == 1)
    //{
    //  return m_pGraphicsImpl->OldUnregisterResource(this);
    //}
    //else
    if(nRefCount == 0)
    {
      //OnDeviceEvent(DE_LostDevice);
      SAFE_RELEASE(m_pShaderImpl);
      m_pGraphicsImpl->UnregisterResource(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXHRESULT GShaderStubImpl::SetTextureSlot(GXLPCSTR pName, GXINT nSlot)
  {
    return GX_OK;
  }
  GXINT GShaderStubImpl::GetTextureSlot(GXLPCSTR pName)
  {
    return -1;
  }

  GXBOOL GShaderStubImpl::IntSortByBinder()
  {
    int nCount = (int)m_aCommonUniforms.size();
    COMMONUNIFORM* lpCommUniform = &m_aCommonUniforms.front();

    // 冒泡排序
    BubbleSort(lpCommUniform, nCount);
    return TRUE;
  }

#ifdef REFACTOR_SHADER
  GXBOOL GShaderStubImpl::UpdateCanvasUniform(GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXSIZE_T cbSize)
  {
    CLBREAK;
    return FALSE;
  }
  GXBOOL GShaderStubImpl::UpdateUniform(int nDefIdx, GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXSIZE_T cbSize)
  {
    CLBREAK;
    return FALSE;
  }
#endif // #ifdef REFACTOR_SHADER

  GXBOOL GShaderStubImpl::IntGenerateBinderTopIndex()
  {
    GXINT n = -1;
    GXINT nIdx = 0;
    for(CommonUniformArray::iterator it = m_aCommonUniforms.begin();
      it != m_aCommonUniforms.end(); ++it, ++nIdx)
    {
      if(n != it->nBinderIdx)
      {
        if(it->nBinderIdx >= 0)
        {
          m_aBinderSectDesc[it->nBinderIdx].nTopIndex = nIdx;
        }
        n = it->nBinderIdx;
      }
    }
    return TRUE;
  }

  //
  // GShaderStubImpl::COMMONUNIFORM 函数
  //
  GXBOOL GShaderStubImpl::COMMONUNIFORM::SortCompare(COMMONUNIFORM& Stru)
  {
    return nBinderIdx > Stru.nBinderIdx;
  }
  GXVOID GShaderStubImpl::COMMONUNIFORM::SortSwap(COMMONUNIFORM& Stru)
  {
    COMMONUNIFORM temp = *this;
    *this = Stru;
    Stru = temp;
  }
}

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
