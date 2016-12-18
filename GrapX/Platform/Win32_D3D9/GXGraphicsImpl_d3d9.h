#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GX_GRAPHICS_H_
#define _GX_GRAPHICS_H_

#define MAX_TEXTURE_STAGE  16
//////////////////////////////////////////////////////////////////////////
//
//GXGraphics
//{
//  GStateBlock_Old
//  管理/创建 GTexture
//  创建 GXImage
//  {
//    引用 GTexture
//  }
//  创建 GXCanvas
//}
class GXCanvasCore;
class GXEffectImpl;
class GTexture;
class GXConsole;
class ILogger;

namespace Marimo {
  class ShaderConstName;
} // namespace Marimo

namespace D3D9
{
  class GTextureImpl;
  class GTextureFromUser;
  //class GTexBaseImpl;
  class GXCanvasCoreImpl;
  class GVertexDeclImpl;
  class GRasterizerStateImpl;
  class GBlendStateImpl;
  class GDepthStencilStateImpl;

  template<class InterfaceT> class GTexBaseImplT;
  typedef GTexBaseImplT<GTextureBase> GTexBaseImpl;

  struct GRAPHICS_CREATION_DESC
  {
    HWND          hWnd;
    GXBOOL        bWaitForVSync;
    LPDIRECT3D9   lpD3D;
    GXLPCWSTR     szRootDir;
    ILogger*      pLogger;
    GXDEFINITION* pParameter;
  };

  class GXGraphicsImpl : public GXGraphics
  {
    friend class GPrimitiveVImpl;
    friend class GPrimitiveVIImpl;
    friend class GSamplerStateImpl;
    friend class GXCanvasImpl;
    friend class GXCanvas3DImpl;
    friend class GShaderImpl;
    typedef clhash_set<GTextureFromUser*>   RTTexSet;
  public:
    enum
    {
      F_CREATEDEVICE  = 0x00000001,  // 对象自己创建的设备
      F_DEVICEHASLOST = 0x00000002,  // 设备已经丢失
      F_ACTIVATE      = 0x00000004,  
      F_SHOWCONSOLE   = 0x00000008,  
      F_HASDRAWCALL   = 0x00000010,  // 自设置过RT之后,调用过DrawCall并且成功
    };
    typedef GrapXInternal::GXResourceMgr  GXResourceMgr;

    static GXGraphics* Create(const GRAPHICS_CREATION_DESC* pDesc);

    // 初始化成员
    virtual GXBOOL Initialize(const GRAPHICS_CREATION_DESC* pDesc);

    //GXHRESULT CreateOffscreenPlainSurface       (GTexture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage);
    GXHRESULT CreateTextureFromD3DSurface       (GTextureImpl** ppTexture, LPDIRECT3DSURFACE9 lpD3DSurface);

#include "Platform/CommonInline/GXGraphicsImpl_ClassDecl.inl"
  public:
    GXEffect*                 IntGetEffect        ();
    void                      IntGetDimension     (GXUINT& nWidth, GXUINT& nHeight);  // 
    inline LPDIRECT3DDEVICE9  D3DGetDevice        ();
    GXBOOL                    D3DGetPresentParam  (D3DPRESENT_PARAMETERS* pd3dpp);
    inline clstd::Locker*     GetLocker           ();
  private:
    GXHRESULT     Test              ();
    GXBOOL        IntCheckSizeOfTargetAndDepth();
    inline GXBOOL IntCheckRTTexture (GTextureImpl* pRTTexture);

  private:
    const GXUINT            s_uCanvasCacheCount;
    HWND                    m_hWnd;
    LPDIRECT3DDEVICE9       m_pd3dDevice;
    D3DPRESENT_PARAMETERS   m_d3dpp;

    GTexture*               m_pBackBufferTex;    // 内部使用的纹理缓冲
    GXImage*                m_pBackBufferImg;    // 引用 m_pBackBufferTex
    GShader*                m_pSimpleShader;
    GXEffect*               m_pSimpleEffect;

    RTTexSet                m_setRTTexture;       // 需要备份内容的RTTexture列表

    IDirect3DSurface9*      m_pD3DOriginSur;        // 设备原来的缓冲区(BackBuffer)
    IDirect3DSurface9*      m_pD3DDepthStencilSur;  // 设备原来的深度缓冲
    GTextureImpl*           m_pDeviceOriginTex;
    GXDWORD                 m_dwBackBufferStencil;  // m_pDeviceOriginTex 使用的模板 [1, 255]

    GXINT                   m_nGraphicsCount;
    DWORD                   m_dwThreadId; // 用来检测Begin()和End()是否调用自同一个线程
  };

  void ConvertVertexDeclToNative    (LPCGXVERTEXELEMENT pVerticesDecl, clBuffer** ppBuffer);
  void ConvertTexResUsageToNative   (IN GXDWORD ResUsage, OUT DWORD& D3DUsage, OUT D3DPOOL& Pool);
  void ConvertPrimResUsageToNative  (IN GXDWORD ResUsage, OUT DWORD* D3DUsage, OUT D3DPOOL* Pool);
  void ConvertNativeToTexResUsage   (IN DWORD Usage, IN D3DPOOL Pool, OUT GXDWORD& ResUsage);
  void ConvertTextureFileSaveFormat (IN GXLPCSTR szFormat, OUT D3DXIMAGE_FILEFORMAT* d3dxiff);
  
  inline LPDIRECT3DDEVICE9 GXGraphicsImpl::D3DGetDevice()
  {
    return m_pd3dDevice; 
  }
  inline clstd::Locker* GXGraphicsImpl::GetLocker()
  {
    return m_pGraphicsLocker;
  }
  inline GXLPCSTR GXGraphicsImpl::InlGetPlatformStringA() const
  {
    return "d3d9";
  }
} // namespace D3D9

#endif // _GX_GRAPHICS_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)