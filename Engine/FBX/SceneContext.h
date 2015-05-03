/****************************************************************************************

Copyright (C) 2012 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SCENE_CONTEXT_H
#define _SCENE_CONTEXT_H

class GVScene;
class GXMaterialInst;
// This class is responsive for loading files and recording current status as
// a bridge between window system such as GLUT or Qt and a specific FBX scene.
class GAMEENGINE_API SceneContext
{
public:
  enum {
    SupportVBO = 0x00000001,
    SaveToSDB  = 0x00000002,
    AddToScene = 0x00000004,
    ApplyTransformation = 0x00000008,
  };
public:
    typedef clvector<GVNode*> NodeArray;
    enum Status
    {
        UNLOADED,               // Unload file or load failure;
        MUST_BE_LOADED,         // Ready for loading file;
        MUST_BE_REFRESHED,      // Something changed and redraw needed;
        REFRESHED               // No redraw needed.
    };
    Status GetStatus() const { return mStatus; }

    // Initialize with a .FBX, .DAE or .OBJ file name and current window size.
    SceneContext(GVScene* pMOScene, GXLPCWSTR pFileName, bool pSupportVBO, GXBOOL bSaveToMyFmt);
    SceneContext(GVScene* pMOScene, GXLPCWSTR pFileName, GXDWORD dwFlags);
    ~SceneContext();

    // Return the FBX scene for more informations.
    const FbxScene * GetFbxScene() const { return mScene; }
    // Load the FBX or COLLADA file into memory.
    bool LoadFile();

    // The time period for one frame.
    const FbxTime GetFrameTime() const { return mFrameTime; }

    GXBOOL LoadTextureFromFile(GXLPCWSTR szFilename, GTexture** ppTexture);

    // Methods for creating menus.
    // Get all the cameras in current scene, including producer cameras.
    const FbxArray<FbxNode *> & GetCameraArray() const { return mCameraArray; }
    // Get all the animation stack names in current scene.
    const FbxArray<FbxString *> & GetAnimStackNameArray() const { return mAnimStackNameArray; }
    // Get all the pose in current scene.
    const FbxArray<FbxPose *> & GetPoseArray() const { return mPoseArray; }

    // The input index is corresponding to the array returned from GetAnimStackNameArray.
    bool SetCurrentAnimStack(int nIndex);
    // Set the current camera with its name.
    bool SetCurrentCamera(const char * pCameraName);
    // The input index is corresponding to the array returned from GetPoseArray.
    bool SetCurrentPoseIndex(int pPoseIndex);
    // Set the currently selected node from external window system.
    void SetSelectedNode(FbxNode * pSelectedNode);
    // Set the shading mode, wire-frame or shaded.

    void SetDefaultMtlInst(GXMaterialInst* pMeshMtl);
    
    GXHRESULT AddNode(GVNode* pNode);
    void ReleaseAllNodes();
    int GetNumberOfNodes();
    GXHRESULT GetNode(GVNode** ppNode, int nIndex);

    inline GVScene* GetMarimoScene()
    {
      return m_pMOScene;
    }
    inline GXMaterialInst* GetDefaultMtlInst()
    {
      return m_pMeshMtl;
    }
    inline FbxTime GetStartTime()
    {
      return mStart;
    }

    inline FbxTime GetStopTime()
    {
      return mStop;
    }

    inline float4x4& GetGlobalMatrix()
    {
      return m_matGlobal;
    }

    inline GXBOOL IsSaveToMyFormat()
    {
      return m_bSaveToMyFmt;
    }

    inline GXLPCWSTR GetFilename()
    {
      return m_FileName;
    }

    inline GXBOOL IsAddToScene() const
    {
      return m_bAddToScene;
    }

    inline GXBOOL UseTransformedVertices() const
    {
      return m_bUseTransformedVertices;
    }



private:
    //enum CameraStatus
    //{
    //    CAMERA_NOTHING,
    //    CAMERA_ORBIT,
    //    CAMERA_ZOOM,
    //    CAMERA_PAN
    //};
    GVScene*            m_pMOScene;
    GXMaterialInst*     m_pMeshMtl;

    GXLPCWSTR m_FileName;
    mutable Status mStatus;
    mutable FbxString mWindowMessage;

    FbxManager*     mSdkManager;
    FbxScene*       mScene;
    FbxImporter*    mImporter;
    FbxAnimLayer*   mCurrentAnimLayer;
    FbxNode*        mSelectedNode;
    GXBOOL          m_bSaveToMyFmt;
    GXBOOL          m_bAddToScene;
    GXBOOL          m_bUseTransformedVertices;

    int             mPoseIndex;
    FbxArray<FbxString*>  mAnimStackNameArray;
    FbxArray<FbxNode*>    mCameraArray;
    FbxArray<FbxPose*>    mPoseArray;

    mutable FbxTime mFrameTime, mStart, mStop, mCurrentTime;
	  mutable FbxTime mCache_Start, mCache_Stop;

    // Data for camera manipulation
    mutable int mLastX, mLastY;
    mutable FbxVector4 mCamPosition, mCamCenter;
    mutable double mRoll;
    //mutable CameraStatus mCameraStatus;

    //bool mPause;
    //ShadingMode mShadingMode;
    bool m_bSupportVBO;

    float4x4 m_matGlobal;
    NodeArray m_aNodes;

    void Init(GVScene* pMOScene, GXLPCWSTR pFileName);
};

// Initialize GLEW, must be called after the window is created.
//bool InitializeOpenGL();

#endif // _SCENE_CONTEXT_H

