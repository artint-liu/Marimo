// Sample_String.cpp : 定义控制台应用程序的入口点。
//

#define _CRT_SECURE_NO_WARNINGS
//#ifdef WIN32
//# include <tchar.h>
//#endif
#include <locale.h>

#include "clstd.h"
#include "clString.h"
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

#define CLPATHFILE_FUNC(__x) __x; CLOG("%s => %s", #__x, strPath.CStr());

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

class P
{
  char* p;
public:
  P(char* pp)
  {
    p = pp;
  }
};


void TestString()
{
  CLOG(__FUNCTION__);
  b32 bresult;
  clStringA str("abcd-efgh-ijkl-mnop");

  ASSERT(*(size_t*)&str == (size_t)str.CStr());

  bresult = str.BeginsWith("ab");
  ASSERT(bresult == TRUE);

  bresult = str.EndsWith("mnop");
  ASSERT(bresult == TRUE);
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

void TestStringResolve() // 测试字符串切分
{
  CLOG(__FUNCTION__);
  const char* test1 = "as,hello,world";
  const char* test2 = "as,,,hello,world";
  clstd::StringUtility::Resolve(test1, clstd::strlenT(test1), ',', [](int i, const char* str, size_t len){
    clStringA sub_str(str, len);
    CLOG("%s", (const char*)sub_str);
  });

  CLOG("--------------------");

  clstd::StringUtility::Resolve(test2, clstd::strlenT(test2), ',', [](int n, const char* str, size_t len){
    clStringA sub_str(str, len);
    CLOG("%s", (const char*)sub_str);
  });
}

void TestCodec() // 测试unicode到ansi转换
{
  CLOG(__FUNCTION__);

  const ch* szTestStringA  =  "这是测试的简单中文字符串啊没有奇怪的字符，1234567890ABCabc";
  const wch* szTestStringW = _CLTEXT("这是测试的简单中文字符串啊没有奇怪的字符，1234567890ABCabc");

  {
    clStringA strA = szTestStringA;
    clStringW strW = strA.CStr();
    ASSERT(strW == szTestStringW);
  }

  {
    clStringW strW = szTestStringW;
    clStringA strA = strW.CStr();
    ASSERT(strA == szTestStringA);
  }
}

void TestLog()
{
  CLOG(__FUNCTION__);
  CLOG("1.log test");
  CLOG("2.日志输出测试");

  CLOG_WARNING("3.log test");
  CLOG_WARNING("4.日志输出测试");

  CLOG_ERROR("5.log test");
  CLOG_ERROR("6.日志输出测试");

  CLOGW(_CLTEXT("7.log test"));
  CLOGW(_CLTEXT("8.日志输出测试"));

  CLOG_WARNINGW(_CLTEXT("9.log test"));
  CLOG_WARNINGW(_CLTEXT("10.日志输出测试"));

  CLOG_ERRORW(_CLTEXT("11.log test"));
  CLOG_ERRORW(_CLTEXT("12.日志输出测试"));
}

// 测试基本字符串操作
void TestBasicStringOp()
{
  CLOG(__FUNCTION__);
  clStringA str = "15";
  ASSERT(str == "15");

  str.Insert(1, "2");
  ASSERT(str == "125");

  str.Insert(2, "34");
  ASSERT(str == "12345");

  clStringA strNew("1234567890", 5);
  ASSERT(str == "12345");

  str = "12345556789";
  str.Replace(5, 2, "");
  ASSERT(str = "123456789");

  str = "1289";
  str.Replace(2, 0, "34567");
  ASSERT(str = "123456789");

  str = "2H31hrJHtgK91LJ3gLk2jKJrh3skKkh69G97t9god2";
  str.MakeLower();
  ASSERT(str == "2h31hrjhtgk91lj3glk2jkjrh3skkkh69g97t9god2");
  str.MakeUpper();
  ASSERT(str == "2H31HRJHTGK91LJ3GLK2JKJRH3SKKKH69G97T9GOD2");
}

void TestStringToFloat()
{
  CLOG(__FUNCTION__);
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

template<typename _Ty>
void TestFormatStrFunc(const ch** fmt_flags, int num_flags, const ch** fmt_width, int num_width, const ch** fmt_specifier, int nem_specifier, const _Ty* samp_int, int num_int)
{
  int index = 0;
  clStringA str;
  char buffer[1024];

  for(int f = 0; f < num_flags; f++)
  {
    for(int w = 0; w < num_width; w++)
    {
      for(int s = 0; s < nem_specifier; s++)
      {
        for(int i = 0; i < num_int; i++)
        {
          clStringA format_str;
          format_str.Reserve(20);
          format_str.Append('%');
          format_str.Append(fmt_flags[f]);
          format_str.Append(fmt_width[w]);
          format_str.Append(fmt_specifier[s]);

          sprintf(buffer, format_str, samp_int[i]);
          CLOG("[sprintf]:\"%s\"", buffer);

          // >>>>>>>>>>> Debug
          if(index == 0) {
            CLNOP
          }
          // <<<<<<<<<<< Debug

          str.Format(format_str, samp_int[i]);
          CLOG("[ string]:\"%s\"", str.CStr());

          if(str == buffer) {
            CLOG("[%d]%s <match>\n", index, format_str.CStr());
          }
          else {
            CLOG_WARNING("[%d]%s !NOT match!\n", index, format_str.CStr());
            CLBREAK;
          }

          if(clstd_cli::kbhit() && clstd_cli::getch() == 27) {
            return;
          }

          index++;
        }
      }
    }
  }
}

void TestFormatString0()
{
  CLOG(__FUNCTION__);

  //char buffer[1024];
  //sprintf(buffer, "%.1f", 0);
  //sprintf(buffer, "%.0f", 0);
  //sprintf(buffer, "%.f", 0);
  const ch* fmt_flags[] = {
    "", "-", "+", "#", " ", "0", "-0", "+0", "#0", "0#", "-#", "+#", "-+", "+-", "- ", " -", "+ ", " +"
  };
  const ch* fmt_width[] = {
    ".5", "5.3", "4.3", "3.4", "3.5", ".1", "."
  };
  const ch* fmt_specifier[] = {
    "f",
  };

  const float samp_float[] = {
    0.0f, -0.0f, 1.0f, -1.0f, 12324.83747f, -12324.83747f, 35.9474945068f, -35.9474945068f
  }; 

  //gcvt(-0.0f, 10, buffer);
  //gcvt(-35.9474945068, 10, buffer);
  //int a, bSign;
  //char* test = NULL;
  //test = ecvt(0.0, 10, &a, &bSign);
  //test = ecvt(-0.0, 10, &a, &bSign);
  //
  //test = fcvt(35.9474945068, 8, &a, &bSign);
  //test = ecvt(35.9474945068, 8, &a, &bSign);
  //
  //test = fcvt(-35.9474945068, 10, &a, &bSign);
  //test = ecvt(-35.9474945068, 10, &a, &bSign);

  //test = fcvt(9.9999996, 6, &a, &bSign);
  //test = ecvt(9.9999996, 6, &a, &bSign);

  //clStringA str0(0, 'E');
  //clStringA str1(-0, 'E');
  //clStringA str2(35.9474945068, 'E');
  //clStringA str3(-35.9474945068, 'E');

  TestFormatStrFunc(fmt_flags, countof(fmt_flags), fmt_width, countof(fmt_width), fmt_specifier, countof(fmt_specifier), samp_float, countof(samp_float));
}

void TestFormatString1()
{
  CLOG(__FUNCTION__);
  //char buffer[1024];
  //sprintf(buffer, "%06.3x", 12);
  const ch* fmt_flags[] = {
    "", "-", "+", "#", " ", "0", "-0", "+0", "#0", "0#", "-#", "+#", "-+", "+-", "- ", " -", "+ ", " +"
  };
  const ch* fmt_width[] = {
    "5", "8", ".5", "5.3", "4.3", "3.4", "3.5"
  };
  const ch* fmt_specifier[] = {
    "d", "i", "u", "o", "x", "X",
  };
  const ch* fmt_specifier_float[] = {
    "f",
  };

  const int samp_int[] = {
    0, 3, 12, 125, 1234, 13456, 237643, -3, -12, -125, -1234, -13456, -237643
  };

  const float samp_float[] = {
    0.0f, -0.0f, 1.0f, -1.0f, 12324.83747f, -12324.83747f
  };

  //TestFormatStrFunc(fmt_flags, countof(fmt_flags), fmt_width, countof(fmt_width), fmt_specifier_float, countof(fmt_specifier_float), samp_float, countof(samp_float));
  TestFormatStrFunc(fmt_flags, countof(fmt_flags), fmt_width, countof(fmt_width), fmt_specifier, countof(fmt_specifier), samp_int, countof(samp_int));
}

int main(int argc, char* argv[])
{
  setlocale(LC_ALL, "");

  //cprintf();
  TestLog();
  TestBasicStringOp();
  TestCodec();
  TestCombinePath();
  TestMatchSpec();
  TestString();
  TestPathFile();
  TestStringResolve();
  TestCodec();
  TestStringToFloat();
  TestFormatString0();
  TestFormatString1();

	return 0;
}

