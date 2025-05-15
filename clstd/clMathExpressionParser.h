#ifndef _CLSTD_MATH_EXPRESSION_PARSER_H_
#define _CLSTD_MATH_EXPRESSION_PARSER_H_

namespace clstd
{
  template<class _TString>
  class MathExpressionParser
  {
  public:
    typedef cllist<_TString> ElementList;

  protected:
    ElementList m_PostfixExpression;

  public:

    i32 OperatorPrecedence(const _TString& str)
    {
      // [ 0] 代表操作数
      // [ 1] '(' ')'
      // [ 4] (*) (/)
      // [ 5] (+) (-)
      // [ 6] (<<) (>>)
      // [ 9] (&)
      // [10] (^)
      // [11] (|)
      if(str == "(" || str == ")") {        return 1;  }
      else if(str == "*" || str == "/") {   return 4;  }
      else if(str == "+" || str == "-") {   return 5;  }
      else if(str == ">>" || str == "<<") { return 6;  }
      else if(str == "&") {                 return 9;  }
      else if(str == "^") {                 return 10; }
      else if(str == "|") {                 return 11; }

      return 0;
    }

    template<class _TList>
    b32 ParseExpression( const _TList& expression )
    {
      m_PostfixExpression.clear();
      if(expression.empty()) {
        return FALSE;
      }
      else if(expression.size() == 1) {
        m_PostfixExpression.push_back(expression.front());
        return TRUE;
      }

      // 后缀表达式转换
      clstack<_TString> stackOp;
      for(auto it = expression.begin(); it != expression.end(); ++it)
      {
        int o = OperatorPrecedence(*it);
        if(o == 0) {
          stackOp.push(*it);
        }
        else {
          if(*it == "(") {
            stackOp.push(*it);
          }
          else if(*it == ")")
          {
            while( ! stackOp.empty()) {
              if(stackOp.top() == "(") {
                stackOp.pop();
                break;
              }
              else {
                m_PostfixExpression.push_back(stackOp.top());
                stackOp.pop();
              }
            }
          }
          else {
            int oTop = OperatorPrecedence(stackOp.top());
            if(o >= oTop) // 当前操作符小于等于栈顶操作符
            {
              m_PostfixExpression.push_back(stackOp.top());
              stackOp.pop();
            }
            stackOp.push(*it);
          }
        }
      }

      while( ! stackOp.empty()) {
        m_PostfixExpression.push_back(stackOp.top());
        stackOp.pop();
      }
      return TRUE;
    }

    template<typename _TNumeric, class _Fn>
    b32 CalculateValue(_TNumeric* pValue, _Fn fn)
    {
      struct ELEMENT
      {
        // 存储时二选一
        _TString* pstr;
        _TNumeric value;
      };

      clstack<ELEMENT> stackCalc;
      ELEMENT e;
      for(auto it = m_PostfixExpression.begin(); it != m_PostfixExpression.end(); ++it) {
        u32 o = OperatorPrecedence(*it);
        if(o == 0) {
          e.pstr = &*it;
          e.value = 0;
          stackCalc.push(e);
        }
        else {
          auto Operand2 = stackCalc.top();
          stackCalc.pop();
          auto Operand1 = stackCalc.top();
          stackCalc.pop();

          if(
            (Operand1.pstr && ( ! fn(*Operand1.pstr, &Operand1.value)))
            ||
            (Operand2.pstr && ( ! fn(*Operand2.pstr, &Operand2.value))) ) {
              return FALSE;
          }

          Operand1.pstr = NULL;
          Operand2.pstr = NULL;
          e.pstr = NULL;

          /**/ if(*it == "+") { e.value = Operand1.value + Operand2.value; stackCalc.push(e); }
          else if(*it == "-") { e.value = Operand1.value - Operand2.value; stackCalc.push(e); }
          else if(*it == "*") { e.value = Operand1.value * Operand2.value; stackCalc.push(e); }
          else if(*it == "/") { e.value = Operand1.value / Operand2.value; stackCalc.push(e); }
          else if(*it == "&") { e.value = Operand1.value & Operand2.value; stackCalc.push(e); }
          else if(*it == "^") { e.value = Operand1.value ^ Operand2.value; stackCalc.push(e); }
          else if(*it == "|") { e.value = Operand1.value | Operand2.value; stackCalc.push(e); }
          else if(*it == ">>") { e.value = Operand1.value >> (int)Operand2.value; stackCalc.push(e); }
          else if(*it == "<<") { e.value = Operand1.value << (int)Operand2.value; stackCalc.push(e); }
        }
      }
      
      *pValue = stackCalc.top().value;
      return stackCalc.size() == 1;
    }

  };
} // namespace clstd

#endif // _CLSTD_MATH_EXPRESSION_PARSER_H_