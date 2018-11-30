#ifndef _WIN32_DIRECT3D_9_H_
#define _WIN32_DIRECT3D_9_H_

#if defined(ENABLE_GRAPHICS_API_DX9) && (defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS))
#include <windows.h>
#include <d3dx9.h>

class IGXPlatform_Win32D3D9 : public IMOPlatform_Win32Base
{
private:
  //static GXDWORD    GXCALLBACK UITask   (GXLPVOID lParam);
  static LRESULT    GXCALLBACK WndProc  (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
  LPDIRECT3D9 m_pd3d9;

  IGXPlatform_Win32D3D9();
  virtual ~IGXPlatform_Win32D3D9();

  GXHRESULT Initialize      (GXIN GXApp* pApp, GXIN GXAPP_DESC* pDesc, GXOUT Graphics** ppGraphics) override;
  GXHRESULT Finalize        (GXINOUT Graphics** ppGraphics) override;
  GXVOID    GetPlatformID   (GXPlatformIdentity* pIdentity) override;
  GXLPCWSTR GetRootDir      () override;
  //GXDWORD   GetAppDescStyle () const override;
};

IGXPlatform_Win32D3D9* AppCreateD3D9Platform(GXApp* pApp, GXAPP_DESC* pDesc, Graphics** ppGraphics);

STATIC_ASSERT(GXTADDRESS_WRAP       == D3DTADDRESS_WRAP       );
STATIC_ASSERT(GXTADDRESS_MIRROR     == D3DTADDRESS_MIRROR     );
STATIC_ASSERT(GXTADDRESS_CLAMP      == D3DTADDRESS_CLAMP      );
STATIC_ASSERT(GXTADDRESS_BORDER     == D3DTADDRESS_BORDER     );
STATIC_ASSERT(GXTADDRESS_MIRRORONCE == D3DTADDRESS_MIRRORONCE );
#endif // _WIN32_XXOO

#endif // _WIN32_DIRECT3D_9_H_