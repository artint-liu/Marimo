#ifndef _GRAPVR_SKELETON_H_
#define _GRAPVR_SKELETON_H_

#ifdef FBX_SDK
//class FbxAMatrix;
#endif // #ifdef FBX_SDK

//class SmartRepository;

struct Bone
{  
  clStringA   Name;
  int         nParent;

  float4x4    matAbs;
  float4x4    matLocal;
  float4x4    BindPose;

#ifdef FBX_SDK
  FbxAMatrix  fmAbs;
  FbxAMatrix  fmLocal;
#endif // #ifdef FBX_SDK

  Bone() : nParent(-1)
  {
    matAbs.identity();
    matLocal.identity();
    BindPose.identity();
#ifdef FBX_SDK
    fmAbs.SetIdentity();
    fmLocal.SetIdentity();
#endif // #ifdef FBX_SDK
  }
};

class GXDLL GVAnimationTrack : public GUnknown
{
public:
  typedef clstd::_quaternion Quaternion;
public:
  struct BONEINFO   // 每个骨骼的动画数据在列表中的索引
  {
    float3      vTranslation;
    Quaternion  vQuaternion;
    float3      vScaling;

    // 这个 BONE 在 GVAnimationTrack::Array 列表里的开始位置
    // 要么不存, 要么就是和 nFrameCount 相等数量
    int nTranslationIdx;
    int nQuaternionIdx;
    int nScalingIdx;
    BONEINFO()
      : vTranslation(0.0f), vQuaternion(0,0,0,0), vScaling(1.0f), 
      nTranslationIdx(-1), nQuaternionIdx(-1), nScalingIdx(-1){}
  };
  typedef clvector<float3>       Vector3Array;
  typedef clvector<BONEINFO>     BoneInfoArray;
  typedef clvector<Quaternion>   QuaternionArray;



protected:
  clStringA       m_Name;
  float3*         m_pTS;
  int             m_nTSCount;
  Quaternion*     m_pQuaternions;
  int             m_nQuaterCount;
  BONEINFO*       m_pBoneInfo;
  int             m_nBoneCount;

  int             m_nFrameCount;
  int             m_nFrameRate;
  float           m_fTimeLength; // (float)m_nFrameCount / m_nFrameRate * 1000; ms
private:
  GVAnimationTrack()
    : m_pTS         (NULL)
    , m_nTSCount    (0)
    , m_pQuaternions(NULL)
    , m_nQuaterCount(0)
    , m_pBoneInfo   (NULL)
    , m_nBoneCount  (0)
    , m_nFrameCount (1)
    , m_nFrameRate  (1)
    , m_fTimeLength (0.0f)
  {
  }
  virtual ~GVAnimationTrack();

  GXBOOL InitializeTrack(
    GXLPCSTR            szName,
    int                 nFrameCount, 
    int                 nFrameRate,
    float3*             pTSData,    // Translation Scaling
    int                 nTSCount,
    Quaternion*         pQuaternions,
    int                 nQuateCount,
    BONEINFO*           pBoneInfo,
    int                 nBoneCount);
  template<class _Vec, class _VecArray>
  inline int CalcComponent(_Vec& vDst, const _VecArray& aVec, const _Vec& vDefault, int nCurrFrame, int nRefIdx)
  {
    if(nRefIdx >= 0) {
      const int nIndex = nRefIdx + nCurrFrame;
      vDst = aVec[nIndex];
      return nIndex + 1;
    }
    else {
      vDst = vDefault;
      return -1;
    }
  }
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
  virtual GXHRESULT Release ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXLPCSTR  GetName             ();
  void      UpdateBoneLocalMarix(Bone& bone, int nBoneIndex, int nFrame, float fResidue);
  GXHRESULT SaveFile            (clSmartRepository* pStorage);
  GXHRESULT LoadFile            (clSmartRepository* pStorage);
  GXHRESULT SaveFileW           (GXLPCWSTR szFilename);

  inline int    GetFrameCount   () const;
  inline int    GetFrameRate    () const;
  inline float  GetTimeLength   () const;

  static GXHRESULT CreateAnimationTrack(
    GVAnimationTrack**  ppTrack,
    GXLPCSTR            szName,
    int                 nFrameCount,
    int                 nFrameRate,
    float3*             pTSData,    // Translation Scaling
    int                 nTSCount,
    Quaternion*         pQuaternions,
    int                 nQuateCount,
    BONEINFO*           pBoneInfo,
    int                 nBoneCount);
  static GXHRESULT CreateFromFileW      (GVAnimationTrack** ppTrack, GXLPCWSTR szFilename);
  static GXHRESULT CreateFromRepository (GVAnimationTrack** ppTrack, clSmartRepository* pStorage);
};

inline int GVAnimationTrack::GetFrameCount() const
{
  return m_nFrameCount;
}

inline int GVAnimationTrack::GetFrameRate() const
{
  return m_nFrameRate;
}

inline float GVAnimationTrack::GetTimeLength() const
{
  return m_fTimeLength;
}

//////////////////////////////////////////////////////////////////////////

class GXDLL GVSkeleton : public GVMesh
{
public:
  typedef clhash_map<clStringA, int>  BoneDict;
  typedef clvector<Bone>              BoneArray;
  typedef clvector<int>               OrderArray;
  typedef clmap<clStringA, int>       NameIdDict;
  typedef clvector<GVAnimationTrack*> AnimTrackArray;
protected:
  GVScene*          m_pScene;
  BoneDict          m_BoneDict;
  BoneArray         m_aBones;
  OrderArray        m_aUpdateOrder;

  // Dict 中的 Id 要减一后才是Array的 Idx 索引
  NameIdDict        m_NameIdDict;
  AnimTrackArray    m_aTracks;
  GVAnimationTrack* m_pCurrTrack;
  clStringA         m_strCurTrack;
  int               m_idCurTrack;

  int               m_nRefFrame;
  float             m_fResidue;   // 两帧之间的插值
public:
  virtual void    GetRenderDesc   (int nRenderCate, GVRENDERDESC* pRenderDesc) override;
protected:
  GXBOOL          UpdateRenderData      ();
  void            BuildUpdateOrderTable ();
  GXBOOL          CheckTrackData        (GVAnimationTrack* pTrack);
public:
  GVSkeleton(GVScene* pScene);

  virtual ~GVSkeleton();

  int         AddBone               (const Bone& bone);
  void        Clear                 ();
  int         FindBone              (GXLPCSTR szName);
  BoneArray&  GetBones              ();
  int         SetParent             (int nIndex, GXLPCSTR szName);
  void        SetParents            (GXLPCSTR* szNames);
  void        UpdateBones           ();
  CFloat4x4&  UpdateBone            (int nBoneIndex, GXBOOL bUpdateParent);
  GXBOOL      BuildRenderData       (GrapX::Graphics* pGraphics);
  void        DbgDump               ();
  int         SetTrackData          (GXLPCSTR szName, GVAnimationTrack* pTrack); // szName 可以为 NULL
  GXBOOL      PlayByName            (GXLPCSTR szName);
  GXBOOL      PlayById              (int nId);  // 用这个接口设置动作名将会为空


  virtual GXHRESULT SaveFile(clSmartRepository* pStorage);
  virtual GXHRESULT LoadFile(GrapX::Graphics* pGraphics, clSmartRepository* pStorage);

  virtual GXBOOL    Update                (const GVSCENEUPDATE& sContext);
  static  GXHRESULT CreateFromFileW       (GVSkeleton** ppSkeleton, GVScene* pScene, GXLPCWSTR szFilename);
  static  GXHRESULT CreateFromRepository  (GVSkeleton** ppSkeleton, GVScene* pScene, clSmartRepository* pStorage);

  //template<class _Vec, class _VecArray>
  //inline int CalcComponent(_Vec& vDst, const _VecArray& aVec, const _Vec& vDefault, int nRefIdx)
  //{
  //  if(nRefIdx >= 0) {
  //    const int nIndex = nRefIdx + m_nRefFrame;
  //    vDst = aVec[nIndex];
  //    return nIndex + 1;
  //  }
  //  else {
  //    vDst = vDefault;
  //    return -1;
  //  }
  //}

}; // class GVSkeleton : public GVNode



#endif // #ifndef _GRAPVR_SKELETON_H_