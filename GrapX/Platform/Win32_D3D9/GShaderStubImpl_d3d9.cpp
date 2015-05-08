#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GXGraphics.H"
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
#include "Platform/Win32_D3D9/GShaderStubImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"


// 私有头文件
//#include <Driver/Shader/VertexDecl.H>
#include "Canvas/GXResourceMgr.h"
//#include "include/GXCanvas3D.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
//#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.H"
//#include "clstd/clPathFile.H"
//#include "Smart/smartstream.h"
//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)

using namespace clstd;

namespace D3D9
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
    BinderSectDescArray aBinder;
    // 第二次调用时, 如果pShader为NULL则表示更新链接表
    if(m_pShaderImpl == NULL && pShader == NULL)
      return GX_FAIL;

    else if(pShader != NULL && m_pShaderImpl != pShader)
    {
      SAFE_RELEASE(m_pShaderImpl);
      m_pShaderImpl = (GShaderImpl*)pShader;
      m_pShaderImpl->AddRef();
    }
    else
    {
      ASSERT(m_pShaderImpl != NULL && pShader == NULL);
      aBinder = m_aBinderSectDesc;
    }

    m_aBinderSectDesc.clear();
    m_aCommonUniforms.clear();
    COMMONUNIFORM cu;
    const GShaderImpl::ConstantDescArray& aConstDescTab
      = m_pShaderImpl->GetConstantDescTable();
    for(GShaderImpl::ConstantDescArray::const_iterator it = aConstDescTab.begin();
      it != aConstDescTab.end(); ++it)
    {
      if(it->Type == D3DXPT_SAMPLER2D)
        continue;
      cu.pConstDesc = &*it;
      cu.cbSize     = 0;
      cu.nOffsetOf  = -1;
      cu.nBinderIdx = -1;

      m_aCommonUniforms.push_back(cu);
    }

    if(aBinder.size() != 0)
    {
      for(BinderSectDescArray::iterator it = aBinder.begin();
        it != aBinder.end(); ++it)
      {
        if(it->Var.IsValid())
        {
          MODataPool* pDataPool = NULL;
          it->Var.GetPool(&pDataPool);
          BindData(pDataPool, NULL);
          SAFE_RELEASE(pDataPool);
        }
        else
        {
          ASSERT(it->lpDefine != NULL);
          BindCommonUniform(it->lpDefine);
        }
      }
    }
    return GX_OK;
  }

  GXINT GShaderStubImpl::IntGetDataPool(MODataPool* pDataPool)
  {
    GXINT nIdx = 0;
    for(BinderSectDescArray::iterator it = m_aBinderSectDesc.begin();
      it != m_aBinderSectDesc.end(); ++it, ++nIdx) {
        if(it->Var.IsSamePool(pDataPool)) {
          return nIdx;
        }
    }
    return -1;
  }

  GXHRESULT GShaderStubImpl::BindData(MODataPool* pDataPool, GXLPCSTR szStruct)
  {
    if(pDataPool == NULL) {
      return GX_FAIL;
    }
    else if(IntGetDataPool(pDataPool) >= 0) {
      return GX_OK;
    }
    //--------------------------
    ASSERT(szStruct == NULL); // Shader Reload 时无法记录这个, 还没想好怎么解决, 这个断言是备忘
    // 参考 SetShaderRef
    //--------------------------

    //DataLayoutArray aDataLayout;
    //pDataPool->GetLayout(szStruct, &aDataLayout);

    BINDERSECTDESC bsd;
    bsd.lpDefine = NULL;

    if(szStruct != NULL)
    {
      CLBREAK; // 这个可能还要修改,所以断在这个地方
      //bsd.StructName = szStruct;
      pDataPool->QueryByExpression(szStruct, &bsd.Var);

      if( ! bsd.Var.IsValid())
      {
        // 无效的Struct Name
        return GX_FAIL;
      }
    }
    else
    {
      //
      // 如果没有指定 szStruct, 则把第一个变量的名字记下来, 这是为了得到 DataPool 的开始地址
      //
      pDataPool->QueryByName(pDataPool->GetVariableName(0), &bsd.Var);
      ASSERT(bsd.Var.IsValid());
    }
    // szStruct 如果不为空要判断是结构体的类型名字还是结构体的变量名字
    //pDataPool->GetType(szStruct); == sturctname? variable?

    //bsd.pDataPool = pDataPool;
    bsd.nTopIndex = -1;
    m_aBinderSectDesc.push_back(bsd);
    //pDataPool->AddRef();

    GXSHORT nBinderIdx = (GXSHORT)m_aBinderSectDesc.size() - 1;

    for(CommonUniformArray::iterator it = m_aCommonUniforms.begin();
      it != m_aCommonUniforms.end(); ++it)
    {
      COMMONUNIFORM& cu = *it;
      MOVariable Var;
      if(cu.nBinderIdx == -1 && pDataPool->QueryByName(cu.pConstDesc->Name, &Var))
      {
        ASSERT(Var.GetSize() == cu.pConstDesc->Bytes);
        cu.cbSize     = cu.pConstDesc->Bytes;
        cu.nOffsetOf  = Var.GetOffset();
        cu.nBinderIdx = nBinderIdx;
      }
    }
    IntSortByBinder();
    IntGenerateBinderTopIndex();    
    return GX_OK;
  }

  GXHRESULT GShaderStubImpl::FindDataPoolByName(GXLPCSTR szName, MODataPool** ppDataPool)
  {
    return GX_OK;
  }

  GXHRESULT GShaderStubImpl::BindCommonUniform(LPCDATALAYOUT lpUniformDef)
  {
    if(FindUniformDef(lpUniformDef) >= 0) {
      return GX_OK;
    }
    BINDERSECTDESC bsd;
    //InlSetZeroT(bsd);
    bsd.lpDefine  = lpUniformDef;
    bsd.nTopIndex = -1;
    //bsd.pDataPool = NULL;
    m_aBinderSectDesc.push_back(bsd);
    GXSHORT nBinderIdx = (GXSHORT)m_aBinderSectDesc.size() - 1;

    for(CommonUniformArray::iterator it = m_aCommonUniforms.begin();
      it != m_aCommonUniforms.end(); ++it)
    {
      COMMONUNIFORM& cu = *it;
      if(cu.nBinderIdx != -1) { continue; }
      for(int i = 0;; i++)
      {
        if(lpUniformDef[i].pName == NULL)
          break;
        if(strcmp(lpUniformDef[i].pName, cu.pConstDesc->Name) == 0)
        {
          ASSERT(lpUniformDef[i].uSize == cu.pConstDesc->Bytes);
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

#ifdef REFACTOR_SHADER
  GXBOOL GShaderStubImpl::UpdateCanvasUniform(GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXUINT cbSize)
  {
    // Canvas-uniform 提交
    GShaderImpl::ConstDescArray& aConstDesc = m_pShaderImpl->m_aConstDesc;
    for(GShaderImpl::ConstDescArray::const_iterator it = aConstDesc.begin();
      it != aConstDesc.end(); ++it)
    {
      if(it->nCanvasUniform == -1) {
        continue;
      }
      ASSERT(it->nCanvasUniform > 0); // 这个值避免了0这个值
      const float* fValue = (const float*)(lpCanvasUniform + it->nCanvasUniform);

      // nFloatCount 应该取 cu.pConstDesc->Bytes >> 2 和 nCanvasUniform 的最小值
      IntUpdateUniform(&aConstDesc.front(), it->dwHandle, fValue, it->Bytes >> 2, (float4*)lpUniform);
    }
    return TRUE;
  }

  GXBOOL GShaderStubImpl::UpdateUniform(int nDefIdx, GXLPCBYTE lpCanvasUniform/*, const float4* pSharedTable*/, GXLPVOID lpUniform, GXUINT cbSize)
  {
    if(nDefIdx == -2) // 绑定的Uniform
    {
      //ASSERT(lpUniform == NULL/* && uCommonOffset == -1*/);
      int nIndex = 0;
      for(BinderSectDescArray::iterator itBinder = m_aBinderSectDesc.begin();
        itBinder != m_aBinderSectDesc.end(); ++itBinder, ++nIndex)
      {
        if( ! itBinder->Var.IsValid()) {
          continue;
        }
        UpdateUniform(nIndex, (GXLPCBYTE)itBinder->Var.GetPtr(), lpUniform, cbSize);
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

        const float* fValue = (float*)(lpCanvasUniform + cu.nOffsetOf); // 这个 lpCanvasUniform 已经不是CanvasUniform了
        IntUpdateUniform(&m_pShaderImpl->GetConstantDescTable().front(),
          cu.pConstDesc->dwHandle, fValue, cu.pConstDesc->Bytes >> 2, (float4*)lpUniform);
      }
    }
    return TRUE;
  }
#else
#endif // #ifdef REFACTOR_SHADER

  GXBOOL GShaderStubImpl::CommitUniform(int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset)
  {
    // nDefIdx == -2 时 lpData 是对应的 bind datapool
    // nDefIdx == -1 时 lpData 是对应的 unusual Buffer
    // nDefIdx >= 0 时 lpData 是对应的 Common Struct

    const float4* pConstBuffer = (const float4*)lpData;
    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();

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
        const float4* fpData = (uVertex == 0)
          ? &pConstBuffer[cu.pConstDesc->RegisterIndex + m_pShaderImpl->GetPixelIndexOffset()]
          : &pConstBuffer[cu.pConstDesc->RegisterIndex];
        IntSetUniform(&m_pShaderImpl->GetConstantDescTable().front(),
          cu.pConstDesc->dwHandle, (const float*)fpData, cu.pConstDesc->Bytes >> 2);
      }
    }
    else if(nDefIdx == -2)
    {
      ASSERT(lpData == NULL && uCommonOffset == -1);
      int nIndex = 0;
      for(BinderSectDescArray::iterator itBinder = m_aBinderSectDesc.begin();
        itBinder != m_aBinderSectDesc.end(); ++itBinder, ++nIndex)
      {
        if( ! itBinder->Var.IsValid()) {
          continue;
        }
        CommitUniform(nIndex, itBinder->Var.GetPtr(), -1);
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
        const float* fValue = (float*)(((GXLPBYTE)pConstBuffer) + cu.nOffsetOf);
//#ifdef REFACTOR_SHADER
//        IntUpdateUniform()
//#else
        IntSetUniform(&m_pShaderImpl->GetConstantDescTable().front(),
          cu.pConstDesc->dwHandle, fValue, cu.pConstDesc->Bytes >> 2);
//#endif // #ifdef REFACTOR_SHADER
      }
    }

    return true;
  }

  GXBOOL GShaderStubImpl::IntIsTextureHandle(GXUINT uHandle)
  {
    const GXUINT uVertex = GXLOWORD(uHandle);  // 肯定为空
    const GXUINT uPixel  = GXHIWORD(uHandle);

    const GXLPCD3DXCONSTDESC lpConstDesc = 
      &m_pShaderImpl->GetConstantDescTable().front();

    return(uVertex == 0 && lpConstDesc[uPixel - 1].RegisterSet == D3DXRS_SAMPLER);
  }

#ifdef REFACTOR_SHADER
  GXBOOL GShaderStubImpl::IntUpdateUniform(const GXLPCD3DXCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom)
  {
    // 如果没有绑定的变量,应该提供pUnusualUnifom参数回写到缓冲区中
    // nFloatCount 就是float的数量,不是float4的数量
    const GXUINT uVertex = GET_VSREGISTER_IDX(dwHandle);
    const GXUINT uPixel  = GET_PSREGISTER_IDX(dwHandle);

    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    if(uVertex != NULL)
    {
      const GXD3DXCONSTDESC& ShaderConst = lpConstDesc[uVertex - 1];
      ASSERT((ShaderConst.Bytes >> 2) == nFloatCount);
      //ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2)); 有可能出现, RegisterCount是优化后的数量
      //ASSERT(UniformBuffer.GetSize() > ShaderConst.RegisterIndex * sizeof(float4));
      //V(pd3dDevice->SetVertexShaderConstantF(
      //  ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));

      if(ShaderConst.RegisterCount == 1) {
        pUnusualUnifom[ShaderConst.RegisterIndex] = *(float4*)fValue;
      }
      else {
        //memcpy(&pUnusualUnifom[ShaderConst.RegisterIndex], 
        //fValue, ShaderConst.RegisterCount * sizeof(float4));
        for(UINT i = 0; i < ShaderConst.RegisterCount; ++i) {
          pUnusualUnifom[ShaderConst.RegisterIndex + i] = ((float4*)fValue)[i];
        }
      }
    }

    if(uPixel != NULL)
    {
      const GXD3DXCONSTDESC& ShaderConst = lpConstDesc[uPixel - 1];
      GXCONST INT nPixelBufferIndex = m_pShaderImpl->GetPixelIndexOffset() + ShaderConst.RegisterIndex;
      ASSERT((ShaderConst.Bytes >> 2) == nFloatCount);
      ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2));
      //ASSERT(UniformBuffer.GetSize() >= nPixelBufferIndex * sizeof(float4));

      //V(pd3dDevice->SetPixelShaderConstantF(
      //  ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));

      if(ShaderConst.RegisterCount == 1) {
        pUnusualUnifom[nPixelBufferIndex] = *(float4*)fValue;
      }
      else {
        //memcpy(&pUnusualUnifom[nPixelBufferIndex], 
        //fValue, ShaderConst.RegisterCount * sizeof(float4));
        for(UINT i = 0; i < ShaderConst.RegisterCount; ++i) {
          pUnusualUnifom[nPixelBufferIndex + i] = ((float4*)fValue)[i];
        }
      }
    }
    return TRUE;
  }
#else
#endif
  GXBOOL GShaderStubImpl::IntSetUniform(const GXLPCD3DXCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom)
  {
    // 如果没有绑定的变量,应该提供pUnusualUnifom参数回写到缓冲区中
    // nFloatCount 就是float的数量,不是float4的数量
    const GXUINT uVertex = GET_VSREGISTER_IDX(dwHandle);
    const GXUINT uPixel  = GET_PSREGISTER_IDX(dwHandle);

    LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    if(uVertex != NULL)
    {
      const GXD3DXCONSTDESC& ShaderConst = lpConstDesc[uVertex - 1];
      ASSERT((ShaderConst.Bytes >> 2) == nFloatCount);
      //ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2)); 有可能出现, RegisterCount是优化后的数量
      //ASSERT(UniformBuffer.GetSize() > ShaderConst.RegisterIndex * sizeof(float4));
      V(pd3dDevice->SetVertexShaderConstantF(
        ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));

      if(pUnusualUnifom != NULL)
      {
        if(ShaderConst.RegisterCount == 1)
          pUnusualUnifom[ShaderConst.RegisterIndex] = *(float4*)fValue;
        else
          memcpy(&pUnusualUnifom[ShaderConst.RegisterIndex], 
          fValue, ShaderConst.RegisterCount * sizeof(float4));
      }
    }
    if(uPixel != NULL)
    {
      const GXD3DXCONSTDESC& ShaderConst = lpConstDesc[uPixel - 1];
      GXCONST INT nPixelBufferIndex = m_pShaderImpl->GetPixelIndexOffset() + ShaderConst.RegisterIndex;
      ASSERT((ShaderConst.Bytes >> 2) == nFloatCount);
      ASSERT(ShaderConst.RegisterCount == ((nFloatCount + 3) >> 2));
      //ASSERT(UniformBuffer.GetSize() >= nPixelBufferIndex * sizeof(float4));

      V(pd3dDevice->SetPixelShaderConstantF(
        ShaderConst.RegisterIndex, fValue, ShaderConst.RegisterCount));

      if(pUnusualUnifom != NULL)
      {
        if(ShaderConst.RegisterCount == 1)
          pUnusualUnifom[nPixelBufferIndex] = *(float4*)fValue;
        else
          memcpy(&pUnusualUnifom[nPixelBufferIndex], 
          fValue, ShaderConst.RegisterCount * sizeof(float4));
      }
    }
    return true;  
  }

  GXBOOL GShaderStubImpl::GetUniformByIndex(GXUINT nIndex, UNIFORMDESC* pDesc) GXCONST
  {
    if(nIndex >= m_aCommonUniforms.size()) {
      return FALSE;
    }
    const COMMONUNIFORM& uniform = m_aCommonUniforms[nIndex];
    pDesc->Name     = uniform.pConstDesc->Name;
    pDesc->eType    = GetHandleType(uniform.pConstDesc->dwHandle);
    pDesc->nOffset  = uniform.pConstDesc->RegisterIndex * sizeof(float4);
    pDesc->cbCount  = uniform.cbSize;
    pDesc->nBindIdx = uniform.nBinderIdx;
    return TRUE;
  }

  //GXBOOL GShaderStubImpl::SetUniformByHandle(clBuffer* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount)
  //{
  //  // 应该验证是非绑定变量
  //  return SetUniform(
  //    &(m_pShaderImpl->GetConstantDescTable().front()),
  //    uHandle, fValue, nFloatCount, pUnusualUnifom ? (float4*)pUnusualUnifom->GetPtr() : NULL);
  //}

  GXBOOL GShaderStubImpl::SetTextureByHandle(GTextureBase** pTextureArray, GXUINT uHandle, GTextureBase* pTexture)
  {
    if( ! IntIsTextureHandle(uHandle)) {
      return FALSE;
    }

    const GXUINT uPixel = GXHIWORD(uHandle);
    const GXLPCD3DXCONSTDESC lpConstDesc = 
      &m_pShaderImpl->GetConstantDescTable().front();

    GTextureBase*& pTexRef = pTextureArray[lpConstDesc[uPixel - 1].RegisterIndex];
    GXHRESULT hval = InlSetNewObjectT(pTexRef, pTexture);
    //SAFE_RELEASE(pTexRef);
    //pTexRef = pTexture;
    //pTexRef->AddRef();
    return GXSUCCEEDED(hval);
  }

  GXBOOL GShaderStubImpl::SetTextureByIndex(GTextureBase** pTextureArray, GXUINT uIndex, GTextureBase* pTexture)
  {
    if(uIndex >= GX_MAX_TEXTURE_STAGE) {
      return FALSE;
    }

    GTextureBase*& pTexRef = pTextureArray[uIndex];
    GXHRESULT hval = InlSetNewObjectT(pTexRef, pTexture);
    return GXSUCCEEDED(hval);
  }

  GXHRESULT GShaderStubImpl::AddRef()
  {
    const GXLONG nRef = gxInterlockedIncrement(&m_nRefCount);
    if(nRef == 1) {
      m_pGraphicsImpl->RegisterResource(this, NULL);
    }
    return nRef;
  }

  GXHRESULT GShaderStubImpl::Release()
  {
    const GXLONG nRef = gxInterlockedDecrement(&m_nRefCount);
    if(nRef == 0)
    {
      SAFE_RELEASE(m_pShaderImpl);
      //for(BinderSectDescArray::iterator it = m_aBinderSectDesc.begin();
      //  it != m_aBinderSectDesc.end(); ++it)
      //{
      //  SAFE_RELEASE(it->pDataPool);
      //}
      m_pGraphicsImpl->UnregisterResource(this);
      delete this;
      return GX_OK;
    }
    return nRef;
  }

  GXHRESULT GShaderStubImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    if(pDesc->szCmdString != NULL)
    {
      // $.这里不处理"reloadshader"命令, 有上层的Effect或者Material来刷新Stub的状态
    }
    return GX_OK;
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
    const int nCount = (int)m_aCommonUniforms.size();
    if(nCount > 0)
    {
      COMMONUNIFORM* lpCommUniform = &m_aCommonUniforms.front();
      BubbleSort(lpCommUniform, nCount); // 冒泡排序
    }
    return TRUE;
  }

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

  //////////////////////////////////////////////////////////////////////////
  void ConvertVertexDeclToNative(LPCGXVERTEXELEMENT pVerticesDecl, clBuffer** ppBuffer)
  {
    int nCount = GetVertexDeclLength<int>(pVerticesDecl);
    clBuffer* pBuffer = new clBuffer(sizeof(D3DVERTEXELEMENT9));
    pBuffer->Resize(sizeof(D3DVERTEXELEMENT9) * (nCount + 1), FALSE);

    LPD3DVERTEXELEMENT9 pd3dve = (LPD3DVERTEXELEMENT9)pBuffer->GetPtr();
    for(int i = 0; i < nCount; i++)
    {
      pd3dve->Stream     = 0;
      pd3dve->Offset     = pVerticesDecl[i].Offset;
      pd3dve->Type       = pVerticesDecl[i].Type;
      pd3dve->Method     = pVerticesDecl[i].Method;
      pd3dve->Usage      = pVerticesDecl[i].Usage;
      pd3dve->UsageIndex = pVerticesDecl[i].UsageIndex;
      pd3dve++;
    }
    const D3DVERTEXELEMENT9 d3dDeclEnd = D3DDECL_END();
    *pd3dve = d3dDeclEnd;
    *ppBuffer = pBuffer;
  }
} // namespace D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)