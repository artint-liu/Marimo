#ifndef _GRAPH_X_CANVAS_H_
#define _GRAPH_X_CANVAS_H_

class GTexture;
class GRegion;
class GPrimitiveVI;
class GXFont;
class GXImage;
class GXGraphics;
class GCamera;
class GXEffect;
struct GXSTATION;

//////////////////////////////////////////////////////////////////////////
// GXCanvasCore ��GUnknown�̳и�Ϊ��GResource�̳�
// ����Ϊ�˱�����Graphics�н��й���ͨ����Ϣ�ַ��õ������豸�ߴ�ı��ϵͳ��Ϣ��
class GXCanvasCore : public GResource
{
public:
  GXCanvasCore(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType){}
  //virtual ~GXCanvasCore() = NULL;

  GXSTDINTERFACE(GXHRESULT    AddRef              ());
  GXSTDINTERFACE(GXVOID       GetTargetDimension  (GXSIZE* pSize) GXCONST);
  GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe   () GXCONST);
  GXSTDINTERFACE(GTexture*    GetTargetUnsafe     () GXCONST);
};

//////////////////////////////////////////////////////////////////////////

enum CanvasParamInfo
{
  CPI_SETTEXTURECOLOR  = 1,    // uParam ����������ɫ
  CPI_SETCOLORADDITIVE = 2,    // uParam ������ɫ�ۼ�ֵ
  CPI_SETTEXTCLIP      = 3,    // �������ֵĲü�, ֻ����ʱ�ı��豸�ľ��βü���, pParamָ��GXRECT,
                               // ���ʹ���κβü���������, ��������������Ч��
  //CPI_SETPIXELSIZEINV  = 4,  // ���سߴ�ĵ���, �൱������������һ�����صĿ��, pParam ָ����float2�ṹ
  CPI_SETEXTTEXTURE    = 5,    // ���ö��������, uParam �������Stage, pParam ָ���������
                               // ���ڵ�һ���������ڻ���ͼ��, ����0��λ�ò�������
};
enum CompositingMode
{
  CM_SourceOver,
  CM_SourceCopy,
};

enum PenStyle
{
  PS_Solid      = 0,  // ______
  PS_Dash       = 1,  // - - - -
  PS_Dot        = 2,  // . . . .
  PS_DashDot    = 3,  // - . - . -
  PS_DashDotDot = 4,  // - .. - .. -
};

enum RotateType
{
  Rotate_None           = 0,
  Rotate_CW90           = 1,
  Rotate_180            = 2,
  Rotate_CCW90          = 3,
  Rotate_FlipHorizontal = 4,
  Rotate_CW90_Flip      = 5,
  Rotate_180_Flip       = 6,
  Rotate_CCW90_Flip     = 7,

  // ���������еĶ���
  Rotate_CW270        = Rotate_CCW90,
  Rotate_CCW270       = Rotate_CW90,
  Rotate_FlipVertical = Rotate_180_Flip,
};

//
// ����ת��־�Ĳ�����
//

// ��÷�ת��־
#define CANVAS_GET_FLIP(_ROTATE) ((_ROTATE) & 4)

// ���˳ʱ��/��ʱ����ת90�ȱ�־
#define CANVAS_GET_90(_ROTATE)  ((_ROTATE) & 1)

// ������ת����
#define CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  int nDelta = (((_WIDTH) - (_HEIGHT)) >> 1);\
  if(((_ROTATE) & 1) == 0) {\
  nDelta = -nDelta;\
  }\
  _X += nDelta;\
  _Y -= nDelta;\
  }

// ˳ʱ����ת90��
#define CANVAS_ROTATE_90(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  const int nFlip = CANVAS_GET_FLIP(_ROTATE);\
  (_ROTATE)++;\
  (_ROTATE) = ((_ROTATE) & 3) | nFlip;\
  CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT);\
  }

// ˳ʱ����ת270��/��ʱ��90��
#define CANVAS_ROTATE_270(_ROTATE, _X, _Y, _WIDTH, _HEIGHT) {\
  const int nFlip = CANVAS_GET_FLIP(_ROTATE);\
  (_ROTATE)--;\
  (_ROTATE) = ((_ROTATE) & 3) | nFlip;\
  CANVAS_ROTATE_AROUND_CENTER(_ROTATE, _X, _Y, _WIDTH, _HEIGHT);\
  }

// ��ֱ��ת
#define CANVAS_FLIP_V(_ROTATE) {\
  int nRotate90 = CANVAS_GET_90(_ROTATE);\
  (_ROTATE) = nRotate90 ? ((_ROTATE) ^ 4) : ((_ROTATE) ^ (2|4));\
  }

// ˮƽ��ת
#define CANVAS_FLIP_H(_ROTATE) {\
  int nRotate90 = CANVAS_GET_90(_ROTATE);\
  (_ROTATE) = nRotate90 ? ((_ROTATE) ^ (2|4)) : ((_ROTATE) ^ 4);\
  }

class GXCanvas : public GXCanvasCore
{
public:

public:
  //virtual ~GXCanvas() = NULL;
  GXCanvas(GXUINT nPriority, GXDWORD dwType) : GXCanvasCore(nPriority, dwType){}

  GXSTDINTERFACE(GXHRESULT   Release              ());
  GXSTDINTERFACE(GXHRESULT   Invoke               (GRESCRIPTDESC* pDesc));

  GXSTDINTERFACE(GXBOOL      SetTransform         (const float4x4* matTransform));
  GXSTDINTERFACE(GXBOOL      GetTransform         (float4x4* matTransform) GXCONST);
  GXSTDINTERFACE(GXBOOL      SetViewportOrg       (GXINT x, GXINT y, GXLPPOINT lpPoint));
  GXSTDINTERFACE(GXBOOL      GetViewportOrg       (GXLPPOINT lpPoint) GXCONST);
  GXSTDINTERFACE(GXVOID      EnableAlphaBlend     (GXBOOL bEnable));
  GXSTDINTERFACE(GXBOOL      Flush                ());
  GXSTDINTERFACE(GXBOOL      SetSamplerState      (GXUINT Sampler, GXSAMPLERDESC* pDesc));
  GXSTDINTERFACE(GXBOOL      SetRenderState       (GXRenderStateType eType, GXDWORD dwValue));
  GXSTDINTERFACE(GXBOOL      SetRenderStateBlock  (GXLPCRENDERSTATE lpBlock));
  GXSTDINTERFACE(GXBOOL      SetEffect            (GXEffect* pEffect));
  GXSTDINTERFACE(GXBOOL      SetEffectConst       (GXLPCSTR lpName, void* pData, int nPackCount));  // ��ʱ
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
  GXSTDINTERFACE(GXBOOL      InvertRect           (GXINT x, GXINT y, GXINT w, GXINT h));

  GXSTDINTERFACE(GXBOOL      ColorFillRegion      (GRegion* pRegion, GXCOLORREF crFill));

  GXSTDINTERFACE(GXBOOL      DrawUserPrimitive    (GTexture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount));
  GXSTDINTERFACE(GXBOOL      DrawTexture          (GTexture*pTexture, const GXREGN *rcDest));
  GXSTDINTERFACE(GXBOOL      DrawTexture          (GTexture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc));
  GXSTDINTERFACE(GXBOOL      DrawTexture          (GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc));
  GXSTDINTERFACE(GXBOOL      DrawTexture          (GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation));

  GXSTDINTERFACE(GXBOOL      DrawImage            (GXImage*pImage, const GXREGN* rgDest));
  GXSTDINTERFACE(GXBOOL      DrawImage            (GXImage*pImage, GXINT xPos, GXINT yPos, const GXREGN* rgSrc));
  GXSTDINTERFACE(GXBOOL      DrawImage            (GXImage*pImage, const GXREGN* rgDest, const GXREGN* rgSrc));
  GXSTDINTERFACE(GXBOOL      DrawImage            (GXImage*pImage, const GXREGN* rgDest, const GXREGN* rgSrc, RotateType eRotation));

  GXSTDINTERFACE(GXINT       DrawTextA            (GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText));
  GXSTDINTERFACE(GXINT       DrawTextW            (GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText));
  GXSTDINTERFACE(GXBOOL      TextOutA             (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCSTR lpString, GXINT cbString, GXCOLORREF crText));
  GXSTDINTERFACE(GXBOOL      TextOutW             (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCWSTR lpString, GXINT cbString, GXCOLORREF crText));
  GXSTDINTERFACE(GXLONG      TabbedTextOutA       (GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText));
  GXSTDINTERFACE(GXLONG      TabbedTextOutW       (GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText)); // �����ɫ��Alpha��0���ʾ�����ַ����ߴ�

  GXSTDINTERFACE(GXINT       SetCompositingMode   (CompositingMode eMode));
  GXSTDINTERFACE(GXBOOL      SetRegion            (GRegion* pRegion, GXBOOL bAbsOrigin));
  GXSTDINTERFACE(GXBOOL      SetClipBox           (const GXLPRECT lpRect));
  GXSTDINTERFACE(GXINT       GetClipBox           (GXLPRECT lpRect));
  GXSTDINTERFACE(GXDWORD     GetStencilLevel      ());
  GXSTDINTERFACE(GXBOOL      GetUniformData       (CANVASUNIFORM* pCanvasUniform));

  GXSTDINTERFACE(GXBOOL      SetEffectUniformByName1f (const GXCHAR* pName, const float fValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniformByName2f (const GXCHAR* pName, const float2* vValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniformByName3f (const GXCHAR* pName, const float3* fValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniformByName4f (const GXCHAR* pName, const float4* fValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniformByName4x4(const GXCHAR* pName, const float4x4* pValue));

  GXSTDINTERFACE(GXBOOL      SetEffectUniform1f       (const GXINT nIndex, const float fValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniform2f       (const GXINT nIndex, const float2* vValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniform3f       (const GXINT nIndex, const float3* fValue));
  GXSTDINTERFACE(GXBOOL      SetEffectUniform4f       (const GXINT nIndex, const float4* fValue));

  GXSTDINTERFACE(GXBOOL      Scroll               (int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate));
};

//////////////////////////////////////////////////////////////////////////
#ifndef _DEV_DISABLE_UI_CODE
class GXDLL GXWndCanvas
{
private:
  GXCanvas*  m_pNative;
  GXHWND    m_hWnd;
  GXSTATION*    m_lpStation;
  GRegion*    m_pSystemRegion;    // Windows Manager ȷ���� Region
  GRegion*    m_pUpdateRegion;    // ��Ҫ���µ� Region
  GRegion*    m_pClipRegion;      // �û��趨�� Region
  GRegion*    m_pEffectiveRegion; // ʵ�ʵ�, ��Ч�� Region, ����Ϊ NULL

  GXINT        UpdateRegion();
  GXLRESULT   Initialize  (GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags);
public:
  GXWndCanvas(GXHWND hWnd);
  GXWndCanvas(GXHWND hWnd, GRegion* pRegion);  // pRegion ��Ļ�ռ�
  GXWndCanvas(GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags);  // pRegion ��Ļ�ռ�
  virtual ~GXWndCanvas();

  GXBOOL    DrawFrameControl(GXLPRECT lprc,GXUINT uType,GXUINT uState);  
  GXHRESULT DrawImage       (GXImage*pImage, const GXREGN *rcDest);
  GXHRESULT DrawImage       (GXImage*pImage, const GXREGN *rcDest, const GXREGN *rcSrc);
  GXHRESULT DrawImage       (GXImage*pImage, GXINT xPos, GXINT yPos, const GXREGN *rcSrc);

  GXINT     DrawText        (GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText);
  GXINT     DrawGlowText    (GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF Color, GXUINT uRadius);
  GXBOOL    TextOutW        (GXFont* pFTFont, GXINT nXStart,GXINT nYStart,GXLPCWSTR lpString,GXINT cbString, GXCOLORREF crText);

  GXVOID    DrawRect        (GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color);
  GXVOID    FillRect        (GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color);
  GXVOID    FillRect        (GXLPRECT lprect, GXCOLORREF Color);
  GXVOID    InvertRect      (GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight);
  GXVOID    SetPixel        (GXINT xPos, GXINT yPos, GXCOLORREF Color);
  GXVOID    DrawLine        (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF Color);
  GXBOOL    SetViewportOrg  (GXINT x, GXINT y, GXLPPOINT lpPoint);
  GXINT     GetClipBox      (GXLPRECT lpRect);
  GXBOOL    GetPaintRect    (GXLPRECT lpRect);  // ��ÿɻ�ͼ��Rect����

  GXCanvas* GetCanvasUnsafe ();

  void      EnableAlphaBlend(GXBOOL bEnable);

  static GXVOID GXGetRenderSurfaceExt (GXHWND GXIN hTopLevel, GXINT* GXOUT nWidth, GXINT* GXOUT nHeight);
  static GXVOID GXGetRenderingRect    (GXHWND GXIN hTopLevel, GXHWND GXIN hChild, GXDWORD GXIN flags, GXLPRECT GXOUT lprcOut);  //TODO: ��Ҫ���ϵ�Canvas����
};
#endif // #ifndef _DEV_DISABLE_UI_CODE

#else
#pragma message(__FILE__ ": warning : Duplicate included this file.")
#endif // _GRAPH_X_CANVAS_H_