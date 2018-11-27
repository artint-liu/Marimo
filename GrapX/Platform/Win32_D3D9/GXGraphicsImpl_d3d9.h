#if defined(ENABLE_GRAPHICS_API_DX9)
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GX_GRAPHICS_H_
#define _GX_GRAPHICS_H_

#define MAX_TEXTURE_STAGE  16
//////////////////////////////////////////////////////////////////////////
//
//GXGraphics
//{
//  GStateBlock_Old
//  ����/���� GTexture
//  ���� GXImage
//  {
//    ���� GTexture
//  }
//  ���� GXCanvas
//}
class CanvasCore;
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

  class GraphicsImpl : public GrapX::GraphicsBaseImpl
  {
    friend class GPrimitiveVertexOnlyImpl;
    friend class GPrimitiveVertexIndexImpl;
    friend class GSamplerStateImpl;
    friend class GXCanvasImpl;
    friend class Canvas3DImpl;
    friend class GShaderImpl;
    typedef clhash_set<GTextureFromUser*>   RTTexSet;
  public:
    enum
    {
      F_CREATEDEVICE  = 0x00000001,  // �����Լ��������豸
      F_DEVICEHASLOST = 0x00000002,  // �豸�Ѿ���ʧ
      F_ACTIVATE      = 0x00000004,  
      F_SHOWCONSOLE   = 0x00000008,  
      F_HASDRAWCALL   = 0x00000010,  // �����ù�RT֮��,���ù�DrawCall���ҳɹ�
    };
    typedef GrapXInternal::GXResourceMgr  GXResourceMgr;

    static Graphics* Create(const GRAPHICS_CREATION_DESC* pDesc);

    // ��ʼ����Ա
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

    GTexture*               m_pBackBufferTex;    // �ڲ�ʹ�õ�������
    GXImage*                m_pBackBufferImg;    // ���� m_pBackBufferTex
    GShader*                m_pBaseShader;
    //GShader*                m_pSimpleShader;
    GXEffect*               m_pBaseEffect;

    RTTexSet                m_setRTTexture;       // ��Ҫ�������ݵ�RTTexture�б�

    IDirect3DSurface9*      m_pD3DOriginSur;        // �豸ԭ���Ļ�����(BackBuffer)
    IDirect3DSurface9*      m_pD3DDepthStencilSur;  // �豸ԭ������Ȼ���
    GTextureImpl*           m_pDeviceOriginTex;
    GXDWORD                 m_dwBackBufferStencil;  // m_pDeviceOriginTex ʹ�õ�ģ�� [1, 255]

    GXINT                   m_nGraphicsCount;
    DWORD                   m_dwThreadId; // �������Begin()��End()�Ƿ������ͬһ���߳�
  };

  void ConvertVertexDeclToNative    (LPCGXVERTEXELEMENT pVerticesDecl, clBuffer** ppBuffer);
  void ConvertTexResUsageToNative   (IN GXDWORD ResUsage, OUT DWORD& D3DUsage, OUT D3DPOOL& Pool);
  void ConvertPrimResUsageToNative  (IN GXDWORD ResUsage, OUT DWORD* D3DUsage, OUT D3DPOOL* Pool);
  void ConvertPrimResUsageToNative  (OUT DWORD* D3DUsage, OUT D3DPOOL* Pool, GXResUsage eUsage);
  void ConvertNativeToTexResUsage   (IN DWORD Usage, IN D3DPOOL Pool, OUT GXDWORD& ResUsage);
  void ConvertTextureFileSaveFormat (IN GXLPCSTR szFormat, OUT D3DXIMAGE_FILEFORMAT* d3dxiff);
  
  inline LPDIRECT3DDEVICE9 GraphicsImpl::D3DGetDevice()
  {
    return m_pd3dDevice; 
  }
  inline clstd::Locker* GraphicsImpl::GetLocker()
  {
    return m_pGraphicsLocker;
  }
  inline GXLPCSTR GraphicsImpl::InlGetPlatformStringA() const
  {
    return "d3d9";
  }
} // namespace D3D9

#endif // _GX_GRAPHICS_H_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #if defined(ENABLE_GRAPHICS_API_DX9)