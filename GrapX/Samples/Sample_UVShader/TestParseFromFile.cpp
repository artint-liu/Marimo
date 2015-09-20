#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

GXLPCSTR Get(UVShader::CodeParser::InputModifier e)
{
  switch(e)
  {
  case UVShader::CodeParser::InputModifier_in:
    return "in ";
  case UVShader::CodeParser::InputModifier_out:
    return "out ";
  case UVShader::CodeParser::InputModifier_inout:
    return "intout ";
  case UVShader::CodeParser::InputModifier_uniform:
    return "uniform ";
  default:
    CLBREAK;
  }
}

GXLPCSTR Get(UVShader::CodeParser::StorageClass e)
{
  switch(e)
  {
  case UVShader::CodeParser::StorageClass_empty:
    return "";
  case UVShader::CodeParser::StorageClass_inline:
    return "inline ";
  default:
    CLBREAK;
  }
}

//////////////////////////////////////////////////////////////////////////

void DbgDumpSyntaxTree(UVShader::CodeParser* pExpp, const UVShader::CodeParser::SYNTAXNODE* pNode, int precedence, clStringA* pStr)
{
  typedef UVShader::CodeParser::SYNTAXNODE SYNTAXNODE;

  clStringA str[2];
  for(int i = 0; i < 2; i++)
  {
    if(pNode->Operand[i].pSym) {
      if(pExpp->IsSymbol(&pNode->Operand[i])) {
        str[i] = pNode->Operand[i].pSym->sym.ToString();
      }
      else {
        DbgDumpSyntaxTree(pExpp, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
      }
    }
    else {
      str[i].Clear();
    }
  }

  TRACE("[%s] [%s] [%s]\n",
    pNode->pOpcode ? pNode->pOpcode->sym.ToString() : "",
    str[0], str[1]);

  clStringA strOut;
  switch(pNode->mode)
  {
  case SYNTAXNODE::MODE_FunctionCall: // 函数调用
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Definition:
    strOut.Format("%s %s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_If:
    strOut.Format("if(%s) {\n%s;\n}", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_Else:
    strOut.Format("%s else {\n%s;\n}", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_ElseIf:
    strOut.Format("%s else %s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_While:
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_For:
    strOut.Format("for(%s) {\n%s\n}", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_ForInit:
  case SYNTAXNODE::MODE_Flow_ForRunning:
    strOut.Format("%s;%s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Return:
    ASSERT(str[0] == "return");
    strOut.Format("return %s", str[1]);
    break;

  case SYNTAXNODE::MODE_Chain:
    if(str[1].IsEmpty()) {
      strOut.Format("%s;\n", str[0]);
    }
    else {
      strOut.Format("%s;\n%s", str[0], str[1]);
    }
    break;

  case SYNTAXNODE::MODE_Normal:
    if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
      strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->sym.ToString(), str[1]);
    }
    else {
      strOut.Format("%s%s%s", str[0], pNode->pOpcode->sym.ToString(), str[1]);
    }
    break;

  default:
    // 没处理的 pNode->mode 类型
    CLBREAK;
    break;
  }

  if(pStr) {
    *pStr = strOut;
  }
  else {
    TRACE("%s\n", strOut);
  }
}
void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput)
{
  clFile file;
  if(file.OpenExistingA(szFilename))
  {
    clBuffer* pBuffer = NULL;
    if(file.MapToBuffer(&pBuffer))
    {
      UVShader::CodeParser expp;
      const UVShader::CodeParser::SymbolArray* pSymbols;
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
            case UVShader::CodeParser::StatementType_FunctionDecl:
              break;
            case UVShader::CodeParser::StatementType_Function:
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
                if(func.pExpression)
                {
                  clStringA str;
                  DbgDumpSyntaxTree(&expp, func.pExpression->expr.sRoot.pNode, 0, &str);
                  file.WritefA("%s;\n", str);
                }
                file.WritefA("}\n");
              }
              break;
            case UVShader::CodeParser::StatementType_Struct:
              break;
            case UVShader::CodeParser::StatementType_Signatures:
              break;
            case UVShader::CodeParser::StatementType_Expression:
              {
                clStringA str;
                DbgDumpSyntaxTree(&expp, s.expr.sRoot.pNode, 0, &str);
                file.WritefA(str);
              }
              break;
            }
          }
        }
      }
    }
    SAFE_DELETE(pBuffer);
  }
}