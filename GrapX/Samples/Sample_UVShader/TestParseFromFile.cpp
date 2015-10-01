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

int DbgDumpSyntaxTree(UVShader::CodeParser* pExpp, const UVShader::CodeParser::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr)
{
  typedef UVShader::CodeParser::SYNTAXNODE SYNTAXNODE;

  clStringA str[2];
  //int chain = 0;

  //int next_depth = depth;
  int next_depth = (
    (pNode->mode == SYNTAXNODE::MODE_Flow_For    ) ||
    (pNode->mode == SYNTAXNODE::MODE_Flow_While  ) ||
    (pNode->mode == SYNTAXNODE::MODE_Flow_If     ) ||
    (pNode->mode == SYNTAXNODE::MODE_Flow_DoWhile) ||
    //(pNode->mode == SYNTAXNODE::MODE_Flow_Else   ) ||
    (pNode->mode == SYNTAXNODE::MODE_Flow_For    ) ||
    (pNode->mode == SYNTAXNODE::MODE_Flow_ForRunning)
    ) ? depth + 1 : depth;

  for(int i = 0; i < 2; i++)
  {

    if(pNode->Operand[i].pSym) {
      if(pExpp->IsSymbol(&pNode->Operand[i])) {
        str[i].Append(pNode->Operand[i].pSym->ToString());
      }
      else {
        DbgDumpSyntaxTree(pExpp, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, next_depth, &str[i]);
      }
    }
    else {
      str[i].Clear();
    }
  }

  //if(str[0] == "glowed=true" || str[1] == "glowed=true")
  //{
  //  CLNOP
  //}

  TRACE("[%s] [%s] [%s]\n",
    pNode->pOpcode ? pNode->pOpcode->ToString() : "",
    str[0], str[1]);

  clStringA strOut;
  const int retraction = depth * 2;           // 缩进
  const int next_retraction = (depth + 1) * 2;// 次级缩进
  switch(pNode->mode)
  {
  case SYNTAXNODE::MODE_FunctionCall: // 函数调用
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Definition:
    strOut.Format("%s %s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_If:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("if(%s) {\n%*s%s%*s}\n", str[0], next_retraction, " ", str[1], retraction, " ");
    break;

  case SYNTAXNODE::MODE_Flow_Else:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("%s%*selse {\n%*s%s%*s}\n", str[0], retraction, " ", next_retraction, " ", str[1], retraction, " ");
    break;

  case SYNTAXNODE::MODE_Flow_ElseIf:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("%s%*selse %s", str[0], retraction, " ", str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_While:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("while(%s) {\n%*s%s%*s}\n", str[0], next_retraction, " ", str[1], retraction, " ");
    break;

  case SYNTAXNODE::MODE_Flow_DoWhile:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("do {\n%*s%s%*s}while(%s);\n", next_retraction, " ", str[1], retraction, " ", str[0]);
    break;

  case SYNTAXNODE::MODE_Flow_For:
    if( ! str[1].EndsWith('\n')) {
      str[1].Append(";\n");
    }
    strOut.Format("for(%s) {\n%*s%s%*s}\n", str[0], next_retraction, " ", str[1], retraction, " ");
    break;

  case SYNTAXNODE::MODE_Flow_ForInit:
  case SYNTAXNODE::MODE_Flow_ForRunning:
    strOut.Format("%s;%s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Return:
    ASSERT(str[0] == "return");
    strOut.Format("return %s;\n", str[1]);
    break;

  case SYNTAXNODE::MODE_Chain:
    if(str[1].IsEmpty()) {
      strOut.Format("%s;\n", str[0]);
    }
    else if(str[0].EndsWith("\n")) {
      strOut.Format("%s%*s%s", str[0], retraction, " ", str[1]);
    }
    else {
      strOut.Format("%s;\n%*s%s", str[0], retraction, " ", str[1]);
    }
    break;

  case SYNTAXNODE::MODE_Normal:
    if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
      strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->ToString(), str[1]);
      //chain = 1;
    }
    else {
      strOut.Format("%s%s%s", str[0], pNode->pOpcode->ToString(), str[1]);
      //chain++;
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
  return 0;
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
        if(it->scope >= 0 && it->semi_scope >= 0) {
          TRACE("<#%d:\"%s\"(%d|%d)> ", nCount, it->ToString(), it->scope, it->semi_scope);
        }
        else if(it->scope >= 0) {
          TRACE("<#%d:\"%s\"(%d|)> ", nCount, it->ToString(), it->scope);
        }
        else if(it->semi_scope >= 0) {
          TRACE("<#%d:\"%s\"(|%d)> ", nCount, it->ToString(), it->semi_scope);
        }
        else {
          TRACE("<#%d:\"%s\"> ", nCount, it->ToString());
        }
        if(((nCount + 1) % 10) == 0 && nCount != 0) {
          TRACE("\n");
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
                if(func.pExpression && func.pExpression->expr.sRoot.pNode)
                {
                  clStringA str;
                  DbgDumpSyntaxTree(&expp, func.pExpression->expr.sRoot.pNode, 0, 1, &str);
                  file.WritefA("  %s", str); // 缩进两个空格
                }
                file.WritefA("}\n\n");
              }
              break;
            case UVShader::CodeParser::StatementType_Struct:
              break;
            case UVShader::CodeParser::StatementType_Signatures:
              break;
            case UVShader::CodeParser::StatementType_Expression:
              {
                clStringA str;
                DbgDumpSyntaxTree(&expp, s.expr.sRoot.pNode, 0, 1, &str);
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