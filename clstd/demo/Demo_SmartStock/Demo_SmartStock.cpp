// Demo_SmartStock.cpp : Defines the entry point for the console application.
//
// Test_SmartStock.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <clstd.h>
#include <conio.h>
#include <clString.H>
#include <clUtility.H>
#include <Smart\smartstream.h>
#include <Smart\Stock.h>

#pragma warning(disable : 4996)
using namespace clstd;
int GeneralRead()
{
    //SmartStockA sp;
    //SmartStockA::HANDLE hFind;
    //sp.Load(_T("Window.GSprite"));
    //hFind = sp.FindFirstSection(NULL, FALSE, ("Image\\Module"), ("rect"));
    ////SmartStockA::HANDLE hKey;
    //do{
    //	SmartStockA::VALUE value;
    //	if(sp.FindKey(hFind, "left", value))
    //	{
    //		printf("left=%d ", value.ToInt());
    //	}
    //	if(sp.FindKey(hFind, "name", value))
    //	{
    //		printf("%s\n", value.ToString());
    //	}
    //}while(sp.FindNextSection(hFind));

    //sp.FindClose(hFind);
    //getch();
    return 0;
}

int EnumAllSectKey()
{
    //SmartStockA sp;
    //SmartStockA::HANDLE hFind;
    //sp.Load(_T("Window.GSprite"));
    //hFind = sp.FindFirstSection(NULL, FALSE, ("Image\\Module"), NULL);
    ////SmartStockA::HANDLE hKey;
    //do{
    //	printf("%s\n", sp.GetSectionName(hFind));
    //	SmartStockA::VALUE value;
    //	SmartStockA::HANDLE hEnumKey;
    //	hEnumKey = sp.FindFirstKey(hFind, value);
    //	do{
    //		printf("  %s::%s\n", value.KeyName(), value.ToString());
    //	} while (sp.FindNextKey(hEnumKey, value));
    //	sp.FindClose(hEnumKey);

    //}while(sp.FindNextSection(hFind));

    //sp.FindClose(hFind);
    //getch();
    return 0;
}

int CreateSectAndSave()
{
    StockA s;

    //SmartStockA::HANDLE hNew = s.CreateSection("123", "456");
    //s.Load(_T("Window.GSprite"));

    //SmartStockA::HANDLE hFind;
    //hFind = s.FindFirstSection(NULL, FALSE, ("Image\\Module"), ("rect"));
    //SmartStockA::HANDLE hNewSect1 = s.CreateSection("smart", "newsection");
    //SmartStockA::HANDLE hNewSect2 = s.CreateSection("smart\\newsection", "newsection2");
    //s.SetKey(hNewSect2, "identify", "123456");
    //s.SetKey(hNewSect2, "whatkey", "abcdef");
    //s.SetKey(hNewSect2, "identify", "daskjl");


    //s.FindClose(hFind);
    //s.FindClose(hNewSect1);
    //s.FindClose(hNewSect2);

    //s.CloseHandle(hNew);

    // 创建Section
    StockA::Section hRoot = s.Create("root");
    //s.CloseSection(hRoot);

    auto hWillBeDelete = s.Create("WillBeDelete");

    hRoot = s.Create("AfterWillBeDelete");
    //s.CloseSection(hRoot);

    // 创建重名Section
    hRoot = s.Create("root");
    //s.CloseSection(hRoot);

    // 创建子Section
    auto hSub = s.Create("root/sub2/sub1/sub0");
    //s.CloseSection(hSub);

    // 设置键值测试
    hSub = s.Open("root/sub2/sub1/sub0");
    hSub.SetKey("Key1", "123");
    hSub.SetKey("Key2", "4567");
    hSub.SetKey("Key1", "3498");

    printf("Key1=%d\nKey2=%d\n", hSub.GetKeyAsInteger("Key1", 0), hSub.GetKeyAsInteger("Key2", 0));

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
    hRoot = s.Open("root");
    if(hRoot.IsValid())
    {
        hRoot.SetKey("TestKey1", "abc");
        hRoot.NextSection("root");
        hRoot.SetKey("TestKey2", "abcd");
        //s.CloseSection(hRoot);
    }
    else {
        CLBREAK;
    }

    // 在根写入Key
    auto pSect = s.Open(NULL);
    //ASSERT(pSect);
    pSect.SetKey("testRootKey", "Hello world");
    pSect.SetKey("testRootKey2", "Hello world, again");
    //s.CloseSection(pSect);


    //s.CloseSection(hWillBeDelete);

    s.SaveW(L"TestSave.txt");

    //getch();
    return 0;
}

void TestRoot()
{
  StockA s;
  s.LoadW(L"TestSave.txt");
  auto pSect = s.Open(NULL);
  ASSERT(pSect.IsValid());
  StockA::ATTRIBUTE p;
  if(pSect.FirstKey(p))
  {
    do {
      printf("key:%s=%s\n", p.KeyName(), p.ToString());
    } while (p.NextKey());
  }

  printf("\n");

  auto pSectChild = s.Open(&pSect, NULL);
  ASSERT(pSectChild.IsValid());
  do {
    printf("pSectChild:%s\n", pSectChild.SectionName());
  }while(pSectChild.NextSection(NULL));

  //s.CloseSection(pSectChild);
  //s.CloseSection(pSect);
}

void test2_write()
{
  StockA ss;
  StockA::Section frames_sect = ss.Create("frames");
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

  StockA::Section anim_sect = ss.Create("anim");
  ASSERT( ! rect_sect.IsValid());
  for(int i = 0; i < 10; i++)
  {
    StockA::Section point_sect = anim_sect.Create("point");
    point_sect.SetKey("x", clStringA(n % 512));
    point_sect.SetKey("y", clStringA(n % 511));

    n += 500;
  }

  ss.SaveA("framelist.txt");
}

void test2_read()
{
  StockA ss;
  if( ! ss.LoadA("framelist.txt")) {
    return;
  }

  StockA::Section root_sect = ss.Open(NULL);
  if( ! root_sect.IsValid()) {
    return;
  }
  //////////////////////////////////////////////////////////////////////////
  // 值读取检查，这个应该与test2_write()中的算法保持一致
  int n = 1000;

  StockA::Section rect_sect = ss.Open("frames/rect");
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
  ASSERT( ! rect_sect.IsValid());

  StockA::Section point_sect = ss.Open("anim/point");
  for(int i = 0; i < 10; i++)
  {
    int x = point_sect.GetKeyAsInteger("x", 0);
    int y = point_sect.GetKeyAsInteger("y", 0);
    ASSERT(x == (n % 512));
    ASSERT(y == (n % 511));

    n += 500;
    ++point_sect;
  }
  ASSERT( ! point_sect.IsValid());

  //////////////////////////////////////////////////////////////////////////

  root_sect = root_sect.Open(NULL);
  while(root_sect.IsValid()) {
    StockA::Section child_sect = root_sect.Open(NULL);
    while(child_sect.IsValid())
    {
      StockA::ATTRIBUTE attr     = child_sect.begin();
      StockA::ATTRIBUTE end_attr = child_sect.end();

      printf("%s\n", child_sect.SectionName());
      for(; attr != end_attr; ++attr)
      {
        printf(" %s=%s\n", attr.KeyName(), attr.ToString());
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
  StockA ss;

  StockA::Section root = ss.Create(NULL);
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

  ASSERT( ! child_a.IsValid()); // 失效
  ASSERT( ! child_b.IsValid()); // 失效
  ASSERT(child_c.IsValid());    // 有效
  ASSERT(root.IsValid());       // 有效

  ss.SaveA("test_04.txt");
}

int _tmain(int argc, _TCHAR* argv[])
{
    // 测试SetKey("key", NULL);
    // 测试SetKey("key", "");
    // 这两种忽略写入，但是Stock文件中如果有“key=;”这种模式还应该能正常读取

    CreateSectAndSave();
    test_outofdate();

    //TestRoot();
    test2_write();
    test2_read();
    printf("Press any key to continue...\n");
    getch();
    //return GeneralRead();
    //return EnumAllSectKey();
}

