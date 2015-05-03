#if 0
//////////////////////////////////////////////////////////////////////////
//
// 特效管理器
// 隶属于 GXStation, 主要用于 Windows 的各种特效实现, 也可用回调函数编写不基于 
// Window 的特效, 所以以"rich"命名.
//
#ifndef _RICH_FX_MANAGER_H_
#define _RICH_FX_MANAGER_H_

//#include <vector>

// RichFX 回调时的参数
struct RFXPROCPARAM
{
  GXDWORD  dwId;
  GXDWORD  dwTime;    // 从注册开始到现在的时间,从0开始, 如果为0xffffffff, 
            // 则说明它已经达到了生命周期的最后, 即将被销毁
  GXDWORD  dwElapse;  // 注册时声明的持续时间
  GXDWORD  dwFlags;  // 注册时用的标志
  GXLPARAM  lParam;    // 用户参数
};

// RichFX 的回调函数
// RichFX的生命周期是注册时声明的dwElapse, 但是如果回调函数返回false则会立即销毁这个特效
typedef GXBOOL (GXCALLBACK* RichFXProc)(const RFXPROCPARAM* pParam);

// 注册用的RichFX描述
struct RICHFX
{
  GXDWORD      dwElapse;
  GXDWORD      dwFlags;
  RichFXProc    lpCallBack;
  GXLPARAM      lParam;
};

struct RICHFXITEM
{
  RICHFX    RichFX;
  GXDWORD    dwId;
  GXDWORD    dwBegin;
  GXDWORD    dwTime;
};


class RichFXMgr
{
  typedef clvector<RICHFXITEM> RichFXArray;
private:
  GXLPSTATION    m_lpStation;
  RichFXArray    m_aRichFX;
  GXDWORD      m_dwTime;
  GXDWORD      m_dwIdCounter;
public:
  RichFXMgr(GXLPSTATION lpStation);
  GXBOOL  TickRender    (GXDWORD dwDeltaTime);
  GXDWORD  Register    (const RICHFX* pDesc);
};

#endif // _RICH_FX_MANAGER_H_
#endif // #if 0