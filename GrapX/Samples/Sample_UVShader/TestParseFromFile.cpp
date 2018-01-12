#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ArithmeticExpression.h"
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "ExpressionSample.h"

#define SRC_ERROR_CASE_SIGN "//{ERROR_CASE}" // 14 chars

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

const int nTabSize = 2;
//////////////////////////////////////////////////////////////////////////
int DumpBlock(UVShader::CodeParser* pExpp, const UVShader::ArithmeticExpression::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr)
{
  typedef UVShader::ArithmeticExpression::SYNTAXNODE SYNTAXNODE;

  clStringA str[2];
  //int chain = 0;

  auto& mode = pNode->mode;
  int next_depth = (
    (mode == SYNTAXNODE::MODE_Block       ) ||
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
      if(pNode->Operand[i].GetType() == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
        str[i].Append(pNode->Operand[i].pTokn->ToString());
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
  const int retraction = depth * nTabSize;           // 缩进
  //const int next_retraction = (depth + 1) * 2;// 次级缩进
  switch(pNode->mode)
  {
  case SYNTAXNODE::MODE_FunctionCall: // 函数调用
    strOut.Format("%s(%s)", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_ArrayIndex:
    strOut.Format("%s[%s]", str[0], str[1]);
    break;

  case SYNTAXNODE::MODE_Definition:
    strOut.Format("%s %s", str[0], str[1]);
    break;

  //case SYNTAXNODE::MODE_DefinitionConst:
  //  strOut.Format("const %s %s", str[0], str[1]);
  //  break;

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

      if( ! str[0].EndsWith("\n")) {
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

void DumpMemory(const void* ptr, size_t count)
{
  // "0123456789ABCDEF 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00 ...............\r\n"
  char buffer[sizeof(void*) + 80];
  CLUINT_PTR nDisplay = ((CLUINT_PTR)ptr) & (~0xf);
  const char HexTab[] = "0123456789ABCDEF";
  const int offset = 49; // 16进制区与字符区的偏移
  size_t n = 0;

  while(n < count) {
    size_t i = 0;
    size_t i2 = 0;

    for(; i < (sizeof(void*) * 2); i++)
    {
      CLUINT_PTR mask = nDisplay >> ((sizeof(void*) * 8) - (i * 4) - 4);
      buffer[i] = HexTab[mask & 0xf];
    }

    buffer[i++] = 0x20;
    buffer[i++] = 0x20;
    i2 = i + offset;

    for(; nDisplay < (CLUINT_PTR)ptr; nDisplay++)
    {
      buffer[i2++] = 0x20;
      buffer[i++] = 0x20;
      buffer[i++] = 0x20;
      buffer[i++] = 0x20;
    }

    ptr = (void*)(((CLUINT_PTR)ptr & (~0xf)) + 16);
    for(; n < count && nDisplay < (CLUINT_PTR)ptr; n++, nDisplay++)
    {
      u8 c = *(u8*)nDisplay;
      if(c < 0x20) {
        buffer[i2++] = '.';
      }
      else if(c < 128) {
        buffer[i2++] = c;
      }
      else {
        buffer[i2++] = '?';
      }

      buffer[i++] = HexTab[(c >> 4) & 0xf];
      buffer[i++] = HexTab[c & 0xf];
      buffer[i++] = 0x20;
    }

    for(; nDisplay < (CLUINT_PTR)ptr; nDisplay++)
    {
      buffer[i2++] = 0x20;
      buffer[i++] = 0x20;
      buffer[i++] = 0x20;
      buffer[i++] = 0x20;
    }

    buffer[i++] = 0x20;
    buffer[i2++] = '\r';
    buffer[i2++] = '\n';
    buffer[i2++] = '\0';

    TRACE(buffer);
  }
}

void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput, GXLPCSTR szReference)
{
  clFile file;

  // 特殊文件不做处理
  if(clstd::strstrT<ch>((GXLPSTR)szFilename, "$CaseList$") != NULL) {
    return;
  }

  TRACE(
    "------------\n"
    "测试文件解析:\n"
    "------------\n"
    "%s\n", szFilename);

  if(file.OpenExisting(szFilename))
  {
    clBuffer* pBuffer = NULL;
    if(file.MapToBuffer(&pBuffer))
    {
      UVShader::CodeParser expp(NULL, NULL);
      const UVShader::ArithmeticExpression::TOKEN::Array* pTokens;
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

      b32 bErrorCase = (clstd::strncmpiT((LPCSTR)pBuffer->GetPtr(), SRC_ERROR_CASE_SIGN, 14) == 0);

      if(_CL_NOT_(bErrorCase) && szOutput != NULL)
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
                switch(s.defn.storage_class)
                {
                case UVShader::CodeParser::VariableStorageClass_extern:
                  file.WritefA("%s ", "extern");
                  break;
                case UVShader::CodeParser::VariableStorageClass_nointerpolation:
                  file.WritefA("%s ", "nointerpolation");
                  break;
                case UVShader::CodeParser::VariableStorageClass_precise:
                  file.WritefA("%s ", "precise");
                  break;
                case UVShader::CodeParser::VariableStorageClass_shared:
                  file.WritefA("%s ", "shared");
                  break;
                case UVShader::CodeParser::VariableStorageClass_groupshared:
                  file.WritefA("%s ", "groupshared");
                  break;
                case UVShader::CodeParser::VariableStorageClass_static:
                  file.WritefA("%s ", "static");
                  break;
                case UVShader::CodeParser::VariableStorageClass_uniform:
                  file.WritefA("%s ", "uniform");
                  break;
                case UVShader::CodeParser::VariableStorageClass_volatile:
                  file.WritefA("%s ", "volatile");
                  break;
                }

                switch(s.defn.modifier)
                {
                case UVShader::CodeParser::UniformModifier_const:
                  file.WritefA("%s ", "const");
                  break;
                case UVShader::CodeParser::UniformModifier_row_major:
                  file.WritefA("%s ", "row_major");
                  break;
                case UVShader::CodeParser::UniformModifier_column_major:
                  file.WritefA("%s ", "column_major");
                  break;
                }
                file.WritefA("%s", str);
                /*
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
                }*/
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
    if(file.OpenExisting(szReference)) {
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

//////////////////////////////////////////////////////////////////////////
b32 GetLine(const clstd::MemBuffer& buf, clStringA& str, size_t& pos)
{
  str.Clear();
  if(pos >= buf.GetSize()) {
    return FALSE;
  }

  clStringA::LPCSTR ptr = (clStringA::LPCSTR)buf.GetPtr() + pos;
  clStringA::LPCSTR pEnd = (clStringA::LPCSTR)buf.GetPtr() + buf.GetSize();
  while(*ptr != '\n' && ptr < pEnd)
  {
    str.Append(*ptr++);
  }

  if(*ptr == '\n') {
    str.Append(*ptr++);
  }

  pos = ptr - (clStringA::LPCSTR)buf.GetPtr();
  return TRUE;
}

void ExportTestCase(const clStringA& strPath)
{
  clstd::File file;
  clstd::MemBuffer buf;
  if(_CL_NOT_(file.OpenExisting(strPath))) {
    return;
  }

  if(_CL_NOT_(file.ReadToBuffer(&buf))) {
    return;
  }
  file.Close();

  clStringA str;
  clStringA strFilename;
  clStringA strDir = strPath;
  //clStringA strExportMsg;
  clmap<int, clStringA> sExportMsgDict;
  size_t pos_prev = 0;
  size_t pos = 0;
  int nLine = 0;
  b32 bErrorCase = // 标记输出文件是否为错误case
    (clstd::strncmpiT((GXLPCSTR)buf.GetPtr(), "$ErrorList", 10) == 0);

  clpathfile::RemoveFileSpec(strDir);
  
  while(GetLine(buf, str, pos))
  {
    nLine++;

    if(str.Compare("$-------", 8) == 0) // 忽略分割行
    {
      continue;
    }
    else if(str.Compare("$file=", 6) == 0) // 获得输出文件名
    {
      strFilename = str.CStr() + 6;
      strFilename.TrimRight("\r\n");
      if(strFilename.IsEmpty()) {
        TRACE("%s(%d) : \"$file\" 文件名为空\n", strPath.CStr(), nLine);
        return;
      }

      clpathfile::CombinePath(strFilename, strDir, strFilename);
      if(file.IsGood()) {
        file.Close();
      }

      if(file.CreateAlways(strFilename)) {
        if(bErrorCase) {
          // 如果标记为错误case, 在文件头加上这个标记
          // 测试程序会用来断言编译错误
          file.WritefA(SRC_ERROR_CASE_SIGN "\r\n");
        }
      }
      continue;
    }
    else if(str.Compare("$msg=", 5) == 0) // 错误消息
    {
      clStringA strMsgLine = str.CStr() + 5;
      clStringA strValue, strMsg;
      strMsgLine.TrimLeft("\r\n\t ");
      strMsgLine.TrimRight("\r\n\t ");
      strMsgLine.DivideBy('=', strValue, strMsg);
      if(strValue.IsEmpty() || strMsg.IsEmpty()) {
        TRACE("%s(%d) : \"$msg\" 格式错误\n", strPath.CStr(), nLine);
        return;
      }

      // 添加至字典并检查id是否重复
      auto r = sExportMsgDict.insert(clmake_pair(strValue.ToInteger(), strMsg));
      if(r.second == FALSE) {
        TRACE("%s(%d) : 重复id=%s\n", strPath.CStr(), nLine, strValue);
      }

      if(file.IsGood()) {
        file.WritefA("// %s\r\n", strMsg.CStr());
      }
      continue;
    }
    else if(str.Compare("$end", 4) == 0) // 结尾
    {
      break;
    }

    // 如果不是标记数据, 直接写入已经打开的文件
    if(file.IsGood()) {
      file.Write(str.CStr(), str.GetLength());
    }
  }

  // 输出所有错误消息, 符合stock文件格式
  for(auto it = sExportMsgDict.begin(); it != sExportMsgDict.end(); ++it)
  {
    TRACE("%d=\"%s\";\r\n", it->first, it->second.CStr()); // 这样才能正确显示"%s"字符串
  }
}