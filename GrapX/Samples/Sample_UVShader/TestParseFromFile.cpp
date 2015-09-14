#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

GXLPCSTR Get(UVShader::ExpressionParser::InputModifier e)
{
  switch(e)
  {
  case UVShader::ExpressionParser::InputModifier_in:
    return "in ";
  case UVShader::ExpressionParser::InputModifier_out:
    return "out ";
  case UVShader::ExpressionParser::InputModifier_inout:
    return "intout ";
  case UVShader::ExpressionParser::InputModifier_uniform:
    return "uniform ";
  default:
    CLBREAK;
  }
}

GXLPCSTR Get(UVShader::ExpressionParser::StorageClass e)
{
  switch(e)
  {
  case UVShader::ExpressionParser::StorageClass_empty:
    return "";
  case UVShader::ExpressionParser::StorageClass_inline:
    return "inline ";
  default:
    CLBREAK;
  }
}

//////////////////////////////////////////////////////////////////////////

void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput)
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
        if(it->scope >= 0) {
          TRACE("<#%d:\"%s\"(%d)> ", nCount, it->sym.ToString(), it->scope);
        }
        else {
          TRACE("<#%d:\"%s\"> ", nCount, it->sym.ToString());
        }
      }

      TRACE("\ncount:%d(%f)\n", pSymbols->size(), (float)pBuffer->GetSize() / pSymbols->size());

      expp.Parse();
      if(szOutput != NULL)
      {
        clstd::File file;
        if(file.CreateAlwaysA(szOutput))
        {
          auto stats = expp.GetStatments();
          for(auto it = stats.begin(); it != stats.end(); ++it)
          {
            const auto& s = *it;
            switch(s.type)
            {
            case UVShader::ExpressionParser::StatementType_FunctionDecl:
              break;
            case UVShader::ExpressionParser::StatementType_Function:
              {
                const auto& func = s.func;

                clStringA strArgs;
                for(clsize i = 0; i < func.nNumOfArguments; i++)
                {
                  strArgs.AppendFormat("%s%s %s", Get(func.pArguments[i].eModifier), func.pArguments[i].szType, func.pArguments[i].szName);
                  if(func.pArguments[i].szSemantic) {
                    strArgs.AppendFormat(" : %s", func.pArguments[i].szSemantic);
                  }
                  strArgs.Append(",");
                }

                if(strArgs.IsNotEmpty()) {
                  strArgs.Remove(strArgs.GetLength() - 1, 1);
                }

                file.WritefA("%s%s %s(%s)", Get(func.eStorageClass), func.szReturnType, func.szName, strArgs);

                if(func.szSemantic) {
                  file.WritefA(" : %s\n", func.szSemantic);
                }
                else {
                  file.WritefA("\n");
                }

                file.WritefA("{\n");
                file.WritefA("}\n");
              }
              break;
            case UVShader::ExpressionParser::StatementType_Struct:
              break;
            case UVShader::ExpressionParser::StatementType_Signatures:
              break;
            case UVShader::ExpressionParser::StatementType_Expression:
              break;
            }
          }
        }
      }
    }
    SAFE_DELETE(pBuffer);
  }
}