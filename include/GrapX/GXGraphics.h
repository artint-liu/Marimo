#ifndef _GRAPH_X_GRAPHICS_H_
#define _GRAPH_X_GRAPHICS_H_

class GXImage;
class GTextureBase;
class GTexture;
class GTexture3D;
class GTextureCube;
class GPrimitive;
class GPrimitiveV;
class GPrimitiveVI;
class GXGraphics;
class GXFont;
class GRegion;
class GAllocator;
class GXShaderMgr;
class GShaderStub;
class GXCanvas3D;
class GBlendState;
class GDepthStencilState;
class GSamplerState;
class GRasterizerState;
class GXEffect;
class GXMaterialInst;
class IGXPlatform;
class GVertexDeclaration;
struct MOSHADER_ELEMENT_SOURCE;
struct MTLFILEPARAMDESC;
enum GXPlaformIdentity;
//enum GXPRIMITIVETYPE;

// GXGRAPHICSPARAM �ı�־λ
//enum GXGRAPHICSPARAMFLAG
//{
//    GPF_VSYNC    = 0x00000001,    // ��ֱͬ��
//    GPF_SIZEABLE = 0x00000002,    // �ɸı䴰�ڵĴ�С�ߴ�
//};

//struct GXIMAGE_LOAD_INFO
//{
//    GXUINT                  Width;
//    GXUINT                  Height;
//    GXUINT                  Depth;
//    GXUINT                  MipLevels;
//    GXGraphicsFormat        Format;
//    D3DRESOURCETYPE         ResourceType;
//    D3DXIMAGE_FILEFORMAT    ImageFileFormat;
//};


//struct GXGRAPHICSPARAM
//{
//    GXUINT                cbSize;                // �ṹ���С, �������ֺ����İ汾
//    GXUINT                BackBufferWidth;    // ��̨����Ŀ��
//    GXUINT                BackBufferHeight;    // ��̨����ĸ߶�
//    D3DFORMAT           BackBufferFormat;    // ��̨����ĸ�ʽ
//    GXUINT                BackBufferCount;    // ��̨��������
//    D3DFORMAT           DepthStencilFormat;    // ��Ȼ����ʽ, �����һ��Ҫ������
//    GXDWORD                dwFlags;            // ��־λ, �ο� GXGRAPHICSPARAMFLAG
//};

// GXImage �� GTexture �Ĺ�ϵ:
// 1. GTexture ֱ������D3DTexture, GXImage���� GTexture
// 2. GTexture �Ǵ�����ԭʼ����, ���ļ�����ʱ���ᱻ����, GXImage ���ļ�����ʱ,�������Ϊ��ͼĿ�����,
//        ����ԭ��������,���´���ΪRenderTarget���Ե�GTexture
// 3. GTexture ƫ����Ч��, GXImageƫ����ʵ��

// ��������������,GX---����߼�����, G---����D3D��ֱ�Ӻ���,GXGraphics����

//////////////////////////////////////////////////////////////////////////
class GXGraphics : public GResource
{
public:
    GXGraphics() : GResource(0, RESTYPE_GRAPHICS){}
    // �ӿ�ʵ��
    GXSTDINTERFACE(GXHRESULT AddRef());
    GXSTDINTERFACE(GXHRESULT Release());

    // �豸�¼�
    GXSTDINTERFACE(GXHRESULT Invoke             (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(void      GetPlatformID      (GXPlaformIdentity* pIdentity));


    // �����,֮�������D3D����������Graphics�ӿ�ʵ��
    GXSTDINTERFACE(GXBOOL    Activate           (GXBOOL bActive));
    GXSTDINTERFACE(GXHRESULT Begin              ());
    GXSTDINTERFACE(GXHRESULT End                ());
    GXSTDINTERFACE(GXHRESULT Present            ());

    GXSTDINTERFACE(GXHRESULT Resize             (int nWidth, int nHeight));

  //GXSTDINTERFACE(GXHRESULT RegisterResource   (GResource* pResource));
    GXSTDINTERFACE(GXHRESULT RegisterResource   (GResource* pResource, LPCRESKETCH pSketch = NULL));
    GXSTDINTERFACE(GXHRESULT UnregisterResource (GResource* pResource));


    GXSTDINTERFACE(GXHRESULT SetPrimitive       (GPrimitive* pPrimitive, GXUINT uStreamSource = 0));
    GXSTDINTERFACE(GXHRESULT SetPrimitiveV      (GPrimitiveV* pPrimitive, GXUINT uStreamSource = 0));
    GXSTDINTERFACE(GXHRESULT SetPrimitiveVI     (GPrimitiveVI* pPrimitive, GXUINT uStreamSource = 0));

    // ������GXGraphicsû�� Set** �ຯ��, SetTexture ����, ��Ϊ SetTexture ͬʱ�縺�����ָ���豸���������
    GXSTDINTERFACE(GXHRESULT SetTexture           (GTextureBase* pTexture, GXUINT uStage = 0));
    GXSTDINTERFACE(GXHRESULT SetRasterizerState   (GRasterizerState* pRasterizerState));
    GXSTDINTERFACE(GXHRESULT SetBlendState        (GBlendState* pBlendState));
    GXSTDINTERFACE(GXHRESULT SetDepthStencilState (GDepthStencilState* pDepthStencilState));
    GXSTDINTERFACE(GXHRESULT SetSamplerState      (GSamplerState* pSamplerState));

    GXSTDINTERFACE(GXHRESULT Clear              (const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil));
    GXSTDINTERFACE(GXHRESULT DrawPrimitive      (const GXPrimitiveType eType, const GXUINT StartVertex, const GXUINT PrimitiveCount));
    GXSTDINTERFACE(GXHRESULT DrawPrimitive      (const GXPrimitiveType eType, const GXINT BaseVertexIndex, const GXUINT MinIndex, const GXUINT NumVertices, const GXUINT StartIndex, const GXUINT PrimitiveCount));

    //////////////////////////////////////////////////////////////////////////
    // �ͼ�����
    // GTexture

    // CreateTexture ֧�� TEXSIZE_* ��Ĳ�����Ϊ����Ŀ���.
    // ���szName��ΪNULL, ����ֵΪ0ʱ��ʾ��һ�δ���, >1ʱ���ظ���Դ�����õĴ���

    // 2D Texture
    GXSTDINTERFACE(GXHRESULT CreateTexture(
      GTexture**  ppTexture,      // ���ص�����ָ��
      GXLPCSTR    szName,         // ��Դ����, ���ΪNULL, ��Ĭ��˽������, �������û�ָ������, 
                                  // ����Ѵ��ڸ�����,��ֱ�ӷ��ض���,���ü�һ, ����Ĳ�����������
      GXUINT      Width,          // ��Ⱥ͸߶�, ֧�� TEXSIZE_* �Ķ���
      GXUINT      Height,
      GXUINT      MipLevels, 
      GXFormat    Format, 
      GXDWORD     ResUsage
      ));

    GXSTDINTERFACE(GXHRESULT CreateTextureFromFileW  (GTexture** ppTexture, GXLPCWSTR pSrcFile));

    GXSTDINTERFACE(GXHRESULT CreateTextureFromFileExW(
      GTexture**  ppTexture, 
      GXLPCWSTR   pSrcFile, 
      GXUINT      Width, 
      GXUINT      Height, 
      GXUINT      MipLevels, 
      GXFormat    Format, 
      GXDWORD     ResUsage, 
      GXDWORD     Filter = GXFILTER_NONE,
      GXDWORD     MipFilter = GXFILTER_NONE, 
      GXCOLORREF  ColorKey = 0,
      GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // Volume Texture
    GXSTDINTERFACE(GXHRESULT CreateTexture3D(
      GTexture3D**  ppTexture, 
      GXLPCSTR      szName,
      GXUINT        Width,          // ���,�߶Ⱥ����
      GXUINT        Height,
      GXUINT        Depth,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage));

    GXSTDINTERFACE(GXHRESULT CreateTexture3DFromFileW(
      GTexture3D**  ppTexture,
      GXLPCWSTR     pSrcFile));

    GXSTDINTERFACE(GXHRESULT CreateTexture3DFromFileExW(
      GTexture3D**  ppTexture,
      GXLPCWSTR     pSrcFile,
      GXUINT        Width,
      GXUINT        Height,
      GXUINT        Depth,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage, 
      GXDWORD       Filter = GXFILTER_NONE,
      GXDWORD       MipFilter = GXFILTER_NONE, 
      GXCOLORREF    ColorKey = 0,
      GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // Cube Texture
    GXSTDINTERFACE(GXHRESULT CreateTextureCube(
      GTextureCube**ppTexture, 
      GXLPCSTR      szName,
      GXUINT        Size,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage));

    GXSTDINTERFACE(GXHRESULT CreateTextureCubeFromFileW(
      GTextureCube**ppTexture,
      GXLPCWSTR     pSrcFile));

    GXSTDINTERFACE(GXHRESULT CreateTextureCubeFromFileExW(
      GTextureCube**ppTexture,
      GXLPCWSTR     pSrcFile,
      GXUINT        Size,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage, 
      GXDWORD       Filter = GXFILTER_NONE,
      GXDWORD       MipFilter = GXFILTER_NONE, 
      GXCOLORREF    ColorKey = 0,
      GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // GPrimitive
    GXSTDINTERFACE(GXHRESULT CreatePrimitiveV(
      GPrimitiveV**       pPrimitive,           // ���ض���
      GXLPCSTR            szName,               // ��Դ��, NULLΪ��������, ����������ڵ�һ�δ���ʱʹ������Ĳ���
      LPCGXVERTEXELEMENT  pVertexDecl,          // ��������
      GXDWORD             ResUsage,             // ��Դ����/����
      GXUINT              uVertexCount,         // ������
      GXUINT              uVertexStride = 0,    // ÿ��������ֽ���
      GXLPVOID            pVertInitData = NULL  // ��ʼ������
      ));

    GXSTDINTERFACE(GXHRESULT CreatePrimitiveVI(
      GPrimitiveVI**      pPrimitive, 
      GXLPCSTR            szName, 
      LPCGXVERTEXELEMENT  pVertexDecl,
      GXDWORD             ResUsage, 
      GXUINT              uIndexCount,          // ��������
      GXUINT              uVertexCount,
      GXUINT              uVertexStride = 0,
      GXLPCVOID           pIdxInitData = NULL,  // ������ʼ������
      GXLPCVOID           pVertInitData = NULL  // �����������ʼ�����ݱ���ͬʱ��Ч����ͬʱΪNULL.
      ));

    // GSahder
    GXSTDINTERFACE(GXHRESULT    CreateShaderFromFileW       (GShader** ppShader, GXLPCWSTR szShaderDesc));
    GXSTDINTERFACE(GXHRESULT    CreateShaderFromFileA       (GShader** ppShader, GXLPCSTR szShaderDesc));
    //GXSTDINTERFACE(GXHRESULT    CreateShader                (GShader** ppShader, MOSHADER_ELEMENT_SOURCE* pSdrElementSrc));
    GXSTDINTERFACE(GXHRESULT    CreateShaderStub            (GShaderStub** ppShaderStub));


    // GRegion
    GXSTDINTERFACE(GXHRESULT    CreateRectRgn              (GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom));
    GXSTDINTERFACE(GXHRESULT    CreateRectRgnIndirect      (GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount));
    GXSTDINTERFACE(GXHRESULT    CreateRoundRectRgn         (GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse));

    GXSTDINTERFACE(GXHRESULT    CreateVertexDeclaration    (GVertexDeclaration** ppVertexDecl, LPCGXVERTEXELEMENT lpVertexElement));
    //////////////////////////////////////////////////////////////////////////
    // �߼�����

    // GXCanvas
    GXSTDINTERFACE(GXCanvas*    LockCanvas                 (GXImage* pImage, GXCONST LPREGN lpRegn, GXDWORD dwFlags));

    // ��� pImage Ϊ NULL, ����� DepthStencil ����
    GXSTDINTERFACE(GXHRESULT    CreateCanvas3D             (GXCanvas3D** ppCanvas3D, GXImage* pImage, GXFormat eDepthStencil, LPCREGN lpRegn = NULL, float fNear = 0.0f, float fFar = 1.0f));
    GXSTDINTERFACE(GXHRESULT    CreateCanvas3D             (GXCanvas3D** ppCanvas3D, GXImage* pImage, GTexture* pDepthStencil, LPCREGN lpRegn = NULL, float fNear = 0.0f, float fFar = 1.0f));

    GXSTDINTERFACE(GXHRESULT    CreateEffect               (GXEffect** ppEffect, GShader* pShader));
    GXSTDINTERFACE(GXHRESULT    CreateMaterial             (GXMaterialInst** ppMtlInst, GShader* pShader));
    GXSTDINTERFACE(GXHRESULT    CreateMaterialFromFileW    (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType));
    GXSTDINTERFACE(GXHRESULT    CreateMaterialFromFileA    (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType));

    GXSTDINTERFACE(GXHRESULT    CreateRasterizerState      (GRasterizerState** ppRasterizerState, GXRASTERIZERDESC* pRazDesc));
    GXSTDINTERFACE(GXHRESULT    CreateBlendState           (GBlendState** ppBlendState, GXBLENDDESC* pState, GXUINT nNum));
    GXSTDINTERFACE(GXHRESULT    CreateDepthStencilState    (GDepthStencilState** ppDepthStencilState, GXDEPTHSTENCILDESC* pState));
    GXSTDINTERFACE(GXHRESULT    CreateSamplerState         (GSamplerState** ppSamplerState));

    // GXImage
    // TODO: ��ʹ����ʧ��,Ҳ�᷵��һ��Ĭ��ͼƬ
    // CreateImage ֧�� TEXSIZE_* ��Ĳ���ָ��Image�Ŀ���߸�
    GXSTDINTERFACE(GXImage*    CreateImage                 (GXLONG nWidth, GXLONG nHeight, GXFormat eFormat, GXBOOL bRenderable, const GXLPVOID lpBits));
    GXSTDINTERFACE(GXImage*    CreateImageFromFile         (GXLPCWSTR lpwszFilename));
    GXSTDINTERFACE(GXImage*    CreateImageFromTexture      (GTexture* pTexture));

    // GXFTFont
    GXSTDINTERFACE(GXFont*    CreateFontIndirectW          (const GXLPLOGFONTW lpLogFont));
    GXSTDINTERFACE(GXFont*    CreateFontIndirectA          (const GXLPLOGFONTA lpLogFont));
    GXSTDINTERFACE(GXFont*    CreateFontW                  (GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXLPCWSTR pFileName));
    GXSTDINTERFACE(GXFont*    CreateFontA                  (GXCONST GXULONG nWidth, GXCONST GXULONG nHeight, GXLPCSTR pFileName));

    GXSTDINTERFACE(GXImage*   GetBackBufferImg             ());
    GXSTDINTERFACE(GTexture*  GetBackBufferTex             ());
    GXSTDINTERFACE(GTexture*  GetDeviceOriginTex           ());
    GXSTDINTERFACE(GXBOOL     ScrollTexture                (const SCROLLTEXTUREDESC* lpScrollTexDesc));

    GXSTDINTERFACE(GXBOOL     GetDesc                      (GXGRAPHICSDEVICE_DESC* pDesc));
    GXSTDINTERFACE(GXBOOL     GetRenderStatistics          (RENDER_STATISTICS* pStat));
    GXSTDINTERFACE(GXBOOL     ConvertToAbsolutePathW       (clStringW& strFilename));
    GXSTDINTERFACE(GXBOOL     ConvertToAbsolutePathA       (clStringA& strFilename));
    GXSTDINTERFACE(GXBOOL     ConvertToRelativePathW       (clStringW& strFilename));
    GXSTDINTERFACE(GXBOOL     ConvertToRelativePathA       (clStringA& strFilename));
    GXSTDINTERFACE(GXDWORD    GetCaps                      (GXGrapCapsCategory eCate));
    GXSTDINTERFACE(void       Enter                        ());
    GXSTDINTERFACE(void       Leave                        ());

    GXSTDINTERFACE(GXBOOL     SwitchConsole                ());

    // ���������ӿ� GRESCRIPTDESC ��bBroadcast��dwTime�ᱻ����
    GXSTDINTERFACE(GXHRESULT  BroadcastScriptCommand       (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(GXHRESULT  BroadcastCategoryCommand     (GXDWORD dwCategoryID, GRESCRIPTDESC* pDesc));
};

GXDLL GXBOOL IsPow2         (GXINT nNum);
GXDLL GXINT  GetAdaptedSize (GXINT nSize);

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_X_GRAPHICS_H_