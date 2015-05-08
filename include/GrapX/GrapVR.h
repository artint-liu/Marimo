// Graph 3D 部分的头文件定义
#ifndef _GRAP_VIRTUAL_REALITY_H_
#define _GRAP_VIRTUAL_REALITY_H_

//#define FBX_SDK

#ifdef FBX_SDK
#include <fbxsdk.h>
#endif // #ifdef FBX_SDK

#include "clTree.H"
#include "clTransform.h"

#include "StdMtl.h"
#include "gvNode.h"
#include "gvMesh.h"
#include "gvSkinnedMesh.h"
#include "gvGeometry.h"
#include "gvScene.h"
#include "gvSkeleton.h"
#include "gvSequence.h"


//////////////////////////////////////////////////////////////////////////
//
//
// 将来去掉这个文件, 把这些文件直接包含到同一的头文件中
// 太多综合头文件不好!!!
//
//
//////////////////////////////////////////////////////////////////////////

#endif // _GRAP_VIRTUAL_REALITY_H_