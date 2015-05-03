/****************************************************************************************

Copyright (C) 2012 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

//#include <clstd.h>
//#include <clBuffer.H>

#include <GrapX.H>
#include <clString.H>
#include <clPathFile.H>
//#include <clTree.H>

#include <GUnknown.H>
#include <GResource.H>
#include <GShader.H>
#include <GXGraphics.H>
//#include <3D/gvNode.h>
//#include <3D/gvMesh.h>
//#include <3D/gvScene.h>
//#include <3D/gvSkeleton.h>
#include "3D/GrapVR.H"

#include <GameEngine.h>

#include "third_party\FBX SDK\2013.2\include\fbxsdk.h"
#include "SceneCache.h"
#include "SceneContext.h"

//#include <GrapX.h>

//#include <Include/GUnknown.H>
//#include <Include/GResource.H>
//#include <Include/GShader.h>
//#include "SceneCache.h"
//#include "SetCamera.h"
//#include "DrawScene.h"
//#include "DrawText.h"
//#include "targa.h"
//#include "../Common/Common.h"

//#pragma comment(lib, "fbxsdk-2013.2d.lib")
#ifdef _DEBUG
//#pragma comment(lib, "fbxsdk-2013.2-mdd.lib")
#pragma comment(lib, "fbxsdk-2013.2-mdd.lib")
#else
#pragma comment(lib, "fbxsdk-2013.2-md.lib")
#endif // #ifdef _DEBUG

//namespace
//{
    // Default file of ViewScene example
    const char * SAMPLE_FILENAME = "humanoid.fbx";

    // Button and action definition
    const int LEFT_BUTTON = 0;
    const int MIDDLE_BUTTON = 1;
    const int RIGHT_BUTTON = 2;

    const int BUTTON_DOWN = 0;
    const int BUTTON_UP = 1;

    void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
    {
      //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
      pManager = FbxManager::Create();
      if( !pManager )
      {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
      }
      else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

      //Create an IOSettings object. This object holds all import/export settings.
      FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
      pManager->SetIOSettings(ios);

      //Load plugins from the executable directory (optional)
      FbxString lPath = FbxGetApplicationDirectory();
      pManager->LoadPluginsDirectory(lPath.Buffer());

      //Create an FBX scene. This object holds most objects imported/exported from/to files.
      pScene = FbxScene::Create(pManager, "My Scene");
      if( !pScene )
      {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
      }
    }

    void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
    {
      //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
      if( pManager ) pManager->Destroy();
      if( pExitStatus ) FBXSDK_printf("Program Success!\n");
    }

    // Find all the cameras under this node recursively.
    void FillCameraArrayRecursive(FbxNode* pNode, FbxArray<FbxNode*>& pCameraArray)
    {
        if (pNode)
        {
            if (pNode->GetNodeAttribute())
            {
                if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
                {
                    pCameraArray.Add(pNode);
                }
            }

            const int lCount = pNode->GetChildCount();
            for (int i = 0; i < lCount; i++)
            {
                FillCameraArrayRecursive(pNode->GetChild(i), pCameraArray);
            }
        }
    }

    // Find all the cameras in this scene.
    void FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray)
    {
        pCameraArray.Clear();

        FillCameraArrayRecursive(pScene->GetRootNode(), pCameraArray);
    }

    // Find all poses in this scene.
    void FillPoseArray(FbxScene* pScene, FbxArray<FbxPose*>& pPoseArray)
    {
        const int lPoseCount = pScene->GetPoseCount();

        for (int i=0; i < lPoseCount; ++i)
        {
            pPoseArray.Add(pScene->GetPose(i));
        }
    }

    void PreparePointCacheData(FbxScene* pScene, FbxTime &pCache_Start, FbxTime &pCache_Stop)
    {
        // This function show how to cycle through scene elements in a linear way.
		const int lNodeCount = pScene->GetSrcObjectCount<FbxNode>();

        for (int lIndex=0; lIndex<lNodeCount; lIndex++)
        {
            FbxNode* lNode = pScene->GetSrcObject<FbxNode>(lIndex);

            if (lNode->GetGeometry()) 
            {
                int i, lVertexCacheDeformerCount = lNode->GetGeometry()->GetDeformerCount(FbxDeformer::eVertexCache);

                // There should be a maximum of 1 Vertex Cache Deformer for the moment
                lVertexCacheDeformerCount = lVertexCacheDeformerCount > 0 ? 1 : 0;

                for (i=0; i<lVertexCacheDeformerCount; ++i )
                {
                    // Get the Point Cache object
                    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(lNode->GetGeometry()->GetDeformer(i, FbxDeformer::eVertexCache));
                    if( !lDeformer ) continue;
                    FbxCache* lCache = lDeformer->GetCache();
                    if( !lCache ) continue;

                    // Process the point cache data only if the constraint is active
                    if (lDeformer->IsActive())
                    {
                        if (lCache->GetCacheFileFormat() == FbxCache::eMaxPointCacheV2)
                        {
                            // This code show how to convert from PC2 to MC point cache format
                            // turn it on if you need it.
#if 0 
                            if (!lCache->ConvertFromPC2ToMC(FbxCache::eMCOneFile, 
                                FbxTime::GetFrameRate(pScene->GetGlobalTimeSettings().GetTimeMode())))
                            {
                                // Conversion failed, retrieve the error here
                                FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();
                            }
#endif
                        }
                        else if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
                        {
                            // This code show how to convert from MC to PC2 point cache format
                            // turn it on if you need it.
                            //#if 0 
                            if (!lCache->ConvertFromMCToPC2(FbxTime::GetFrameRate(pScene->GetGlobalSettings().GetTimeMode()), 0))
                            {
                                // Conversion failed, retrieve the error here
                                FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();
                            }
                            //#endif
                        }


                        // Now open the cache file to read from it
                        if (!lCache->OpenFileForRead())
                        {
                            // Cannot open file 
                            FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();

                            // Set the deformer inactive so we don't play it back
                            lDeformer->SetActive(false);
                        }
						else
						{
							// get the start and stop time of the cache
							int lChannelCount = lCache->GetChannelCount();
							
							for (int iChannelNo=0; iChannelNo < lChannelCount; iChannelNo++)
							{
								FbxTime lChannel_Start;
								FbxTime lChannel_Stop;

								if(lCache->GetAnimationRange(iChannelNo, lChannel_Start, lChannel_Stop))
								{
									// get the smallest start time
									if(lChannel_Start < pCache_Start) pCache_Start = lChannel_Start;

									// get the biggest stop time
									if(lChannel_Stop  > pCache_Stop)  pCache_Stop  = lChannel_Stop;
								}
							}
						}
                    }
                }
            }
        }
    }

    // Load a texture file (TGA only now) into GPU and return the texture object name
    //bool LoadTextureFromFile(const FbxString & pFilePath, unsigned int & pTextureObject)
    //{
    //    //if (pFilePath.Right(3).Upper() == "TGA")
    //    //{
    //    //    tga_image lTGAImage;

    //    //    if (tga_read(&lTGAImage, pFilePath.Buffer()) == TGA_NOERR)
    //    //    {
    //    //        // Make sure the image is left to right
    //    //        if (tga_is_right_to_left(&lTGAImage))
    //    //            tga_flip_horiz(&lTGAImage);

    //    //        // Make sure the image is bottom to top
    //    //        if (tga_is_top_to_bottom(&lTGAImage))
    //    //            tga_flip_vert(&lTGAImage);

    //    //        // Make the image BGR 24
    //    //        tga_convert_depth(&lTGAImage, 24);

    //    //        // Transfer the texture date into GPU
    //    //        glGenTextures(1, &pTextureObject);
    //    //        glBindTexture(GL_TEXTURE_2D, pTextureObject);
    //    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    //    //        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //    //        glTexImage2D(GL_TEXTURE_2D, 0, 3, lTGAImage.width, lTGAImage.height, 0, GL_BGR,
    //    //            GL_UNSIGNED_BYTE, lTGAImage.image_data);
    //    //        glBindTexture(GL_TEXTURE_2D, 0);

    //    //        tga_free_buffers(&lTGAImage);

    //    //        return true;
    //    //    }
    //    //}

    //    return false;
    //}

    // Triangulate all NURBS, patch and mesh under this node recursively.
    void TriangulateRecursive(FbxNode* pNode)
    {
      FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

      if (lNodeAttribute)
      {
        FbxNodeAttribute::EType eAttrType = lNodeAttribute->GetAttributeType();
        if (eAttrType == FbxNodeAttribute::eMesh ||
          eAttrType == FbxNodeAttribute::eNurbs ||
          eAttrType == FbxNodeAttribute::eNurbsSurface ||
          eAttrType == FbxNodeAttribute::ePatch)
        {              
          FbxGeometryConverter lConverter(pNode->GetFbxManager());
          lConverter.TriangulateInPlace(pNode);
        }
      }

      const int lChildCount = pNode->GetChildCount();
      for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
      {
        TriangulateRecursive(pNode->GetChild(lChildIndex));
      }
    }

    // Bake node attributes and materials under this node recursively.
    // Currently only mesh, light and material.
    void LoadCacheRecursive(SceneContext* pSceneCtx, FbxNode * pNode, FbxAnimLayer * pAnimLayer, bool pSupportVBO)
    {
        // Bake material and hook as user data.
        const int lMaterialCount = pNode->GetMaterialCount();
        for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
        {
            FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
            if (lMaterial && !lMaterial->GetUserDataPtr())
            {
                FbxAutoPtr<MaterialCache> lMaterialCache(new MaterialCache);
                if (lMaterialCache->Initialize(lMaterial))
                {
                    lMaterial->SetUserDataPtr(lMaterialCache.Release());
                }
            }
        }

        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
        if (lNodeAttribute)
        {
            // Bake mesh as VBO(vertex buffer object) into GPU.
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxMesh * lMesh = pNode->GetMesh();
                if (pSupportVBO && lMesh && !lMesh->GetUserDataPtr())
                {
                    FbxAutoPtr<VBOMesh> lMeshCache(new VBOMesh);
                    if (lMeshCache->Initialize(pSceneCtx, lMesh))
                    {
                        lMesh->SetUserDataPtr(lMeshCache.Release());
                    }
                }
            }
            // Bake light properties.
            else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
            {
                FbxLight * lLight = pNode->GetLight();
                if (lLight && !lLight->GetUserDataPtr())
                {
                    FbxAutoPtr<LightCache> lLightCache(new LightCache);
                    if (lLightCache->Initialize(lLight, pAnimLayer))
                    {
                        lLight->SetUserDataPtr(lLightCache.Release());
                    }
                }
            }
        }

        const int lChildCount = pNode->GetChildCount();
        for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
        {
            LoadCacheRecursive(pSceneCtx, pNode->GetChild(lChildIndex), pAnimLayer, pSupportVBO);
        }
    }

    // Unload the cache and release the memory under this node recursively.
    void UnloadCacheRecursive(FbxNode * pNode)
    {
        // Unload the material cache
        const int lMaterialCount = pNode->GetMaterialCount();
        for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
        {
            FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
            if (lMaterial && lMaterial->GetUserDataPtr())
            {
                MaterialCache * lMaterialCache = static_cast<MaterialCache *>(lMaterial->GetUserDataPtr());
                lMaterial->SetUserDataPtr(NULL);
                delete lMaterialCache;
            }
        }

        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
        if (lNodeAttribute)
        {
            // Unload the mesh cache
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxMesh * lMesh = pNode->GetMesh();
                if (lMesh && lMesh->GetUserDataPtr())
                {
                    VBOMesh * lMeshCache = static_cast<VBOMesh *>(lMesh->GetUserDataPtr());
                    lMesh->SetUserDataPtr(NULL);
                    delete lMeshCache;
                }
            }
            // Unload the light cache
            else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
            {
                FbxLight * lLight = pNode->GetLight();
                if (lLight && lLight->GetUserDataPtr())
                {
                    LightCache * lLightCache = static_cast<LightCache *>(lLight->GetUserDataPtr());
                    lLight->SetUserDataPtr(NULL);
                    delete lLightCache;
                }
            }
        }

        const int lChildCount = pNode->GetChildCount();
        for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
        {
            UnloadCacheRecursive(pNode->GetChild(lChildIndex));
        }
    }

    // Bake node attributes and materials for this scene and load the textures.
    void LoadCacheRecursive(SceneContext* pSceneCtx, FbxScene * pScene, FbxAnimLayer * pAnimLayer, GXLPCWSTR pFbxFileName, bool pSupportVBO)
    {
        // Load the textures into GPU, only for file texture now
        const int lTextureCount = pScene->GetTextureCount();
        for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
        {
            FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
            FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
            if (lFileTexture && !lFileTexture->GetUserDataPtr())
            {
                // Try to load the texture from absolute path
                const FbxString lFileName = lFileTexture->GetFileName();
                
                // Only TGA textures are supported now.
                //if (lFileName.Right(3).Upper() != "TGA")
                //{
                //    FBXSDK_printf("Only TGA textures are supported now: %s\n", lFileName.Buffer());
                //    continue;
                //}

                //unsigned int lTextureObject = 0;
                //bool lStatus = LoadTextureFromFile(lFileName, lTextureObject);

                //const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName);
                //const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);
                //if (!lStatus)
                //{
                //    // Load texture from relative file name (relative to FBX file)
                //    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
                //    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
                //}

                //if (!lStatus)
                //{
                //    // Load texture from file name only (relative to FBX file)
                //    const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
                //    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
                //    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
                //}

                //if (!lStatus)
                //{
                //    FBXSDK_printf("Failed to load texture file: %s\n", lFileName.Buffer());
                //    continue;
                //}

                //if (lStatus)
                //{
                //    int * lTextureName = new int(lTextureObject);
                //    lFileTexture->SetUserDataPtr(lTextureName);
                //}
            }
        }

        LoadCacheRecursive(pSceneCtx, pScene->GetRootNode(), pAnimLayer, pSupportVBO);
    }

    // Unload the cache and release the memory fro this scene and release the textures in GPU
    void UnloadCacheRecursive(FbxScene * pScene)
    {
        const int lTextureCount = pScene->GetTextureCount();
        for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
        {
            FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
            FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
            if (lFileTexture && lFileTexture->GetUserDataPtr())
            {
                int * lTextureName = static_cast<int *>(lFileTexture->GetUserDataPtr());
                lFileTexture->SetUserDataPtr(NULL);
                //glDeleteTextures(1, lTextureName);
                delete lTextureName;
            }
        }

        UnloadCacheRecursive(pScene->GetRootNode());
    }
//}

//bool InitializeOpenGL()
//{
//    // Initialize GLEW.
//    GLenum lError = glewInit();
//    if (lError != GLEW_OK)
//    {
//        FBXSDK_printf("GLEW Error: %s\n", glewGetErrorString(lError));
//        return false;
//    }
//
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glClearColor(0.0, 0.0, 0.0, 0.0);
//
//    // OpenGL 1.5 at least.
//    if (!GLEW_VERSION_1_5)
//    {
//        FBXSDK_printf("The OpenGL version should be at least 1.5 to display shaded scene!\n");
//        return false;
//    }
//
//    return true;
//}

    SceneContext::SceneContext(GVScene* pMOScene, GXLPCWSTR pFileName, bool pSupportVBO, GXBOOL bSaveToMyFmt)
      : m_pMOScene(pMOScene)
      , m_FileName(pFileName)
      , mStatus(UNLOADED)
      , mSdkManager(NULL)
      , mScene(NULL)
      , mImporter(NULL)
      , mCurrentAnimLayer(NULL)
      , mSelectedNode(NULL)
      , mPoseIndex(-1)
      , m_bSupportVBO(pSupportVBO)
      , m_bSaveToMyFmt(bSaveToMyFmt)
      , m_bAddToScene(TRUE)
      , m_bUseTransformedVertices(FALSE)
      , m_pMeshMtl(NULL)
    {
      Init(pMOScene, pFileName);
    }

    SceneContext::SceneContext(GVScene* pMOScene, GXLPCWSTR pFileName, GXDWORD dwFlags)
      : m_pMOScene(pMOScene)
      , m_FileName(pFileName)
      , mStatus(UNLOADED)
      , mSdkManager(NULL)
      , mScene(NULL)
      , mImporter(NULL)
      , mCurrentAnimLayer(NULL)
      , mSelectedNode(NULL)
      , mPoseIndex(-1)
      , m_bSupportVBO(TEST_FLAG(dwFlags, SupportVBO))
      , m_bSaveToMyFmt(TEST_FLAG(dwFlags, SaveToSDB))
      , m_bAddToScene(TEST_FLAG(dwFlags, AddToScene))
      , m_bUseTransformedVertices(TEST_FLAG(dwFlags, ApplyTransformation))
      , m_pMeshMtl(NULL)
    {
      Init(pMOScene, pFileName);
    }

SceneContext::~SceneContext()
{
  SAFE_RELEASE(m_pMeshMtl);
  FbxArrayDelete(mAnimStackNameArray);

  //delete mDrawText;

  // Unload the cache and free the memory
  if (mScene)
  {
    UnloadCacheRecursive(mScene);
  }

  // Delete the FBX SDK manager. All the objects that have been allocated 
  // using the FBX SDK manager and that haven't been explicitly destroyed 
  // are automatically destroyed at the same time.
  DestroySdkObjects(mSdkManager, true);

  ReleaseAllNodes();
}

bool SceneContext::LoadFile()
{
    bool lResult = false;
    // Make sure that the scene is ready to load.
    if (mStatus == MUST_BE_LOADED)
    {
        if (mImporter->Import(mScene) == true)
        {
            // Set the scene status flag to refresh 
            // the scene in the first timer callback.
            mStatus = MUST_BE_REFRESHED;

            // Convert Axis System to what is used in this example, if needed
            FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
            //FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
            FbxAxisSystem OurAxisSystem(FbxAxisSystem::eOpenGL);
            //m_matGlobal.SetRow(1, float4(0,0,-1,0));
            //m_matGlobal.SetRow(2, float4(0,1,0,0));
            if( SceneAxisSystem != OurAxisSystem )
            {
                OurAxisSystem.ConvertScene(mScene);
            }

            // Convert Unit System to what is used in this example, if needed
            FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
            if( SceneSystemUnit.GetScaleFactor() != 1.0 )
            {
                //The unit in this example is centimeter.
                FbxSystemUnit::cm.ConvertScene( mScene);
            }

            // Get the list of all the animation stack.
            mScene->FillAnimStackNameArray(mAnimStackNameArray);

            SetCurrentAnimStack(0);

            // Get the list of all the cameras in the scene.
            FillCameraArray(mScene, mCameraArray);

            // Convert mesh, NURBS and patch into triangle mesh
            TriangulateRecursive(mScene->GetRootNode());

            // Bake the scene for one frame
            //LoadCacheRecursive(this, mScene, mCurrentAnimLayer, mFileName, mSupportVBO);

            // Convert any .PC2 point cache data into the .MC format for 
            // vertex cache deformer playback.
            PreparePointCacheData(mScene, mCache_Start, mCache_Stop);

            // Get the list of pose in the scene
            FillPoseArray(mScene, mPoseArray);

            // Initialize the window message.
            mWindowMessage = "File ";
            mWindowMessage += WS2AS(m_FileName);
            mWindowMessage += "\nClick on the right mouse button to enter menu.";
            mWindowMessage += "\nEsc to exit.";

            // Initialize the frame period.
            mFrameTime.SetTime(0, 0, 0, 1, 0, mScene->GetGlobalSettings().GetTimeMode());

            // Print the keyboard shortcuts.
            //FBXSDK_printf("Play/Pause Animation: Space Bar.\n");
            //FBXSDK_printf("Camera Rotate: Left Mouse Button.\n");
            //FBXSDK_printf("Camera Pan: Left Mouse Button + Middle Mouse Button.\n");
            //FBXSDK_printf("Camera Zoom: Middle Mouse Button.\n");

            lResult = true;
        }
        else
        {
            // Import failed, set the scene status flag accordingly.
            mStatus = UNLOADED;

            mWindowMessage = "Unable to import file ";
            mWindowMessage += WS2AS(m_FileName);
            mWindowMessage += "\nError reported: ";
            mWindowMessage += mImporter->GetLastErrorString();
        }

        // Destroy the importer to release the file.
        mImporter->Destroy();
        mImporter = NULL;
    }

    // Bake the scene for one frame
    LoadCacheRecursive(this, mScene, mCurrentAnimLayer, m_FileName, m_bSupportVBO);
    return lResult;
}

bool SceneContext::SetCurrentAnimStack(int nIndex)
{
    const int nAnimStackCount = mAnimStackNameArray.GetCount();
    if (!nAnimStackCount || nIndex >= nAnimStackCount)
    {
        return false;
    }

    // select the base layer from the animation stack
   FbxAnimStack * lCurrentAnimationStack = mScene->FindMember<FbxAnimStack>(mAnimStackNameArray[nIndex]->Buffer());
   if (lCurrentAnimationStack == NULL)
   {
       // this is a problem. The anim stack should be found in the scene!
       return false;
   }

   // we assume that the first animation layer connected to the animation stack is the base layer
   // (this is the assumption made in the FBXSDK)
   mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
   mScene->GetEvaluator()->SetContext(lCurrentAnimationStack);

   FbxTakeInfo* lCurrentTakeInfo = mScene->GetTakeInfo(*(mAnimStackNameArray[nIndex]));
   if (lCurrentTakeInfo)
   {
       mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
       mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
   }
   else
   {
       // Take the time line value
       FbxTimeSpan lTimeLineTimeSpan;
       mScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);

       mStart = lTimeLineTimeSpan.GetStart();
       mStop  = lTimeLineTimeSpan.GetStop();
   }

   // check for smallest start with cache start
   if(mCache_Start < mStart)
	   mStart = mCache_Start;

   // check for biggest stop with cache stop
   if(mCache_Stop  > mStop)  
	   mStop  = mCache_Stop;

   // move to beginning
   mCurrentTime = mStart;

   // Set the scene status flag to refresh 
   // the scene in the next timer callback.
   mStatus = MUST_BE_REFRESHED;

   return true;
}

bool SceneContext::SetCurrentCamera(const char * pCameraName)
{
    if (!pCameraName)
    {
        return false;
    }

    FbxGlobalSettings& lGlobalCameraSettings = mScene->GetGlobalSettings();
    lGlobalCameraSettings.SetDefaultCamera(pCameraName);
    mStatus = MUST_BE_REFRESHED;
    return true;
}

bool SceneContext::SetCurrentPoseIndex(int pPoseIndex)
{
    mPoseIndex = pPoseIndex;
    mStatus = MUST_BE_REFRESHED;
    return true;
}

//void SceneContext::OnTimerClick() const
//{
//    // Loop in the animation stack if not paused.
//    if (mStop > mStart && !mPause)
//    {
//        // Set the scene status flag to refresh 
//        // the scene in the next timer callback.
//        mStatus = MUST_BE_REFRESHED;
//
//        mCurrentTime += mFrameTime;
//
//        if (mCurrentTime > mStop)
//        {
//            mCurrentTime = mStart;
//        }
//    }
//    // Avoid displaying the same frame on 
//    // and on if the animation stack has no length.
//    else
//    {
//        // Set the scene status flag to avoid refreshing 
//        // the scene in the next timer callback.
//        mStatus = REFRESHED;
//    }
//}

// Redraw the scene
//bool SceneContext::OnDisplay()
//{
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    // Test if the scene has been loaded yet.
//    if (mStatus != UNLOADED && mStatus != MUST_BE_LOADED)
//    {
//        glPushAttrib(GL_ENABLE_BIT);
//        glPushAttrib(GL_LIGHTING_BIT);
//        glEnable(GL_DEPTH_TEST);
//        // Draw the front face only, except for the texts and lights.
//        glEnable(GL_CULL_FACE);
//
//        // Set the view to the current camera settings.
//        SetCamera(mScene, mCurrentTime, mCurrentAnimLayer, mCameraArray,
//            mWindowWidth, mWindowHeight);
//
//        FbxPose * lPose = NULL;
//        if (mPoseIndex != -1)
//        {
//            lPose = mScene->GetPose(mPoseIndex);
//        }
//
//        // If one node is selected, draw it and its children.
//        FbxAMatrix lDummyGlobalPosition;
//        
//        if (mSelectedNode)
//        {
//            // Set the lighting before other things.
//            InitializeLights(mScene, mCurrentTime, lPose);
//            DrawNodeRecursive(mSelectedNode, mCurrentTime, mCurrentAnimLayer, lDummyGlobalPosition, lPose, mShadingMode);
//            DisplayGrid(lDummyGlobalPosition);
//        }
//        // Otherwise, draw the whole scene.
//        else
//        {
//            InitializeLights(mScene, mCurrentTime, lPose);
//            DrawNodeRecursive(mScene->GetRootNode(), mCurrentTime, mCurrentAnimLayer, lDummyGlobalPosition, lPose, mShadingMode);
//            DisplayGrid(lDummyGlobalPosition);
//        }
//
//        glPopAttrib();
//        glPopAttrib();
//    }
//
//    DisplayWindowMessage();
//
//    return true;
//}

//void SceneContext::OnReshape(int pWidth, int pHeight)
//{
//    glViewport(0, 0, (GLsizei)pWidth, (GLsizei)pHeight);
//    mWindowWidth = pWidth;
//    mWindowHeight = pHeight;
//}

//void SceneContext::OnKeyboard(unsigned char pKey)
//{
//    // Zoom In on '+' or '=' keypad keys
//    if (pKey == 43 || pKey == 61)
//    {
//        FbxCamera* lCamera = GetCurrentCamera(mScene);
//        if(lCamera)
//        {
//            //double lOriginalAperture = sqrt(lCamera->GetApertureWidth());
//            CameraZoom(mScene, 10, mCameraZoomMode);
//            mStatus = MUST_BE_REFRESHED;
//        }
//    }
//
//    // Zoom Out on '-' or '_' keypad keys
//    if (pKey == 45 || pKey == 95)
//    {
//        FbxCamera* lCamera = GetCurrentCamera(mScene);
//        if(lCamera)
//        {
//            //double lOriginalAperture = sqrt(lCamera->GetApertureWidth()); 
//            CameraZoom(mScene, 0 - 10, mCameraZoomMode);
//            mStatus = MUST_BE_REFRESHED;
//        }
//    }
//
//    // 'N' enable/disable normal display
//    if (pKey == 'N' || pKey == 'n')
//    {
//        //gOGLScene->GetShadingManager()->SetDrawNormal(!gOGLScene->GetShadingManager()->IsDrawNormal());
//    }
//
//    // Pause and unpause when spacebar is pressed.
//    if (pKey == ' ')
//    {
//        SetPause(!GetPause());
//    }
//}

//void SceneContext::OnMouse(int pButton, int pState, int pX, int pY)
//{
//    // Move the camera (orbit, zoom or pan) with the mouse.
//    FbxCamera* lCamera = GetCurrentCamera(mScene);
//    if (lCamera)
//    {
//        mCamPosition = lCamera->Position.Get();
//        mCamCenter = lCamera->InterestPosition.Get();
//        mRoll = lCamera->Roll.Get();
//    }
//    mLastX = pX;
//    mLastY = pY;
//
//    switch (pButton)
//    {
//    case LEFT_BUTTON:
//        // ORBIT (or PAN)
//        switch (pState)
//        {
//        case BUTTON_DOWN:
//            if (mCameraStatus == CAMERA_ZOOM)
//            {
//                mCameraStatus = CAMERA_PAN;
//            }
//            else
//            {
//                mCameraStatus = CAMERA_ORBIT;
//            }
//            break;
//
//        default:
//            if (mCameraStatus == CAMERA_PAN)
//            {
//                mCameraStatus = CAMERA_ZOOM;
//            }
//            else
//            {
//                mCameraStatus = CAMERA_NOTHING;
//            }
//            break;
//        }
//        break;
//
//    case MIDDLE_BUTTON:
//        // ZOOM (or PAN)
//        switch (pState)
//        {
//        case BUTTON_DOWN:
//            if (mCameraStatus == CAMERA_ORBIT)
//            {
//                mCameraStatus = CAMERA_PAN;
//            }
//            else
//            {
//                mCameraStatus = CAMERA_ZOOM;
//            }
//            break;
//        
//        default:
//            if (mCameraStatus == CAMERA_PAN)
//            {
//                mCameraStatus = CAMERA_ORBIT;
//            }
//            else
//            {
//                mCameraStatus = CAMERA_NOTHING;
//            }
//            break;
//        }
//        break;
//    }
//}
//
//void SceneContext::OnMouseMotion(int pX, int pY)
//{
//    int motion;
//
//    switch (mCameraStatus)
//    {
//    case CAMERA_ORBIT:
//        CameraOrbit(mScene, mCamPosition, mRoll, pX-mLastX, mLastY-pY);
//        mStatus = MUST_BE_REFRESHED;
//        break;
//
//    case CAMERA_ZOOM:
//        motion = mLastY-pY;
//        CameraZoom(mScene, motion, mCameraZoomMode);
//        mLastY = pY;
//        mStatus = MUST_BE_REFRESHED;
//        break;
//
//    case CAMERA_PAN:
//        CameraPan(mScene, mCamPosition, mCamCenter, mRoll, pX-mLastX, mLastY-pY);
//        mStatus = MUST_BE_REFRESHED;
//        break;
//    }
//}

void SceneContext::SetSelectedNode(FbxNode * pSelectedNode)
{
    mSelectedNode = pSelectedNode;
    mStatus = MUST_BE_REFRESHED;
}

//void SceneContext::SetShadingMode(ShadingMode pMode)
//{
//    mShadingMode = pMode;
//    mStatus = MUST_BE_REFRESHED;
//}

//void SceneContext::DisplayWindowMessage()
//{
//    glColor3f(1.0, 1.0, 1.0);
//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//    gluOrtho2D(0, mWindowWidth, 0, mWindowHeight);
//    glMatrixMode(GL_MODELVIEW);
//    glPushMatrix();
//    glLoadIdentity();
//
//    // Display message in the left up corner of the window
//    const float lX = 5;
//    const float lY = static_cast<float>(mWindowHeight) - 20;
//    glTranslatef(lX, lY, 0);
//
//    mDrawText->SetPointSize(15.f);
//    mDrawText->Display(mWindowMessage.Buffer());
//
//    glPopMatrix();
//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
//}
//
//void SceneContext::DisplayGrid(const FbxAMatrix & pTransform)
//{
//    glPushMatrix();
//    glMultMatrixd(pTransform);
//
//    // Draw a grid 500*500
//    glColor3f(0.3f, 0.3f, 0.3f);
//    glLineWidth(1.0);
//    const int hw = 500;
//    const int step = 20;
//    const int bigstep = 100;
//    int       i;
//
//    // Draw Grid
//    for (i = -hw; i <= hw; i+=step) {
//
//        if (i % bigstep == 0) {
//            glLineWidth(2.0);
//        } else {
//            glLineWidth(1.0);
//        }
//        glBegin(GL_LINES);
//        glVertex3i(i,0,-hw);
//        glVertex3i(i,0,hw);
//        glEnd();
//        glBegin(GL_LINES);
//        glVertex3i(-hw,0,i);
//        glVertex3i(hw,0,i);
//        glEnd();
//
//    }
//
//    // Write some grid info
//    const GLfloat zoffset = -2.f;
//    const GLfloat xoffset = 1.f;
//    mDrawText->SetPointSize(4.f);
//    for (i = -hw; i <= hw; i+=bigstep)
//    {
//
//        FbxString scoord;
//        int lCount;
//
//        // Don't display origin
//        //if (i == 0) continue;
//        if (i == 0) {
//            scoord = "0";
//            lCount = (int)scoord.GetLen();
//            glPushMatrix();
//            glVertex3f(i+xoffset,0,zoffset);
//            glRotatef(-90,1,0,0);
//            
//            mDrawText->Display(scoord.Buffer());
//
//            glPopMatrix();
//
//            continue;
//        }
//
//        // X coordinates
//        scoord = "X: ";
//        scoord += i;
//        lCount = (int)scoord.GetLen();
//
//        glPushMatrix();
//        glTranslatef(i+xoffset,0,zoffset);
//        glRotatef(-90,1,0,0);
//        mDrawText->Display(scoord.Buffer());
//        glPopMatrix();
//
//        // Z coordinates
//        scoord = "Z: ";
//        scoord += i;
//        lCount = (int)scoord.GetLen();
//
//        glPushMatrix();
//        glTranslatef(xoffset,0,i+zoffset);
//        glRotatef(-90,1,0,0);
//        mDrawText->Display(scoord.Buffer());
//        glPopMatrix();
//
//    }
//
//    glPopMatrix();
//}

GXBOOL SceneContext::LoadTextureFromFile(GXLPCWSTR szFilename, GTexture** ppTexture)
{
  if(szFilename == NULL || clstd::strlenT(szFilename) == 0)
    return FALSE;

  GXGraphics* pGraphics = m_pMOScene->GetGraphicsUnsafe();
  if(SUCCEEDED(pGraphics->CreateTextureFromFileW(ppTexture, szFilename))) {
    return TRUE;
  }

  int nPath = clpathfile::FindFileNameW(m_FileName);
  int nFile = clpathfile::FindFileNameW(szFilename);

  clStringW strFilename(m_FileName, nPath);
  strFilename += &szFilename[nFile];

  if(FAILED(pGraphics->CreateTextureFromFileW(ppTexture, strFilename)))
  {
    CLOG_ERROR(__FUNCTION__": Can't load texture from:\"%s\" or \"%s\".\n", szFilename, strFilename);
    return FALSE;
  }
  return TRUE;
}


//void SceneContext::SetZoomMode( CameraZoomMode pZoomMode)
//{
//  if( pZoomMode == ZOOM_POSITION)
//  {
//    mCameraZoomMode = ZOOM_POSITION;
//  }
//  else
//  {
//    mCameraZoomMode =  ZOOM_FOCAL_LENGTH;
//  }    
//}

void SceneContext::SetDefaultMtlInst(GXMaterialInst* pMeshMtl)
{
  m_pMeshMtl = pMeshMtl;
  if(m_pMeshMtl) {
    m_pMeshMtl->AddRef();
  }
}

void SceneContext::ReleaseAllNodes()
{
  for(NodeArray::iterator it = m_aNodes.begin();
    it != m_aNodes.end(); ++it) {
      SAFE_RELEASE(*it);
  }
}

int SceneContext::GetNumberOfNodes()
{
  return (int)m_aNodes.size();
}

GXHRESULT SceneContext::GetNode( GVNode** ppNode, int nIndex )
{
  if(nIndex < 0 || nIndex >= (int)m_aNodes.size()) {
    return GX_FAIL;
  }
  *ppNode = m_aNodes[nIndex];
  return (*ppNode)->AddRef();
}

GXHRESULT SceneContext::AddNode( GVNode* pNode )
{
  m_aNodes.push_back(pNode);
  return pNode->AddRef();
}

void SceneContext::Init( GVScene* pMOScene, GXLPCWSTR pFileName)
{
  m_matGlobal.identity();
  if (m_FileName == NULL) {
    return;
  }
  char szFilenameUtf8[MAX_PATH / 2 * 3];  // Utf8 ���� Unicode16 ת���Ľ�������� 1~3 �ֽ�֮��
  memset(szFilenameUtf8, 0, MAX_PATH / 2 * 3);
  WideCharToMultiByte(CP_UTF8, 0, pFileName, GXSTRLEN(pFileName), szFilenameUtf8, MAX_PATH / 2 * 3, NULL, NULL);

  // initialize cache start and stop time
  mCache_Start = FBXSDK_TIME_INFINITE;
  mCache_Stop  = FBXSDK_TIME_MINUS_INFINITE;

  // Create the FBX SDK manager which is the object allocator for almost 
  // all the classes in the SDK and create the scene.
  InitializeSdkObjects(mSdkManager, mScene);

  if (mSdkManager)
  {
    // Create the importer.
    int lFileFormat = -1;
    mImporter = FbxImporter::Create(mSdkManager,"");
    if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(WS2AS(m_FileName), lFileFormat) )
    {
      // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
      lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
    }

    // Initialize the importer by providing a filename.
    if(mImporter->Initialize(szFilenameUtf8, lFileFormat) == true)
    {
      // The file is going to be imported at 
      // the end of the first display callback.
      mWindowMessage = "Importing file ";
      mWindowMessage += WS2AS(m_FileName);
      mWindowMessage += "\nPlease wait!";

      // Set scene status flag to ready to load.
      mStatus = MUST_BE_LOADED;
    }
    else
    {
      mWindowMessage = "Unable to open file ";
      mWindowMessage += WS2AS(m_FileName);
      mWindowMessage += "\nError reported: ";
      mWindowMessage += mImporter->GetLastErrorString();
      mWindowMessage += "\nEsc to exit";
    }
  }
  else
  {
    mWindowMessage = "Unable to create the FBX SDK manager";
    mWindowMessage += "\nEsc to exit";
  }
  TRACE("%s\n", (const char*)mWindowMessage);
}
