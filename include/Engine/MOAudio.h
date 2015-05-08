#ifndef _MARIMO_AUDIO_H_
#define _MARIMO_AUDIO_H_

#define MARIMO_AUDIO_OPENAL_1             GXMAKEFOURCC('O','A','L','1')
#define MOAUDIO_CREATION_FLAG_MULTITHREAD 0x00000001
struct MOAUDIO_DESC
{
  int       nBits;
  int       nChannels;
  int       nFrequency;
  GXSIZE_T  nBufferSize;  // 数据源提供的建议缓冲区大小，
                          // 过大的缓冲区会导致解码时间过长，过小会导致播放时间很短而数据可能还没有补充上来。
};

class MOAudio : public GUnknown
{
public:
  enum StreamType
  {
    RAW_STREAM,
  };
//public:
//  MOAudio();
//  ~MOAudio();

public:
  GXSTDINTERFACE(GXHRESULT AddRef   ());
  GXSTDINTERFACE(GXHRESULT Release  ());

  GXSTDINTERFACE(GXHRESULT Tick     (GXDWORD dwDelta));
  GXSTDINTERFACE(GXHRESULT PlayFromFileW(GXLPCWSTR szFilename));
};


#endif // _MARIMO_AUDIO_H_