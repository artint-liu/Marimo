// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
#include <GUnknown.H>
#include <GResource.H>
#include <GPrimitive.h>
#include <GXGraphics.H>
#include <GShader.H>
#include <GXCanvas.H>
#include <GXCanvas3D.h>
#include <GCamera.H>

// 私有头文件
#include "clTree.H"
#include "clTransform.H"
#include <Smart/smartstream.h>
#include "Smart/SmartProfile.h"
#include <Utility/VertexDecl.H>
#include "gxError.H"

#include <3D/gvNode.h>
#include <3D/gvMesh.h>
#include <3D/gvScene.h>

#include "GameEngine.h"
#include "gxUtility.h"
#include "GFX.h"
#include "GFXPatch3D.h"
#include "GFXParticles.h"

#define CMP_PARAM(_STR) GXSTRCMP(aDefines[i].szName, _STR) == 0

namespace GFX
{
  PROPERTY::ENUMLIST Patch3DImpl::s_aEnumAlign[] = {
    {"Toward",       Patch3DImpl::TowardCamera},
    {"Billboard",    Patch3DImpl::Billboard},
    {NULL, 0},
  };

  PROPERTY Patch3DImpl::s_aProperty[] = {
    {PROPERTY::T_HALFFLOAT,   "width",    offsetof(Patch3DImpl, m_fHalfWidth)},
    {PROPERTY::T_HALFFLOAT,   "height",   offsetof(Patch3DImpl, m_fHalfHeight)},
    {PROPERTY::T_NMFLOAT3,    "up",       offsetof(Patch3DImpl, m_vUp)},
    {(INT_PTR)s_aEnumAlign,   "align",    offsetof(Patch3DImpl, m_eAlign)},
    {PROPERTY::T_UNKNOWN,     NULL,       0},
  };


  GXBOOL Patch3DImpl::Update(const GVSCENEUPDATE& sContext)
  {
    float3 vRight;
    float3 vTop;

    CalcAxis(sContext, vTop, vRight);

    GXVERTEX_P3T2F_C1D* pVertices = (GXVERTEX_P3T2F_C1D*)m_pPrimitive->GetVerticesBuffer();
    if(pVertices)
    {
      vRight *= m_fHalfWidth;
      vTop *= m_fHalfHeight;

      pVertices[0].pos =  vRight - vTop;
      pVertices[1].pos = -vRight - vTop;
      pVertices[2].pos = -vRight + vTop;
      pVertices[3].pos =  vRight + vTop;

      m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
    }

    return TRUE;
  }

  GXBOOL Patch3DImpl::Initialize(GXGraphics* pGraphics)
  {
    //GXVERTEXELEMENT 
    VIndex aIndices[] = {0,1,3,2,3,1};
    pGraphics->CreatePrimitiveVI(&m_pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P3T2F_C1D), NULL, 6, 4);
    if(m_pPrimitive) {
      memcpy(m_pPrimitive->GetIndicesBuffer(), aIndices, sizeof(aIndices));

      GXVERTEX_P3T2F_C1D* pVertices = (GXVERTEX_P3T2F_C1D*)m_pPrimitive->GetVerticesBuffer();

      memset(pVertices, 0, sizeof(GXVERTEX_P3T2F_C1D) * 4);
      pVertices[0].texcoord.set(0, 0);
      pVertices[1].texcoord.set(1, 0);
      pVertices[2].texcoord.set(1, 1);
      pVertices[3].texcoord.set(0, 1);

      pVertices[0].color =
      pVertices[1].color =
      pVertices[2].color =
      pVertices[3].color = 0xffffffff;

      m_pPrimitive->UpdateResouce(GPrimitive::ResourceAll);
    }
    m_nPrimiCount = 2;
    m_nVertCount  = 4;
    m_nStartIndex = 0;

    SET_FLAG(m_dwFlags, GVNF_NOCLIP);
    return TRUE;
  }

  GXBOOL Patch3DImpl::SolveParams(GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount)
  {
    COMMONPARAMDESC sCommParam;
    sCommParam.vPos = 0.0f;
    for(GXUINT i = 0; i <nCount; i++)
    {
      if(ImportCommonParam(aDefines[i], sCommParam))
      {
        continue;
      }
      else if(ImportParam(this, aDefines[i], s_aProperty))
      {
        continue;
      }
    }

    CreateCommonRes(pGraphics, sCommParam);
    return TRUE;
  }

  void Patch3DImpl::CalcAxis( const GVSCENEUPDATE& sContext, float3& vTop, float3& vRight )
  {
    float3 vFront = sContext.pCamera->GetFront();

    if(m_eAlign == TowardCamera)
    {
      vRight = float3::cross(m_vUp, vFront);
      vRight.normalize();
      vTop = m_vUp;
    }
    else if(m_eAlign == Billboard)
    {
      if(m_vUp == float3::AxisY)
      {
        vRight = sContext.pCamera->GetRight();
        vTop = sContext.pCamera->GetTop();
      }
      else
      {
        GCAMERACONETXT ctx;
        ctx.dwMask = GCC_VIEW | GCC_PROJECTION;
        sContext.pCamera->GetContext(&ctx);

        float4x4 matInvView = float4x4::inverse(ctx.matView);
        float3 vUp = m_vUp * matInvView;

        vRight = float3::cross(vUp, vFront);
        vRight.normalize();

        vTop = float3::cross(vFront, vRight);
        vTop.normalize();
      }
    }
    else {
      CLBREAK;  // 错误的枚举
    }
  }

  Patch3DImpl::Patch3DImpl( GXGraphics* pGraphics ) : Element(pGraphics, Patch3D)
    , m_fHalfWidth(0.5f)
    , m_fHalfHeight(0.5f)
    , m_vUp(0,1,0)
    , m_eAlign(TowardCamera)
  {
  }

  GXUINT Patch3DImpl::MakeParams(GXDefinition* aDefines, GXUINT nArrayCount)
  {
    return 0;
  }

  //////////////////////////////////////////////////////////////////////////
} // namespace GFX
