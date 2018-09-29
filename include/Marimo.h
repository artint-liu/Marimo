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
//#include "socket/clSocket.h"
//#include "3D/GrapVR.h"

// fbx 文件解析库, 来自 Autodesk
//#include "GameEngine\third_party\FBX SDK\2013.2\include\fbxsdk.h"

// ode 物理仿真/碰撞检测库, 来自ode.org
//#include "GameEngine\third_party\include\ode\ode.h"

//////////////////////////////////////////////////////////////////////////
//
// GrapX Headers
//

#include <GrapX.h>
#include <GXApp.h>
//#include <GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GShader.h>
#include <GrapX/GPrimitive.h>
#include <GrapX/GTexture.h>
#include <GrapX/GRegion.h>
#include <GrapX/GXFont.h>
#include <GrapX/GXImage.h>
#include <GrapX/GXSprite.h>
#include <GrapX/MOSprite.h>
#include <GrapX/GXCanvas.h>
#include <GrapX/GXCanvas3D.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GCamera.h>
#include <GrapX/GVPhySimulator.h>
#include "GrapX/MOLogger.h"

#include <clTransform.h>

#include <GrapX/StdMtl.h>
#include <GrapX/GVNode.h>
#include <GrapX/GVMesh.h>
#include <GrapX/GVGeometry.h>
#include <GrapX/gvSkinnedMesh.h>
#include <GrapX/gvScene.h>
#include <GrapX/gvSkeleton.h>
#include <GrapX/gvSequence.h>

#include <GrapX/GXUser.h>
#include <GrapX/gxDevice.h>
#include <GrapX/gxKernel.h>
#include <GrapX/Platform.h>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include <GrapX/DataPoolIterator.h>
#include <GrapX/DataInfrastructure.h>
//#include "third_party\FBX SDK\2013.2\include\fbxsdk.h"


//////////////////////////////////////////////////////////////////////////
//
// Game Engine Headers
//

#include <Engine.h>
#include <Engine/TransformAxis.h>
#include <Engine/ScalingAxis.h>
#include <Engine/RotationAxis.h>
#include <Engine/TranslationAxis.h>
#include <Engine/Drag.h>

#include <Engine/GXFC.h>
#include <Engine/SkyQuad.h>
#include <Engine/BrunetonSkyQuad.h>
//#include <Engine/VideoCapture.h>
#include <Engine/MOAudio.h>

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