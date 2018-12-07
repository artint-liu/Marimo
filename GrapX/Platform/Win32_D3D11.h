#ifdef ENABLE_GRAPHICS_API_DX11

#ifndef _WIN32_DIRECT3D_11_H_
#define _WIN32_DIRECT3D_11_H_

#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#include <windows.h>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <DirectXTex.h>

//#define WM_GX_RESETDEVICE (WM_USER + 100)

class IGXPlatform_Win32D3D11 : public IMOPlatform_Win32Base
{
private:
  //static GXDWORD    GXCALLBACK UITask   (GXLPVOID lParam);
  //static GXLRESULT  GXCALLBACK WndProc  (HWND hWnd, UINT message, GXWPARAM wParam, LPARAM lParam);

public:
  IGXPlatform_Win32D3D11();

  virtual GXHRESULT Initialize    (GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);
  virtual GXHRESULT Finalize      (GXINOUT GrapX::Graphics** ppGraphics);
  virtual GXVOID    GetPlatformID (GXPlatformIdentity* pIdentity);
  virtual GXLPCWSTR GetRootDir    ();
};

IGXPlatform_Win32D3D11* AppCreateD3D11Platform(GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics);

namespace GrapXToDX11
{
  struct GXD3D11_INPUT_ELEMENT_DESC
  {
    clStringA   SemanticName;
    UINT        SemanticIndex;
    DXGI_FORMAT Format;
    UINT        InputSlot;
    UINT        AlignedByteOffset;
    D3D11_INPUT_CLASSIFICATION InputSlotClass;
    UINT        InstanceDataStepRate;
  };
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, SemanticName) == offsetof(D3D11_INPUT_ELEMENT_DESC, SemanticName));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, SemanticIndex) == offsetof(D3D11_INPUT_ELEMENT_DESC, SemanticIndex));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, Format) == offsetof(D3D11_INPUT_ELEMENT_DESC, Format));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, InputSlot) == offsetof(D3D11_INPUT_ELEMENT_DESC, InputSlot));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, AlignedByteOffset) == offsetof(D3D11_INPUT_ELEMENT_DESC, AlignedByteOffset));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, InputSlotClass) == offsetof(D3D11_INPUT_ELEMENT_DESC, InputSlotClass));
  STATIC_ASSERT(offsetof(GXD3D11_INPUT_ELEMENT_DESC, InstanceDataStepRate) == offsetof(D3D11_INPUT_ELEMENT_DESC, InstanceDataStepRate));
  typedef clvector<GXD3D11_INPUT_ELEMENT_DESC>  GXD3D11InputElementDescArray;

  // 这些名字都没想好!!
  D3D_PRIMITIVE_TOPOLOGY      PrimitiveTopology  (GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount);

  GXLPCSTR      FormatToString              (GXFormat eFormat);
  DXGI_FORMAT   FormatFrom                  (GXFormat eFormat);
  //void          PrimitiveDescFromResUsage   (IN GXDWORD ResUsage, D3D11_BUFFER_DESC* pDesc);  // 只填充Usage和CPUAccessFlags
  void          PrimitiveDescFromResUsage   (D3D11_BUFFER_DESC* pDesc, GXResUsage eResUsage);
  //void          TextureDescFromResUsage     (IN GXDWORD ResUsage, D3D11_TEXTURE2D_DESC* pDesc);  // 只填充BindFlags,Usage和CPUAccessFlags
  void          TextureDescFromResUsage     (D3D11_TEXTURE2D_DESC* pDesc, GXResUsage eResUsage, GXUINT nMipLevels, GXBOOL bHasInitData);  // 只填充BindFlags,Usage和CPUAccessFlags
  D3D11_MAP     PrimitiveMapFromResUsage    (IN GXDWORD ResUsage);
  void          VertexLayoutFromVertexDecl  (LPCGXVERTEXELEMENT pVerticesDecl, GXD3D11InputElementDescArray* pArray);
  D3D11_FILTER  FilterFrom                  (GXTextureFilterType eMag, GXTextureFilterType eMin, GXTextureFilterType eMip);
} // namespace GrapXToDX11

//STATIC_ASSERT(GXTADDRESS_WRAP       == D3DTADDRESS_WRAP       );
//STATIC_ASSERT(GXTADDRESS_MIRROR     == D3DTADDRESS_MIRROR     );
//STATIC_ASSERT(GXTADDRESS_CLAMP      == D3DTADDRESS_CLAMP      );
//STATIC_ASSERT(GXTADDRESS_BORDER     == D3DTADDRESS_BORDER     );
//STATIC_ASSERT(GXTADDRESS_MIRRORONCE == D3DTADDRESS_MIRRORONCE );
#endif // _WIN32_XXOO

#endif // #ifndef _WIN32_DIRECT3D_11_H_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
