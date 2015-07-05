// Sample_UVShader.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
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
      ExpressionParser expp;
      expp.Attach((const char*)pBuffer->GetPtr(), pBuffer->GetSize());
      int nCount = 0;
      for(SmartStreamA::iterator it = expp.begin();
        it != expp.end(); ++it)
      {
        TRACE("|%s|  ", it.ToString());
        nCount++;
      }

      TRACE("\ncount:%d\n", nCount);
    }
    SAFE_DELETE(pBuffer);
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
  clpathfile::LocalWorkingDirA("..");

  TestExpressionParser();
  TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt");
	return 0;
}

