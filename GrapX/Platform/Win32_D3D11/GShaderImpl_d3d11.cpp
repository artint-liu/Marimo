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

      clvector<GXLPCSTR>        arraySampler;
      clvector<GXLPCSTR>        arrayTexture;

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

      size_t GetBufferCount() const
      {
        return (aGlobal.empty() ? 0 : 1) + aConstantBuffers.size();
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

    GXBOOL MergeDataPoolMapped(DATAPOOL_MAPPER& dest, const DATAPOOL_MAPPER& mapper_a, clvector<size_t>& aIndexTabA, const DATAPOOL_MAPPER& mapper_b, clvector<size_t>& aIndexTabB)
    {
      dest.aGlobal.reserve(clMax(mapper_a.aGlobal.size(), mapper_b.aGlobal.size()));
      dest.aConstantBuffers.reserve(mapper_a.aConstantBuffers.size() + mapper_b.aConstantBuffers.size());
      dest.aCBMembers.reserve(mapper_a.aCBMembers.size() + mapper_b.aCBMembers.size());
      dest.aTypes.reserve(mapper_a.aTypes.size() + mapper_b.aTypes.size());
      dest.aMembers.reserve(mapper_a.aMembers.size() + mapper_b.aMembers.size());
      aIndexTabA.reserve(mapper_a.GetBufferCount());
      aIndexTabB.reserve(mapper_b.GetBufferCount());

      const DataPoolDeclaration_T& a = mapper_a.aGlobal;
      const DataPoolDeclaration_T& b = mapper_b.aGlobal;
      size_t nDestIndex = 0;

      // $Globals 必须完全一致
      if(_CL_NOT_(mapper_a.aGlobal.empty()) && _CL_NOT_(mapper_b.aGlobal.empty()))
      {
        if(CompareVariableDeclarationArray(
          &mapper_a.aGlobal.front(), mapper_a.aGlobal.size() - 1,
          &mapper_b.aGlobal.front(), mapper_b.aGlobal.size() - 1, mapper_a, mapper_b) == FALSE)
        {
          CLOG_ERROR("$Globals 变量不一致");
          return FALSE;
        }
        dest.Copy(dest.aGlobal, &mapper_a.aGlobal.front(), mapper_a.aGlobal.size(), &mapper_a);
        aIndexTabA.push_back(nDestIndex);
        aIndexTabB.push_back(nDestIndex);
        nDestIndex++;
      }
      else if(_CL_NOT_(mapper_a.aGlobal.empty()))
      {
        dest.Copy(dest.aGlobal, &mapper_a.aGlobal.front(), mapper_a.aGlobal.size(), &mapper_a);
        aIndexTabA.push_back(nDestIndex);
        nDestIndex++;
      }
      else if(_CL_NOT_(mapper_b.aGlobal.empty()))
      {
        dest.Copy(dest.aGlobal, &mapper_b.aGlobal.front(), mapper_b.aGlobal.size(), &mapper_b);
        aIndexTabB.push_back(nDestIndex);
        nDestIndex++;
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
            CLOG_ERROR("const buffer(%s) 名字一致但是变量不一致", *iter_name);
            return FALSE;
          }

          dest.Copy(dest.aCBMembers, pCBListA, nCBCountA, &mapper_a);
          aIndexTabA.push_back(nDestIndex);
          aIndexTabB.push_back(nDestIndex);
          nDestIndex++;
        }
        else if(itFindA != sNameDictA.end())
        {
          size_t nCBCountA;
          const Marimo::DATAPOOL_DECLARATION* pCBListA = mapper_a.GetConstantBufferListByIndex(itFindA->second, &nCBCountA);
          dest.Copy(dest.aCBMembers, pCBListA, nCBCountA, &mapper_a);
          aIndexTabA.push_back(nDestIndex);
          nDestIndex++;
        }
        else if(itFindB != sNameDictB.end())
        {
          size_t nCBCountB;
          const Marimo::DATAPOOL_DECLARATION* pCBListB = mapper_b.GetConstantBufferListByIndex(itFindB->second, &nCBCountB);
          dest.Copy(dest.aCBMembers, pCBListB, nCBCountB, &mapper_b);
          aIndexTabB.push_back(nDestIndex);
          nDestIndex++;
        }
      }

      // sampler
      if(_CL_NOT_(mapper_a.arraySampler.empty()) && _CL_NOT_(mapper_b.arraySampler.empty())) {
        if(mapper_a.arraySampler != mapper_b.arraySampler) {
          CLOG_ERROR("sampler 列表不一致");
          return FALSE;
        }
      }
      else if(_CL_NOT_(mapper_a.arraySampler.empty())) {
        dest.arraySampler = mapper_a.arraySampler;
      }
      else if(_CL_NOT_(mapper_b.arraySampler.empty())) {
        dest.arraySampler = mapper_b.arraySampler;
      }

      // texture
      if(_CL_NOT_(mapper_a.arrayTexture.empty()) && _CL_NOT_(mapper_b.arrayTexture.empty())) {
        if(mapper_a.arrayTexture != mapper_b.arrayTexture) {
          CLOG_ERROR("sampler 列表不一致");
          return FALSE;
        }
      }
      else if(_CL_NOT_(mapper_a.arrayTexture.empty())) {
        dest.arrayTexture = mapper_a.arrayTexture;
      }
      else if(_CL_NOT_(mapper_b.arrayTexture.empty())) {
        dest.arrayTexture = mapper_b.arrayTexture;
      }

      ASSERT(aIndexTabA.size() == mapper_a.GetBufferCount());
      ASSERT(aIndexTabB.size() == mapper_b.GetBufferCount());
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
      , m_pDataPoolDecl(NULL)
      , m_pDataPoolTypeDef(NULL)
      , m_pBindResourceDesc(NULL)
      , m_nBindResourceDesc(0)
      , m_buffer(8)
      , m_pVertexCB(NULL)
      , m_pPixelCB(NULL)
    {
    }

    ShaderImpl::~ShaderImpl()
    {
      SAFE_RELEASE(m_pD3DVertexInterCode);
      SAFE_RELEASE(m_pD3D11VertexShader);
      SAFE_RELEASE(m_pD3D11PixelShader);

      if(_CL_NOT_(m_arrayCB.empty())){
        ID3D11Buffer** ppD3D11Buffer = &m_arrayCB.front();
        for(; ppD3D11Buffer != m_pVertexCB; ppD3D11Buffer++)
        {
          SAFE_RELEASE(*ppD3D11Buffer);
        }
      }
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
          hr = pd3dDevice->CreateVertexShader(InterCode.pCode->GetBufferPointer(),
            InterCode.pCode->GetBufferSize(), NULL, &m_pD3D11VertexShader);

          TRACE("[Vertex Shader]\n");
          Reflect(decl_mapper_vs, InterCode.pReflection);

          //m_VertexBuf.Append(InterCode.pCode->GetBufferPointer(), InterCode.pCode->GetBufferSize());
          m_pD3DVertexInterCode = InterCode.pCode;
          m_pD3DVertexInterCode->AddRef();
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
      clvector<size_t> aIndexTabA, aIndexTabB;
      if((bval = MergeDataPoolMapped(decl_mapper, decl_mapper_vs, aIndexTabA, decl_mapper_ps, aIndexTabB)))
      {
        DATAPOOL_MAPPER arrayMapper[2] = {decl_mapper_vs, decl_mapper_ps};
        clvector<size_t> arrayIndexTab[2] = {aIndexTabA, aIndexTabB};
        BuildIndexedCBTable(decl_mapper, arrayMapper, arrayIndexTab);
        bval = BuildDataPoolDecl(decl_mapper);
      }
      
      if(_CL_NOT_(bval))
      {
        SAFE_RELEASE(m_pD3D11PixelShader);
        SAFE_RELEASE(m_pD3D11VertexShader);
      }
      else
      {
#ifdef _DEBUG
        DbgCheck(aCodes);
#endif
      }

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
          TRACE("sampler %s Point:%d, Count:%d \n", bind_desc.Name, bind_desc.BindPoint, bind_desc.BindCount);
          ASSERT(decl_mapper.arraySampler.size() == bind_desc.BindPoint); // 测试出现顺序与BindPoint一致
          decl_mapper.arraySampler.push_back(bind_desc.Name);
          break;

        case D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE:
          TRACE("texture %s Point:%d, Count:%d \n", bind_desc.Name, bind_desc.BindPoint, bind_desc.BindCount);
          ASSERT(decl_mapper.arrayTexture.size() == bind_desc.BindPoint);
          decl_mapper.arrayTexture.push_back(bind_desc.Name);
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
      ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();
      pImmediateContext->VSSetShader(m_pD3D11VertexShader, NULL, 0);
      pImmediateContext->PSSetShader(m_pD3D11PixelShader, NULL, 0);
      return TRUE;
    }

    GXBOOL ShaderImpl::BuildIndexedCBTable(const DATAPOOL_MAPPER& combine, const DATAPOOL_MAPPER* pMapper, clvector<size_t>* pIndexTab)
    {
      const size_t nCombine = combine.GetBufferCount();
      const size_t nMapper = pMapper[0].GetBufferCount() + pMapper[1].GetBufferCount();
      if(nCombine + nMapper == 0) {
        return TRUE;
      }
      m_arrayCB.reserve(nCombine + nMapper);
      m_arrayCB.assign(nCombine, NULL);
      for(int i = 0; i < 2; i++)
      {
        //m_arrayCB.insert(m_arrayCB.end(), pIndexTab[i].begin(), pIndexTab[i].end());
        //for(size_t n = 0; n < pIndexTab[i].size(); n++)
        std::for_each(pIndexTab[i].begin(), pIndexTab[i].end(), [this](size_t n)
        {
          m_arrayCB.push_back(reinterpret_cast<ID3D11Buffer*>(n));
        });
      }
      m_pVertexCB = &m_arrayCB[nCombine];
      m_pPixelCB  = &m_arrayCB[nCombine + pMapper[0].GetBufferCount()];
      ASSERT(m_arrayCB.size() == nCombine + nMapper);
      return TRUE;
    }

    GXBOOL ShaderImpl::BuildCBTable(Marimo::DataPool* pDataPool)
    {
      if(m_arrayCB.front()) {
        return FALSE;
      }

      Marimo::DataPoolUtility::iterator iter_var = pDataPool->begin();
      Marimo::DataPoolUtility::iterator iter_var_end = pDataPool->end();
      Marimo::DataPoolVariable var;
      size_t nCB = 0;

      // $Global D3D11 CB
      for(; iter_var != iter_var_end; ++iter_var) {
        if(clstd::strncmpT(iter_var.TypeName(), CB_PREFIX_NAME, sizeof(CB_PREFIX_NAME) - 1) == 0) {
          iter_var.ToVariable(var);
          if(var.GetOffset() > 0) {
            m_arrayCB[nCB++] = D3D11CreateBuffer(var.GetOffset());
            break;
          }
        }
      }

      if(iter_var == iter_var_end)
      {
        m_arrayCB[nCB++] = D3D11CreateBuffer(pDataPool->GetRootSize());
      }
      else
      {
        // Named D3D11 CB
        for(; iter_var != iter_var_end; ++iter_var) {
          if(clstd::strncmpT(iter_var.TypeName(), CB_PREFIX_NAME, sizeof(CB_PREFIX_NAME) - 1) == 0) {
            iter_var.ToVariable(var);
            m_arrayCB[nCB++] = D3D11CreateBuffer(var.GetSize());
          }
          else {
            break;
          }
        }
      }

      ASSERT((m_pVertexCB - &m_arrayCB.front()) == nCB); // 检查结尾准确性
      for(size_t i = nCB; i < m_arrayCB.size(); i++)
      {
        // 索引转成内容
        ASSERT(reinterpret_cast<size_t>(m_arrayCB[i]) < nCB);
        m_arrayCB[i] = m_arrayCB[reinterpret_cast<size_t>(m_arrayCB[i])];
      }

      return TRUE;
    }

    GXBOOL ShaderImpl::CommitConstantBuffer(Marimo::DataPool* pDataPool)
    {
      ID3D11DeviceContext* const pImmediateContext = m_pGraphicsImpl->D3DGetDeviceContext();

      // FIXME: 假定有$Globals
      pImmediateContext->UpdateSubresource(m_arrayCB.front(), 0, NULL, pDataPool->GetRootPtr(), 0, 0);

      pImmediateContext->VSSetConstantBuffers(0, m_pPixelCB - m_pVertexCB, m_pVertexCB);
      pImmediateContext->PSSetConstantBuffers(0, &m_arrayCB.back() - m_pPixelCB + 1, m_pPixelCB);
      return FALSE;
    }

    const ShaderImpl::BINDRESOURCE_DESC* ShaderImpl::FindBindResource(GXLPCSTR szName) const
    {
      if(m_pBindResourceDesc)
      {
        return
          clstd::BinarySearch(m_pBindResourceDesc, m_pBindResourceDesc + m_nBindResourceDesc, szName,
            [](BINDRESOURCE_DESC* pDesc, GXLPCSTR szName) -> int
        {
          return clstd::strcmpT(pDesc->name, szName);
        });
      }
      return NULL;
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
      // *.Bind resource table <- m_pBindResourceDesc

      m_nBindResourceDesc = mapper.arraySampler.size() + mapper.arrayTexture.size();

      if(mapper.GetBufferCount() == 0 && m_nBindResourceDesc == 0) {
        return TRUE;
      }

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

      for(auto it = mapper.arraySampler.begin(); it != mapper.arraySampler.end(); ++it)
      {
        mapper.Strings.add(*it);
      }
      for(auto it = mapper.arrayTexture.begin(); it != mapper.arrayTexture.end(); ++it)
      {
        mapper.Strings.add(*it);
      }

      // 计算总缓冲区大小
      const size_t nStringBufSize = mapper.Strings.buffer_size();
      const size_t nTotalSize = nStringBufSize +
        sizeof(Marimo::DATAPOOL_DECLARATION) * (mapper.aGlobal.size() + mapper.aMembers.size() + mapper.aCBMembers.size() + 1) +
        sizeof(Marimo::DATAPOOL_TYPE_DECLARATION) * (mapper.aConstantBuffers.size() + mapper.aTypes.size() + 1) +
        sizeof(BINDRESOURCE_DESC) * m_nBindResourceDesc;
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
        DataPoolDeclaration_T& rDecl = aDecl[n];
        if(n == 2 && _CL_NOT_(rDecl.empty())) {
          m_pDataPoolDecl = reinterpret_cast<Marimo::DATAPOOL_DECLARATION*>(
            reinterpret_cast<size_t>(m_buffer.GetPtr()) + m_buffer.GetSize());
        }

        for(size_t i = 0; i < rDecl.size(); i++)
        {
          if(rDecl[i].Name && rDecl[i].Type)
          {
            rDecl[i].Name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(rDecl[i].Name));
            if(clstd::strncmpT(rDecl[i].Type, STRUCT_PREFIX_NAME, sizeof(STRUCT_PREFIX_NAME) - 1) == 0 ||
              clstd::strncmpT(rDecl[i].Type, CB_PREFIX_NAME, sizeof(CB_PREFIX_NAME) - 1) == 0) {
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
        static Marimo::DataPoolPack s_aPacks[] = {
          Marimo::DataPoolPack::NotCross16BoundaryShort,  // 这里与D3D11文档写的不一样
          Marimo::DataPoolPack::NotCross16Boundary };

        for(size_t i = 0; i < rType.size(); i++)
        {
          rType[i].Name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(rType[i].Name));
          rType[i].as.Struct = pMemberBase + (size_t)rType[i].as.Struct;
          rType[i].MemberPack = s_aPacks[n];
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

      if(m_nBindResourceDesc)
      {
        m_pBindResourceDesc = reinterpret_cast<BINDRESOURCE_DESC*>(
          reinterpret_cast<size_t>(m_buffer.GetPtr()) + m_buffer.GetSize());
        BINDRESOURCE_DESC brd;

        for(auto it = mapper.arraySampler.begin(); it != mapper.arraySampler.end(); ++it)
        {
          brd.name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(*it));
          brd.type = BindType::Sampler;
          brd.slot = it - mapper.arraySampler.begin();
          m_buffer.Append(&brd, sizeof(BINDRESOURCE_DESC));
        }

        for(auto it = mapper.arrayTexture.begin(); it != mapper.arrayTexture.end(); ++it)
        {
          brd.name = (GXLPCSTR)((size_t)m_buffer.GetPtr() + mapper.Strings.offset(*it));
          brd.type = BindType::Texture;
          brd.slot = it - mapper.arrayTexture.begin();
          m_buffer.Append(&brd, sizeof(BINDRESOURCE_DESC));
        }

        // 排序, 后面用二分查找
        struct BIND : BINDRESOURCE_DESC
        {
          GXBOOL SortCompare(const BIND& b)
          {
            return clstd::strcmpT(name, b.name) > 0;
          }

          void SortSwap(BIND& b)
          {
            BIND t = b;
            b = *this;
            *this = t;
          }
        };

        clstd::BubbleSort(static_cast<BIND*>(m_pBindResourceDesc), m_nBindResourceDesc);
      }

      ASSERT(m_buffer.GetSize() == nTotalSize); // 校验实际填充大小和计算大小
      return TRUE;
    }

    ID3D11Buffer* ShaderImpl::D3D11CreateBuffer(size_t cbSize)
    {
      ID3D11Device* const pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      D3D11_BUFFER_DESC bd;
      InlSetZeroT(bd);
      ASSERT(cbSize > 0);

      bd.Usage          = D3D11_USAGE_DEFAULT;
      bd.ByteWidth      = cbSize;
      bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
      bd.CPUAccessFlags = 0;

      ID3D11Buffer* pD3D11Buffer = NULL;
      HRESULT hr = pd3dDevice->CreateBuffer(&bd, NULL, &pD3D11Buffer);
      if(SUCCEEDED(hr)) {
        return pD3D11Buffer;
      }
      return NULL;
    }

    void ShaderImpl::DbgCheck(INTERMEDIATE_CODE::Array& aInterCode)
    {
      if(m_pDataPoolDecl == NULL) {
        return; // 没有常量
      }
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
      //CLBREAK;
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


//SetVertexShaderConstantB
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)