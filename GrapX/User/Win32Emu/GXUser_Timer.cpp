#ifndef _DEV_DISABLE_UI_CODE
#include <GrapX.H>
#include <clSchedule.h>
#include <User/GrapX.Hxx>
#include "GrapX/GXUser.H"
#include <GXStation.H>
#include <User/GXWindow.h>

struct GXTIMERCHAIN
{
  GXHWND        hWnd;
  GXUINT        nIDEvent;
  GXUINT        uElapse;
  GXTIMERPROC   lpTimerFunc;
  GXDWORD       uLastTick;
  GXTIMERCHAIN* lpNext;
};

//LPGXTIMERCHAIN g_lpgxTimerChain;

GXUINT GXDLLAPI gxSetTimer(
        GXHWND hWnd,        // handle of window for timer messages
        GXUINT nIDEvent,      // timer identifier
        GXUINT uElapse,        // time-out value
        GXTIMERPROC lpTimerFunc   // address of timer procedure
        )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  if( ! lpStation) {
    return 0;
  }

  if(lpStation->GetUpdateRate() == UpdateRate_Lazy)
  {
    CLOG_WARNING("NOT implement in lazy mode.");
  }
#ifdef REFACTOR_TIMER
  lpStation->m_pShcedule->CreateTimer(hWnd, nIDEvent, uElapse, (clstd::TimerProc)lpTimerFunc);
#else
  GXTIMERCHAIN* pNewTimer = NULL;
  GXTIMERCHAIN* pTimer = lpStation->m_pTimerChain;
  pNewTimer = new GXTIMERCHAIN;

  pNewTimer->hWnd        = hWnd;
  pNewTimer->nIDEvent    = nIDEvent;
  pNewTimer->uElapse     = uElapse;
  pNewTimer->lpTimerFunc = lpTimerFunc;
  pNewTimer->uLastTick   = gxGetTickCount();
  pNewTimer->lpNext      = NULL;
  if(lpStation->m_pTimerChain == NULL)
  {
    lpStation->m_pTimerChain = pNewTimer;
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
#endif // REFACTOR_TIMER
}

GXBOOL GXDLLAPI gxKillTimer(
         GXHWND hWnd,          // handle of window that installed timer
         GXUINT uIDEvent       // timer identifier
         )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  if(!lpStation) {
    return 0;
  }
#ifdef REFACTOR_TIMER
  lpStation->m_pShcedule->DestroyTimer(hWnd, uIDEvent);
#else
  GXTIMERCHAIN* pPrev = NULL;
  GXTIMERCHAIN* pTimer = lpStation->m_pTimerChain;
  GXBOOL bRet = FALSE;
  while(pTimer != NULL)
  {
    if(pTimer->hWnd == hWnd && (pTimer->nIDEvent == uIDEvent || uIDEvent == NULL))
    {
      bRet = TRUE;
      if(pPrev == NULL)
      {
        lpStation->m_pTimerChain = pTimer->lpNext;
        delete pTimer;
        pTimer = lpStation->m_pTimerChain;
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
#endif // #ifdef REFACTOR_TIMER
}

extern "C" GXVOID GXDLLAPI GXUIUpdateTimerEvent()
{
#ifdef REFACTOR_TIMER
#else
  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
  if(!lpStation) {
    return;
  }
  GXTIMERCHAIN* pPrevTimer = NULL;
  GXTIMERCHAIN* pTimer = lpStation->m_pTimerChain;
  //GXLPSTATION lpStation = IntGetStationPtr();
  lpStation->Enter();
  GXDWORD uTick = gxGetTickCount();
  while (pTimer != NULL)
  {
    if(pTimer->uElapse == -1)
    {
      if(pPrevTimer == NULL)
      {
        lpStation->m_pTimerChain = lpStation->m_pTimerChain->lpNext;
        SAFE_DELETE(pTimer);
        pTimer = lpStation->m_pTimerChain;
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
#endif // #ifdef REFACTOR_TIMER
}
#endif // _DEV_DISABLE_UI_CODE