/****************************************************************************************

  Copyright (C) 2012 Autodesk, Inc.
  All rights reserved.

  Use of this software is subject to the terms of the Autodesk license agreement
  provided at the time of installation or download, or which otherwise accompanies
  this software in either electronic or hard copy form.

****************************************************************************************/

#include <GrapX.H>
//#include <clTree.H>
#include <clPathFile.H>

//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GShader.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GTexture.H>
//#include <3D/gvNode.h>
//#include <3D/gvMesh.h>
//#include <3D/gvScene.h>
//#include <3D/gvSkeleton.h>
//#include <3D/gvSkinnedMesh.h>
#include "GrapX/GrapVR.H"

#include <Engine.h>

#include "third_party\FBX SDK\2013.2\include\fbxsdk.h"
#include "GrapX/gxError.H"
#include "GetPosition.h"
#include "SceneCache.h"
#include "SceneContext.h"

//#pragma comment(lib, "d3dx9.lib")

void Dump(const float4x4& m)
{
  TRACE("%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f [float4x4]\n",
    m._11, m._12, m._13, m._14,
    m._21, m._22, m._23, m._24,
    m._31, m._32, m._33, m._34,
    m._41, m._42, m._43, m._44);
}
void Dump(const FbxAMatrix& m)
{
  TRACE("%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f [FbxAMatrix]\n",
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]);
}

quaternion* ComposeQuaternionFromFbxEuler(quaternion* q, float yaw, float pitch, float roll)
{
  q->YawPitchRollA(yaw, pitch, roll);
  float t = q->x;
  q->x = q->y;
  q->y = t;
  return q;
}

float4x4* ConvertMatrix(float4x4* pMat, const FbxAMatrix* pFbxMat)
{
  for(int i = 0; i < 16; i++)
  {
    pMat->m[i] = (float)(((double*)pFbxMat)[i]);
  }
  return pMat;
}

template<class _IObjectT>
GXHRESULT SaveObject(_IObjectT* pObject, GXLPCWSTR szFilename, GXLPCWSTR szExt)
{
  clStringW strMeshName;
  clStringW strSavedFilename = szFilename;
  if(clstd::strlenT(pObject->GetName()) == 0)
  {
    strMeshName.Format(L"_%x", pObject);
  }
  else
  {
    strMeshName = L"_";
    strMeshName += pObject->GetName();
  }
  clpathfile::RenameExtensionW(strSavedFilename, szExt);
  strSavedFilename.Insert(strSavedFilename.GetLength() - (clstd::strlenT(szExt) + 1), strMeshName);
  return pObject->SaveFileW(strSavedFilename);
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace
{
    const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;
    const float BLACK_COLOR[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const float GREEN_COLOR[] = {0.0f, 1.0f, 0.0f, 1.0f};
    const float WHITE_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const float WIREFRAME_COLOR[] = {0.5f, 0.5f, 0.5f, 1.0f};

    const int TRIANGLE_VERTEX_COUNT = 3;

    // Four floats for every position.
    const int VERTEX_STRIDE = 3;
    // Three floats for every normal.
    const int NORMAL_STRIDE = 3;
    // Two floats for every UV.
    const int UV_STRIDE = 2;

    const int TANGENT_STRIDE = 3;
    const int BINORMAL_STRIDE = 3;

    const float DEFAULT_LIGHT_POSITION[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const float DEFAULT_DIRECTION_LIGHT_POSITION[] = {0.0f, 0.0f, 1.0f, 0.0f};
    const float DEFAULT_SPOT_LIGHT_DIRECTION[] = {0.0f, 0.0f, -1.0f};
    const float DEFAULT_LIGHT_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const float DEFAULT_LIGHT_SPOT_CUTOFF = 180.0f;

    // Get specific property value and connected texture if any.
    // Value = Property value * Factor property value (if no factor property, multiply by 1).
    FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
        const char * pPropertyName,
        const char * pFactorPropertyName,
        int & pTextureName)
    {
        FbxDouble3 lResult(0, 0, 0);
        const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
        const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
        if (lProperty.IsValid() && lFactorProperty.IsValid())
        {
            lResult = lProperty.Get<FbxDouble3>();
            double lFactor = lFactorProperty.Get<FbxDouble>();
            if (lFactor != 1)
            {
                lResult[0] *= lFactor;
                lResult[1] *= lFactor;
                lResult[2] *= lFactor;
            }
        }

        if (lProperty.IsValid())
        {
            const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
            if (lTextureCount)
            {
                const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
                if (lTexture && lTexture->GetUserDataPtr())
                {
                    pTextureName = *(static_cast<int *>(lTexture->GetUserDataPtr()));
                }
            }
        }

        return lResult;
    }
}

VBOMesh::VBOMesh() 
  : m_bHasNormal  (false)
  , m_bHasUV      (false)
  , m_bAllByControlPoint(true)
  , m_bHasTangent(false)
  , m_bHasBinormal(false)
  , m_pSceneCtx (NULL)
  , m_pSkeleton (NULL)
  , m_pTrack    (NULL)
{
    // Reset every VBO to zero, which means no buffer.
    for (int lVBOIndex = 0; lVBOIndex < VBO_COUNT; ++lVBOIndex)
    {
        mVBONames[lVBOIndex] = 0;
    }
}

VBOMesh::~VBOMesh()
{
  SAFE_RELEASE(m_pTrack);
  SAFE_RELEASE(m_pSkeleton);
    // Delete VBO objects, zeros are ignored automatically.
    //glDeleteBuffers(VBO_COUNT, mVBONames);
	
//	FbxArrayDelete(mSubMeshes);

	for(int i=0; i < m_aSubMeshes.GetCount(); i++)
	{
		delete m_aSubMeshes[i];
	}	
	m_aSubMeshes.Clear();

}

bool VBOMesh::Initialize(SceneContext* pSceneCtx, const FbxMesh *pMesh)
{
    FbxNode* pNode = pMesh->GetNode();
    if ( ! pNode)
        return false;
    m_pSkeleton = new GVSkeleton(pSceneCtx->GetMarimoScene());
    m_pSkeleton->AddRef();
    m_pSceneCtx = pSceneCtx;
    const int lPolygonCount = pMesh->GetPolygonCount();

    //FbxAMatrix& GlobalMatrix = pNode->EvaluateGlobalTransform(0);
    //FbxAMatrix& GlobalMatrix = GetGeometry(pNode);
    //Dump(GlobalMatrix);

    // Count the polygon count of each material
    FbxLayerElementArrayTemplate<int>* aMaterialIndice = NULL;
    FbxGeometryElement::EMappingMode eMaterialMappingMode = FbxGeometryElement::eNone;

    GenerateSubMesh(pMesh, aMaterialIndice, eMaterialMappingMode);

    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    m_bHasNormal   = pMesh->GetElementNormalCount() > 0;
    m_bHasUV       = pMesh->GetElementUVCount() > 0;
    m_bHasTangent  = pMesh->GetElementTangentCount() > 0;
    m_bHasBinormal = pMesh->GetElementBinormalCount() > 0;

    FbxGeometryElement::EMappingMode lNormalMappingMode   = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode       = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lTangentMappingMode  = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lBinormalMappingMode = FbxGeometryElement::eNone;
    if(m_bHasNormal)
    {
        lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lNormalMappingMode == FbxGeometryElement::eNone)
        {
            m_bHasNormal = false;
        }
        if (m_bHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
        {
            m_bAllByControlPoint = false;
        }
    }

    if(m_bHasUV)
    {
        lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if(lUVMappingMode == FbxGeometryElement::eNone)
        {
            m_bHasUV = false;
        }
        if(m_bHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
        {
            m_bAllByControlPoint = false;
        }
    }

    if(m_bHasTangent)
    {
      lTangentMappingMode = pMesh->GetElementTangent(0)->GetMappingMode();
      if(lTangentMappingMode == FbxGeometryElement::eNone)
      {
        m_bHasTangent = false;
      }
      if(m_bHasTangent && lTangentMappingMode != FbxGeometryElement::eByControlPoint)
      {
        m_bAllByControlPoint = false;
      }
    }

    if(m_bHasBinormal)
    {
      lBinormalMappingMode = pMesh->GetElementBinormal(0)->GetMappingMode();
      if(lBinormalMappingMode == FbxGeometryElement::eNone)
      {
        m_bHasBinormal = false;
      }
      if(m_bHasBinormal && lBinormalMappingMode != FbxGeometryElement::eByControlPoint)
      {
        m_bAllByControlPoint = false;
      }
    }

    // Allocate the array memory, by control point or by polygon vertex.
    int nPolygonVertexCount = pMesh->GetControlPointsCount();
    if ( ! m_bAllByControlPoint)
    {
        nPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
    float* pVertices = new float[nPolygonVertexCount * VERTEX_STRIDE];
    unsigned int* pIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    float* pNormals   = NULL;
    float* pTangents  = NULL;
    float* pBinormals = NULL;

    if (m_bHasNormal) {
        pNormals = new float[nPolygonVertexCount * NORMAL_STRIDE];
    }

    if (m_bHasTangent) {
      pTangents = new float[nPolygonVertexCount * TANGENT_STRIDE];
    }

    if (m_bHasBinormal) {
      pBinormals = new float[nPolygonVertexCount * BINORMAL_STRIDE];
    }

    float* lUVs = NULL;
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    if (m_bHasUV && lUVNames.GetCount())
    {
        lUVs = new float[nPolygonVertexCount * UV_STRIDE];
        lUVName = lUVNames[0];
    }

    float* pCtrlWeight = NULL;
    float* pWeight = NULL;
    //float4x4* pCurrMat = NULL;
    int nVertexCount = pMesh->GetControlPointsCount();
    int nClusterCount = 0;

    const int nSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
    if(nSkinCount == 1)
    {
      const int nSkinIndex = 0;
      nClusterCount = ((FbxSkin *)(pMesh->GetDeformer(nSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
      pCtrlWeight = new float[nClusterCount * nVertexCount];
      //pCurrMat = new float4x4[nClusterCount];
      if(nPolygonVertexCount != nVertexCount) {
        pWeight = new float[nClusterCount * nPolygonVertexCount];
      }
      GenerateSkinWeight((FbxMesh*)pMesh, 0, pCtrlWeight, /*pCurrMat, */nVertexCount, nClusterCount);
    }
    else if(nClusterCount > 0)
    {
      CLOG_ERROR("暂时不支持多个Skin.\n");
      ASSERT(0);
    }


    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * pControlPoints = pMesh->GetControlPoints();
    FbxVector4 vCurrentVertex;
    FbxVector4 vCurrentNormal;
    FbxVector4 vCurrentTangent;
    FbxVector4 vCurrentBinormal;
    FbxVector2 vCurrentUV;
    if (m_bAllByControlPoint)
    {
        const FbxGeometryElementNormal*   pNormalElement   = NULL;
        const FbxGeometryElementUV*       pUVElement       = NULL;
        const FbxGeometryElementTangent*  pTangentElement  = NULL;
        const FbxGeometryElementBinormal* pBinormalElement = NULL;
        if(m_bHasNormal) {
            pNormalElement = pMesh->GetElementNormal(0);
        }
        if(m_bHasUV) {
            pUVElement = pMesh->GetElementUV(0);
        }
        if(m_bHasTangent) {
          pTangentElement = pMesh->GetElementTangent(0);
        }
        if(m_bHasBinormal) {
          pBinormalElement = pMesh->GetElementBinormal(0);
        }

        for (int lIndex = 0; lIndex < nPolygonVertexCount; ++lIndex)
        {
          // Save the vertex position.
          /*
          // Test
          float3 vCurrentVertex(0.0f);
          lCurrentVertex.Set(0,0,0);
          float3 vControlPoint(
            (float)pControlPoints[lIndex][0],
            (float)pControlPoints[lIndex][1],
            (float)pControlPoints[lIndex][2]);

          for(int nClusterIndex = 0; nClusterIndex < nClusterCount; nClusterIndex++)
          {
            float fWeight = pCtrlWeight[lIndex * nClusterCount + nClusterIndex];
            const Bone& bone = m_pSkeleton->GetBones()[nClusterIndex];
            vCurrentVertex += (vControlPoint * (bone.matInit * bone.matAbs)) * fWeight;

            // fbx test
            //FbxVector4 vMul = m_pSkeleton->GetBones()[nClusterIndex].m_fmAbs.MultT(pControlPoints[lIndex]);
            //lCurrentVertex += vMul * fWeight;
          }
          lCurrentVertex[0] = vCurrentVertex.x;
          lCurrentVertex[1] = vCurrentVertex.y;
          lCurrentVertex[2] = vCurrentVertex.z;
          TRACE("[%d] %f,%f,%f\n", lIndex, vCurrentVertex.x,vCurrentVertex.y,vCurrentVertex.z);
          // -- Test
          /*/
          vCurrentVertex = pControlPoints[lIndex];
          //*/

            pVertices[lIndex * VERTEX_STRIDE    ] = static_cast<float>(vCurrentVertex[0]);
            pVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(vCurrentVertex[1]);
            pVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(vCurrentVertex[2]);
            //pVertices[lIndex * VERTEX_STRIDE + 3] = 1;

            // Save the normal.
            if (m_bHasNormal)
            {
                int lNormalIndex = lIndex;
                if (pNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = pNormalElement->GetIndexArray().GetAt(lIndex);
                }
                vCurrentNormal = pNormalElement->GetDirectArray().GetAt(lNormalIndex);
                pNormals[lIndex * NORMAL_STRIDE    ] = static_cast<float>(vCurrentNormal[0]);
                pNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(vCurrentNormal[1]);
                pNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(vCurrentNormal[2]);
            }

            // Save the UV.
            if (m_bHasUV)
            {
                int lUVIndex = lIndex;
                if (pUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = pUVElement->GetIndexArray().GetAt(lIndex);
                }
                vCurrentUV = pUVElement->GetDirectArray().GetAt(lUVIndex);
                // 不明白这里为啥要乘0.5，使用一些实际模型发现不乘贴图才是对的
                //lUVs[lIndex * UV_STRIDE    ] = (static_cast<float>(vCurrentUV[0])) * 0.5f;
                //lUVs[lIndex * UV_STRIDE + 1] = (1 - static_cast<float>(vCurrentUV[1])) * 0.5f;
                lUVs[lIndex * UV_STRIDE    ] = (static_cast<float>(vCurrentUV[0]));
                lUVs[lIndex * UV_STRIDE + 1] = (1 - static_cast<float>(vCurrentUV[1]));
            }

            // Save the tangent.
            if (m_bHasTangent)
            {
              int lTangentIndex = lIndex;
              if (pTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
              {
                lTangentIndex = pTangentElement->GetIndexArray().GetAt(lIndex);
              }
              vCurrentTangent = pTangentElement->GetDirectArray().GetAt(lTangentIndex);
              pTangents[lIndex * TANGENT_STRIDE    ] = static_cast<float>(vCurrentTangent[0]);
              pTangents[lIndex * TANGENT_STRIDE + 1] = static_cast<float>(vCurrentTangent[1]);
              pTangents[lIndex * TANGENT_STRIDE + 2] = static_cast<float>(vCurrentTangent[2]);
            }

            // Save the binormal.
            if (m_bHasBinormal)
            {
              int lBinormalIndex = lIndex;
              if (pBinormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
              {
                lBinormalIndex = pBinormalElement->GetIndexArray().GetAt(lIndex);
              }
              vCurrentBinormal = pBinormalElement->GetDirectArray().GetAt(lBinormalIndex);
              pBinormals[lIndex * BINORMAL_STRIDE    ] = static_cast<float>(vCurrentBinormal[0]);
              pBinormals[lIndex * BINORMAL_STRIDE + 1] = static_cast<float>(vCurrentBinormal[1]);
              pBinormals[lIndex * BINORMAL_STRIDE + 2] = static_cast<float>(vCurrentBinormal[2]);
            }
        }
    }

    int lVertexCount = 0;
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        // The material for current face.
        int nMaterialIndex = 0;
        if (aMaterialIndice && eMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            nMaterialIndex = aMaterialIndice->GetAt(lPolygonIndex);
        }

        // Where should I save the vertex attribute index, according to the material
        const int lIndexOffset = m_aSubMeshes[nMaterialIndex]->IndexOffset +
            m_aSubMeshes[nMaterialIndex]->TriangleCount * 3;
        for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

            if (m_bAllByControlPoint)
            {
                pIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
            }
            // Populate the array with vertex attribute, if by polygon vertex.
            else
            {
                pIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

                /*
                // Test
                float3 vCurrentVertex(0.0f);
                lCurrentVertex.Set(0,0,0);
                float3 vControlPoint(
                  (float)pControlPoints[lControlPointIndex][0],
                  (float)pControlPoints[lControlPointIndex][1],
                  (float)pControlPoints[lControlPointIndex][2]);

                for(int nClusterIndex = 0; nClusterIndex < nClusterCount; nClusterIndex++)
                {
                  float fWeight = pCtrlWeight[lControlPointIndex * nClusterCount + nClusterIndex];
                  const Bone& bone = m_pSkeleton->GetBones()[nClusterIndex];
                  vCurrentVertex += (vControlPoint * (bone.matInit * bone.matAbs)) * fWeight;

                  // fbx test
                  //FbxVector4 vMul = m_pSkeleton->GetBones()[nClusterIndex].m_fmAbs.MultT(pControlPoints[lControlPointIndex]);
                  //lCurrentVertex += vMul * fWeight;
                }
                lCurrentVertex[0] = vCurrentVertex.x;
                lCurrentVertex[1] = vCurrentVertex.y;
                lCurrentVertex[2] = vCurrentVertex.z;
                // -- Test /
                /*/
                vCurrentVertex = pControlPoints[lControlPointIndex];
                //*/
                memcpy(&pWeight[nClusterCount * lVertexCount], &pCtrlWeight[nClusterCount * lControlPointIndex], sizeof(float) * nClusterCount);
                pVertices[lVertexCount * VERTEX_STRIDE    ] = static_cast<float>(vCurrentVertex[0]);
                pVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(vCurrentVertex[1]);
                pVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(vCurrentVertex[2]);
                //pVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;

                if (m_bHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, vCurrentNormal);
                    pNormals[lVertexCount * NORMAL_STRIDE    ] = static_cast<float>(vCurrentNormal[0]);
                    pNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(vCurrentNormal[1]);
                    pNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(vCurrentNormal[2]);
                }

                if (m_bHasUV)
                {
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, vCurrentUV);
                    lUVs[lVertexCount * UV_STRIDE    ] = (static_cast<float>(vCurrentUV[0]));// * 0.5f;
                    lUVs[lVertexCount * UV_STRIDE + 1] = (1 - static_cast<float>(vCurrentUV[1]));// * 0.5f;
                }

                if (m_bHasTangent)
                {
                  const FbxGeometryElementTangent* pTangentElement = pMesh->GetElementTangent(0);
                  ASSERT(pTangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

                  ReadElement(pTangentElement, lPolygonIndex, lVerticeIndex, vCurrentTangent);

                  pTangents[lVertexCount * TANGENT_STRIDE    ] = static_cast<float>(vCurrentTangent[0]);
                  pTangents[lVertexCount * TANGENT_STRIDE + 1] = static_cast<float>(vCurrentTangent[1]);
                  pTangents[lVertexCount * TANGENT_STRIDE + 2] = static_cast<float>(vCurrentTangent[2]);
                }

                if (m_bHasBinormal)
                {
                  const FbxGeometryElementBinormal* pBinormalElement = pMesh->GetElementBinormal(0);
                  ASSERT(pBinormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

                  ReadElement(pBinormalElement, lPolygonIndex, lVerticeIndex, vCurrentBinormal);

                  pBinormals[lVertexCount * BINORMAL_STRIDE    ] = static_cast<float>(vCurrentBinormal[0]);
                  pBinormals[lVertexCount * BINORMAL_STRIDE + 1] = static_cast<float>(vCurrentBinormal[1]);
                  pBinormals[lVertexCount * BINORMAL_STRIDE + 2] = static_cast<float>(vCurrentBinormal[2]);

                  //const FbxGeometryElementBinormal* pBinormalElement = pMesh->GetElementBinormal(0);
                  //pMesh->GetPolygonVertexLayerElementValue<FbxGeometryElementBinormal>(
                  //  pBinormalElement, lPolygonIndex, lVerticeIndex, vCurrentBinormal);

                  //pBinormals[lVertexCount * BINORMAL_STRIDE    ] = static_cast<float>(vCurrentBinormal[0]);
                  //pBinormals[lVertexCount * BINORMAL_STRIDE + 1] = static_cast<float>(vCurrentBinormal[1]);
                  //pBinormals[lVertexCount * BINORMAL_STRIDE + 2] = static_cast<float>(vCurrentBinormal[2]);
                }
            }
            ++lVertexCount;
        }
        m_aSubMeshes[nMaterialIndex]->TriangleCount += 1;
    }
    GVMESHDATA MeshData;
    InlSetZeroT(MeshData);
    MeshData.nVertexCount = nPolygonVertexCount;
    MeshData.pVertices = (float3*)pVertices;

    //GXVERTEX_P3T2N3F* pMOVertex = new GXVERTEX_P3T2N3F[nPolygonVertexCount];
    // Create VBOs
    //glGenBuffers(VBO_COUNT, mVBONames);

    // Save vertex attributes into GPU
    //glBindBuffer(GL_ARRAY_BUFFER, mVBONames[VERTEX_VBO]);
    //glBufferData(GL_ARRAY_BUFFER, nPolygonVertexCount * VERTEX_STRIDE * sizeof(float), lVertices, GL_STATIC_DRAW);
    //for(int i = 0; i < nPolygonVertexCount; i++) {
    //  pMOVertex[i].pos.set(pVertices[i * VERTEX_STRIDE], pVertices[i * VERTEX_STRIDE + 1], pVertices[i * VERTEX_STRIDE + 2]);
    //}
    //delete [] pVertices;

    if(m_bHasNormal) {
        //glBindBuffer(GL_ARRAY_BUFFER, mVBONames[NORMAL_VBO]);
        //glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * NORMAL_STRIDE * sizeof(float), pNormals, GL_STATIC_DRAW);
      //for(int i = 0; i < nPolygonVertexCount; i++) {
      //  pMOVertex[i].normal.set(pNormals[i * NORMAL_STRIDE], 
      //    pNormals[i * NORMAL_STRIDE + 1], pNormals[i * NORMAL_STRIDE + 2]);
      //  pMOVertex[i].normal.normalize();
      //}
      //delete [] pNormals;
      MeshData.pNormals = (float3*)pNormals;
    }
    
    if(m_bHasUV) {
        //glBindBuffer(GL_ARRAY_BUFFER, mVBONames[UV_VBO]);
        //glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * UV_STRIDE * sizeof(float), lUVs, GL_STATIC_DRAW);
      //for(int i = 0; i < nPolygonVertexCount; i++) {
      //  pMOVertex[i].texcoord.set(lUVs[i * UV_STRIDE], lUVs[i * UV_STRIDE + 1]);
      //}
      //delete [] lUVs;
      MeshData.pTexcoord0 = (float2*)lUVs;
    }

    if(m_bHasTangent) {
      MeshData.pTangents = (float3*)pTangents;
    }

    if(m_bHasBinormal) {
      MeshData.pBinormals = (float3*)pBinormals;
    }

    GXWORD* pMOIndices = new GXWORD[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    for(int i = 0; i < lPolygonCount * TRIANGLE_VERTEX_COUNT; i++)
    {
      pMOIndices[i] = (GXWORD)pIndices[i];
    }
    GVScene* pScene = pSceneCtx->GetMarimoScene();

    GVMesh* pMoMesh = NULL;

    if(nSkinCount > 0)
    {
      GVSkinnedMeshSoft* pSkinMesh = NULL;
      MeshData.pIndices = pMOIndices;
      MeshData.nIndexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
      GVSkinnedMeshSoft::CreateFromMeshData(pScene->GetGraphicsUnsafe(), 
        &MeshData, m_pSkeleton, pWeight == NULL ? pCtrlWeight : pWeight, nClusterCount, &pSkinMesh);

      //GVSkinnedMeshSoft::CreateMesh(
      //  pScene->GetGraphicsUnsafe(), lPolygonCount, MOGetSysVertexDecl(GXVD_P3T2N3F), pMOVertex,
      //  nPolygonVertexCount, pMOIndices, lPolygonCount * TRIANGLE_VERTEX_COUNT, m_pSkeleton, 
      //  pWeight == NULL ? pCtrlWeight : pWeight, nClusterCount, &pSkinMesh);

      pMoMesh = static_cast<GVMesh*>(pSkinMesh);
    }
    else {

      MeshData.pIndices = pMOIndices;
      MeshData.nIndexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
      GVMesh::CreateMesh(pScene->GetGraphicsUnsafe(), &MeshData, &pMoMesh);

      //GVMesh::CreateUserPrimitive(
      //  pScene->GetGraphicsUnsafe(), lPolygonCount, MOGetSysVertexDecl(GXVD_P3T2N3F), pMOVertex,
      //  nPolygonVertexCount, pMOIndices, lPolygonCount * TRIANGLE_VERTEX_COUNT, &pMoMesh);
    
      float4x4 matGlobalMesh;
      FbxAMatrix GlobalTransform = GetGlobalPosition(pNode, 0);
      FbxAMatrix GeometryTransform = GetGeometry(pNode);
      //GetGeometry()
      //ConvertMatrix(&matGlobalMesh, &(GlobalTransform * GeometryTransform));
      ConvertMatrix(&matGlobalMesh, &(GlobalTransform * GeometryTransform));
      pMoMesh->SetTransform(matGlobalMesh);
    }

    if(pMoMesh != NULL) {
      //GXGraphics* pGraphics = pSceneCtx->GetMarimoScene()->GetGraphicsUnsafe();
      int nSubMeshCount = m_aSubMeshes.Size();
      if(nSubMeshCount == 1)
      {
        //pDefaultMtlInst->Clone(&pMtlInst);

        //if(pSceneCtx->LoadTextureFromFile(m_aSubMeshes[0]->strMajorTexture, &pTexture))
        //{
        //  pMtlInst->SetTextureByName("Simple_Sampler", pTexture);
        //  SAFE_RELEASE(pTexture);
        //}

        //pMoMesh->SetMaterialInst(pMtlInst, TRUE);
        //SAFE_RELEASE(pMtlInst);
        SetMeshMtl(0, pMoMesh);
      }
      else if(nSubMeshCount > 0)
      {
        GVRENDERDESC Desc;
        pMoMesh->SetFlags(pMoMesh->GetFlags() | GVNF_CONTAINER);
        pMoMesh->GetRenderDesc(GVRT_Normal, &Desc);

        for(int nMeshIndex = 0; nMeshIndex < nSubMeshCount; nMeshIndex++)
        {
          GVMesh* pSubMesh = NULL;

          if(m_aSubMeshes[nMeshIndex] != NULL) {
            GVMesh::CreateUserPrimitive(pScene->GetGraphicsUnsafe(),
              m_aSubMeshes[nMeshIndex]->TriangleCount, m_aSubMeshes[nMeshIndex]->IndexOffset,
              (GPrimitiveVI*)Desc.pPrimitive, &pSubMesh);

            SetMeshMtl(nMeshIndex, pSubMesh);
            pSubMesh->SetParent(pMoMesh);
          }
        }
      }

      GXWCHAR buffer[MAX_PATH];
      const char* szUtf8Name = pNode->GetName();
      memset(buffer, 0, MAX_PATH * sizeof(GXWCHAR));
      MultiByteToWideChar(CP_UTF8, 0, szUtf8Name, GXSTRLEN(szUtf8Name), buffer, MAX_PATH);

      pMoMesh->SetName(clStringA(buffer));
      if(m_pSceneCtx->IsSaveToMyFormat())
      {
        SaveObject(pMoMesh, m_pSceneCtx->GetFilename(), L"SDB");
        SaveObject(m_pSkeleton, m_pSceneCtx->GetFilename(), L"BONE.SDB");
        SaveObject(m_pTrack, m_pSceneCtx->GetFilename(), L"TCK.SDB");
      }

      if(m_pSceneCtx->IsAddToScene())
      {
        pScene->Add(pMoMesh);
      }

      if(m_pSceneCtx->UseTransformedVertices())
      {
        pMoMesh->ApplyTransform();
      }

      pSceneCtx->AddNode(pMoMesh);
      if(nSkinCount > 0)
      {
        m_pSkeleton->BuildRenderData(pScene->GetGraphicsUnsafe());
        pScene->Add(m_pSkeleton);
        pSceneCtx->AddNode(m_pSkeleton);
      }
      SAFE_RELEASE(pMoMesh);
    }

    delete [] pTangents;
    delete [] pBinormals;
    delete [] pIndices;
    delete [] pVertices;
    delete [] pNormals;
    delete [] lUVs;

    SAFE_DELETE_ARRAY(pMOIndices);
    SAFE_DELETE_ARRAY(pWeight);
    SAFE_DELETE_ARRAY(pCtrlWeight);

    return true;
}

void VBOMesh::SetMeshMtl(int nIndex, GVMesh* pMoMesh)
{
  GXMaterialInst* pDefaultMtlInst = m_pSceneCtx->GetDefaultMtlInst();
  GTexture* pTexture = NULL;

  pMoMesh->SetMaterialInst(pDefaultMtlInst, NODEMTL_CLONEINST | NODEMTL_SETCHILDREN); 

  if(m_pSceneCtx->LoadTextureFromFile(m_aSubMeshes[nIndex]->strMajorTexture, &pTexture))
  {
    GXMaterialInst* pMtlInst = NULL;
    pMoMesh->GetMaterialInst(&pMtlInst);
    pMtlInst->SetTextureByName("MainSampler", pTexture);
    SAFE_RELEASE(pMtlInst);
    SAFE_RELEASE(pTexture);
  }

  //pMoMesh->SetMaterialInst(pMtlInst, TRUE);
  //SAFE_RELEASE(pMtlInst);
}

void VBOMesh::GenerateSubMesh(const FbxMesh *pMesh, FbxLayerElementArrayTemplate<int>*& aMaterialIndice, FbxGeometryElement::EMappingMode& eMaterialMappingMode)
{
  const int lPolygonCount = pMesh->GetPolygonCount();
  const FbxGeometryElementMaterial* pGeoElementMtl = pMesh->GetElementMaterial();
  if (pGeoElementMtl)
  {
    aMaterialIndice = &pGeoElementMtl->GetIndexArray();
    eMaterialMappingMode = pGeoElementMtl->GetMappingMode();
    if (aMaterialIndice && eMaterialMappingMode == FbxGeometryElement::eByPolygon)
    {
      ASSERT(aMaterialIndice->GetCount() == lPolygonCount);
      if (aMaterialIndice->GetCount() == lPolygonCount)
      {
        // Count the faces of each material
        for (int nPolygonIndex = 0; nPolygonIndex < lPolygonCount; ++nPolygonIndex)
        {
          const int nMaterialIndex = aMaterialIndice->GetAt(nPolygonIndex);
          if (m_aSubMeshes.GetCount() < nMaterialIndex + 1)
          {
            m_aSubMeshes.Resize(nMaterialIndex + 1);
          }
          if (m_aSubMeshes[nMaterialIndex] == NULL)
          {
            m_aSubMeshes[nMaterialIndex] = new SubMesh;
          }
          m_aSubMeshes[nMaterialIndex]->TriangleCount += 1;
        }

        // Record the offset (how many vertex)
        const int nMaterialCount = m_aSubMeshes.GetCount();
        int nOffset = 0;
        for (int nIndex = 0; nIndex < nMaterialCount; ++nIndex)
        {
          if(m_aSubMeshes[nIndex] == NULL) {
            CLOG_WARNING(__FUNCTION__": 导出的材质中有空材质, 可能是由于Max不支持插件材质导致的.\n");
            continue;
          }
          m_aSubMeshes[nIndex]->IndexOffset = nOffset;
          nOffset += m_aSubMeshes[nIndex]->TriangleCount * 3;

          // This will be used as counter in the following procedures, reset to zero
          m_aSubMeshes[nIndex]->TriangleCount = 0;
        }
        ASSERT(nOffset == lPolygonCount * 3);
      }
    }
  }//*/

  // All faces will use the same material.
  if (m_aSubMeshes.GetCount() == 0)
  {
    m_aSubMeshes.Resize(1);
    m_aSubMeshes[0] = new SubMesh();
  }

  FbxNode* pNode = pMesh->GetNode();
  int nMaterialCount = pNode->GetMaterialCount();
  const int nTextureLayerIndex = 0;
  for(int i = 0; i < m_aSubMeshes.Size(); ++i)
  {
    GXWCHAR buffer[MAX_PATH] = L"";
    FbxSurfaceMaterial* pSurMtl = pNode->GetMaterial(i);

    if(pSurMtl == NULL) continue;

    FbxProperty pProperty = pSurMtl->FindProperty(FbxLayerElement::sTextureChannelNames[nTextureLayerIndex]);  
    if(pProperty.IsValid())  
    {  
      int textureCount = pProperty.GetSrcObjectCount(FbxCriteria::ObjectType(FbxTexture::ClassId));  

      //for(int j = 0 ; j < textureCount ; ++j)  
      int j = 0;
      {  
        FbxTexture* pTexture = FbxCast<FbxTexture>(pProperty.GetSrcObject(FbxCriteria::ObjectType(FbxTexture::ClassId), j));  
        if(pTexture != NULL)
        {
          FbxFileTexture * pFileTexture = FbxCast<FbxFileTexture>(pTexture);
          const char* szName = pFileTexture->GetFileName();
          memset(buffer, 0, MAX_PATH * sizeof(GXWCHAR));
          MultiByteToWideChar(CP_UTF8, 0, szName, GXSTRLEN(szName), buffer, MAX_PATH);
        }
      }
    }
    if(m_aSubMeshes[i] != NULL) {
      m_aSubMeshes[i]->strMajorTexture = buffer;
    }
  }
}

void VBOMesh::GenerateSkinWeight(FbxMesh* pMesh, int nSkinIndex, float* pWeight/*, float4x4* pTransform*/, int nVertexCount, int nClusterCount)
{

  FbxSkin* pSkinDeformer = (FbxSkin*)pMesh->GetDeformer(nSkinIndex, FbxDeformer::eSkin);
  FbxSkin::EType eSkinningType = pSkinDeformer->GetSkinningType();

  if(eSkinningType == FbxSkin::eLinear || eSkinningType == FbxSkin::eRigid)
  {
    GenLinearDeformationWeight(pSkinDeformer, pWeight/*, pTransform*/, nVertexCount, nClusterCount);
  }
}


void VBOMesh::GenLinearDeformationWeight(FbxSkin* pSkinDeformer, float* pWeight/*, float4x4* pTransform*/, int nVertexCount, int nClusterCount)
{
  //int nClusterCount = pSkinDeformer->GetClusterCount();
  //float* pWeight = new float[nVertexCount * nClusterCount];
  memset(pWeight, 0, nClusterCount * nVertexCount * sizeof(float));
  //memset(pTransform, 0, nClusterCount * sizeof(float4x4)); // 这个测试的
  FbxTime StartTime = m_pSceneCtx->GetStartTime();
  FbxTime StopTime = m_pSceneCtx->GetStopTime();
  FbxLongLong nFrameCount = StopTime.GetFrameCount();
  FbxTime FrameFreqTime = (StopTime - StartTime) / nFrameCount;

  m_aBoneInfo.reserve(nClusterCount);
  //m_pSkeleton->SetFrameInfo(nFrameCount, static_cast<int>((FbxLongLong)StopTime.GetFrameRate(FbxTime::eDefaultMode)));

  struct BONE_CONTEXT
  {
    int         nIndex;
    FbxNode*    pParentNode;
  };
  typedef clvector<BONE_CONTEXT> ContextArray;
  ContextArray aBoneContext;

  const int nClusterCountCheck = pSkinDeformer->GetClusterCount();
  ASSERT(nClusterCountCheck == nClusterCount);
  for(int nClusterIndex = 0; nClusterIndex < nClusterCount; ++nClusterIndex)
  {
    Bone bone;  // 放在这里, 每次都会初始化新的
    BONEINFO BoneInfo;

    FbxCluster* pCluster = pSkinDeformer->GetCluster(nClusterIndex);
    FbxCluster::ELinkMode eClusterMode = pCluster->GetLinkMode();
    ASSERT(eClusterMode == FbxCluster::eNormalize);
    // 获得动画的接口: pCluster->GetLink()->EvaluateLocalTranslation();

    FbxNode* pNode = pCluster->GetLink();
    if ( ! pNode)
      continue;

    FbxNode* pParentNode = pNode->GetParent();

    //TRACE("%s", pNode->GetName());

    if(pParentNode)
    {
      //aNames.push_back(pParentNode->GetName());
      //TRACE("\t%s\n", pParentNode->GetName());
    }
    else
    {
      //aNames.push_back(NULL);
    }

    //bone.nParent = -1;
    bone.Name = pNode->GetName();
    GenerateAnimData(bone, BoneInfo, pCluster, pNode, StartTime, FrameFreqTime, nFrameCount);

    int nIndex = m_pSkeleton->AddBone(bone);
    m_aBoneInfo.push_back(BoneInfo);
    if(pParentNode != NULL)
    {
      int nParentIndex = m_pSkeleton->SetParent(nIndex, pParentNode->GetName());
      if(nParentIndex < 0)
      {
        BONE_CONTEXT bc;

        bc.nIndex = nIndex;
        bc.pParentNode = pParentNode;

        aBoneContext.push_back(bc);
      }
    }


    int nVertexIndexCount = pCluster->GetControlPointIndicesCount();
    for (int k = 0; k < nVertexIndexCount; ++k) 
    {            
      int nIndex = pCluster->GetControlPointIndices()[k];

      // Sometimes, the mesh can have less points than at the time of the skinning
      // because a smooth operator was active when skinning but has been deactivated during export.
      if (nIndex >= nVertexCount)
        continue;

      float fWeight = (float)pCluster->GetControlPointWeights()[k];

      if (fWeight == 0.0f)
      {
        continue;
      }
      pWeight[nClusterCount * nIndex + nClusterIndex] = fWeight;
    }
  }

  //m_pSkeleton->BuildDict();
  //m_pSkeleton->SetParents((GXLPCSTR*)&aNames.front());
  FbxNode* pRootNode = m_pSceneCtx->GetFbxScene()->GetRootNode();
  for(ContextArray::iterator it = aBoneContext.begin();
    it != aBoneContext.end(); ++it)
  {
    int nParentIndex = m_pSkeleton->SetParent(it->nIndex, it->pParentNode->GetName());
    if(nParentIndex < 0)
    {
      //TRACE("找不到父对象的: %s\n", it->pParentNode->GetName());
      FbxNode* pNode = it->pParentNode;
      if(pNode != NULL && pNode != pRootNode)
      {
        AddUndeformerBone(pNode);
        m_pSkeleton->SetParent(it->nIndex, pNode->GetName());
      }
    }
  }
  //GVAnimationTrack* pTrack;
  float3* pTSData = m_aTS.size() == 0 ? NULL : &m_aTS.front();
  quaternion* pQuaterData = m_aQuaternions.size() == 0 ? NULL : &m_aQuaternions.front();
  GVAnimationTrack::CreateAnimationTrack(&m_pTrack, NULL, (int)nFrameCount, (int)(FbxLongLong)StopTime.GetFrameRate(FbxTime::eDefaultMode),
    pTSData, m_aTS.size(), pQuaterData, m_aQuaternions.size(), &m_aBoneInfo.front(), m_aBoneInfo.size());

  int nId = m_pSkeleton->SetTrackData(NULL, m_pTrack);
  m_pSkeleton->PlayById(nId);
  m_pSkeleton->UpdateBones();
  m_pSkeleton->DbgDump();
  //if(m_pSceneCtx->IsSaveToMyFormat())
  //{
  //  SaveObject(m_pSkeleton, m_pSceneCtx->GetFilename(), L"BONE.SDB");
  //  SaveObject(pTrack, m_pSceneCtx->GetFilename(), L"TCK.SDB");
  //}
  //SAFE_RELEASE(pTrack);
}

void VBOMesh::GenerateAnimData(Bone& bone, GVAnimationTrack::BONEINFO& BoneInfo, FbxCluster* pCluster, FbxNode* pNode, FbxTime StartTime, FbxTime FrameFreq, FbxLongLong nFrameCount)
{
  ASSERT(pCluster == NULL || pCluster->GetLink() == pNode);

  // 全局矩阵
  FbxAMatrix GlobalMat;
  GlobalMat.SetIdentity();

  // 计算初始矩阵
  if(pCluster) {
    FbxAMatrix VertexTransformMat;
    ComputeClusterDeformation(GlobalMat, pNode, pCluster, VertexTransformMat, NULL);
    ConvertMatrix(&bone.BindPose, &VertexTransformMat);
  }

  GVAnimationTrack::Vector3Array aTranslations;
  GVAnimationTrack::QuaternionArray aQuaternions;
  GVAnimationTrack::Vector3Array aScalings;

  GXBOOL bStaticT = TRUE;
  GXBOOL bStaticS = TRUE;
  GXBOOL bStaticR = TRUE;

  for(int nFrameIndex = 0; nFrameIndex < nFrameCount; nFrameIndex++)
  {
    FbxTime CurrTime = StartTime + FrameFreq * nFrameIndex;
    FbxVector4 LclTranslation = pNode->EvaluateLocalTranslation(CurrTime);
    FbxVector4 LclRotation    = pNode->EvaluateLocalRotation(CurrTime);
    FbxVector4 LclScaling     = pNode->EvaluateLocalScaling(CurrTime);

    if(pNode->GetRotationActive())
    {
      FbxAMatrix Result = pNode->EvaluateLocalTransform(CurrTime);

      LclTranslation = Result.GetT();
      LclRotation    = Result.GetR();
      LclScaling     = Result.GetS();
    }
    // [1] [0] [2]
    //TRACE("%4d: %f,%f,%f,%f\t%f,%f,%f,%f\t%f,%f,%f,%f\n", nFrameIndex, 
    //  LclTranslation.mData[0], LclTranslation.mData[1], LclTranslation.mData[2], LclTranslation.mData[3],
    //  LclRotation.mData[0], LclRotation.mData[1], LclRotation.mData[2], LclRotation.mData[3],
    //  LclScaling.mData[0], LclScaling.mData[1], LclScaling.mData[2], LclScaling.mData[3]);

    quaternion q;
    float3 t((float)LclTranslation[0], (float)LclTranslation[1], (float)LclTranslation[2]);
    float3 s((float)LclScaling[0], (float)LclScaling[1], (float)LclScaling[2]);

    ComposeQuaternionFromFbxEuler(&q, (float)LclRotation[0],
      (float)LclRotation[1], (float)LclRotation[2]);

    aTranslations.push_back(t);
    aQuaternions.push_back(q);
    aScalings.push_back(s);

    if(nFrameIndex == 0)
    {
      EFbxRotationOrder eOrder;
      pNode->GetRotationOrder(FbxNode::eSourcePivot, eOrder);
      ASSERT(eOrder == eEulerXYZ);

      // Debug 代码
#ifdef FBX_SDK
      const FbxAMatrix& GlbTransform = pNode->EvaluateGlobalTransform(CurrTime);
      const FbxAMatrix& LclTransform = pNode->EvaluateLocalTransform(CurrTime);

      bone.fmLocal.SetTRS(LclTranslation, LclRotation, LclScaling);
      bone.fmAbs = *(FbxAMatrix*)&GlbTransform;
      //if(LclTransform != bone.fmLocal)
      //{
      //  FbxAMatrix fmA, fmB;
      //  fmA.SetR(PreRotation);
      //  fmB.SetR(LclRotation);
      //  fmA = fmA * fmB;
      //  FbxVector4 fbxr2 = fmA.GetR();

      //  FbxVector4 fbxt = LclTransform.GetT();
      //  FbxVector4 fbxr = LclTransform.GetR();
      //  FbxVector4 fbxs = LclTransform.GetS();
      //  Dump(LclTransform);
      //  TRACE("\n");
      //  Dump(bone.fmLocal);
      //  TRACE("\n\n");
      //}

      FbxNode* pParentNode = pNode->GetParent();
      if(pParentNode != NULL) {
        FbxAMatrix ParentTransform = pParentNode->EvaluateGlobalTransform(CurrTime);
        FbxAMatrix CheckTransform = ParentTransform * bone.fmLocal;
        if(CheckTransform != GlbTransform)
        {
          Dump(CheckTransform);
          TRACE("\n");
          Dump(GlbTransform);
          TRACE("\n\n");
        }
      }
#endif // #ifdef FBX_SDK

      BoneInfo.vTranslation = t;
      BoneInfo.vQuaternion = q;
      BoneInfo.vScaling = s;

      bone.matLocal.AffineTransformation(&s, NULL, &q, &t);
    }
    else
    {
      if(t != BoneInfo.vTranslation) {
        bStaticT = FALSE;
      }
      if(q != BoneInfo.vQuaternion) {
        bStaticR = FALSE;
      }
      if(s != BoneInfo.vScaling) {
        bStaticS = FALSE;
      }
    }
  }
  if( ! bStaticT)
  {
    BoneInfo.nTranslationIdx = m_aTS.size();
    for(GVAnimationTrack::Vector3Array::iterator it = aTranslations.begin();
      it != aTranslations.end(); ++it)
    {
      m_aTS.push_back(*it);
    }
    ASSERT(m_aTS[BoneInfo.nTranslationIdx] == BoneInfo.vTranslation);
  }
  if( ! bStaticR)
  {
    BoneInfo.nQuaternionIdx = m_aQuaternions.size();
    for(GVAnimationTrack::QuaternionArray::iterator it = aQuaternions.begin();
      it != aQuaternions.end(); ++it)
    {
      m_aQuaternions.push_back(*it);
    }
    ASSERT(m_aQuaternions[BoneInfo.nQuaternionIdx] == BoneInfo.vQuaternion);
  }
  if( ! bStaticS)
  {
    BoneInfo.nScalingIdx = m_aTS.size();
    for(GVAnimationTrack::Vector3Array::iterator it = aScalings.begin();
      it != aScalings.end(); ++it)
    {
      m_aTS.push_back(*it);
    }
    ASSERT(m_aTS[BoneInfo.nScalingIdx] == BoneInfo.vScaling);
  }
}

int VBOMesh::AddUndeformerBone(FbxNode* pNode)
{
  Bone bone;
  BONEINFO BoneInfo;
  //bone.nParent = -1;
  bone.Name = pNode->GetName();
  //bone.matAbs.identity();
  //bone.fmAbs.SetIdentity();
  //bone.m_vGlbPos.set(0,0,0);

  const FbxVector4& LclRotation = pNode->EvaluateLocalRotation();
  const FbxVector4& LclTranslation = pNode->EvaluateLocalTranslation();
  const FbxVector4& LclScaling = pNode->EvaluateLocalScaling();
  const FbxMatrix& GlbTransform = pNode->EvaluateGlobalTransform();

  //if(bone.Name == "Humanoid:Hips")
  //  __asm nop
  float3 t((float)LclTranslation[0], (float)LclTranslation[1], (float)LclTranslation[2]);
  float3 s((float)LclScaling[0], (float)LclScaling[1], (float)LclScaling[2]);
  quaternion q;
  //q.YawPitchRollA((float)LclRotation[1], (float)LclRotation[0], (float)LclRotation[2]);
  ComposeQuaternionFromFbxEuler(&q, (float)LclRotation[0],
    (float)LclRotation[1], (float)LclRotation[2]);

  bone.matLocal.AffineTransformation(&s, NULL, &q, &t);
#ifdef FBX_SDK
  bone.fmLocal.SetTRS(LclTranslation, LclRotation, LclScaling);
  bone.fmAbs = *(FbxAMatrix*)&GlbTransform;
#endif // #ifdef FBX_SDK

  GenerateAnimData(bone, BoneInfo, NULL, pNode, 0, 0, 1);

  int nIndex = m_pSkeleton->AddBone(bone);
  m_aBoneInfo.push_back(BoneInfo);

  FbxNode* pParentNode = pNode->GetParent();
  FbxNode* pRootNode = m_pSceneCtx->GetFbxScene()->GetRootNode();
  //TRACE("%s typename is %s\n", pNode->GetName(), pNode->GetTypeName());
  if(pParentNode != NULL && pParentNode != pRootNode)
  {
    int nParentIndex = m_pSkeleton->SetParent(nIndex, pParentNode->GetName());
    if(nParentIndex < 0) {
      AddUndeformerBone(pParentNode);
      m_pSkeleton->SetParent(nIndex, pParentNode->GetName());
    }
  }
  else {
    FbxAMatrix matRoot = GetGlobalPosition(pNode, 0);
    TRACE("%s\n", pNode->GetName());
    Dump(matRoot);
  }
  return nIndex;
  //pNode = pNode->GetParent();
}



MaterialCache::MaterialCache() : mShinness(0)
{

}

MaterialCache::~MaterialCache()
{

}

// Bake material properties.
bool MaterialCache::Initialize(const FbxSurfaceMaterial * pMaterial)
{
    const FbxDouble3 lEmissive = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, mEmissive.mTextureName);
    mEmissive.mColor[0] = static_cast<float>(lEmissive[0]);
    mEmissive.mColor[1] = static_cast<float>(lEmissive[1]);
    mEmissive.mColor[2] = static_cast<float>(lEmissive[2]);

    const FbxDouble3 lAmbient = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, mAmbient.mTextureName);
    mAmbient.mColor[0] = static_cast<float>(lAmbient[0]);
    mAmbient.mColor[1] = static_cast<float>(lAmbient[1]);
    mAmbient.mColor[2] = static_cast<float>(lAmbient[2]);

    const FbxDouble3 lDiffuse = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, mDiffuse.mTextureName);
    mDiffuse.mColor[0] = static_cast<float>(lDiffuse[0]);
    mDiffuse.mColor[1] = static_cast<float>(lDiffuse[1]);
    mDiffuse.mColor[2] = static_cast<float>(lDiffuse[2]);

    const FbxDouble3 lSpecular = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, mSpecular.mTextureName);
    mSpecular.mColor[0] = static_cast<float>(lSpecular[0]);
    mSpecular.mColor[1] = static_cast<float>(lSpecular[1]);
    mSpecular.mColor[2] = static_cast<float>(lSpecular[2]);

    FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        mShinness = static_cast<float>(lShininess);
    }

    return true;
}

void MaterialCache::SetCurrentMaterial() const
{
    //glMaterialfv(GL_FRONT, GL_EMISSION, mEmissive.mColor);
    //glMaterialfv(GL_FRONT, GL_AMBIENT, mAmbient.mColor);
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, mDiffuse.mColor);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, mSpecular.mColor);
    //glMaterialf(GL_FRONT, GL_SHININESS, mShinness);

    //glBindTexture(GL_TEXTURE_2D, mDiffuse.mTextureName);
}

void MaterialCache::SetDefaultMaterial()
{
    //glMaterialfv(GL_FRONT, GL_EMISSION, BLACK_COLOR);
    //glMaterialfv(GL_FRONT, GL_AMBIENT, BLACK_COLOR);
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, GREEN_COLOR);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, BLACK_COLOR);
    //glMaterialf(GL_FRONT, GL_SHININESS, 0);

    //glBindTexture(GL_TEXTURE_2D, 0);
}

int LightCache::sLightCount = 0;

LightCache::LightCache() : mType(FbxLight::ePoint)
{
    //mLightIndex = GL_LIGHT0 + sLightCount++;
}

LightCache::~LightCache()
{
    //glDisable(mLightIndex);
    --sLightCount;
}

// Bake light properties.
bool LightCache::Initialize(const FbxLight * pLight, FbxAnimLayer * pAnimLayer)
{
    mType = pLight->LightType.Get();

    FbxPropertyT<FbxDouble3> lColorProperty = pLight->Color;
    FbxDouble3 lLightColor = lColorProperty.Get();
    mColorRed.mValue = static_cast<float>(lLightColor[0]);
    mColorGreen.mValue = static_cast<float>(lLightColor[1]);
    mColorBlue.mValue = static_cast<float>(lLightColor[2]);

    mColorRed.mAnimCurve = lColorProperty.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_RED);
    mColorGreen.mAnimCurve = lColorProperty.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_GREEN);
    mColorBlue.mAnimCurve = lColorProperty.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_BLUE);

    if (mType == FbxLight::eSpot)
    {
        FbxPropertyT<FbxDouble> lConeAngleProperty = pLight->InnerAngle;
        mConeAngle.mValue = static_cast<float>(lConeAngleProperty.Get());
        mConeAngle.mAnimCurve = lConeAngleProperty.GetCurve(pAnimLayer);
    }

    return true;
}

void LightCache::SetLight(const FbxTime & pTime) const
{
    const float lLightColor[4] = {mColorRed.Get(pTime), mColorGreen.Get(pTime), mColorBlue.Get(pTime), 1.0f};
    const float lConeAngle = mConeAngle.Get(pTime);

    //glColor3fv(lLightColor);

    //glPushAttrib(GL_ENABLE_BIT);
    //glPushAttrib(GL_POLYGON_BIT);
    //// Visible for double side.
    //glDisable(GL_CULL_FACE);
    //// Draw wire-frame geometry.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //if (mType == FbxLight::eSpot)
    //{
    //    // Draw a cone for spot light.
    //    glPushMatrix();
    //    glScalef(1.0f, 1.0f, -1.0f);
    //    const double lRadians = ANGLE_TO_RADIAN * lConeAngle;
    //    const double lHeight = 15.0;
    //    const double lBase = lHeight * tan(lRadians / 2);
    //    GLUquadricObj * lQuadObj = gluNewQuadric();
    //    gluCylinder(lQuadObj, 0.0, lBase, lHeight, 18, 1);
    //    gluDeleteQuadric(lQuadObj);
    //    glPopMatrix();
    //}
    //else
    //{
    //    // Draw a sphere for other types.
    //    GLUquadricObj * lQuadObj = gluNewQuadric();
    //    gluSphere(lQuadObj, 1.0, 10, 10);
    //    gluDeleteQuadric(lQuadObj);
    //}
    //glPopAttrib();
    //glPopAttrib();

    //// The transform have been set, so set in local coordinate.
    //if (mType == FbxLight::eDirectional)
    //{
    //    glLightfv(mLightIndex, GL_POSITION, DEFAULT_DIRECTION_LIGHT_POSITION);
    //}
    //else
    //{
    //    glLightfv(mLightIndex, GL_POSITION, DEFAULT_LIGHT_POSITION);
    //}

    //glLightfv(mLightIndex, GL_DIFFUSE, lLightColor);
    //glLightfv(mLightIndex, GL_SPECULAR, lLightColor);
    //
    //if (mType == FbxLight::eSpot && lConeAngle != 0.0)
    //{
    //    glLightfv(mLightIndex, GL_SPOT_DIRECTION, DEFAULT_SPOT_LIGHT_DIRECTION);

    //    // If the cone angle is 0, equal to a point light.
    //    if (lConeAngle != 0.0f)
    //    {
    //        // OpenGL use cut off angle, which is half of the cone angle.
    //        glLightf(mLightIndex, GL_SPOT_CUTOFF, lConeAngle/2);
    //    }
    //}
    //glEnable(mLightIndex);
}

void LightCache::IntializeEnvironment(const FbxColor & pAmbientLight)
{
    //glLightfv(GL_LIGHT0, GL_POSITION, DEFAULT_DIRECTION_LIGHT_POSITION);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, DEFAULT_LIGHT_COLOR);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, DEFAULT_LIGHT_COLOR);
    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, DEFAULT_LIGHT_SPOT_CUTOFF);
    //glEnable(GL_LIGHT0);

    //// Set ambient light.
    //float lAmbientLight[] = {static_cast<float>(pAmbientLight[0]), static_cast<float>(pAmbientLight[1]),
    //    static_cast<float>(pAmbientLight[2]), 1.0f};
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbientLight);
}

//////////////////////////////////////////////////////////////////////////
//

//Compute the transform matrix that the cluster will transform the vertex.
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
  FbxNode* pNode,
  FbxCluster* pCluster, 
  FbxAMatrix& pVertexTransformMatrix,
  //FbxTime pTime, 
  FbxPose* pPose)
{
  FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

  FbxAMatrix lReferenceGlobalInitPosition;
  //FbxAMatrix lReferenceGlobalCurrentPosition;
  FbxAMatrix lAssociateGlobalInitPosition;
  FbxAMatrix lAssociateGlobalCurrentPosition;
  FbxAMatrix lClusterGlobalInitPosition;
  //FbxAMatrix lClusterGlobalCurrentPosition;

  FbxAMatrix lReferenceGeometry;
  FbxAMatrix lAssociateGeometry;
  FbxAMatrix lClusterGeometry;

  FbxAMatrix lClusterRelativeInitPosition;
  FbxAMatrix lClusterRelativeCurrentPositionInverse;

  if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
  {
    ASSERT(0); // 暂时不支持
    /*
    pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
    // Geometric transform of the model
    lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
    lAssociateGlobalInitPosition *= lAssociateGeometry;
    lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = GetGeometry(pNode);
    lReferenceGlobalInitPosition *= lReferenceGeometry;
    lReferenceGlobalCurrentPosition = pGlobalPosition;

    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    // Multiply lClusterGlobalInitPosition by Geometric Transformation
    lClusterGeometry = GetGeometry(pCluster->GetLink());
    lClusterGlobalInitPosition *= lClusterGeometry;
    lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

    // Compute the shift of the link relative to the reference.
    //ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
    pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
      lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
    //*/
  }
  else
  {
    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    //lReferenceGlobalCurrentPosition = pGlobalPosition;
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = GetGeometry(pNode);
    lReferenceGlobalInitPosition *= lReferenceGeometry;

    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    //lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), 0, pPose);

    // Compute the initial position of the link relative to the reference.
    lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

    // 原型 v1:
    // Compute the current position of the link relative to the reference.
    //lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

    // Compute the shift of the link relative to the reference.
    //pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;

    // 原型 v2: pVertexTransformMatrix = /*lClusterGlobalCurrentPosition * */(lReferenceGlobalCurrentPosition.Inverse().Transpose() * lClusterRelativeInitPosition);
    //pVertexTransformMatrix = (lReferenceGlobalCurrentPosition.Inverse()/*.Transpose()*/ * lClusterRelativeInitPosition);
    pVertexTransformMatrix = lClusterRelativeInitPosition;
  }
}

template<class _ElementT>
void VBOMesh::ReadElement(_ElementT* pElement, int ctrlPointIndex, int vertecCounter, FbxVector4& vElementVector)
{  
  //FbxGeometryElementTangent* pTangent = pMesh->GetElementTangent(0);

  switch(pElement->GetMappingMode())
  {
  case FbxGeometryElement::eByControlPoint:  
    {
      switch(pElement->GetReferenceMode())  
      {  
      case FbxGeometryElement::eDirect:  
        vElementVector = pElement->GetDirectArray().GetAt(ctrlPointIndex);
        break;  

      case FbxGeometryElement::eIndexToDirect:  
        {  
          int id = pElement->GetIndexArray().GetAt(ctrlPointIndex);  
          vElementVector = pElement->GetDirectArray().GetAt(id);
        }  
        break;  

      default:  
        break;  
      }  
    }  
    break;  

  case FbxGeometryElement::eByPolygonVertex:  
    {  
      switch(pElement->GetReferenceMode())  
      {  
      case FbxGeometryElement::eDirect:  
        vElementVector = pElement->GetDirectArray().GetAt(vertecCounter);
        break;  

      case FbxGeometryElement::eIndexToDirect:  
        {  
          int id = pElement->GetIndexArray().GetAt(vertecCounter);  
          vElementVector = pElement->GetDirectArray().GetAt(id);
        }  
        break;  

      default:  
        break;  
      }  
    }  
    break;  
  }  
}  

int InflateWeightTable(const clBuffer* pWeightBuf, clBuffer* pInflateWeightBuf, int nVertexCount)
{
  return 0;
}