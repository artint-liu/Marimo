// Sample_PathFile.cpp : 定义控制台应用程序的入口点。
//
#include "clstd.h"
#include "clString.h"
#include "clPathFile.h"

#if defined(_CL_ARCH_X86)
# ifdef _DEBUG
#   pragma comment(lib, "clstd.Win32.Debug_MD.lib")
# else
#   pragma comment(lib, "clstd.Win32.Release_MD.lib")
# endif
#elif defined(_CL_ARCH_X64)
# ifdef _DEBUG
#   pragma comment(lib, "clstd.x64.Debug_MD.lib")
# else
#   pragma comment(lib, "clstd.x64.Release_MD.lib")
# endif
#endif

#define CLPATHFILE_FUNC(__x) __x; CLOG("%s => %s", #__x, strPath.CStr());

void TestPathFile()
{
  CLOG(__FUNCTION__);
  clStringA strPath;

  strPath = "abc";
  clpathfile::RenameExtension(strPath, ".exe");
  ASSERT(strPath == "abc.exe");

  clpathfile::RenameExtension(strPath, "txt");
  ASSERT(strPath == "abc.txt");

  //////////////////////////////////////////////////////////////////////////
}

void TestCombinePath()
{
  CLOG(__FUNCTION__);
  clStringA strPath;

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc", "def"));
  ASSERT(strPath == "abc\\def" || strPath == "abc/def");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc\\", "def"));
  ASSERT(strPath == "abc\\def" || strPath == "abc/def");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc\\", "/def"));
  ASSERT(strPath == "\\def" || strPath == "/def");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc\\def", "ghi"));
  ASSERT(strPath == "abc\\def\\ghi" || strPath == "abc/def/ghi" || strPath == "abc\\def/ghi");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc\\def", "./ghi"));
  ASSERT(strPath == "abc\\def\\ghi" || strPath == "abc/def/ghi" || strPath == "abc\\def/ghi");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc\\def", "../ghi"));
  ASSERT(strPath == "abc\\ghi" || strPath == "abc/ghi");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc", "./def"));
  ASSERT(strPath == "abc\\def" || strPath == "abc/def");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "abc", "../def"));
  ASSERT(strPath == "def");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "", "./abc"));
  ASSERT(strPath == "abc");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "", "../abc"));
  ASSERT(strPath == "..\\abc" || strPath == "../abc");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, NULL, "./abc"));
  ASSERT(strPath == "abc");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, NULL, "../abc"));
  ASSERT(strPath == "..\\abc" || strPath == "../abc");

#if defined(_CL_SYSTEM_WINDOWS)
  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "c:\\abc\\def", "d:\\ghi\\jkl"));
  ASSERT(strPath == "d:\\ghi\\jkl");

  CLPATHFILE_FUNC(clpathfile::CombinePath(strPath, "c:\\abc\\def", "\\ghi\\jkl"));
  ASSERT(strPath == "c:\\ghi\\jkl" || strPath == "c:/ghi/jkl");
#endif // #if defined(_CL_SYSTEM_WINDOWS)
}

void TestMatchSpec()
{
  CLOG(__FUNCTION__);
  // 文件匹配
  b32 bresult;
  bresult = clpathfile::MatchSpec("abcdefghijklmnopqrst", "abcd*opq");
  ASSERT(bresult == FALSE);

  bresult = clpathfile::MatchSpec("abcdefghijklmnopq", "abcd*opq");
  ASSERT(bresult == TRUE);

  bresult = clpathfile::MatchSpec("abcdefghijklmnopqrst", "abcd*opq*");
  ASSERT(bresult == TRUE);

  bresult = clpathfile::MatchSpec("abcdefghijklmnopq.rst", "*.rst");
  ASSERT(bresult == TRUE);

  bresult = clpathfile::MatchSpec("fgg\\eyr\\abcd\\efg\\eqkl\\swj", "*\\abcd\\efg\\*");
}


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
          CLOG("(%d)\t%s\t%s", index++, ("   <DIR>"), sFindFileData.cFileName);
        }
        else
        {
          //CLOGW(_CLTEXT("(%d)\t%8d\t%s"), index++, sFindFileData.nFileSizeLow, sFindFileData.Filename);
          clStringA strTime((clStringA::U64)sFindFileData.nLastAccessTime);
          CLOG("(%d)\t%20d\t%s\t%s", index++, sFindFileData.nFileSize, sFindFileData.cFileName, strTime.CStr());
        }
      }
    }
  }
}

int main(int argc, char* argv[])
{
  TestPathFile();
  TestCombinePath();
  TestMatchSpec();
  TestFindFile();
	return 0;
}

