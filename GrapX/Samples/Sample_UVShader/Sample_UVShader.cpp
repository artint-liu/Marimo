// Sample_UVShader.cpp : 定义控制台应用程序的入口点。
//

#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"

void TestExpressionParser();

void TestFromFile(GXLPCSTR szFilename)
{
  clFile file;
  if(file.OpenExistingA(szFilename))
  {
    clBuffer* pBuffer = NULL;
    if(file.MapToBuffer(&pBuffer))
    {
      UVShader::ExpressionParser expp;
      const UVShader::ExpressionParser::SymbolArray* pSymbols;
      expp.Attach((const char*)pBuffer->GetPtr(), pBuffer->GetSize());

      expp.GenerateSymbols();
      pSymbols = expp.GetSymbolsArray();
      int nCount = 0;
      for(auto it = pSymbols->begin(); it != pSymbols->end(); ++it, ++nCount)
      {
        if(it->pair >= 0) {
          TRACE("<#%d:\"%s\"(%d)> ", nCount, it->sym.ToString(), it->pair);
        }
        else {
          TRACE("<#%d:\"%s\"> ", nCount, it->sym.ToString());
        }
      }

      TRACE("\ncount:%d(%f)\n", pSymbols->size(), (float)pBuffer->GetSize() / pSymbols->size());

      expp.Parse();
    }
    SAFE_DELETE(pBuffer);
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
  clpathfile::LocalWorkingDirA("..");

  TestExpressionParser();
  //TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt");
	return 0;
}

