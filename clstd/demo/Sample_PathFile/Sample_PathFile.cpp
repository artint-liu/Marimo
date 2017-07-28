// Sample_PathFile.cpp : 定义控制台应用程序的入口点。
//
#include "clstd.h"
#include "clString.h"
#include "clPathFile.h"

#ifdef _DEBUG
# pragma comment(lib, "clstd_d.lib")
#else
# pragma comment(lib, "clstd.lib")
#endif

void TestFindFile()
{
  CLOG(__FUNCTION__);

  clStringA strCurDir;
  clpathfile::GetCurrentDirectory(strCurDir);

  CLOG("CurrentDirectory:%s", strCurDir.CStr());

  const char* szFindFile[] = {
    "*.*",
    "Samp*",
    "notfound*",
  };

  for(int i = 0; i < countof(szFindFile); i++)
  {
    clstd::FindFile find_file;

    CLOG("");
    CLOG("");
    CLOG("Find %s", szFindFile[i]);

    if(find_file.NewFind(szFindFile[i]))
    {
      clstd::FINDFILEDATAA sFindFileData;
      int index = 1;
      while(find_file.GetFile(&sFindFileData))
      {
        if(TEST_FLAG(sFindFileData.dwAttributes, clstd::FileAttribute_Directory))
        {
          //CLOGW(_CLTEXT("(%d)\t%s\t%s"), index++, _CLTEXT("   <DIR>"), sFindFileData.Filename);
          CLOG("(%d)\t%s\t%s", index++, ("   <DIR>"), sFindFileData.Filename);
        }
        else
        {
          //CLOGW(_CLTEXT("(%d)\t%8d\t%s"), index++, sFindFileData.nFileSizeLow, sFindFileData.Filename);
          clStringA strTime((clStringA::U64)sFindFileData.nLastAccessTime);
          CLOG("(%d)\t%8d\t%s\t%s", index++, sFindFileData.nFileSizeLow, sFindFileData.Filename, strTime.CStr());
        }
      }
    }
  }
}

int main(int argc, char* argv[])
{
  TestFindFile();
	return 0;
}

