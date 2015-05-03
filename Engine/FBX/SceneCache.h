/****************************************************************************************

Copyright (C) 2012 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SCENE_CACHE_H
#define _SCENE_CACHE_H

//#include "GlFunctions.h"
//#include <fbxsdk.h>
struct Bone;
class GVSkeleton;
class SceneContext;
void Dump(const float4x4& m);
void Dump(const FbxAMatrix& m);
//FbxAMatrix GetGeometry(FbxNode* pNode);
void ComputeClusterDeformation(
  FbxAMatrix& pGlobalPosition, 
  FbxNode* pNode, FbxCluster* pCluster, 
  FbxAMatrix& pVertexTransformMatrix, 
  //FbxTime pTime, 
  FbxPose* pPose);
//FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime = FBXSDK_TIME_INFINITE, FbxPose* pPose = NULL, FbxAMatrix* pParentGlobalPosition = NULL);
//FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);

// ½ôËõWeightTable
int InflateWeightTable(const clBuffer* pWeightBuf, clBuffer* pInflateWeightBuf, int nVertexCount);




// Save mesh vertices, normals, UVs and indices in GPU with OpenGL Vertex Buffer Objects
class VBOMesh
{
  typedef GVAnimationTrack::BONEINFO BONEINFO;
public:
    VBOMesh();
    ~VBOMesh();

    // Save up data into GPU buffers.
    bool Initialize(SceneContext* pSceneCtx, const FbxMesh * pMesh);

    // Update vertex positions for deformed meshes.
    //void UpdateVertexPosition(const FbxMesh * pMesh, const FbxVector4 * pVertices) const;


    void DumpAnim();

    // Get the count of material groups
    int GetSubMeshCount() const { return m_aSubMeshes.GetCount(); }
    GVSkeleton* GetSkeleton()
    {
      return m_pSkeleton;
    }
private:
  void GenerateSkinWeight(FbxMesh* pMesh, int nSkinIndex, float* pWeight/*, float4x4* pTransform*/, int nVertexCount, int nClusterCount);
  void GenLinearDeformationWeight(FbxSkin* pSkinDeformer, float* pWeight/*, float4x4* pTransform*/, int nVertexCount, int nClusterCount);
  int AddUndeformerBone(FbxNode* pNode);
  void GenerateAnimData(Bone& bone, BONEINFO& BoneInfo, FbxCluster* pCluster, FbxNode* pNode, FbxTime StartTime, FbxTime FrameFreq, FbxLongLong nFrameCount);
  void GenerateSubMesh(const FbxMesh *pMesh, FbxLayerElementArrayTemplate<int>*& aMaterialIndice, FbxGeometryElement::EMappingMode& eMaterialMappingMode);
  void SetMeshMtl(int nIndex, GVMesh* pMoMesh);
  //void ReadTangent(FbxMesh* pMesh, int ctrlPointIndex, int vertecCounter, FbxVector4& vTangent);
  template<class _ElementT>
  void ReadElement(_ElementT* pElement, int ctrlPointIndex, int vertecCounter, FbxVector4& vElementVector);

private:
    enum
    {
        VERTEX_VBO,
        NORMAL_VBO,
        UV_VBO,
        INDEX_VBO,
        VBO_COUNT,
    };

    // For every material, record the offsets in every VBO and triangle counts
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0) {}

        int       IndexOffset;
        int       TriangleCount;
        clStringW strMajorTexture;
    };
    SceneContext* m_pSceneCtx;
    int mVBONames[VBO_COUNT];
    FbxArray<SubMesh*> m_aSubMeshes;
    bool m_bHasNormal;
    bool m_bHasUV;
    bool m_bHasTangent;
    bool m_bHasBinormal;
    bool m_bAllByControlPoint; // Save data in VBO by control point or by polygon vertex.
    GVSkeleton* m_pSkeleton;
    GVAnimationTrack* m_pTrack;

    //GVAnimationTrack* m_pTrack;
    GVAnimationTrack::Vector3Array    m_aTS;
    GVAnimationTrack::QuaternionArray m_aQuaternions;
    GVAnimationTrack::BoneInfoArray   m_aBoneInfo;
};

// Cache for FBX material
class MaterialCache
{
public:
    MaterialCache();
    ~MaterialCache();

    bool Initialize(const FbxSurfaceMaterial * pMaterial);
    
    // Set material colors and binding diffuse texture if exists.
    void SetCurrentMaterial() const;

    bool HasTexture() const { return mDiffuse.mTextureName != 0; }

    // Set default green color.
    static void SetDefaultMaterial();

private:
    struct ColorChannel
    {
        ColorChannel() : mTextureName(0)
        {
            mColor[0] = 0.0f;
            mColor[1] = 0.0f;
            mColor[2] = 0.0f;
            mColor[3] = 1.0f;
        }

        int mTextureName;
        float mColor[4];
    };
    ColorChannel mEmissive;
    ColorChannel mAmbient;
    ColorChannel mDiffuse;
    ColorChannel mSpecular;
    float mShinness;
};

// Property cache, value and animation curve.
struct PropertyChannel
{
    PropertyChannel() : mAnimCurve(NULL), mValue(0.0f) {}
    // Query the channel value at specific time.
    float Get(const FbxTime & pTime) const
    {
        if (mAnimCurve)
        {
            return mAnimCurve->Evaluate(pTime);
        }
        else
        {
            return mValue;
        }
    }

    FbxAnimCurve * mAnimCurve;
    float mValue;
};

// Cache for FBX lights
class LightCache
{
public:
    LightCache();
    ~LightCache();

    // Set ambient light. Turn on light0 and set its attributes to default (white directional light in Z axis).
    // If the scene contains at least one light, the attributes of light0 will be overridden.
    static void IntializeEnvironment(const FbxColor & pAmbientLight);

    bool Initialize(const FbxLight * pLight, FbxAnimLayer * pAnimLayer);

    // Draw a geometry (sphere for point and directional light, cone for spot light).
    // And set light attributes.
    void SetLight(const FbxTime & pTime) const;

private:
    static int sLightCount;         // How many lights in this scene.

    int mLightIndex;
    FbxLight::EType mType;
    PropertyChannel mColorRed;
    PropertyChannel mColorGreen;
    PropertyChannel mColorBlue;
    PropertyChannel mConeAngle;
};

#endif // _SCENE_CACHE_H
