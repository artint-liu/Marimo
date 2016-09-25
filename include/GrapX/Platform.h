#ifndef _GRAPHX_PLATFORM_H_
#define _GRAPHX_PLATFORM_H_

enum   GXPlaformIdentity;
class  GXApp;
class  GXGraphics;
struct GXAPP_DESC;

class IGXPlatform
{
public:
  GXApp* m_pApp;
  virtual ~IGXPlatform() {}
  GXSTDINTERFACE(GXHRESULT    Initialize    (GXApp* m_pApp, GXAPP_DESC* pDesc, GXOUT GXGraphics** ppGraphics));
  GXSTDINTERFACE(GXHRESULT    Finalize      (GXINOUT GXGraphics** ppGraphics));
  GXSTDINTERFACE(GXHRESULT    MainLoop      ());
  GXSTDINTERFACE(GXVOID       GetPlatformID (GXPlaformIdentity* pIdentity));
  GXSTDINTERFACE(GXLPCWSTR    GetRootDir    ());
  GXSTDINTERFACE(GXHRESULT    QueryFeature  (GXDWORD dwFeatureCode, GXVOID** ppUnknown)); // 查询平台相关的特征值, 这个与平台相关的扩展, ppUnknown 可能是对象或者指针
};

// 平台方便创建的模板
template<class _TIPlatform>
_TIPlatform* AppCreatePlatformT(GXApp* pApp, GXAPP_DESC* pDesc, GXGraphics** ppGraphics)
{
  _TIPlatform* pPlatform = new _TIPlatform;
  GXHRESULT hval = pPlatform->Initialize(pApp, pDesc, ppGraphics);
  if(GXSUCCEEDED(hval)) {
    return pPlatform;
  }
  else {
    pPlatform->Finalize(ppGraphics);
    SAFE_DELETE(pPlatform);
    return NULL;
  }
}

#endif // _GRAPHX_PLATFORM_H_