#ifndef _DEV_DISABLE_UI_CODE
#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "GrapX/GXUser.H"
#include <GXStation.H>
#include <User/GXWindow.h>

typedef struct __tagGXTIMERCHAIN
{
  GXHWND        hWnd;
  GXUINT        nIDEvent;
  GXUINT        uElapse;
  GXTIMERPROC    lpTimerFunc;
  GXDWORD        uLastTick;
  __tagGXTIMERCHAIN*  lpNext;
}GXTIMERCHAIN, *LPGXTIMERCHAIN;

LPGXTIMERCHAIN g_lpgxTimerChain;

GXUINT GXDLLAPI gxSetTimer(
        GXHWND hWnd,        // handle of window for timer messages
        GXUINT nIDEvent,      // timer identifier
        GXUINT uElapse,        // time-out value
        GXTIMERPROC lpTimerFunc   // address of timer procedure
        )
{
  LPGXTIMERCHAIN pNewTimer;
  LPGXTIMERCHAIN pTimer = g_lpgxTimerChain;
  pNewTimer = new GXTIMERCHAIN;

  pNewTimer->hWnd        = hWnd;
  pNewTimer->nIDEvent    = nIDEvent;
  pNewTimer->uElapse     = uElapse;
  pNewTimer->lpTimerFunc = lpTimerFunc;
  pNewTimer->uLastTick   = gxGetTickCount();
  pNewTimer->lpNext      = NULL;
  if(g_lpgxTimerChain == NULL)
  {
    g_lpgxTimerChain = pNewTimer;
    return (GXUINT)(GXUINT_PTR)pNewTimer;
  }
  while(pTimer->lpNext != NULL)
  {
    if(pTimer->hWnd == hWnd && pTimer->nIDEvent == nIDEvent)
    {
      SAFE_DELETE(pNewTimer);
      return 0;
    }
    pTimer = pTimer->lpNext;
  }
  pTimer->lpNext = pNewTimer;
  return (GXUINT)(GXUINT_PTR)pNewTimer;
}

GXBOOL GXDLLAPI gxKillTimer(
         GXHWND hWnd,          // handle of window that installed timer
         GXUINT uIDEvent       // timer identifier
         )
{
  LPGXTIMERCHAIN pPrev = NULL;
  LPGXTIMERCHAIN pTimer = g_lpgxTimerChain;
  GXBOOL bRet = FALSE;
  while(pTimer != NULL)
  {
    if(pTimer->hWnd == hWnd && (pTimer->nIDEvent == uIDEvent || uIDEvent == NULL))
    {
      bRet = TRUE;
      if(pPrev == NULL)
      {
        g_lpgxTimerChain = pTimer->lpNext;
        delete pTimer;
        pTimer = g_lpgxTimerChain;
        continue;
      }
      else
      {
        pPrev->lpNext = pTimer->lpNext;
        delete pTimer;
        pTimer = pPrev->lpNext;
        continue;
      }
    }
    pPrev = pTimer;
    pTimer = pTimer->lpNext;
  }

   return bRet;
}

extern "C" GXVOID GXDLLAPI GXUIUpdateTimerEvent()
{
  LPGXTIMERCHAIN pPrevTimer = NULL;
  LPGXTIMERCHAIN pTimer = g_lpgxTimerChain;
  GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->Enter();
  GXDWORD uTick = gxGetTickCount();
  while (pTimer != NULL)
  {
    if(pTimer->uElapse == -1)
    {
      if(pPrevTimer == NULL)
      {
        g_lpgxTimerChain = g_lpgxTimerChain->lpNext;
        SAFE_DELETE(pTimer);
        pTimer = g_lpgxTimerChain;
      }
      else
      {
        pPrevTimer->lpNext = pTimer->lpNext;
        SAFE_DELETE(pTimer);
        pTimer = pPrevTimer->lpNext;
      }
      continue;
    }
    while(uTick - pTimer->uLastTick > pTimer->uElapse)
    {
      if(pTimer->lpTimerFunc != NULL)
        pTimer->lpTimerFunc(pTimer->hWnd, GXWM_TIMER, pTimer->nIDEvent, uTick);
      else
        gxPostMessage(pTimer->hWnd, GXWM_TIMER, (GXWPARAM)pTimer->nIDEvent, (GXLPARAM)pTimer->lpTimerFunc);
      pTimer->uLastTick += pTimer->uElapse;
    }
    pPrevTimer = pTimer;
    pTimer = pTimer->lpNext;
  }
  lpStation->Leave();
}
#endif // _DEV_DISABLE_UI_CODE