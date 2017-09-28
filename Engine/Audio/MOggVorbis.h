#ifndef MAROMO_OGG_VORBIS_H_
#define MAROMO_OGG_VORBIS_H_

// 内部头文件, 不要当作接口暴露
#pragma pack(push)
#pragma pack(8)
#include "OggVorbis\include\vorbis\vorbisfile.h"

struct MOAUDIO_DESC;
struct OggVorbis_File;

class MOAudioData : public GUnknown
{
public:
  GXSTDINTERFACE(GXHRESULT  AddRef       ());
  GXSTDINTERFACE(GXHRESULT  Release      ());
  GXSTDINTERFACE(GXBOOL     GetProperty  (MOAUDIO_DESC* pDesc));
  GXSTDINTERFACE(GXSIZE_T   GetData      (GXLPBYTE pData, GXSIZE_T nSize));
};


class OggVorbis : public MOAudioData
{
  OggVorbis_File* m_pOVFile;
  vorbis_info*    m_pVorbisInfo;

  u32             m_uFrequency;
  u32             m_uFormat;
  u32             m_uChannels;
  u32             m_uBufferSize;
  u32             m_uBytesWritten;
  char*           m_pDecodeBuffer;

#ifdef DELAY_LOAD_OGGVORBIS
  typedef int             (*LPOVCLEAR)        (OggVorbis_File *vf);
  typedef long            (*LPOVREAD)         (OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
  typedef ogg_int64_t     (*LPOVPCMTOTAL)     (OggVorbis_File *vf,int i);
  typedef vorbis_info*    (*LPOVINFO)         (OggVorbis_File *vf,int link);
  typedef vorbis_comment* (*LPOVCOMMENT)      (OggVorbis_File *vf,int link);
  typedef int             (*LPOVOPENCALLBACKS)(void *datasource, OggVorbis_File *vf,char *initial, long ibytes, ov_callbacks callbacks);

  //static HINSTANCE          s_hVorbisFileDLL;
  static LPOVCLEAR          fn_clear;
  static LPOVREAD           fn_read;
  static LPOVPCMTOTAL       fn_pcm_total;
  static LPOVINFO           fn_info;
  static LPOVCOMMENT        fn_comment;
  static LPOVOPENCALLBACKS  fn_open_callbacks;
#endif // #ifdef DELAY_LOAD_OGGVORBIS

  //static size_t ov_read_func  (void* ptr, size_t size, size_t nmemb, void* datasource);
  //static int    ov_seek_func  (void* datasource, ogg_int64_t offset, int whence);
  //static int    ov_close_func (void* datasource);
  //static long   ov_tell_func  (void* datasource);

  static ov_callbacks  s_Callbacks;
protected:
  GXBOOL  Initialize      ();
  GXBOOL  Finalize        ();
  GXBOOL  BuildBuffer     ();
  u32     DecodeOggVorbis (char *pDecodeBuffer, size_t ulBufferSize);
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef      ();
  virtual GXHRESULT Release     ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXBOOL    GetProperty (MOAUDIO_DESC* pDesc);
  virtual GXSIZE_T  GetData     (GXLPBYTE pData, GXSIZE_T nSize);

  virtual size_t StreamRead (void* ptr, size_t size, size_t nmemb) = NULL;
  virtual int    StreamSeek (ogg_int64_t offset, int whence) = NULL;
  virtual int    StreamClose() = NULL;
  virtual long   StreamTell () = NULL;

  int Clear();
  long Read(char* buffer, int length, int bigendianp, int word, int sgned, int* bitstream);
  ogg_int64_t PCMTotal(int i);
  vorbis_info* Info(int link);
  vorbis_comment* Comment(int link);
  //int OpenCallbacks(void* datasource, char* initial, long ibytes, ov_callbacks callbacks);


  OggVorbis();

};


class OggVorbisFromFile : public OggVorbis
{
public:
  //FILE* fp;
  clFile file;
public:
  virtual size_t StreamRead (void* ptr, size_t size, size_t nmemb);
  virtual int    StreamSeek (ogg_int64_t offset, int whence);
  virtual int    StreamClose();
  virtual long   StreamTell ();

  OggVorbisFromFile()/* : fp(NULL) */{}
  ~OggVorbisFromFile();
  bool OpenFromFile(const char* file);
};
#pragma pack(pop)
#endif // #ifndef MAROMO_OGG_VORBIS_H_