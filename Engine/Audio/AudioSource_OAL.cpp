#include <GrapX.H>
#include "thread/clMessageThread.h"
#include <GrapX/GXKernel.H>
//#include <GrapX/GUnknown.h>
#include <Engine/MOAudio.H>
#include "Audio_OAL.h"
#include "AudioSource_OAL.h"
#include "AudioBuffer_OAL.h"
#include "MOggVorbis.h"
namespace OAL1
{
  MOAudioSourceImpl::~MOAudioSourceImpl()
  {
    SAFE_RELEASE(m_pAudioBuffer);
    SAFE_RELEASE(m_pAudioData);
    SAFE_DELETE(m_pBuffer);
    m_nBufferSize = 0;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MOAudioSourceImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT MOAudioSourceImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      delete this;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXBOOL MOAudioSourceImpl::Initialize()
  {
    alGenSources(1, &m_idSource);
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
      OALERRLOG("can not create \"Source\"", err);
      return FALSE;
    }

    MOAudioBufferImpl* pAudioBuffer = new MOAudioBufferImpl(2);
    if( ! InlCheckNewAndIncReference(pAudioBuffer))
    {
      return FALSE;
    }
    if( ! pAudioBuffer->Initialize()) {
      pAudioBuffer->Release();
      pAudioBuffer = NULL;
      return FALSE;
    }
    GXHRESULT hval = SetAudioBuffer(pAudioBuffer);
    SAFE_RELEASE(pAudioBuffer);
    return GXSUCCEEDED(hval);
  }

  GXBOOL MOAudioSourceImpl::Finalize()
  {
    alDeleteSources(1, &m_idSource);
    ASSERT(alGetError() == AL_NO_ERROR);
    return TRUE;
  }

  GXHRESULT MOAudioSourceImpl::SetAudioBuffer(MOAudioBufferImpl* pBuffer)
  {
    return InlSetNewObjectT(m_pAudioBuffer, pBuffer);
  }

  GXHRESULT MOAudioSourceImpl::SetAudioData(MOAudioData* pAudioData)
  {
    GXHRESULT hval = InlSetNewObjectT(m_pAudioData, pAudioData);
    if(GXFAILED(hval)) {
      return hval;
    }
    MOAUDIO_DESC Desc;
    pAudioData->GetProperty(&Desc);
    m_pAudioBuffer->SetProperty(&Desc);

    if(m_pBuffer != NULL)
    {
      if(m_nBufferSize != Desc.nBufferSize)
      {
        delete[] m_pBuffer;
        m_pBuffer = NULL;
      }
    }
    m_nBufferSize = Desc.nBufferSize;
    if(m_pBuffer == NULL) {
      m_pBuffer = new GXBYTE[m_nBufferSize];
    }

    int nBufferCount = m_pAudioBuffer->GetBufferCount();

    for(int i = 0; i < nBufferCount; i++)
    {
      GXSIZE_T nDecodeSize = m_pAudioData->GetData(m_pBuffer, m_nBufferSize);
      if(nDecodeSize == 0) {
        break;
      }

      ALuint idBuf = m_pAudioBuffer->SetData(m_pBuffer, nDecodeSize);
      alSourceQueueBuffers(m_idSource, 1, &idBuf);
      ASSERT(alGetError() == AL_NO_ERROR);
    }

    return hval;
  }

  GXBOOL MOAudioSourceImpl::Tick(GXDWORD dwTick)
  {
    ALint nBuffersProcessed = 0;
    alGetSourcei(m_idSource, AL_BUFFERS_PROCESSED, &nBuffersProcessed);
    ASSERT(alGetError() == AL_NO_ERROR);

    while(nBuffersProcessed)
    {
      ALuint idBuffer = 0;
      alSourceUnqueueBuffers(m_idSource, 1, &idBuffer);
      ASSERT(alGetError() == AL_NO_ERROR);
      GXSIZE_T nDecodeSize = m_pAudioData->GetData(m_pBuffer, m_nBufferSize);
      if(nDecodeSize == 0) {
        break;
      }

      ASSERT(idBuffer == m_pAudioBuffer->IntGetCurrentBuffer());
      GXUINT idBuf = m_pAudioBuffer->SetData(m_pBuffer, nDecodeSize);
      ASSERT(idBuf == idBuffer);

      alSourceQueueBuffers(m_idSource, 1, &idBuffer);
      ASSERT(alGetError() == AL_NO_ERROR);
      nBuffersProcessed--;
    }

    ALint eState;
    alGetSourcei(m_idSource, AL_SOURCE_STATE, &eState);
    if(eState != AL_PLAYING)
    {
      ALint idQueuedBuffers;
      alGetSourcei(m_idSource, AL_BUFFERS_QUEUED, &idQueuedBuffers);
      if (idQueuedBuffers) {
        alSourcePlay(m_idSource);
      }
      else {
        return FALSE;
      }
    }

    return nBuffersProcessed == 0;
  }

  GXHRESULT MOAudioSourceImpl::Play()
  {
    alSourcePlay(m_idSource);
    //ALint eState;
    //alGetSourcei(m_idSource, AL_SOURCE_STATE, &eState);
    ASSERT(alGetError() == AL_NO_ERROR);
    return 0;
  }
}