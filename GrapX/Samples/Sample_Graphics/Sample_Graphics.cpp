// TestGXGraphics.cpp : 定义应用程序的入口点。
//

//#include "stdafx.h"
//#include <Include/Marimo.H>
// 如果必须将位于下面指定平台之前的平台作为目标，请修改下列定义。
// 有关不同平台对应值的最新信息，请参考 MSDN。
//#ifndef WINVER        // 允许使用特定于 Windows XP 或更高版本的功能。
//#define WINVER 0x0501    // 将此值更改为相应的值，以适用于 Windows 的其他版本。
//#endif
//
//#ifndef _WIN32_WINNT    // 允许使用特定于 Windows XP 或更高版本的功能。
//#define _WIN32_WINNT 0x0501  // 将此值更改为相应的值，以适用于 Windows 的其他版本。
//#endif            
//
//#ifndef _WIN32_WINDOWS    // 允许使用特定于 Windows 98 或更高版本的功能。
//#define _WIN32_WINDOWS 0x0410 // 将此值更改为适当的值，以指定将 Windows Me 或更高版本作为目标。
//#endif
//
//#ifndef _WIN32_IE      // 允许使用特定于 IE 6.0 或更高版本的功能。
//#define _WIN32_IE 0x0600  // 将此值更改为相应的值，以适用于 IE 的其他版本。
//#endif
//

#include <tchar.h>

#include <GrapX.H>
#include <GXApp.H>
#include <GrapX/GResource.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GTexture.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXImage.H>
#include <GrapX/GXFont.H>
#include <GrapX/GXCanvas.H>
#include <GrapX/gxDevice.H>
#include "Sample_Graphics.h"
#define MAX_LOADSTRING 100
//#include "Resource.h"
#include <clPathFile.H>

using namespace std;
//#include "vld.h"


// 全局变量:
//HINSTANCE hInst;                // 当前实例
//TCHAR szTitle[MAX_LOADSTRING];        // 标题栏文本
//TCHAR szWindowClass[MAX_LOADSTRING];      // 主窗口类名

GTexture* g_pPatternTex;
GTexture* g_pImage0;
GTexture* g_pImage1;
//HWND g_hWnd;


//void Test1(RECT& rect);
//void Test2(RECT& rect);
//void Test3(RECT& rect);
//void OnPaint(HWND hWnd);
b32 SplitPathA(const clStringW& strPath, clStringW* pstrDir, clStringW* pstrFile)
{
  size_t nPos = strPath.ReverseFind(_T('\\'));
  if(nPos == clStringW::npos)
  {
    if(pstrDir != NULL)
      pstrDir->Clear();
    if(pstrFile != NULL)
      (*pstrFile) = strPath;
    return FALSE;
  }
  else if(nPos == strPath.GetLength() - 1)
  {
    ASSERT(0); // 验证后去掉
    if(pstrDir != NULL)
      (*pstrDir) = strPath;
    if(pstrFile != NULL)
      pstrFile->Clear();
    return FALSE;
  }
  else
  {
    if(pstrDir != NULL)
      (*pstrDir) = strPath.SubString(0, nPos + 1);
    if(pstrFile != NULL)
      (*pstrFile) = strPath.SubString(nPos + 1, strPath.GetLength());
    
  }
  return TRUE;
}


class MyGraphicsTest : public GXApp
{
  GXFont*     m_pFont;
  GXFont*     m_pFontS;
  GXImage*    m_pTarget;
  GXImage*    m_pTarget2;
  GXImage*    m_pImageNonPow2;
  GTexture*   m_pTestIcon;
  GTexture*   m_pEmptyTex;
  GTexture*   m_pTexture;
  GRegion*    m_pRegion;
  GRegion*    m_pRegion2;
  int         m_nPage;
  int         m_nSubPage;
  float       m_fTime;
  GXDWORD     m_dwTick;
  GXPOINT     m_ptAction;
  GXBOOL      m_bAction;
public:
  MyGraphicsTest()
  : m_pFont         (NULL)
  , m_pFontS        (NULL)
  , m_pTarget       (NULL)
  , m_pTarget2      (NULL)
  , m_pEmptyTex     (NULL)
  , m_pImageNonPow2 (NULL)
  , m_pTexture      (NULL)
  , m_pRegion       (NULL)
  , m_pRegion2      (NULL)
  , m_nPage         (0)
  , m_nSubPage      (0)
  , m_fTime         (0.0f)
  , m_dwTick        (0)
  , m_bAction       (FALSE)
  {

  }
  virtual GXHRESULT OnCreate()
  {
    m_nPage = 0;
    m_nSubPage = 0;
    m_pGraphics->CreateTextureFromFileW(&m_pTexture, L"textures/AOX.png");
    m_pGraphics->CreateTextureFromFileW(&m_pTestIcon, L"textures/AOX.png");
    //m_pGraphics->CreateTextureFromFile(&g_pTexture, L"RGB.bmp");
    m_pFont = m_pGraphics->CreateFontA(NULL, 48, "fonts/wqy-microhei.ttc");
    m_pFontS = m_pGraphics->CreateFontA(NULL, 16, "fonts/wqy-microhei.ttc");
    m_pTarget = m_pGraphics->CreateImage(TEXSIZE_HALF, TEXSIZE_HALF, GXFMT_A8R8G8B8, TRUE, NULL);
    m_pTarget2 = m_pGraphics->CreateImage(TEXSIZE_HALF, TEXSIZE_HALF, GXFMT_A8R8G8B8, TRUE, NULL);
    m_pImageNonPow2 = m_pGraphics->CreateImageFromFile(L"nonpow2.png");
    m_pGraphics->CreateTexture(&m_pEmptyTex, NULL, TEXSIZE_HALF, TEXSIZE_HALF, 1, GXFMT_A8R8G8B8, GXRU_DEFAULT);
    m_pEmptyTex->Clear(NULL, 0xff000000);
//    m_pTarget->GetTextureUnsafe()->Clear(NULL, -1);

    GXRECT rect1(40, 40, 404, 265);
    GXRECT rect2(262, 184, 675, 437);
    GRegion* prgnTemp;
    m_pGraphics->CreateRoundRectRgn(&m_pRegion, rect1, 10, 10);
    m_pGraphics->CreateRoundRectRgn(&prgnTemp, rect2, 10, 10);
    m_pRegion->Union(prgnTemp);
    SAFE_RELEASE(prgnTemp);

    m_pGraphics->CreateRectRgn(&prgnTemp, 146, 0, 150, 512);
    m_pRegion->Subtract(prgnTemp);
    SAFE_RELEASE(prgnTemp);

    m_pGraphics->CreateRectRgn(&prgnTemp, 345, 0, 360, 231);
    m_pRegion->Subtract(prgnTemp);
    SAFE_RELEASE(prgnTemp);

    gxSetRect(&rect1, 5, 5, 155, 120);
    m_pGraphics->CreateRoundRectRgn(&m_pRegion2, rect1, 50, 50);
    m_pGraphics->CreateRectRgn(&prgnTemp, 80, 65, 190, 160);
    m_pRegion2->Xor(prgnTemp);
    SAFE_RELEASE(prgnTemp);
    m_pRegion2->Offset(0, 620);

    return GXApp::OnCreate();
  }
  virtual GXHRESULT OnDestroy()
  {
    SAFE_RELEASE(m_pTestIcon);
    SAFE_RELEASE(m_pImageNonPow2);
    SAFE_RELEASE(m_pEmptyTex);
    SAFE_RELEASE(m_pRegion2);
    SAFE_RELEASE(m_pRegion);
    SAFE_RELEASE(m_pFont);
    SAFE_RELEASE(m_pFontS);
    SAFE_RELEASE(g_pImage0);
    SAFE_RELEASE(g_pImage1);
    SAFE_RELEASE(m_pTexture);
    SAFE_RELEASE(m_pTarget);
    SAFE_RELEASE(m_pTarget2);
    return GXApp::OnDestroy();
  }
  virtual GXHRESULT ActionBegin(GXAPPACTIONINFO* pActionInfo)
  {
    m_bAction = TRUE;
    m_ptAction.x = pActionInfo->ptCursor.x;
    m_ptAction.y = pActionInfo->ptCursor.y;
    return GXApp::ActionBegin(pActionInfo);
  }
  virtual GXHRESULT ActionMove(GXAPPACTIONINFO* pActionInfo)
  {
    m_ptAction.x = pActionInfo->ptCursor.x;
    m_ptAction.y = pActionInfo->ptCursor.y;
    return GXApp::ActionMove(pActionInfo);
  }
  virtual GXHRESULT ActionEnd(GXAPPACTIONINFO* pActionInfo)
  {
    m_bAction = FALSE;
    m_ptAction.x = pActionInfo->ptCursor.x;
    m_ptAction.y = pActionInfo->ptCursor.y;
    GXRECT rect(0,0,100,100);
    if(gxPtInRect(&rect, m_ptAction))
    {
      m_nPage++;
      if(m_nPage > 9)
        m_nPage = 0;
    }
    return GXApp::ActionEnd(pActionInfo);
  }

  virtual GXHRESULT KeyMessage(GXAPPKEYINFO* pKeyInfo)
  {
    if(pKeyInfo->dwKey >= '0' && pKeyInfo->dwKey <= '9')
      m_nPage = pKeyInfo->dwKey - '0';
    if(pKeyInfo->dwKey == ' ')
      m_nSubPage++;
    return GXApp::KeyMessage(pKeyInfo);
  }
  void DrawGrid(GXCanvas* pCanvas, GXRECT& rect);
  void TestDrawHelpText(GXRECT& rect);
  void TestDrawElements(GXRECT& rect);
  void TestTextAndClip(GXRECT& rect);
  void TestDrawToRTInRegion(GXRECT& rect);
  void TestDrawToTwoRT(GXRECT& rect);
  void TestCopyAndStretchRect(GXRECT& rect);
  void TestScroll(GXRECT& rect);
  void TestClear(GXRECT& rect);
  virtual GXHRESULT Render();
};

GXDWORD RandColor()
{
  return GXRGB(rand() & 255, rand() & 255, rand() & 255) | 0xff000000;
}
GXDWORD RandAlphaColor()
{
  return GXRGB(rand() & 255, rand() & 255, rand() & 255) | ((rand() & 255) << 24);
}

void MyGraphicsTest::DrawGrid(GXCanvas* pCanvas, GXRECT& rect)
{
  for(int y = 100; y < rect.bottom; y += 100)
    pCanvas->DrawLine(0, y, rect.right, y, 0xffd0d0d0);

  for(int x = 100; x < rect.right; x += 100)
    pCanvas->DrawLine(x, 0, x, rect.bottom, 0xffd0d0d0);

}

void MyGraphicsTest::TestDrawHelpText(GXRECT& rect)
{
#ifdef _WIN32
  static GXWCHAR* pszHelp = L"1.测试基本绘图\n"
    L"2.测试基本裁剪和文字\n"
    L"3.测试Region限制下的绘制\n"
    L"4.测试绘制到纹理\n"
    L"5.用Region限制绘制到纹理\n"
    L"6.测试分别绘制到两个纹理\n"
    L"7.测试CopyRect和StretchRect\n"
    L"8.测试纹理滚动\n";
#else
  static clString pszHelp = L"1.Basic Drawing\n"
  L"2.Basic clip and text\n"
  L"3.Test Region drawing\n"
  L"4.test draw to texture\n"
  L"5.test draw to texture with region\n"
  L"6.test draw to two texture\n"
  L"7.test CopyRect and StretchRect\n"
  L"8.test scroll texture\n";
#endif
  GXCanvas* pCanvas = m_pGraphics->LockCanvas(NULL, NULL, NULL);
  pCanvas->Clear(0xffe0e0e0);

  DrawGrid(pCanvas, rect);

  if(m_bAction)
  {
    pCanvas->FillRectangle(m_ptAction.x - 10, m_ptAction.y - 10, 20, 20, 0xffff0000);
  }
  
  //pszHelp.SubString(10, 20);
  pCanvas->DrawTextW(m_pFont, pszHelp, -1, (GXLPRECT)&rect, GXDT_VCENTER | GXDT_CENTER, 0xff000000);
  SAFE_RELEASE(pCanvas);
}

void MyGraphicsTest::TestDrawElements(GXRECT& rect)
{
  GXREGN regnDst;
  GXREGN regnSrc;

  GXCanvas* pCanvas = m_pGraphics->LockCanvas(NULL, NULL, NULL);
  pCanvas->Clear(0xffe0e0e0);
  DrawGrid(pCanvas, rect);

  pCanvas->TextOutA(m_pFontS, 10, 10, "SetPixel:", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 10, 100, "DrawRectangle:", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 200, 10, "DrawLine:", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 400, 10, "AlphaBlend:", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 600, 10, "Opaque:", -1, 0xff000000);
  for(int y = 0; y < 32; y++)
  {
    for(int x = 0; x < 32; x++)
    {
      pCanvas->SetPixel(x * 2 + 100, y * 2 + 30, GXColor32(1.0f, (float)x / 32.0f, (float)y / 32.0f, 1.0f).color);
    }
  }

  pCanvas->DrawRectangle(40, 150, 40, 40, 0xff0000ff);
  pCanvas->DrawRectangle(80, 150, 40, 40, GXColor32(0.5f,0.5f,0.5f,1.0f).color);

  for(int y = 0; y < 32; y++)
  {
    for(int x = 0; x < 32; x++)
    {
      pCanvas->SetPixel(x * 2 + 20, y * 2 + 30, GXColor32((float)x / 32.0f, (float)y / 32.0f, 1.0f, 1.0f).color);
    }
  }

  pCanvas->DrawRectangle(20, 130, 40, 40, 0xff00ff00);
  pCanvas->DrawRectangle(60, 130, 40, 40, 0xffff0000);

  for(int y = 0; y < 80; y++)
  {
    pCanvas->DrawLine(200, y * 2 + 30, 400, y * 2 + 30, GXColor32((float)y / 80, 0.f, 0.f, 1.f).color);
  }

  pCanvas->TextOutA(m_pFontS, 10, 200, "DrawTexture(Dest):", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 210, 200, "DrawTexture(Dest,Src):", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 410, 200, "DrawTexture(Pos,Src):", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 610, 200, "FillRectangle:", -1, 0xff000000);

  pCanvas->FillRectangle(610, 230, 150, 160, 0xff0000ff);

  // 测试纹理绘画
  regnDst.left   = 10;
  regnDst.top    = 230;
  regnDst.width  = 180;
  regnDst.height = 160;
  pCanvas->DrawTexture(m_pTexture, &regnDst);

  regnDst.left   = 210;
  regnDst.top    = 230;
  regnDst.width  = 180;
  regnDst.height = 160;

  regnSrc.left   = 10;
  regnSrc.top    = 10;
  regnSrc.width  = 128;
  regnSrc.height = 128;
  pCanvas->DrawTexture(m_pTexture, &regnDst, &regnSrc);

  regnSrc.left   = 10;
  regnSrc.top    = 10;
  regnSrc.width  = 180;
  regnSrc.height = 160;
  pCanvas->DrawTexture(m_pTexture, 410, 230, &regnSrc);

  
  pCanvas->TextOutA(m_pFontS, 10, 400, "Image NonePow2(Dest):", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 210, 400, "Image NonePow2(Dest, Src):", -1, 0xff000000);
  pCanvas->TextOutA(m_pFontS, 510, 400, "Image NonePow2(Pos, Src):", -1, 0xff000000);

  if(m_pImageNonPow2 != NULL)
  {
    regnDst.left   = 30;
    regnDst.top    = 430;
    regnDst.width  = 160;
    regnDst.height = 160;
    pCanvas->DrawImage(m_pImageNonPow2, &regnDst);

    regnDst.left   = 230;
    regnDst.top    = 430;
    regnDst.width  = 260;
    regnDst.height = 160;

    regnSrc.left   = 0;
    regnSrc.top    = 0;
    regnSrc.width  = 128;
    regnSrc.height = 128;
    pCanvas->DrawImage(m_pImageNonPow2, &regnDst, &regnSrc);

    regnSrc.left   = 0;
    regnSrc.top    = 0;
    regnSrc.width  = 210;
    regnSrc.height = 160;
    pCanvas->DrawImage(m_pImageNonPow2, 530, 430, &regnSrc);
  }

  pCanvas->FillRectangle(400 + 10, 50 - 10, 100, 100, 0x8000ff00);
  pCanvas->FillRectangle(450 + 10, 100 - 10, 100, 100, 0x80ff0000);

  pCanvas->SetCompositingMode(CM_SourceCopy);
  pCanvas->FillRectangle(600 + 10, 50 - 10, 100, 100, 0x8000ff00);
  pCanvas->FillRectangle(650 + 10, 100 - 10, 100, 100, 0x80ff0000);

  pCanvas->SetCompositingMode(CM_SourceOver);
  pCanvas->TextOutA(m_pFontS, 10, 600, "Clip Drawing:", -1, 0xff000000);
  GXRECT rect2;
  m_pRegion2->GetBounding(&rect2);
  gxRectToRegn(&regnDst, &rect2);
  pCanvas->SetRegion(m_pRegion2, TRUE);
  pCanvas->DrawTexture(m_pTexture, &regnDst);

  pCanvas->SetRegion(NULL, FALSE);
  // Draw Rotated Texture
  {
    GXREGN rgSrc(0);
    GXREGN rgDest(0, 0, 128, 128);
    m_pTestIcon->GetDimension((GXUINT*)&rgSrc.width, (GXUINT*)&rgSrc.height);

    rgDest.left = 210;
    rgDest.top = 625;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_None", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_None);

    rgDest.left = 360;
    rgDest.top = 625;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_CW90", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_CW90);

    rgDest.left = 510;
    rgDest.top = 625;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_CCW90", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_CCW90);

    rgDest.left = 10;
    rgDest.top = 825;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_180", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_180);

    rgDest.left = 160;
    rgDest.top = 825;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_CW90_Flip", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_CW90_Flip);

    rgDest.left = 310;
    rgDest.top = 825;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_180_Flip", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_180_Flip);

    rgDest.left = 460;
    rgDest.top = 825;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_CCW90_Flip", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_CCW90_Flip);

    rgDest.left = 610;
    rgDest.top = 825;
    pCanvas->TextOutA(m_pFontS, rgDest.left, rgDest.top - 25, "Rotate_FlipHorizontal", -1, 0xff000000);
    pCanvas->DrawTexture(m_pTestIcon, &rgDest, &rgSrc, Rotate_FlipHorizontal);
  }

  SAFE_RELEASE(pCanvas);
}

void MyGraphicsTest::TestTextAndClip(GXRECT& rect)
{
  REGN regn;
  regn.left = 40;
  regn.top = 40;
  regn.width = (rect.right - rect.left) / 4 * 3;
  regn.height = (rect.bottom - rect.top) / 2 - 80;

  GXCanvas* pCanvas = m_pGraphics->LockCanvas(NULL, &regn, NULL);
  pCanvas->Clear(0xffe0e0e0);

  const int c_max = 50;
  for(int i = 1; i < 50; i++)
  {
    float fFactor = (float)i / (float)c_max;
    pCanvas->DrawLine(i * 10, 0, 0, (50 - i) * 10, GXColor32(fFactor, 1.0f, 1.0f - fFactor, 1.0f).color);
  }

  GXRECT rcText;
  gxSetRect(&rcText, 210, 20, 310, 100);
  pCanvas->FillRectangle(&rcText, 0x80808080);
  pCanvas->DrawTextW(m_pFont, L"Hello World!", -1, &rcText, GXDT_SINGLELINE, 0xff00ff00);


  SAFE_RELEASE(pCanvas);
}
void MyGraphicsTest::TestDrawToRTInRegion(GXRECT& rect)
{
  // m_nPage == 3 裁剪测试
  // m_nPage == 4 RT绘制
  // m_nPage == 5 裁剪测试 + RT绘制
  GXCanvas* pCanvas = m_nPage == 3
    ? m_pGraphics->LockCanvas(NULL, NULL, NULL)
    : m_pGraphics->LockCanvas(m_pTarget, NULL, NULL);

  if(m_nPage != 3)
    pCanvas->Clear(0);

  if(m_nPage == 3 || m_nPage == 5)
    pCanvas->SetRegion(m_pRegion, TRUE);
  pCanvas->Clear(0xffe0e0e0);

  GXREGN regnDst;
  GXREGN regnSrc;

  regnDst.left   = (GXLONG)(sin((float)gxGetTickCount() * 0.001f) * 200.0f);
  regnDst.top    = 0;
  regnDst.width  = 1024;
  regnDst.height = 768;
  pCanvas->DrawTexture(m_pTexture, &regnDst);

  regnDst.left   = gxGetTickCount() % 768;
  regnDst.top    = 150;
  regnDst.width  = 200;
  regnDst.height = 200;
  pCanvas->DrawTexture(m_pTexture, &regnDst);

  GXRECT rcText((gxGetTickCount() / 10) % 768, -20, 250, 100);
  rcText.right += rcText.left;
  pCanvas->TextOutW(m_pFont, rcText.left - 20, rcText.top, L"TextOutString!~", 15, 0xff000000);
  pCanvas->DrawTextW(m_pFont, _CLTEXT("Hello World"), -1, &rcText, DT_LEFT, 0xffffffff);

  rcText.top = 100;
  rcText.bottom = 200;
  pCanvas->DrawTextW(m_pFont, _CLTEXT("Hello World"), -1, &rcText, DT_LEFT, 0xff00ffff);

  rcText.left -= 20;
  rcText.top = 480;
  rcText.bottom = rcText.top + 200;
  pCanvas->DrawTextW(m_pFont, _CLTEXT("Hello World"), -1, &rcText, DT_LEFT, 0xffff00ff);


  //regnDst.left   = (GetTickCount() / 7) % 768;
  //regnDst.top    = 0;
  //regnDst.width  = 100;
  //regnDst.height = 768;
  SAFE_RELEASE(pCanvas);

  if(m_nPage != 3)
  {
    pCanvas = m_pGraphics->LockCanvas(NULL, NULL, NULL);
    gxSetRegn(&regnSrc, 0, 0, rect.right, rect.bottom);
    //pCanvas->DrawImage(m_pTarget, cos(m_fTime) * 40.0f + 40.0f, sin(m_fTime) * 40.0f + 40.0f, &regnSrc);
    pCanvas->DrawImage(m_pTarget, 0, 0, &regnSrc);
    SAFE_RELEASE(pCanvas);
  }
}

void MyGraphicsTest::TestDrawToTwoRT(GXRECT& rect)
{
  GXCanvas* pCanvas;
  GXREGN regnDst;
  const float fRadius = 40.0f;
  const GXCOLORREF crClear0 = 0xe0e0e0e0;
  const GXCOLORREF crClear1 = 0xe0c0c0c0;
  gxSetRegn(&regnDst, 0, 150, 128, 128);
  pCanvas = m_pGraphics->LockCanvas(m_pTarget, NULL, NULL);
  pCanvas->Clear(crClear1);
  pCanvas->DrawTexture(m_pTexture, &regnDst);
  SAFE_RELEASE(pCanvas);

  pCanvas = m_pGraphics->LockCanvas(m_pTarget2, NULL, NULL);
  pCanvas->Clear(crClear1);
  regnDst.left = (GXLONG)(cos(m_fTime) * fRadius + fRadius);
  regnDst.top  = (GXLONG)(sin(m_fTime) * fRadius + fRadius);
  pCanvas->DrawTexture(m_pTexture, &regnDst);
  SAFE_RELEASE(pCanvas);

  regnDst.width = rect.right >> 1;
  regnDst.height = rect.bottom >> 1;

  pCanvas = m_pGraphics->LockCanvas(NULL, NULL, NULL);
  pCanvas->Clear(crClear0);

  pCanvas->DrawImage(m_pTarget, &regnDst);

  regnDst.left = (rect.right >> 1);
  regnDst.top = 150;

  pCanvas->DrawImage(m_pTarget2, &regnDst);

  SAFE_RELEASE(pCanvas);
}

void MyGraphicsTest::TestCopyAndStretchRect(GXRECT& rect)
{
  GXCanvas* pCanvas;
  GXREGN regn;
  GXRECT rcSrc;
  GXRECT rcDest;
  GXREGN regnSrc;
  GXREGN regnDest;
  pCanvas = m_pGraphics->LockCanvas(NULL, NULL, NULL);

  m_pEmptyTex->GetDimension((GXUINT*)&regn.width, (GXUINT*)&regn.height);
  regn.left = regn.width >> 1;
  regn.top = regn.height >> 1;

  gxSetRegn(&regnSrc, 64, 32, 128, 32 * 5);
  gxRegnToRect(&rcSrc, &regnSrc);

  m_nSubPage = m_nSubPage % 2;

  if(m_nSubPage == 0)
  {
    m_pEmptyTex->CopyRect(m_pTexture, &rcSrc, NULL);
    pCanvas->DrawTexture(m_pTexture, rcSrc.left, rcSrc.top, &regnSrc);
  }
  else
  {
    rcDest = rcSrc;
    gxInflateRect(&rcDest, 32, 32);
    gxOffsetRect(&rcDest, -rcDest.left, -rcDest.top);
    m_pEmptyTex->StretchRect(m_pTexture, &rcDest, &rcSrc, GXTEXFILTER_LINEAR);
    gxRectToRegn(&regnDest, &rcDest);
    pCanvas->DrawTexture(m_pTexture, &regnDest, &regnSrc);
  }
  pCanvas->DrawTexture(m_pEmptyTex, &regn);

  SAFE_RELEASE(pCanvas);
}

void MyGraphicsTest::TestScroll(GXRECT& rect)
{
  static int s_nIndex = 1;
  //const float fRadius = 50;

  REGN regn;

  gxSetRegn(&regn, 0, 0, rect.right, rect.bottom);
  GXCanvas* pCanvas = m_pGraphics->LockCanvas(NULL, &regn, NULL);

  //pCanvas->SetSamplerState(0, GXSAMP_MINFILTER, GXTEXFILTER_POINT);
  //pCanvas->SetSamplerState(0, GXSAMP_MAGFILTER, GXTEXFILTER_POINT);

  REGN regnDst, regnSrc;
  GXRECT rcScroll(30, 50, 300, 300);

  regnDst.left   = 0;//fRadius + cos(m_fTime) * fRadius;
  regnDst.top    = 0;//fRadius + sin(m_fTime) * fRadius;
  regnDst.width  = 400;
  regnDst.height = 400;

  regnSrc = regnDst;
  
  pCanvas->DrawTexture(m_pTexture, &regnDst);
  
  if(s_nIndex == 0)
  {
    pCanvas->Scroll(-128, 128, &rcScroll, NULL, NULL, NULL);
  }
  else if(s_nIndex == 1)
  {
    GXRECT rcClip(50, 70, 650, 250);
    GRegion* prgnUpdate = NULL;
    pCanvas->Scroll(100, 70, &rcScroll, &rcClip, &prgnUpdate, NULL);
    pCanvas->DrawRectangle(rcScroll.left, rcScroll.top, rcScroll.right - rcScroll.left, rcScroll.bottom - rcScroll.top, 0xff00ff00);
    pCanvas->DrawRectangle(rcClip.left, rcClip.top, rcClip.right - rcClip.left, rcClip.bottom - rcClip.top, 0xffff0000);
    if(prgnUpdate != NULL)
    {
      int nCount = prgnUpdate->GetRectCount();
      GXRECT* pRects = new GXRECT[nCount];
      prgnUpdate->GetRects(pRects, nCount);
      for(int i = 0; i < nCount; i++)
        pCanvas->FillRectangle(pRects[i].left, pRects[i].top, pRects[i].right - pRects[i].left, pRects[i].bottom - pRects[i].top, 0x4000ff00);
      SAFE_DELETE(pRects);
    }
    SAFE_RELEASE(prgnUpdate);
  }

  SAFE_RELEASE(pCanvas);
}
void MyGraphicsTest::TestClear(GXRECT& rect)
{
  REGN regn;
  gxSetRegn(&regn, 0, 0, rect.right, rect.bottom);
  GXCanvas* pCanvas = m_pGraphics->LockCanvas(NULL, &regn, NULL);
  
  pCanvas->Clear(0xff00ff00);
  
  SAFE_RELEASE(pCanvas);
}
GXHRESULT MyGraphicsTest::Render()
{
  GXRECT rect;
  GXRECT rcMini;
  GXSIZE size;

  GXUIGetStationDesc(GXSD_EXTENT, 0, (GXLPARAM)&size);
  gxSetRect(&rect, 0, 0, size.cx, size.cy);
  gxSetRect(&rcMini, 10, 10, 200, 200);

  m_pGraphics->Clear(0, NULL, GXCLEAR_TARGET, 0xff00a0a0, 1.0f, 0);
  m_pGraphics->Clear(&rcMini, 1, GXCLEAR_TARGET, 0xffff00a0, 1.0f, 0);
  m_dwTick = gxGetTickCount();
  m_fTime = (float)m_dwTick * 0.001f;
  if(GXSUCCEEDED(m_pGraphics->Begin()))
  {
    if(m_nPage == 0)
      TestDrawHelpText(rect);
    else if(m_nPage == 1)
      TestDrawElements(rect);
    else if(m_nPage == 2)
      TestTextAndClip(rect);
    else if(m_nPage == 3)
      TestDrawToRTInRegion(rect);
    else if(m_nPage == 4)
      TestDrawToRTInRegion(rect);
    else if(m_nPage == 5)
      TestDrawToRTInRegion(rect);
    else if(m_nPage == 6)
      TestDrawToTwoRT(rect);
    else if(m_nPage == 7)
      TestCopyAndStretchRect(rect);
    else if(m_nPage == 8)
      TestScroll(rect);
    else if(m_nPage == 9)
      TestClear(rect);

    m_pGraphics->End();
    m_pGraphics->Present();
  }
  return GX_OK;
}

#if defined(_WIN32) || defined(_WINDOWS)
int APIENTRY _tWinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPTSTR    lpCmdLine,
  int       nCmdShow)
#else
int main(int argc, char *argv[])
#endif // #if defined(_WIN32) || defined(_WINDOWS)
{
  clpathfile::LocalWorkingDirW(L"..");

  GXBYTE AppStruct[BYTE_ALIGN_4(sizeof(MyGraphicsTest))];
  MyGraphicsTest* appTest = new(AppStruct) MyGraphicsTest;
  GXAPP_DESC sAppDesc = {0};
  sAppDesc.cbSize     = sizeof(GXAPP_DESC);
  sAppDesc.lpName     = L"Test Graphics";
  sAppDesc.nWidth     = 768;
  sAppDesc.nHeight    = 1024;
  sAppDesc.dwStyle    = GXADS_SIZABLE;
  sAppDesc.pParameter = NULL;
  //#ifdef _WIN32
  //  if(MessageBox(NULL, L"是否使用DX9渲染?", L"渲染方式", MB_YESNO) == IDYES)
  //    gxad.idPlatform = GXPlatform_Win32_Direct3D9;
  //  else {
  //    gxad.idPlatform = GXPlatform_X_OpenGLES2;
  //    //gxad.idPlatform = GXPlatform_Win32_OpenGL;
  //  }
  //#else
  //  gxad.idPlatform = GXPlatform_X_OpenGLES2;
  //#endif
  if(MOUICreatePlatformSelectedDlg(hInstance, &sAppDesc))
  {
    appTest->Go(&sAppDesc);
  }
  return 0;
}