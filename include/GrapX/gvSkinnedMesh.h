#ifndef _GRAPVR_SCENE_NODE_SKINNEDMESH_H_
#define _GRAPVR_SCENE_NODE_SKINNEDMESH_H_

class GVSkeleton;
class GVAnimationTrack;

class GXDLL GVSkinnedMeshSoft : public GVMesh
{
private:
  float*      m_pWeight;
  GVSkeleton* m_pSkeleton;
  GXLPBYTE    m_pVertices;
  GXUINT      m_nVertStride;
  GXUINT      m_nPosOffset;
  GXUINT      m_nNormalOffset;
  GXSIZE_T    m_nClusterCount;

protected:
  GVSkinnedMeshSoft(GrapX::Graphics* pGraphics);
  virtual ~GVSkinnedMeshSoft();

  GXBOOL Initialize(GrapX::Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, GXWORD* pIndices, GXSIZE_T nIdxCount, GVSkeleton* pSkeleton, float* pWeight, GXSIZE_T nClusterCount);
  GXBOOL Initialize(GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, GXSIZE_T nClusterCount);
  GXBOOL CheckSkeleton(GVSkeleton* pSkeleton);

public:
  GXHRESULT SetSkeleton   (GVSkeleton* pSkeleton);
  int       SetTrackData  (GXLPCSTR szName, GVAnimationTrack* pTrack);
  GXBOOL    PlayByName    (GXLPCSTR szName);
  GXBOOL    PlayById      (int nId);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT   Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXBOOL      Update    (const GVSCENEUPDATE& sContext);

  virtual GXHRESULT   SaveFile  (clSmartRepository* pStorage);
  virtual GXHRESULT   LoadFile  (GrapX::Graphics* pGraphics, clSmartRepository* pStorage);

  static  GXHRESULT   CreateMesh            (GrapX::Graphics* pGraphics, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, GXWORD* pIndices, int nIdxCount, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount, GVSkinnedMeshSoft** ppSkinnedMesh);
  static  GXHRESULT   CreateFromMeshData    (GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount, GVSkinnedMeshSoft** ppSkinnedMesh);
  static  GXHRESULT   CreateFromFileA       (GrapX::Graphics* pGraphics, GXLPCSTR szFilename, GVSkinnedMeshSoft** ppMesh);
  static  GXHRESULT   CreateFromFileW       (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GVSkinnedMeshSoft** ppMesh);
  static  GXHRESULT   CreateFromRepository  (GrapX::Graphics* pGraphics, clSmartRepository* pStorage, GVSkinnedMeshSoft** ppMesh);
};


#endif // #ifndef _GRAPVR_SCENE_NODE_SKINNEDMESH_H_