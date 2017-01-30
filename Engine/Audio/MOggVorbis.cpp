#if defined(_WIN32) || defined(_WINDOWS)
#include <windows.h>
#endif // #if defined(_WIN32) || defined(_WINDOWS)

#include <GrapX.H>
//#include "GrapX/GUnknown.H"
#include "GrapX/GXKernel.H"
#include "Engine/MOAudio.H"
//#include <clstd.h>
//#include <clString.H>
//#include <clFile.H>

//#include "GameEngine.h"

#include <al.h>

#include "MOggVorbis.h"

#pragma comment(lib, "libogg.lib")
#pragma comment(lib, "libvorbis.lib")
#pragma comment(lib, "libvorbisfile.lib")
//#include<stdio.h>
//#include<io.h>
//#include<fcntl.h>
//#include<conio.h>

// 下面这些重构后去除
#define NUMBUFFERS              (4)
ALuint     uBuffers[NUMBUFFERS];
ALuint     uSource;
ALuint     uBuffer;
ALint      iState;
ALint      iLoop;


#ifdef DELAY_LOAD_OGGVORBIS
HINSTANCE s_hVorbisFileDLL = NULL;
OggVorbis::LPOVCLEAR          OggVorbis::fn_clear = NULL;
OggVorbis::LPOVREAD           OggVorbis::fn_read = NULL;
OggVorbis::LPOVPCMTOTAL       OggVorbis::fn_pcm_total = NULL;
OggVorbis::LPOVINFO           OggVorbis::fn_info = NULL;
OggVorbis::LPOVCOMMENT        OggVorbis::fn_comment = NULL;
OggVorbis::LPOVOPENCALLBACKS  OggVorbis::fn_open_callbacks = NULL;
#endif // #ifdef DELAY_LOAD_OGGVORBIS

size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  return ((OggVorbis*)datasource)->StreamRead(ptr, size, nmemb);
}

int ov_seek_func(void* datasource, ogg_int64_t offset, int whence)
{
  return ((OggVorbis*)datasource)->StreamSeek(offset, whence);
}

int ov_close_func(void* datasource)
{
  return ((OggVorbis*)datasource)->StreamClose();
}

long ov_tell_func(void* datasource)
{
  return ((OggVorbis*)datasource)->StreamTell();
}

ov_callbacks OggVorbis::s_Callbacks = {
  ov_read_func, ov_seek_func, ov_close_func, ov_tell_func
};

int OggVorbis::Clear()
{
  return ov_clear(&m_OVFile);
}

long OggVorbis::Read(char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream)
{
  return ov_read(&m_OVFile, buffer, length, bigendianp, word, sgned, bitstream);
}

ogg_int64_t OggVorbis::PCMTotal(int i)
{
  return ov_pcm_total(&m_OVFile, i);
}

vorbis_info* OggVorbis::Info(int link)
{
  return ov_info(&m_OVFile, link);
}

vorbis_comment* OggVorbis::Comment(int link)
{
  return ov_comment(&m_OVFile, link);
}

//int OggVorbis::OpenCallbacks(void *datasource, char *initial, long ibytes, ov_callbacks callbacks)
//{
//  return fn_open_callbacks(datasource, &vf, initial, ibytes, callbacks);
//}

OggVorbis::OggVorbis()
  : m_pVorbisInfo(NULL)
  , m_uFrequency(0)
  , m_uFormat(0)
  , m_uChannels(0)
  , m_uBufferSize(0)
  , m_uBytesWritten(0)
  , m_pDecodeBuffer(NULL)
{
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT OggVorbis::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT OggVorbis::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXBOOL OggVorbis::GetProperty(MOAUDIO_DESC* pDesc)
{
  if( ! m_pVorbisInfo) {
    return FALSE;
  }

  pDesc->nBits = 16;
  pDesc->nChannels = m_uChannels;
  pDesc->nFrequency = m_uFrequency;
  pDesc->nBufferSize = m_uBufferSize;
  return TRUE;
}

GXSIZE_T OggVorbis::GetData(GXLPBYTE pData, GXSIZE_T nSize)
{
  u32 nDecodeSize = DecodeOggVorbis((char*)pData, nSize);
  return nDecodeSize;
}

GXBOOL OggVorbis::Initialize()
{
#ifdef DELAY_LOAD_OGGVORBIS
  if(s_hVorbisFileDLL == NULL)
  {
    s_hVorbisFileDLL = LoadLibraryW(L"libvorbisfile");
    if (s_hVorbisFileDLL)
    {
      fn_clear          = (LPOVCLEAR)         GetProcAddress(s_hVorbisFileDLL, "ov_clear");
      fn_read           = (LPOVREAD)          GetProcAddress(s_hVorbisFileDLL, "ov_read");
      fn_pcm_total      = (LPOVPCMTOTAL)      GetProcAddress(s_hVorbisFileDLL, "ov_pcm_total");
      fn_info           = (LPOVINFO)          GetProcAddress(s_hVorbisFileDLL, "ov_info");
      fn_comment        = (LPOVCOMMENT)       GetProcAddress(s_hVorbisFileDLL, "ov_comment");
      fn_open_callbacks = (LPOVOPENCALLBACKS) GetProcAddress(s_hVorbisFileDLL, "ov_open_callbacks");

      int nret = fn_open_callbacks(this, &m_OVFile, NULL, 0, s_Callbacks);
      m_pVorbisInfo = fn_info(&vf, -1);
      return true;
    }
  }
#else
  InlSetZeroT(m_OVFile);
  int nret = ov_open_callbacks(this, &m_OVFile, NULL, 0, s_Callbacks);
  m_pVorbisInfo = ov_info(&m_OVFile, -1);
#endif // #ifdef DELAY_LOAD_OGGVORBIS

  if( ! m_pVorbisInfo) {
    return FALSE;
  }

  m_uFrequency = m_pVorbisInfo->rate;
  m_uChannels = m_pVorbisInfo->channels;

  BuildBuffer();

  /*
  // Allocate a buffer to be used to store decoded data for all Buffers
  m_pDecodeBuffer = new char[m_uBufferSize * 2];
  if ( ! m_pDecodeBuffer)
  {
    CLOG_ERROR("Failed to allocate memory for decoded OggVorbis data\n");
    ov_clear(&m_OVFile);
    //ShutdownVorbisFile();
    //ALFWShutdownOpenAL();
    //ALFWShutdown();
    return FALSE;
  }


  // Generate some AL Buffers for streaming
  alGenBuffers(NUMBUFFERS, uBuffers);

  // Generate a Source to playback the Buffers
  alGenSources(1, &uSource);

  // Fill all the Buffers with decoded audio data from the OggVorbis file
  for (iLoop = 0; iLoop < NUMBUFFERS; iLoop++)
  {
    m_uBytesWritten = DecodeOggVorbis(m_pDecodeBuffer, m_uBufferSize);
    if(m_uBytesWritten)
    {
      alBufferData(uBuffers[iLoop], m_uFormat, m_pDecodeBuffer, m_uBytesWritten, m_uFrequency);
      alSourceQueueBuffers(uSource, 1, &uBuffers[iLoop]);
    }
  }

  // Start playing source
  alSourcePlay(uSource);

  //ALint eState;
  //alGetSourcei(uSource, AL_SOURCE_STATE, &eState);

  while (1)
  {
    //Sleep( SERVICE_UPDATE_PERIOD );

    // Request the number of OpenAL Buffers have been processed (played) on the Source
    ALint iBuffersProcessed = 0;
    alGetSourcei(uSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

    // For each processed buffer, remove it from the Source Queue, read next chunk of audio
    // data from disk, fill buffer with new data, and add it to the Source Queue
    while (iBuffersProcessed)
    {
      // Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
      uBuffer = 0;
      alSourceUnqueueBuffers(uSource, 1, &uBuffer);

      // Read more audio data (if there is any)
      m_uBytesWritten = DecodeOggVorbis(m_pDecodeBuffer, m_uBufferSize);
      if (m_uBytesWritten)
      {
        alBufferData(uBuffer, m_uFormat, m_pDecodeBuffer, m_uBytesWritten, m_uFrequency);
        alSourceQueueBuffers(uSource, 1, &uBuffer);
      }

      iBuffersProcessed--;
    }

    // Check the status of the Source.  If it is not playing, then playback was completed,
    // or the Source was starved of audio data, and needs to be restarted.
    alGetSourcei(uSource, AL_SOURCE_STATE, &iState);
    if (iState != AL_PLAYING)
    {
      ALint iQueuedBuffers;
      // If there are Buffers in the Source Queue then the Source was starved of audio
      // data, so needs to be restarted (because there is more audio data to play)
      alGetSourcei(uSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
      if (iQueuedBuffers)
      {
        alSourcePlay(uSource);
      }
      else
      {
        // Finished playing
        break;
      }
    }
  }

  alSourceStop(uSource);
  alSourcei(uSource, AL_BUFFER, 0);

  // Clean up buffers and sources
  alDeleteSources( 1, &uSource );
  alDeleteBuffers( NUMBUFFERS, uBuffers );
  SAFE_DELETE(m_pDecodeBuffer);
  //*/

  return TRUE;
}

GXBOOL OggVorbis::BuildBuffer()
{
  if (m_pVorbisInfo->channels == 1)
  {
    m_uFormat = AL_FORMAT_MONO16;
    // Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
    m_uBufferSize = m_uFrequency >> 1;
    // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
    m_uBufferSize -= (m_uBufferSize % 2);
  }
  else if (m_pVorbisInfo->channels == 2)
  {
    m_uFormat = AL_FORMAT_STEREO16;
    // Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
    m_uBufferSize = m_uFrequency;
    // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
    m_uBufferSize -= (m_uBufferSize % 4);
  }
  else if (m_pVorbisInfo->channels == 4)
  {
    m_uFormat = alGetEnumValue("AL_FORMAT_QUAD16");
    // Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
    m_uBufferSize = m_uFrequency * 2;
    // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
    m_uBufferSize -= (m_uBufferSize % 8);
  }
  else if (m_pVorbisInfo->channels == 6)
  {
    m_uFormat = alGetEnumValue("AL_FORMAT_51CHN16");
    // Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
    m_uBufferSize = m_uFrequency * 3;
    // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
    m_uBufferSize -= (m_uBufferSize % 12);
  }

  if (m_uFormat == 0)
  {
    return FALSE;
  }

  return TRUE;
}

GXBOOL OggVorbis::Finalize()
{
  SAFE_DELETE(m_pDecodeBuffer);
  ov_clear(&m_OVFile);
  return TRUE;
}

u32 OggVorbis::DecodeOggVorbis(char *pDecodeBuffer, size_t uBufferSize)
{
  int current_section;
  long lDecodeSize;
  unsigned long ulSamples;
  short *pSamples;

  u32 uBytesDone = 0;
  while (1)
  {
    lDecodeSize = ov_read(&m_OVFile, pDecodeBuffer + uBytesDone, uBufferSize - uBytesDone, 0, 2, 1, &current_section);
    if (lDecodeSize > 0)
    {
      uBytesDone += lDecodeSize;

      if (uBytesDone >= uBufferSize)
        break;
    }
    else
    {
      break;
    }
  }

  // Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
  // however 6-Channels files need to be re-ordered
  if (m_uChannels == 6)
  {    
    pSamples = (short*)pDecodeBuffer;
    for (ulSamples = 0; ulSamples < (uBufferSize >> 1); ulSamples+=6)
    {
      // WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
      // OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
      clSwap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
      clSwap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
      clSwap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
    }
  }

  return uBytesDone;
}
//////////////////////////////////////////////////////////////////////////

OggVorbisFromFile::~OggVorbisFromFile()
{
  Finalize();
}

size_t OggVorbisFromFile::StreamRead(void* ptr, size_t size, size_t nmemb)
{
  u32 nRead;
  file.Read(ptr, size * nmemb, &nRead);
  return nRead;
}

int OggVorbisFromFile::StreamSeek(ogg_int64_t offset, int whence)
{
  return (int)file.SetPointer64(offset, whence);
}

int OggVorbisFromFile::StreamClose()
{
  file.Close();
  return 0;
}

long OggVorbisFromFile::StreamTell()
{
  return file.GetPointer();
}

bool OggVorbisFromFile::OpenFromFile(const char* szFile)
{
  if(file.OpenExisting(szFile))
  {
    Initialize();
    return true;
  }
  return false;
}