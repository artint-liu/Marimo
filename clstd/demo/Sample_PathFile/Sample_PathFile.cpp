// Sample_PathFile.cpp : 定义控制台应用程序的入口点。
//
#include "clstd.h"

#ifdef _DEBUG
# pragma comment(lib, "clstd_d.lib")
#else
# pragma comment(lib, "clstd.lib")
#endif

void TestFindFile()
{
  clstd::FindFile find_file;
  if(find_file.NewFind("*.*"))
  {
    clstd::FINDFILEDATAW sFindFileData;
    int index = 1;
    while(find_file.GetFile(&sFindFileData))
    {
      if(TEST_FLAG(sFindFileData.dwAttributes, clstd::FileAttribute_Directory))
      {
        CLOGW(_CLTEXT("(%d)\t%s\t%s"), index++, _CLTEXT("   <DIR>"), sFindFileData.Filename);
      }
      else
      {
        CLOGW(_CLTEXT("(%d)\t%8d\t%s"), index++, sFindFileData.nFileSizeLow, sFindFileData.Filename);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  TestFindFile();
	return 0;
}

