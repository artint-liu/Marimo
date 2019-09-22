#ifndef _GRAPH_X_CANVAS_H_
#define _GRAPH_X_CANVAS_H_

class GRegion;
class GXFont;
struct GXSTATION;

namespace GrapX
{
  class Primitive;
  class Camera;
  class Texture;
  class RenderTarget;
  class Effect;

  //////////////////////////////////////////////////////////////////////////
  // GXCanvasCore 从GUnknown继承改为从GResource继承
  // 这是为了便于在Graphics中进行管理，通过消息分发得到诸如设备尺寸改变等系统信息。
  class CanvasCore : public GResource
  {
  public:
    CanvasCore(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType) {}
    //virtual ~GXCanvasCore() = NULL;

    GXSTDINTERFACE(GXHRESULT        AddRef              ());
    GXSTDINTERFACE(GXSIZE*          GetTargetDimension  (GXSIZE* pSize) const);
    GXSTDINTERFACE(Graphics*        GetGraphicsUnsafe   () const);
    GXSTDINTERFACE(RenderTarget*    GetTargetUnsafe     () const);
  };

  //////////////////////////////////////////////////////////////////////////

  enum CanvasParamInfo
  {
    CPI_SETTEXTURECOLOR = 1,    // uParam 设置纹理颜色
    CPI_SETCOLORADDITIVE = 2,   // uParam 设置颜色累加值
    CPI_SETTEXTCLIP = 3,        // 设置文字的裁剪, 只是临时改变设备的矩形裁剪区, pParam指向GXRECT,
                                // 如果使用任何裁剪函数设置, 将清除这个函数的效果
    //CPI_SETPIXELSIZEINV  = 4, // 像素尺寸的倒数, 相当于纹理坐标上一个像素的跨度, pParam 指向了float2结构
    //CPI_SETEXTTEXTURE = 5,      // 设置额外的纹理, uParam 是纹理的Stage, pParam 指向纹理对象
                                // 由于第一个纹理用于绘制图像, 所以0号位置不能设置
  };

  enum CompositingMode
  {
    CompositingMode_SourceOver,   // Alpha 合成
    CompositingMode_SourceCopy,   // 颜色直接覆盖
    CompositingMode_InvertTarget, // 目标反色
  };

  enum PenStyle
  {
    PS_Solid = 0,       // ______
    PS_Dash = 1,        // - - - -
    PS_Dot = 2,         // . . . .
    PS_DashDot = 3,     // - . - . -
    PS_DashDotDot = 4,  // - .. - .. -
  };

  enum RotateType
  {
    Rotate_None = 0,
    Rotate_CW90 = 1,
    Rotate_180 = 2,
    Rotate_CCW90 = 3,
    Rotate_FlipHorizontal = 4,
    Rotate_CW90_Flip = 5,
    Rotate_180_Flip = 6,
    Rotate_CCW90_Flip = 7,

    // 重命名已有的定义
    Rotate_CW270 = Rotate_CCW90,
    Rotate_CCW270 = Rotate_CW90,
    Rotate_FlipVertical = Rotate_180_Flip,
  };

  //
  // 对旋转标志的操作宏
  //

  // 获得翻转标志
#define CANVAS_GET_FLIP(_ROTATE) ((_ROTATE) & 4)

// 获得顺时针/逆时针旋转90度标志
#define CANVAS_GET_90(_ROTATE)  ((_ROTATE) & 1)

// 调整旋转中心
#define CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  int nDelta = (((_WIDTH) - (_HEIGHT)) >> 1);\
  if(((_ROTATE) & 1) == 0) {\
  nDelta = -nDelta;\
  }\
  _X += nDelta;\
  _Y -= nDelta;\
  }

// 顺时针旋转90度
#define CANVAS_ROTATE_90(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  const int nFlip = CANVAS_GET_FLIP(_ROTATE);\
  (_ROTATE)++;\
  (_ROTATE) = ((_ROTATE) & 3) | nFlip;\
  CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT);\
  }

// 顺时针旋转270度/逆时针90度
#define CANVAS_ROTATE_270(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  const int nFlip = CANVAS_GET_FLIP(_ROTATE);\
  (_ROTATE)--;\
  (_ROTATE) = ((_ROTATE) & 3) | nFlip;\
  CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT);\
  }

// 垂直翻转
#define CANVAS_FLIP_V(_ROTATE) {\
  int nRotate90 = CANVAS_GET_90(_ROTATE);\
  (_ROTATE) = nRotate90 ? ((_ROTATE) ^ 4) : ((_ROTATE) ^ (2|4));\
  }

// 水平翻转
#define CANVAS_FLIP_H(_ROTATE) {\
  int nRotate90 = CANVAS_GET_90(_ROTATE);\
  (_ROTATE) = nRotate90 ? ((_ROTATE) ^ (2|4)) : ((_ROTATE) ^ 4);\
  }

  class Canvas : public CanvasCore
  {
  public:
    //virtual ~GXCanvas() = NULL;
    Canvas(GXUINT nPriority, GXDWORD dwType) : CanvasCore(nPriority, dwType) {}

    GXSTDINTERFACE(GXHRESULT   Release              ());
    GXSTDINTERFACE(GXHRESULT   Invoke               (GRESCRIPTDESC* pDesc));

    GXSTDINTERFACE(GXBOOL      SetEffect            (Effect* pEffect));
    GXSTDINTERFACE(GXBOOL      SetTransform         (const float4x4* matTransform));
    GXSTDINTERFACE(GXBOOL      GetTransform         (float4x4* matTransform) const);
    GXSTDINTERFACE(GXBOOL      SetViewportOrg       (GXINT x, GXINT y, GXLPPOINT lpPoint)); // 设置当前的原点与新坐标系的映射关系
    GXSTDINTERFACE(GXBOOL      GetViewportOrg       (GXLPPOINT lpPoint) const);
    GXSTDINTERFACE(GXBOOL      Flush                ());
    GXSTDINTERFACE(GXBOOL      SetSamplerState      (GXUINT slot, GXSAMPLERDESC* pDesc));
    GXSTDINTERFACE(GXDWORD     SetParametersInfo    (CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam));
    GXSTDINTERFACE(PenStyle    SetPenStyle          (PenStyle eStyle));

    GXSTDINTERFACE(GXBOOL      Clear                (GXCOLORREF crClear));

    GXSTDINTERFACE(GXBOOL      SetPixel             (GXINT xPos, GXINT yPos, GXCOLORREF crPixel));
    GXSTDINTERFACE(GXBOOL      DrawLine             (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine));
    GXSTDINTERFACE(GXBOOL      DrawRectangle        (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect));
    GXSTDINTERFACE(GXBOOL      DrawRectangle        (GXLPCRECT lprc, GXCOLORREF crRect));
    GXSTDINTERFACE(GXBOOL      DrawRectangle        (GXLPCREGN lprg, GXCOLORREF crRect));
    GXSTDINTERFACE(GXBOOL      FillRectangle        (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill));
    GXSTDINTERFACE(GXBOOL      FillRectangle        (GXLPCRECT lprc, GXCOLORREF crFill));
    GXSTDINTERFACE(GXBOOL      FillRectangle        (GXLPCREGN lprg, GXCOLORREF crFill));

    GXSTDINTERFACE(GXBOOL      FillRegion           (GRegion* pRegion, GXCOLORREF crFill));

    GXSTDINTERFACE(GXBOOL      DrawUserPrimitive    (Texture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount));
    GXSTDINTERFACE(GXBOOL      DrawTexture          (Texture*pTexture, const GXREGN *rcDest));
    GXSTDINTERFACE(GXBOOL      DrawTexture          (Texture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc));
    GXSTDINTERFACE(GXBOOL      DrawTexture          (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc));
    GXSTDINTERFACE(GXBOOL      DrawTexture          (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation));

    GXSTDINTERFACE(GXINT       DrawText            (GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText));
    GXSTDINTERFACE(GXINT       DrawText            (GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText));
    GXSTDINTERFACE(GXBOOL      TextOut             (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCSTR lpString, GXINT cbString, GXCOLORREF crText));
    GXSTDINTERFACE(GXBOOL      TextOut             (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCWSTR lpString, GXINT cbString, GXCOLORREF crText));
    GXSTDINTERFACE(GXLONG      TabbedTextOut       (GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText));
    GXSTDINTERFACE(GXLONG      TabbedTextOut       (GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText)); // 如果颜色的Alpha是0则表示测量字符串尺寸

    GXSTDINTERFACE(CompositingMode SetCompositingMode   (CompositingMode eMode));
    GXSTDINTERFACE(CompositingMode GetCompositingMode   ());
    GXSTDINTERFACE(GXBOOL      SetRegion            (GRegion* pRegion, GXBOOL bAbsOrigin));
    GXSTDINTERFACE(GXBOOL      SetClipBox           (const GXLPRECT lpRect));
    GXSTDINTERFACE(GXINT       GetClipBox           (GXLPRECT lpRect));
    GXSTDINTERFACE(GXDWORD     GetStencilLevel      ());
    //GXSTDINTERFACE(GXBOOL      GetUniformData       (CANVASUNIFORM* pCanvasUniform));

    //GXSTDINTERFACE(GXBOOL      SetEffectUniformByName1f (const GXCHAR* pName, const float fValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniformByName2f (const GXCHAR* pName, const float2* vValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniformByName3f (const GXCHAR* pName, const float3* fValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniformByName4f (const GXCHAR* pName, const float4* fValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniformByName4x4(const GXCHAR* pName, const float4x4* pValue));

    //GXSTDINTERFACE(GXBOOL      SetEffectUniform1f       (const GXINT nIndex, const float fValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniform2f       (const GXINT nIndex, const float2* vValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniform3f       (const GXINT nIndex, const float3* fValue));
    //GXSTDINTERFACE(GXBOOL      SetEffectUniform4f       (const GXINT nIndex, const float4* fValue));

    GXSTDINTERFACE(GXBOOL      Scroll               (int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate));
  };
} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
#ifndef _DEV_DISABLE_UI_CODE
class GXDLL GXWndCanvas // TODO: 是否能改成从GXCanvas继承？
{
private:
  GrapX::Canvas*  m_pNative;
  GXHWND    m_hWnd;
  GXSTATION*    m_lpStation;
  GRegion*    m_pSystemRegion;    // Windows Manager 确定的 Region
  GRegion*    m_pUpdateRegion;    // 需要更新的 Region
  GRegion*    m_pClipRegion;      // 用户设定的 Region
  GRegion*    m_pEffectiveRegion; // 实际的, 有效的 Region, 不能为 NULL

  GXINT        UpdateRegion();
  GXLRESULT   Initialize  (GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags);
public:
  GXWndCanvas(GXHWND hWnd);
  GXWndCanvas(GXHWND hWnd, GRegion* pRegion);  // pRegion 屏幕空间
  GXWndCanvas(GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags);  // pRegion 屏幕空间
  virtual ~GXWndCanvas();

  GXBOOL    DrawFrameControl(GXLPRECT lprc,GXUINT uType,GXUINT uState);  
  GXHRESULT DrawTexture     (GrapX::Texture* pTexture, const GXREGN *rcDest);
  GXHRESULT DrawTexture     (GrapX::Texture* pTexture, const GXREGN *rcDest, const GXREGN *rcSrc);
  GXHRESULT DrawTexture     (GrapX::Texture* pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc);

  GXINT     DrawText        (GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText);
  GXINT     DrawGlowText    (GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF Color, GXUINT uRadius);
  GXBOOL    TextOut         (GXFont* pFTFont, GXINT nXStart,GXINT nYStart,GXLPCWSTR lpString,GXINT cbString, GXCOLORREF crText);

  GXVOID    DrawRect        (GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color);
  GXVOID    FillRect        (GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color);
  GXVOID    FillRect        (GXLPRECT lprect, GXCOLORREF Color);
  GXVOID    SetPixel        (GXINT xPos, GXINT yPos, GXCOLORREF Color);
  GXVOID    DrawLine        (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF Color);
  GXBOOL    SetViewportOrg  (GXINT x, GXINT y, GXLPPOINT lpPoint);
  GXINT     GetClipBox      (GXLPRECT lpRect);
  GXBOOL    GetPaintRect    (GXLPRECT lpRect);  // 获得可绘图的Rect区域

  GrapX::Canvas* GetCanvasUnsafe ();

  //void      EnableAlphaBlend(GXBOOL bEnable);

  static GXVOID GXGetRenderSurfaceExt (GXHWND GXIN hTopLevel, GXINT* GXOUT nWidth, GXINT* GXOUT nHeight);
  static GXVOID GXGetRenderingRect    (GXHWND GXIN hTopLevel, GXHWND GXIN hChild, GXDWORD GXIN flags, GXLPRECT GXOUT lprcOut);  //TODO: 需要整合到Canvas类中
};
#endif // #ifndef _DEV_DISABLE_UI_CODE

#else
#pragma message(__FILE__ ": warning : Duplicate included this file.")
#endif // _GRAPH_X_CANVAS_H_