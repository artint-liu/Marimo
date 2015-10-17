// Sample_UVShader.cpp : 定义控制台应用程序的入口点。
//
#define _CRT_SECURE_NO_WARNINGS

#include <tchar.h>
#include <conio.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "ExpressionSample.h"
#include "gdiplus.h"

#pragma comment(lib, "gdiplus.lib")
void TestExpressionParser();
void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput);
void TestExpressionParser(const SAMPLE_EXPRESSION* pSamples);
//#define ENABLE_GRAPH // 毫无意义的开始了语法树转图形化的工作，有舍不得删代码，先注释掉

//////////////////////////////////////////////////////////////////////////

extern SAMPLE_EXPRESSION samplesNumeric[];
extern SAMPLE_EXPRESSION samplesOpercode[];
extern SAMPLE_EXPRESSION samplesExpression[];
extern SAMPLE_EXPRESSION samplesIfExpression[];
extern SAMPLE_EXPRESSION samplesForExpression[];
extern SAMPLE_EXPRESSION samplesSimpleExpression[];

// 使用继承类为了暴露ParseStatementAs_Expression接口进行测试


//////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_GRAPH
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  using namespace Gdiplus;

  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize(&num, &size);
  if(size == 0)
    return -1;  // Failure

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if(pImageCodecInfo == NULL)
    return -1;  // Failure

  GetImageEncoders(num, size, pImageCodecInfo);

  for(UINT j = 0; j < num; ++j)
  {
    if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }    
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

struct SYNTAXBOX
{
  clStringA op;
  clStringA token[2];
  SYNTAXBOX* pNode[2];
  GXRECT rect;
};

void MesureString(GXRECT& rect, const clStringA& str)
{
  const int s = 8;
  gxSetRect(&rect, 0, 0, str.GetLength() * s, s);
};

void MakeGraphBox(TestExpression& expp, SYNTAXBOX* pParent, UVShader::CodeParser::SYNTAXNODE* pNode)
{
  GXRECT rcop = {0};
  GXRECT rect[2]; 
  if(pNode->pOpcode) {
    pParent->op = pNode->pOpcode->ToString();
    MesureString(rcop, pParent->op);
  }
  else {
    pParent->op = NULL;
  }

  for(int i = 0; i < 2; i++)
  {
    if(pNode->GetOperandType(i) == UVShader::CodeParser::SYNTAXNODE::FLAG_OPERAND_IS_TOKEN)
    {
      pParent->token[i] = pNode->Operand[i].pSym->ToString();
      pParent->pNode[i] = NULL;
      MesureString(rect[i], pParent->token[i]);
      if(i == 0) {
        gxOffsetRect(&rect[i], (rect[i].left - rect[i].right) + (rcop.left - rcop.right), 0);
      }
      else if(i == 1) {
        gxOffsetRect(&rect[i], (rcop.right - rcop.left), 0);
      }
    }
    else {
      pParent->token[i].Clear();
      pParent->pNode[i] = new SYNTAXBOX();
      MakeGraphBox(expp, pParent->pNode[i], pNode->Operand[i].pNode);

      if(i == 0) {
        gxOffsetRect(&rect[i], (rect[i].left - rect[i].right) + (rcop.left - rcop.right), 0);
      }
      else if(i == 1) {
        gxOffsetRect(&rect[i], (rcop.right - rcop.left), 0);
      }
    }
  }
  //if(pUnion->pSym
}

void DrawGraphBox(SYNTAXBOX* pParent)
{

}

void CleanGraphBox(SYNTAXBOX* pParent)
{
  for(int i = 0; i < 2; i++)
  {
    if(pParent->pNode[i]) {
      CleanGraphBox(pParent->pNode[i]);
      delete pParent->pNode[i];
      pParent->pNode[i] = NULL;
    }
  }
}

void MakeGraphicalExpression(GXLPCSTR szExpression, GXLPCSTR szOutputFile)
{
  TestExpression expp;
  UVShader::CodeParser::SYNTAXNODE::UN Union;
  const auto nSize = strlen(szExpression);
  expp.Attach(szExpression, nSize);
  expp.GenerateTokens();
  expp.TestGraph(&Union);

  ASSERT(expp.TryGetNodeType(&Union) == UVShader::CodeParser::SYNTAXNODE::FLAG_OPERAND_IS_NODE); // 没处理其他情况
  if( ! expp.TestGraph(&Union)) {
    CLBREAK;
    return;
  }

  SYNTAXBOX box;
  GXPOINT pt = {0, 0};
  MakeGraphBox(expp, &box, Union.pNode);
  DrawGraphBox(&box);
  CleanGraphBox(&box);


  Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(1024, 1024, PixelFormat32bppARGB);
  CLSID  encoderClsid;
  INT    result;
  result = GetEncoderClsid(L"image/png", &encoderClsid);
  pBitmap->Save(clStringW(szOutputFile), &encoderClsid);
  SAFE_DELETE(pBitmap);
}
#endif // #ifdef ENABLE_GRAPH
//////////////////////////////////////////////////////////////////////////

void TestFlowIf()
{
  int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5;

  if(a < b)
    if(b < c)
      if(c > d)
        a = a;
      else if(d < e)
        b = b;
      else
        c = c;


  switch(a)
  {
  case 0:
  case 1:
    if(b>c)
    {
  case 2:
    break;
    }
  default:
    break;
  };

  do 
  {
    a++;
  } while (a < b);
}

void TestShaderToys(GXBOOL bShowList)
{
  clstd::FindFile find("Test\\shaders\\ShaderToy\\*.txt");
  clstd::FINDFILEDATAA find_data;
  struct ITEM
  {
    clStringA strName;
    clStringA strInput;
    clStringA strOutput;
  };

  typedef clvector<ITEM> ItemList;

  ItemList toy_list;

  while(find.GetFileA(&find_data))
  {
    ITEM item;
    item.strName = find_data.Filename;
    item.strOutput = find_data.Filename;

    clpathfile::RenameExtensionA(item.strName, "");

    if(item.strOutput.EndsWith("[output].txt")) {
      continue;
    }

    clsize pos = clpathfile::FindExtensionA(item.strOutput);
    if(pos != clStringA::npos)
    {
      item.strOutput.Insert(pos, "[output]");
      clpathfile::CombinePathA(item.strInput, "Test\\shaders\\ShaderToy", find_data.Filename);
      clpathfile::CombinePathA(item.strOutput, "Test\\shaders\\ShaderToy", item.strOutput);
      if(bShowList)
      {
        toy_list.push_back(item);
      }
      else
      {
        TestFromFile(item.strInput, item.strOutput);
      }
    }
  }

  if(bShowList)
  {
    for(auto it = toy_list.begin(); it != toy_list.end(); ++it)
    {
      auto n = it - toy_list.begin();
      printf("%3d.%*s", n, -35, it->strName);
      if(n % 2 == 1) {
        printf("\n");
      }
    }
    printf("type \"all\" for all.\n");

    char szBuffer[128];
    gets(szBuffer);
    if(GXSTRCMPI(szBuffer, "all") == 0) {
      for(auto it = toy_list.begin(); it != toy_list.end(); ++it) {
        TestFromFile(it->strInput, it->strOutput);
      }
    }
    else
    {
      int nSelect = atoi(szBuffer);
      if(nSelect >= 0 && nSelect < (int)toy_list.size()) {
        auto& item = toy_list[nSelect];
        TestFromFile(item.strInput, item.strOutput);
      }
    }
  }
}

// 测试开关
//#define TEST_ARITHMETIC_PARSING
#define TEST_ALL_SHADERTOYS

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef ENABLE_GRAPH
  using namespace Gdiplus;
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif // ENABLE_GRAPH

  int a = 1, b = 2, c = 3, d = 4;

  TestFlowIf();
  clpathfile::LocalWorkingDirA("..");


  //
  // 数学表达式解析
  //
#ifdef TEST_ARITHMETIC_PARSING
  // 2015/10/06 改为只测试数学表达式
  TestExpressionParser(samplesOpercode);
  TestExpressionParser(samplesNumeric);
  TestExpressionParser(samplesSimpleExpression);
  TestExpressionParser(samplesExpression);

  // if/for 表达式不在这里测试了！
  //TestExpressionParser(samplesIfExpression);
  //TestExpressionParser(samplesForExpression);
#endif // TEST_ARITHMETIC_PARSING



#ifdef ENABLE_GRAPH
  MakeGraphicalExpression("Output.I.rgb = (1.0f - Output.E.rgb) * I( Theta ) * g_vLightDiffuse.xyz * g_fSunIntensity", "Test\\shaders\\output.png");
#endif // #ifdef ENABLE_GRAPH

  //TestFromFile("Test\\shaders\\std_samples.uvs", "Test\\shaders\\std_samples[output].txt");

#ifdef TEST_ALL_SHADERTOYS
  TestShaderToys(TRUE);
#endif // TEST_ALL_SHADERTOYS

  //TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt", "Test\\shaders\\Flame[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\Anatomy of an explosion.txt", "Test\\shaders\\Anatomy of an explosion[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\Warp speed.txt", "Test\\shaders\\Warp speed[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt", "Test\\shaders\\TrivialRaytracer3[output].txt");
  
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt");
#ifdef ENABLE_GRAPH
  GdiplusShutdown(gdiplusToken);
#endif // ENABLE_GRAPH
	return 0;
}

