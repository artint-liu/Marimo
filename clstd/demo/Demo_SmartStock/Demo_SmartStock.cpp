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
#include <clTokens.h>
#include <clStock.h>

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
    StockA::Section hRoot = s.CreateSection("root");
    //s.CloseSection(hRoot);

    auto hWillBeDelete = s.CreateSection("WillBeDelete");

    hRoot = s.CreateSection("AfterWillBeDelete");
    //s.CloseSection(hRoot);

    // 创建重名Section
    hRoot = s.CreateSection("root");
    //s.CloseSection(hRoot);

    // 创建子Section
    auto hSub = s.CreateSection("root/sub2/sub1/sub0");
    //s.CloseSection(hSub);

    // 设置键值测试
    hSub = s.OpenSection("root/sub2/sub1/sub0");
    hSub.SetKey("Key1", "123");
    hSub.SetKey("Key2", "4567");
    hSub.SetKey("Key1", "3498");
    hSub.SetKey("Visible", "true");
    b32 bResult = hSub.GetKeyAsBoolean("Visible", FALSE);
    ASSERT(bResult);
    hSub.SetKey("Visible", "false");
    bResult = hSub.GetKeyAsBoolean("Visible", TRUE);
    ASSERT( ! bResult);


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
    hRoot = s.OpenSection("root");
    if(hRoot)
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
    auto pSect = s.OpenSection(NULL);
    //ASSERT(pSect);
    pSect.SetKey("testRootKey", "Hello world");
    pSect.SetKey("testRootKey2", "Hello world, again");
    //s.CloseSection(pSect);


    //s.CloseSection(hWillBeDelete);

    s.SaveToFile(L"TestSave.txt");

    //getch();
    return 0;
}

void TestRoot()
{
  StockA s;
  s.LoadFromFile(L"TestSave.txt");
  auto pSect = s.OpenSection(NULL);
  ASSERT(pSect);
  StockA::ATTRIBUTE p;
  if(pSect.FirstKey(p))
  {
    do {
      printf("key:%s=%s\n", p.KeyName(), p.ToString());
    } while (p.NextKey());
  }

  printf("\n");

  auto pSectChild = s.OpenSection(&pSect, NULL);
  ASSERT(pSectChild);
  do {
    printf("pSectChild:%s\n", pSectChild.SectionName());
  }while(pSectChild.NextSection(NULL));

  //s.CloseSection(pSectChild);
  //s.CloseSection(pSect);
}

void test2_write()
{
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
      printf("%s\n", child_sect.SectionName());
      for(StockA::ATTRIBUTE attr = child_sect.FirstKey(); ! attr.IsEmpty(); ++attr)
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

int _tmain(int argc, _TCHAR* argv[])
{
    // 测试SetKey("key", NULL);
    // 测试SetKey("key", "");
    // 这两种忽略写入，但是Stock文件中如果有“key=;”这种模式还应该能正常读取

    CreateSectAndSave();
    test_outofdate();

    TestRoot();
    test2_write();
    test2_read();
    printf("Press any key to continue...\n");
    getch();
    //return GeneralRead();
    //return EnumAllSectKey();
}

