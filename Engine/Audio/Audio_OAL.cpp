/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 *        the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *        and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 *        promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// Win32 version of the Creative Labs OpenAL 1.1 Framework for samples
#include <windows.h>
#include <stdio.h>

//#include "clstd.h"
#include "GrapX.H"
#include "GUnknown.H"
#include "GXKernel.H"
#include "GameEngine.h"
#include "MOAudio.H"
#include "Audio_OAL.h"
//#include "CWaves.h"
#include "aldlist.h"

#include "MOggVorbis.h"
#include "AudioSource_OAL.h"

//static CWaves *g_pWaveLoader = NULL;

// Imported EFX functions

// Effect objects
LPALGENEFFECTS    alGenEffects = NULL;
LPALDELETEEFFECTS alDeleteEffects = NULL;
LPALISEFFECT      alIsEffect = NULL;
LPALEFFECTI       alEffecti = NULL;
LPALEFFECTIV      alEffectiv = NULL;
LPALEFFECTF       alEffectf = NULL;
LPALEFFECTFV      alEffectfv = NULL;
LPALGETEFFECTI    alGetEffecti = NULL;
LPALGETEFFECTIV   alGetEffectiv = NULL;
LPALGETEFFECTF    alGetEffectf = NULL;
LPALGETEFFECTFV   alGetEffectfv = NULL;

//Filter objects
LPALGENFILTERS    alGenFilters = NULL;
LPALDELETEFILTERS alDeleteFilters = NULL;
LPALISFILTER      alIsFilter = NULL;
LPALFILTERI       alFilteri = NULL;
LPALFILTERIV      alFilteriv = NULL;
LPALFILTERF       alFilterf = NULL;
LPALFILTERFV      alFilterfv = NULL;
LPALGETFILTERI    alGetFilteri = NULL;
LPALGETFILTERIV   alGetFilteriv = NULL;
LPALGETFILTERF    alGetFilterf = NULL;
LPALGETFILTERFV   alGetFilterfv = NULL;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS     alGenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS  alDeleteAuxiliaryEffectSlots = NULL;
LPALISAUXILIARYEFFECTSLOT       alIsAuxiliaryEffectSlot = NULL;
LPALAUXILIARYEFFECTSLOTI        alAuxiliaryEffectSloti = NULL;
LPALAUXILIARYEFFECTSLOTIV       alAuxiliaryEffectSlotiv = NULL;
LPALAUXILIARYEFFECTSLOTF        alAuxiliaryEffectSlotf = NULL;
LPALAUXILIARYEFFECTSLOTFV       alAuxiliaryEffectSlotfv = NULL;
LPALGETAUXILIARYEFFECTSLOTI     alGetAuxiliaryEffectSloti = NULL;
LPALGETAUXILIARYEFFECTSLOTIV    alGetAuxiliaryEffectSlotiv = NULL;
LPALGETAUXILIARYEFFECTSLOTF     alGetAuxiliaryEffectSlotf = NULL;
LPALGETAUXILIARYEFFECTSLOTFV    alGetAuxiliaryEffectSlotfv = NULL;

// XRAM functions and enum values

LPEAXSETBUFFERMODE eaxSetBufferMode = NULL;
LPEAXGETBUFFERMODE eaxGetBufferMode = NULL;

ALenum eXRAMSize = 0;
ALenum eXRAMFree = 0;
ALenum eXRAMAuto = 0;
ALenum eXRAMHardware = 0;
ALenum eXRAMAccessible = 0;


ALboolean ALFWInitOpenAL()
{
  ALDeviceList *pDeviceList = NULL;
  ALCcontext *pContext = NULL;
  ALCdevice *pDevice = NULL;
  ALint i = 0;
  ALboolean bReturn = AL_FALSE;

  pContext = alcGetCurrentContext();
  if(pContext != NULL) {
    return true;
  }

  pDeviceList = new ALDeviceList();
  if ((pDeviceList) && (pDeviceList->GetNumDevices()))
  {
    //ALFWprintf("\nSelect OpenAL Device:\n");
    //for (i = 0; i < pDeviceList->GetNumDevices(); i++) 
    //  ALFWprintf("%d. %s%s\n", i + 1, pDeviceList->GetDeviceName(i), i == pDeviceList->GetDefaultDevice() ? "(DEFAULT)" : "");
    i = pDeviceList->GetDefaultDevice();
    //do {
    //  ALchar ch = _getch();
    //  i = atoi(&ch);
    //} while ((i < 1) || (i > pDeviceList->GetNumDevices()));

    pDevice = alcOpenDevice(pDeviceList->GetDeviceName(i));
    if (pDevice)
    {
      pContext = alcCreateContext(pDevice, NULL);
      if (pContext)
      {
        //ALFWprintf("\nOpened %s Device\n", alcGetString(pDevice, ALC_DEVICE_SPECIFIER));
        alcMakeContextCurrent(pContext);
        bReturn = AL_TRUE;
      }
      else
      {
        alcCloseDevice(pDevice);
      }
    }
  }

  SAFE_DELETE(pDeviceList);
  return bReturn;
}

ALboolean ALFWShutdownOpenAL()
{
  TRACE("ALFWShutdownOpenAL()\n");
  ALCcontext *pContext;
  ALCdevice *pDevice;

  pContext = alcGetCurrentContext();
  if(pContext) {
    pDevice = alcGetContextsDevice(pContext);

    if(pDevice) {
      alcMakeContextCurrent(NULL);
      alcDestroyContext(pContext);
      alcCloseDevice(pDevice);
    }
  }

  return AL_TRUE;
}


// Extension Queries
ALboolean ALFWIsXRAMSupported()
{
  ALboolean bXRAM = AL_FALSE;
  
  if (alIsExtensionPresent("EAX-RAM") == AL_TRUE)
  {
    // Get X-RAM Function pointers
    eaxSetBufferMode = (EAXSetBufferMode)alGetProcAddress("EAXSetBufferMode");
    eaxGetBufferMode = (EAXGetBufferMode)alGetProcAddress("EAXGetBufferMode");

    if (eaxSetBufferMode && eaxGetBufferMode)
    {
      eXRAMSize = alGetEnumValue("AL_EAX_RAM_SIZE");
      eXRAMFree = alGetEnumValue("AL_EAX_RAM_FREE");
      eXRAMAuto = alGetEnumValue("AL_STORAGE_AUTOMATIC");
      eXRAMHardware = alGetEnumValue("AL_STORAGE_HARDWARE");
      eXRAMAccessible = alGetEnumValue("AL_STORAGE_ACCESSIBLE");

      if (eXRAMSize && eXRAMFree && eXRAMAuto && eXRAMHardware && eXRAMAccessible)
        bXRAM = AL_TRUE;
    }
  }

  return bXRAM;
}

ALboolean ALFWIsEFXSupported()
{
  ALCdevice *pDevice = NULL;
  ALCcontext *pContext = NULL;
  ALboolean bEFXSupport = AL_FALSE;

  pContext = alcGetCurrentContext();
  pDevice = alcGetContextsDevice(pContext);

  if (alcIsExtensionPresent(pDevice, (ALCchar*)ALC_EXT_EFX_NAME))
  {
    // Get function pointers
    *(void**)&alGenEffects                  = alGetProcAddress("alGenEffects");
    *(void**)&alDeleteEffects               = alGetProcAddress("alDeleteEffects");
    *(void**)&alIsEffect                    = alGetProcAddress("alIsEffect");
    *(void**)&alEffecti                     = alGetProcAddress("alEffecti");
    *(void**)&alEffectiv                    = alGetProcAddress("alEffectiv");
    *(void**)&alEffectf                     = alGetProcAddress("alEffectf");
    *(void**)&alEffectfv                    = alGetProcAddress("alEffectfv");
    *(void**)&alGetEffecti                  = alGetProcAddress("alGetEffecti");
    *(void**)&alGetEffectiv                 = alGetProcAddress("alGetEffectiv");
    *(void**)&alGetEffectf                  = alGetProcAddress("alGetEffectf");
    *(void**)&alGetEffectfv                 = alGetProcAddress("alGetEffectfv");
    *(void**)&alGenFilters                  = alGetProcAddress("alGenFilters");
    *(void**)&alDeleteFilters               = alGetProcAddress("alDeleteFilters");
    *(void**)&alIsFilter                    = alGetProcAddress("alIsFilter");
    *(void**)&alFilteri                     = alGetProcAddress("alFilteri");
    *(void**)&alFilteriv                    = alGetProcAddress("alFilteriv");
    *(void**)&alFilterf                     = alGetProcAddress("alFilterf");
    *(void**)&alFilterfv                    = alGetProcAddress("alFilterfv");
    *(void**)&alGetFilteri                  = alGetProcAddress("alGetFilteri");
    *(void**)&alGetFilteriv                 = alGetProcAddress("alGetFilteriv");
    *(void**)&alGetFilterf                  = alGetProcAddress("alGetFilterf");
    *(void**)&alGetFilterfv                 = alGetProcAddress("alGetFilterfv");
    *(void**)&alGenAuxiliaryEffectSlots     = alGetProcAddress("alGenAuxiliaryEffectSlots");
    *(void**)&alDeleteAuxiliaryEffectSlots  = alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    *(void**)&alIsAuxiliaryEffectSlot       = alGetProcAddress("alIsAuxiliaryEffectSlot");
    *(void**)&alAuxiliaryEffectSloti        = alGetProcAddress("alAuxiliaryEffectSloti");
    *(void**)&alAuxiliaryEffectSlotiv       = alGetProcAddress("alAuxiliaryEffectSlotiv");
    *(void**)&alAuxiliaryEffectSlotf        = alGetProcAddress("alAuxiliaryEffectSlotf");
    *(void**)&alAuxiliaryEffectSlotfv       = alGetProcAddress("alAuxiliaryEffectSlotfv");
    *(void**)&alGetAuxiliaryEffectSloti     = alGetProcAddress("alGetAuxiliaryEffectSloti");
    *(void**)&alGetAuxiliaryEffectSlotiv    = alGetProcAddress("alGetAuxiliaryEffectSlotiv");
    *(void**)&alGetAuxiliaryEffectSlotf     = alGetProcAddress("alGetAuxiliaryEffectSlotf");
    *(void**)&alGetAuxiliaryEffectSlotfv    = alGetProcAddress("alGetAuxiliaryEffectSlotfv");

    if (alGenEffects && alDeleteEffects && alIsEffect && alEffecti && alEffectiv && alEffectf &&
      alEffectfv && alGetEffecti && alGetEffectiv && alGetEffectf && alGetEffectfv && alGenFilters &&
      alDeleteFilters && alIsFilter && alFilteri && alFilteriv && alFilterf && alFilterfv &&
      alGetFilteri && alGetFilteriv && alGetFilterf && alGetFilterfv && alGenAuxiliaryEffectSlots &&
      alDeleteAuxiliaryEffectSlots && alIsAuxiliaryEffectSlot && alAuxiliaryEffectSloti &&
      alAuxiliaryEffectSlotiv && alAuxiliaryEffectSlotf && alAuxiliaryEffectSlotfv &&
      alGetAuxiliaryEffectSloti && alGetAuxiliaryEffectSlotiv && alGetAuxiliaryEffectSlotf &&
      alGetAuxiliaryEffectSlotfv)
      bEFXSupport = AL_TRUE;
  }

  return bEFXSupport;
}

namespace OAL1
{
#ifdef _DEBUG
  u32 MOAudioOALImpl::s_idOpenALThread;
#endif // #ifdef _DEBUG

  MOAudioOALImpl::MOAudioOALImpl()
  {
  }

  MOAudioOALImpl::~MOAudioOALImpl()
  {
  }

  GXHRESULT MOAudioOALImpl::IntGetSource(MOAudioSourceImpl** ppSource)
  {
    CHECK_THREAD_CONTEXT;
    MOAudioSourceImpl* pSource = NULL;
    if( ! m_listUnused.empty()) {
      pSource = m_listUnused.front();
      m_listUnused.pop_front();
    }
    else
    {
      pSource = new MOAudioSourceImpl();
      if( ! InlCheckNewAndIncReference(pSource)) {
        return GX_FAIL;
      }

      if( ! pSource->Initialize()) {
        pSource->Release();
        pSource = NULL;
        return GX_FAIL;
      }
    }

    if(pSource)
    {
      *ppSource = pSource;
      m_listPlaying.push_back(pSource);
      pSource->AddRef();
    }
    return GX_OK;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MOAudioOALImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT MOAudioOALImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      Finalize();
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXHRESULT MOAudioOALImpl::Tick(GXDWORD dwDelta)
  {
    // 这个只是为了避免初期测试时播放源无止境增长的问题, 如果实际游戏中同时播放很多声音的话m_listPlaying数量超过限制是正常的
    ASSERT(m_listPlaying.size() <= c_MaxUnusedSource && m_listUnused.size() <= c_MaxUnusedSource);

    for(AudioSourceList::iterator it = m_listPlaying.begin();
      it != m_listPlaying.end();)
    {
      MOAudioSourceImpl* pSource = *it;
      if( ! pSource->Tick(dwDelta))
      {
        if(m_listUnused.size() >= c_MaxUnusedSource) {
          pSource->Release();
          pSource = NULL;
        }
        else {
          m_listUnused.push_back(pSource);
        }
        it = m_listPlaying.erase(it);
        continue;
      }
      ++it;
    }
    return 0;
  }

  GXHRESULT MOAudioOALImpl::IntPlay(MOAudioData* pAudioData)
  {
    CHECK_THREAD_CONTEXT;
    //TRACE(__FUNCTION__"\n");

    MOAudioSourceImpl* pSource = NULL;
    GXHRESULT hval = IntGetSource(&pSource);
    if(GXFAILED(hval)) {
      return hval;
    }

    hval = pSource->SetAudioData(pAudioData);
    if(GXSUCCEEDED(hval)) {
      hval = pSource->Play();
    }

    SAFE_RELEASE(pSource);
    return hval;
  }

  GXHRESULT MOAudioOALImpl::PlayFromFileW(GXLPCWSTR szFilename)
  {
    OggVorbisFromFile* pOgg = new OggVorbisFromFile();
    if( ! InlCheckNewAndIncReference(pOgg))
    {
      return GX_FAIL;
    }

    if( ! pOgg->OpenFromFile(clStringA(szFilename)))
    {
      pOgg->Release();
      pOgg = NULL;
    }

    GXHRESULT hval = IntPlay(pOgg);

    SAFE_RELEASE(pOgg);
    return hval;
  }

  GXHRESULT MOAudioOALImpl::PlayByObject(MOAudioData* pAudioData)
  {
    return IntPlay(pAudioData);
  }

  GXBOOL MOAudioOALImpl::Initialize()
  {
    SETUP_THREAD_ID;
    return ALFWInitOpenAL();
  }

  GXBOOL MOAudioOALImpl::Finalize()
  {
    for(AudioSourceList::iterator it = m_listPlaying.begin();
      it != m_listPlaying.end(); ++it) {
      SAFE_RELEASE(*it);
    }

    for(AudioSourceList::iterator it = m_listUnused.begin();
      it != m_listUnused.end(); ++it) {
      SAFE_RELEASE(*it);
    }

    ALFWShutdownOpenAL();
    return TRUE;
  }

  GXHRESULT MOAudioOALImpl::CreateAudio(MOAudioOALImpl** ppAudio)
  {
    MOAudioOALImpl* pAudio = new MOAudioOALImpl;
    if( ! InlCheckNewAndIncReference(pAudio)) {
      return GX_FAIL;
    }

    if( ! pAudio->Initialize())
    {
      pAudio->Release();
      pAudio = NULL;
      return GX_FAIL;
    }
    *ppAudio = pAudio;

    return GX_OK;
  }
  //////////////////////////////////////////////////////////////////////////
  MOAudioOAL_MTImpl::MOAudioOAL_MTImpl(MOAudioOALImpl* pAudio)
    : m_pAudio(pAudio)
  {
  }

  MOAudioOAL_MTImpl::~MOAudioOAL_MTImpl()
  {
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT MOAudioOAL_MTImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT MOAudioOAL_MTImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      Finalize();
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXHRESULT MOAudioOAL_MTImpl::Tick(GXDWORD dwDelta)
  {
    return GX_OK;
  }

  GXHRESULT MOAudioOAL_MTImpl::PlayFromFileW(GXLPCWSTR szFilename)
  {
    OggVorbisFromFile* pOgg = new OggVorbisFromFile();
    if( ! InlCheckNewAndIncReference(pOgg))
    {
      return GX_FAIL;
    }

    GXHRESULT hval = GX_FAIL;
    if(pOgg->OpenFromFile(clStringA(szFilename)))
    {
      AUDIOTHREADMSG msg;
      msg.message = AM_PLAY;
      msg.lParam = (GXLPARAM)pOgg;
      hval = pOgg->AddRef();
      PostMessageW(&msg);      
    }

    pOgg->Release();
    pOgg = NULL;

    return hval;
  }

  GXHRESULT MOAudioOAL_MTImpl::PlayByObject(MOAudioData* pAudioData)
  {
    AUDIOTHREADMSG msg;
    msg.message = AM_PLAY;
    msg.lParam = (GXLPARAM)pAudioData;
    pAudioData->AddRef();
    PostMessageW(&msg);
    return GX_OK;
  }

  GXBOOL MOAudioOAL_MTImpl::Initialize()
  {
    return Start();
  }

  GXBOOL MOAudioOAL_MTImpl::Finalize()
  {
    PostQuitMessage(0);
    WaitThreadQuit(-1);

    SAFE_RELEASE(m_pAudio);
    return TRUE;
  }

  GXHRESULT MOAudioOAL_MTImpl::CreateAudio(MOAudioOAL_MTImpl** ppAudio)
  {
    MOAudioOALImpl* pAudioST = new MOAudioOALImpl;
    if( ! InlCheckNewAndIncReference(pAudioST)) {
      return GX_FAIL;
    }

    MOAudioOAL_MTImpl* pAudioMT = new MOAudioOAL_MTImpl(pAudioST);
    if( ! InlCheckNewAndIncReference(pAudioMT)) {
      pAudioST->Release();
      pAudioST = NULL;
      return GX_FAIL;
    }

    if( ! pAudioMT->Initialize())
    {
      pAudioMT->Release();
      pAudioMT = NULL;
      return GX_FAIL;
    }
    
    *ppAudio = pAudioMT;
    return GX_OK;
  }

  //////////////////////////////////////////////////////////////////////////
  i32 MOAudioOAL_MTImpl::Run()
  {
    m_pAudio->Initialize();
    AUDIOTHREADMSG msg;
    i32 r;
    while(r = GetMessageTimeOut(&msg, 20))
    {
      if(r == -1) {
        m_pAudio->Tick(20);
      }
      else {
        switch(msg.message)
        {
        case AM_PLAY:
          {
            MOAudioData* pAudioData = (MOAudioData*)msg.lParam;
            if(pAudioData)
            {
              m_pAudio->IntPlay(pAudioData);
              pAudioData->Release();
              pAudioData = NULL;
            }
          }
          break;
        }
      }
    }
    m_pAudio->Finalize();
    return (i32)msg.handle;
  }

  GXLPCSTR FormatError(ALenum err)
  {
    switch(err)
    {
    case AL_NO_ERROR:           return "AL_NO_ERROR";
    case AL_INVALID_NAME:       return "AL_INVALID_NAME";
    case AL_ILLEGAL_ENUM:       return "AL_ILLEGAL_ENUM/AL_INVALID_ENUM";
    case AL_INVALID_VALUE:      return "AL_INVALID_VALUE";
    case AL_ILLEGAL_COMMAND:    return "AL_ILLEGAL_COMMAND/AL_INVALID_OPERATION";
    case AL_OUT_OF_MEMORY:      return "AL_OUT_OF_MEMORY";
    }
    return "<unknown>";
  }

} // namespace OAL1

extern "C"
{
  GAMEENGINE_API GXHRESULT MOCreateAudio(MOAudio** ppAudio, GXDWORD dwAudioInterfaceCC, GXDWORD dwFlags)
  {
    switch(dwAudioInterfaceCC)
    {
    case MARIMO_AUDIO_OPENAL_1:
      {
        GXHRESULT hval;
        if(TEST_FLAG(dwFlags, MOAUDIO_CREATION_FLAG_MULTITHREAD))
        {
          OAL1::MOAudioOAL_MTImpl* pAudioOALMT = NULL;
          hval = OAL1::MOAudioOAL_MTImpl::CreateAudio(&pAudioOALMT);
          *ppAudio = pAudioOALMT;
        }
        else {
          OAL1::MOAudioOALImpl* pAudioOAL = NULL;
          hval = OAL1::MOAudioOALImpl::CreateAudio(&pAudioOAL);
          *ppAudio = pAudioOAL;
        }
        return hval;
      }
      break;
    }
    return GX_FAIL;
  }
} // extern "C"