#ifndef _GRAPX_INTERNAL_MESSAGE_H_
#define _GRAPX_INTERNAL_MESSAGE_H_

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
    virtual i32 Run                 ();
};

#endif // _GRAPX_INTERNAL_MESSAGE_H_