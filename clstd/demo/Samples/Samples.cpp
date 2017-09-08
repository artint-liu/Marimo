// Sample_Standard.cpp : 定义控制台应用程序的入口点。
//
#include "clstd.h"
#include "Sample_Timer/Samples_Timer.h"

typedef void (CL_CALLBACK *SampleProc)();

struct SAMPLEITEM
{
  const char* szName;
  SampleProc  SampleEntry;
};

SAMPLEITEM g_aSamples[] = {
  {"Timer", clstd_sample::Sample_Timer},
};

int main(int argc, char* argv[])
{
  while(true)
  {
    for(int i = 0; i < countof(g_aSamples); i++)
    {
      printf("%d.%s\n", i, g_aSamples[i].szName);
    }

    char c = clstd_cli::getch();
    if(c == 27) {
      break;
    }
    int item = (int)(c - '0');
    if(item < countof(g_aSamples))
    {
      g_aSamples[item].SampleEntry();
    }
  }

  return 0;
}
