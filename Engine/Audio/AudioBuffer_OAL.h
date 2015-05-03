#ifndef _AUDIO_PLAY_BUFFER_H_
#define _AUDIO_PLAY_BUFFER_H_

struct MOAUDIO_DESC;

namespace OAL1
{
  class MOAudioBufferImpl : public GUnknown
  {
    const static int c_nMaxBufferCount = 4;
    ALuint  m_idBuffers[c_nMaxBufferCount];
    int     m_nBufferCount;
    int     m_nLoop;
    ALenum  m_eFormat;
    ALsizei m_nFrequency;
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef();
    virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  public:
    MOAudioBufferImpl(int nBufferCount)
      : m_nBufferCount(nBufferCount)
      , m_eFormat(0)
      , m_nFrequency(0)
      , m_nLoop(0)
    {
#ifdef _DEBUG
      memset(m_idBuffers, 0, sizeof(m_idBuffers));
#endif // #ifdef _DEBUG
    }
    GXBOOL Initialize();
    GXBOOL Finalize();

    ALuint IntGetCurrentBuffer();

    GXINT  GetBufferCount ();
    GXBOOL SetProperty    (MOAUDIO_DESC* pDesc);
    GXUINT SetData        (GXLPBYTE pData, GXSIZE_T nSize);
  };
}
#endif // #ifndef _AUDIO_PLAY_BUFFER_H_