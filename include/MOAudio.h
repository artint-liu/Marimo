#ifndef _MARIMO_AUDIO_H_
#define _MARIMO_AUDIO_H_

#define MARIMO_AUDIO_OPENAL_1             GXMAKEFOURCC('O','A','L','1')
#define MOAUDIO_CREATION_FLAG_MULTITHREAD 0x00000001
struct MOAUDIO_DESC
{
  int       nBits;
  int       nChannels;
  int       nFrequency;
  GXSIZE_T  nBufferSize;  // ����Դ�ṩ�Ľ��黺������С��
                          // ����Ļ������ᵼ�½���ʱ���������С�ᵼ�²���ʱ��̶ܶ����ݿ��ܻ�û�в���������
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