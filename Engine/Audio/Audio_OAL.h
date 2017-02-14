#ifndef _FRAMEWORK_H_ // Win32 version
#define _FRAMEWORK_H_

// Get some classic includes
//#include<Windows.h>
//#include<stdio.h>
//#include<io.h>
//#include<fcntl.h>
//#include<conio.h>
#include"al.h"
#include"alc.h"
#include"efx.h"
#include"efx-creative.h"
#include"xram.h"


// OpenAL initialization and shutdown
ALboolean ALFWInitOpenAL();
ALboolean ALFWShutdownOpenAL();

// File loading functions
ALboolean ALFWLoadWaveToBuffer(const char *szWaveFile, ALuint uiBufferID, ALenum eXRAMBufferMode = 0);

// Extension Queries 
ALboolean ALFWIsXRAMSupported();
ALboolean ALFWIsEFXSupported();


// EFX Extension function pointer variables

// Effect objects
extern LPALGENEFFECTS                 alGenEffects;
extern LPALDELETEEFFECTS              alDeleteEffects;
extern LPALISEFFECT                   alIsEffect;
extern LPALEFFECTI                    alEffecti;
extern LPALEFFECTIV                   alEffectiv;
extern LPALEFFECTF                    alEffectf;
extern LPALEFFECTFV                   alEffectfv;
extern LPALGETEFFECTI                 alGetEffecti;
extern LPALGETEFFECTIV                alGetEffectiv;
extern LPALGETEFFECTF                 alGetEffectf;
extern LPALGETEFFECTFV                alGetEffectfv;

// Filter objects
extern LPALGENFILTERS                 alGenFilters;
extern LPALDELETEFILTERS              alDeleteFilters;
extern LPALISFILTER                   alIsFilter;
extern LPALFILTERI                    alFilteri;
extern LPALFILTERIV                   alFilteriv;
extern LPALFILTERF                    alFilterf;
extern LPALFILTERFV                   alFilterfv;
extern LPALGETFILTERI                 alGetFilteri;
extern LPALGETFILTERIV                alGetFilteriv;
extern LPALGETFILTERF                 alGetFilterf;
extern LPALGETFILTERFV                alGetFilterfv;

// Auxiliary slot object
extern LPALGENAUXILIARYEFFECTSLOTS    alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT      alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI       alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV      alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF       alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV      alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI    alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV   alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF    alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV   alGetAuxiliaryEffectSlotfv;

// XRAM Extension function pointer variables and enum values

typedef ALboolean (__cdecl *LPEAXSETBUFFERMODE)(ALsizei n, ALuint *buffers, ALint value);
typedef ALenum    (__cdecl *LPEAXGETBUFFERMODE)(ALuint buffer, ALint *value);

extern LPEAXSETBUFFERMODE eaxSetBufferMode;
extern LPEAXGETBUFFERMODE eaxGetBufferMode;

// X-RAM Enum values
extern ALenum eXRAMSize, eXRAMFree;
extern ALenum eXRAMAuto, eXRAMHardware, eXRAMAccessible;
#define OALERRLOG(_MSG, _ERR)   CLOG_ERROR("OpenAL:"_MSG"(code:%s).\r\n", FormatError(_ERR));

#ifdef _DEBUG
#define SETUP_THREAD_ID       s_idOpenALThread = clstd::this_thread::GetId()
#define CHECK_THREAD_CONTEXT  ASSERT(s_idOpenALThread == clstd::this_thread::GetId())
#else
#define SETUP_THREAD_ID
#define CHECK_THREAD_CONTEXT
#endif // #ifdef _DEBUG

class MOAudioData;

namespace OAL1
{
  class MOAudioOALImpl;
  enum AUDIOMSGDEF
  {
    AM_NULL,
    //AM_PLAYFILE,
    AM_PLAY,
  };

  struct AUDIOTHREADMSG
  {
    void*         handle;
    u32           message;
    union {
      struct {
        float3    v3;
      };
      struct {
        GXWPARAM  wParam;
        GXLPARAM  lParam;
      };
    };
  };

  //class AudioThread : 
  //{
  //  MOAudioOALImpl* m_pAudio;
  //public:
  //  AudioThread(MOAudioOALImpl* pAudio)
  //    : m_pAudio(pAudio)
  //  {
  //  }
  //  i32 Run();
  //};

  //////////////////////////////////////////////////////////////////////////
  class MOAudioSourceImpl;
  class MOAudioOALImpl : public MOAudio
  {
    typedef cllist<MOAudioSourceImpl*> AudioSourceList;
    friend class AudioThread;
    friend class MOAudioOAL_MTImpl;

  public:
    //enum StreamType
    //{
    //  RAW_STREAM,
    //};
#ifdef _DEBUG
    static u32 s_idOpenALThread;
#endif // #ifdef _DEBUG
    static const int c_MaxUnusedSource = 10;
  private:
    AudioSourceList   m_listPlaying;
    AudioSourceList   m_listUnused;
  private:
    MOAudioOALImpl();
    ~MOAudioOALImpl();
    GXBOOL    Initialize    ();
    GXBOOL    Finalize      ();
    GXHRESULT IntGetSource  (MOAudioSourceImpl** ppSource);
    GXHRESULT IntPlay       (MOAudioData* pAudioData);
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        ();
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT Tick          (GXDWORD dwDelta);
    virtual GXHRESULT PlayFromFileW (GXLPCWSTR szFilename);
    virtual GXHRESULT PlayByObject  (MOAudioData* pAudioData);

  public:
    static GXHRESULT CreateAudio(MOAudioOALImpl** ppAudio);
  };

  class MOAudioOAL_MTImpl : public MOAudio, public clstd::MsgThreadT<AUDIOTHREADMSG>
  {
    friend class AudioThread;
  private:
    MOAudioOALImpl*   m_pAudio;
    //AudioThread*      m_pThread;
  private:
    MOAudioOAL_MTImpl(MOAudioOALImpl* pAudio);
    ~MOAudioOAL_MTImpl();
    GXBOOL    Initialize    ();
    GXBOOL    Finalize      ();
    i32       Run           ();
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        ();
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT Tick          (GXDWORD dwDelta);
    virtual GXHRESULT PlayFromFileW (GXLPCWSTR szFilename);
    virtual GXHRESULT PlayByObject  (MOAudioData* pAudioData);
  public:
    static GXHRESULT CreateAudio(MOAudioOAL_MTImpl** ppAudio);
  };

  GXLPCSTR FormatError(ALenum err);
} // namespace OAL1

#endif _FRAMEWORK_H_