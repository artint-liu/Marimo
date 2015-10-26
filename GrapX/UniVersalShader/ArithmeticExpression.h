#ifndef _ARITHMETIC_EXPRESSION_H_
#define _ARITHMETIC_EXPRESSION_H_

#define IDX2ITER(_IDX)                           m_aTokens[_IDX]
#define ERROR_MSG__MISSING_SEMICOLON(_token)      m_pMsg->WriteErrorW(TRUE, (_token).marker.offset(), 1000)
#define ERROR_MSG__MISSING_OPENBRACKET    CLBREAK
#define ERROR_MSG__MISSING_CLOSEDBRACKET  CLBREAK

//
// 竟然可以使用中文 ಠ౪ಠ
//
#define ERROR_MSG_缺少分号(_token)    ERROR_MSG__MISSING_SEMICOLON(_token)
#define ERROR_MSG_缺少开括号  ERROR_MSG__MISSING_OPENBRACKET  
#define ERROR_MSG_缺少闭括号  ERROR_MSG__MISSING_CLOSEDBRACKET
#define ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动 CLBREAK
//#define ERROR_MSG_C1021_无效的预处理器命令 

#define E1021_无效的预处理器命令  1021
#define E1189_用户定义错误        1189
#define E4006_undef应输入标识符   4006
#define E4067_预处理器指令后有意外标记_应输入换行符 4067
#define E2007_define缺少定义     2007
#define E2008_宏定义中的意外     2008
#define E2010_宏形参表中的意外   2010

#define UNARY_LEFT_OPERAND    2 // 10B， 注意是允许操作数在左侧
#define UNARY_RIGHT_OPERAND   1 // 01B
#define ENABLE_STRINGED_SYMBOL
#define OPP(_PRE) (ArithmeticExpression::TOKEN::FIRST_OPCODE_PRECEDENCE + _PRE)


namespace Marimo
{
  template<typename _TChar>
  class DataPoolErrorMsg;
} // namespace Marimo

namespace UVShader
{
  class ArithmeticExpression : public SmartStreamA
  {
  public:
    struct TOKEN // 运行时记录符号和操作符等属性
    {
      typedef cllist<TOKEN>   List;
      typedef clvector<TOKEN> Array;
      const static int scope_bits = 22;
      const static int precedence_bits = 7;
      const static int FIRST_OPCODE_PRECEDENCE = 1;
      const static int ID_BRACE = 15;
#ifdef ENABLE_STRINGED_SYMBOL
      clStringA symbol;
#endif // #ifdef ENABLE_STRINGED_SYMBOL
      iterator  marker;
      int       scope;                        // 括号匹配索引
      int       semi_scope : scope_bits;      // 分号作用域
      int       precedence : precedence_bits; // 符号优先级
      u32       unary      : 1;               // 是否为一元操作符
      u32       unary_mask : 2;               // 一元操作符允许位：11B表示操作数可以在左边和右边，01B表示操作数只能在右边，10B表示操作数只能在左边
      // [precedence]
      // 1~14 表示运算符的优先级
      // 15 表示"{","}","(",")","[","]"这些之一，此时scope数据是相互对应的，例如
      // array[open].scope = closed && array[closed].scope = open

      void ClearMarker()
      {
#ifdef ENABLE_STRINGED_SYMBOL
        symbol.Clear();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
        marker.marker = 0;
      }

      void Set(const iterator& _iter)
      {
        ASSERT(_iter.length != 0);
#ifdef ENABLE_STRINGED_SYMBOL
        symbol = _iter.ToString();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
        marker = _iter;
      }

      template<class _Ty>
      void SetArithOperatorInfo(const _Ty& t) // 设置数学符号信息
      {
        unary      = t.unary;
        unary_mask = t.unary_mask;
        precedence = t.precedence;
      }

      void ClearArithOperatorInfo()
      {
        unary      = 0;
        unary_mask = 0;
        precedence = 0;
      }

      clStringA ToString() const
      {
        return marker.ToString();
      }

      int GetScope() const
      {
        return scope >= 0 ? scope : semi_scope;
      }

      GXBOOL operator==(const TOKEN& t) const
      {
        // 不可能出现指向同一地址却长度不同的情况
        ASSERT((marker.marker == t.marker.marker && marker.length == t.marker.length) || 
          (marker.marker != t.marker.marker));

        return (marker.marker == t.marker.marker) || (marker.length == t.marker.length
          && GXSTRNCMP(marker.marker, t.marker.marker, marker.length) == 0);
      }

      GXBOOL operator==(SmartStreamA::T_LPCSTR str) const
      {
        return (marker == str);
      }

      GXBOOL operator==(SmartStreamA::TChar ch) const
      {
        return (marker == ch);
      }

      GXBOOL operator!=(SmartStreamA::T_LPCSTR str) const
      {
        return (marker != str);
      }

      GXBOOL operator!=(SmartStreamA::TChar ch) const
      {
        return (marker != ch);
      }

      b32 operator<(const TOKEN& _token) const
      {
        return (b32)(marker < _token.marker);
      }
    };
    //typedef clvector<TOKEN> TokenArray;
    //typedef cllist<TOKEN> TokenList;


    struct MBO // 静态定义符号属性用的结构体
    {
      clsize nLen;
      char* szOperator;
      int precedence; // 优先级，越大越高

      u32 unary      : 1; // 参考 TOKEN 机构体说明
      u32 unary_mask : 2;
    };

    struct PAIRMARK
    {
      GXCHAR    chOpen;         // 开区间
      GXCHAR    chClosed;       // 闭区间
      GXUINT    bNewEOE   : 1;  // 更新End Of Expression的位置
      GXUINT    bCloseAE  : 1;  // AE = Another Explanation, 闭区间符号有另外解释，主要是"...?...:..."操作符
    };


    struct SYNTAXNODE
    {
      enum FLAGS
      {
        FLAG_OPERAND_SHIFT      = 8,
        FLAG_OPERAND_UNDEFINED  = 0,
        FLAG_OPERAND_TYPEMASK   = 0x0000000F,
        FLAG_OPERAND_IS_NODEIDX = 0x00000001,
        FLAG_OPERAND_IS_NODE    = 0x00000002,
        FLAG_OPERAND_IS_TOKEN   = 0x00000004,
      };

      enum MODE
      {
        MODE_Undefined,
        MODE_Opcode,          // 操作符 + 操作数 模式
        MODE_FunctionCall,    // 函数调用
        MODE_ArrayIndex,      // 函数调用
        MODE_Definition,      // 变量定义
        MODE_DefinitionConst, // 常量量定义
        MODE_StructDef,       // 结构定义
        MODE_Flow_While,
        MODE_Flow_If,         // if(A) {B}
        MODE_Flow_ElseIf,
        MODE_Flow_Else,
        MODE_Flow_For,            // [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
        MODE_Flow_ForInit,        // for 的初始化部分 [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
        MODE_Flow_ForRunning,     // for 的条件和步进部分
        //MODE_Flow_Switch,
        MODE_Flow_DoWhile,
        MODE_Flow_Break,
        MODE_Flow_Continue,
        MODE_Flow_Discard,
        MODE_Return,
        MODE_Block,
        MODE_Chain,         // 表达式链表,链表中的应该属于同一个作用域, [A:statement][B:next chain]，在chain结尾应该是[A:statement][B:NULL]这样
      };

      const static int s_NumOfOperand = 2;

      GXDWORD flags : 16;
      MODE    mode  : 16;
      const TOKEN* pOpcode;

      union UN {
        void*         ptr;    // 任意类型，在只是判断UN是否有效时用具体类型可能会产生误解，所以定义了通用类型
        SYNTAXNODE*   pNode;
        size_t        nNodeIndex;
        const TOKEN*  pSym;
      };

      struct DESC {
        UN    un;
        FLAGS flag; // 标记un具体是哪一个类型
        DESC& operator=(const TOKEN& token) {
          flag = FLAG_OPERAND_IS_TOKEN;
          un.pSym = &token;
          return *this;
        }
      };

      UN Operand[s_NumOfOperand];

      inline FLAGS GetOperandType(const int index) const {
        const int shift = FLAG_OPERAND_SHIFT * index;
        return (FLAGS)((flags >> shift) & FLAG_OPERAND_TYPEMASK);
      }

      inline void SetOperandType(const int index, FLAGS flag) {
        const int shift = FLAG_OPERAND_SHIFT * index;
        flags = (flags & (~(FLAG_OPERAND_TYPEMASK << shift))) | (flag << shift);
      }

      inline GXBOOL OperandA_IsNodeIndex() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK;
        return (flags & dwTypeMask) == FLAG_OPERAND_IS_NODEIDX;
      }

      inline GXBOOL OperandB_IsNodeIndex() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK << FLAG_OPERAND_SHIFT;
        const GXDWORD dwNode = FLAG_OPERAND_IS_NODEIDX << FLAG_OPERAND_SHIFT;
        return (flags & dwTypeMask) == dwNode;
      }

      inline void Clear()
      {
        flags  = 0;
        mode   = MODE_Undefined;
        pOpcode = NULL;
        Operand[0].ptr = NULL;
        Operand[1].ptr = NULL;
      }

      template<class _Func>
      static void RecursiveNode(ArithmeticExpression* pParser, SYNTAXNODE* pNode, _Func func) // 广度优先递归
      {
        if(func(pNode)) {
          int i = 0;
          do {
            auto type = pNode->GetOperandType(i);
            switch(type)
            {
            case FLAG_OPERAND_IS_NODE:
              RecursiveNode(pParser, pNode->Operand[i].pNode, func);
              break;
            case FLAG_OPERAND_IS_NODEIDX:
              RecursiveNode(pParser, &pParser->
                m_aSyntaxNodePack[pNode->Operand[i].nNodeIndex], func);
              break;
            }
            i++;
          }while(i < 2);
        }
      }

      template<class _Func>
      static void RecursiveNode2(ArithmeticExpression* pParser, SYNTAXNODE* pNode, _Func func) // 深度优先递归
      {
        int i = 0;
        do {
          auto type = pNode->GetOperandType(i);
          switch(type)
          {
          case FLAG_OPERAND_IS_NODE:
            RecursiveNode2(pParser, pNode->Operand[i].pNode, func);
            break;
          case FLAG_OPERAND_IS_NODEIDX:
            RecursiveNode2(pParser, &pParser->
              m_aSyntaxNodePack[pNode->Operand[i].nNodeIndex], func);
            break;
          }
          i++;
        }while(i < 2);
        func(pNode);
      }

    };
    typedef clvector<SYNTAXNODE>  SyntaxNodeArray;
    typedef clstack<int>          PairStack;


    struct RTSCOPE // 运行时的范围描述结构体
    {
      typedef cllist<RTSCOPE> List;
      typedef clsize TYPE;
      const static TYPE npos = -1;
      TYPE begin;
      TYPE end;

      RTSCOPE(){}
      RTSCOPE(clsize _begin, clsize _end) : begin(_begin), end(_end) {}

      inline GXBOOL IsValid() const {
        return (begin != (clsize)-1) && begin < end;
      }
    };

    struct INTRINSIC_TYPE // 内置类型
    {
      GXLPCSTR name;
      clsize   name_len;
      int      R;         // 最大允许值
      int      C;         // 最大允许值
    };

    typedef Marimo::DataPoolErrorMsg<ch> ErrorMessage;
  protected:
    static INTRINSIC_TYPE s_aIntrinsicType[];
    static MBO s_semantic;
    static MBO s_plus_minus[];
    static MBO s_Operator1[];
    static MBO s_Operator2[];
    static MBO s_Operator3[];
    static PAIRMARK s_PairMark[4];
    static const int s_MaxPrecedence = 14;
    static const int s_nPairMark = sizeof(s_PairMark) / sizeof(PAIRMARK);



    ErrorMessage*       m_pMsg;
    TOKEN::Array        m_aTokens;
    //int                 m_nMaxPrecedence;   // 优先级最大值
    SyntaxNodeArray     m_aSyntaxNodePack;  // 表达式语法节点

    int                 m_nDbgNumOfExpressionParse; // 调试模式用于记录解析表达式迭代次数的变量
    clStringArrayA      m_aDbgExpressionOperStack;

  protected:
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);

    void    MarryBracket(PairStack* sStack, TOKEN& token, int& EOE);
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB);
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB);
    GXBOOL  MakeInstruction(const TOKEN* pOpcode, int nMinPrecedence, const RTSCOPE* pScope, SYNTAXNODE::DESC* pParent, int nMiddle); // nMiddle是把RTSCOPE分成两个RTSCOPE的那个索引

    GXBOOL  ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    GXBOOL  ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    
  public:
    ArithmeticExpression();
    virtual ~ArithmeticExpression();

    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const; // TODO: 修改所属类
    //SYNTAXNODE::MODE  TryGetNode    (const SYNTAXNODE::UN* pUnion) const; // TODO: 修改所属类
    SYNTAXNODE::MODE  TryGetNode    (const SYNTAXNODE::DESC* pDesc) const; // TODO: 修改所属类

    clsize              EstimateForTokensCount  () const;   // 从Stream的字符数估计Token的数量
    //clsize              GenerateTokens          ();
    const TOKEN::Array* GetTokensArray          () const;

    GXBOOL  ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::DESC* pDesc);
    GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, int nMinPrecedence); // 递归函数

    void DbgDumpScope(clStringA& str, const RTSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end, GXBOOL bRaw);
    void DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB);
    const clStringArrayA& DbgGetExpressionStack() const;

  };


} // namespace UVShader

namespace stdext
{
  inline size_t hash_value(const UVShader::ArithmeticExpression::TOKEN& _token)
  {
    u32 _Val = 2166136261U;

    auto* pBegin = _token.marker.marker;
    auto* pEnd   = _token.marker.end();
    while (pBegin != pEnd) {
      _Val = 16777619U * _Val ^ (u32)*pBegin++;
    }
    return (_Val);
  }
}

// gcc stlport
namespace std
{
  template<> struct hash<UVShader::ArithmeticExpression::TOKEN>
  {
    size_t operator()(const UVShader::ArithmeticExpression::TOKEN& _token) const { return stdext::hash_value(_token); }
  };
}

#endif // _ARITHMETIC_EXPRESSION_H_