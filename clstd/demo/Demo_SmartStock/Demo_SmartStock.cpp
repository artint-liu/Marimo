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
#include <Smart\SmartStock.h>

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
    SmartStockA s;

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

    // ����Section
    SmartStockA::Section hRoot = s.Create("root");
    //s.CloseSection(hRoot);

    auto hWillBeDelete = s.Create("WillBeDelete");

    hRoot = s.Create("AfterWillBeDelete");
    //s.CloseSection(hRoot);

    // ��������Section
    hRoot = s.Create("root");
    //s.CloseSection(hRoot);

    // ������Section
    auto hSub = s.Create("root/sub2/sub1/sub0");
    s.CloseSection(hSub);

    // ���ü�ֵ����
    hSub = s.Open("root/sub2/sub1/sub0");
    hSub->SetKey("Key1", "123");
    hSub->SetKey("Key2", "4567");
    hSub->SetKey("Key1", "3498");

    printf("Key1=%d\nKey2=%d\n", hSub->GetKeyAsInteger("Key1", 0), hSub->GetKeyAsInteger("Key2", 0));

    hSub->DeleteKey("Key1");
    hSub->DeleteKey("Key2");
    hSub->DeleteKey("Key3"); // ���Ӧ��ʧ��

    hSub->SetKey("NewKey2", "4567");
    hSub->SetKey("NewKey1", "3498");
    hSub->SetKey("NewKey2", "abc");
    hSub->SetKey("NewKey1", "bcd");

    hSub->Rename("sub0renamed");
    s.CloseSection(hSub);

    s.DeleteSection("WillBeDelete");

    // ���Ա���Section
    hRoot = s.Open("root");
    if(hRoot.IsValid())
    {
        hRoot->SetKey("TestKey1", "abc");
        hRoot->NextSection("root");
        hRoot->SetKey("TestKey2", "abcd");
        //s.CloseSection(hRoot);
    }
    else {
        CLBREAK;
    }

    // �ڸ�д��Key
    auto pSect = s.Open(NULL);
    ASSERT(pSect);
    pSect->SetKey("testRootKey", "Hello world");
    pSect->SetKey("testRootKey2", "Hello world, again");
    s.CloseSection(pSect);


    s.CloseSection(hWillBeDelete);

    s.SaveW(L"TestSave.txt");

    //getch();
    return 0;
}

void TestRoot()
{
  SmartStockA s;
  s.LoadW(L"TestSave.txt");
  auto pSect = s.Open(NULL);
  ASSERT(pSect);
  SmartStockA::ATTRIBUTE p;
  if(pSect->FirstKey(p))
  {
    do {
      printf("key:%s=%s\n", p.KeyName(), p.ToString());
    } while (p.NextKey());
  }

  printf("\n");

  auto pSectChild = s.Open(pSect, NULL);
  ASSERT(pSectChild);
  do {
    printf("pSectChild:%s\n", pSectChild->SectionName());
  }while(pSectChild->NextSection(NULL));

  s.CloseSection(pSectChild);
  s.CloseSection(pSect);
}

void test2_write()
{
  SmartStockA ss;
  SmartStockA::Section frames_sect = ss.Create("frames");
  clstd::Rand r;
  for(int i = 0; i < 10; i++)
  {
    SmartStockA::Section rect_sect = ss.Create(&frames_sect, "rect");
    rect_sect->SetKey("x", clStringA(r.rand() % 640));
    rect_sect->SetKey("y", clStringA(r.rand() % 480));
    rect_sect->SetKey("w", clStringA(r.rand() % 640));
    rect_sect->SetKey("h", clStringA(r.rand() % 480));
  }
  ss.SaveA("framelist.txt");
}

void test2_read()
{
  SmartStockA ss;
  if( ! ss.LoadA("framelist.txt")) {
    return;
  }

  SmartStockA::Section root_sect = ss.Open(NULL);
  if( ! root_sect->IsValid()) {
    return;
  }

  root_sect = root_sect->Open(NULL);
  if(root_sect->IsValid()) {
    SmartStockA::Section child_sect = root_sect->Open(NULL);
    while(child_sect->IsValid())
    {
      SmartStockA::ATTRIBUTE attr = child_sect->begin();
      SmartStockA::ATTRIBUTE end_attr = child_sect->end();

      printf("%s\n", child_sect->SectionName());
      for(; attr != end_attr; ++attr)
      {
        printf("%s=%s\n", attr.KeyName(), attr.ToString());
      }

      if( ! child_sect->NextSection(NULL)) {
        break;
      }
    }
  }

}

int _tmain(int argc, _TCHAR* argv[])
{
    // ����SetKey("key", NULL);
    // ����SetKey("key", "");
    // �����ֺ���д�룬����Stock�ļ�������С�key=;������ģʽ��Ӧ����������ȡ

    CreateSectAndSave();
    //TestRoot();
    test2_write();
    test2_read();
    printf("Press any key to continue...\n");
    getch();
    //return GeneralRead();
    //return EnumAllSectKey();
}

