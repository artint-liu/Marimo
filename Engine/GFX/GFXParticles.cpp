// 全局头文件
#include <GrapX.H>
#include <../GrapX/User/GrapX.Hxx>

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
#include "Smart/SmartProfile.h"
#include <GrapX/VertexDecl.H>
#include "GrapX/gxError.H"

#include <GrapX/gvNode.h>
#include <GrapX/gvMesh.h>
#include <GrapX/gvScene.h>

#include "Engine.h"
//#include "gxUtility.h"
#include "Engine/GFX.h"
#include "GFXParticles.h"

#define CMP_PARAM(_STR) GXSTRCMP(aDefines[i].szName, _STR) == 0

namespace GFX
{
  //////////////////////////////////////////////////////////////////////////
  PROPERTY::ENUMLIST aEnumAlign[] = {
    {"Toward", ParticlesImpl::TowardCamera},
    {"Billboard", ParticlesImpl::Billboard},
    {NULL, 0},
  };
  PROPERTY ParticlesImpl::s_aProperty[] = {
    {PROPERTY::T_HALFFLOAT, "width",                 offsetof(ParticlesImpl, m_fHalfWidth)},
    {PROPERTY::T_HALFFLOAT, "height",                offsetof(ParticlesImpl, m_fHalfHeight)},
    {PROPERTY::T_FLOAT,     "extentrange",            offsetof(ParticlesImpl, m_fExtentRange)},
    {PROPERTY::T_FLOAT,     "aspectrange",           offsetof(ParticlesImpl, m_fAspectRange)},
    {PROPERTY::T_FLOAT3,    "gravity",               offsetof(ParticlesImpl, m_vGravity)},
    {PROPERTY::T_INT,       "lifetime",              offsetof(ParticlesImpl, m_nLifeTime)},
    {PROPERTY::T_USER,      "quota",                 0},
    {PROPERTY::T_UINT,      "seed",                  offsetof(ParticlesImpl, m_Rand)},
    {PROPERTY::T_RANGEF,    "speedrange",            offsetof(ParticlesImpl, m_Speed)},
    {PROPERTY::T_RANGEI,    "particlelifetimerange", offsetof(ParticlesImpl, m_ParticleLifeTime)},
    {(INT_PTR)aEnumAlign,   "align",                 offsetof(ParticlesImpl, m_eAlign)},
    {PROPERTY::T_UNKNOWN,   NULL, 0},

    {PROPERTY::T_UNKNOWN, ""},
  };

  GXBOOL ParticlesImpl::Update(const GVSCENEUPDATE& sContext)
  {
    GXDWORD dwDeltaTime = sContext.dwDeltaTime;
    float fDeltaTime = sContext.fDeltaTime;
    //if(m_eState != Play && TEST_FLAG_NOT(m_eState, UpdateOnce)) {
    //  return TRUE;
    //}

    //RESET_FLAG(m_eState, UpdateOnce);

    //if(m_eState == Play) {
    //  m_nElapse += (GXINT)sContext.dwDeltaTime;
    //}
    switch(m_eState)
    {
    case Freeze:
      return TRUE;
    case Pause:
    case Stop:
      dwDeltaTime = 0;
      fDeltaTime = 0;
      break;
    case Step:
      dwDeltaTime = 20;
      fDeltaTime = 0.02f;
      m_eState = Pause;
      break;
    }

    GXBOOL bFixedSize = m_fExtentRange == 0;

    GXVERTEX_P3T2F_C1D* pVertices = (GXVERTEX_P3T2F_C1D*)m_pPrimitive->GetVerticesBuffer();
    if(pVertices)
    {
      float3 vRight; // sContext.pCamera->GetRight();
      float3 vTop; // sContext.pCamera->GetTop();
      CalcAxis(sContext, vTop, vRight);

      if(bFixedSize)
      {
        vRight *= m_fHalfWidth;
        vTop   *= m_fHalfHeight;
      }

      VIndex vtx = 0;
      for(ParticleArray::iterator it = m_aParticles.begin();
        it != m_aParticles.end(); ++it)
      {
        if(m_vGravity == 0.0f) {
          it->Pos = it->Pos + it->Velocity * fDeltaTime;
        }
        else {
          const float t = fDeltaTime;
          float3& v = it->Velocity;
          it->Pos = it->Pos + v * t + 0.5f * m_vGravity * t * t;
          v = v + t * m_vGravity;
        }

        pVertices[vtx + 0].texcoord.set(0, 0);
        pVertices[vtx + 1].texcoord.set(1, 0);
        pVertices[vtx + 2].texcoord.set(1, 1);
        pVertices[vtx + 3].texcoord.set(0, 1);

        if(bFixedSize)
        {
          pVertices[vtx + 0].pos = it->Pos +  vRight - vTop;
          pVertices[vtx + 1].pos = it->Pos + -vRight - vTop;
          pVertices[vtx + 2].pos = it->Pos + -vRight + vTop;
          pVertices[vtx + 3].pos = it->Pos +  vRight + vTop;
        }
        else
        {
          float3 vCurrRight = vRight * it->fWidthH;
          float3 vCurrTop   = vTop * it->fHeightH;
          pVertices[vtx + 0].pos = it->Pos +  vCurrRight - vCurrTop;
          pVertices[vtx + 1].pos = it->Pos + -vCurrRight - vCurrTop;
          pVertices[vtx + 2].pos = it->Pos + -vCurrRight + vCurrTop;
          pVertices[vtx + 3].pos = it->Pos +  vCurrRight + vCurrTop;
        }

        pVertices[vtx + 0].color =
          pVertices[vtx + 1].color =
          pVertices[vtx + 2].color =
          pVertices[vtx + 3].color = 0xffffffff;

        if(it->nElapse >= it->nLifeTime) {
          if(m_nLifeTime == -1 || m_nElapse < m_nLifeTime) {
            NewOne(*it);
            vtx += 4;
          }
        }
        else {
          vtx += 4;
        }

        it->nElapse += (GXINT)dwDeltaTime;
      }

      m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);

      m_nPrimiCount = (vtx / 4) * 2;
      ASSERT(m_nPrimiCount / 2 <= (int)m_aParticles.size());

      if(m_nPrimiCount == 0)
      {
        m_eState = Stop;
      }
    }

    return TRUE;
  }

  GXBOOL ParticlesImpl::Initialize(GXGraphics* pGraphics)
  {
    //GXVERTEXELEMENT
    SET_FLAG(m_dwFlags, GVNF_NOCLIP);
    return TRUE;
  }

  void ParticlesImpl::NewOne(PARTICLE& p)
  {
    p.nLifeTime = m_Rand.RandRangeI(m_ParticleLifeTime);
    p.nElapse = m_Rand.rand() % p.nLifeTime;
    p.Pos.set(0,0,0);
    m_Rand.RandVector(p.Velocity);
    p.Velocity.normalize();
    p.Velocity *= m_Rand.RandRangeF(m_Speed);
    if(m_fExtentRange != 0)
    {
      float fExt = m_Rand.randf2() * m_fExtentRange;
      p.fWidthH = m_fHalfWidth + fExt;

      if(m_fAspectRange != 0)
      {
        float fAspect = m_Rand.randf2();
        p.fHeightH = m_fHalfHeight + fExt + m_fAspectRange * fAspect;
        CLNOP;
      }
      else
      {
        p.fHeightH = m_fHalfHeight + fExt;
      }
    } else {
      p.fWidthH = m_fHalfWidth;
      p.fHeightH = m_fHalfHeight;
    }

    //if(m_fHeightRange != 0)
    //{
    //  p.fHeightH = m_fHalfHeight + m_fHeightRange * m_Rand.randf2();
    //} else {
    //  p.fHeightH = m_fHalfHeight;
    //}
  }

  void ParticlesImpl::CalcAxis(const GVSCENEUPDATE& sContext, float3& vTop, float3& vRight)
  {
    float3 vFront = sContext.pCamera->GetFront();

    if(m_eAlign == TowardCamera)
    {
      vTop = sContext.pCamera->GetUp();
      vRight = float3::cross(vTop, vFront);
      vRight.normalize();
    }
    else if(m_eAlign == Billboard)
    {
      vRight = sContext.pCamera->GetRight();
      vTop = sContext.pCamera->GetTop();
    }
    else {
      CLBREAK;  // 错误的枚举
    }
  }

  GXBOOL ParticlesImpl::SolveParams(GXGraphics* pGraphics, GXDEFINITION* aDefines, GXUINT nCount)
  {
    int nNumParticles = 0;
    m_sCommParam.vPos = 0.0f;

    m_nElapse = 0;
    for(GXUINT i = 0; i <nCount; i++)
    {
      if(ImportCommonParam(aDefines[i], m_sCommParam)) {
        continue;
      }
      else if(ImportParam(this, aDefines[i], s_aProperty)) {
        continue;
      }
      else if(CMP_PARAM("quota")) {
        nNumParticles = clstd::xtoi(10, aDefines[i].szValue);
      }
    }

    if(m_pPrimitive == NULL && ! nNumParticles) {
      nNumParticles = 10;
    }

    if(nNumParticles > 0)
    {
      clClamp<int>(1, 1000000, &nNumParticles);

      SAFE_RELEASE(m_pPrimitive);
      pGraphics->CreatePrimitiveVI(&m_pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P3T2F_C1D), 
        NULL, 6 * nNumParticles, 4 * nNumParticles);

      m_nPrimiCount = nNumParticles * 2;
      m_nVertCount  = nNumParticles * 4;
      m_nStartIndex = 0;
      m_aParticles.reserve(nNumParticles);

      if(m_aParticles.size() > (size_t)nNumParticles)
      {
        m_aParticles.erase(m_aParticles.begin() + nNumParticles, m_aParticles.end());
        ASSERT(m_aParticles.size() == nNumParticles);
      }
      else if(m_aParticles.size() < (size_t)nNumParticles)
      {
        for(GXUINT i = m_aParticles.size(); i < (GXUINT)nNumParticles; i++) {
          PARTICLE p;
          NewOne(p);
          m_aParticles.push_back(p);
        }
        ASSERT(m_aParticles.size() == nNumParticles);
      }

      int idx = 0;
      VIndex vtx = 0;
      VIndex* pIndices = (VIndex*)m_pPrimitive->GetIndicesBuffer();
      for(int i = 0; i < nNumParticles; i++, vtx += 4, idx += 6)
      {
        pIndices[idx + 0] = vtx + 0;
        pIndices[idx + 1] = vtx + 1;
        pIndices[idx + 2] = vtx + 3;
        pIndices[idx + 3] = vtx + 2;
        pIndices[idx + 4] = vtx + 3;
        pIndices[idx + 5] = vtx + 1;
      }
      m_pPrimitive->UpdateResouce(GPrimitive::ResourceIndices);
    }

    // 材质要在Primitive之后才能加载成功
    CreateCommonRes(pGraphics, m_sCommParam);

    return TRUE;
  }

  ParticlesImpl::ParticlesImpl( GXGraphics* pGraphics ) : Element(pGraphics, Particles)
    , m_vGravity(0, -1, 0)
    , m_Speed(1,2)
    , m_fHalfWidth(0.05f)
    , m_fHalfHeight(0.05f)
    , m_nElapse(0)
    , m_nLifeTime(1000)
    , m_ParticleLifeTime(1000, 2000)
    , m_eAlign(Billboard)
    , m_eState(Stop)
    , m_fExtentRange(0)
    , m_fAspectRange(0)
  {
  }

  GXINT ParticlesImpl::Execute( GXLPCSTR szCmd, GXWPARAM wParam, GXLPARAM lParam )
  {
    if(GXSTRCMP(szCmd, "play") == 0)
    {
      m_eState = Play;
    }
    else if(GXSTRCMP(szCmd, "pause") == 0)
    {
      m_eState = Pause;
    }
    else if(GXSTRCMP(szCmd, "stop") == 0)
    {
      m_eState = Stop;
      m_nElapse = 0;
    }
    else if(GXSTRCMP(szCmd, "step") == 0)
    {
      m_eState = Step;
    }
    else if(GXSTRCMP(szCmd, "freeze") == 0)
    {
      m_eState = Freeze;
    }
    else if(GXSTRCMP(szCmd, "getstate") == 0)
    {
      return m_eState == Play ? Play : Element::Execute(szCmd, wParam, lParam);
    }
    return Element::Execute(szCmd, wParam, lParam);
  }

  GXUINT ParticlesImpl::MakeParams( GXDefinition* aDefines, GXUINT nArrayCount )
  {
    GXUINT n = 0;
    GXUINT i = 0;

    n = ExportCommonParam(aDefines, nArrayCount, m_sCommParam);

    for(; s_aProperty[i].szName != NULL && n < nArrayCount; i++, n++)
    {
      if(ExportParam(this, aDefines[n], s_aProperty[i])) {
        continue;
      }
      else if(GXSTRCMP(s_aProperty[i].szName, "quota") == 0) {
        aDefines[n].Name = s_aProperty[i].szName;
        aDefines[n].Value.Format("%d", m_aParticles.size());
      }
    }

    return n;
  }

  //////////////////////////////////////////////////////////////////////////
} // namespace GFX
