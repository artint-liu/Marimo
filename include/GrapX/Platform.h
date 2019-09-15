#ifndef _GRAPHX_PLATFORM_H_
#define _GRAPHX_PLATFORM_H_

enum   GXPlatformIdentity;
class  GXApp;

namespace GrapX
{
  class  Graphics;
}
struct GXAPP_DESC;

class IGXPlatform
{
public:
  GXApp* m_pApp;
  virtual ~IGXPlatform() {}
  GXSTDINTERFACE(GXHRESULT    Initialize      (GXApp* m_pApp, GXAPP_DESC* pDesc, GXOUT GrapX::Graphics** ppGraphics));
  GXSTDINTERFACE(GXHRESULT    Finalize        (GXINOUT GrapX::Graphics** ppGraphics));
  GXSTDINTERFACE(GXHRESULT    MainLoop        ());
  GXSTDINTERFACE(GXPlatformIdentity GetPlatformID   () const);
  GXSTDINTERFACE(GXLPCWSTR    GetRootDir      ());
  GXSTDINTERFACE(GXHRESULT    QueryFeature    (GXDWORD dwFeatureCode, GXVOID** ppUnknown)); // 查询平台相关的特征值, 这个与平台相关的扩展, ppUnknown 可能是对象或者指针
  //GXSTDINTERFACE(GXUpdateRate GetUpdateRate   () const);
};

// 平台方便创建的模板
template<class _TIPlatform>
_TIPlatform* AppCreatePlatformT(GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics)
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