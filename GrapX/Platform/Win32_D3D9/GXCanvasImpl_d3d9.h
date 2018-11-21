#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPH_X_CANVAS_IMPLEMENT_H_
#define _GRAPH_X_CANVAS_IMPLEMENT_H_

#define CANVAS_SHARED_SHADER_REGCOUNT 64

class CKinematicGrid;
class GXEffectImpl;

//extern GXRENDERSTATE s_OpaqueFinalBlend[];
//extern GXRENDERSTATE s_AlphaFinalBlend[];
//extern GXRENDERSTATE s_OpaquePreBlend[];
//extern GXRENDERSTATE s_AlphaPreBlend[];

extern "C" GXBOOL GXDLLAPI  gxSetRectEmpty  (GXLPRECT lprc);
namespace D3D9
{
  class GraphicsImpl;
  class GTextureImpl;
  class GRenderState;

#include "Canvas/GXCanvasCoreImpl.h"
#include "Canvas/GXCanvasImpl_ClassDecl.inl"
} // namespace D3D9

//////////////////////////////////////////////////////////////////////////
#endif // _GRAPH_X_CANVAS_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
