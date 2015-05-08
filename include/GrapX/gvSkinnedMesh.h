#ifndef _GRAPVR_SCENE_NODE_SKINNEDMESH_H_
#define _GRAPVR_SCENE_NODE_SKINNEDMESH_H_

class GVSkeleton;
class GVAnimationTrack;

class GXDLL GVSkinnedMeshSoft : public GVMesh
{
private:
  float*      m_pWeight;
  GVSkeleton* m_pSkeleton;
  GXLPVOID    m_pVertices;
  GXUINT      m_nVertStride;
  GXUINT      m_nPosOffset;
  GXUINT      m_nNormalOffset;
  int         m_nClusterCount;

protected:
  GVSkinnedMeshSoft(GXGraphics* pGraphics);
  virtual ~GVSkinnedMeshSoft();

  GXBOOL Initialize(GXGraphics* pGraphics, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, GXWORD* pIndices, int nIdxCount, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount);
  GXBOOL Initialize(GXGraphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount);
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

  virtual GXHRESULT   SaveFile  (SmartRepository* pStorage);
  virtual GXHRESULT   LoadFile  (GXGraphics* pGraphics, SmartRepository* pStorage);

  static  GXHRESULT   CreateMesh            (GXGraphics* pGraphics, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, GXWORD* pIndices, int nIdxCount, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount, GVSkinnedMeshSoft** ppSkinnedMesh);
  static  GXHRESULT   CreateFromMeshData    (GXGraphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount, GVSkinnedMeshSoft** ppSkinnedMesh);
  static  GXHRESULT   CreateFromFileA       (GXGraphics* pGraphics, GXLPCSTR szFilename, GVSkinnedMeshSoft** ppMesh);
  static  GXHRESULT   CreateFromFileW       (GXGraphics* pGraphics, GXLPCWSTR szFilename, GVSkinnedMeshSoft** ppMesh);
  static  GXHRESULT   CreateFromRepository  (GXGraphics* pGraphics, SmartRepository* pStorage, GVSkinnedMeshSoft** ppMesh);
};


#endif // #ifndef _GRAPVR_SCENE_NODE_SKINNEDMESH_H_