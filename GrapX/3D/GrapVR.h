// Graph 3D 部分的头文件定义
#ifndef _GRAP_VIRTUAL_REALITY_H_
#define _GRAP_VIRTUAL_REALITY_H_

//#define FBX_SDK

#ifdef FBX_SDK
#include <fbxsdk.h>
#endif // #ifdef FBX_SDK

#include "clTree.H"
#include "clTransform.h"

#include "3D/StdMtl.h"
#include "3D/gvNode.h"
#include "3D/gvMesh.h"
#include "3D/gvSkinnedMesh.h"
#include "3D/gvGeometry.h"
#include "3D/gvScene.h"
#include "3D/gvSkeleton.h"
#include "3D/gvSequence.h"


//////////////////////////////////////////////////////////////////////////
//
//
// 将来去掉这个文件, 把这些文件直接包含到同一的头文件中
// 太多综合头文件不好!!!
//
//
//////////////////////////////////////////////////////////////////////////

#endif // _GRAP_VIRTUAL_REALITY_H_