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
//#include <GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GShader.H>
#include <GrapX/GPrimitive.H>
#include <GrapX/GTexture.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXFont.H>
#include <GrapX/GXImage.H>
#include <GrapX/GXSprite.H>
#include <GrapX/MOSprite.H>
#include <GrapX/GXCanvas.H>
#include <GrapX/GXCanvas3D.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GCamera.h>
#include <GrapX/GVPhySimulator.h>
#include "GrapX/MOLogger.h"

#include <clTransform.h>

#include <GrapX/StdMtl.h>
#include <GrapX/GVNode.H>
#include <GrapX/GVMesh.H>
#include <GrapX/GVGeometry.H>
#include <GrapX/gvSkinnedMesh.h>
#include <GrapX/gvScene.h>
#include <GrapX/gvSkeleton.h>
#include <GrapX/gvSequence.h>

#include <GrapX/GXUser.H>
#include <GrapX/gxDevice.H>
#include <GrapX/gxKernel.H>
#include <GrapX/Platform.h>
#include <GrapX/DataPool.H>
#include <GrapX/DataPoolVariable.H>
#include <GrapX/DataPoolIterator.H>
#include <GrapX/DataInfrastructure.H>
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

#include <Engine/GXFC.H>
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