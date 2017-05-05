// Sample_String.cpp : 定义控制台应用程序的入口点。
//

#include <tchar.h>
#include <locale.h>

#include "clstd.h"
#include "clString.H"
#include "clPathFile.h"

void TestPathFile()
{
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
  clStringA strPath;
  clpathfile::CombinePath(strPath, "abc", "def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePath(strPath, "abc\\", "def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePath(strPath, "abc\\", "/def");
  ASSERT(strPath == "\\def");

  clpathfile::CombinePath(strPath, "abc\\def", "ghi");
  ASSERT(strPath == "abc\\def\\ghi");

  clpathfile::CombinePath(strPath, "abc\\def", "./ghi");
  ASSERT(strPath == "abc\\def\\ghi");

  clpathfile::CombinePath(strPath, "abc\\def", "../ghi");
  ASSERT(strPath == "abc\\ghi");

  clpathfile::CombinePath(strPath, "abc", "./def");
  ASSERT(strPath == "abc\\def");

  clpathfile::CombinePath(strPath, "abc", "../def");
  ASSERT(strPath == "def");

  clpathfile::CombinePath(strPath, "", "./abc");
  ASSERT(strPath == "abc");

  clpathfile::CombinePath(strPath, "", "../abc");
  ASSERT(strPath == "..\\abc");

  clpathfile::CombinePath(strPath, NULL, "./abc");
  ASSERT(strPath == "abc");

  clpathfile::CombinePath(strPath, NULL, "../abc");
  ASSERT(strPath == "..\\abc");

  clpathfile::CombinePath(strPath, "c:\\abc\\def", "d:\\ghi\\jkl");
  ASSERT(strPath == "d:\\ghi\\jkl");

  clpathfile::CombinePath(strPath, "c:\\abc\\def", "\\ghi\\jkl");
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

void TestLog()
{
  CLOG("log test");
  CLOG("日志输出测试");

  CLOG_WARNING("log test");
  CLOG_WARNING("日志输出测试");

  CLOG_ERROR("log test");
  CLOG_ERROR("日志输出测试");

  CLOGW(_CLTEXT("log test"));
  CLOGW(_CLTEXT("日志输出测试"));

  CLOG_WARNINGW(_CLTEXT("log test"));
  CLOG_WARNINGW(_CLTEXT("日志输出测试"));

  CLOG_ERRORW(_CLTEXT("log test"));
  CLOG_ERRORW(_CLTEXT("日志输出测试"));
}

void TestStringToFloat()
{
  const char* sz123 = "1234\000e4321";
  float v = clstd::xtof(sz123);
  ASSERT(v == 1234.0f);

  const char* szSamp001 = "123.456e-20";

  {
    v = clstd::xtof(szSamp001); // 1.23456013
    ASSERT(v < 1.0f);

    v = clstd::xtof(szSamp001, 0);
    ASSERT(v == 0.f);

    v = clstd::xtof(szSamp001, 1);
    ASSERT(v == 1.f);

    v = clstd::xtof(szSamp001, 2);
    ASSERT(v == 12.f);

    v = clstd::xtof(szSamp001, 3);
    ASSERT(v == 123.f);

    v = clstd::xtof(szSamp001, 4);
    ASSERT(v == 123.f);

    v = clstd::xtof(szSamp001, 5);
    ASSERT(v == 123.400002f);

    v = clstd::xtof(szSamp001, 6);
    ASSERT(v == 123.450005f);

    v = clstd::xtof(szSamp001, 7);
    ASSERT(v == 123.456009f);

    v = clstd::xtof(szSamp001, 8);
    ASSERT(v == 123.456009f);

    v = clstd::xtof(szSamp001, 9);
    ASSERT(v == 123.456009f);

    v = clstd::xtof(szSamp001, 10);
    ASSERT(v == 1.23456013f);

    v = clstd::xtof(szSamp001, 11);
    ASSERT(v < 1.f);

    v = clstd::xtof(szSamp001, 12);
    ASSERT(v < 1.f);
  }

  const char* szSamp002 = "123.456e20";
  {
    v = clstd::xtof(szSamp002); // 1.23456013
    ASSERT(v > 1.0e10f);

    v = clstd::xtof(szSamp002, 0);
    ASSERT(v == 0.f);

    v = clstd::xtof(szSamp002, 1);
    ASSERT(v == 1.f);

    v = clstd::xtof(szSamp002, 2);
    ASSERT(v == 12.f);

    v = clstd::xtof(szSamp002, 3);
    ASSERT(v == 123.f);

    v = clstd::xtof(szSamp002, 4);
    ASSERT(v == 123.f);

    v = clstd::xtof(szSamp002, 5);
    ASSERT(v == 123.400002f);

    v = clstd::xtof(szSamp002, 6);
    ASSERT(v == 123.450005f);

    v = clstd::xtof(szSamp002, 7);
    ASSERT(v == 123.456009f);

    v = clstd::xtof(szSamp002, 8);
    ASSERT(v == 123.456009f);

    v = clstd::xtof(szSamp002, 9);
    ASSERT(v == 12345.6006f);

    v = clstd::xtof(szSamp002, 10);
    ASSERT(v > 1.0e10f);

    v = clstd::xtof(szSamp002, 11);
    ASSERT(v > 1.0e10f);

    v = clstd::xtof(szSamp002, 12);
    ASSERT(v > 1.0e10f);
  }

}

int _tmain(int argc, _TCHAR* argv[])
{
  setlocale(LC_ALL, "");
  //cprintf();
  TestLog();
  TestCodec();
  TestCombinePath();
  TestMatchSpec();
  TestString();
  TestPathFile();
  TestStringResolve();
  TestCodec();
  TestStringToFloat();

	return 0;
}

