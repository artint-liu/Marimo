﻿// Demo_SmartStock.cpp : Defines the entry point for the console application.
//
// Test_SmartStock.cpp : Defines the entry point for the console application.
//

//#include <tchar.h>
#include <clstd.h>
//#include <conio.h>
#include <clString.h>
#include <clUtility.h>
//#include <smart\smartstream.h>
#include <clTokens.h>
#include <clStock.h>

#pragma warning(disable : 4996)
using namespace clstd;

void Test_UnitA()
{
  {
    // 测试打开空文件的情况
    {
      File file;
      file.CreateAlways("empty.txt");
      file.Close();
    }
    StockA stock;
    b32 result = stock.LoadFromFile("empty.txt");
    ASSERT(result);
    StockA::Section sect = stock.OpenSection(NULL);
    CLOG("sect is %s", sect.empty() ? "empty(异常)" : "not empty");
    ASSERT(_CL_NOT_(sect.empty()));

    sect = stock.OpenSection("");
    CLOG("sect is %s", sect.empty() ? "empty" : "not empty(异常)");
    ASSERT(sect.empty());

    sect = stock.OpenSection("default");
    CLOG("sect is %s", sect.empty() ? "empty" : "not empty(异常)");
    ASSERT(sect.empty());
  }

  {
    // 测试构造后直接打开的情况
    StockA stock;
    StockA::Section sect = stock.OpenSection("default");
    CLOG("sect is %s", sect.empty() ? "empty" : "not empty(异常)");
    ASSERT(sect.empty());
  }

  {
    // 测试StockA::Section自我赋值
    StockA stock;
    StockA::Section sect = stock.CreateSection("default");
    sect = sect.Create("win32"); // sect子段赋值给自己
    sect.SetKey("file", "abc.text"); // 不应该崩溃
  }

  {
    // 测试遍历某一段下的一段的情况
    StockA stock;
    StockA::Section sect_default = stock.CreateSection("default");
    StockA::Section sect = sect_default.Create("win32");
    sect.SetKey("file", "abc.text");

    sect_default = stock.OpenSection("default");
    sect = sect_default.Open(NULL);
    if(sect)
    {
      CLOG("sect name:%s", sect.SectionName());
      ASSERT(sect.SectionName() == "win32");
    }
  }
}

void StockToText(clStringA& strStockText, const StockA& stock)
{
  strStockText.Clear();
  strStockText.Append((clStringA::LPCSTR)stock.GetBuffer().GetPtr(), stock.GetBuffer().GetSize() / sizeof(clStringA::TChar));

  strStockText.Remove('\r');
  strStockText.Remove('\n');
  strStockText.Remove('\t');
  strStockText.Remove(0x20);
}

int CreateSectAndSave()
{
  CLOG(__FUNCTION__);
  StockA s;
  clStringA strStock;
    
  // 创建Section
  StockA::Section hRoot = s.CreateSection("root");
  StockToText(strStock, s);
  ASSERT(strStock == "root{}");
  //s.CloseSection(hRoot);

  

  auto hWillBeDelete = s.CreateSection("WillBeDelete");

  hRoot = s.CreateSection("AfterWillBeDelete");
  //s.CloseSection(hRoot);

  // 创建重名Section
  hRoot = s.CreateSection("root");
  
  //s.CloseSection(hRoot);

  // 创建子Section
  auto hSub = s.CreateSection("root/sub2/sub1/sub0");
  StockToText(strStock, s);
  ASSERT(strStock == "root{sub2{sub1{sub0{}}}}WillBeDelete{}AfterWillBeDelete{}root{}");

  // 设置键值测试
  hSub = s.OpenSection("root/sub2/sub1/sub0");
  ASSERT(hSub);
  
  hSub.SetKey("Key1", "123");  
  hSub.SetKey("Key2", "4567");  
  hSub.SetKey("Key1", "3498");  
  hSub.SetKey("Visible", "true");
  
  b32 bResult = hSub.GetKeyAsBoolean("Visible", FALSE);
  ASSERT(bResult);
  hSub.SetKey("Visible", "false");
  bResult = hSub.GetKeyAsBoolean("Visible", TRUE);
  ASSERT(!bResult);


  CLOG("Key1=%d\nKey2=%d\n", hSub.GetKeyAsInteger("Key1", 0), hSub.GetKeyAsInteger("Key2", 0));

  hSub.DeleteKey("Key1");
  hSub.DeleteKey("Key2");
  hSub.DeleteKey("Key3"); // 这个应该失败

  hSub.SetKey("NewKey2", "4567");
  hSub.SetKey("NewKey1", "3498");
  hSub.SetKey("NewKey2", "abc");
  hSub.SetKey("NewKey1", "bcd");

  hSub.Rename("sub0renamed");
  //s.CloseSection(hSub);

  s.DeleteSection("WillBeDelete");

  // 测试遍历Section
  hRoot = s.OpenSection("root");
  if(hRoot)
  {
    hRoot.SetKey("TestKey1", "abc");
    hRoot.NextSection("root");
    hRoot.SetKey("TestKey2", "abcd");
    //s.CloseSection(hRoot);
  }
  else {
    CLOG("%s(%d) : hRoot == NULL", __FILE__, __LINE__);
    CLBREAK;
  }

  // 在根写入Key
  auto pSect = s.OpenSection(NULL);
  //ASSERT(pSect);
  pSect.SetKey("testRootKey", "Hello world");
  pSect.SetKey("testRootKey2", "Hello world, again");
  //s.CloseSection(pSect);


    s.SaveToFile(_CLTEXT("TestSave.txt"));

  s.SaveToFile(_CLTEXT("TestSave.txt"));

  //getch();
  return 0;
}

void TestRoot()
{
  CLOG(__FUNCTION__);

  StockA s;
  s.LoadFromFile(_CLTEXT("TestSave.txt"));
  auto pSect = s.OpenSection(NULL);
  ASSERT(pSect);
  StockA::ATTRIBUTE p;
  if(pSect.FirstKey(p))
  {
    do {
      CLOG("key:%s=%s", p.KeyName().CStr(), p.ToString().CStr());
    } while (p.NextKey());
  }

  CLOG("");

  auto pSectChild = s.OpenSection(&pSect, NULL);
  ASSERT(pSectChild);
  do {
    CLOG("pSectChild:%s", pSectChild.SectionName().CStr());
  }while(pSectChild.NextSection(NULL));

  //s.CloseSection(pSectChild);
  //s.CloseSection(pSect);
}

void test2_write()
{
  CLOG(__FUNCTION__);

  StockA ss;
  StockA::Section frames_sect = ss.CreateSection("frames");
  //clstd::Rand r;
  int n = 1000;

  StockA::Section rect_sect;
  for(int i = 0; i < 10; i++)
  {
    rect_sect = frames_sect.Create("rect");
    rect_sect.SetKey("a", clStringA(n % 640));
    rect_sect.SetKey("b", clStringA(n % 481));
    rect_sect.SetKey("c", clStringA(n % 639));
    rect_sect.SetKey("d", clStringA(n % 479));
    n += 1000;
  }

  StockA::Section anim_sect = ss.CreateSection("anim");
  ASSERT( ! rect_sect);
  for(int i = 0; i < 10; i++)
  {
    StockA::Section point_sect = anim_sect.Create("point");
    point_sect.SetKey("x", clStringA(n % 512));
    point_sect.SetKey("y", clStringA(n % 511));

    n += 500;
  }

  ss.SaveToFile("framelist.txt");
}

void test2_read()
{
  CLOG(__FUNCTION__);
  StockA ss;
  if( ! ss.LoadFromFile("framelist.txt")) {
    return;
  }

  StockA::Section root_sect = ss.OpenSection(NULL);
  if( ! root_sect) {
    return;
  }

  StockA::ATTRIBUTE attr = root_sect["not_exist"];
  ASSERT( ! attr);
  auto strMustBeEmpty = attr.ToString();
  ASSERT(strMustBeEmpty.IsEmpty());

  //////////////////////////////////////////////////////////////////////////
  // 值读取检查，这个应该与test2_write()中的算法保持一致
  int n = 1000;

  StockA::Section rect_sect = ss.OpenSection("frames/rect");
  for(int i = 0; i < 10; i++)
  {
    int a = rect_sect["a"].ToInt();
    int b = rect_sect["b"].ToInt();
    int c = rect_sect["c"].ToInt();
    int d = rect_sect["d"].ToInt();

    ASSERT(a == (n % 640));
    ASSERT(b == (n % 481));
    ASSERT(c == (n % 639));
    ASSERT(d == (n % 479));
    n += 1000;
    ++rect_sect;
  }
  ASSERT( ! rect_sect);

  StockA::Section point_sect = ss.OpenSection("anim/point");
  for(int i = 0; i < 10; i++)
  {
    int x = point_sect.GetKeyAsInteger("x", 0);
    int y = point_sect.GetKeyAsInteger("y", 0);
    ASSERT(x == (n % 512));
    ASSERT(y == (n % 511));

    n += 500;
    ++point_sect;
  }
  ASSERT( ! point_sect);

  //////////////////////////////////////////////////////////////////////////

  root_sect = root_sect.Open(NULL);
  while(root_sect) {
    StockA::Section child_sect = root_sect.Open(NULL);
    while(child_sect)
    {
      CLOG("%s", child_sect.SectionName().CStr());
      for(StockA::ATTRIBUTE attr = child_sect.FirstKey(); ! attr.IsEmpty(); ++attr)
      {
        CLOG(" %s=%s", attr.KeyName().CStr(), attr.ToString().CStr());
      }

      if( ! child_sect.NextSection(NULL)) {
        break;
      }
    }

    ++root_sect;
  }
}

void test_outofdate()
{
  CLOG(__FUNCTION__);
  StockA ss;

  // 测试失效键值
  // 按照一定顺序插入key/section时，由于数据连续性关系，section记录的
  // 定位信息可能会发生改变，section中会通知所有的父section做出相应的
  // 调整。但是无法调整兄弟section或者父section的兄弟section，这时
  // 这些无法调整的section会被标记为失效，表示不再可用。

  StockA::Section root = ss.CreateSection(NULL);
  root.SetKey("AAA", "aaa");
  root.SetKey("BBB", "bbb");
  root.SetKey("CCC", "ccc");
  root.SetKey("DDD", "ddd");

  StockA::Section child_a = root.Create("Child_A");
  child_a.SetKey("AA", "aa");
  child_a.SetKey("BB", "bb");
  child_a.SetKey("CC", "cc");

  StockA::Section child_b = root.Create("Child_B");
  child_b.SetKey("AA", "aa");
  child_b.SetKey("BB", "bb");
  child_b.SetKey("CC", "cc");

  StockA::Section child_c = root.Create("Child_C");
  child_c.SetKey("AA", "aa");
  child_c.SetKey("BB", "bb");
  child_c.SetKey("CC", "cc");

  ASSERT( ! child_a); // 失效
  ASSERT( ! child_b); // 失效
  ASSERT(child_c);    // 有效
  ASSERT(root);       // 有效

  ss.SaveToFile("test_04.txt");
}


void RecursiveTraceSectionName(clstd::StockA& ss, clstd::StockA::Section& sect)
{
  auto sub_sect = ss.OpenSection(&sect, NULL);
  if(sub_sect)
  {
    do {
      TRACE("sect name: %s\n", sub_sect.SectionName());
      RecursiveTraceSectionName(ss, sub_sect);
    } while(sub_sect.NextSection());
  }
}

void RecursiveTraceSectionNameV2(clStringListA& str_list, clstd::StockA& ss, clstd::StockA::Section& sect)
{
  auto sub_sect = ss.OpenSection(&sect, NULL);
  clStringA str;
  if(sub_sect)
  {
    do {
      if(sub_sect.name + 1 != sub_sect.iter_begin)
      {
        auto iter_name_next = sub_sect.name + 1;
        clStringA strParam(iter_name_next.marker, sub_sect.iter_begin.marker - iter_name_next.marker);
        str.Format("<sect>%s:%s", sub_sect.SectionName(), strParam.CStr());
      }
      else
      {
        str.Format("<sect>%s", sub_sect.SectionName());
      }
      TRACE("%s\n", str);
      str_list.push_back(str);

      auto attr = sub_sect.FirstKey();
      for(; _CL_NOT_(attr.IsEmpty()); ++attr)
      {
        str.Format("< key>%s=%s", attr.KeyName(), attr.value.ToString());
        TRACE("%s\n", str);
        str_list.push_back(str);
      }

      RecursiveTraceSectionNameV2(str_list, ss, sub_sect);
    } while(sub_sect.NextSection());
  }
}

void Test_Version2()
{
  const char* szSampleText = "sect(param){ name = green; age(12); sub_sect(k){ empty = yes;} space(20) = middle; } sect(1,2){ name(text)=1342; }"
    "property[12]{} shader[]{lalala=ok;}";
  const char* aExpect[] = {
    "<sect>sect:(param)",
    "< key>name=green",
    "< key>age=;",
    "< key>space=middle",
    "<sect>sub_sect:(k)",
    "< key>empty=yes",
    "<sect>sect:(1,2)",
    "< key>name=1342",
    "<sect>property:[12]",
    "<sect>shader:[]",
    "< key>lalala=ok",
    NULL};
  StockA ss(StockA::StockFlag_Version2);

  ss.Set(szSampleText, clstd::strlenT(szSampleText));

  auto sect = ss.OpenSection(NULL);
  if(sect)
  {
    clStringListA str_list;
    RecursiveTraceSectionNameV2(str_list, ss, sect);
    const char** pszExpect = aExpect;
    for(auto it = str_list.begin(); it != str_list.end(); ++it)
    {
      ASSERT(*it == *pszExpect);
      pszExpect++;
    }
    ASSERT(*pszExpect == NULL);
  }
}

void Test_UnusalFile() // 测试一些不太规整的文件格式
{
  char szFilename[MAX_PATH];
  printf("输入文件名：");
  scanf("%s", &szFilename);

  if(clstd::strlenT(szFilename) == 0) {
    return;
  }

  StockA ss;
  if(_CL_NOT_(ss.LoadFromFile(szFilename)))
  {
    printf("无法打开文件(%s)\n", szFilename);
    clstd_cli::getch();
    return;
  }

  auto sect = ss.OpenSection(NULL);
  if(sect)
  {
    RecursiveTraceSectionName(ss, sect);
  }

  ss.SaveToFile("test.shader");
}

int main(int argc, char* argv[])
{
  // 测试SetKey("key", NULL);
  // 测试SetKey("key", "");
  // 这两种忽略写入，但是Stock文件中如果有“key=;”这种模式还应该能正常读取
  CLOG(__FUNCTION__);

  Test_UnitA();

  CreateSectAndSave();
  test_outofdate();

  TestRoot();
  test2_write();
  test2_read();
  Test_Version2();
  //Test_UnusalFile();
  printf("Press any key to continue...\n");
  clstd_cli::getch();
  //return GeneralRead();
  //return EnumAllSectKey();
}

