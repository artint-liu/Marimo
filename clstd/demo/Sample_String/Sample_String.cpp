﻿// Sample_String.cpp : 定义控制台应用程序的入口点。
//

#include <tchar.h>

#include "clstd.h"
#include "clString.H"
#include "clPathFile.h"

void TestPathFile()
{
  clStringA strPath;

  strPath = "abc";
  clpathfile::RenameExtensionA(strPath, ".exe");
  ASSERT(strPath == "abc.exe");

  clpathfile::RenameExtensionA(strPath, "txt");
  ASSERT(strPath == "abc.txt");

  //////////////////////////////////////////////////////////////////////////
}

void TestCombinePath()
{
  clStringA strPath;
  clpathfile::CombinePathA(strPath, "abc", "def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePathA(strPath, "abc\\", "def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePathA(strPath, "abc\\", "//def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePathA(strPath, "abc\\def", "ghi");
  ASSERT(strPath == "abc\\def\\ghi");

  clpathfile::CombinePathA(strPath, "abc\\def", "./ghi");
  ASSERT(strPath == "abc\\def\\ghi");

  clpathfile::CombinePathA(strPath, "abc\\def", "../ghi");
  ASSERT(strPath == "abc\\ghi");

  clpathfile::CombinePathA(strPath, "abc", "./def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePathA(strPath, "abc", "../def");
  ASSERT(strPath == "def");

  clpathfile::CombinePathA(strPath, "", "./abc");
  ASSERT(strPath == "abc");

  clpathfile::CombinePathA(strPath, "", "../abc");
  ASSERT(strPath == "../abc");

  clpathfile::CombinePathA(strPath, NULL, "./abc");
  ASSERT(strPath == "abc");

  clpathfile::CombinePathA(strPath, NULL, "../abc");
  ASSERT(strPath == "../abc");

}

void TestString()
{
  b32 bresult;
  clStringA str("abcd-efgh-ijkl-mnop");

  bresult = str.BeginsWith("ab");
  ASSERT(bresult == TRUE);

  bresult = str.EndsWith("mnop");
  ASSERT(bresult == TRUE);
}

void TestMatchSpec()
{
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
}

void TestStringResolve()
{
  char* test1 = "as,hello,world";
  char* test2 = "as,,,hello,world";
  clstd::StringUtility::Resolve(test1, clstd::strlenT(test1), ',', [](const char* str, size_t len){
    clStringA sub_str(str, len);
    printf("%s\n", sub_str);
  });

  printf("--------------------\n");

  clstd::StringUtility::Resolve(test2, clstd::strlenT(test2), ',', [](const char* str, size_t len){
    clStringA sub_str(str, len);
    printf("%s\n", sub_str);
  });

}

int _tmain(int argc, _TCHAR* argv[])
{
  TestCombinePath();
  TestMatchSpec();
  TestString();
  TestPathFile();
  TestStringResolve();

	return 0;
}
