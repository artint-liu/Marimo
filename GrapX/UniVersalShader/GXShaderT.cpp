// 全局头文件
#include <GrapX.h>
#include <clTokens.h>
#include <clStock.h>
#include <clTextLines.h>
#include <clStringSet.h>
#include <clStablePool.h>
#include "ArithmeticExpression.h"
#include "ExpressionParser.h"
#include <GrapX/GXShaderT.h>

namespace GrapX
{
  namespace ShaderT
  {
    const int nTabSize = 2;
    GXLPCSTR Get(UVShader::CodeParser::FunctionStorageClass e);
    GXLPCSTR Get(UVShader::InputModifier e);
    int DumpBlock(const UVShader::CodeParser* pExpp, const UVShader::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr);

    Script::Script()
      : m_stock(clstd::StockA::StockFlag_Version2)
    {
    }

    Script::~Script()
    {
      for(ShaderObject* pShaderObject : m_ShaderList)
      {
        SAFE_DELETE(pShaderObject);
      }
    }

    GXBOOL Script::Load(GXLPCWSTR szFilename)
    {
      clstd::File file;
      clstd::MemBuffer buf;
      if(_CL_NOT_(file.OpenExisting(szFilename))) {
        return FALSE;
      }

      if(_CL_NOT_(file.ReadToBuffer(&buf))) {
        return FALSE;
      }

      return Load(buf, szFilename);
    }

    GXBOOL Script::Load(GXLPCSTR szFilename)
    {
      clStringW strFilename = szFilename;
      return Load(strFilename);
    }

    GXBOOL Script::Load(const clstd::BufferBase& buf, GXLPCWSTR szFilename)
    {
      if(_CL_NOT_(m_stock.Set(&buf))) {
        return FALSE;
      }

      m_strFilename = szFilename;

      m_lines.Generate(buf.CastPtr<ch>(), buf.CastSize<ch>());
      clstd::StockA::Section sect = m_stock.OpenSection("shader");
      if(sect)
      {
        do {
          TRACE("shader:%s\n", sect.name.ToString());
          ShaderObject* pShaderObject = new ShaderObject(m_strFilename, m_stock, m_lines);
          if(pShaderObject->InitObject(sect)) {
            m_ShaderList.push_back(pShaderObject);
          }
          else {
            SAFE_DELETE(pShaderObject);
          }
        } while (sect.NextSection("shader"));
      }

      return FALSE;
    }

    //////////////////////////////////////////////////////////////////////////

    ShaderObject::ShaderObject(const clStringW& strFilename, const clstd::StockA& stock, const clstd::TextLines<ch>& lines)
      : m_strFilename(strFilename)
      , m_stock(stock)
      , m_lines(lines)
    {
    }

    ShaderObject::~ShaderObject()
    {
    }

    GXBOOL ShaderObject::InitObject(const clstd::StockA::Section& sect)
    {
      sect.ForEachSection("pass",
        [this](const clstd::StockA::Section& sect_pass) -> b32
      {
        InitPass(sect_pass);
        return TRUE;
      });
      //clstd::StockA::Section sect_pass = sect.Open("pass");
      //if(sect_pass)
      //{
      //  do {
      //    InitPass(sect_pass);
      //  } while (sect_pass.NextSection("pass"));
      //}
      return TRUE;
    }

    GXBOOL ShaderObject::InitPass(const clstd::StockA::Section& sect_pass)
    {
      sect_pass.ForEachKey([](const clstd::StockA::ATTRIBUTE& attr) -> b32
      {
        if(attr.key == "vertex") {
          TRACE("vertex shader:%s\n", attr.value.ToString());
        }
        else if(attr.key == "pixel") {
          TRACE("pixel shader:%s\n", attr.value.ToString());
        }
        return TRUE;
      });

      clstd::MemBuffer code_buf;

      sect_pass.ForEachSection("code", [this, &code_buf](const clstd::StockA::Section& sect_code) -> b32
      {
        auto iter_code = sect_code.iter_begin + 1;
        clStringA strLine;
        int line, row;
        const size_t nCodeLength = sizeof(clStringA::TChar) *
          (sect_code.iter_end.marker - iter_code.marker);
        m_lines.PosFromOffset(iter_code.offset(), &line, &row);
        strLine.Format("#line %d\r\n", line);
        code_buf.Reserve(code_buf.GetSize() + strLine.GetLength() + nCodeLength);

        code_buf.Append(strLine.CStr(), strLine.GetLength() * sizeof(clStringA::TChar));
        code_buf.Append(iter_code.marker, nCodeLength);

#if 1
        clstd::File file;
        if(file.CreateAlways("test_output_code.txt"))
        {
          file.Write(code_buf);
          file.Close();
        }
#endif
        return TRUE;
      });

      UVShader::CodeParser parser;
      parser.Attach(code_buf.CastPtr<char>(), code_buf.CastSize<char>(), 0, m_strFilename);
      if(parser.GenerateTokens() > 0 && parser.Parse())
      {
        CodeInstance ci;
        ci.Init(parser);
        ci.SaveToFile(_CLTEXT("test_code_instance.txt"));
      }

      return TRUE;
    }
            
    int DumpBlock(const UVShader::CodeParser* pExpp, const UVShader::SYNTAXNODE* pNode, int precedence, int depth, clStringA* pStr)
    {
      typedef UVShader::SYNTAXNODE SYNTAXNODE;

      clStringA str[2];
      //int chain = 0;
      const int retraction = depth * nTabSize;           // 缩进

      auto& mode = pNode->mode;
      int next_depth = (
        (mode == SYNTAXNODE::MODE_Block) ||
        (mode == SYNTAXNODE::MODE_Flow_DoWhile) ||
        (mode == SYNTAXNODE::MODE_Flow_ForRunning)
        ) ? depth + 1 : depth;

      clStringA strComma;
      clStringA strOut;

    REDUMP_NODES:
      for(int i = 0; i < 2; i++)
      {
        if(pNode->Operand[i].ptr) {
          //if(pExpp->TryGetNodeType(&pNode->Operand[i]) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          if(pNode->Operand[i].IsToken()) {
            if(pNode->Operand[i].pTokn->HasReplacedValue()) {
              UVShader::VALUE value;
              pExpp->GetRepalcedValue(value, pNode->Operand[i]);
              str[i].Append(value.ToString());
            }
            else {
              str[i].Append(pNode->Operand[i].pTokn->ToString());
            }
          }
          else if(pNode->Operand[i].IsReplaced()) {
            UVShader::VALUE value;
            pExpp->GetRepalcedValue(value, pNode->Operand[i]);
            str[i].Append(value.ToString());
          }
          else if(mode == SYNTAXNODE::MODE_CommaList && pNode->Operand[i].CompareAsNode(SYNTAXNODE::MODE_CommaList))
          {
            ASSERT(i == 1);
            break;
          }
          else if(mode == SYNTAXNODE::MODE_Chain && i == 1)
          {
            ASSERT(i == 1);
            break;
          }
          else {
            DumpBlock(pExpp, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, next_depth, &str[i]);
          }
        }
        else {
          str[i].Clear();
        }
      }

      if(pNode->mode == SYNTAXNODE::MODE_CommaList && pNode->Operand[1].CompareAsNode(SYNTAXNODE::MODE_CommaList))
      {
        ASSERT(strOut.IsEmpty()); // 没处理这种情况

        strComma.AppendFormat("%s,", str[0]);
        str[0].Clear();
        pNode = pNode->Operand[1].pNode;
        goto REDUMP_NODES;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Chain)
      {
        ASSERT(strComma.IsEmpty()); // 没处理这种情况

        if(str[0].IsNotEmpty())
        {
          if(!str[0].EndsWith("\n")) {
            strOut.Append(' ', retraction).AppendFormat("%s;\n", str[0]);
          }
          else {
            strOut.Append(' ', retraction).AppendFormat("%s", str[0]);
          }
        }

        if(pNode->Operand[1].ptr)
        {
          ASSERT(pNode->Operand[1].IsNode()); // 只能是node
          pNode = pNode->Operand[1].pNode;
          str[0].Clear();
          goto REDUMP_NODES;
        }
        else
        {
          goto FUNC_RET;
        }
      }


      //if(str[0] == "count+=i" || str[1] == "count+=i")
      //{
      //  CLNOP
      //}

      //TRACE("[%s] [%s] [%s]\n",
      //  pNode->pOpcode ? pNode->pOpcode->ToString() : "",
      //  str[0], str[1]);

      //const int next_retraction = (depth + 1) * 2;// 次级缩进
      switch(pNode->mode)
      {
      case SYNTAXNODE::MODE_FunctionCall: // 函数调用
        strOut.Format("%s(%s)", str[0], str[1]);
        break;

      case SYNTAXNODE::MODE_Subscript:
        strOut.Format("%s[%s]", str[0], str[1]);
        break;

      case SYNTAXNODE::MODE_Definition:
        strOut.Format("%s %s", str[0], str[1]);
        break;

      case SYNTAXNODE::MODE_TypeCast:
        strOut.Format("(%s)(%s)", str[0], str[1]);
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
        CLBREAK; // 前面处理了
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

      case SYNTAXNODE::MODE_Typedef:
        strOut.Format("typedef %s %s;\n", str[0], str[1]);
        break;

      case SYNTAXNODE::MODE_Subscript0:
        if(str[1].IsEmpty())
        {
          strOut.Format("%s[]", str[0]);
        }
        else
        {
          strOut.Format("%s[%s]", str[0], str[1]);
        }
        break;

      case SYNTAXNODE::MODE_Assignment:
        strOut.Format("%s=%s", str[0], str[1]);
        break;

      case SYNTAXNODE::MODE_InitList:
        ASSERT(pNode->Operand[1].ptr == NULL);
        strOut.Format("{%s}", str[0]);
        break;

      case SYNTAXNODE::MODE_CommaList:
        if(strComma.IsNotEmpty()) {
          strOut.Format("%s%s,%s", strComma, str[0], str[1]);
        }
        else {
          strOut.Format("%s,%s", str[0], str[1]);
        }
        break;

      default:
        // 没处理的 pNode->mode 类型
        strOut = "[!ERROR!]";
        CLBREAK;
        break;
      }

      //if(strOut.EndsWith(";\n")) {
      //  strOut.Insert(0, ' ', depth * 2);
      //}
    FUNC_RET:
      if(pStr) {
        *pStr = strOut;
      }
      else {
        TRACE("%s\n", strOut);
      }
      return 0;
    }


    void CodeInstance::DumpVariableStorageClass(UVShader::VariableStorageClass storage_class)
    {
      switch(storage_class)
      {
      case UVShader::VariableStorageClass_extern:
        WriteFormat("%s ", "extern");
        break;
      case UVShader::VariableStorageClass_nointerpolation:
        WriteFormat("%s ", "nointerpolation");
        break;
      case UVShader::VariableStorageClass_precise:
        WriteFormat("%s ", "precise");
        break;
      case UVShader::VariableStorageClass_shared:
        WriteFormat("%s ", "shared");
        break;
      case UVShader::VariableStorageClass_groupshared:
        WriteFormat("%s ", "groupshared");
        break;
      case UVShader::VariableStorageClass_static:
        WriteFormat("%s ", "static");
        break;
      case UVShader::VariableStorageClass_uniform:
        WriteFormat("%s ", "uniform");
        break;
      case UVShader::VariableStorageClass_volatile:
        WriteFormat("%s ", "volatile");
        break;
      }
    }

    void CodeInstance::DumpVariableModifier(UVShader::UniformModifier modifier)
    {
      switch(modifier)
      {
      case UVShader::UniformModifier_const:
        WriteFormat("%s ", "const");
        break;
      case UVShader::UniformModifier_row_major:
        WriteFormat("%s ", "row_major");
        break;
      case UVShader::UniformModifier_column_major:
        WriteFormat("%s ", "column_major");
        break;
      }
    }

    GXLPCSTR Get(UVShader::InputModifier e)
    {
      switch(e)
      {
      case 0:
        return "";
      case UVShader::InputModifier_in:
        return "in ";
      case UVShader::InputModifier_out:
        return "out ";
      case UVShader::InputModifier_inout:
        return "inout ";
      case UVShader::InputModifier_uniform:
        return "uniform ";
      case (UVShader::InputModifier_uniform | UVShader::InputModifier_in):
        return "const in ";
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

    CodeInstance::CodeInstance()
    {
    }

    CodeInstance::~CodeInstance()
    {
    }

    GXBOOL CodeInstance::Init(const UVShader::CodeParser& parser)
    {
      m_buffer.Reserve(parser.GetStreamCount());
      Dump(parser);
      return TRUE;
    }

    GXBOOL CodeInstance::SaveToFile(GXLPCWSTR szFilename)
    {
      clstd::File file;
      if(file.CreateAlways(szFilename)) {
        file.Write(m_buffer);
        return TRUE;
      }
      return FALSE;
    }

    void CodeInstance::WriteFormat(GXLPCSTR szFormat, ...)
    {
      va_list arglist;
      va_start(arglist, szFormat);

      clStringA buffer;
      buffer.VarFormat(szFormat, arglist);
      m_buffer.Append(buffer.CStr(), buffer.GetLength() * sizeof(clStringA::TChar));

      va_end(arglist);
    }

    void CodeInstance::Dump(const UVShader::CodeParser& parser)
    {
      const UVShader::CodeParser::StatementArray& stats = parser.GetStatements();
      for(auto it = stats.begin(); it != stats.end(); ++it)
      {
        const auto& s = *it;
        switch(s.type)
        {
        case UVShader::CodeParser::StatementType_FunctionDecl:
          break;
        case UVShader::CodeParser::StatementType_Definition:
        {
          const auto& definition = s;
          clStringA str;
          if(definition.sRoot.pNode->mode == UVShader::SYNTAXNODE::MODE_Definition)
          {
            DumpBlock(&parser, definition.sRoot.pNode, 0, 0, &str);
            DumpVariableStorageClass(s.defn.storage_class);
            DumpVariableModifier(s.defn.modifier);
            WriteFormat("%s;\n", str);
          }
          else if(definition.sRoot.pNode->mode == UVShader::SYNTAXNODE::MODE_Chain)
          {
            UVShader::SYNTAXNODE* pNode = definition.sRoot.pNode;
            while(pNode)
            {
              DumpBlock(&parser, pNode->Operand[0].pNode, 0, 0, &str);
              DumpVariableStorageClass(s.defn.storage_class);
              DumpVariableModifier(s.defn.modifier);
              WriteFormat("%s;\n", str);
              pNode = pNode->Operand[1].pNode;
            }
          }
          else
          {
            CLBREAK;
          }
        }
        break;
        case UVShader::CodeParser::StatementType_Function:
        {
          const auto& func = s.func;

          clStringA strArgs;
          //clStringA strType;
          //clStringA strName;
          //for(clsize i = 0; i < func.nNumOfArguments; i++)
          //{
          //  strArgs.AppendFormat("%s%s %s", Get(static_cast<UVShader::InputModifier>(func.pArguments[i].eModifier)),
          //    func.pArguments[i].ptkType->ToString(strType).CStr(),
          //    func.pArguments[i].ptkName->ToString(strName).CStr());

          //  if(func.pArguments[i].szSemantic) {
          //    strArgs.AppendFormat(" : %s", func.pArguments[i].szSemantic);
          //  }
          //  strArgs.Append(",");
          //}

          //if(strArgs.IsNotEmpty()) {
          //  strArgs.Remove(strArgs.GetLength() - 1, 1);
          //}
          if(s.func.arguments_glob.IsNode()) {
            DumpBlock(&parser, s.func.arguments_glob.pNode, 0, 0, &strArgs);
          }
          else if(s.func.arguments_glob.IsToken()) {
            s.func.arguments_glob.pTokn->ToString(strArgs);
          }

          WriteFormat("%s%s %s(%s)", Get(func.eStorageClass), func.szReturnType, func.szName, strArgs);

          if(func.szSemantic) {
            WriteFormat(" : %s\n", func.szSemantic);
          }
          else {
            WriteFormat("\n");
          }

          if(s.sRoot.ptr)
          {
            clStringA str;
            DumpBlock(&parser, s.sRoot.pNode, 0, 0, &str);
            WriteFormat("%s", str); // 缩进两个空格, 防止str中含有恰好符合格式化的字符序列
          }
          WriteFormat("\n");
        }
        break;
        case UVShader::CodeParser::StatementType_Struct:
        case UVShader::CodeParser::StatementType_Signatures:
        {
          const auto& stru = s;
          clStringA str;
          WriteFormat("struct %s\n", s.stru.szName);
          DumpBlock(&parser, stru.sRoot.pNode, 0, 0, &str);
          WriteFormat(str);
        }
        break;

        case UVShader::CodeParser::StatementType_Typedef:
        {
          clStringA str;
          DumpBlock(&parser, s.sRoot.pNode, 0, 1, &str);
          WriteFormat(str);
        }
        break;
        default:
          CLBREAK;
          break;
        }
      }
    }

  } // namespace ShaderT
} // namespace GrapX

