#include <clstd.h>
#include <clString.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clColorSpace.h>
#include <GrapX/GBaseTypes.h>
#include "ExpressionParser.h"

//  Precedence  Operator          Description                                               Associativity 
//
//  1           ::                Scope resolution                                          Left-to-right
//
//  2           ++   --           Suffix/postfix increment and decrement                    Left-to-right
//              ()                Function call
//              []                Array subscripting
//              .                 Element selection by reference
//              −>                Element selection through pointer
//
//  3           ++   --           Prefix increment and decrement                            Right-to-left 
//              +   −             Unary plus and minus
//              !   ~             Logical NOT and bitwise NOT
//              (type)            Type cast
//              *                 Indirection (dereference)
//              &                 Address-of
//              sizeof            Size-of
//              new, new[]        Dynamic memory allocation
//              delete, delete[]  Dynamic memory deallocation
//
//  4           .*   ->*          Pointer to member                                         Left-to-right 
//  5           *   /   %         Multiplication, division, and remainder                   Left-to-right 
//  6           +   −             Addition and subtraction                                  Left-to-right 
//  7           <<   >>           Bitwise left shift and right shift                        Left-to-right 
//  8           <   <=            For relational operators < and ≤ respectively             Left-to-right 
//              >   >=            For relational operators > and ≥ respectively             Left-to-right 
//  9           ==   !=           For relational = and ≠ respectively                       Left-to-right 
//  10          &                 Bitwise AND                                               Left-to-right 
//  11          ^                 Bitwise XOR (exclusive or)                                Left-to-right 
//  12          |                 Bitwise OR (inclusive or)                                 Left-to-right 
//  13          &&                Logical AND                                               Left-to-right 
//  14          ||                Logical OR                                                Left-to-right 
//  15          ?:                Ternary conditional                                       Right-to-left 
//
//              =                 Direct assignment (provided by default for C++ classes)
//              +=   −=           Assignment by sum and difference
//              *=   /=   %=      Assignment by product, quotient, and remainder
//              <<=   >>=         Assignment by bitwise left shift and right shift
//              &=   ^=   |=      Assignment by bitwise AND, XOR, and OR
//
//  16          throw             Throw operator (for exceptions)                           Right-to-left 
//  17          ,                 Comma                                                     Left-to-right 

static clsize s_nMultiByteOperatorLen = 0; // 最大长度
struct MBO
{
  clsize nLen;
  char* szOperator;
};
static MBO s_MultiByteOperator[] = {
  {0, "::", },
  {0, "--", },
  {0, "++", },
  {0, "->", },
  {0, ".*", },
  {0, "->*",}, 
  {0, ">>", },
  {0, "<<", },
  {0, "<=", },
  {0, ">=", },
  {0, "==", },
  {0, "!=", },
  {0, "&&", },
  {0, "||", },
  {0, "+=", },
  {0, "-=", },
  {0, "*=", },
  {0, "/=", },
  {0, "%=", },
  {0, "<<=",}, 
  {0, ">>=",}, 
  {0, "&=", },
  {0, "^=", },
  {0, "|=", },
  NULL };

#define FOR_EACH_MBO(_IDX) for(int _IDX = 0; s_MultiByteOperator[_IDX].szOperator != NULL; _IDX++)

inline b32 IS_NUM(char c)
{
  return c >= '0' && c <= '9';
}

ExpressionParser::ExpressionParser()
{
  u32 aCharSem[128];
  GetCharSemantic(aCharSem, 0, 128);

  FOR_EACH_MBO(i)
  {
    aCharSem[s_MultiByteOperator[i].szOperator[0]] |= M_CALLBACK;
    s_MultiByteOperator[i].nLen = clstd::strlenT(s_MultiByteOperator[i].szOperator);
    s_nMultiByteOperatorLen = clMax(
      s_nMultiByteOperatorLen, s_MultiByteOperator[i].nLen);
  }
  SetFlags(GetFlags() | F_SYMBOLBREAK);
  SetCharSemantic(aCharSem, 0, 128);
  SetIteratorCallBack(IteratorProc, 0);
  SetTriggerCallBack(MultiByteOperatorProc, 0);
}

b32 ExpressionParser::Attach( const char* szExpression, clsize nSize )
{
  return SmartStreamA::Initialize(szExpression, nSize);
}

u32 CALLBACK ExpressionParser::MultiByteOperatorProc( iterator& it, u32 nRemain, u32_ptr lParam )
{
  FOR_EACH_MBO(i)
  {
    const clsize nCmpLen = clMin(s_MultiByteOperator[i].nLen, nRemain);
    if(clstd::strncmpT(s_MultiByteOperator[i].szOperator, it.marker, 
      s_MultiByteOperator[i].nLen) == 0)
    {
      it.length = nCmpLen;
      break;
    }
  }
  return 0;
}

u32 CALLBACK ExpressionParser::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
{
  GXBOOL bENotation = FALSE;

  //
  // 进入数字判断模式
  //
  if((it.front() == '.' && IS_NUM(it.marker[it.length])) ||             // 第一个是'.'
    (IS_NUM(it.front()) && (it.back() == 'e' || it.back() == 'E')) ||   // 第一个是数字，最后以'e'结尾
    (IS_NUM(it.back()) && it.marker[it.length] == '.'))                 // 最后是数字，下一个是'.'
  {
    it.length++;
    while(--remain)
    {
      if(IS_NUM(it.marker[it.length])) {
        it.length++;
      }
      else if( ! bENotation && // 没有切换到科学计数法时遇到‘e’标记
        (it.marker[it.length] == 'e' || it.marker[it.length] == 'E'))
      {
        bENotation = TRUE;
        it.length++;

        // 科学计数法，+/- 符号判断
        if((--remain) != 0 && (*(it.end()) == '-' || *(it.end()) == '+')) {
          it.length++;
        }
      }
      else {
        break;
      }
    }
    if(it.marker[it.length] == 'f' || it.marker[it.length] == 'F') {
      it.length++;
    }
  }
  else if(it.marker[0] == '/' && (remain > 0 && it.marker[1] == '/')) // 处理单行注释“//...”
  {
    SmartStreamUtility::ExtendToNewLine(it, 2, remain);
    ++it;
  }
  else if(it.marker[0] == '/' && (remain > 0 && it.marker[1] == '*')) // 处理块注释“/*...*/”
  {
    SmartStreamUtility::ExtendToCStyleBlockComment(it, 2, remain);
    ++it;
  }
  ASSERT((int)remain >= 0);
  return 0;
}