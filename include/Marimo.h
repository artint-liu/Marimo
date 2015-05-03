#ifndef _MARIMO_H_
#define _MARIMO_H_

#ifdef GAMEENGINE_EXPORTS
#error Can not include this file in GameEngine project
#endif // #ifdef GAMEENGINE_EXPORTS

#ifdef GXDLL_API_EXPORTS
#error Can not include this file in GrapX project
#endif // #ifdef GAMEENGINE_EXPORTS

//////////////////////////////////////////////////////////////////////////
//
// clstd Headers
//
#include <cltree.h>

//#include "3D/GrapVR.H"

// fbx 文件解析库, 来自 Autodesk
//#include "GameEngine\third_party\FBX SDK\2013.2\include\fbxsdk.h"

// ode 物理仿真/碰撞检测库, 来自ode.org
//#include "GameEngine\third_party\include\ode\ode.h"

//////////////////////////////////////////////////////////////////////////
//
// GrapX Headers
//

#include <GrapX.H>
#include <GXApp.H>
#include <GUnknown.H>
#include <GResource.H>
#include <GShader.H>
#include <GPrimitive.H>
#include <GTexture.H>
#include <GRegion.H>
#include <GXFont.H>
#include <GXImage.H>
#include <GXSprite.H>
#include <GXCanvas.H>
#include <GXCanvas3D.h>
#include <GXGraphics.h>
#include <GCamera.h>
#include <GVPhySimulator.h>
#include "MOLogger.h"

#include <clTransform.h>

#include <3D/StdMtl.h>
#include <3D/GVNode.H>
#include <3D/GVMesh.H>
#include <3D/GVGeometry.H>
#include <3D/gvSkinnedMesh.h>
#include <3D/gvScene.h>
#include <3D/gvSkeleton.h>
#include <3D/gvSequence.h>

#include <GXUser.H>
#include <gxDevice.H>
#include <gxKernel.H>
#include <Platform/Platform.h>
#include <DataPool.H>
#include <DataPoolVariable.H>
#include <DataInfrastructure.H>
//#include "third_party\FBX SDK\2013.2\include\fbxsdk.h"


//////////////////////////////////////////////////////////////////////////
//
// Game Engine Headers
//

#include <GameEngine/GameEngine.h>
#include <GameEngine/EditorUtility/TransformAxis.h>
#include <GameEngine/EditorUtility/ScalingAxis.h>
#include <GameEngine/EditorUtility/RotationAxis.h>
#include <GameEngine/EditorUtility/TranslationAxis.h>
#include <GameEngine/EditorUtility/Drag.h>

#include <GameEngine/UI/GXFC.H>
#include <GameEngine/Scene/SkyQuad.h>
#include <GameEngine/Scene/BrunetonSkyQuad.h>
#include <GameEngine/VideoCapture.h>
#include <MOAudio.h>

class CMOWnd;
class CMODialog;

typedef CMOWnd*               LPCMOWnd;
typedef CMODialog*            LPCMODialog;
typedef GVPhySimulator*       LPGVPHYSIMULATOR;
typedef MOAudio*              LPMOAUDIO;
typedef IStreamLogger*        LPSTREAMLOGGER;

extern "C"
{
  GAMEENGINE_API GXHRESULT    CreateBulletSimulator(float fGravity, GXBOOL bAsync, LPGVPHYSIMULATOR* ppPhySimulator);
  GAMEENGINE_API GXHRESULT    MOCreateAudio(LPMOAUDIO* ppAudio, GXDWORD dwAudioInterfaceCC, GXDWORD dwFlags);
  GAMEENGINE_API LPCMODialog  MOCreateConsoleDlg(LPSTREAMLOGGER pLogger);
  GAMEENGINE_API LPCMODialog  GXUICreateUniversalDialog(GXLPCWSTR szTemplate, LPCMOWnd pParent);
} // extern "C"

#endif // _MARIMO_H_