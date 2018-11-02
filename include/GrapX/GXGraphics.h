#ifndef _GRAPH_X_GRAPHICS_H_
#define _GRAPH_X_GRAPHICS_H_

//class GXImage;
class GRegion;
class GTextureBase;
class GTexture;
class GTexture3D;
class GTextureCube;
class GPrimitive;
//class GPrimitiveV;
//class GPrimitiveVI;
class GXGraphics;
class GXRenderTarget;
class GXFont;
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

// GXGRAPHICSPARAM 的标志位
//enum GXGRAPHICSPARAMFLAG
//{
//    GPF_VSYNC    = 0x00000001,    // 垂直同步
//    GPF_SIZEABLE = 0x00000002,    // 可改变窗口的大小尺寸
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
//    GXUINT                cbSize;                // 结构体大小, 用来区分后续的版本
//    GXUINT                BackBufferWidth;    // 后台缓冲的宽度
//    GXUINT                BackBufferHeight;    // 后台缓冲的高度
//    D3DFORMAT           BackBufferFormat;    // 后台缓冲的格式
//    GXUINT                BackBufferCount;    // 后台缓冲数量
//    D3DFORMAT           DepthStencilFormat;    // 深度缓冲格式, 这个是一定要创建的
//    GXDWORD                dwFlags;            // 标志位, 参考 GXGRAPHICSPARAMFLAG
//};

// GXImage 和 GTexture 的关系:
// 1. GTexture 直接引用D3DTexture, GXImage引用 GTexture
// 2. GTexture 是创建的原始纹理, 由文件创建时不会被更改, GXImage 由文件创建时,如果被作为绘图目标出现,
//        则复制原来的问题,重新创建为RenderTarget属性的GTexture
// 3. GTexture 偏向于效率, GXImage偏向于实用

// 将来的命名规则,GX---面向高级函数, G---面向D3D的直接函数,GXGraphics除外

//////////////////////////////////////////////////////////////////////////
class GXGraphics : public GResource
{
public:
    GXGraphics() : GResource(0, RESTYPE_GRAPHICS){}
    // 接口实现
    GXSTDINTERFACE(GXHRESULT AddRef());
    GXSTDINTERFACE(GXHRESULT Release());

    // 设备事件
    GXSTDINTERFACE(GXHRESULT Invoke             (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(void      GetPlatformID      (GXPlaformIdentity* pIdentity));


    // 激活函数,之后的所有D3D操作必须由Graphics接口实现
    GXSTDINTERFACE(GXBOOL    Activate           (GXBOOL bActive));
    GXSTDINTERFACE(GXHRESULT Begin              ());
    GXSTDINTERFACE(GXHRESULT End                ());
    GXSTDINTERFACE(GXHRESULT Present            ());

    GXSTDINTERFACE(GXHRESULT Resize             (int nWidth, int nHeight));

  //GXSTDINTERFACE(GXHRESULT RegisterResource   (GResource* pResource));
    GXSTDINTERFACE(GXHRESULT RegisterResource   (GResource* pResource, LPCRESKETCH pSketch = NULL));
    GXSTDINTERFACE(GXHRESULT UnregisterResource (GResource* pResource));


    GXSTDINTERFACE(GXHRESULT SetPrimitive       (GPrimitive* pPrimitive, GXUINT uStreamSource = 0));
    //GXSTDINTERFACE(GXHRESULT SetPrimitiveV      (GPrimitiveV* pPrimitive, GXUINT uStreamSource = 0));
    //GXSTDINTERFACE(GXHRESULT SetPrimitiveVI     (GPrimitiveVI* pPrimitive, GXUINT uStreamSource = 0));

    // 理论上GXGraphics没有 Set** 类函数, SetTexture 例外, 因为 SetTexture 同时肩负这清空指定设备纹理的任务
    GXSTDINTERFACE(GXHRESULT SetTexture           (GTextureBase* pTexture, GXUINT uStage = 0));
    GXSTDINTERFACE(GXHRESULT SetRasterizerState   (GRasterizerState* pRasterizerState));
    GXSTDINTERFACE(GXHRESULT SetBlendState        (GBlendState* pBlendState));
    GXSTDINTERFACE(GXHRESULT SetDepthStencilState (GDepthStencilState* pDepthStencilState));
    GXSTDINTERFACE(GXHRESULT SetSamplerState      (GSamplerState* pSamplerState));

    GXSTDINTERFACE(GXHRESULT Clear              (const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil));
    GXSTDINTERFACE(GXHRESULT DrawPrimitive      (const GXPrimitiveType eType, const GXUINT StartVertex, const GXUINT PrimitiveCount));
    GXSTDINTERFACE(GXHRESULT DrawPrimitive      (const GXPrimitiveType eType, const GXINT BaseVertexIndex, const GXUINT MinIndex, const GXUINT NumVertices, const GXUINT StartIndex, const GXUINT PrimitiveCount));

    //////////////////////////////////////////////////////////////////////////
    // 低级函数
    // GTexture

    // CreateTexture 支持 TEXSIZE_* 族的参数作为纹理的宽或高.
    // 如果szName不为NULL, 返回值为0时表示第一次创建, >1时是重复资源被引用的次数

    // 2D Texture
    GXSTDINTERFACE(GXHRESULT CreateTexture(
      GTexture**  ppTexture,      // 返回的纹理指针
      GXLPCSTR    szName,         // 资源名字, 如果为NULL, 则默认私有名字, 否则按照用户指定命名, 
                                  // 如果已存在该名字,则直接返回对象,引用加一, 后面的参数将被忽略
      GXUINT      Width,          // 宽度和高度
      GXUINT      Height,
      GXFormat    Format,
      GXResUsage  eResUsage,
      GXUINT      MipLevels = 0, 
      GXLPCVOID   pInitData = NULL,
      GXUINT      nPitch = 0));   // 0表示使用默认pitch（nWidth*像素字节数）

    GXSTDINTERFACE(GXHRESULT CreateTexture(GTexture** ppTexture, GXLPCSTR szName, GXResUsage eUsage, GTexture* pSourceTexture));

    // eUsage参数只有在首次创建时限定纹理的使用行为，对于重复创建的纹理文件，eUsage作用会被忽略
    GXSTDINTERFACE(GXHRESULT CreateTextureFromMemory(GTexture** ppTexture, GXLPCWSTR szName, clstd::Buffer* pBuffer, GXResUsage eUsage));
    GXSTDINTERFACE(GXHRESULT CreateTextureFromFile  (GTexture** ppTexture, GXLPCWSTR szFilePath, GXResUsage eUsage));

    //GXSTDINTERFACE(GXHRESULT CreateTextureFromFileEx(
    //  GTexture**  ppTexture, 
    //  GXLPCWSTR   pSrcFile, 
    //  GXUINT      Width, 
    //  GXUINT      Height, 
    //  GXUINT      MipLevels, 
    //  GXFormat    Format, 
    //  GXResUsage  ResUsage, 
    //  GXDWORD     Filter = GXFILTER_NONE,
    //  GXDWORD     MipFilter = GXFILTER_NONE, 
    //  GXCOLORREF  ColorKey = 0,
    //  GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // Volume Texture
    GXSTDINTERFACE(GXHRESULT CreateTexture3D(
      GTexture3D**  ppTexture, 
      GXLPCSTR      szName,
      GXUINT        Width,          // 宽度,高度和深度
      GXUINT        Height,
      GXUINT        Depth,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage));

    GXSTDINTERFACE(GXHRESULT CreateTexture3DFromFile(
      GTexture3D**  ppTexture,
      GXLPCWSTR     pSrcFile));

    //GXSTDINTERFACE(GXHRESULT CreateTexture3DFromFileEx(
    //  GTexture3D**  ppTexture,
    //  GXLPCWSTR     pSrcFile,
    //  GXUINT        Width,
    //  GXUINT        Height,
    //  GXUINT        Depth,
    //  GXUINT        MipLevels, 
    //  GXFormat      Format, 
    //  GXDWORD       ResUsage, 
    //  GXDWORD       Filter = GXFILTER_NONE,
    //  GXDWORD       MipFilter = GXFILTER_NONE, 
    //  GXCOLORREF    ColorKey = 0,
    //  GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // Cube Texture
    GXSTDINTERFACE(GXHRESULT CreateTextureCube(
      GTextureCube**ppTexture, 
      GXLPCSTR      szName,
      GXUINT        Size,
      GXUINT        MipLevels, 
      GXFormat      Format, 
      GXDWORD       ResUsage));

    GXSTDINTERFACE(GXHRESULT CreateTextureCubeFromFile(
      GTextureCube**ppTexture,
      GXLPCWSTR     pSrcFile));

    //GXSTDINTERFACE(GXHRESULT CreateTextureCubeFromFileEx(
    //  GTextureCube**ppTexture,
    //  GXLPCWSTR     pSrcFile,
    //  GXUINT        Size,
    //  GXUINT        MipLevels, 
    //  GXFormat      Format, 
    //  GXDWORD       ResUsage, 
    //  GXDWORD       Filter = GXFILTER_NONE,
    //  GXDWORD       MipFilter = GXFILTER_NONE, 
    //  GXCOLORREF    ColorKey = 0,
    //  GXOUT LPGXIMAGEINFOX pSrcInfo = NULL));

    // GPrimitive
    GXSTDINTERFACE(GXHRESULT CreatePrimitive(
      GPrimitive**        pPrimitive,             // 返回对象
      GXLPCSTR            szName,                 // 资源名, NULL为匿名对象, 具名对象仅在第一次创建时使用下面的参数
      GXLPCVERTEXELEMENT  pVertexDecl,            // 顶点声明
      GXResUsage          eResUsage,              // 资源类型/属性
      GXUINT              uVertexCount,           // 顶点数
      GXUINT              uVertexStride = 0,      // 每个顶点的字节数
      GXLPCVOID           pVertInitData = NULL,   // 初始化数据
      GXUINT              uIndexCount = 0,        // 索引数量
      GXUINT              uIndexSize = 2,         // 每个索引的字节数, 2或者4
      GXLPCVOID           pIndexInitData = NULL   // 索引初始化数据
    ));

    //GXSTDINTERFACE(GXHRESULT CreatePrimitiveV(
    //  GPrimitiveV**       pPrimitive,           // 返回对象
    //  GXLPCSTR            szName,               // 资源名, NULL为匿名对象, 具名对象仅在第一次创建时使用下面的参数
    //  LPCGXVERTEXELEMENT  pVertexDecl,          // 顶点声明
    //  GXDWORD             ResUsage,             // 资源类型/属性
    //  GXUINT              uVertexCount,         // 顶点数
    //  GXUINT              uVertexStride = 0,    // 每个顶点的字节数
    //  GXLPVOID            pVertInitData = NULL  // 初始化数据
    //  ));

    //GXSTDINTERFACE(GXHRESULT CreatePrimitiveVI(
    //  GPrimitiveVI**      pPrimitive, 
    //  GXLPCSTR            szName, 
    //  LPCGXVERTEXELEMENT  pVertexDecl,
    //  GXDWORD             ResUsage, 
    //  GXUINT              uIndexCount,          // 索引数量
    //  GXUINT              uVertexCount,
    //  GXUINT              uVertexStride = 0,
    //  GXLPCVOID           pIdxInitData = NULL,  // 索引初始化数据
    //  GXLPCVOID           pVertInitData = NULL  // 顶点和索引初始化数据必须同时有效或者同时为NULL.
    //  ));

    // GSahder
    GXSTDINTERFACE(GXHRESULT    CreateShaderFromSource      (GShader** ppShader, GXLPCSTR szShaderSource, size_t nSourceLen, GXDEFINITION* pMacroDefinition)); // nSourceLen是字符长度，如果是0，szShaderSource必须以'\0'结尾
    GXSTDINTERFACE(GXHRESULT    CreateShaderFromFile        (GShader** ppShader, GXLPCWSTR szShaderDesc));
    GXSTDINTERFACE(GXHRESULT    CreateShaderFromFile        (GShader** ppShader, GXLPCSTR szShaderDesc));
    //GXSTDINTERFACE(GXHRESULT    CreateShader                (GShader** ppShader, MOSHADER_ELEMENT_SOURCE* pSdrElementSrc));
    GXSTDINTERFACE(GXHRESULT    CreateShaderStub            (GShaderStub** ppShaderStub));


    // GRegion
    GXSTDINTERFACE(GXHRESULT    CreateRectRgn              (GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom));
    GXSTDINTERFACE(GXHRESULT    CreateRectRgnIndirect      (GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount));
    GXSTDINTERFACE(GXHRESULT    CreateRoundRectRgn         (GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse));

    GXSTDINTERFACE(GXHRESULT    CreateVertexDeclaration    (GVertexDeclaration** ppVertexDecl, LPCGXVERTEXELEMENT lpVertexElement));
    //////////////////////////////////////////////////////////////////////////
    // 高级函数

    // GXCanvas
    GXSTDINTERFACE(GXCanvas*    LockCanvas                 (GXRenderTarget* pTarget, const LPREGN lpRegn, GXDWORD dwFlags));

    // 如果 pImage 为 NULL, 则忽略 DepthStencil 参数
    GXSTDINTERFACE(GXHRESULT    CreateCanvas3D             (GXCanvas3D** ppCanvas3D, GXRenderTarget* pTarget, LPCREGN lpRegn = NULL, float fNear = 0.0f, float fFar = 1.0f));
    //GXSTDINTERFACE(GXHRESULT    CreateCanvas3D             (GXCanvas3D** ppCanvas3D, GXRenderTarget* pTarget, GTexture* pDepthStencil, LPCREGN lpRegn = NULL, float fNear = 0.0f, float fFar = 1.0f));

    GXSTDINTERFACE(GXHRESULT    CreateEffect               (GXEffect** ppEffect, GShader* pShader));
    GXSTDINTERFACE(GXHRESULT    CreateMaterial             (GXMaterialInst** ppMtlInst, GShader* pShader));
    GXSTDINTERFACE(GXHRESULT    CreateMaterialFromFile     (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType));
    //GXSTDINTERFACE(GXHRESULT    CreateMaterialFromFile     (GXMaterialInst** ppMtlInst, GXLPCWSTR szShaderDesc, MtlLoadType eLoadType));

    GXSTDINTERFACE(GXHRESULT    CreateRasterizerState      (GRasterizerState** ppRasterizerState, GXRASTERIZERDESC* pRazDesc));
    GXSTDINTERFACE(GXHRESULT    CreateBlendState           (GBlendState** ppBlendState, GXBLENDDESC* pState, GXUINT nNum));
    GXSTDINTERFACE(GXHRESULT    CreateDepthStencilState    (GDepthStencilState** ppDepthStencilState, GXDEPTHSTENCILDESC* pState));
    GXSTDINTERFACE(GXHRESULT    CreateSamplerState         (GSamplerState** ppSamplerState));

    // GXImage
    // TODO: 即使创建失败,也会返回一个默认图片
    // CreateImage 支持 TEXSIZE_* 族的参数指定Image的宽或者高
    //GXSTDINTERFACE(GXImage*    CreateImage                 (GXLONG nWidth, GXLONG nHeight, GXFormat eFormat, GXBOOL bRenderable, const GXLPVOID lpBits));
    //GXSTDINTERFACE(GXImage*    CreateImageFromFile         (GXLPCWSTR lpwszFilename));
    //GXSTDINTERFACE(GXImage*    CreateImageFromTexture      (GTexture* pTexture));
    GXSTDINTERFACE(GXHRESULT  CreateRenderTarget           (GXRenderTarget** ppRenderTarget, GXLPCWSTR szName, GXINT nWidth, GXINT nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat));
    GXSTDINTERFACE(GXHRESULT  CreateRenderTarget           (GXRenderTarget** ppRenderTarget, GXLPCWSTR szName, GXSizeRatio nWidth, GXINT nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat));
    GXSTDINTERFACE(GXHRESULT  CreateRenderTarget           (GXRenderTarget** ppRenderTarget, GXLPCWSTR szName, GXINT nWidth, GXSizeRatio nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat));
    GXSTDINTERFACE(GXHRESULT  CreateRenderTarget           (GXRenderTarget** ppRenderTarget, GXLPCWSTR szName, GXSizeRatio nWidth, GXSizeRatio nHeight, GXFormat eColorFormat, GXFormat eDepthStencilFormat));

    // GXFTFont
    GXSTDINTERFACE(GXFont*    CreateFontIndirect          (const GXLPLOGFONTW lpLogFont));
    GXSTDINTERFACE(GXFont*    CreateFontIndirect          (const GXLPLOGFONTA lpLogFont));
    GXSTDINTERFACE(GXFont*    CreateFont                  (const GXULONG nWidth, const GXULONG nHeight, GXLPCWSTR pFileName));
    GXSTDINTERFACE(GXFont*    CreateFont                  (const GXULONG nWidth, const GXULONG nHeight, GXLPCSTR pFileName));

    //GXSTDINTERFACE(GXImage*   GetBackBufferImg             ());
    //GXSTDINTERFACE(GTexture*  GetBackBufferTex             ());
    GXSTDINTERFACE(GXHRESULT  GetBackBuffer                (GXRenderTarget** ppTarget));
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

    // 下面两个接口 GRESCRIPTDESC 中bBroadcast和dwTime会被设置
    GXSTDINTERFACE(GXHRESULT  BroadcastScriptCommand       (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(GXHRESULT  BroadcastCategoryCommand     (GXDWORD dwCategoryID, GRESCRIPTDESC* pDesc));
};

GXDLL GXBOOL IsPow2         (GXINT nNum);
GXDLL GXINT  GetAdaptedSize (GXINT nSize);

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPH_X_GRAPHICS_H_