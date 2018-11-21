#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_

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
#include "Platform/Win32_D3D11/GVertexDeclImpl_D3D11.h"


// 私有头文件
#include <GrapX/VertexDecl.h>
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataPoolIterator.h"
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
//#include "Platform/Win32_D3D11/GXCanvasImpl_D3D11.h"
#include "clPathFile.h"
#include "Platform/Win32_D3D11/GShaderImpl_D3D11.h"
#include "clStringSet.h"

//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)
#define PS_HANDLE_SHIFT 16
namespace GrapX
{
  namespace D3D11
  {
    //////////////////////////////////////////////////////////////////////////
#define DEFINE_TYPE_LIST(_NAME) \
    _NAME, _NAME"2", _NAME"3", _NAME"4",  \
    _NAME"1x1", _NAME"1x2", _NAME"1x3", _NAME"1x4",\
    _NAME"2x1", _NAME"2x2", _NAME"2x3", _NAME"2x4",\
    _NAME"3x1", _NAME"3x2", _NAME"3x3", _NAME"3x4",\
    _NAME"4x1", _NAME"4x2", _NAME"4x3", _NAME"4x4",\
    "row_"_NAME"1x1", "row_"_NAME"1x2", "row_"_NAME"1x3", "row_"_NAME"1x4", \
    "row_"_NAME"2x1", "row_"_NAME"2x2", "row_"_NAME"2x3", "row_"_NAME"2x4", \
    "row_"_NAME"3x1", "row_"_NAME"3x2", "row_"_NAME"3x3", "row_"_NAME"3x4", \
    "row_"_NAME"4x1", "row_"_NAME"4x2", "row_"_NAME"4x3", "row_"_NAME"4x4",

    static char* s_szTypeName[][36]{
      {DEFINE_TYPE_LIST("float")}, // 0
      {DEFINE_TYPE_LIST("int")}, // 1
      {DEFINE_TYPE_LIST("bool")}, // 2
      {DEFINE_TYPE_LIST("uint")}, // 3
    };
#undef DEFINE_TYPE_LIST

  //////////////////////////////////////////////////////////////////////////

#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/D3D_ShaderImpl.inl"
#include "Platform/CommonInline/X_ShaderImpl.inl"

  //////////////////////////////////////////////////////////////////////////
    STATIC_ASSERT(D3D_INCLUDE_LOCAL == c_D3D_INCLUDE_LOCAL);
    STATIC_ASSERT(D3D_INCLUDE_SYSTEM == c_D3D_INCLUDE_SYSTEM);
#ifdef REFACTOR_GRAPX_SHADER
    //////////////////////////////////////////////////////////////////////////
    GShaderImpl::GShaderImpl(Graphics* pGraphics)
      : GShader         ()
      , m_pGraphicsImpl  ((GraphicsImpl*)pGraphics)
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

      // Const Buffer 循环
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

            cd.strName = SdrVarDesc.Name;
            cd.dwNameID = cd.strName.GetHash();
            cd.nConstBuf = ncb;
            cd.nCBArrayIdx = nCBArrayIdx;
            cd.dwHandle = (i + dwTopIndex) << uHandleShift;
            cd.StartOffset = SdrVarDesc.StartOffset;    // Offset in constant buffer's backing store
            cd.Size = SdrVarDesc.Size;           // Size of variable (in bytes)
            cd.uFlags = SdrVarDesc.uFlags;         // Variable flags
            cd.StartTexture = SdrVarDesc.StartTexture;   // First texture index (or -1 if no textures used)
            cd.TextureSize = SdrVarDesc.TextureSize;    // Number of texture slots possibly used.
            cd.StartSampler = SdrVarDesc.StartSampler;   // First sampler index (or -1 if no textures used)
            cd.SamplerSize = SdrVarDesc.SamplerSize;    // Number of sampler slots possibly used.
            cd.Class = SdrTypeDesc.Class;          // Variable class (e.g. object, matrix, etc.)
            cd.Type = SdrTypeDesc.Type;           // Variable type (e.g. float, sampler, etc.)
            cd.Rows = SdrTypeDesc.Rows;           // Number of rows (for matrices, 1 for other numeric, 0 if not applicable)
            cd.Columns = SdrTypeDesc.Columns;        // Number of columns (for vectors & matrices, 1 for other numeric, 0 if not applicable)
            cd.Elements = SdrTypeDesc.Elements;       // Number of elements (0 if not an array)
            cd.Members = SdrTypeDesc.Members;        // Number of members (0 if not a structure)
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

      // 清理旧的对象
      CleanUp();


      if(pVertexBuf == NULL || pPixelBuf == NULL) {
        return GX_FAIL;
      }

      hval = pd3dDevice->CreateVertexShader(pVertexBuf->GetPtr(), pVertexBuf->GetSize(), NULL, &m_pVertexShader);
      if(SUCCEEDED(hval))
      {
        hval = D3DReflect(pVertexBuf->GetPtr(), pVertexBuf->GetSize(),
          IID_ID3D11ShaderReflection, (void**)&m_pvct);

        if(SUCCEEDED(hval)) {
          m_dwFlag |= GXSHADERCAP_VERTEX;

          // 这里复制一份 VertexBuffer, 注意不是 VertexShader
          ASSERT(m_pVertexBuf == NULL);
          m_pVertexBuf = new clBuffer;
          m_pVertexBuf->Append(pVertexBuf->GetPtr(), pVertexBuf->GetSize());
        }
      }

      hval = pd3dDevice->CreatePixelShader(pPixelBuf->GetPtr(), pPixelBuf->GetSize(), NULL, &m_pPixelShader);
      if(SUCCEEDED(hval))
      {
        hval = D3DReflect(pPixelBuf->GetPtr(), pPixelBuf->GetSize(),
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

      // D3DCompile
      HRESULT hval = D3DCompile((LPCSTR)pBuffer->GetPtr(), pBuffer->GetSize(),
        szShaderFile, (D3D10_SHADER_MACRO*)pMacros, pInclude, szFunctionName, szProfile, dwShaderFlags, 0, &pShader, &pErrorBlob);

      if(FAILED(hval))
      {
        if(pErrorBlob != NULL) {
          TRACE("Shader compiled error:\n>%s\n", (char*)pErrorBlob->GetBufferPointer());
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

      bd.Usage = D3D11_USAGE_DEFAULT;
      bd.ByteWidth = cbSize;
      bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      bd.CPUAccessFlags = 0;
      HRESULT hr = pd3dDevice->CreateBuffer(&bd, NULL, &Pair.pD3D11ResBufer);
      if(FAILED(hr))
      {
        ASSERT(0);
        return -1;
      }
      Pair.nIdx = nIdx;
      Pair.pUserBuffer = new GXBYTE[cbSize];
      Pair.BufferSize = cbSize;
      Pair.bNeedUpdate = FALSE;
      Pair.bPS = bPS;

      m_aBufPairs.push_back(Pair);
      return (UINT)m_aBufPairs.size() - 1;
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
        //OnDeviceEvent(DE_LostDevice);

        if(m_pVertexShader && m_pPixelShader)
        {
          m_pGraphicsImpl->UnregisterResource(this);
        }
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT GShaderImpl::Activate()
    {
      ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
      pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
      pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
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
          pImmediateContext->UpdateSubresource(it->pD3D11ResBufer, 0, NULL, it->pUserBuffer, 0, 0);
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
      ASSERT(0); // FIXME: 没实现
      //m_aConstDesc[handle & 0xffff].Class
      return GXUB_UNDEFINED;
    }

    GXUINT GShaderImpl::GetStageByHandle(GXUINT handle) const
    {
      CLBREAK; // FIXME: 没实现
      return 0;
    }

    void GShaderImpl::PutInResourceMgr()
    {
      SET_FLAG(m_dwFlag, ShaderFlag_PutInResourceManager);
    }

#ifdef REFACTOR_SHADER
    GXBOOL GShaderImpl::CommitToDevice(GXLPVOID lpUniform, GXUINT cbSize)
    {
      CLBREAK;
      return FALSE;
    }
#endif // #ifdef REFACTOR_SHADER
#endif // #ifdef REFACTOR_GRAPX_SHADER
    //////////////////////////////////////////////////////////////////////////
  } // namespace D3D11
} // namespace GrapX

#define STRUCT_PREFIX_NAME "st_noname_"
#define CB_PREFIX_NAME "cb_"
namespace GrapX
{
  namespace D3D11
  {
    struct DATAPOOL_MAPPER
    {
      // TODO: 没认真解决结构体成员是结构体的问题
      typedef clmap<clStringA, int> CBNameDict;
      typedef clset<clStringA> CBNameSet;

      DataPoolDeclaration_T     aGlobal;            // 全局常量
      DataPoolTypeDefinition_T  aConstantBuffers;   // 常量组，等价于DataPool的结构体
      DataPoolDeclaration_T     aCBMembers;         // 常量组的成员列表
      DataPoolTypeDefinition_T  aTypes;             // 自定义类型，一般就是结构体
      DataPoolDeclaration_T     aMembers;           // 结构体成员列表
      clstd::StringSetA         Strings;

      const Marimo::DATAPOOL_DECLARATION* FindStructureByName(GXLPCSTR szName, size_t* pCount) const
      {
        for(auto it = aTypes.begin(); it != aTypes.end(); ++it)
        {
          if(clstd::strcmpiT(it->Name, szName) == 0) {
            size_t nMemberBegin = reinterpret_cast<size_t>(it->as.Struct);
            ++it;
            if(it == aTypes.end()) {
              *pCount = aMembers.size() - nMemberBegin - 1;
            }
            else {
              *pCount = reinterpret_cast<size_t>(it->as.Struct) - nMemberBegin - 1;
            }
            return &aMembers[nMemberBegin];
          }
        }

        CLBREAK; // 找不到说明表格有问题
        return NULL;
      }

      const Marimo::DATAPOOL_DECLARATION* GetConstantBufferListByIndex(size_t index, size_t* pCount) const
      {
        size_t nBegin = reinterpret_cast<size_t>(aConstantBuffers[index].as.Struct);
        *pCount = (index + 1 == aConstantBuffers.size())
          ? aCBMembers.size() - nBegin
          : reinterpret_cast<size_t>(aConstantBuffers[index + 1].as.Struct) - nBegin;

        return &aCBMembers[nBegin];
      }

      GXVOID GenerateNameDict(CBNameSet& sSet, CBNameDict& sDict) const
      {
        for(auto it = aConstantBuffers.begin(); it != aConstantBuffers.end(); ++it)
        {
          sSet.insert(it->Name);
          sDict.insert(clmake_pair(it->Name, it - aConstantBuffers.begin()));
        }
      }

      GXVOID Copy(DataPoolDeclaration_T& rDest, const Marimo::DATAPOOL_DECLARATION* pSrc, size_t nCount, const DATAPOOL_MAPPER* pSrcMapper)
      {
        ASSERT(&rDest == &aGlobal || &rDest == &aCBMembers || &rDest == &aMembers);
        clStringA str;
        Marimo::DATAPOOL_DECLARATION sEmpty = { NULL };

        for(size_t i = 0; i < nCount; i++)
        {
          rDest.push_back(pSrc[i]);
          if(pSrc[i].Type == NULL) {
            continue;
          }
          else if(clstd::strncmpT(pSrc[i].Type, STRUCT_PREFIX_NAME, sizeof(STRUCT_PREFIX_NAME) - 1) != 0)
          {
            // 确保是内置类型
            ASSERT(clstd::strncmpT(pSrc[i].Type, "int", 3) == 0 || clstd::strncmpT(pSrc[i].Type, "uint", 4) == 0 ||
              clstd::strncmpT(pSrc[i].Type, "bool", 4) == 0 || clstd::strncmpT(pSrc[i].Type, "float", 5) == 0 ||
              clstd::strncmpT(pSrc[i].Type, "row_int", 7) == 0 || clstd::strncmpT(pSrc[i].Type, "row_uint", 8) == 0 ||
              clstd::strncmpT(pSrc[i].Type, "row_bool", 8) == 0 || clstd::strncmpT(pSrc[i].Type, "row_float", 9) == 0);
            rDest.back().Name = Strings.add(pSrc[i].Name);
            continue;
          }
          else
          {
            rDest.back().Name = Strings.add(pSrc[i].Name);
          }
          
          size_t nMemberCount;
          Marimo::DATAPOOL_TYPE_DEFINITION sTypeDef = { Marimo::DataPoolTypeClass::Structure };
          const Marimo::DATAPOOL_DECLARATION* pMemberDeclList = pSrcMapper->FindStructureByName(pSrc[i].Type, &nMemberCount);
          ASSERT(pMemberDeclList != NULL && nMemberCount != 0);

          // 先拷贝，如果内部含新的结构体定义则先拷贝
          Copy(aMembers, pMemberDeclList, nMemberCount, this);
          aMembers.push_back(sEmpty); // 结尾

          str.Format(STRUCT_PREFIX_NAME"%u", aTypes.size());
          sTypeDef.Name = Strings.add(str);
          sTypeDef.as.Struct = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(aMembers.size() - nMemberCount - 1);
          sTypeDef.Cate = Marimo::DataPoolTypeClass::Structure;
          aTypes.push_back(sTypeDef);

          rDest.back().Type = sTypeDef.Name; // 换用新名字
        }
      }
    };


    GXBOOL CompareVariableDeclarationArray(
      const Marimo::DATAPOOL_DECLARATION* a, size_t count_a,
      const Marimo::DATAPOOL_DECLARATION* b, size_t count_b,
      const DATAPOOL_MAPPER& mapper_a, const DATAPOOL_MAPPER& mapper_b)
    {
      if(count_a != count_b) {
        return FALSE;
      }

      for(size_t i = 0; i < count_a; i++)
      {
        if(clstd::strcmpT(a[i].Name, b[i].Name) != 0 || a[i].Count != b[i].Count) {
          return FALSE;
        }
        else if(a[i].Type == b[i].Type) { // 来自s_szTypeName常量表的可以这么比较！
          continue;
        }

        size_t nMemberCountA;
        size_t nMemberCountB;
        const Marimo::DATAPOOL_DECLARATION* a_members = mapper_a.FindStructureByName(a[i].Type, &nMemberCountA);
        const Marimo::DATAPOOL_DECLARATION* b_members = mapper_b.FindStructureByName(b[i].Type, &nMemberCountB);
        if(CompareVariableDeclarationArray(a_members, nMemberCountA, b_members, nMemberCountB, mapper_a, mapper_b) == FALSE) {
          return FALSE;
        }
      }
      return TRUE;
    }

    GXBOOL MergeDataPoolMapped(DATAPOOL_MAPPER& dest, const DATAPOOL_MAPPER& mapper_a, const DATAPOOL_MAPPER& mapper_b)
    {
      dest.aGlobal.reserve(clMax(mapper_a.aGlobal.size(), mapper_b.aGlobal.size()));
      dest.aConstantBuffers.reserve(mapper_a.aConstantBuffers.size() + mapper_b.aConstantBuffers.size());
      dest.aCBMembers.reserve(mapper_a.aCBMembers.size() + mapper_b.aCBMembers.size());
      dest.aTypes.reserve(mapper_a.aTypes.size() + mapper_b.aTypes.size());
      dest.aMembers.reserve(mapper_a.aMembers.size() + mapper_b.aMembers.size());

      const DataPoolDeclaration_T& a = mapper_a.aGlobal;
      const DataPoolDeclaration_T& b = mapper_b.aGlobal;

      // $Globals 必须完全一致
      if(_CL_NOT_(mapper_a.aGlobal.empty()) && _CL_NOT_(mapper_b.aGlobal.empty()))
      {
        if(CompareVariableDeclarationArray(
          &mapper_a.aGlobal.front(), mapper_a.aGlobal.size() - 1,
          &mapper_b.aGlobal.front(), mapper_b.aGlobal.size() - 1, mapper_a, mapper_b) == FALSE)
        {
          CLOG_WARNING("$Globals 变量不一致");
          return FALSE;
        }
        dest.Copy(dest.aGlobal, &mapper_a.aGlobal.front(), mapper_a.aGlobal.size(), &mapper_a);
      }
      else if(_CL_NOT_(mapper_a.aGlobal.empty()))
      {
        dest.Copy(dest.aGlobal, &mapper_a.aGlobal.front(), mapper_a.aGlobal.size(), &mapper_a);
      }
      else if(_CL_NOT_(mapper_b.aGlobal.empty()))
      {
        dest.Copy(dest.aGlobal, &mapper_b.aGlobal.front(), mapper_b.aGlobal.size(), &mapper_b);
      }

      typedef clmap<clStringA, int> CBNameDict;
      typedef clset<clStringA> CBNameSet;
      CBNameDict sNameDictA, sNameDictB;
      CBNameSet sNameSet;
      mapper_a.GenerateNameDict(sNameSet, sNameDictA);
      mapper_b.GenerateNameDict(sNameSet, sNameDictB);

      Marimo::DATAPOOL_TYPE_DECLARATION sConstBufferDecl = { Marimo::DataPoolTypeClass::Structure };
      // Constant buffer 名字集合
      // 名字一致时内容必须一致      
      for(auto iter_name = sNameSet.begin(); iter_name != sNameSet.end(); ++iter_name)
      {
        CBNameDict::iterator itFindA = sNameDictA.find(*iter_name);
        CBNameDict::iterator itFindB = sNameDictB.find(*iter_name);

        sConstBufferDecl.as.Struct = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(dest.aCBMembers.size());
        sConstBufferDecl.Name = dest.Strings.add(*iter_name);
        dest.aConstantBuffers.push_back(sConstBufferDecl);

        if(itFindA != sNameDictA.end() && itFindB != sNameDictB.end())
        {
          size_t nCBCountA, nCBCountB;
          const Marimo::DATAPOOL_DECLARATION* pCBListA = mapper_a.GetConstantBufferListByIndex(itFindA->second, &nCBCountA);
          const Marimo::DATAPOOL_DECLARATION* pCBListB = mapper_b.GetConstantBufferListByIndex(itFindB->second, &nCBCountB);

          if(CompareVariableDeclarationArray(pCBListA, nCBCountA - 1, pCBListB, nCBCountB - 1, mapper_a, mapper_b) == FALSE)
          {
            CLOG_WARNING("const buffer(%s) 名字一致但是变量不一致", *iter_name);
            return FALSE;
          }

          dest.Copy(dest.aCBMembers, pCBListA, nCBCountA, &mapper_a);
        }
        else if(itFindA != sNameDictA.end())
        {
          size_t nCBCountA;
          const Marimo::DATAPOOL_DECLARATION* pCBListA = mapper_a.GetConstantBufferListByIndex(itFindA->second, &nCBCountA);
          dest.Copy(dest.aCBMembers, pCBListA, nCBCountA, &mapper_a);
        }
        else if(itFindB != sNameDictB.end())
        {
          size_t nCBCountB;
          const Marimo::DATAPOOL_DECLARATION* pCBListB = mapper_b.GetConstantBufferListByIndex(itFindB->second, &nCBCountB);
          dest.Copy(dest.aCBMembers, pCBListB, nCBCountB, &mapper_b);
        }
      }
      return TRUE;
    }
    //////////////////////////////////////////////////////////////////////////

    GXHRESULT ShaderImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT ShaderImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);

      if(nRefCount == 0)
      {
        if(m_pD3D11VertexShader && m_pD3D11PixelShader)
        {
          m_pGraphicsImpl->UnregisterResource(this);
        }
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    GXHRESULT ShaderImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      return GX_OK;
    }

    ShaderImpl::ShaderImpl(GraphicsImpl* pGraphicsImpl)
      : m_pGraphicsImpl(pGraphicsImpl)
      , m_pD3D11VertexShader(NULL)
      , m_pD3D11PixelShader(NULL)
      , m_buffer(8)
    {
    }

    ShaderImpl::~ShaderImpl()
    {
      SAFE_RELEASE(m_pD3D11VertexShader);
      SAFE_RELEASE(m_pD3D11PixelShader);
    }

    GXBOOL ShaderImpl::InitShader(GXLPCWSTR szResourceDir, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount)
    {
      INTERMEDIATE_CODE::Array aCodes;
      GXBOOL bval = TRUE;
      aCodes.reserve(nCount);

      IHLSLInclude* pInclude = new IHLSLInclude(NULL, clStringA(szResourceDir));
      ID3D11Device* pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      DATAPOOL_MAPPER decl_mapper_vs;
      DATAPOOL_MAPPER decl_mapper_ps;

      for(GXUINT i = 0; i < nCount; i++)
      {
        INTERMEDIATE_CODE InterCode;
        GXHRESULT hr = CompileShader(&InterCode, pShaderDescs + i, pInclude);

        if(GXFAILED(hr) || InterCode.type == TargetType::Undefine) {
          bval = FALSE;
          break;
        }

        aCodes.push_back(InterCode);
        if(InterCode.type == TargetType::Vertex)
        {
          hr = pd3dDevice->CreateVertexShader(
            InterCode.pCode->GetBufferPointer(),
            InterCode.pCode->GetBufferSize(),
            NULL, &m_pD3D11VertexShader);
          TRACE("[Vertex Shader]\n");
          Reflect(decl_mapper_vs, InterCode.pReflection);

          m_VertexBuf.Append(InterCode.pCode->GetBufferPointer(), InterCode.pCode->GetBufferSize());
        }
        else if(InterCode.type == TargetType::Pixel)
        {
          hr = pd3dDevice->CreatePixelShader(
            InterCode.pCode->GetBufferPointer(),
            InterCode.pCode->GetBufferSize(),
            NULL, &m_pD3D11PixelShader);
          TRACE("[Pixel Shader]\n");
          Reflect(decl_mapper_ps, InterCode.pReflection);
        }

        if(FAILED(hr)) {
          bval = FALSE;
          break;
        }


        SAFE_RELEASE(InterCode.pCode);
      }

      DATAPOOL_MAPPER decl_mapper;
      if((bval = MergeDataPoolMapped(decl_mapper, decl_mapper_ps, decl_mapper_vs)))
      {
        bval = BuildDataPoolDecl(decl_mapper);
      }
      
      if(_CL_NOT_(bval))
      {
        SAFE_RELEASE(m_pD3D11PixelShader);
        SAFE_RELEASE(m_pD3D11VertexShader);
      }
      else
      {
        DbgCheck(aCodes);
      }

      //SAFE_RELEASE(InterCode.pReflection);

      // 释放decl_mapper相关的的字符串
      for(auto it = aCodes.begin(); it != aCodes.end(); ++it) {
        SAFE_RELEASE(it->pReflection);
      }

      SAFE_DELETE(pInclude);
      return bval;
    }

    GXBOOL ShaderImpl::Reflect(DATAPOOL_MAPPER& decl_mapper, ID3D11ShaderReflection* pReflection)
    {
      if(pReflection == NULL) {
        return FALSE;
      }

      D3D11_SHADER_DESC sShaderDesc;
      pReflection->GetDesc(&sShaderDesc);
      for(UINT nn = 0; nn < sShaderDesc.BoundResources; nn++)
      {
        D3D11_SHADER_INPUT_BIND_DESC bind_desc;
        pReflection->GetResourceBindingDesc(nn, &bind_desc);
        switch(bind_desc.Type)
        {
        case D3D_SHADER_INPUT_TYPE::D3D10_SIT_CBUFFER:
          TRACE("cbuffer %s\n", bind_desc.Name);
          break;

        case D3D_SHADER_INPUT_TYPE::D3D10_SIT_SAMPLER:
          TRACE("sampler %s\n", bind_desc.Name);
          break;

        case D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE:
          TRACE("texture %s\n", bind_desc.Name);
          break;

        default:
          CLBREAK;
          break;
        }
        CLNOP;
      }

      Marimo::DATAPOOL_DECLARATION sEmpty = { NULL };

      for(UINT nn = 0; nn < sShaderDesc.ConstantBuffers; nn++)
      {
        ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer = pReflection->GetConstantBufferByIndex(nn);

        D3D11_SHADER_BUFFER_DESC buffer_desc;
        pReflectionConstantBuffer->GetDesc(&buffer_desc);
        TRACE("Constant Buffer:%s\n", buffer_desc.Name);

        if(clstd::strcmpT(buffer_desc.Name, "$Globals") == 0) {
          Reflect_ConstantBuffer(decl_mapper.aGlobal, decl_mapper, pReflectionConstantBuffer, buffer_desc);
          //decl_mapper.aGlobal.push_back(sEmpty);
        }
        else {
          clStringA strCBName = "cb_";
          Marimo::DATAPOOL_TYPE_DEFINITION sCBDef = { Marimo::DataPoolTypeClass::Structure };
          strCBName.Append(buffer_desc.Name);
          
          sCBDef.Name = decl_mapper.Strings.add(strCBName);
          sCBDef.as.Struct = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(decl_mapper.aCBMembers.size());

          Reflect_ConstantBuffer(decl_mapper.aCBMembers, decl_mapper, pReflectionConstantBuffer, buffer_desc);
          decl_mapper.aCBMembers.push_back(sEmpty);
          decl_mapper.aConstantBuffers.push_back(sCBDef);
        }

        CLNOP;
      }
      return TRUE;
    }

    GXBOOL ShaderImpl::Reflect_ConstantBuffer(DataPoolDeclaration_T& aArray, DATAPOOL_MAPPER& aStructDesc, ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer, const D3D11_SHADER_BUFFER_DESC& buffer_desc)
    {
      Marimo::DATAPOOL_VARIABLE_DECLARATION vari_decl;
      for(UINT kkk = 0; kkk < buffer_desc.Variables; kkk++)
      {
        ID3D11ShaderReflectionVariable* pReflectionVariable = pReflectionConstantBuffer->GetVariableByIndex(kkk);
        ID3D11ShaderReflectionType* pReflectionType = pReflectionVariable->GetType();

        D3D11_SHADER_TYPE_DESC type_desc;
        D3D11_SHADER_VARIABLE_DESC variable_desc;
        pReflectionVariable->GetDesc(&variable_desc);

        InlSetZeroT(vari_decl);

        vari_decl.Type = Reflect_MakeTypename(aStructDesc, type_desc, pReflectionType);
        vari_decl.Name = variable_desc.Name;
        vari_decl.Count = type_desc.Elements;

        if(type_desc.Elements > 0) {          
          TRACE("Variable: (%s)%s[%d] (start:%d, end:%d)[%d]\n", vari_decl.Type, variable_desc.Name, type_desc.Elements,
            variable_desc.StartOffset, variable_desc.StartOffset + variable_desc.Size, variable_desc.Size);
        }
        else {
          TRACE("Variable: (%s)%s (start:%d, end:%d)[%d]\n", vari_decl.Type, variable_desc.Name,
            variable_desc.StartOffset, variable_desc.StartOffset + variable_desc.Size, variable_desc.Size);
        }
        CLNOP;
        aArray.push_back(vari_decl);
      }
      return TRUE;
    }

    GXLPCSTR ShaderImpl::Reflect_MakeTypename(DATAPOOL_MAPPER& aStructDesc, D3D11_SHADER_TYPE_DESC& type_desc, ID3D11ShaderReflectionType* pReflectionType)
    {
      clStringA strTypeName;
      int type = 0;
      const size_t nStartMember = aStructDesc.aMembers.size();

      pReflectionType->GetDesc(&type_desc);
      switch(type_desc.Type)
      {
      case D3D_SVT_FLOAT:   strTypeName = "float";  type = 0;   break;
      case D3D_SVT_INT:     strTypeName = "int";    type = 1;   break;
      case D3D_SVT_BOOL:    strTypeName = "bool";   type = 2;   break;
      case D3D_SVT_UINT:    strTypeName = "uint";   type = 3;   break;
      case D3D_SVT_VOID:    strTypeName = "void";   break;
      default:              CLBREAK;                break;
      }

      int type_class = 0;
      switch(type_desc.Class)
      {
      case D3D_SVC_SCALAR:
        type_class = 0;
        break;

      case D3D_SVC_VECTOR:
        strTypeName.AppendInteger32(type_desc.Columns);
        type_class = type_desc.Columns - 1;
        break;

      case D3D_SVC_MATRIX_COLUMNS:
        strTypeName.AppendFormat("%dx%d", type_desc.Rows, type_desc.Columns);
        type_class = 4 + (type_desc.Rows - 1) * 4 + (type_desc.Columns - 1);
        break;

      case D3D_SVC_MATRIX_ROWS:
        strTypeName.Insert(0, "row_");
        strTypeName.AppendFormat("%dx%d", type_desc.Rows, type_desc.Columns);
        type_class = 20 + (type_desc.Rows - 1) * 4 + (type_desc.Columns - 1);
        break;

      case D3D_SVC_STRUCT:
      {
        Marimo::DATAPOOL_DECLARATION member_decl = { NULL };
        //ID3D11ShaderReflectionType* pReflectionType = pReflectionVariable->GetType();


        for(UINT member = 0; member < type_desc.Members; member++)
        {
          ID3D11ShaderReflectionType* pMemberType = pReflectionType->GetMemberTypeByIndex(member);
          D3D11_SHADER_TYPE_DESC member_type_desc;

          member_decl.Type = Reflect_MakeTypename(aStructDesc, member_type_desc, pMemberType);
          member_decl.Name = pReflectionType->GetMemberTypeName(member);
          member_decl.Count = member_type_desc.Elements;

          if(member_type_desc.Class == D3D_SVC_STRUCT) {
            CLBREAK;
          }

#if 0
          pMemberType->GetDesc(&member_type_desc);

          TRACE("%s.%s(%d)\n", "<struct name>", pReflectionType->GetMemberTypeName(member),
            member_type_desc.Offset);

          ID3D11ShaderReflectionType* pSubType = pReflectionType->GetSubType();
          D3D11_SHADER_TYPE_DESC sub_type_desc;
          if(member_type_desc.Class == D3D_SVC_STRUCT)
          {
            ASSERT(pSubType);
            
            CLBREAK; // TODO: 不能嵌套
            //pSubType->GetDesc(&sub_type_desc);
            member_decl.Type = Reflect_MakeTypename(aStructDesc, sub_type_desc, pSubType);
            member_decl.Name = pReflectionType->GetMemberTypeName(member);
            member_decl.Count = sub_type_desc.Elements;
            aStructDesc.aMembers.push_back(member_decl);
          }
          else
          {
            ASSERT(pSubType == NULL);
            member_decl.Type = Reflect_MakeTypename(aStructDesc, sub_type_desc, pSubType);
            member_decl.Name = pReflectionType->GetMemberTypeName(member);
            member_decl.Count = sub_type_desc.Elements;
          }
#endif
#if 0
          ID3D11ShaderReflectionType* pInterfaceType = pReflectionType->GetSubType();
          if(pInterfaceType)
          {
            D3D11_SHADER_TYPE_DESC interface_type_desc;
            pInterfaceType->GetDesc(&interface_type_desc);
          }
#endif
          aStructDesc.aMembers.push_back(member_decl);
          CLNOP;
        }

        InlSetZeroT(member_decl);
        aStructDesc.aMembers.push_back(member_decl);

        strTypeName.Format(STRUCT_PREFIX_NAME"%u", aStructDesc.aTypes.size());
      }
      break;

      default:
        CLBREAK;
        break;
      }

      //vari_decl.Type = ::D3D11::s_szTypeName[type][type_class];
      //vari_decl.Name = variable_desc.Name;
      //vari_decl.Count = type_desc.Elements;

      
      if(type_desc.Class == D3D_SVC_STRUCT)
      {
        Marimo::DATAPOOL_TYPE_DEFINITION sTypeDef = { Marimo::DataPoolTypeClass::Structure };
        sTypeDef.Name = aStructDesc.Strings.add(strTypeName);
        sTypeDef.as.Struct = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(nStartMember);
        aStructDesc.aTypes.push_back(sTypeDef);

        // 返回这个指针必须是稳定的
        return sTypeDef.Name;
      }
      else
      {
        ASSERT(strTypeName == s_szTypeName[type][type_class]);
        return s_szTypeName[type][type_class]; 
      }

      //if(type_desc.Elements > 0) {
      //  TRACE("Variable: (%s)%s[%d] (start:%d, end:%d)[%d]\n", strDbgTypeName.CStr(), variable_desc.Name, type_desc.Elements,
      //    variable_desc.StartOffset, variable_desc.StartOffset + variable_desc.Size, variable_desc.Size);
      //}
      //else {
      //  TRACE("Variable: (%s)%s (start:%d, end:%d)[%d]\n", strDbgTypeName.CStr(), variable_desc.Name,
      //    variable_desc.StartOffset, variable_desc.StartOffset + variable_desc.Size, variable_desc.Size);
      //}
    }

    GXBOOL ShaderImpl::Activate()
    {
      CLBREAK;
      return FALSE;
    }

    GXBOOL ShaderImpl::BuildDataPoolDecl(DATAPOOL_MAPPER& mapper)
    {
      // mapper 是会被修改的
      // 缓冲区顺序
      // *.字符串
      // *.结构体成员列表
      // *.CB成员列表（本质也是结构体）
      // *.全局变量列表（CB作为结构体变量在尾部声明）<- m_pDataPoolDecl
      // *.结构体类型定义 <- m_pDataPoolTypeDef
      // *.CB结构体类型

      // 把 constant buffer 作为结构体添加在局部变量后面
      Marimo::DATAPOOL_DECLARATION desc = { NULL };
      clStringA str;
      for(size_t i = 0; i < mapper.aConstantBuffers.size(); i++)
      {
        str = mapper.aConstantBuffers[i].Name + sizeof(CB_PREFIX_NAME) - 1;
        desc.Type = mapper.aConstantBuffers[i].Name;
        desc.Name = mapper.Strings.add(str);
        mapper.aGlobal.push_back(desc);
      }

      // 计算总缓冲区大小
      const size_t nStringBufSize = mapper.Strings.buffer_size();
      const size_t nTotalSize = nStringBufSize +
        sizeof(Marimo::DATAPOOL_DECLARATION) * (mapper.aGlobal.size() + mapper.aMembers.size() + mapper.aCBMembers.size() + 1) +
        sizeof(Marimo::DATAPOOL_TYPE_DECLARATION) * (mapper.aConstantBuffers.size() + mapper.aTypes.size() + 1);
      m_buffer.Reserve(nTotalSize);
      m_buffer.Resize(nStringBufSize, FALSE);

      // 所有字符串写入缓冲区
      mapper.Strings.gather(reinterpret_cast<GXLPCSTR>(m_buffer.GetPtr()));
      
      // 拷贝变量列表
      TRACE("%s:\n", __FUNCTION__);
      DataPoolDeclaration_T aDecl[] = {mapper.aMembers, mapper.aCBMembers, mapper.aGlobal};
      Marimo::DATAPOOL_DECLARATION* pMemberBase = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(
        reinterpret_cast<size_t>(m_buffer.GetPtr()) + m_buffer.GetSize());

      for(int n = 0; n < 3; n++)
      {
        if(n == 2) {
          m_pDataPoolDecl = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(
            reinterpret_cast<size_t>(m_buffer.GetPtr()) + m_buffer.GetSize());
        }

        DataPoolDeclaration_T& rDecl = aDecl[n];
        for(size_t i = 0; i < rDecl.size(); i++)
        {
          if(rDecl[i].Name && rDecl[i].Type)
          {
            rDecl[i].Name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(rDecl[i].Name));
            if(clstd::strncmpT(rDecl[i].Type, STRUCT_PREFIX_NAME, sizeof(STRUCT_PREFIX_NAME) - 1) == 0) {
              rDecl[i].Type = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(rDecl[i].Type));
            }
          }

          ASSERT(rDecl[i].Name == NULL || rDecl[i].Name[0] != '\0');

          TRACE("%s %s[%d]\n", rDecl[i].Type, rDecl[i].Name, rDecl[i].Count);
          m_buffer.Append(&rDecl[i], sizeof(Marimo::DATAPOOL_DECLARATION));
          ASSERT(mapper.Strings.buffer_size() == nStringBufSize);
        }
      }

      // $Globals结尾
      Marimo::DATAPOOL_DECLARATION sEmptyVar = { NULL };
      m_buffer.Append(&sEmptyVar, sizeof(Marimo::DATAPOOL_DECLARATION));

      // 没有结构体或者CB就把类型定义设置为NULL
      if(mapper.aTypes.empty() && mapper.aConstantBuffers.empty()) {
        m_pDataPoolTypeDef = NULL;
      }
      else {
        m_pDataPoolTypeDef = reinterpret_cast<Marimo::DATAPOOL_TYPE_DEFINITION*>(
          reinterpret_cast<size_t>(m_buffer.GetPtr()) + m_buffer.GetSize());
      }

      TRACE("%s type list\n", __FUNCTION__);

      for(int n = 0; n < 2; n++)
      {
        DataPoolTypeDefinition_T& rType = (n == 0) ? mapper.aTypes : mapper.aConstantBuffers;
        for(size_t i = 0; i < rType.size(); i++)
        {
          rType[i].Name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(rType[i].Name));
          rType[i].as.Struct = pMemberBase + (size_t)rType[i].as.Struct;
          rType[i].MemberPack = Marimo::DataPoolPack::NotCross16BoundaryShort; // 这里与D3D11文档写的不一样
          // 文档上写的是结构体是16字节的整数倍，实际测试发现最后一个成员没有按照16字节扩充
          TRACE("%s\n", rType[i].Name);
          m_buffer.Append(&rType[i], sizeof(Marimo::DATAPOOL_TYPE_DEFINITION));
          ASSERT(mapper.Strings.buffer_size() == nStringBufSize);
#ifdef _DEBUG
          for(int d = 0; rType[i].as.Struct[d].Name != NULL; d++)
          {
            ASSERT(rType[i].as.Struct[d].Name >= m_buffer.GetPtr() &&
              (size_t)rType[i].as.Struct[d].Name < (size_t)m_buffer.GetPtr() + nStringBufSize);
          }
#endif
        }
        pMemberBase += mapper.aMembers.size();
      }

      // 类型列表的结尾
      Marimo::DATAPOOL_TYPE_DEFINITION sEmptyType = { Marimo::DataPoolTypeClass::Undefine };
      m_buffer.Append(&sEmptyType, sizeof(Marimo::DATAPOOL_TYPE_DEFINITION));

      ASSERT(m_buffer.GetSize() == nTotalSize); // 校验实际填充大小和计算大小
      return TRUE;
    }

    void ShaderImpl::DbgCheck(INTERMEDIATE_CODE::Array& aInterCode)
    {
#define CHECK_VALUE(_VAL, _EXPRA, _EXPRB) ASSERT((_VAL = _EXPRA) == _EXPRB)
      Marimo::DataPool* pDataPool = NULL;
      GXHRESULT hr = Marimo::DataPool::CreateDataPool(&pDataPool, NULL, m_pDataPoolTypeDef, m_pDataPoolDecl, Marimo::DataPoolCreation_NotCross16BytesBoundary);
      ASSERT(GXSUCCEEDED(hr));

      MOVariable var;
      size_t dbg_value;
      for(auto it = aInterCode.begin(); it != aInterCode.end(); ++it)
      {
        ID3D11ShaderReflection* pReflection = it->pReflection;
        D3D11_SHADER_DESC D3D11ShaderDesc;
        pReflection->GetDesc(&D3D11ShaderDesc);

        for(UINT cb_index = 0; cb_index < D3D11ShaderDesc.ConstantBuffers; cb_index++)
        {
          D3D11_SHADER_BUFFER_DESC D3D11ShaderBufDesc;
          ID3D11ShaderReflectionConstantBuffer* pD3D11CB = pReflection->GetConstantBufferByIndex(cb_index);
          pD3D11CB->GetDesc(&D3D11ShaderBufDesc);
          Marimo::DataPool::iterator iter_var;
          Marimo::DataPool::iterator iter_var_end;
          size_t nBaseOffset = 0;
          if(clstd::strcmpT(D3D11ShaderBufDesc.Name, "$Globals") == 0)
          {
            iter_var = pDataPool->begin();
            iter_var_end = pDataPool->end();
          }
          else
          {
            MOVariable varCB;
            GXBOOL bval = pDataPool->QueryByName(D3D11ShaderBufDesc.Name, &varCB);
            ASSERT(bval);
            iter_var = varCB.begin();
            iter_var_end = varCB.end();
            nBaseOffset = varCB.GetOffset();
          }
          
          for(UINT var_index = 0; iter_var != iter_var_end; ++iter_var, var_index++)
          {
            iter_var.ToVariable(var);

            // 遇到结尾的CB定义就结束
            if(clstd::strncmpT(var.GetTypeName(), CB_PREFIX_NAME, sizeof(CB_PREFIX_NAME) - 1) == 0) {
              break;
            }

            ID3D11ShaderReflectionVariable* pD3D11Var = pD3D11CB->GetVariableByIndex(var_index);
            ASSERT(pD3D11Var);

            D3D11_SHADER_VARIABLE_DESC D3D11VarDesc;
            pD3D11Var->GetDesc(&D3D11VarDesc);
            ID3D11ShaderReflectionType* pD3DType = pD3D11Var->GetType();
            D3D11_SHADER_TYPE_DESC D3D11TypeDesc;
            pD3DType->GetDesc(&D3D11TypeDesc);
            TRACE("%s %d %d\n", var.GetName(), var.GetOffset(), var.GetSize());
            ASSERT(clstd::strcmpT(var.GetName(), D3D11VarDesc.Name) == 0);
            CHECK_VALUE(dbg_value, var.GetOffset() - nBaseOffset, D3D11VarDesc.StartOffset);
            CHECK_VALUE(dbg_value, var.GetSize(), D3D11VarDesc.Size);
          }
        }


      }
#undef CHECK_VALUE
      SAFE_RELEASE(pDataPool);
    }

    GXINT ShaderImpl::GetCacheSize() const
    {
      CLBREAK;
      return 0;
    }

    Graphics* ShaderImpl::GetGraphicsUnsafe() const
    {
      return m_pGraphicsImpl;
    }

    void ShaderImpl::GetDataPoolDeclaration(Marimo::DATAPOOL_MANIFEST* pManifest) const
    {
      pManifest->pTypes       = m_pDataPoolTypeDef;
      pManifest->pVariables   = m_pDataPoolDecl;
      pManifest->pImportFiles = NULL;
    }

    GXBOOL ShaderImpl::CheckUpdateConstBuf()
    {
      CLBREAK;
      return FALSE;         
    }

    ShaderImpl::TargetType ShaderImpl::TargetNameToType(GXLPCSTR szTargetName)
    {
      if(szTargetName[0] == 'p' && szTargetName[1] == 's' && szTargetName[2] == '_') {
        return TargetType::Pixel;
      }
      else if(szTargetName[0] == 'v' && szTargetName[1] == 's' && szTargetName[2] == '_') {
        return TargetType::Vertex;
      }
      CLBREAK; // 不支持的shader类型
    }

    GXHRESULT ShaderImpl::CompileShader(INTERMEDIATE_CODE* pInterCode, const GXSHADER_SOURCE_DESC* pShaderDesc, ID3DInclude* pInclude)
    {
      DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
      dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
      ID3DBlob* pErrorBlob = NULL;
      pInterCode->type = TargetType::Undefine;

      const SIZE_T nSourceLength = pShaderDesc->nSourceLen == 0
        ?clstd::strlenT(pShaderDesc->szSourceData)
        : pShaderDesc->nSourceLen;

      // 源代码编译为中间代码
      HRESULT hval = D3DCompile(pShaderDesc->szSourceData, nSourceLength,
        __FUNCTION__, (D3D10_SHADER_MACRO*)pShaderDesc->pDefines,
        pInclude, pShaderDesc->szEntry, pShaderDesc->szTarget, dwShaderFlags, 0, &pInterCode->pCode, &pErrorBlob);

      if(FAILED(hval))
      {
        if(pErrorBlob != NULL) {
          TRACE("Shader compiled error:\n>%s\n", (char*)pErrorBlob->GetBufferPointer());
        }
        SAFE_RELEASE(pErrorBlob);
        return hval;
      }

      // shader 输入参数信息
      //LPVOID pInterCodePtr = pInterCode->pCode->GetBufferPointer();
      //SIZE_T InterCodeLen = pInterCode->pCode->GetBufferSize();
      hval = D3DReflect(pInterCode->pCode->GetBufferPointer(), pInterCode->pCode->GetBufferSize(),
        IID_ID3D11ShaderReflection, (void**)&pInterCode->pReflection);

      if(FAILED(hval))
      {
        SAFE_RELEASE(pInterCode->pCode);
        return hval;
      }

      // profile 转换为枚举
      pInterCode->type = TargetNameToType(pShaderDesc->szTarget);
      SAFE_RELEASE(pErrorBlob);
      return S_OK;
    }

    //////////////////////////////////////////////////////////////////////////


  } // namespace D3D11
} // namespace GrapX

#ifdef REFACTOR_GRAPX_SHADER
GXHRESULT GXDLLAPI MOCompileHLSL(GXLPCWSTR szShaderDesc, GXLPCWSTR szResourceDir, GXLPCSTR szPlatformSect, MOSHADERBUFFERS* pBuffers)
{
  MOSHADER_ELEMENT_SOURCE SrcComponent;
  GXHRESULT hval = GrapX::GShader::Load(szShaderDesc, szResourceDir, szPlatformSect, &SrcComponent, NULL);
  if(GXFAILED(hval)) {
    return hval;
  }

  GXDefinitionArray aMacros;
  MOSHADERBUFFERS ShaderSources;
  clBuffer*& pVBuf = ShaderSources.pVertexShader; // 另外声明一个短名变量,便于阅读
  clBuffer*& pPBuf = ShaderSources.pPixelShader;
  GXDEFINITION* pShaderMacro = NULL;
  pVBuf = NULL;
  pPBuf = NULL;
  hval = GX_OK;

  if( ! GrapX::GShader::ComposeSource(&SrcComponent, GXPLATFORM_WIN32_DIRECT3D11, &ShaderSources, &aMacros)) {
    // LOG: 组合Shader Source失败
    hval = GX_FAIL;
    goto FUNC_RET;
  }

  const GXBOOL bCompiledVS = ((GXLPBYTE)pVBuf->GetPtr())[0] == '\0';
  const GXBOOL bCompiledPS = ((GXLPBYTE)pPBuf->GetPtr())[0] == '\0';

  if(bCompiledVS || bCompiledPS) {
    // LOG: VS/PS必须都为源代码才能编译
    hval = GX_FAIL;
    goto FUNC_RET;
  }

  // 建立编译时需要的宏
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

  const GXBOOL bVSComposing = SrcComponent.strVSComposer.IsNotEmpty();
  const GXBOOL bPSComposing = SrcComponent.strPSComposer.IsNotEmpty();

  GrapX::D3D11::IHLSLInclude* pInclude = new GrapX::D3D11::IHLSLInclude(NULL, clStringA(szResourceDir));

  if(GXFAILED(hval = GrapX::D3D11::GShaderImpl::CompileShader(pVBuf, pInclude, pShaderMacro, bVSComposing 
    ? GrapX::D3D11::GShaderImpl::CompiledComponentVertexShder : GrapX::D3D11::GShaderImpl::CompiledVertexShder))) {
    goto FUNC_RET;
  }

  if(GXFAILED(hval = GrapX::D3D11::GShaderImpl::CompileShader(pPBuf, pInclude, pShaderMacro, bPSComposing 
    ? GrapX::D3D11::GShaderImpl::CompiledComponentPixelShder : GrapX::D3D11::GShaderImpl::CompiledPixelShder))) {
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
  // TODO: 判断返回值
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
#endif // #ifdef REFACTOR_GRAPX_SHADER

//SetVertexShaderConstantB
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)