#ifndef _GRAPX_INTERNAL_MESSAGE_H_
#define _GRAPX_INTERNAL_MESSAGE_H_

//#define REFACTOR_SYSQUEUE

namespace GrapX
{
  namespace Internal
  {
    class SystemMessage
    {
    private:
      typedef cllist<GXSYSMSG> SysMsgList;
      IGXPlatform*  m_pPlatform;

      clstd::Signal         m_signal;
      clstd::Locker         m_locker;
      SysMsgList            m_messages;

    public:
      SystemMessage(IGXPlatform* pPlatform)
        : m_pPlatform(pPlatform)
        //, m_pSendingMsg(NULL)
        //, m_pResult(NULL)
      {
      }
      //GXLRESULT   SendRemoteMessage   (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
      //GXBOOL      CallSentMessageProc ();
      void PostQuitMessage(u32_ptr nExitCode)
      {
        GXSYSMSG msg = {};
        msg.message = GXSysMessage_Quit;
        msg.wParam  = nExitCode;
        msg.dwTime = gxGetTickCount();

        m_locker.Lock();
        m_messages.insert(m_messages.begin(), msg);
        m_locker.Unlock();
        m_signal.Set();
      }

      void Post(const GXSYSMSG& msg)
      {
        m_locker.Lock();
        m_messages.push_back(msg);
        m_messages.back().dwTime = gxGetTickCount();
        m_locker.Unlock();
        m_signal.Set();
      }

      void Post(GXSysMessage message, GXWPARAM wParam, GXLPARAM lParam)
      {
        GXSYSMSG msg;
        msg.handle = NULL;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;
        msg.dwTime = gxGetTickCount();

        m_locker.Lock();
        m_messages.push_back(msg);
        m_locker.Unlock();

        m_signal.Set();
      }

      GXBOOL Peek(GXSYSMSG* msg, GXUINT wMsgFilterMin, GXUINT wMsgFilterMax, GXBOOL bRemoveMsg)
      {
      }

      GXBOOL Get(GXSYSMSG* msg, GXUINT wMsgFilterMin, GXUINT wMsgFilterMax)
      {
      }

      GXLRESULT   SendRemoteMessage   (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
      {
      }
      
      GXBOOL      CallSentMessageProc ()
      {
      }
      //i32 StartRoutine                 () override;
    };
  } // namespace Internal
} // namespace GrapX

//#else

class GXUIMsgThread : public clstd::MsgThreadT<MOUIMSG>
{
    IGXPlatform*  m_pPlatform;
    clstd::Signal m_SendingWaiter;
    MOUIMSG*      m_pSendingMsg;
    GXLRESULT*    m_pResult;
public:
    GXUIMsgThread(IGXPlatform* pPlatform)
        : m_pPlatform(pPlatform)
        , m_pSendingMsg(NULL)
        , m_pResult(NULL)
    {
    }
    GXLRESULT   SendRemoteMessage   (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    GXBOOL      CallSentMessageProc ();
    i32         StartRoutine        () override;
};

//#endif

#endif // _GRAPX_INTERNAL_MESSAGE_H_