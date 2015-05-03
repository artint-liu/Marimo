#ifndef _AUDIO_PLAY_SOURCE_H_
#define _AUDIO_PLAY_SOURCE_H_

class MOAudioData;

namespace OAL1
{
  class MOAudioBufferImpl;

  class MOAudioSourceImpl : public GUnknown
  {
    ALuint m_idSource;
    MOAudioBufferImpl*  m_pAudioBuffer;
    MOAudioData*        m_pAudioData;
    GXLPBYTE            m_pBuffer;
    GXSIZE_T            m_nBufferSize;
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT AddRef();
    GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  public:
    MOAudioSourceImpl()
      : m_pAudioBuffer(NULL)
      , m_pAudioData  (NULL)
      , m_pBuffer     (NULL)
      , m_nBufferSize(0)
    {
    }
    ~MOAudioSourceImpl();

    GXBOOL Initialize();
    GXBOOL Finalize();

    GXHRESULT SetAudioBuffer  (MOAudioBufferImpl* pBuffer);
    GXHRESULT SetAudioData    (MOAudioData* pAudioData);
    GXBOOL    Tick            (GXDWORD dwTick);
    GXHRESULT Play            ();
  };
}
#endif // #ifndef _AUDIO_PLAY_SOURCE_H_