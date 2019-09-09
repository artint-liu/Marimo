#include <GrapX.h>
#include <GrapX/GResource.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXRenderTarget.h>
#include <GrapX/GXCanvas.h>


void TestRenderTarget_SaveToFile(GrapX::Graphics* pGraphics)
{
  GrapX::RenderTarget* pRenderTarget = NULL;
  GXHRESULT hr = pGraphics->CreateRenderTarget(&pRenderTarget, NULL, 512, 512, Format_B8G8R8A8, Format_D24S8);
  GrapX::Canvas* pCanvas = pGraphics->LockCanvas(pRenderTarget, NULL, NULL);

  GXRegn regn(32, 32, 512 - 64, 512 - 64);
  pCanvas->Clear(0xffff0000);
  pCanvas->FillRectangle(regn, 0xff00ff00);

  SAFE_RELEASE(pCanvas);

  pRenderTarget->SaveToFile(_CLTEXT("TestSaveTo.png"), "PNG");
  SAFE_RELEASE(pRenderTarget);
}

void TestRenderTarget(GrapX::Graphics* pGraphics)
{
  TestRenderTarget_SaveToFile(pGraphics);
}