#include "GrapX.h"
//#include "GrapX/GUnknown.h"
#include "GXApp.h"
#include "GrapX/Platform.h"
#include "GrapX/gxDevice.h"

class IGXPlatform_Win32OpenGL;
class IGXPlatform_Win32D3D9;
class IGXPlatform_Win32D3D11;
class IGXPlatform_XOpenGLES2;
//////////////////////////////////////////////////////////////////////////
IGXPlatform_Win32OpenGL*  AppCreateOpenGLPlatform   (GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);
IGXPlatform_Win32D3D9*    AppCreateD3D9Platform     (GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);
IGXPlatform_Win32D3D11*   AppCreateD3D11Platform    (GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);
IGXPlatform_XOpenGLES2*   AppCreateOpenGLES2Platform(GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);
//////////////////////////////////////////////////////////////////////////

GXHRESULT GXApp::Go(GXAPP_DESC* pDesc)
{  
  m_pGraphics = NULL;
  m_pIPlatform = NULL;

  if(pDesc->cbSize != sizeof(GXAPP_DESC)) {
    CLOG_ERROR("pDesc->cbSize != sizeof(GXAPP_DESC)");
    return GX_ERROR_OUTOFRANGE;
  }

//#ifdef _DEV_PROOF
//  IGXPlatform_Win32D3D9* pPlatform = new IGXPlatform_Win32D3D9;
//  IGXPlatform_Win32OpenGL* pPlatformDevProof = new IGXPlatform_Win32OpenGL;
//  m_pGraphics = NULL;
//  m_pIPlatform = pPlatform;
//  m_pIPlatformDevProof = pPlatformDevProof;
//  pPlatform->Initialize(this, pDesc, &m_pGraphics);
//  m_pIPlatformDevProof->Initialize(this, pDesc, NULL);
//#else
  switch(pDesc->idPlatform)
  {
  case GXPLATFORM_UNKNOWN:
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX9
  case GXPLATFORM_WIN32_DIRECT3D9:
    {
      m_pIPlatform = (IGXPlatform*)AppCreateD3D9Platform(this, pDesc, &m_pGraphics);
    }
    break;
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11
  case GXPLATFORM_WIN32_DIRECT3D11:
    {
      m_pIPlatform = (IGXPlatform*)AppCreateD3D11Platform(this, pDesc, &m_pGraphics);
    }
    break;
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

  case GXPLATFORM_WIN32_OPENGL:
    {
#ifdef ENABLE_GRAPHICS_API_WOGL
      m_pIPlatform = (IGXPlatform*)AppCreateOpenGLPlatform(this, pDesc, &m_pGraphics);
#else
      return GX_FAIL;
#endif // ENABLE_GRAPHICS_API_WOGL
    }
    break;
  case GXPLATFORM_X_OPENGLES2:
    {
#ifdef ENABLE_GRAPHICS_API_GLES2
      m_pIPlatform = (IGXPlatform*)AppCreateOpenGLES2Platform(this, pDesc, &m_pGraphics);
#else
      return GX_FAIL;
#endif // ENABLE_GRAPHICS_API_GLES2
    }
    break;
  default:
    ASSERT(0);
    return GX_FAIL;
  }


  // 循环
  if(m_pIPlatform) {
    m_pIPlatform->MainLoop();
    m_pIPlatform->Finalize(&m_pGraphics);
    SAFE_DELETE(m_pIPlatform);
  }

  return GX_OK;
}

GrapX::Graphics*  GXApp::GetGraphicsUnsafe()
{
  return m_pGraphics;
}

GXHRESULT GXApp::OnCreate()
{
  return GX_OK;
}
GXHRESULT GXApp::OnDestroy()
{
  return GX_OK;
}
GXHRESULT GXApp::OnResizing(GXSIZE* sizeNew)
{
  return GX_OK;
}
GXHRESULT GXApp::Render()
{
  return GX_OK;
}

GXHRESULT GXApp::ActionBegin(GXAPPACTIONINFO* pActionInfo)
{
  return GX_OK;
}
GXHRESULT GXApp::ActionMove(GXAPPACTIONINFO* pActionInfo)
{
  return GX_OK;
}
GXHRESULT GXApp::ActionEnd(GXAPPACTIONINFO* pActionInfo)
{
  return GX_OK;
}
GXHRESULT GXApp::ActionExtra(GXAPPACTIONINFO* pActionInfo)
{
  return GX_OK;
}
GXHRESULT GXApp::KeyMessage(GXAPPKEYINFO* pKeyInfo)
{
  return GX_OK;
}
//////////////////////////////////////////////////////////////////////////
extern "C"
{
  GXBOOL GXDLLAPI MOPlatformEnumToStringW (GXPlaformIdentity ePlatform, GXLPWSTR szName, int nSize)
  {
    // TODO: 使用注册链表方式记录
    switch (ePlatform)
    {
    case GXPLATFORM_WIN32_DIRECT3D9:
      clstd::strcpynT(szName, _CLTEXT("D3D9"), nSize);
      return TRUE;
    case GXPLATFORM_WIN32_DIRECT3D11:
      clstd::strcpynT(szName, _CLTEXT("D3D11"), nSize);
      return TRUE;
    case GXPLATFORM_WIN32_OPENGL:
      clstd::strcpynT(szName, _CLTEXT("WGL2"), nSize);
      return TRUE;
    case GXPLATFORM_X_OPENGLES2:
      clstd::strcpynT(szName, _CLTEXT("GLES2"), nSize);
      return TRUE;
    }
    return FALSE;
  }

  GXPlaformIdentity GXDLLAPI MOPlatformStringToEnumW (GXLPCWSTR szName)
  {
    // TODO: 使用注册链表方式记录
    if(clstd::strncmpT(szName, _CLTEXT("D3D9"), 8) == 0)
      return GXPLATFORM_WIN32_DIRECT3D9;
    else if(clstd::strncmpT(szName, _CLTEXT("D3D11"), 8) == 0)
      return GXPLATFORM_WIN32_DIRECT3D11;
    else if(clstd::strncmpT(szName, _CLTEXT("WGL2"), 8) == 0)
      return GXPLATFORM_WIN32_OPENGL;
    else if(clstd::strncmpT(szName, _CLTEXT("GLES2"), 8) == 0)
      return GXPLATFORM_X_OPENGLES2;
    else
      return GXPLATFORM_UNKNOWN;
  }
}

#if defined(_WIN32) || defined(_WINDOWS)
GXBOOL PixelFormatFromDescStructToGraphX(PIXELFORMATDESCRIPTOR* pfd, GXFormat* pFmtColor, GXFormat* pFmtDepthStencil)
{
  GXBOOL bRet = FALSE;
  struct COLORFORMATCONTEXT
  {
    GXBYTE    cColorBits;
    GXBYTE    cRedBits;
    GXBYTE    cRedShift;
    GXBYTE    cGreenBits;
    GXBYTE    cGreenShift;
    GXBYTE    cBlueBits;
    GXBYTE    cBlueShift;
    GXBYTE    cAlphaBits;
    GXBYTE    cAlphaShift;
    GXFormat  eGraphFormat;
  };
  const static COLORFORMATCONTEXT ColorFormatTable[] ={
    {32, 8, 16, 8, 8, 8, 0, 0, 0, GXFMT_X8R8G8B8},
    {0,},
  };

  if(pFmtColor != NULL)
  {
    for(int i = 0;; i++)
    {
      if(ColorFormatTable[i].cColorBits == 0)
      {
        CLBREAK;
        *pFmtColor = GXFMT_UNKNOWN;
        break;
      }
      if(ColorFormatTable[i].cColorBits == pfd->cColorBits && 
        ColorFormatTable[i].cRedBits    == pfd->cRedBits &&
        ColorFormatTable[i].cRedShift   == pfd->cRedShift &&
        ColorFormatTable[i].cGreenBits  == pfd->cGreenBits &&
        ColorFormatTable[i].cGreenShift == pfd->cGreenShift &&
        ColorFormatTable[i].cBlueBits   == pfd->cBlueBits &&
        ColorFormatTable[i].cBlueShift  == pfd->cBlueShift &&
        ColorFormatTable[i].cAlphaBits  == pfd->cAlphaBits &&
        ColorFormatTable[i].cAlphaShift == pfd->cAlphaShift)
      {
        *pFmtColor = ColorFormatTable[i].eGraphFormat;
        bRet = TRUE;
        break;
      }
    }
  }

  struct DEPTHSTENCILFORMATCONTEXT
  {
    GXBYTE  cDepthBits;
    GXBYTE  cStencilBits;
    GXFormat eGraphFormat;
  };

  const static DEPTHSTENCILFORMATCONTEXT DepthStencilFormatTable[] = {
    {24, 8, GXFMT_D24S8},
    {0, 0, GXFMT_UNKNOWN},
  };

  if(pFmtDepthStencil != NULL)
  {
    for(int i = 0;; i++)
    {
      if(DepthStencilFormatTable[i].eGraphFormat == GXFMT_UNKNOWN)
      {
        CLBREAK;
        *pFmtDepthStencil = GXFMT_UNKNOWN;
        break;
      }
      if(DepthStencilFormatTable[i].cDepthBits == pfd->cDepthBits &&
        DepthStencilFormatTable[i].cStencilBits == pfd->cStencilBits)
      {
        *pFmtDepthStencil = DepthStencilFormatTable[i].eGraphFormat;
        bRet = TRUE;
        break;
      }
    }
  }
  return bRet;
}
//////////////////////////////////////////////////////////////////////////
#endif // defined(_WIN32) || defined(_WINDOWS)
