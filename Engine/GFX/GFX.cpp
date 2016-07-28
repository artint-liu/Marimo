// 全局头文件
#include <GrapX.H>
#include "../GrapX/User/GrapX.Hxx"

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GPrimitive.h>
#include <GrapX/GXGraphics.H>
#include <GrapX/GShader.H>
#include <GrapX/GXCanvas.H>
#include <GrapX/GXCanvas3D.h>
#include <GrapX/GCamera.H>

// 私有头文件
#include "clTree.H"
#include "clTransform.H"
#include <Smart/smartstream.h>
#include "Smart/Stock.h"
#include <GrapX/VertexDecl.H>
#include "GrapX/gxError.H"

#include <GrapX/gvNode.h>
#include <GrapX/gvMesh.h>
#include <GrapX/gvScene.h>

#include "Engine.h"
//#include "GrapX/gxUtility.h"
#include "Engine/GFX.h"
#include "GFXPatch3D.h"
#include "GFXParticles.h"

#define CMP_PARAM(_STR) GXSTRCMP(aDefines[i].szName, _STR) == 0
using namespace clstd;
namespace GFX
{
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  GXHRESULT Create(Element** ppElement, GXGraphics* pGraphics, TypeId eId, GXDEFINITION* aDefines, GXUINT nCount)
  {
    Element* pElement = NULL;
    switch (eId)
    {
    case Patch3D:
      pElement = new Patch3DImpl(pGraphics);
      break;
    case Particles:
      pElement = new ParticlesImpl(pGraphics);
      break;
    }

    if( ! InlCheckNewAndIncReference(pElement)) {
      return GX_FAIL;
    }

    if( ! pElement->Initialize(pGraphics) || ! pElement->SolveParams(pGraphics, aDefines, nCount)) {
      SAFE_RELEASE(pElement);
      return GX_FAIL;
    }

    *ppElement = pElement;
    return GX_OK;
  }

  GXHRESULT GAMEENGINE_API CreateFromFileA( Element** ppElement, GXGraphics* pGraphics, GXLPCSTR szFilename )
  {
    clStringW strFilename = szFilename;
    return CreateFromFileW(ppElement, pGraphics, strFilename);
  }
  
  GXHRESULT GAMEENGINE_API CreateFromFileW( Element** ppElement, GXGraphics* pGraphics, GXLPCWSTR szFilename )
  {
    clStockW sp;
    if( ! sp.LoadW(szFilename)) {
      return GX_E_OPEN_FAILED;
    }
    typedef cllist<Element*> ElementList;
    ElementList listElement;

    GXHRESULT hval = GX_OK;
    //clStockW::Section handle = sp.FindFirstSection(NULL, FALSE, NULL, NULL);
    clStockW::Section handle = sp.Open(NULL);
    if(handle.IsValid()) {
      do {
        clStringW strSect = handle.SectionName(); // sp.GetSectionName(handle);
        TypeId eTypeId = Empty;
        if(strSect == L"particles")
        {
          eTypeId = Particles;
        }
        else if(strSect == L"patch3d")
        {
          eTypeId = Patch3D;
        }
        
        if(eTypeId != Empty)
        {
          clStockW::ATTRIBUTE val;
          handle.FirstKey(val);

          GXDefinitionArray aParams;
          aParams.reserve(30);

          if(val.IsValid())
          {
            do {
              GXDefinition def;
              def.Name = val.KeyName();
              def.Value = val.ToString();
              aParams.push_back(def);
            }while(val.NextKey());

            //sp.FindClose(hParam);
            Element* pSubElement = NULL;
            if(GXSUCCEEDED(Create(&pSubElement, pGraphics, eTypeId, 
              aParams.front(), aParams.size()))) {
                listElement.push_back(pSubElement);
            }
            else {
              hval = GX_OK; // 不完全成功
            }
          }
        }
      }while(handle.NextSection());
      //sp.FindClose(handle);
    }

    if(listElement.empty()) { // 空的Element或者全部创建失败
      return GX_FAIL;
    }
    else if(listElement.size() == 1) {  // 只有一个Element
      *ppElement = listElement.front();
    }
    else { // 多个Element, 要先创建一个容器
      ElementJar* pJar = new ElementJar(pGraphics);
      if( ! InlCheckNewAndIncReference(pJar)) {
        CLOG_ERROR(__FUNCTION__": out of memory.\r\n");
        hval = GX_FAIL;
      }
      else if( ! pJar->Initialize(pGraphics)) {
        hval = GX_FAIL;
      }
      else {
        for(ElementList::iterator it = listElement.begin();
          it != listElement.end(); ++it) {
            (*it)->SetParent(pJar);
        }
        *ppElement = pJar;
      }
    }

    // 如果失败则释放掉已经创建的资源
    if(GXFAILED(hval)) {
      for(ElementList::iterator it = listElement.begin();
        it != listElement.end(); ++it) {
          SAFE_RELEASE(*it);
      }
    }    

    return hval;
  }

  GXBOOL Element::ImportCommonParam( const GXDEFINITION& sDef, COMMONPARAMDESC& sCommParam )
  {
    if(GXSTRCMP(sDef.szName, "texture") == 0) {
      sCommParam.strTexture = sDef.szValue;
    }
    else if(GXSTRCMP(sDef.szName, "shader") == 0) {
      sCommParam.strMaterial = sDef.szValue;
    }
    else if(GXSTRCMP(sDef.szName, "pos") == 0)
    {
      sCommParam.vPos = clstd::strtovec3f(sDef.szValue);
    }
    else {
      return FALSE;
    }
    return TRUE;
  }

  GXUINT Element::ExportCommonParam(GXDefinition* aDef, GXUINT nCount, COMMONPARAMDESC& sCommParam)
  {
    GXUINT i = 0;
    if(nCount > i)
    {
      aDef[i].Name = "shader";
      aDef[i].Value = sCommParam.strMaterial;
      i++;
    }

    if(nCount > i)
    {
      aDef[i].Name = "texture";
      aDef[i].Value = sCommParam.strTexture;
      i++;
    }

    if(nCount > i)
    {
      float3 vPos = GetPosition();
      aDef[i].Name = "pos";
      aDef[i].Value.Format("%f,%f,%f", vPos.x, vPos.y, vPos.z);
      i++;
    }

    return i;
  }

  GXBOOL Element::CreateCommonRes( GXGraphics* pGraphics, COMMONPARAMDESC& sCommParam )
  {
    GXHRESULT hval = GX_OK;
    if(sCommParam.strMaterial.IsNotEmpty()) {
      hval = SetMaterialInstFromFileW(pGraphics, sCommParam.strMaterial, NODEMTL_CLONEINST);
    }

    if(GXSUCCEEDED(hval) && sCommParam.strTexture.IsNotEmpty()) {
      m_pMtlInst->SetTextureByIndexFromFileW(0, sCommParam.strTexture);
    }

    SetPosition(sCommParam.vPos);
    return GXSUCCEEDED(hval);
  }

  GXINT Element::Execute(GXLPCSTR szCmd, GXWPARAM wParam, GXLPARAM lParam)
  {
    GXINT nResult = 0;
    GVNode* pNode = GetFirstChild();
    while(pNode) {
      if(pNode->GetClass() == ClassCode) {
        nResult += static_cast<Element*>(pNode)->Execute(szCmd, wParam, lParam);
      }
      pNode = pNode->GetNext();
    }
    return nResult;
  }

  GXBOOL Element::ImportParam(GXLPVOID pBasePtr, const GXDEFINITION& sDefine, const PROPERTY* aProps)
  {
    const GXDEFINITION& d = sDefine;
    for(GXUINT j = 0; aProps[j].szName != NULL; j++)
    {
      const PROPERTY& p = aProps[j];
      if(GXSTRCMP(d.szName, p.szName) == 0) {
        switch(p.eType)
        {
        case PROPERTY::T_UINT:
          {
            auto& i = p.ToVar<u32>(pBasePtr);
            i = clstd::xtou(d.szValue);
          }
          break;

        case PROPERTY::T_INT:
          {
            auto& i = p.ToVar<int>(pBasePtr);
            i = clstd::xtoi(d.szValue);
          }
          break;

        case PROPERTY::T_INT2:
          {
            auto& i = p.ToVar<int2>(pBasePtr);
            i = clstd::strtovec2i<int>(d.szValue);
          }
          break;

        case PROPERTY::T_INT3:
          {
            auto& i = p.ToVar<int3>(pBasePtr);
            i = clstd::strtovec3i<int>(d.szValue);
          }
          break;

        case PROPERTY::T_FLOAT:
          {
            auto& i = p.ToVar<float>(pBasePtr);
            i = (float)clstd::_xstrtof(d.szValue);
          }
          break;

        case PROPERTY::T_FLOAT2:
          {
            auto& i = p.ToVar<float2>(pBasePtr);
            i = clstd::strtovec2f(d.szValue);
          }
          break;

        case PROPERTY::T_FLOAT3:
          {
            auto& i = p.ToVar<float3>(pBasePtr);
            i = clstd::strtovec3f(d.szValue);
          }
          break;

        case PROPERTY::T_NMFLOAT2:
          {
            auto& v = p.ToVar<float2>(pBasePtr);
            v = clstd::strtovec2f(d.szValue);
            v.normalize();
          }
          break;

        case PROPERTY::T_NMFLOAT3:
          {
            auto& v = p.ToVar<float3>(pBasePtr);
            v = clstd::strtovec3f(d.szValue);
            v.normalize();
          }
          break;

        case PROPERTY::T_RANGEF:
          {
            auto& v = p.ToVar<RANGEF>(pBasePtr);
            float2 v2 = clstd::strtovec2f(d.szValue);
            v._min = v2.x;
            v._max = v2.y;
          }
          break;

        case PROPERTY::T_RANGEI:
          {
            auto& v = p.ToVar<RANGEI>(pBasePtr);
            int2 v2 = clstd::strtovec2i<int>(d.szValue);
            v._min = v2.x;
            v._max = v2.y;
          }
          break;

        case PROPERTY::T_STRINGA:
          {
            auto& v = p.ToVar<clStringA>(pBasePtr);
            v = d.szValue;
          }
          break;

        case PROPERTY::T_STRINGW:
          {
            auto& v = p.ToVar<clStringW>(pBasePtr);
            v = d.szValue;
          }
          break;

        case PROPERTY::T_HALFFLOAT:
          {
            auto& v = p.ToVar<float>(pBasePtr);
            v = (float)clstd::_xstrtof(d.szValue) * 0.5f;
          }
          break;

        case PROPERTY::T_USER:
          return FALSE;

        default:
          if(IS_PTR(p.aEnum)) {
            for(GXUINT k = 0; p.aEnum[k].szName != NULL; k++)
            {
              if(GXSTRCMP(d.szValue, p.aEnum[k].szName) == 0)
              {
                auto& v = p.ToVar<int>(pBasePtr);
                v = p.aEnum[k].nEnum;
                return TRUE;
              }
            }
            return TRUE; // 如果没有找到枚举名则跳过.
          }
          CLBREAK; // 指定了错误类型
          return FALSE;
        }
        return TRUE;
      }
    }
    return FALSE;
  }

  GXBOOL Element::ExportParam(GXLPCVOID pBasePtr, GXDefinition& sDefine, const PROPERTY& sProps)
  {
    clStringA& str = sDefine.Value;
    const PROPERTY& p = sProps;

    sDefine.Name = p.szName;

    switch(p.eType)
    {
    case PROPERTY::T_UINT:
      {
        auto& i = p.ToVar<u32>(pBasePtr);
        str.AppendUInt32(i);
      }
      break;

    case PROPERTY::T_INT:
      {
        auto& i = p.ToVar<int>(pBasePtr);
        str.AppendInteger32(i);
      }
      break;

    case PROPERTY::T_INT2:
      {
        auto& i = p.ToVar<int2>(pBasePtr);
        str.Format("%d,%d", i.x, i.y);
      }
      break;

    case PROPERTY::T_INT3:
      {
        auto& i = p.ToVar<int3>(pBasePtr);
        str.Format("%d,%d,%d", i.x, i.y, i.z);
      }
      break;

    case PROPERTY::T_FLOAT:
      {
        auto& i = p.ToVar<float>(pBasePtr);
        str.AppendFloat(i);
      }
      break;

    case PROPERTY::T_NMFLOAT2:
    case PROPERTY::T_FLOAT2:
      {
        auto& i = p.ToVar<float2>(pBasePtr);
        str.Format("%f,%f", i.x, i.y);
      }
      break;

    case PROPERTY::T_NMFLOAT3:
    case PROPERTY::T_FLOAT3:
      {
        auto& i = p.ToVar<float3>(pBasePtr);
        str.Format("%f,%f,%f", i.x, i.y, i.z);
      }
      break;

    case PROPERTY::T_RANGEF:
      {
        auto& v = p.ToVar<RANGEF>(pBasePtr);
        str.Format("%f,%f", v._min, v._max);
      }
      break;

    case PROPERTY::T_RANGEI:
      {
        auto& v = p.ToVar<RANGEI>(pBasePtr);
        str.Format("%d,%d", v._min, v._max);
      }
      break;

    case PROPERTY::T_STRINGA:
      {
        auto& v = p.ToVar<clStringA>(pBasePtr);
        str = v;
      }
      break;

    case PROPERTY::T_STRINGW:
      {
        auto& v = p.ToVar<clStringW>(pBasePtr);
        str = v;
      }
      break;

    case PROPERTY::T_HALFFLOAT:
      {
        auto& v = p.ToVar<float>(pBasePtr);
        str.AppendFloat(v * 2.0f);
      }
      break;

    case PROPERTY::T_USER:
      return FALSE;

    default:
      if(IS_PTR(p.aEnum)) {
        auto& v = p.ToVar<int>(pBasePtr);
        for(GXUINT k = 0; p.aEnum[k].szName != NULL; k++)
        {
          if(v == p.aEnum[k].nEnum)
          {
            str = p.aEnum[k].szName;
            return TRUE;
          }
        }
      }
      CLBREAK; // 指定了错误类型
      return FALSE;
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  ElementJar::ElementJar( GXGraphics* pGraphics ) : Element(pGraphics, Empty)
  {
  }

  GXBOOL ElementJar::Initialize( GXGraphics* pGraphics )
  {
    SetFlags(GetFlags() | GVNF_CONTAINER);
    return TRUE;
  }

  GXBOOL ElementJar::SolveParams( GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount )
  {
    return TRUE;
  }

  GXUINT ElementJar::MakeParams(GXDefinition* aDefines, GXUINT nArrayCount)
  {
    return 0;
  }

  GXBOOL ElementJar::Update( const GVSCENEUPDATE& sContext )
  {
    return TRUE;
  }

  size_t PROPERTY::SizeOf()
  {
    switch(eType)
    {
    case T_UNKNOWN:   return 0;
    case T_UINT:      return sizeof(u32);
    case T_INT:       return sizeof(i32);
    case T_INT2:      return sizeof(i32) * 2;
    case T_INT3:      return sizeof(i32) * 3;
    case T_FLOAT:     return sizeof(float);
    case T_FLOAT2:    return sizeof(float) * 2;
    case T_FLOAT3:    return sizeof(float) * 3;
    case T_NMFLOAT2:  return sizeof(float) * 2;
    case T_NMFLOAT3:  return sizeof(float) * 3;
    case T_RANGEF:    return sizeof(RANGEF);
    case T_RANGEI:    return sizeof(RANGEI);
    case T_STRINGA:   return sizeof(clStringA);
    case T_STRINGW:   return sizeof(clStringW);
    case T_HALFFLOAT: return sizeof(float);
    case T_USER:      return 0;
    default:
      ASSERT(IS_PTR(aEnum));
      return sizeof(GXINT);
    }
  }
} // namespace GFX
