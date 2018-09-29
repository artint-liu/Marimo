#if 0
#include <GrapX.h>
#include <User/GrapX.Hxx>
#include "RichFXMgr.h"

RichFXMgr::RichFXMgr(GXLPSTATION lpStation)
  : m_lpStation  (lpStation)
  , m_dwTime    (0)
  , m_dwIdCounter  (1)
{

}

GXBOOL RichFXMgr::TickRender(GXDWORD dwDeltaTime)
{
  RFXPROCPARAM RFXParam;
  for(RichFXArray::iterator it = m_aRichFX.begin();
    it != m_aRichFX.end(); ++it)
  {
    RICHFXITEM& Item = *it;
    GXBOOL bCallBackRet = TRUE;

    Item.dwTime += dwDeltaTime;
    if(Item.dwTime > Item.RichFX.dwElapse)
      Item.dwTime = (GXDWORD)-1;
    RFXParam.dwElapse = Item.RichFX.dwElapse;
    RFXParam.dwFlags  = Item.RichFX.dwFlags;
    RFXParam.dwId     = Item.dwId;
    RFXParam.dwTime   = Item.dwTime;
    RFXParam.lParam   = Item.RichFX.lParam;

    if(Item.RichFX.lpCallBack != NULL)
      bCallBackRet = Item.RichFX.lpCallBack(&RFXParam);

    if(bCallBackRet == FALSE || Item.dwTime == (GXDWORD)-1)
    {
      it = m_aRichFX.erase(it);
      if(it == m_aRichFX.end())
        break;
    }
  }
  m_dwTime += dwDeltaTime;
  return TRUE;
}

GXDWORD RichFXMgr::Register(const RICHFX* pDesc)
{
  RICHFXITEM Item;
  Item.RichFX  = *pDesc;
  Item.dwId    = m_dwIdCounter++;
  Item.dwTime  = 0;
  Item.dwBegin = m_dwTime;
  m_aRichFX.push_back(Item);
  return Item.dwId;
}
#endif // #if 0