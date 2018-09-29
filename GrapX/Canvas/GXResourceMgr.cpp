#include "GrapX.h"
#include "User/GrapX.Hxx"
#include "GrapX/GResource.h"
#include "GrapX/GXUser.h"
#include "GXResourceMgr.h"

//////////////////////////////////////////////////////////////////////////
extern u32 chksum_crc32 (unsigned char *block, unsigned int length);
//////////////////////////////////////////////////////////////////////////

GXDWORD GXDLLAPI MOGenerateVertexDeclCode(LPCGXVERTEXELEMENT pVertDecl)
{
  const int nCount = GetVertexDeclLength<int>(pVertDecl);
  return clstd::GenerateCRC32((GXBYTE*)pVertDecl, nCount * sizeof(GXVERTEXELEMENT));
}

namespace GrapXInternal
{
  GXResourceMgr::GXResourceMgr()
  {
  }
  
  GXResourceMgr::~GXResourceMgr()
  {
  }

  GXLRESULT GXResourceMgr::Initialize()
  {
    return GX_OK;
  }

  GXLRESULT GXResourceMgr::Finalize()
  {
    return GX_OK;
  }

  GXHRESULT GXResourceMgr::RegisterUnfeatured(GResource* pResource)
  {
    GRESKETCH Desc = {NULL};
    return Register(&Desc, pResource);
  }

  GXHRESULT GXResourceMgr::Register(LPCRESKETCH pDesc, GResource* pResource)
  {
    // 1.按资源特征注册
    // $1.分类节点
    CateDict::iterator itCate = m_CategoryDict.find(pDesc->dwCategoryId);

    if(itCate == m_CategoryDict.end()) // 分类还没有创建
    {
      ResNameDict sResNameDict;
      if(pDesc->strResourceName.IsNotEmpty())
      {
        sResNameDict[pDesc->strResourceName] = pResource;
      }
      m_CategoryDict[pDesc->dwCategoryId] = sResNameDict;
    }
    else if(pDesc->strResourceName.IsNotEmpty()) // 分类已经创建, 资源特征不为空 
    {
      ResNameDict& rResNameDict = itCate->second;
      if(rResNameDict.find(pDesc->strResourceName) != rResNameDict.end())
      {
        ASSERT(0); // 重复注册
        return -1;
      }
      rResNameDict[pDesc->strResourceName] = pResource;
    }

    // 2.按照指针注册
    // $.查找重复注册的资源
    if(m_ResourceDict.find(pResource) != m_ResourceDict.end())
    {
      ASSERT(0); // 重复注册
      return -2;
    }

    m_ResourceDict[pResource] = *pDesc;
    return GX_OK;
  }

  GXHRESULT GXResourceMgr::Unregister(GResource* pResource)
  {
    ResDict::iterator itRes = m_ResourceDict.find(pResource);
    if(itRes == m_ResourceDict.end())
    {
      TRACE("Released by wrong resource manager.\n");
      ASSERT(0);
      return -1;
    }
    GRESKETCH& ResFeatureDesc = itRes->second;

    CateDict::iterator itCate = m_CategoryDict.find(ResFeatureDesc.dwCategoryId);
    if(itCate != m_CategoryDict.end())
    {
      ResNameDict& rResNameDict = itCate->second;
      if(ResFeatureDesc.strResourceName.IsNotEmpty())
      {
        ResNameDict::iterator itResId = rResNameDict.find(ResFeatureDesc.strResourceName);
        if(itResId != rResNameDict.end())
        {
          rResNameDict.erase(itResId);
        }
        if(rResNameDict.size() == 0)
        {
          m_CategoryDict.erase(itCate);
        }
      }
    }

    m_ResourceDict.erase(pResource);
    return GX_OK;
  }

  GXHRESULT GXResourceMgr::BroadcastScriptCommand(GRESCRIPTDESC* pDesc)
  {
    pDesc->bBroadcast = TRUE;
    pDesc->dwTime = gxGetTickCount();

    int nPriorityCount[4] = {INT_MAX, 0, 0, 0};
#ifdef _DEBUG
    int nDebugIdx = 0;
#endif // #ifdef _DEBUG

    for(GXUINT nPriorityIndex = 0; nPriorityIndex < 4; nPriorityIndex++)
    {
      if(nPriorityCount[nPriorityIndex] == 0) {
        continue;
      }
      for(ResDict::iterator it = m_ResourceDict.begin();
        it != m_ResourceDict.end(); ++it)
      {
        GResource* pResource = it->first;
        const GXUINT nThisPriority = pResource->GetPriority();

        if(nThisPriority == nPriorityIndex) { // 如果是当前优先级则实现调用
          pResource->Invoke(pDesc);
#ifdef _DEBUG
          nDebugIdx++;
#endif // #ifdef _DEBUG
        }
        else if(nThisPriority == nPriorityIndex + 1) { // 收集下个优先级对象的数量
          nPriorityCount[nThisPriority]++;
        }
      }
    }
    return GX_OK;
  }

  GResource* GXResourceMgr::Find(LPCRESKETCH pDesc) const
  {
    // 无效或者大众资源则直接返回
    if(pDesc == NULL || pDesc->dwCategoryId == NULL) {
      return NULL;
    }

    CateDict::const_iterator itCate = m_CategoryDict.find(pDesc->dwCategoryId);
    if(itCate != m_CategoryDict.end())
    {
      const ResNameDict& rResNameDict = itCate->second;
      ResNameDict::const_iterator it = rResNameDict.find(pDesc->strResourceName);
      if(it != rResNameDict.end())
      {
        return it->second;
      }
    }
    return NULL;
  }

  LPCRESKETCH GXResourceMgr::Find(GResource* pResource) const
  {
    ResDict::const_iterator it = m_ResourceDict.find(pResource);
    if(it != m_ResourceDict.end())
    {
      return &it->second;
    }
    return NULL;
  }

  GXHRESULT GXResourceMgr::BroadcastCategoryMessage(GXDWORD dwCategoryId, GRESCRIPTDESC* pDesc)
  {
    CateDict::iterator it = m_CategoryDict.find(dwCategoryId);
    if(it == m_CategoryDict.end()) {
      return GX_FAIL;
    }

    ResNameDict& sCateRes = it->second;
    for(ResNameDict::iterator it = sCateRes.begin(); it != sCateRes.end(); ++it) {
      it->second->Invoke(pDesc);
    }

    return GX_OK;
  }

  namespace ResourceSketch
  {
    GXLRESULT GenerateTexture(GRESKETCH* pDesc, GXLPCWSTR pSrcFile, 
      GXUINT Width, GXUINT Height, GXUINT Depth, 
      GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, 
      GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey)
    {
      pDesc->dwCategoryId = RCC_Texture;
      pDesc->strResourceName.Format(L"%s,%d,%d,%d,%u,%d,%u,%u,%u,%08x", 
        pSrcFile, Width, Height, Depth, MipLevels, Format, ResUsage, Filter, MipFilter, ColorKey);

      return 0;
    }

    GXLRESULT GenerateTextureNameA(GRESKETCH* pDesc, GXLPCSTR szName)
    {
      pDesc->dwCategoryId = RCC_TextureName;
      pDesc->strResourceName = szName;
      return 0;
    }

    GXLRESULT GenerateImage(GRESKETCH* pDesc, GXLPCWSTR lpwszFilename)
    {
      pDesc->dwCategoryId = RCC_Image;
      pDesc->strResourceName = lpwszFilename;

      return 0;
    }

    template<class _StringT, typename _Ty, typename _LPLOGFONTT>
    GXLRESULT GenerateFontT(GRESKETCH* pDesc, GXDWORD dwCategory, const _Ty* szFormat, const _LPLOGFONTT lpLogFont)
    {
      //_StringT strCategory;
      _StringT strResource;
      _StringT strFontName = lpLogFont->lfFaceName;

      //strCategory = szCategory;//L"FontIndirectW";

      if(strFontName.Find('\\') != clStringA::npos ||
        strFontName.Find('.') != clStringA::npos ||
        strFontName.Find(':') != clStringA::npos)
      {
        strFontName.MakeLower();
      }

      strResource.Format(szFormat,
        lpLogFont->lfHeight, lpLogFont->lfWidth,
        lpLogFont->lfEscapement, lpLogFont->lfOrientation,
        lpLogFont->lfWeight, lpLogFont->lfItalic,
        lpLogFont->lfUnderline, lpLogFont->lfStrikeOut,
        lpLogFont->lfCharSet, lpLogFont->lfOutPrecision,
        lpLogFont->lfClipPrecision, lpLogFont->lfQuality,
        lpLogFont->lfPitchAndFamily, lpLogFont->lfFaceName);

      pDesc->dwCategoryId = dwCategory;
      pDesc->strResourceName = strResource;

      return 0;
    }

    //GXLRESULT GenerateFontW(GRESKETCH* pDesc, const GXLPLOGFONTW lpLogFont)
    //{
    //  return GenerateFontT<clStringW, GXWCHAR, GXLPLOGFONTW>(pDesc, RCC_FontW, 
    //    L"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s", lpLogFont);
    //}

    GXLRESULT GenerateFontA(GRESKETCH* pDesc, const GXLPLOGFONTA lpLogFont)
    {
      return GenerateFontT<clStringA, GXCHAR, GXLPLOGFONTA>(pDesc, RCC_FontA,
        "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s", lpLogFont);
    }

    GXLRESULT GenerateSpriteW(GRESKETCH* pDesc, GXLPCWSTR pszSpriteFile)
    {
      pDesc->dwCategoryId = RCC_SpriteW;
      pDesc->strResourceName = pszSpriteFile;

      return 0;
    }

    //GXLRESULT GenerateSpriteA(GRESKETCH* pDesc, GXLPCSTR pszSpriteFile)
    //{
    //  pDesc->dwCategoryId = RCC_SpriteA;
    //  pDesc->strResourceName = pszSpriteFile;

    //  return 0;
    //}

    //GXLRESULT GenerateShaderFileW(GRESKETCH* pDesc, GXLPCWSTR szFilename, const GXDEFINITION* pMacros)
    //{
    //  return GX_OK;
    //}

    GXLRESULT GenerateShaderElementA(GRESKETCH* pDesc, const MOSHADER_ELEMENT_SOURCE* pSdrElementSrc, const GXDEFINITION* pMacros)
    {
      pDesc->dwCategoryId = RCC_Shader;
      pDesc->strResourceName.Clear();

      pDesc->strResourceName.Append(pSdrElementSrc->strPreVS);
      pDesc->strResourceName.Append('|');
      pDesc->strResourceName.Append(pSdrElementSrc->strVS);
      pDesc->strResourceName.Append('|');
      //pDesc->strResourceName.Append(pSdrElementSrc->strVSExtra);
      //pDesc->strResourceName.Append('|');
      pDesc->strResourceName.Append(pSdrElementSrc->strPS);
      pDesc->strResourceName.Append('|');
      pDesc->strResourceName.Append(pSdrElementSrc->strVSComposer);
      pDesc->strResourceName.Append('|');
      pDesc->strResourceName.Append(pSdrElementSrc->strPSComposer);
      //pDesc->strResourceName.Append('{');
      //for(clStringArrayA::const_iterator it = pSdrElementSrc->aPSComponent.begin();
      //  it != pSdrElementSrc->aPSComponent.end(); ++it) {
      //  pDesc->strResourceName.Append(*it);
      //}
      //pDesc->strResourceName.Append('}');

      // 文件名转换为小写, 下面的宏不做转换处理!
      pDesc->strResourceName.MakeLower();
      pDesc->strResourceName.Append(pSdrElementSrc->strMacros);

      return GX_OK;
    }

    GXLRESULT GenerateMaterialDescW(GRESKETCH* pDesc, GXLPCWSTR szShaderDesc)
    {
      pDesc->dwCategoryId = RCC_MaterialDesc;
      pDesc->strResourceName = szShaderDesc;
      return GX_OK;
    }

    GXLRESULT GenerateVertexDecl(GRESKETCH* pDesc, LPCGXVERTEXELEMENT lpVertexElement)
    {
      pDesc->dwCategoryId = RCC_VertexDecl;

      int i = 0;
      while((GXINT)lpVertexElement[i].UsageIndex >= 0) {
        pDesc->strResourceName.AppendFormat(L"%08x-", 
          chksum_crc32((unsigned char*)&lpVertexElement[i], sizeof(GXVERTEXELEMENT)));
        i++;
      }
      //unsigned int uSize = GetVertexDeclLength<unsigned int>(lpVertexElement) * sizeof(GXVERTEXELEMENT);
      //pDesc->strResourceName = chksum_crc32((unsigned char*)lpVertexElement, uSize);

      return GX_OK;
    }

    GXLRESULT GenerateRasterizerState(GRESKETCH* pDesc, const GXRASTERIZERDESC* pStateDesc)
    {
      STATIC_ASSERT(sizeof(GXRASTERIZERDESC) == 9 * 4); // 防止DESC结构体修改导致这里收集错误而设置
      ASSERT(pStateDesc->FillMode <= 3);
      ASSERT(pStateDesc->CullMode <= 3);
      struct CONTEXT
      {
        union
        {
          struct
          {
            GXDWORD dwFillMode : 2;
            GXDWORD dwCullMode : 2;
            GXDWORD dwFrontCounterClockwise : 1;
            GXDWORD dwDepthClipEnable : 1;
            GXDWORD dwScissorEnable : 1;
          };
          GXDWORD dwCode;
        };
      }ctx;
      STATIC_ASSERT(sizeof(CONTEXT) == sizeof(ctx.dwCode));
      ctx.dwFillMode              = pStateDesc->FillMode;
      ctx.dwCullMode              = pStateDesc->CullMode;
      ctx.dwFrontCounterClockwise = pStateDesc->FrontCounterClockwise;
      ctx.dwDepthClipEnable       = pStateDesc->DepthClipEnable;
      ctx.dwScissorEnable         = pStateDesc->ScissorEnable;
      
      pDesc->dwCategoryId = RCC_RasterizerState;
      pDesc->strResourceName.Format(
        L"%02x|%08x|%08x|%08x", 
        ctx.dwCode, 
        pStateDesc->DepthBias, 
        pStateDesc->DepthBiasClamp, 
        pStateDesc->SlopeScaledDepthBias);

      return GX_OK;
    }

    GXLRESULT GenerateBlendState(GRESKETCH* pDesc, const GXBLENDDESC* pStateDesc)
    {
      STATIC_ASSERT(sizeof(GXBLENDDESC) == 8 * 4 + 4);  // 防止DESC结构体修改导致这里收集错误而设置
      ASSERT(pStateDesc->SrcBlend < 32);
      ASSERT(pStateDesc->DestBlend < 32);
      ASSERT(pStateDesc->BlendOp < 8);
      ASSERT(pStateDesc->SrcBlendAlpha < 32);
      ASSERT(pStateDesc->DestBlendAlpha < 32);
      ASSERT(pStateDesc->BlendOpAlpha < 8);

      struct CONTEXT
      {
        union
        {
          struct
          {
            GXDWORD dwBlendEnable : 1;
            GXDWORD dwSrcBlend : 5;
            GXDWORD dwDestBlend : 5;
            GXDWORD dwBlendOp : 3;
            GXDWORD dwSeparateAlphaBlend : 1;
            GXDWORD dwSrcBlendAlpha : 5;
            GXDWORD dwDestBlendAlpha : 5;
            GXDWORD dwBlendOpAlpha : 3;
            GXDWORD dwWriteMask : 8;

          };
          GXDWORD dwCode[2];
        };
        CONTEXT()
        {
          dwCode[0] = 0;
          dwCode[1] = 0;
        }
      }ctx;
      //GXDWORD dwCode = 
      STATIC_ASSERT(sizeof(CONTEXT) == sizeof(ctx.dwCode));

      ctx.dwBlendEnable        = pStateDesc->BlendEnable;
      ctx.dwSrcBlend           = pStateDesc->SrcBlend;
      ctx.dwDestBlend          = pStateDesc->DestBlend;
      ctx.dwBlendOp            = pStateDesc->BlendOp;
      ctx.dwSeparateAlphaBlend = pStateDesc->SeparateAlphaBlend;
      ctx.dwSrcBlendAlpha      = pStateDesc->SrcBlendAlpha;
      ctx.dwDestBlendAlpha     = pStateDesc->DestBlendAlpha;
      ctx.dwBlendOpAlpha       = pStateDesc->BlendOpAlpha;
      ctx.dwWriteMask          = pStateDesc->WriteMask;

      pDesc->dwCategoryId = RCC_BlendState;
      pDesc->strResourceName.Format(
        L"%02x%08x", 
        ctx.dwCode[1], ctx.dwCode[0]);

      return GX_OK;
    }

    GXLRESULT GenerateDepthStencilState(GRESKETCH* pDesc, const GXDEPTHSTENCILDESC* pStateDesc)
    {
      STATIC_ASSERT(sizeof(GXDEPTHSTENCILDESC) == 4 * 4 + 4 + 4 * 4 * 2); // 防止DESC结构体修改导致这里收集错误而设置

      ASSERT(pStateDesc->DepthFunc - 1 < 8);
      ASSERT(pStateDesc->FrontFace.StencilFailOp - 1 < 8);
      ASSERT(pStateDesc->FrontFace.StencilDepthFailOp - 1 < 8);
      ASSERT(pStateDesc->FrontFace.StencilPassOp - 1 < 8);
      ASSERT(pStateDesc->FrontFace.StencilFunc - 1 < 8);
      ASSERT(pStateDesc->BackFace.StencilFailOp - 1 < 8);
      ASSERT(pStateDesc->BackFace.StencilDepthFailOp - 1 < 8);
      ASSERT(pStateDesc->BackFace.StencilPassOp - 1 < 8);
      ASSERT(pStateDesc->BackFace.StencilFunc - 1 < 8);

      struct CONTEXT
      {
        union
        {
          struct
          {
            GXDWORD dwDepthWriteMask;
            GXDWORD dwDepthEnable : 1;
            GXDWORD dwDepthFunc : 3;
            GXDWORD dwStencilEnable : 1;
            GXDWORD dwStencilReadMask : 8;
            GXDWORD dwStencilWriteMask : 8;
            GXDWORD dwFrontFace_StencilFailOp : 3;
            GXDWORD dwFrontFace_StencilDepthFailOp : 3;
            GXDWORD dwFrontFace_StencilPassOp : 3;
            GXDWORD dwFrontFace_StencilFunc : 3;
            GXDWORD dwBackFace_StencilFailOp : 3;
            GXDWORD dwBackFace_StencilDepthFailOp : 3;
            GXDWORD dwBackFace_StencilPassOp : 3;
            GXDWORD dwBackFace_StencilFunc : 3;
          };
          GXDWORD dwCode[3];
        };
      }ctx;

      STATIC_ASSERT(sizeof(CONTEXT) == sizeof(ctx.dwCode));

      ctx.dwDepthEnable                   = pStateDesc->DepthEnable;
      ctx.dwDepthWriteMask                = pStateDesc->DepthWriteMask;
      ctx.dwDepthFunc                     = pStateDesc->DepthFunc - 1;
      ctx.dwStencilEnable                 = pStateDesc->StencilEnable;
      ctx.dwStencilReadMask               = pStateDesc->StencilReadMask;
      ctx.dwStencilWriteMask              = pStateDesc->StencilWriteMask;
      ctx.dwFrontFace_StencilFailOp       = pStateDesc->FrontFace.StencilFailOp - 1;
      ctx.dwFrontFace_StencilDepthFailOp  = pStateDesc->FrontFace.StencilDepthFailOp - 1;
      ctx.dwFrontFace_StencilPassOp       = pStateDesc->FrontFace.StencilPassOp - 1;
      ctx.dwFrontFace_StencilFunc         = pStateDesc->FrontFace.StencilFunc - 1;
      ctx.dwBackFace_StencilFailOp        = pStateDesc->BackFace.StencilFailOp - 1;
      ctx.dwBackFace_StencilDepthFailOp   = pStateDesc->BackFace.StencilDepthFailOp - 1;
      ctx.dwBackFace_StencilPassOp        = pStateDesc->BackFace.StencilPassOp - 1;
      ctx.dwBackFace_StencilFunc          = pStateDesc->BackFace.StencilFunc;

      pDesc->dwCategoryId = RCC_DepthStencilState;
      pDesc->strResourceName.Format(
        L"%08x|%08x|%08x", 
        ctx.dwCode[0], ctx.dwCode[1], ctx.dwCode[2]);

      return GX_OK;
    }

    /*
    GXLRESULT GenerateSamplerState(GRESKETCH* pDesc, const GXSAMPLERDESC* pStateDesc)
    {
      STATIC_ASSERT(sizeof(GXSAMPLERDESC) == 6 * 4 + 4 * 4);  // 防止DESC结构体修改导致这里收集错误而设置

      ASSERT(pStateDesc->AddressU < 8);
      ASSERT(pStateDesc->AddressV < 8);
      ASSERT(pStateDesc->AddressW < 8);
      ASSERT(pStateDesc->MagFilter < 8);
      ASSERT(pStateDesc->MinFilter < 8);
      ASSERT(pStateDesc->MipFilter < 8);
      struct CONTEXT
      {
        union{
          struct
          {
            GXDWORD AddressU : 3;
            GXDWORD AddressV : 3;
            GXDWORD AddressW : 3;
            GXDWORD MagFilter : 3;
            GXDWORD MinFilter : 3;
            GXDWORD MipFilter : 3;
          };
          GXDWORD dwCode;
        };
      }ctx;
      STATIC_ASSERT(sizeof(CONTEXT) == sizeof(ctx.dwCode));
      //GXColor               BorderColor;

      ctx.AddressU  = pStateDesc->AddressU;
      ctx.AddressV  = pStateDesc->AddressV;
      ctx.AddressW  = pStateDesc->AddressW;
      ctx.MagFilter = pStateDesc->MagFilter;
      ctx.MinFilter = pStateDesc->MinFilter;
      ctx.MipFilter = pStateDesc->MipFilter;

      pDesc->dwCategoryId = GXMAKEFOURCC('S','A','P','T');
      pDesc->strResourceName.Format(
        L"%08x|%08x|%08x|%08x|%08x", 
        ctx.dwCode, 
        pStateDesc->BorderColor.a,
        pStateDesc->BorderColor.r,
        pStateDesc->BorderColor.g,
        pStateDesc->BorderColor.b);

      return GX_OK;
    }
    //*/

  } // namespace ResourceSketch
} // namespace GrapXInternal
