#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
//#include "GrapX/GXCanvas.h"
#include "GrapX/GShader.h"
//#include "GrapX/GTexture.h"
//#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
//#include "GrapX/DataPool.h"
//#include "GrapX/DataPoolVariable.h"
#include "Platform/Win32_XXX.h"
//#include "Platform/Win32_D3D9.h"
//#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"

// 私有头文件
#include <GrapX/VertexDecl.h>
//#include "Canvas/GXResourceMgr.h"
//#include "GrapX/GXCanvas3D.h"
//#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.h"
#include "clPathFile.h"
#include "Smart/smartstream.h"
#include "GDI/GXShaderMgr.h"
//#define PS_REG_IDX_SHIFT 16
//#define PS_REG_IDX_PART  (1 << PS_REG_IDX_SHIFT)

#define PS_HANDLE_SHIFT 16

using namespace clstd;

#include <Smart/smartstream.h>
#include <clTokens.h>
#include <clStock.h>

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
  clStockA sp;
  clStringA strSect = clStringA("shader/") + szPlatformSect;
  clStringW strFilename;
  clStringA strMacros;

  GShader::ResolveProfileDescW(szShaderDesc, &strFilename, &strMacros);
  if(strFilename.IsEmpty()) {
    return FALSE;
  }

  if(!IsFullPath(strFilename)) {
    clpathfile::CombinePath(strFilename, szResourceDir, strFilename);
  }

  if(!sp.LoadFromFile(strFilename)) {
    CLOG_WARNINGW(L"GShader : Can not open file(%s).\n", strFilename);
    return GX_E_OPEN_FAILED;
  }

  clStringA strResourceDirA(szResourceDir);
  MOSHADER_ELEMENT_SOURCE& ses = *pElement;
  GShader::LoadElementSource(&sp, strSect, pElement, pMtlParam == NULL ? NULL : &pMtlParam->aBindPool);
  ses.strMacros = strMacros;

  if(pElement->strPreVS.IsNotEmpty() && !IsFullPath(ses.strPreVS))
    clpathfile::CombinePath(ses.strPreVS, strResourceDirA, ses.strPreVS);

  if(ses.strVS.IsNotEmpty() && !IsFullPath(ses.strVS))
    clpathfile::CombinePath(ses.strVS, strResourceDirA, ses.strVS);

  //if(ses.strVSExtra.IsNotEmpty() && ! IsFullPath(ses.strVSExtra))
  //  clpathfile::CombinePathA(ses.strVSExtra, strResourceDirA, ses.strVSExtra);

  if(ses.strPS.IsNotEmpty() && !IsFullPath(ses.strPS))
    clpathfile::CombinePath(ses.strPS, strResourceDirA, ses.strPS);

  if(ses.strVSComposer.IsNotEmpty() && !IsFullPath(ses.strVSComposer))
    clpathfile::CombinePath(ses.strVSComposer, strResourceDirA, ses.strVSComposer);

  if(ses.strPSComposer.IsNotEmpty() && !IsFullPath(ses.strPSComposer))
    clpathfile::CombinePath(ses.strPSComposer, strResourceDirA, ses.strPSComposer);

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

GXBOOL GShader::LoadElementSource(clStockA* pSmart, GXLPCSTR szSection, MOSHADER_ELEMENT_SOURCE* pElement, clStringArrayA* aDataPool)
{
  typedef clStockA::Section Section;
  typedef clStockA::ATTRIBUTE ATTRIBUTE;

  //clStringA strSect = clStringA("shader\\") + szPlatformSect;
  //clStringA strComponentSect = clStringA(szSection) + "\\PixelComponent";


  Section handle = pSmart->OpenSection(szSection);
  //sHANDLE hPixelCpn = pSmart->OpenSection(strComponentSect);

  if(!handle) {
    return GX_ERROR_HANDLE;
  }
  //MOSHADER_ELEMENT_SOURCE ses;
  //clStringArrayA aPixelComponentKeys;
  pElement->bExtComposer = handle.GetKeyAsBoolean("ExternalComposer", FALSE);
  pElement->strPreVS = handle.GetKeyAsString("PreVertexShader", "");
  pElement->strVS = handle.GetKeyAsString("VertexShader", "");

  pElement->strPS = handle.GetKeyAsString("PixelShader", "");
  pElement->strVSComposer = handle.GetKeyAsString("VertexShaderComposer", "");
  pElement->strPSComposer = handle.GetKeyAsString("PixelShaderComposer", "");

  if(aDataPool != NULL)
  {
    clStringA strDataPool = handle.GetKeyAsString("DataPool", "");
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
  //pSmart->FindClose(handle);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
inline GXBOOL IntLoadShaderComponent(const clStringA& strFilename, clFile& File, GXBOOL bCompiled, clBuffer** ppComponentBuf, GXLPCSTR szFailedText)
{
  if(strFilename.IsNotEmpty())
  {
    if(File.OpenExisting(strFilename)) {
      if(File.MapToBuffer(ppComponentBuf)) {
        // TODO: 应该优化,避免插入的内存复制
        if(!bCompiled)  // 不是编译格式的话就插入 #line 宏
        {
          clStringA strFileDesc;
          ASSERT(!clpathfile::IsRelative(strFilename));
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
  clBuffer*& pPixelBuffer = pSources->pPixelShader;
  clBuffer* pComponentBuf = NULL;
  clBuffer* pDeclCodesBuf = NULL;
  GXBOOL result = TRUE;
  //GXHRESULT hr = GX_OK;
  //clvector<GXDefinition> aMacros; // 这个要放在这里, 避免在其他的作用域中析构

  clsize pos;
  // 只有扩展名为"HLSL"才认为是源代码
  pos = clpathfile::FindExtension(pSrcComponent->strVS);
  const GXBOOL bCompiledVS = (pos <= 0 || GXSTRCMPI(&pSrcComponent->strVS[(int)pos], ".hlsl") != 0);
  pos = clpathfile::FindExtension(pSrcComponent->strPS);
  const GXBOOL bCompiledPS = (pos <= 0 || GXSTRCMPI(&pSrcComponent->strPS[(int)pos], ".hlsl") != 0);

  // TODO: 二进制数据和源码数据可能会有混合错误的问题...
  if(!IntLoadShaderComponent(pSrcComponent->strVS, File, bCompiledVS, &pVertexBuffer, "Can't load VertexShader(%s).\n")) {
    result = FALSE;
  }

  // 如果文件名一致则跳过磁盘IO直接复制一份
  if(GXSTRCMPI<ch>(pSrcComponent->strVS, pSrcComponent->strPS) == 0)
  {
    if(result)
    {
      pPixelBuffer = new clBuffer;
      pPixelBuffer->Append(pVertexBuffer->GetPtr(), pVertexBuffer->GetSize());
    }
  }
  else if(!IntLoadShaderComponent(pSrcComponent->strPS, File, bCompiledPS, &pPixelBuffer, "Can't load PixelShader(%s).\n")) {
    result = FALSE;
  }
  GXDEFINITION* pShaderMacro = NULL;
  int nCodeLenWithoutSwitcherMacro = 0; // pDeclCodesBuf 不带宏开关时的长度
  if(!(bCompiledPS && bCompiledVS))
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
    if(!bCompiledVS)
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

    if(!bCompiledPS)
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

    if(!bCompiledVS)
    {
      const GXBOOL bComposing = pSrcComponent->strVSComposer.IsNotEmpty();
      if(pDeclCodesBuf) {
        pVertexBuffer->Insert(0, pDeclCodesBuf->GetPtr(), bComposing ? pDeclCodesBuf->GetSize() : nCodeLenWithoutSwitcherMacro);
      }
    }

    if(!bCompiledPS)
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

GXBOOL GShader::LoadUniformSet(clStockA* pSmart, GXLPCSTR szSection, ParamArray* aUniforms)
{
  typedef clStockA::Section Section;
  typedef clStockA::ATTRIBUTE ATTRIBUTE;

  clStringA strSection = clStringA(szSection) + "/" + "parameter";

  //for(int i = 0; i < 2; i++)
  {
    Section hParam = pSmart->OpenSection(strSection);
    if(hParam)
    {
      ATTRIBUTE val;
      if(hParam.FirstKey(val)) {
        do {
          GXDefinition Def;
          Def.Name = val.KeyName();
          Def.Value = val.ToString();
          aUniforms->push_back(Def);
        } while(val.NextKey());
      }

      //sHANDLE hSect = pSmart->FindFirstSection(hParam, FALSE, NULL, NULL);
      Section hSect = pSmart->OpenSection(strSection);
      GXDefinition Def;

      if(hSect)
      {
        hSect = hSect.Open(NULL);

        if(hSect)
        {
          do {
            if(hSect.FirstKey(val))
            {
              Def.Name = "SAMPLER";
              Def.Value = hSect.SectionName();
              aUniforms->push_back(Def);
              do {
                Def.Name = val.KeyName();
                Def.Value = val.ToString();
                aUniforms->push_back(Def);
              } while(val.NextKey());
            }
            Def.Name = "SAMPLER";
            Def.Value = "";
            aUniforms->push_back(Def);
            //pSmart->CloseHandle(hKeySampler);
          } while(hSect.NextSection());
          //pSmart->CloseHandle(hSect);
        }
      }
      //pSmart->CloseHandle(hKey);
      //pSmart->CloseHandle(hParam);
    }
  }
  return TRUE;
}

GXBOOL GShader::LoadStateSet(clStockA* pSmart, GXLPCSTR szSection, ParamArray* aStates)
{
  typedef clStockA::Section Section;
  typedef clStockA::ATTRIBUTE ATTRIBUTE;

  for(int i = 0; i < 2; i++)
  {
    Section hParam = pSmart->OpenSection(clStringA(szSection) + "/state");
    if(hParam)
    {
      ATTRIBUTE val;
      //Section hKey = pSmart->FindFirstKey(hParam, val);
      if(hParam.FirstKey(val)) {
        do {
          GXDefinition Def;
          Def.Name = val.KeyName();
          Def.Value = val.ToString();
          aStates->push_back(Def);
        } while(val.NextKey());
      }

      //pSmart->CloseHandle(hKey);
      //pSmart->CloseHandle(hParam);
    }
  }
  return TRUE;
}

//SetVertexShaderConstantB
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
