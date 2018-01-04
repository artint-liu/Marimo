// Sample_FrameShader.cpp : 定义控制台应用程序的入口点。
//
#include "Marimo.h"
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ArithmeticExpression.h"
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"

GXLPCSTR Get(UVShader::CodeParser::InputModifier e)
{
  switch(e)
  {
  case UVShader::CodeParser::InputModifier_in:
    return "in ";
  case UVShader::CodeParser::InputModifier_out:
    return "out ";
  case UVShader::CodeParser::InputModifier_inout:
    return "inout ";
  case UVShader::CodeParser::InputModifier_uniform:
    return "uniform ";
  default:
    CLBREAK;
  }
}

GXLPCSTR Get(UVShader::CodeParser::FunctionStorageClass e)
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
const int nTabSize = 2;

int DumpBlock(UVShader::CodeParser* pExpp, const UVShader::CodeParser::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr)
{
  typedef UVShader::CodeParser::SYNTAXNODE SYNTAXNODE;

  clStringA str[2];
  //int chain = 0;

  auto& mode = pNode->mode;
  int next_depth = (
    (mode == SYNTAXNODE::MODE_Block) ||
    (mode == SYNTAXNODE::MODE_Flow_DoWhile) ||
    (mode == SYNTAXNODE::MODE_Flow_ForRunning)
    ) ? depth + 1 : depth;

  if(mode == SYNTAXNODE::MODE_StructDef)
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
      //if(pExpp->TryGetNodeType(&pNode->Operand[i]) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
      SYNTAXNODE::FLAGS fg = pNode->GetOperandType(i);
      if(fg == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN)
      {
        str[i].Append(pNode->Operand[i].pTokn->ToString());
      }
      else if(fg == SYNTAXNODE::FLAG_OPERAND_IS_NODE)
      {
        DumpBlock(pExpp, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, next_depth, &str[i]);
      }
      else {
        CLBREAK;
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
  const int retraction = depth * nTabSize;           // 缩进
                                                     //const int next_retraction = (depth + 1) * 2;// 次级缩进
  switch(pNode->mode)
  {
  case SYNTAXNODE::MODE_FunctionCall: // 函数调用
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_ArrayAlloc:
    ASSERT(str[1].IsEmpty());
    strOut.Format("%s[]", str[0]);
    break;

  case SYNTAXNODE::MODE_ArrayIndex:
    strOut.Format("%s[%s]", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Definition:
    strOut.Format("%s %s", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_DefinitionConst:
    strOut.Format("const %s %s", str[0], str[1]);
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
    if(str[1].IsNotEmpty()) {
      ASSERT(str[1] == ";"); // 目前只可能是分号
      strOut.AppendFormat("}%s\n", str[1]);
    }
    else {
      strOut.Append("}\n");
    }
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

      if(!str[0].EndsWith("\n")) {
        strOut.Append(";\n");
      }

      if(str[1].IsNotEmpty()) {
        //strOut.Append('$', retraction);
        strOut.AppendFormat("%s", str[1]);
      }
    }
    break;

  case SYNTAXNODE::MODE_StructDef:
    strOut.Format("struct %s ", str[0]);
    strOut.Append(str[1]);
    break;

  case SYNTAXNODE::MODE_Opcode:
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

  case SYNTAXNODE::MODE_Flow_Continue:
    strOut = "continue";
    break;

  case SYNTAXNODE::MODE_Flow_Discard:
    strOut = "discard";
    break;

  case SYNTAXNODE::MODE_ArrayAssignment:
    strOut.Format("%s={%s}", str[0], str[1]);
    break;

  default:
    // 没处理的 pNode->mode 类型
    strOut = "[!ERROR!]";
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

void DumpStructMembers(clStringA& str, int depth, UVShader::CodeParser::STRUCT_MEMBER* pMembers, clsize nNumOfMembers)
{
  for(clsize i = 0; i < nNumOfMembers; i++)
  {
    str.AppendFormat("%*s%s %s", depth * nTabSize, " ", pMembers[i].szType, pMembers[i].szName);
    if(pMembers[i].szSignature) {
      str.AppendFormat(" : %s;\n", pMembers[i].szSignature);
    }
    else {
      str.Append(";\n");
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput, GXLPCSTR szReference)
{
  clFile file;
  if(file.OpenExisting(szFilename))
  {
    clBuffer* pBuffer = NULL;
    if(file.MapToBuffer(&pBuffer))
    {
      UVShader::CodeParser expp(NULL, NULL);
      const UVShader::CodeParser::TOKEN::Array* pTokens;
      clStringW strFullname = szFilename;
      clpathfile::CombineAbsPath(strFullname);
      expp.Attach((const char*)pBuffer->GetPtr(), pBuffer->GetSize(), 0, strFullname);

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

      //if(strcmpi(szFilename, "Test\\shaders\\ShaderToy\\GLSL smallpt.txt") == 0) 
      //{
      //  CLNOP
      //}

      // 解析语法
      expp.Parse();

      if(szOutput != NULL)
      {
        clstd::File file;
        if(file.CreateAlways(szOutput))
        {
          const UVShader::CodeParser::StatementArray& stats = expp.GetStatments();
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
              clStringA str;
              DumpBlock(&expp, definition.sRoot.un.pNode, 0, 0, &str);
              file.WritefA("%s", str);
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
              if(func.pExpression && func.pExpression->expr.sRoot.un.pNode)
              {
                clStringA str;
                //str.Format("{\n");
                //DbgDumpSyntaxTree(&expp, func.pExpression->expr.sRoot.pNode, 0, 1, &str);
                DumpBlock(&expp, func.pExpression->expr.sRoot.un.pNode, 0, 0, &str);
                file.WritefA(str); // 缩进两个空格
              }
              else
              {
                file.WritefA("{\n}\n");
              }
              file.WritefA("\n");
              //file.WritefA("}\n\n");
            }
            break;
            case UVShader::CodeParser::StatementType_Struct:
            case UVShader::CodeParser::StatementType_Signatures:
            {
              const auto& stru = s.stru;
              clStringA str;
              file.WritefA("struct %s {\n", stru.szName);
              DumpStructMembers(str, 1, stru.pMembers, stru.nNumOfMembers);
              file.WritefA(str);
              file.WritefA("};\n");
            }
            break;
            case UVShader::CodeParser::StatementType_Expression:
            {
              clStringA str;
              CLBREAK;
              DumpBlock(&expp, s.expr.sRoot.un.pNode, 0, 1, &str);
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

  // TODO: 对比输出文件与参考文件
  do {
    if(szReference != NULL && clstd::strlenT(szReference) > 0 &&
      file.OpenExisting(szReference))
    {
      clBuffer* pRefBuffer = NULL;
      clBuffer* pOutBuffer = NULL;
      clFile sOutFile;

      if( ! sOutFile.OpenExisting(szOutput)) {
        break;
      }

      file.MapToBuffer(&pRefBuffer);
      sOutFile.MapToBuffer(&pOutBuffer);

      clstd::TokensA tokens_ref;
      clstd::TokensA tokens_out;

      tokens_ref.Attach((GXLPCSTR)pRefBuffer->GetPtr(), pRefBuffer->GetSize());
      tokens_out.Attach((GXLPCSTR)pOutBuffer->GetPtr(), pOutBuffer->GetSize());

      clstd::TokensA::iterator it_ref = tokens_ref.begin();
      clstd::TokensA::iterator it_out = tokens_out.begin();

      for(; it_ref != tokens_ref.end() && it_out != tokens_out.end(); ++it_ref, ++it_out)
      {
        if(it_ref.ToString() != it_out.ToString())
        {
          printf("\"%s\" not equals \"%s\"\n", it_out.ToString(), it_ref.ToString());
          CLBREAK;
        }
      }

      if(it_ref != tokens_ref.end() || it_out != tokens_out.end())
      {
        printf("Not same end.\n");
        CLBREAK;
      }

      /*
      clStringA strRef((GXLPCSTR)pRefBuffer->GetPtr(), pRefBuffer->GetSize());
      clStringA strOut((GXLPCSTR)pOutBuffer->GetPtr(), pOutBuffer->GetSize());

      // FIXME: 这里不对 清除空格会导致语法变化
      strRef.Remove(' ');
      strRef.Remove('\t');
      strRef.Remove('\n');
      strRef.Remove('\a');

      strOut.Remove(' ');
      strOut.Remove('\t');
      strOut.Remove('\n');
      strOut.Remove('\a');

      auto outLen = strOut.GetLength();
      auto refLen = strRef.GetLength();
      auto len = clMin(outLen, refLen);

      if(outLen != refLen)
      {
      printf("%s not equals %s\n", szOutput, szReference);
      }

      for(size_t i = 0; i < len; i++)
      {
      ch& c1 = strOut[i];
      ch& c2 = strRef[i];

      if(c1 != c2) {
      TRACE("\n");
      DumpMemory(&c1, 48);
      TRACE("--------------------------------------------------------\n");
      DumpMemory(&c2, 48);
      TRACE("\n");
      CLBREAK;
      break;
      }
      }
      //*/

      SAFE_DELETE(pRefBuffer);
      SAFE_DELETE(pOutBuffer);
    }
  } while(0);
}

int main(int argc, char* argv[])
{
  clpathfile::LocalWorkingDirW(_CLTEXT(".."));
  //TestFromFile("Test\\FrameShader\\SimpleShader_ps.hlsl", "Test\\FrameShader\\SimpleShader_ps.out.hlsl", NULL);
  TestFromFile("Test\\FrameShader\\AtmoScatt_FrameShader.hlsl", "Test\\FrameShader\\AtmoScatt_FrameShader.out.hlsl", NULL);
	return 0;
}

