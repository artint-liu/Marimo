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

      clstd::Signal*        m_pSignal;
      clstd::Locker         m_locker;
      SysMsgList            m_messages;

    public:
      SystemMessage(IGXPlatform* pPlatform, clstd::Signal* pSignal)
        : m_pPlatform(pPlatform)
        , m_pSignal(pSignal)
      {
      }

      void Post(const GXSYSMSG& msg)
      {
        m_locker.Lock();
        m_messages.push_back(msg);
        m_messages.back().dwTime = gxGetTickCount();
        m_locker.Unlock();
        m_pSignal->Set();
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

        m_pSignal->Set();
      }

      GXBOOL Peek(GXSYSMSG* msg, GXUINT wMsgFilterMin, GXUINT wMsgFilterMax, GXBOOL bRemoveMsg)
      {
        m_locker.Lock();
        size_t uMsgCount = m_messages.size();
        if(uMsgCount > 0)
        {
          for(auto it = m_messages.begin(); it != m_messages.end(); ++it)
          {
            if(static_cast<GXUINT>(it->message) >= wMsgFilterMin && (static_cast<GXUINT>(it->message) <= wMsgFilterMax || wMsgFilterMax == 0))
            {
              if(msg != NULL) {
                *msg = *it;
              }

              if(bRemoveMsg == 0x0001/*PM_REMOVE*/) {
                m_messages.erase(it);
              }

              m_locker.Unlock();
              ASSERT(uMsgCount >= 1);
              return (b32)uMsgCount;
            }
          }
        }
        //ASSERT(m_aMessage.size() == 0);
        m_locker.Unlock();
        return FALSE;

        //CLBREAK;
        //return FALSE;
      }

      GXBOOL Get(GXSYSMSG* msg, GXUINT wMsgFilterMin, GXUINT wMsgFilterMax)
      {
        CLBREAK;
        return FALSE;
      }

      //GXLRESULT   SendRemoteMessage   (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
      //{
      //}
      //
      //GXBOOL      CallSentMessageProc ()
      //{
      //}
      //i32 StartRoutine                 () override;
    };
  } // namespace Internal
} // namespace GrapX

//#else

class GXUIMsgThread : public clstd::MsgThreadT<MOUIMSG>
{
  IGXPlatform*  m_pPlatform;
  GXDWORD       m_dwThreadId;
  //clstd::Signal m_SendingWaiter;
  //GXLRESULT*    m_pResult;

  // 其它线程发送的消息
  clstd::Locker       m_RemoteLocker;     // 远程消息锁，保证同时只有一个远程消息在处理中
  clstd::Signal       m_RemoteSignal;     // 远程消息完成的信号
  volatile MOUIMSG*   m_pRemoteMsg;
  volatile GXLRESULT* m_pRemoteResult;
  // <<<@

public:
  GXUIMsgThread(IGXPlatform* pPlatform)
    : m_pPlatform(pPlatform)
    , m_dwThreadId(0)
    //, m_pSendingMsg(NULL)
    //, m_pResult(NULL)
    , m_pRemoteMsg(NULL)
    , m_pRemoteResult(NULL)
  {
    m_dwThreadId = gxGetCurrentThreadId();
  }

  clstd::Signal* GetMessageSignal();
  GXLRESULT   SendRemoteMessage(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  GXBOOL      ResponseSentMessage();
  i32         StartRoutine() override;
};

//#endif

#endif // _GRAPX_INTERNAL_MESSAGE_H_