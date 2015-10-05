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
int DumpBlock(UVShader::CodeParser* pExpp, const UVShader::CodeParser::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr)
{
  typedef UVShader::CodeParser::SYNTAXNODE SYNTAXNODE;

  clStringA str[2];
  //int chain = 0;

  auto& mode = pNode->mode;
  int next_depth = (
    (mode == SYNTAXNODE::MODE_Block       ) ||
    (mode == SYNTAXNODE::MODE_Flow_DoWhile) ||
    (mode == SYNTAXNODE::MODE_Flow_ForRunning)
    ) ? depth + 1 : depth;

  if(mode == SYNTAXNODE::MODE_Flow_For)
  {
    CLNOP
  }
  
  //auto flag0 = pNode->GetOperandType(0);
  //if(flag0 == SYNTAXNODE::FLAG_OPERAND_IS_NODE && pNode->Operand[0].pNode != NULL && pNode->Operand[0].pNode->mode == SYNTAXNODE::MODE_Chain)
  //{
  //  CLNOP
  //}

  for(int i = 0; i < 2; i++)
  {
    if(pNode->Operand[i].ptr) {
      if(pExpp->TryGetNodeType(&pNode->Operand[i]) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
        str[i].Append(pNode->Operand[i].pSym->ToString());
      }
      else {
        DumpBlock(pExpp, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, next_depth, &str[i]);
      }
    }
    else {
      str[i].Clear();
    }
  }

  //if(str[0] == "count+=i" || str[1] == "count+=i")
  //{
  //  CLNOP
  //}

  //TRACE("[%s] [%s] [%s]\n",
  //  pNode->pOpcode ? pNode->pOpcode->ToString() : "",
  //  str[0], str[1]);

  clStringA strOut;
  const int retraction = depth * 2;           // 缩进
  //const int next_retraction = (depth + 1) * 2;// 次级缩进
  switch(pNode->mode)
  {
  case SYNTAXNODE::MODE_FunctionCall: // 函数调用
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Definition:
    strOut.Format("%s %s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_If:
    strOut.Format("if(%s) ", str[0]);
    strOut.Append(str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_Else:
    strOut.Append(str[0]);
    strOut.Append(' ', retraction);
    strOut.AppendFormat("else %s", str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_ElseIf:
    strOut.Append(str[0]);
    strOut.Append(' ', retraction);
    strOut.AppendFormat("else %s", str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_While:
    strOut.Format("while(%s) ", str[0]);
    strOut.Append(str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_DoWhile:
    strOut.Append("do {\n");
    if(str[1].IsNotEmpty()) {
      strOut.Append(str[1]);
    }
    strOut.Append(' ', retraction);
    strOut.AppendFormat("}while(%s)", str[0]);
    break;

  case SYNTAXNODE::MODE_Flow_For:
    strOut.AppendFormat("for(%s) ", str[0]);
    strOut.Append(str[1]);
    break;

  case SYNTAXNODE::MODE_Flow_ForInit:
  case SYNTAXNODE::MODE_Flow_ForRunning:
    strOut.Format("%s;%s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Return:
    ASSERT(str[0] == "return");
    strOut.AppendFormat("return %s;\n", str[1]);
    break;

  case SYNTAXNODE::MODE_Block:
    strOut.Append("{\n");
    strOut.Append(str[0]);
    strOut.Append(' ', retraction);
    strOut.Append("}\n");
    break;

  case SYNTAXNODE::MODE_Chain:
    //if(pNode->mode == SYNTAXNODE::MODE_ChainHead)
    //{
    //  strOut.Append("{\n");
    //}

    if(str[0].IsNotEmpty())
    {
      strOut.Append(' ', retraction);
      strOut.AppendFormat("%s", str[0]);

      if( ! str[0].EndsWith("\n")) {
        strOut.Append(";\n");
      }

      if(str[1].IsNotEmpty()) {
        //strOut.Append('$', retraction);
        strOut.AppendFormat("%s", str[1]);
      }
    }
    break;

  case SYNTAXNODE::MODE_Normal:
    if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
      strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->ToString(), str[1]);
    }
    else {
      strOut.Format("%s%s%s", str[0], pNode->pOpcode->ToString(), str[1]);
    }
    break;

  case SYNTAXNODE::MODE_Flow_Break:
    strOut = "break";
    break;

  default:
    // 没处理的 pNode->mode 类型
    CLBREAK;
    break;
  }

  //if(strOut.EndsWith(";\n")) {
  //  strOut.Insert(0, ' ', depth * 2);
  //}

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
      const UVShader::CodeParser::TokenArray* pTokens;
      expp.Attach((const char*)pBuffer->GetPtr(), pBuffer->GetSize());

      expp.GenerateTokens();
      pTokens = expp.GetTokensArray();
      int nCount = 0;
      for(auto it = pTokens->begin(); it != pTokens->end(); ++it, ++nCount)
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

      TRACE("\ncount:%d(%f)\n", pTokens->size(), (float)pBuffer->GetSize() / pTokens->size());

      if(strcmpi(szFilename, "Test\\shaders\\ShaderToy\\GLSL smallpt.txt") == 0) 
      {
        CLNOP
      }
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
            case UVShader::CodeParser::StatementType_Definition:
              {
                const auto& definition = s.defn;
                file.WritefA("%s %s", definition.szType, definition.szName);
                if(definition.sRoot.ptr) {
                  clStringA str;
                  auto type = expp.TryGetNodeType(&definition.sRoot);
                  if(type == UVShader::CodeParser::SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
                    str = definition.sRoot.pSym->ToString();
                    file.WritefA("=%s", str);
                  }
                  else if(type == UVShader::CodeParser::SYNTAXNODE::FLAG_OPERAND_IS_NODE) {
                    if(definition.sRoot.pNode->mode != UVShader::CodeParser::SYNTAXNODE::MODE_Undefined) {
                      DumpBlock(&expp, definition.sRoot.pNode, 0, 0, &str);
                      file.WritefA("=%s", str);
                    }
                  }
                  else {
                    CLBREAK;
                  }
                }
                file.WritefA(";\n");
              }
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

                //file.WritefA("{\n");
                if(func.pExpression && func.pExpression->expr.sRoot.pNode)
                {
                  clStringA str;
                  //str.Format("{\n");
                  //DbgDumpSyntaxTree(&expp, func.pExpression->expr.sRoot.pNode, 0, 1, &str);
                  DumpBlock(&expp, func.pExpression->expr.sRoot.pNode, 0, 0, &str);
                  file.WritefA(str); // 缩进两个空格
                }
                file.WritefA("\n");
                //file.WritefA("}\n\n");
              }
              break;
            case UVShader::CodeParser::StatementType_Struct:
              break;
            case UVShader::CodeParser::StatementType_Signatures:
              break;
            case UVShader::CodeParser::StatementType_Expression:
              {
                clStringA str;
                CLBREAK;
                DumpBlock(&expp, s.expr.sRoot.pNode, 0, 1, &str);
                file.WritefA(str);
              }
              break;
            default:
              CLBREAK;
              break;
            }
          }
        }
      }
    }
    SAFE_DELETE(pBuffer);
  }
}