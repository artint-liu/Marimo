// Sample_String.cpp : 定义控制台应用程序的入口点。
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

  clpathfile::CombinePathA(strPath, "abc\\", "/def");
  ASSERT(strPath == "\\def");

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
  ASSERT(strPath == "..\\abc");

  clpathfile::CombinePathA(strPath, NULL, "./abc");
  ASSERT(strPath == "abc");

  clpathfile::CombinePathA(strPath, NULL, "../abc");
  ASSERT(strPath == "..\\abc");

  clpathfile::CombinePathA(strPath, "c:\\abc\\def", "d:\\ghi\\jkl");
  ASSERT(strPath == "d:\\ghi\\jkl");

  clpathfile::CombinePathA(strPath, "c:\\abc\\def", "\\ghi\\jkl");
  ASSERT(strPath == "c:\\ghi\\jkl");
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

  bresult = clpathfile::MatchSpec("fgg\\eyr\\abcd\\efg\\eqkl\\swj", "*\\abcd\\efg\\*");
}

void TestStringResolve() // 测试字符串切分
{
  char* test1 = "as,hello,world";
  char* test2 = "as,,,hello,world";
  clstd::StringUtility::Resolve(test1, clstd::strlenT(test1), ',', [](int i, const char* str, size_t len){
    clStringA sub_str(str, len);
    printf("%s\n", sub_str);
  });

  printf("--------------------\n");

  clstd::StringUtility::Resolve(test2, clstd::strlenT(test2), ',', [](int n, const char* str, size_t len){
    clStringA sub_str(str, len);
    printf("%s\n", sub_str);
  });
}

void TestCodec() // 测试unicode到ansi转换
{
  const ch* szTestStringA  =  "这是测试的简单中文字符串啊没有奇怪的字符，1234567890ABCabc";
  const wch* szTestStringW = L"这是测试的简单中文字符串啊没有奇怪的字符，1234567890ABCabc";

  {
    clStringA strA = szTestStringA;
    clStringW strW = strA;
    ASSERT(strW == szTestStringW);
  }

  {
    clStringW strW = szTestStringW;
    clStringA strA = strW;
    ASSERT(strA == szTestStringA);
  }
}

void TestCodec()
{
  const ch* szSampleStringA  =  "测试中文转换的简单字符串没有奇怪的字符，1234567890abcABC";
  const wch* szSampleStringW = L"测试中文转换的简单字符串没有奇怪的字符，1234567890abcABC";
  {
    clStringA strA = szSampleStringA;
    clStringW strW = strA;
    ASSERT(strW == szSampleStringW);
  }
  {
    clStringW strW = szSampleStringW;
    clStringA strA = strW;
    ASSERT(strA == szSampleStringA);
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
  TestCodec();
  TestCombinePath();
  TestMatchSpec();
  TestString();
  TestPathFile();
  TestStringResolve();
  TestCodec();

	return 0;
}

