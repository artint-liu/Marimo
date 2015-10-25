#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "ArithmeticExpression.h"
#include "ExpressionParser.h"

#include "clTextLines.h"
#include "../User/DataPoolErrorMsg.h"

// TODO:
// 1.float3(0) => float3(0,0,0)
// 2.返回值未完全初始化
// 3.code block 表达式中如果中间语句缺少分号不报错


//////////////////////////////////////////////////////////////////////////
//
// 函数定义
//
//例：float4 VertexShader_Tutorial_1(float4 inPos : POSITION ) : POSITION
//
//  A function declaration contains:
//
//  A return type 
//  A function name 
//  An argument list (optional) 
//  An output semantic (optional) 
//  An annotation (optional) 
//
//////////////////////////////////////////////////////////////////////////

// 步进符号指针，但是不期望遇到结尾指针
#define INC_BUT_NOT_END(_P, _END) \
  if(++_P >= _END) {  \
    return FALSE;     \
  }

#define OUT_OF_SCOPE(s) (s == (clsize)-1)

//static clsize s_nMultiByteOperatorLen = 0; // 最大长度


namespace UVShader
{

  //////////////////////////////////////////////////////////////////////////


  CodeParser::CodeParser()
    : m_pSubParser(NULL)
    , m_dwState(0)
  {
    SetIteratorCallBack(CodeParser::IteratorProc, 0);
    InitPacks();
  }

  CodeParser::~CodeParser()
  {
    SAFE_DELETE(m_pSubParser);
    SAFE_DELETE(m_pMsg);
  }

  void CodeParser::InitPacks()
  {
    //
    // 所有pack类型，都存一个空值，避免记录0索引与NULL指针混淆的问题
    //
    SYNTAXNODE node = {NULL};
    m_aSyntaxNodePack.clear();
    m_aSyntaxNodePack.push_back(node);

    STRUCT_MEMBER member = {NULL};
    m_aMembersPack.clear();
    m_aMembersPack.push_back(member);

    FUNCTION_ARGUMENT argument = {InputModifier_in};
    m_aArgumentsPack.clear();
    m_aArgumentsPack.push_back(argument);

    STATEMENT stat = {StatementType_Empty};
    m_aSubStatements.push_back(stat);
  }

  void CodeParser::Cleanup()
  {
    m_aTokens.clear();
    m_aStatements.clear();
    m_Macros.clear();
    SAFE_DELETE(m_pSubParser);
    InitPacks();
  }

  b32 CodeParser::Attach(const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename)
  {
    Cleanup();
    if(TEST_FLAG_NOT(dwFlags, AttachFlag_NotLoadMessage))
    {
      if( ! m_pMsg) {
        m_pMsg = new ErrorMessage();
        m_pMsg->SetCurrentFilenameW(szFilename);
        m_pMsg->LoadErrorMessageW(L"uvsmsg.txt");
        m_pMsg->SetMessageSign('C');
      }
      m_pMsg->GenerateCurLines(szExpression, nSize);
    }
    return SmartStreamA::Initialize(szExpression, nSize);
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  u32 CALLBACK CodeParser::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
  {
    GXBOOL bENotation = FALSE;

    if(it.marker[0] == '#')
    {
      CodeParser* pThis = (CodeParser*)it.pContainer;

      // 测试是否已经在预处理中
      if(TEST_FLAG(pThis->m_dwState, State_InPreprocess)) {
        return 0;
      }
      SET_FLAG(pThis->m_dwState, State_InPreprocess);

      if( ! SmartStreamUtility::IsHeadOfLine(it.pContainer, it.marker)) {
        ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动;
      }

      RTPPCONTEXT ctx;
      ctx.iter_next = it;
      SmartStreamUtility::ExtendToNewLine(ctx.iter_next, 1, remain, 0x0001);
      
      // ppend 与 iter_next.marker 之间可能存在空白，大量注释等信息，为了
      // 减少预处理解析的工作量，这里预先存好 ppend 再步进 iter_next
      ctx.ppend = ctx.iter_next.end();
      ctx.stream_end = pThis->GetStreamPtr() + pThis->GetStreamCount();
      ++ctx.iter_next;

      if(++it >= ctx.iter_next) { // 只有一个'#', 直接跳过
        RESET_FLAG(pThis->m_dwState, State_InPreprocess);
        return 0 ;
      }

      // define
      // if, ifndef, ifdef, elif, else, endif, undef
      // include
      // line, file
      // program
      // error

      if(it == "else")
      {
        it.marker = pThis->Macro_SkipConditionalBlock(ctx.iter_next.marker, ctx.stream_end);
        it.length = 0;
      }
      else if(it == "endif")
      {
        it = ctx.iter_next;
      }
      else if(it == "error")
      {
        clStringW str(it.marker + it.length, ctx.ppend - (it.marker + it.length));
        pThis->OutputErrorW(it.marker, E1189_用户定义错误, str);
      }
      else
      {
        auto next_marker = pThis->ParseMacro(ctx, it.marker, ctx.ppend);
        if(next_marker == ctx.iter_next.marker) {
          it = ctx.iter_next;
        }
        else if(next_marker != ctx.stream_end)
        {
          it.marker = next_marker;
          it.length = 0;
        }
        ASSERT(it.marker >= ctx.iter_next.marker);
      }//*/


      // 如果在预处理中步进iterator则会将#当作一般符号处理
      // 这里判断如果下一个token仍然是#则置为0，这样会重新激活迭代器处理当前这个预处理
      if(it == '#') {
        it.length = 0;
      }

      RESET_FLAG(pThis->m_dwState, State_InPreprocess);
    }
    else
    {
      ArithmeticExpression::IteratorProc(it, remain, lParam);
    }
    ASSERT((int)remain >= 0);
    return 0;
  }

  clsize CodeParser::GenerateTokens()
  {
    auto stream_end = end();
    ASSERT(m_aTokens.empty()); // 调用者负责清空

    m_aTokens.reserve(EstimateForTokensCount());
    TOKEN token;
    TOKEN l_token; // 用来在迭代器中储存符号优先级的信息
    clStringA strMacro;
    
    // 只是清理
    l_token.ClearMarker();
    l_token.precedence = 0;
    l_token.unary      = 0;
    SetTriggerCallBack(MultiByteOperatorProc, (u32_ptr)&l_token);


    PairStack sStack[s_nPairMark];

    int EOE = 0; // End Of Expression

    for(auto it = begin(); it != stream_end; ++it)
    {
      // 上一个预处理结束后，后面的语句长度设置为0可以回到主循环步进
      // 这样做是为了避免如果下一条语句也是预处理指令，不会在处理回调中发生递归调用
      if(it.length == 0) {
        continue;
      }


      const int c_size = (int)m_aTokens.size();
      token.Set(it);
      token.scope = -1;
      token.semi_scope = -1;

      ASSERT(l_token.marker.marker == NULL ||
        l_token.marker.marker == it.marker); // 遍历时一定这个要保持一致

      token.SetArithOperatorInfo(l_token);

      // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
      if((it == '-' || it == '+') && ! m_aTokens.empty())
      {        
        const auto& l_back = m_aTokens.back();

        // 一元操作符，+/-就不转换为正负号
        // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
        if(l_back.precedence != 0 && l_back != ')' && l_back != ']' && ( ! l_back.unary)) {
          const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
          token.SetArithOperatorInfo(p);
        }
      }

      
      // 只是清理
      l_token.ClearMarker();
      l_token.ClearArithOperatorInfo();


      // 符号配对处理
      MarryBracket(sStack, token, EOE);
      
      if(token.precedence == 0 && OnToken(token, strMacro)) {
        token.ClearMarker();
      }

      // 带有参数的宏展开
      if(token == ')' && strMacro.IsNotEmpty() && Macro_ExpandMacroInvoke(strMacro, token)) {
        token.ClearMarker();
      }

      // 可能被宏展开后清除
      if(token.marker.marker) {
        m_aTokens.push_back(token);
      }

      if(it == ';') {
        ASSERT(EOE < (int)m_aTokens.size());
        for(auto it = m_aTokens.begin() + EOE; it != m_aTokens.end(); ++it)
        {
          it->semi_scope = c_size;
        }
        EOE = c_size + 1;
      }
    }

    for(int i = 0; i < s_nPairMark; ++i)
    {
      PairStack& s = sStack[i];
      if( ! s.empty()) {
        // ERROR: 不匹配
        ERROR_MSG__MISSING_CLOSEDBRACKET;
      }
    }

    SetTriggerCallBack(MultiByteOperatorProc, NULL);
    return m_aTokens.size();
  }

  GXBOOL CodeParser::OnToken(const TOKEN& token, clStringA& strMacro)
  {
    clStringA strTokenName = token.ToString();
    //{
    //  auto it = m_Macros.find(strTokenName);
    //  if(it == m_MacrosSet.end()) {
    //    return FALSE;
    //  }
    //}

    //auto it = m_Macros.find(strTokenName);
    //ASSERT(it != m_Macros.end());

    //auto iter_next = it;
    //++iter_next;
    //strTokenName.Append("@A");

    //if(iter_next == m_Macros.end() || ! iter_next->first.BeginsWith(strTokenName)) {
    //  m_aTokens.insert(m_aTokens.end(), it->second.aTokens.begin(), it->second.aTokens.end());
    //  return TRUE;
    //}


    auto it = m_Macros.find(strTokenName);
    if(it == m_Macros.end()) {
      return FALSE;
    }

    if(it->second.aFormalParams.empty()) {
      m_aTokens.insert(m_aTokens.end(), it->second.aTokens.begin(), it->second.aTokens.end());
      return TRUE;
    }

    strMacro = strTokenName;
    return FALSE;
  }


  GXBOOL CodeParser::Parse()
  {
    RTSCOPE scope(0, m_aTokens.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatement(RTSCOPE* pScope)
  {    
    return (pScope->begin < pScope->end) && (
      ParseStatementAs_Definition(pScope) ||
      ParseStatementAs_Function(pScope) || ParseStatementAs_Struct(pScope));
  }

  GXBOOL CodeParser::ParseStatementAs_Definition(RTSCOPE* pScope)
  {
    TOKEN* p = &m_aTokens[pScope->begin];

    if((RTSCOPE::TYPE)p->semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)p->semi_scope > pScope->end) {
      return FALSE;
    }
    const TOKEN* pEnd = &m_aTokens.front() + p->semi_scope;

    STATEMENT stat = {StatementType_Definition};
    RTSCOPE::TYPE definition_end = p->semi_scope;

    // 修饰
    if(*p == "const") {
      stat.defn.modifier = UniformModifier_const;
      INC_BUT_NOT_END(p, pEnd);
    }
    else {
      stat.defn.modifier = UniformModifier_empty;
    }

    stat.defn.szType = GetUniqueString(p);
    //if(p + 1 < pEnd) {
    //  stat.defn.szName = GetUniqueString(p + 1);
    //}

    if( ! ParseExpression(RTSCOPE(pScope->begin, definition_end), &stat.defn.sRoot))
    {
      ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(definition_end));
      return FALSE;
    }
    //*
    //
    // 将类型定义中的逗号表达式展开为独立的类型定义
    // 如 float a, b, c; 改为
    // float a; float b; float c;
    //
    SYNTAXNODE* pNode = &m_aSyntaxNodePack[stat.defn.sRoot.nNodeIndex];
    cllist<SYNTAXNODE*> sDefinitionList;
    SYNTAXNODE::RecursiveNode(this, pNode, [this, &sDefinitionList](SYNTAXNODE* pNode) -> GXBOOL {
      if( ! (pNode->mode == CodeParser::SYNTAXNODE::MODE_Definition || 
        pNode->mode == CodeParser::SYNTAXNODE::MODE_DefinitionConst || (pNode->pOpcode && (*(pNode->pOpcode)) == ',')))
      {
        return FALSE;
      }
      sDefinitionList.push_back(pNode);
      return TRUE;
    });

    if(sDefinitionList.size() == 1) {
      m_aStatements.push_back(stat);
    }
    else {
      ASSERT( ! sDefinitionList.empty());
      // 这里list应该是如下形式
      // (define) [type] [node1]
      // (node1) [node2] [var define node N]
      // (node2) [node3] [var define node (N-1)]
      // ...
      // (nodeN) [var define node 1] [var define node 2]

      auto& front = sDefinitionList.front();
      auto* pNodeFrontPtr = &m_aSyntaxNodePack.front();

      ASSERT(front->mode == SYNTAXNODE::MODE_Definition || front->mode == SYNTAXNODE::MODE_DefinitionConst);

      front->Operand[1].ptr = sDefinitionList.back()->Operand[0].ptr;
      front->SetOperandType(1, sDefinitionList.back()->GetOperandType(0));
      stat.defn.sRoot.nNodeIndex = front - &m_aSyntaxNodePack.front();
      m_aStatements.push_back(stat);

      auto it = sDefinitionList.end();
      ASSERT(front->GetOperandType(0) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN);
      for(--it; it != sDefinitionList.begin(); --it)
      {
        auto& SyntaxNode = **it;

        // 逗号并列式改为独立类型定义式
        SyntaxNode.mode = front->mode;
        SyntaxNode.pOpcode = NULL;
        SyntaxNode.Operand[0].ptr = front->Operand[0].ptr; // type
        SyntaxNode.SetOperandType(0, SYNTAXNODE::FLAG_OPERAND_IS_TOKEN); // front->GetOperandType(0)

        // 加入列表
        stat.defn.sRoot.nNodeIndex = *it - pNodeFrontPtr;
        m_aStatements.push_back(stat);
      }
    }//*/
    ASSERT(pEnd->semi_scope == definition_end && *pEnd == ';');
    pScope->begin = definition_end + 1;
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Function(RTSCOPE* pScope)
  {
    // 函数语法
    //[StorageClass] Return_Value Name ( [ArgumentList] ) [: Semantic]
    //{
    //  [StatementBlock]
    //};

    TOKEN* p = &m_aTokens[pScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pScope->end;

    STATEMENT stat = {StatementType_Empty};

    // ## [StorageClass]
    if(*p == "inline") {
      stat.func.eStorageClass = StorageClass_inline;
      INC_BUT_NOT_END(p, pEnd); // ERROR: inline 应该修饰函数
    }
    else {
      stat.func.eStorageClass = StorageClass_empty;
    }

    // 检测函数格式特征
    if(p[2] != "(" || p[2].scope < 0) {
      return FALSE;
    }

    // #
    // # Return_Value
    // #
    stat.func.szReturnType = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd); // ERROR: 缺少“；”

    // #
    // # Name
    // #
    stat.func.szName = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd); // ERROR: 缺少“；”

    ASSERT(*p == '('); // 由前面的特征检查保证
    ASSERT(p->scope >= 0);  // 由 GenerateTokens 函数保证

    // #
    // # ( [ArgumentList] )
    // #
    if(p[0].scope != p[1].scope + 1) // 有参数: 两个括号不相邻
    {
      RTSCOPE ArgScope(m_aTokens[p->scope].scope + 1, p->scope);
      ParseFunctionArguments(&stat, &ArgScope);
    }
    p = &m_aTokens[p->scope];
    ASSERT(*p == ')');

    ++p;

    // #
    // # [: Semantic]
    // #
    if(*p == ":") {
      stat.func.szSemantic = m_Strings.add((p++)->ToString());
    }

    if(*p == ";") { // 函数声明
      stat.type = StatementType_FunctionDecl;
    }
    else if(*p == "{") { // 函数体
      RTSCOPE func_statement_block(m_aTokens[p->scope].scope, p->scope);

      stat.type = StatementType_Function;
      p = &m_aTokens[p->scope];
      ++p;

      if(func_statement_block.IsValid())
      {
        STATEMENT sub_stat = {StatementType_Expression};
        if(ParseStatementAs_Expression(&sub_stat, &func_statement_block/*, FALSE*/))
        {
          stat.func.pExpression = (STATEMENT*)m_aSubStatements.size();
          m_aSubStatements.push_back(sub_stat);
        }
      }
    }
    else {
      // ERROR: 声明看起来是一个函数，但是既不是声明也不是实现。
      return FALSE;
    }


    m_aStatements.push_back(stat);
    pScope->begin = p - &m_aTokens.front();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Struct( RTSCOPE* pScope )
  {
    TOKEN* p = &m_aTokens[pScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pScope->end;

    if(*p != "struct") {
      return FALSE;
    }

    STATEMENT stat = {StatementType_Empty};
    INC_BUT_NOT_END(p, pEnd);

    stat.stru.szName = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd);

    if(*p != "{") {
      // ERROR: 错误的结构体定义
      return FALSE;
    }

    RTSCOPE StruScope(m_aTokens[p->scope].scope + 1, p->scope);
    // 保证分析函数的域
    ASSERT(m_aTokens[StruScope.begin - 1] == "{" && m_aTokens[StruScope.end] == "}"); 

    pScope->begin = StruScope.end + 1;
    if(m_aTokens[pScope->begin] != ";") {
      // ERROR: 缺少“；”
      return FALSE;
    }
    ++pScope->begin;

    // #
    // # 解析成员变量
    // #
    ParseStructMembers(&stat, &StruScope);

    m_aStatements.push_back(stat);
    return TRUE;
  }

  GXBOOL CodeParser::ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

    TOKEN* p = &m_aTokens[pArgScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pArgScope->end;
    ASSERT(*pEnd == ")"); // 函数参数列表的结尾必须是闭圆括号

    if(pArgScope->begin >= pArgScope->end) {
      // ERROR: 函数参数声明不正确
      return FALSE;
    }

    ArgumentsArray aArgs;

    while(1)
    {
      FUNCTION_ARGUMENT arg = {InputModifier_in};

      if(*p == "in") {
        arg.eModifier = InputModifier_in;
      }
      else if(*p == "out") {
        arg.eModifier = InputModifier_out;
      }
      else if(*p == "inout") {
        arg.eModifier = InputModifier_inout;
      }
      else if(*p == "uniform") {
        arg.eModifier = InputModifier_uniform;
      }
      else {
        goto NOT_INC_P;
      }

      INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确

NOT_INC_P:
      arg.szType = GetUniqueString(p);

      INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确

      arg.szName = GetUniqueString(p);

      if(++p >= pEnd) {
        aArgs.push_back(arg);
        break;
      }

      if(*p == ",") {
        aArgs.push_back(arg);
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        continue;
      }
      else if(*p == ":") {
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        arg.szSemantic = GetUniqueString(p);
      }
      else {
        // ERROR: 函数参数声明不正确
        return FALSE;
      }
    }

    ASSERT( ! aArgs.empty());
    pStat->func.pArguments = (FUNCTION_ARGUMENT*)m_aArgumentsPack.size();
    pStat->func.nNumOfArguments = aArgs.size();
    m_aArgumentsPack.insert(m_aArgumentsPack.end(), aArgs.begin(), aArgs.end());
    return TRUE;
  }

  void CodeParser::RelocalePointer()
  {
    RelocaleStatements(m_aStatements);
    RelocaleStatements(m_aSubStatements);
  }

  void CodeParser::RelocaleSyntaxPtr(SYNTAXNODE* pNode)
  {
    if(pNode->OperandA_IsNodeIndex() && pNode->Operand[0].pNode) {
      IndexToPtr(pNode->Operand[0].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[0].pNode);
      pNode->SetOperandType(0, SYNTAXNODE::FLAG_OPERAND_IS_NODE);
    }

    if(pNode->OperandB_IsNodeIndex() && pNode->Operand[1].pNode) {
      IndexToPtr(pNode->Operand[1].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[1].pNode);
      pNode->SetOperandType(1, SYNTAXNODE::FLAG_OPERAND_IS_NODE);
    }
  }

  GXBOOL CodeParser::ParseStructMember(STATEMENT* pStat, STRUCT_MEMBER& member, TOKEN**pp, const TOKEN* pMemberEnd)
  {
    TOKEN*& p = *pp;

    member.szName = GetUniqueString(p);
    p++;
    //INC_BUT_NOT_END(p, pMemberEnd); // ERROR: 结构体成员声明不正确

    if(p == pMemberEnd || *p == ',') {
      if(pStat->type != StatementType_Empty && pStat->type != StatementType_Struct) {
        // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
        return FALSE;
      }
      pStat->type = StatementType_Struct;
      //++p;
    }
    else if(*p == ':') {
      if(pStat->type != StatementType_Empty && pStat->type != StatementType_Struct) {
        // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
        return FALSE;
      }
      pStat->type = StatementType_Signatures;
      INC_BUT_NOT_END(p, pMemberEnd);
      member.szSignature = GetUniqueString(p);

      // TODO: 检查这个是Signature

      INC_BUT_NOT_END(p, pMemberEnd);

      ASSERT(*p == ';' || *p == ','); // 如果发生这个断言错误，检查如何处理这个编译错误
      //if(*p != ";") {
      //  ERROR_MSG__MISSING_SEMICOLON; // ERROR: 缺少“；”
      //  return FALSE;
      //}
      //++p;
    }
    ASSERT(p <= pMemberEnd);
    return TRUE;
  }

  GXBOOL CodeParser::ParseStructMembers( STATEMENT* pStat, RTSCOPE* pStruScope )
  {
    // 作为结构体成员
    // Type[RxC] MemberName; 
    // 作为Shader标记
    // Type[RxC] MemberName : ShaderFunction; 
    // 这个结构体成员必须一致，要么全是普通成员变量，要么全是Shader标记

    TOKEN* p = &m_aTokens[pStruScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pStruScope->end;
    MemberArray aMembers;

    while(p < pEnd)
    {
      STRUCT_MEMBER member = {NULL};

      if((RTSCOPE::TYPE)p->semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)p->semi_scope >= pStruScope->end) {
        ERROR_MSG__MISSING_SEMICOLON(*p);
        return FALSE;
      }

      const TOKEN* pMemberEnd = &m_aTokens.front() + p->semi_scope;
      member.szType = GetUniqueString(p);
      INC_BUT_NOT_END(p, pMemberEnd); // ERROR: 结构体成员声明不正确

      if( ! ParseStructMember(pStat, member, &p, pMemberEnd)) {
        return FALSE;
      }

      aMembers.push_back(member);

      while(*p == ',' && p < pMemberEnd) {
        INC_BUT_NOT_END(p, pMemberEnd); // 语法错误
        if( ! ParseStructMember(pStat, member, &p, pEnd)) {
          return FALSE;
        }
        aMembers.push_back(member);
      }

      ASSERT(*p == ';');
      p++;

      //else if(*p == ',') {
      //  INC_BUT_NOT_END(p, pEnd); // 语法错误
      //  if( ! ParseStructMember(pStat, member, &p, pEnd)) {
      //    return FALSE;
      //  }
      //  CLBREAK;
      //}
      //else {
      //  // ERROR: 缺少“；”
      //  ERROR_MSG__MISSING_SEMICOLON;
      //  return FALSE;
      //}

      //aMembers.push_back(member);
    }

    pStat->stru.pMembers = (STRUCT_MEMBER*)m_aMembersPack.size();
    pStat->stru.nNumOfMembers = aMembers.size();
    m_aMembersPack.insert(m_aMembersPack.end(), aMembers.begin(), aMembers.end());
    return TRUE;
  }

  GXLPCSTR CodeParser::GetUniqueString( const TOKEN* pSym )
  {
    return m_Strings.add(pSym->ToString());
  }

  const CodeParser::TYPE* CodeParser::ParseType(const TOKEN* pSym)
  {
    TYPE sType = {NULL, 1, 1};

    // 对于内置类型，要解析出 Type[RxC] 这种格式
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i)
    {
      const INTRINSIC_TYPE& t = s_aIntrinsicType[i];
      if(pSym->marker.BeginsWith(t.name, t.name_len)) {
        const auto* pElement = pSym->marker.marker + t.name_len;
        const int   remain   = pSym->marker.length - (int)t.name_len;
        sType.name = t.name;

        // [(1..4)[x(1..4)]]
        if(remain == 0) {
          ;
        }
        else if(remain == 1 && *pElement >= '1' && *pElement <= '4') { // TypeR 格式
          sType.R = *pElement - '0';
          ASSERT(sType.R >= 1 && sType.R <= 4);
        }
        else if(remain == 3 && *pElement >= '1' && *pElement <= '4' && // TypeRxC 格式
          pElement[1] == 'x' && pElement[2] >= '1' && pElement[2] <= '4')
        {
          sType.R = pElement[0] - '0';
          sType.C = pElement[2] - '0';
          ASSERT(sType.R >= 1 && sType.R <= 4);
          ASSERT(sType.C >= 1 && sType.C <= 4);
        }
        else {
          break;
        }

        return &(*m_TypeSet.insert(sType).first);
      }
    }
    
    // TODO: 查找用户定义类型
    // 1.typdef 定义
    // 1.struct 定义

    return NULL;
  }

  GXBOOL CodeParser::ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope/*, GXBOOL bDbgRelocale */)
  {
    m_nDbgNumOfExpressionParse = 0;
    m_aDbgExpressionOperStack.clear();

    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    STATEMENT& stat = *pStat;
    
    GXBOOL bret = ParseExpression(*pScope, &stat.expr.sRoot);
    //TRACE("m_nDbgNumOfExpressionParse=%d\n", m_nDbgNumOfExpressionParse);

//#ifdef _DEBUG
//    // 这个测试会重定位指针，所以仅作为一次性调用，之后m_aSyntaxNodePack不能再添加新的数据了
//    if( ! bret)
//    {
//      TRACE("编译错误\n");
//      m_aDbgExpressionOperStack.clear();
//    }
//    else if(bDbgRelocale && TryGetNodeType(&stat.expr.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX) {
//      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
//      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
//      DbgDumpSyntaxTree(stat.expr.sRoot.pNode, 0);
//    }
//#endif // #ifdef _DEBUG
    //m_aStatements.push_back(stat);
    return bret;
  }

  void CodeParser::DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr)
  {
    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].pSym) {
        const auto flag = TryGetNodeType(&pNode->Operand[i]);
        if(flag == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          str[i] = pNode->Operand[i].pSym->ToString();
        }
        else if(flag == SYNTAXNODE::FLAG_OPERAND_IS_NODE) {
          DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
        }
        else if(flag == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX) {
          DbgDumpSyntaxTree(pArray, &m_aSyntaxNodePack[pNode->Operand[i].nNodeIndex], pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
        }
        else {
          CLBREAK;
        }
      }
      else {
        str[i].Clear();
      }
    }

    clStringA strCommand;
    strCommand.Format("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
    if(pArray) {
      pArray->push_back(strCommand);
    }
    TRACE("%s\n", strCommand);

    clStringA strOut;
    switch(pNode->mode)
    {
    case SYNTAXNODE::MODE_FunctionCall: // 函数调用
      strOut.Format("%s(%s)", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_ArrayIndex:
      strOut.Format("%s[%s]", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_DefinitionConst:
      strOut.Format("const %s %s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Definition:
      strOut.Format("%s %s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_If:
      strOut.Format("if(%s){%s;}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_Else:
      strOut.Format("%s else {%s;}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_ElseIf:
      strOut.Format("%s else %s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_While:
      strOut.Format("%s(%s)", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_For:
      strOut.Format("for(%s){%s}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_ForInit:
    case SYNTAXNODE::MODE_Flow_ForRunning:
      strOut.Format("%s;%s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Return:
      ASSERT(str[0] == "return");
      strOut.Format("return %s", str[1]);
      break;

    case SYNTAXNODE::MODE_Block:
      strOut.Format("{%s}", str[0]);
      break;

    case SYNTAXNODE::MODE_Chain:
      if(str[1].IsEmpty()) {
        strOut.Format("%s", str[0]);
      }
      else {
        strOut.Format("%s;%s", str[0], str[1]);
      }
      break;

    case SYNTAXNODE::MODE_Opcode:
      if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
        strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->ToString(), str[1]);
      }
      else {
        strOut.Format("%s%s%s", str[0], pNode->pOpcode->ToString(), str[1]);
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

  //////////////////////////////////////////////////////////////////////////

  //GXBOOL CodeParser::ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end)
  //{
  //  RTSCOPE scope(begin, end);
  //  return ParseExpression(&scope, pUnion);
  //}
 
  GXBOOL CodeParser::ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    GXBOOL bret = TRUE;
    if(parse_end == RTSCOPE::npos) {
      return FALSE;
    }
    ASSERT(parse_end <= scope.end);
    if(parse_end < scope.end)
    {
      SYNTAXNODE::UN A, B = {0};
      A = *pUnion;

      bret = ParseExpression(RTSCOPE(parse_end + 1, scope.end), &B) &&
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, &A, &B);
    }
    return bret ? parse_end != RTSCOPE::npos : FALSE;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL CodeParser::TryKeywords(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end)
  {
    // 如果是关键字，返回true，否则返回false
    // 解析成功parse_end返回表达式最后一个token的索引，parse_end是这个关键字表达式之内的！
    // 解析失败parse_end返回RTSCOPE::npos

    const auto& front = m_aTokens[scope.begin];
    auto& pend = *parse_end;
    GXBOOL bret = TRUE;
    SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    ASSERT(pend == RTSCOPE::npos); // 强调调用者要初始化这个变量

    if(front == "else") {
      // ERROR: "else" 不能独立使用
    }
    else if(front == "for") {
      pend = ParseFlowFor(scope, pUnion);
    }
    else if(front == "if") {
      pend = ParseFlowIf(scope, pUnion, FALSE);
    }
    else if(front == "while") {
      pend = ParseFlowWhile(scope, pUnion);
    }
    else if(front == "do") {
      pend = ParseFlowDoWhile(scope, pUnion);
    }
    else if(front == "struct") {
      pend = ParseStructDefine(scope, pUnion);
    }
    else if(front == "break") {
      eMode = SYNTAXNODE::MODE_Flow_Break;
    }
    else if(front == "continue") {
      eMode = SYNTAXNODE::MODE_Flow_Continue;
    }
    else if(front == "discard") {
      eMode = SYNTAXNODE::MODE_Flow_Discard;
    }
    else if(front == "return")
    {
      eMode = SYNTAXNODE::MODE_Return;
    }
    else {
      bret = FALSE;
    }

    while(eMode == SYNTAXNODE::MODE_Flow_Break || eMode == SYNTAXNODE::MODE_Flow_Continue ||
      eMode == SYNTAXNODE::MODE_Flow_Discard || eMode == SYNTAXNODE::MODE_Return)
    {
      SYNTAXNODE::UN A = {0}, B = {0};

      A.pSym = &front;
      pend = scope.begin + 1;

      if(front.semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)front.semi_scope > scope.end) {
        // ERROR: 缺少;
        ERROR_MSG__MISSING_SEMICOLON(front);
        break;
      }

      if(eMode == SYNTAXNODE::MODE_Return) {
        bret = ParseArithmeticExpression(scope.begin + 1, front.semi_scope, &B);
        MakeSyntaxNode(pUnion, eMode, NULL, &A, &B);
        pend = front.semi_scope;
      }
      else {
        MakeSyntaxNode(pUnion, eMode, NULL, &A, NULL);
      }
      break;
    }

    ASSERT(( ! bret && pend == RTSCOPE::npos) || bret);
    ASSERT(pend == RTSCOPE::npos || (pend > scope.begin && pend <= scope.end));
    return bret;
  }

  //GXBOOL CodeParser::TryBlock(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end)
  //{
  //  const auto& first = m_aTokens[scope.begin];
  //  auto& pend = *parse_end;
  //  GXBOOL bret = TRUE;

  //  ASSERT(pend == RTSCOPE::npos); // 强调调用者要初始化这个变量

  //  if(first == '{')
  //  {
  //    if(first.scope > scope.end) {
  //      // ERROR: 没有正确的'}'来匹配
  //    }
  //    else {
  //      ParseExpression(pUnion, scope.begin + 1, first.scope);
  //    }
  //  }
  //  else {
  //    ASSERT(first.semi_scope < scope.end);
  //  }

  //  return bret;
  //}

  GXBOOL CodeParser::ParseExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    ASSERT(scope.end == m_aTokens.size() || m_aTokens[scope.end] == ';' || 
      m_aTokens[scope.end] == '}');


    const GXINT_PTR count = scope.end - scope.begin;
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    RTSCOPE::TYPE parse_end = RTSCOPE::npos;

    if(count <= 1) {
      if(count < 0) {
        CLBREAK;
        return FALSE;
      }
      else if(count == 1) {
        pUnion->pSym = &m_aTokens[scope.begin];
      }
      
      return TRUE;  // count == 0 / count == 1
    }

    const auto& front = m_aTokens[scope.begin];

    if(front == '{') // 代码块
    {
      if((clsize)front.scope > scope.end) {
        // ERROR: 没有正确的'}'来匹配
      }

      ASSERT((RTSCOPE::TYPE)front.scope <= scope.end);  // scopeB.begin 最多仅仅比scope.end大1

      //////////////////////////////////////////////////////////////////////////
      //
      // ParseRemainStatement 在处理 statement block 剩余部分时有微妙的不同：
      // parse_end 如果严格限定等于 scope.end，pUnion不做任何修改，这种用法通常在
      // if/else if/else/for/while关键字的代码块解析中使用
      //
      // parse_end 如果小于 scope.end, pUnion会被修改为MODE_Chain，需要注意：
      // * parse_end 如果仅仅比 scope.end 小 1，则意味着生成的MODE_Chain是chain的结尾。
      // * 这种用法在 statement block 中用来标记chain的结尾
      //
      //////////////////////////////////////////////////////////////////////////
      bret = ParseExpression(RTSCOPE(scope.begin + 1, front.scope), pUnion);
      bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Block, pUnion, NULL);
      return ParseRemainStatement(front.scope, scope, pUnion);
    }
    else if(TryKeywords(scope, pUnion, &parse_end))
    {
      if(parse_end == RTSCOPE::npos) {
        return FALSE; // 解析错误, 直接返回
      }
      // 解析剩余部分
      return ParseRemainStatement(parse_end, scope, pUnion);
    }
    else if(front.semi_scope == RTSCOPE::npos) {
      ERROR_MSG__MISSING_SEMICOLON(front);
      return FALSE;
    }
    else if((clsize)front.semi_scope < scope.end)
    {
      ASSERT(m_aTokens[front.semi_scope] == ';'); // 目前进入这个循环的只可能是遇到分号

      RTSCOPE scopeA(scope.begin, front.semi_scope);

      // 跳过只是一个分号的情况
      if(scopeA.begin == scopeA.end) {
        return ParseExpression(RTSCOPE(scopeA.end + 1, scope.end), pUnion);
      }
      else
      {
        //RTSCOPE scopeB(front.semi_scope + 1, scope.end);
        ASSERT((RTSCOPE::TYPE)front.semi_scope + 1 <= scope.end); // 感觉有可能begin>end，这里如果遇到就改成if(scopeB.begin >= scopeB.end)

        bret = ParseExpression(scopeA, pUnion);
        return ParseRemainStatement(front.semi_scope, scope, pUnion);
      }
      //bret = bret && ParseExpression(scopeB, &B);
      //bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, &A, &B);
    }

    ParseArithmeticExpression(scope, pUnion);
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////



  //////////////////////////////////////////////////////////////////////////


  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "if");

    RTSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    bret = bret && ParseArithmeticExpression(sConditional, &A);



    sBlock.begin = sConditional.end + 1;
    sBlock.end = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[sBlock.begin];
    //SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    {
      //eMode = SYNTAXNODE::MODE_Flow_If;
      bret = sBlock.end != RTSCOPE::npos;
    }
    else
    {
      if(block_begin == '{')
      {
        sBlock.end = block_begin.scope;
        //sBlock.begin ++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: if 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(sBlock, &B);
    }

    const SYNTAXNODE::MODE eMode = TryGetNode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_If, &A, &B);

    auto result = sBlock.end;


    if(bret && (scope.end - sBlock.end) > 1 && m_aTokens[sBlock.end + 1] == "else")
    {
      auto nNextBegin = sBlock.end + 1;

      // 只处理 else if/else 两种情况
      A = *pUnion;
      ++nNextBegin;
      if(nNextBegin >= scope.end) {
        // ERROR: else 语法错误
        return RTSCOPE::npos;
      }

      SYNTAXNODE::MODE eNextMode = SYNTAXNODE::MODE_Flow_Else;

      if(m_aTokens[nNextBegin] == "if")
      {
        eNextMode = SYNTAXNODE::MODE_Flow_ElseIf;
        result = ParseFlowIf(RTSCOPE(nNextBegin, scope.end), &B, TRUE);
      }
      else
      {
        auto& else_begin = m_aTokens[nNextBegin];
        result = RTSCOPE::npos;
        if(TryKeywords(RTSCOPE(nNextBegin, scope.end), &B, &result))
        {
          ;
        }
        else
        {
          result = else_begin == '{' ? else_begin.scope : else_begin.semi_scope;

          if(result == RTSCOPE::npos || result > scope.end) {
            // ERROR: else 语法错误
            return RTSCOPE::npos;
          }

          bret = ParseExpression(RTSCOPE(nNextBegin, result), &B);
        }
      }

      // else if/else 处理
      if(eNextMode == SYNTAXNODE::MODE_Flow_Else)
      {
        const SYNTAXNODE::MODE eMode = TryGetNode(&B);
        if(eMode != SYNTAXNODE::MODE_Block) {
          if(eMode != SYNTAXNODE::MODE_Chain) {
            bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
          }
          bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
        }
      }
           

      bret = bret && MakeSyntaxNode(pUnion, eNextMode, &A, &B);

    //  if(eNextMode != SYNTAXNODE::MODE_Chain) {
    //    if(eNextMode == SYNTAXNODE::MODE_Flow_Else) {
    //      DbgDumpScope("else", RTSCOPE(0,0), RTSCOPE(nNextBegin, scope.end));
    //    }
    //    DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
    //  }
    }
    //else {
    //  DbgDumpScope("if", sConditional, sBlock);
    //}

    return bret ? result : RTSCOPE::npos;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "while");


    RTSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }
    
    bret = bret && ParseArithmeticExpression(sConditional, &A);


    sBlock.begin = sConditional.end + 1;
    sBlock.end   = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[sBlock.begin];
    //SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;
    if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    {
      bret = sBlock.end != RTSCOPE::npos;
    }
    else
    {
      if(block_begin == '{')
      {
        sBlock.end = block_begin.scope;
        //sBlock.begin++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: while 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(sBlock, &B);
    }

    const SYNTAXNODE::MODE eMode = TryGetNode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    //if(TryGetNode(&B) != SYNTAXNODE::MODE_Block) {
    //  bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    //}

    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_While, &A, &B);

    DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end : RTSCOPE::npos;
  }
  
  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    ASSERT(m_aTokens[scope.begin] == "do");

    if(scope.begin + 1 >= scope.end) {
      // ERROR: do 语法错误
      return RTSCOPE::npos;
    }

    RTSCOPE sConditional;
    RTSCOPE sBlock;
    SYNTAXNODE::UN A = {0}, B = {0};


    if( ! MakeScope(&sBlock, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }

    RTSCOPE::TYPE while_token = sBlock.end + 1;
    
    if(while_token >= scope.end && m_aTokens[while_token] != "while") {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, while_token + 2, FALSE, while_token + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }

    // TODO： 验证域的开始是括号和花括号

    GXBOOL bret = ParseExpression(sBlock, &B);
    bret = bret && ParseArithmeticExpression(sConditional, &A);
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_DoWhile, NULL, &A, &B);

    RTSCOPE::TYPE while_end = sConditional.end + 1;
    if(while_end >= scope.end || m_aTokens[while_end] != ';') {
      // ERROR: 缺少 ";"
      return while_end;
    }

    return bret ? while_end : RTSCOPE::npos;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseStructDefine(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    SYNTAXNODE::UN T, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "struct");


    //RTSCOPE sName(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE::TYPE index = scope.begin + 2;

    if(index >= scope.end) {
      // ERROR: struct 定义错误，缺少定义名
      return RTSCOPE::npos;
    }

    // 确定定义块的范围
    RTSCOPE sBlock(index, m_aTokens[index].scope);
    if(sBlock.end == RTSCOPE::npos || sBlock.end >= scope.end) {
      ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(sBlock.begin));
      return RTSCOPE::npos;
    }

    clstack<SYNTAXNODE::UN> NodeStack;

    while(++index < sBlock.end && bret)
    {
      auto& decl = m_aTokens[index];
      if(decl.semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)decl.semi_scope >= sBlock.end) {
        ERROR_MSG_缺少分号(decl);
        break;
      }
      else if(index == decl.semi_scope) {
        continue;
      }

      bret = bret && ParseArithmeticExpression(RTSCOPE(index, decl.semi_scope), &T);
      NodeStack.push(T);
      
      index = decl.semi_scope;
    }

    while( ! NodeStack.empty())
    {
      MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &NodeStack.top(), &B);
      NodeStack.pop();
    }


    index = sBlock.end + 1;
    if(index >= scope.end || m_aTokens[index] != ';') {
      ERROR_MSG_缺少分号(IDX2ITER(sBlock.end - 1));
    }
    else {
      T.pSym = &m_aTokens[index];
      ASSERT(*T.pSym == ';');
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, &T);
    }

    T.pSym = &m_aTokens[scope.begin + 1];
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_StructDef, &T, &B);

    //DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end + 1 : RTSCOPE::npos;
  }

  GXBOOL CodeParser::MakeScope(RTSCOPE* pOut, MAKESCOPE* pParam)
  {
    // 这个函数用来从用户指定的begin和end中获得表达式所用的scope
    // 如果输入的begin和end符合要求则直接设置为scope，否则将返回FALSE
    // 可以通过mate标志获得begin或end位置上的配偶范围
    const MAKESCOPE& p = *pParam;
    RTSCOPE::TYPE& begin = pOut->begin;
    RTSCOPE::TYPE& end   = pOut->end;

    ASSERT(p.pScope->begin < p.pScope->end);

    if(p.begin >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bBeginMate) {
      begin = m_aTokens[p.begin].GetScope();

      if(OUT_OF_SCOPE(begin) || begin >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      begin = p.begin;
    }

    if(p.chTermin != 0 && m_aTokens[begin] == (TChar)p.chTermin) {
      end = begin;
      return TRUE;
    }

    if(p.end >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bEndMate) {
      end = m_aTokens[p.end].GetScope();

      if(OUT_OF_SCOPE(end) || end >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      end = p.end;
    }

    return TRUE;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::UN* pBlockNode)
  {
    ASSERT(m_aTokens[scope.begin] == "for"); // 外部保证调用这个函数的正确性

    auto open_bracket = scope.begin + 1;
    if(open_bracket >= scope.end || m_aTokens[open_bracket] != '(' || 
      (pIter->end = m_aTokens[open_bracket].scope) > scope.end)
    {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // initializer
    // 初始化部分
    //
    pInit->begin  = scope.begin + 2;
    pInit->end    = m_aTokens[scope.begin].semi_scope;
    if(pInit->begin >= scope.end || pInit->end == -1) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }
    ASSERT(pInit->begin <= pInit->end);

    //
    // conditional
    // 条件部分
    //
    pCond->begin  = pInit->end + 1;
    pCond->end    = m_aTokens[pCond->begin].semi_scope;

    if(pCond->begin >= scope.end) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // iterator
    // 迭代部分
    //
    pIter->begin = pCond->end + 1;
    // 上面设置过 pIter->end
    if(pIter->begin >= scope.end || pIter->begin > pIter->end) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // block
    //
    //RTSCOPE sBlock;
    pBlock->begin = pIter->end + 1;
    pBlock->end   = RTSCOPE::npos;
    if(pBlock->begin >= scope.end) {
      // ERROR: for 缺少执行
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[pBlock->begin];
    if(block_begin == '{')
    {
      pBlock->end = block_begin.scope;
      //pBlock->begin++;
    }
    else if(TryKeywords(RTSCOPE(pBlock->begin, scope.end), pBlockNode, &pBlock->end))
    {
      ; // 没想好该干啥，哇哈哈哈!
    }
    //else if(block_begin == "for")
    //{
    //  pBlock->end = RTSCOPE::npos;
    //  return scope.end;  // 这个地方有特殊处理，特殊返回
    //  //return ParseFlowFor(&RTSCOPE(pBlock->begin, pScope->end), pUnion);
    //}
    // "if"
    else
    {
      pBlock->end = block_begin.semi_scope;
    }

    ASSERT(pBlock->end != -1);

    if(pBlock->end > scope.end) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //ParseExpression(pBlock, pUnion);
    return pBlock->end;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE sInitializer, sConditional, sIterator;
    RTSCOPE sBlock;

    SYNTAXNODE::UN uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;
    
    auto result = MakeFlowForScope(scope, &sInitializer, &sConditional, &sIterator, &sBlock, &uBlock);
    if(result == RTSCOPE::npos)
    {
      return result;
    }

    ASSERT(m_aTokens[sBlock.begin] == "for" || m_aTokens[sBlock.end] == ';' || m_aTokens[sBlock.end] == '}');

    ParseArithmeticExpression(sInitializer, &uInit, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(sConditional, &uCond, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(sIterator   , &uIter, TOKEN::FIRST_OPCODE_PRECEDENCE);
    //if(sBlock.end == RTSCOPE::npos)
    //{
    //  ASSERT(m_aTokens[sBlock.begin] == "for"); // MakeFlowForScope 函数保证
    //  sBlock.end = ParseFlowFor(RTSCOPE(sBlock.begin, scope.end), &uBlock);
    //}
    //else

    if( ! uBlock.ptr) {
      ParseExpression(sBlock, &uBlock);
      //MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Chain, &uBlock, NULL);
    }

    GXBOOL bret = TRUE;
    // 如果不是代码块，就转换为代码块
    const SYNTAXNODE::MODE eMode = TryGetNode(&uBlock);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Chain, &uBlock, NULL);
      }
      bret = bret && MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Block, &uBlock, NULL);
    }

    //DbgDumpScope("for_2", sConditional, sIterator);
    //DbgDumpScope("for_1", sInitializer, sBlock);

    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForRunning, &uCond, &uIter);
    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForInit, &uInit, &D);
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_For, &D, &uBlock);
    
    return bret ? sBlock.end : RTSCOPE::npos;
  }






  //GXBOOL CodeParser::IsToken(const SYNTAXNODE::UN* pUnion) const
  //{
  //  const TOKEN* pBegin = &m_aTokens.front();
  //  const TOKEN* pBack   = &m_aTokens.back();

  //  return pUnion->pSym >= pBegin && pUnion->pSym <= pBack;
  //}

  //clsize CodeParser::FindSemicolon( clsize begin, clsize end ) const
  //{
  //  for(; begin < end; ++begin)
  //  {
  //    if(m_aTokens[begin] == ';') {
  //      return begin;
  //    }
  //  }
  //  return -1;
  //}

  const CodeParser::StatementArray& CodeParser::GetStatments() const
  {
    return m_aStatements;
  }

  void CodeParser::RelocaleStatements( StatementArray& aStatements )
  {
    for(auto it = aStatements.begin(); it != aStatements.end(); ++it)
    {
      switch(it->type)
      {
      case StatementType_FunctionDecl:
      case StatementType_Function:
        //it->func.pArguments = &m_aArgumentsPack[(GXINT_PTR)it->func.pArguments];
        IndexToPtr(it->func.pArguments, m_aArgumentsPack);
        IndexToPtr(it->func.pExpression, m_aSubStatements);
        break;

      case StatementType_Struct:
      case StatementType_Signatures:
        //it->stru.pMembers = &m_aMembersPack[(GXINT_PTR)it->stru.pMembers];
        IndexToPtr(it->stru.pMembers, m_aMembersPack);
        break;

      case StatementType_Expression:
        IndexToPtr(it->expr.sRoot.pNode, m_aSyntaxNodePack);
        RelocaleSyntaxPtr(it->expr.sRoot.pNode);
        break;

      case StatementType_Definition:
        if(TryGetNodeType(&it->defn.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX)
        {
          IndexToPtr(it->defn.sRoot.pNode, m_aSyntaxNodePack);
          RelocaleSyntaxPtr(it->defn.sRoot.pNode);
        }
        break;
      }
    }
  }


  //////////////////////////////////////////////////////////////////////////

  CodeParser* CodeParser::GetSubParser()
  {
    if( ! m_pSubParser) {
      m_pSubParser = new CodeParser;
    }
    return m_pSubParser;
  }

  CodeParser::T_LPCSTR CodeParser::ParseMacro(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end)
  {
    CodeParser* pParse = GetSubParser();
    pParse->Attach(begin, (clsize)end - (clsize)begin, AttachFlag_NotLoadMessage, NULL);
    pParse->GenerateTokens();
    const auto& tokens = *pParse->GetTokensArray();

    if(tokens.front() == "define") {
      Macro_Define(tokens);
    }
    else if(tokens.front() == "undef") {
      Macro_Undefine(ctx, tokens);
    }
    else if(tokens.front() == "ifdef") {
      return Macro_IfDefine(ctx, FALSE, tokens);
    }
    else if(tokens.front() == "ifndef") {
      return Macro_IfDefine(ctx, TRUE, tokens);
    }
    else {
      OutputErrorW(tokens.front(), E1021_无效的预处理器命令, clStringW(tokens.front().ToString()));
      return end;
    }
    return ctx.iter_next.marker;
  }

  void CodeParser::Macro_Define(const TokenArray& tokens)
  {
    MACRO l_m;
    //const auto& tokens = *m_pSubParser->GetTokensArray();
    ASSERT( ! tokens.empty() && tokens.front() == "define");
    const auto count = tokens.size();
    clStringA strMacroName(tokens[1].marker.marker, tokens[1].marker.length);
    //m_MacrosSet.insert(strMacroName);

    if(count == 1) {
      OutputErrorW(tokens.front(), E2007_define缺少定义);
    }
    else if(count == 2) // "#define MACRO" 形
    {
      m_Macros.insert(clmake_pair(strMacroName, l_m));
    }
    else if(count == 3) // "#define MACRO XXX" 形
    {
      auto result = m_Macros.insert(clmake_pair(strMacroName, l_m));

      // 如果已经添加过，清除原有数据
      if( ! result.second) {
        result.first->second.clear();
      }
      result.first->second.aTokens.push_back(tokens[2]);
      result.first->second.ClearContainer();      
    }
    else
    {
      int l_define = 2;

      // 宏定义名后不是开括号并且紧跟在宏定义名后则报错
      if(tokens[1].marker.end() == tokens[2].marker.marker)
      {
        if(tokens[2] != '(') {
          OutputErrorW(tokens[2], E2008_宏定义中的意外, clStringW(tokens[2].ToString()));
          return;
        }

        const int scope_end = tokens[2].scope;
        if(scope_end > 3) // #define MACRO(...) ... 形解析
        {
          for(int i = 3; i < scope_end; i++)
          {
            if((tokens[i] == ',' && (i & 1)) || (tokens[i] != ',' && (i & 1) == 0)) {
              OutputErrorW(tokens[i], E2010_宏形参表中的意外, clStringW(tokens[i].ToString()));
              return;
            }
            if(i & 1) {
              l_m.aFormalParams.push_back(tokens[i++]);
            }
          }

          //m_Macros.insert(clmake_pair(strMacroName, l_m)); // 添加原名
          //strMacroName.AppendFormat("@A%d", l_m.aFormalParams.size());
        }
        l_define = scope_end + 1;
      }
      else {} // #define MACRO ... 形解析

      auto result = m_Macros.insert(clmake_pair(strMacroName, l_m));
      if( ! result.second) {
        result.first->second.clear();
      }

      result.first->second.set(m_Macros, tokens, l_define);
    }
  }

  GXBOOL CodeParser::Macro_ExpandMacroInvoke(clStringA& strMacro, TOKEN& token)
  {
    ASSERT(token == ')' && token.scope != -1);
    ASSERT(token.scope >= 1); // 不可能出现宏所用的括号在m_aTokens第一个位置的情况

    RTSCOPE MacroArgsScope(token.scope + 1, m_aTokens.size());
    if( ! MacroArgsScope.IsValid()) {
      // ERROR: C4003: “ADD”宏的实参不足
      return FALSE;
    }

    RTSCOPE::List arg_list;
    RTSCOPE arg(MacroArgsScope.begin, MacroArgsScope.begin);

    for(RTSCOPE::TYPE i = MacroArgsScope.begin; i < MacroArgsScope.end; i++)
    {
      if(m_aTokens[i] == ',') {
        arg.end = i;
        arg_list.push_back(arg);
        arg.begin = i + 1;
      }
    }
    arg.end = MacroArgsScope.end;
    arg_list.push_back(arg);

    auto iter_macro = m_Macros.find(strMacro);
    ASSERT(iter_macro != m_Macros.end()); // 不应该出现记录了宏处理的名字却在宏字典里找不到的情况
    strMacro.Clear();

    const MACRO& macro = iter_macro->second;
    if(macro.aFormalParams.size() != arg_list.size()) {
      // ERROR: 宏参数不一致
      return FALSE;
    }

    TOKEN::List token_list;
    token_list.insert(token_list.begin(), macro.aTokens.begin(), macro.aTokens.end());

    auto iter_arg = arg_list.begin();
    for(auto iter_formalparam = macro.aFormalParams.begin(); 
      iter_formalparam != macro.aFormalParams.end(); ++iter_formalparam, ++iter_arg)
    {
      for(auto it = token_list.begin(); it != token_list.end();)
      {
        if(*it == *iter_formalparam) {
          *it = m_aTokens[iter_arg->begin];
          ++it;
          for(RTSCOPE::TYPE i = iter_arg->begin + 1; i < iter_arg->end; i++)
          {
            it = token_list.insert(it, m_aTokens[i]);
            ++it;
          }
        }
        else {
          ++it;
        }
      }
    }

    m_aTokens.erase(m_aTokens.begin() + token.scope - 1, m_aTokens.end());
    Append(token_list.begin(), token_list.end());
    //// 宏定义参数中不应该出现分号，此时刚遇到')'，也还没有设置整个表达式的分号域
    //if(m_aTokens[token.scope - 1].semi_scope == -1) {
    //  
    //}

    //SYNTAXNODE::UN sUnion;
    //CodeParser* pParser = GetSubParser();
    //pParser->Cleanup();
    //pParser->m_aTokens.insert(pParser->m_aTokens.begin(), m_aTokens.begin() + argscope.begin, m_aTokens.end()); // TODO: 增加一个专门的传递方法，调整scope标记
    //if( ! pParser->ParseArithmeticExpression(0, pParser->m_aTokens.size(), &sUnion)) {
    //  return;
    //}

    return TRUE;
  }

  void CodeParser::Macro_Undefine(const RTPPCONTEXT& ctx, const TokenArray& aTokens)
  {
    ASSERT(aTokens.front() == "undef");
    if(aTokens.size() == 1) {
      OutputErrorW(aTokens.front(), E4006_undef应输入标识符);
      return;
    }
    else if(aTokens.size() > 2) {
      OutputErrorW(aTokens.front(), E4067_预处理器指令后有意外标记_应输入换行符);
      return;
    }

    //clStringA str = aTokens[1].ToString();
    clStringA strMacroName = aTokens[1].ToString();
    //m_MacrosSet.erase(strMacroName);
    auto it = m_Macros.find(strMacroName);
    ASSERT(it != m_Macros.end()); // 集合里有的话化名表里也应该有
    
    //strMacroName.Append("@A");
    //do {
    //  it = m_Macros.erase(it);
    //} while (it->first.BeginsWith(strMacroName));
  }

  CodeParser::T_LPCSTR CodeParser::Macro_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TokenArray& tokens)
  {
    //const auto& tokens = *m_pSubParser->GetTokensArray();
    ASSERT( ! tokens.empty() && (tokens.front() == "ifdef" || tokens.front() == "ifndef"));
    T_LPCSTR stream_end = GetStreamPtr() + GetStreamCount();

    if(tokens.size() == 1) {
      // ERROR: ifdef 缺少定义
    }
    else if(tokens.size() == 2) {
      const GXBOOL bFind = (m_Macros.find(tokens[1].ToString()) == m_Macros.end());
      if(( ! bNot && bFind) || (bNot && ! bFind))
      {
        const T_LPCSTR pBlockEnd = 
          Macro_SkipConditionalBlock(ctx.ppend, stream_end);
        ASSERT(pBlockEnd >= ctx.iter_next.marker);
        return pBlockEnd;
      }
      else {
        return ctx.iter_next.marker;
      }
    }
    else {
      CLBREAK; // 没完成
    }
    return stream_end;
  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipConditionalBlock( T_LPCSTR begin, T_LPCSTR end )
  {
    typedef clstack<T_LPCSTR> PPNestStack;  // 预处理命令嵌套堆栈
    PPNestStack pp_stack;

    T_LPCSTR p = begin;
    for(; p < end; ++p)
    {
      if(*p != '\n') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
        return end;
      }

      if(*p != '#') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
        return end;
      }

      // if 要放在两个 if* 后面
      if(CompareString(p, "ifdef", 5) || CompareString(p, "ifndef", 6) || CompareString(p, "if", 2))
      {
        pp_stack.push(p);
      }
      else if(CompareString(p, "elif", 4))
      {
        // pp_stack 不空，说明在预处理域内，直接忽略
        // pp_stack 为空，测试表达式(TODO)
        if( ! pp_stack.empty()) {
          continue;;
        }
      }
      else if(CompareString(p, "else", 4))
      {
        // pp_stack 不空，说明在预处理域内，直接忽略
        // pp_stack 为空，转到下行行首
        if( ! pp_stack.empty()) {
          continue;
        }

        p += 4; // "else" 长度
        break;
      }
      else if(CompareString(p, "endif", 5))
      {
        // 测试了下，vs2010下 #endif 后面随便敲些什么都不会报错

        if( ! pp_stack.empty()) {
          pp_stack.pop();
          continue;
        }

        p += 5; // "endif" 长度
        break;
      }
      else {
        // ERROR: 无效的预处理命令
        T_LPCSTR pend = p;
        for(; pend < end; pend++) {
          if(*pend == 0x20 || *pend == '\t' || *pend == '\r' || *pend == '\n') {
            break;
          }
        }
        clStringW str(p, (size_t)pend - (size_t)p);
        OutputErrorW(p, E1021_无效的预处理器命令, str);
      }
    }
    
    for(; *p != '\n' && p < end; p++);
    return p != end ? ++p : end;
  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGaps( T_LPCSTR p, T_LPCSTR end )
  {
    do {
      p++;
    } while ((*p == '\t' || *p == 0x20) && p < end);
    return p;
  }

  GXBOOL CodeParser::CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count)
  {
    TChar c = str1[count];
    ASSERT(c != '\0');
    return GXSTRNCMP(str1, str2, count) == 0 && 
      (c == '\t' || c == 0x20 || c == '\r' || c == '\n');
  }

  //void CodeParser::OutputErrorW( GXSIZE_T offset, GXUINT code, ... )
  //{
  //  va_list  arglist;
  //  va_start(arglist, code);
  //  m_pMsg->VarWriteErrorW(TRUE, offset, code, arglist);
  //  va_end(arglist);
  //}

  void CodeParser::OutputErrorW(const TOKEN& token, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, token.marker.marker, code, arglist);
    va_end(arglist);
  }

  void CodeParser::OutputErrorW(T_LPCSTR ptr, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, ptr, code, arglist);
    va_end(arglist);
  }

  //////////////////////////////////////////////////////////////////////////

  bool CodeParser::TYPE::operator<( const TYPE& t ) const
  {
    const int r = GXSTRCMP(name, t.name);
    if(r < 0) {
      return TRUE;
    }
    else if(r > 0) {
      return FALSE;
    }
    return ((R << 3) | C) < ((t.R << 3) | t.C);
  }

  //////////////////////////////////////////////////////////////////////////

  void CodeParser::MACRO::clear()
  {
    aTokens.clear();
    aFormalParams.clear();
  }

  void CodeParser::MACRO::set(const Dict& dict, const TokenArray& tokens, int begin_at)
  {
    ASSERT(tokens.front() == "define");
    aTokens.insert(aTokens.begin(), tokens.begin() + begin_at, tokens.end());
    ClearContainer();
    while(ExpandMacro(dict) > 0); // 反复调用直到返回0 
  }

  void CodeParser::MACRO::ClearContainer()
  {
    for(auto it = aTokens.begin(); it != aTokens.end(); ++it) {
      it->marker.pContainer = NULL;
    }
    for(auto it = aFormalParams.begin(); it != aFormalParams.end(); ++it) {
      it->marker.pContainer = NULL;
    }
  }

  int CodeParser::MACRO::ExpandMacro( const Dict& dict )
  {
    int result = 0;
    for(auto it = aTokens.begin(); it != aTokens.end();) {
      if(it->precedence == 0 && it->scope == -1) {
        auto iter_dict = dict.find(it->ToString());
        if(iter_dict != dict.end() && &iter_dict->second != this)
        {
          it = aTokens.erase(it);
          aTokens.insert(it, iter_dict->second.aTokens.begin(), iter_dict->second.aTokens.end());
          result++;
          continue;
        }
      }
      ++it;
    }
    return result;
  }

} // namespace UVShader
