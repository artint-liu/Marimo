#include "GrapX.H"

//#include <clstd/clTree.H>
#include <smart/SmartRepository.h>

#include "GrapX/GResource.H"
#include "GrapX/GShader.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXError.h"

#include "GrapX/GrapVR.H"

#define ANIMTRACK_FRAMECOUNT     "AnimTrack@FrameCount"
#define ANIMTRACK_FRAMERATE      "AnimTrack@FrameRate"
#define ANIMTRACK_TSDATA         "AnimTrack@TSData"
#define ANIMTRACK_TSCOUNT        "AnimTrack@TSCount"
#define ANIMTRACK_QUATERDATA     "AnimTrack@QuaternionData"
#define ANIMTRACK_QUATERCOUNT    "AnimTrack@QuaternionCount"
#define ANIMTRACK_BONEINFODATA   "AnimTrack@BoneInfoData"
#define ANIMTRACK_BONEINFOCOUNT  "AnimTrack@BoneInfoCount"

#define SKELETON_BONECOUNT       "Skeleton@BoneCount"
#define SKELETON_FMT_BONENAME    "Skeleton@BoneName_%d"
#define SKELETON_FMT_BONEPT      "Skeleton@BonePT_%d"  // Parent & Transform

using namespace clstd;

//////////////////////////////////////////////////////////////////////////
struct BONE_PARENT_TRANSFORM
{
  int       nParent;
  float4x4  matAbs;
  float4x4  matLocal;
  float4x4  BindPose;
};

//////////////////////////////////////////////////////////////////////////
#ifdef FBX_SDK
#pragma comment(lib, "E:\\MyCodes\\FrameUI\\Work\\Project\\Test\\Test3D\\FBX SDK\\2013.2\\lib\\vs2010\\x86\\fbxsdk-2013.2-mdd.lib")

void Dump(const FbxAMatrix& m)
{
  TRACE("%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f [FbxAMatrix]\n",
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]);
}
#endif // #ifdef FBX_SDK

void Dump(const float4x4& m)
{
  TRACE("%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f [float4x4]\n",
    m._11, m._12, m._13, m._14,
    m._21, m._22, m._23, m._24,
    m._31, m._32, m._33, m._34,
    m._41, m._42, m._43, m._44);
}

inline float3* VectorLerp(float3* pout, const float3* pq1, const float3* pq2, float t)
{
  *pout = ((*pq2) - (*pq1)) * t + (*pq1);
  return pout;
}

//////////////////////////////////////////////////////////////////////////
GVAnimationTrack::~GVAnimationTrack()
{
  SAFE_DELETE(m_pTS);
  SAFE_DELETE(m_pQuaternions);
  SAFE_DELETE(m_pBoneInfo);
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVAnimationTrack::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVAnimationTrack::Release()
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

GXLPCSTR GVAnimationTrack::GetName()
{
  return m_Name;
}

void GVAnimationTrack::UpdateBoneLocalMarix(Bone& bone, int nBoneIndex, int nFrame, float fResidue)
{
  //Bone& bone = m_aBones[nBoneIndex];
  const BONEINFO& BoneInfo = m_pBoneInfo[nBoneIndex];
  float3      vTranslation;
  quaternion  vRotation;
  float3      vScaling;

  ASSERT(BoneInfo.nTranslationIdx < 0 || m_pTS[BoneInfo.nTranslationIdx] == BoneInfo.vTranslation);
  ASSERT(BoneInfo.nQuaternionIdx < 0  || m_pQuaternions[BoneInfo.nQuaternionIdx] == BoneInfo.vQuaternion);
  ASSERT(BoneInfo.nScalingIdx < 0     || m_pTS[BoneInfo.nScalingIdx] == BoneInfo.vScaling);

  const int nTF = CalcComponent(vTranslation, m_pTS, BoneInfo.vTranslation, nFrame, BoneInfo.nTranslationIdx);
  const int nRF = CalcComponent(vRotation, m_pQuaternions, BoneInfo.vQuaternion, nFrame, BoneInfo.nQuaternionIdx);
  const int nSF = CalcComponent(vScaling, m_pTS, BoneInfo.vScaling, nFrame, BoneInfo.nScalingIdx);

  if(fResidue > 0.0f && nFrame < m_nFrameCount - 1)
  {
    if(nTF > 0)
      VectorLerp(&vTranslation, &vTranslation, &m_pTS[nTF], fResidue);
    if(nRF > 0)
      clstd::QuaternionSlerp(&vRotation, &vRotation, &m_pQuaternions[nRF], fResidue);
    if(nSF > 0)
      VectorLerp(&vScaling, &vScaling, &m_pTS[nSF], fResidue);
  }

  bone.matLocal.AffineTransformation(&vScaling, NULL, &vRotation, &vTranslation);
}

GXHRESULT GVAnimationTrack::SaveFile(SmartRepository* pStorage)
{
  pStorage->WriteNumeric(NULL, ANIMTRACK_FRAMECOUNT, m_nFrameCount);
  pStorage->WriteNumeric(NULL, ANIMTRACK_FRAMERATE, m_nFrameRate);
  pStorage->WriteNumeric(NULL, ANIMTRACK_TSCOUNT, m_nTSCount);
  pStorage->WriteNumeric(NULL, ANIMTRACK_QUATERCOUNT, m_nQuaterCount);
  pStorage->WriteNumeric(NULL, ANIMTRACK_BONEINFOCOUNT, m_nBoneCount);

  // 其实下面的 Count 不需要储存,只是为了校验
  ASSERT(m_nFrameCount == m_nTSCount || m_nTSCount == 0);
  pStorage->Write(NULL, ANIMTRACK_TSDATA, m_pTS, m_nFrameCount * sizeof(float3));
  pStorage->Write(NULL, ANIMTRACK_QUATERDATA, m_pQuaternions, m_nQuaterCount * sizeof(quaternion));
  pStorage->Write(NULL, ANIMTRACK_BONEINFODATA, m_pBoneInfo, m_nBoneCount * sizeof(BONEINFO));
  return GX_OK;
}

GXHRESULT GVAnimationTrack::LoadFile(SmartRepository* pStorage)
{
  SAFE_DELETE(m_pTS);
  SAFE_DELETE(m_pQuaternions);
  SAFE_DELETE(m_pBoneInfo);

  m_nFrameCount   = pStorage->ReadNumeric(NULL, ANIMTRACK_FRAMECOUNT);
  m_nFrameRate    = pStorage->ReadNumeric(NULL, ANIMTRACK_FRAMERATE);

  // 其实下面的 Count 不需要储存,只是为了校验
  m_nTSCount      = pStorage->ReadNumeric(NULL, ANIMTRACK_TSCOUNT);
  m_nQuaterCount  = pStorage->ReadNumeric(NULL, ANIMTRACK_QUATERCOUNT);
  m_nBoneCount    = pStorage->ReadNumeric(NULL, ANIMTRACK_BONEINFOCOUNT);

  ASSERT(m_nFrameCount == m_nTSCount || m_nTSCount == 0);


  if(m_nTSCount != 0)
  {
    m_pTS = new float3[m_nTSCount];
    pStorage->Read(NULL, ANIMTRACK_TSDATA, m_pTS, m_nFrameCount * sizeof(float3));
  }

  if(m_nQuaterCount != 0)
  {
    m_pQuaternions = new quaternion[m_nQuaterCount];
    pStorage->Read(NULL, ANIMTRACK_QUATERDATA, m_pQuaternions, m_nQuaterCount * sizeof(quaternion));
  }

  if(m_nBoneCount != 0)
  {
    m_pBoneInfo = new BONEINFO[m_nBoneCount];
    pStorage->Read(NULL, ANIMTRACK_BONEINFODATA, m_pBoneInfo, m_nBoneCount * sizeof(BONEINFO));
  }

  m_fTimeLength = (float)m_nFrameCount / m_nFrameRate * 1000;
  return GX_OK;
}

GXHRESULT GVAnimationTrack::SaveFileW(GXLPCWSTR szFilename)
{
  SmartRepository Storage;
  GXHRESULT hval = SaveFile(&Storage);
  if(GXSUCCEEDED(hval)) {
    hval = Storage.SaveW(szFilename) ? GX_OK : GX_FAIL;
  }
  return hval;
}

GXBOOL GVAnimationTrack::InitializeTrack(
  GXLPCSTR            szName,
  int                 nFrameCount, 
  int                 nFrameRate,
  float3*             pTSData,    // Translation Scaling
  int                 nTSCount,
  quaternion*         pQuaternions,
  int                 nQuateCount,
  BONEINFO*           pBoneInfo,
  int                 nBoneCount)
{
  m_Name          = szName;
  m_nFrameCount   = nFrameCount;
  m_nFrameRate    = nFrameRate;

  m_nTSCount      = nTSCount;
  m_nQuaterCount  = nQuateCount;
  m_nBoneCount    = nBoneCount;

  SAFE_DELETE(m_pTS);
  SAFE_DELETE(m_pQuaternions);
  SAFE_DELETE(m_pBoneInfo);

  m_pTS = new float3[nTSCount];
  m_pQuaternions = new quaternion[nQuateCount];
  m_pBoneInfo = new BONEINFO[nBoneCount];

  memcpy(m_pTS, pTSData, sizeof(float3) * nTSCount);
  memcpy(m_pQuaternions, pQuaternions, sizeof(quaternion) * nQuateCount);
  memcpy(m_pBoneInfo, pBoneInfo, sizeof(BONEINFO) * nBoneCount);

  m_fTimeLength = (float)m_nFrameCount / m_nFrameRate * 1000;
  return TRUE;
}


GXHRESULT GVAnimationTrack::CreateAnimationTrack(
  GVAnimationTrack**  ppTrack,
  GXLPCSTR            szName,
  int                 nFrameCount, 
  int                 nFrameRate,
  float3*             pTSData,    // Translation Scaling
  int                 nTSCount,
  quaternion*         pQuaternions,
  int                 nQuateCount,
  BONEINFO*           pBoneInfo,
  int                 nBoneCount)
{
  GXHRESULT hval = GX_OK;
  GVAnimationTrack* pTrack = new GVAnimationTrack;
  if( ! InlCheckNewAndIncReference(pTrack)) {
    return GX_FAIL;
  }

  if( ! pTrack->InitializeTrack(szName == NULL ? "" : szName, nFrameCount, nFrameRate, 
    pTSData, nTSCount, pQuaternions, nQuateCount, pBoneInfo, nBoneCount))
  {
    pTrack->Release();
    pTrack = NULL;
    hval = GX_FAIL;
  }

  *ppTrack = pTrack;  
  return hval;
}

GXHRESULT GVAnimationTrack::CreateFromFileW(GVAnimationTrack** ppTrack, GXLPCWSTR szFilename)
{
  SmartRepository Storage;
  if( ! Storage.LoadW(szFilename)) {
    return GX_FAIL;
  }

  return CreateFromRepository(ppTrack, &Storage);
}

GXHRESULT GVAnimationTrack::CreateFromRepository(GVAnimationTrack** ppTrack, SmartRepository* pStorage)
{
  GXHRESULT hval = GX_OK;
  GVAnimationTrack* pTrack = new GVAnimationTrack;
  if( ! InlCheckNewAndIncReference(pTrack)) {
    return GX_FAIL;
  }

  if(GXFAILED(pTrack->LoadFile(pStorage)))
  {
    pTrack->Release();
    pTrack = NULL;
    hval = GX_FAIL;
  }
  *ppTrack = pTrack;
  return hval;
}
//////////////////////////////////////////////////////////////////////////
GVSkeleton::GVSkeleton( GVScene* pScene ) : GVMesh(pScene->GetGraphicsUnsafe(), GXMAKEFOURCC('S','K','T','N'))
  , m_pScene      (pScene)
  , m_pCurrTrack  (NULL)
  , m_idCurTrack  (0)
  , m_nRefFrame   (0)
  , m_fResidue    (0)
{
  SetName("skeleton");
  RESET_FLAG(m_dwFlags, GVNF_VISIBLE);
}

GVSkeleton::~GVSkeleton()
{
  for(AnimTrackArray::iterator it = m_aTracks.begin();
    it != m_aTracks.end(); ++it)
  {
    GVAnimationTrack*& pTrack = *it;
    SAFE_RELEASE(pTrack);
  }

  m_NameIdDict.clear();
  SAFE_RELEASE(m_pCurrTrack);
  SAFE_RELEASE(m_pPrimitive);
}

void GVSkeleton::GetRenderDesc(GVRenderType eType, GVRENDERDESC* pRenderDesc)
{
  pRenderDesc->pPrimitive       = m_pPrimitive;
  pRenderDesc->ePrimType        = GXPT_LINELIST;
  pRenderDesc->pMaterial        = m_pMtlInst;
  pRenderDesc->matWorld         = m_Transformation.GlobalMatrix;

  pRenderDesc->dwFlags          = m_dwFlags;
  pRenderDesc->BaseVertexIndex  = 0;
  pRenderDesc->MinIndex         = 0;
  pRenderDesc->NumVertices      = m_nPrimiCount * 2;
  pRenderDesc->StartIndex       = 0;
  pRenderDesc->PrimitiveCount   = m_nPrimiCount;
}

void GVSkeleton::BuildUpdateOrderTable()
{
  // 根据树形深度排序
  struct ORDER_CONTEXT
  {
    int nIndex;
    int nDepth;
    GXBOOL SortCompare(ORDER_CONTEXT& c) const
    {
      return nDepth > c.nDepth;
    }
    GXVOID SortSwap(ORDER_CONTEXT& c)
    {
      ORDER_CONTEXT temp = *this;
      *this = c;
      c = temp;
    }
  };
  typedef clvector<ORDER_CONTEXT> IndexOrderArray;
  IndexOrderArray aOrder;
  ORDER_CONTEXT oc = {0, 0};

  // 记录每层深度
  for(BoneArray::iterator it = m_aBones.begin();
    it != m_aBones.end(); ++it)
  {
    Bone* pBone = &*it;
    while(pBone->nParent >= 0)
    {
      pBone = &m_aBones[pBone->nParent];
      oc.nDepth++;
    }
    aOrder.push_back(oc);
    oc.nIndex++;
    oc.nDepth = 0;
  }

  // 根据深度排序
  QuickSort(&aOrder.front(), (int)0, (int)aOrder.size());

  // 把排序后的索引值写入表中
  for(IndexOrderArray::iterator it = aOrder.begin();
    it != aOrder.end(); ++it)
  {
    m_aUpdateOrder.push_back(it->nIndex);
  }
}

GXBOOL GVSkeleton::BuildRenderData(GXGraphics* pGraphics)
{
  CombineFlags(GVNF_VISIBLE);
  SAFE_RELEASE(m_pPrimitive);

  const GXUINT nBoneCount = (GXUINT)m_aBones.size();
  if(nBoneCount == 0) {
    return FALSE;
  }
  else if(m_BoneDict.size() == 0)
  {
    CLOG_ERROR("%s : Must call BuildDict() first!\n", __FUNCTION__);
    return FALSE;
  }

  GXVERTEX_P3T2C4F* pVertex = NULL;
  GXWORD* pIndices = NULL;
  try {
    pVertex = new GXVERTEX_P3T2C4F[nBoneCount];
    pIndices = new GXWORD[nBoneCount * 2];

    m_nPrimiCount = 0;
    for(GXUINT i = 0; i < nBoneCount; i++)
    {
      pVertex[i].pos = m_aBones[i].matAbs.GetRow(3);
  #ifdef FBX_SDK
      FbxVector4 vAbsPos = m_aBones[i].fmAbs.GetRow(3);
      pVertex[i].pos.set(vAbsPos[0], vAbsPos[1], vAbsPos[2]);
  #endif // #ifdef FBX_SDK
      pVertex[i].texcoord.set(0, 0);
      pVertex[i].color.set(1, (i & 1) ? 1.0f : 0.0f, 1, 1);

      if(m_aBones[i].nParent >= 0)
      {
        pIndices[m_nPrimiCount * 2] = i;
        pIndices[m_nPrimiCount * 2 + 1] = m_aBones[i].nParent;
        m_nPrimiCount++;
      }
    }
    m_nVertCount = m_nPrimiCount * 2;

    pGraphics->CreatePrimitiveVI(&m_pPrimitive,
      NULL, MOGetSysVertexDecl(GXVD_P3T2C4F), GXRU_DEFAULT, m_nPrimiCount * 2, nBoneCount, 
      0, pIndices, pVertex);

    SAFE_DELETE_ARRAY(pVertex);
    SAFE_DELETE_ARRAY(pIndices);

    return TRUE;
  }
  catch(...) {
    SAFE_DELETE_ARRAY(pVertex);
    SAFE_DELETE_ARRAY(pIndices);
    return FALSE;
  }
}

GXBOOL GVSkeleton::UpdateRenderData()
{
  GXVERTEX_P3T2C4F* pVertices;
  GXWORD* pIndices;
  const GXUINT nBoneCount = (GXUINT)m_aBones.size();
  if(m_pPrimitive->Lock(0, 0, 0, 0, (GXLPVOID*)&pVertices, &pIndices))
  {
    for(GXUINT i = 0; i < nBoneCount; i++)
    {
      pVertices->pos = m_aBones[i].matAbs.GetRow(3);
      pVertices++;
    }

    m_pPrimitive->Unlock();
  }
  return TRUE;
}

GXBOOL GVSkeleton::Update(const GVSCENEUPDATE& sContext)
{
  if(m_pCurrTrack == NULL) {
    return TRUE;
  }
  //const float fElapse = (float)m_nFrameCount / m_nFrameRate * 1000; // ms
  const float fElapse = m_pCurrTrack->GetTimeLength();
  const int nElapse = (int)fElapse;
  GXDWORD dwTime = m_pScene->GetAbsTime() % nElapse;
  if(nElapse == 0)
  {
    m_nRefFrame = 0;
    m_fResidue = 0;
  }
  else {
    float fFrame = (float)m_pCurrTrack->GetFrameCount() * dwTime / fElapse;
    m_nRefFrame = (int)fFrame;
    m_fResidue = fFrame - (float)m_nRefFrame;
    ASSERT(m_fResidue >= 0 && m_fResidue < 1.0f);
  }

  UpdateBones();
  if(m_pPrimitive != NULL)
  {
    UpdateRenderData();
  }
  return TRUE;
}

GXHRESULT GVSkeleton::CreateFromFileW(GVSkeleton** ppSkeleton, GVScene* pScene, GXLPCWSTR szFilename)
{
  SmartRepository Storage;
  if( ! Storage.LoadW(szFilename)) {
    return GX_FAIL;
  }
  return CreateFromRepository(ppSkeleton, pScene, &Storage);
}

GXHRESULT GVSkeleton::CreateFromRepository(GVSkeleton** ppSkeleton, GVScene* pScene, SmartRepository* pStorage)
{
  GXHRESULT hval = GX_OK;
  GVSkeleton* pSkeleton = new GVSkeleton(pScene);
  if( ! InlCheckNewAndIncReference(pSkeleton)) {
    return GX_FAIL;
  }

  if(GXFAILED(pSkeleton->LoadFile(pScene->GetGraphicsUnsafe(), pStorage)))
  {
    pSkeleton->Release();
    pSkeleton = NULL;
    hval = GX_FAIL;
  }
  *ppSkeleton = pSkeleton;
  return hval;
}

CFloat4x4& GVSkeleton::UpdateBone(int nBoneIndex, GXBOOL bUpdateParent)
{
  Bone& CurrBone = m_aBones[nBoneIndex];
  const int nParent = CurrBone.nParent;

  m_pCurrTrack->UpdateBoneLocalMarix(CurrBone, nBoneIndex, m_nRefFrame, m_fResidue);
  if(nParent >= 0) {
    const float4x4& matParentAbs = bUpdateParent 
      ? UpdateBone(nParent, TRUE)
      : m_aBones[nParent].matAbs;
    CurrBone.matAbs = CurrBone.matLocal * matParentAbs;
  }
  else {
    CurrBone.matAbs = CurrBone.matLocal;
  }
  return CurrBone.matAbs;
}


//void GVSkeleton::SetFrameInfo(int nFrameCount, int nFrameRate)
//{
//  m_nFrameCount = nFrameCount;
//  m_nFrameRate = nFrameRate;
//}

int GVSkeleton::AddBone(const Bone& bone)
{
  if(bone.Name.GetLength() > 0)
  {
    int nIndex = (int)m_aBones.size();
    m_aBones.push_back(bone);
    m_BoneDict[bone.Name] = nIndex;
    return nIndex;
  }
  return -1;
}

void GVSkeleton::Clear()
{
  m_BoneDict.clear();
  m_aBones.clear();
  m_aUpdateOrder.clear();
  SAFE_RELEASE(m_pCurrTrack);

  m_nRefFrame = 0;
  m_fResidue  = 0.0f;
}

int GVSkeleton::SetParent(int nIndex, GXLPCSTR szName)
{
  int nParentIndex = FindBone(szName);
  if(nParentIndex >= 0) {
    m_aBones[nIndex].nParent = nParentIndex;
  }
  return nParentIndex;
}

void GVSkeleton::SetParents(GXLPCSTR* szNames)
{
  int nCount = (int)m_aBones.size();
  for(int i = 0; i < nCount; i++)
  {
    m_aBones[i].nParent = FindBone(szNames[i]);
  }
}

int GVSkeleton::FindBone(GXLPCSTR szName)
{
  BoneDict::iterator it = m_BoneDict.find(szName);
  if(it == m_BoneDict.end()) {
    return -1;
  }
  return it->second;
}

GVSkeleton::BoneArray& GVSkeleton::GetBones()
{
  return m_aBones;
}

void GVSkeleton::UpdateBones()
{
  if(m_pCurrTrack == NULL)
    return;

  if(m_aUpdateOrder.size() == 0) {
    BuildUpdateOrderTable();
  }

  for(OrderArray::iterator it = m_aUpdateOrder.begin();
    it != m_aUpdateOrder.end(); ++it)
  {
    UpdateBone(*it, FALSE);
  }
}

GXBOOL GVSkeleton::CheckTrackData(GVAnimationTrack* pTrack)
{
  if(pTrack == NULL) {
    return FALSE;
  }
  return TRUE;
}

//GXHRESULT GVSkeleton::SetTrackData(GVAnimationTrack* pTrack)
int GVSkeleton::SetTrackData(GXLPCSTR szName, GVAnimationTrack* pTrack)
{
  if( ! CheckTrackData(pTrack)) {
    return GX_FAIL;
  }
  //InlSetNewObjectT(m_pCurrTrack, pTrack);
  if(GXSUCCEEDED(pTrack->AddRef())) {
    m_aTracks.push_back(pTrack);
    int nId = (int)m_aTracks.size(); // 不用 -1, 因为不是索引

    if(szName != NULL) {
      m_NameIdDict[szName] = nId;
    }
    return nId;
  }
  return 0;
  //return InlSetNewObjectT(m_pCurrTrack, pTrack);
}

GXBOOL GVSkeleton::PlayByName(GXLPCSTR szName)
{
  NameIdDict::iterator it = m_NameIdDict.find(szName);
  if(it == m_NameIdDict.end()) {
    return FALSE;
  }

  m_strCurTrack = szName;
  m_idCurTrack = it->second;
  InlSetNewObjectT(m_pCurrTrack, m_aTracks[m_idCurTrack - 1]);
  return m_pCurrTrack != NULL;
}

GXBOOL GVSkeleton::PlayById(int nId)
{
  if(nId > (int)m_aTracks.size() || nId <= 0) {
    return FALSE;
  }

  m_strCurTrack.Clear();
  m_idCurTrack = nId;
  InlSetNewObjectT(m_pCurrTrack, m_aTracks[m_idCurTrack - 1]);
  return m_pCurrTrack != NULL;
}

GXHRESULT GVSkeleton::SaveFile(SmartRepository* pStorage)
{
  GXUINT nBoneCount = (GXUINT)m_aBones.size();
  pStorage->Write64(NULL, SKELETON_BONECOUNT, nBoneCount, NULL);
  for(GXUINT i = 0; i < nBoneCount; i++)
  {
    clStringA str;
    BONE_PARENT_TRANSFORM bpt;
    str.Format(SKELETON_FMT_BONENAME, i);
    pStorage->WriteStringA(NULL, str, m_aBones[i].Name);

    bpt.nParent  = m_aBones[i].nParent;
    bpt.BindPose = m_aBones[i].BindPose;
    bpt.matAbs   = m_aBones[i].matAbs;
    bpt.matLocal = m_aBones[i].matLocal;
    
    str.Format(SKELETON_FMT_BONEPT, i);
    pStorage->WriteStructT(NULL, str, bpt);
  }
  return GX_OK;
}

GXHRESULT GVSkeleton::LoadFile(GXGraphics* pGraphics, SmartRepository* pStorage)
{
  Clear();
  int nBoneCount;
  pStorage->Read64(NULL, SKELETON_BONECOUNT, (u32*)&nBoneCount, NULL);
  for(int i = 0; i < nBoneCount; i++)
  {
    clStringA str;
    Bone bone;
    BONE_PARENT_TRANSFORM bpt;
    str.Format(SKELETON_FMT_BONENAME, i);
    pStorage->ReadStringA(NULL, str, bone.Name);

    str.Format(SKELETON_FMT_BONEPT, i);
    pStorage->ReadStructT(NULL, str, bpt);

    bone.nParent   = bpt.nParent;
    bone.BindPose  = bpt.BindPose;
    bone.matAbs    = bpt.matAbs;
    bone.matLocal  = bpt.matLocal;

    AddBone(bone);
  }
  return GX_OK;
}

void GVSkeleton::DbgDump()
{
  int i = 0;
  for(BoneArray::iterator it = m_aBones.begin();
    it != m_aBones.end(); ++it, ++i)
  {
    const float4& vPos = it->matAbs.GetRow(3);
    TRACE("\n\n%d:%s %d (%f,%f,%f)\n", i, (const char*)it->Name, it->nParent, vPos.x, vPos.y, vPos.z);
    //TRACE("%s\n", it->Name);
#ifdef FBX_SDK
    Dump(it->fmAbs);
    TRACE("\n");
    Dump(it->matAbs);
#endif // #ifdef FBX_SDK
  }
}