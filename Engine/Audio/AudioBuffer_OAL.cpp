#include <GrapX.H>
#include <GXKernel.H>
#include <GUnknown.h>
#include <MOAudio.H>
#include "Audio_OAL.h"
#include "AudioBuffer_OAL.h"
namespace OAL1
{
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MOAudioBufferImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT MOAudioBufferImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      delete this;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXBOOL MOAudioBufferImpl::Initialize()
  {
    if(m_nBufferCount >= c_nMaxBufferCount) {
      return FALSE;
    }

    alGenBuffers(m_nBufferCount, m_idBuffers);

    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
      OALERRLOG("can not create audio buffer", err);
      return FALSE;
    }

    return TRUE;
  }

  GXBOOL MOAudioBufferImpl::Finalize()
  {
    alDeleteBuffers(m_nBufferCount, m_idBuffers);
    ASSERT(alGetError() == AL_NO_ERROR);
    return TRUE;
  }

  ALuint MOAudioBufferImpl::IntGetCurrentBuffer()
  {
    return m_idBuffers[m_nLoop];
  }

  GXINT MOAudioBufferImpl::GetBufferCount()
  {
    return m_nBufferCount;
  }

  GXBOOL MOAudioBufferImpl::SetProperty(MOAUDIO_DESC* pDesc)
  {
    m_nFrequency = pDesc->nFrequency;

    if(pDesc->nBits == 8)
    {
      switch(pDesc->nChannels)
      {
      case 1:
        m_eFormat = AL_FORMAT_MONO8;
        break;
      case 2:
        m_eFormat = AL_FORMAT_STEREO8;
        break;
      default:
        return FALSE;
      }
    }
    else if(pDesc->nBits == 16)
    {
      switch(pDesc->nChannels)
      {
      case 1:
        m_eFormat = AL_FORMAT_MONO16;
        break;
      case 2:
        m_eFormat = AL_FORMAT_STEREO16;
        break;
      case 4:
        m_eFormat = alGetEnumValue("AL_FORMAT_QUAD16");
        ASSERT(alGetError() == AL_NO_ERROR);
        break;
      case 6:
        m_eFormat = alGetEnumValue("AL_FORMAT_51CHN16");
        ASSERT(alGetError() == AL_NO_ERROR);
        break;
      default:
        return FALSE;
      }
    }
    else {
      return FALSE;
    }
    return TRUE;
  }

  GXUINT MOAudioBufferImpl::SetData(GXLPBYTE pData, GXSIZE_T nSize)
  {
    GXUINT uret = (GXUINT)m_idBuffers[m_nLoop];
    alBufferData(m_idBuffers[m_nLoop], m_eFormat, pData, nSize, m_nFrequency);
    ASSERT(alGetError() == AL_NO_ERROR);
    m_nLoop++;

    if(m_nLoop >= m_nBufferCount) {
      m_nLoop = 0;
    }
    return uret;
  }
}