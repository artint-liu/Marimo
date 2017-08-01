#ifndef _STRING_FORMATTED_H_
#define _STRING_FORMATTED_H_

namespace clstd
{
  namespace StringCommon
  {
    //////////////////////////////////////////////////////////////////////////

    template<class _TStr>
    class StringFormattedT : public _TStr
    {
    public:
      typedef typename _TStr::TChar    TChar;
      typedef const TChar*             LPCSTR;
      const static int MAX_DIGITS = 80;
      const static int nDefaultDoublePrecision = 6;

    public:
      StringFormattedT& VarFormat(LPCSTR pFmt, va_list arglist)  // ��ԭʼ���ݺ��棬ʹ�ñ���б�׷�Ӹ�ʽ���ַ���
      {
        LPCSTR ptr = pFmt;
        TChar  buffer[MAX_DIGITS + 1];  // ������Ϊ����ת���Ļ�����,����32λ�����͸�����,ת��Ϊ�ַ����󳤶ȶ�������80
        int    i;


        while(*ptr != '\0')
        {
          LPCSTR ptr2 = clstd::strchrT(ptr, '%');
          if(ptr2 == NULL)
          {
            _TStr::Append(ptr);
            break;
          }
          else
          {
            // %[flags][width][.precision][length]specifier
            int nWidth = 0;
            int nPrecision = 0;
            int nLong = 0;
            b32 bLeftAlign = FALSE;  // '-' �����
            b32 bZeroPrefix = FALSE; // '0' ����λ��0���, ��'-'���ȼ���
            b32 bForceSign = FALSE;  // '+' ǿ����ʾ����
            b32 bSpace = FALSE;      // ' ' ����λռλ����'+'ͬʱ����ʱ��'+'���ȼ���
            b32 bPound = FALSE;      // '#' ��ʾ�˽��ƻ���ʮ������ǰ׺
            b32 bPrecision = FALSE;  // ����'.'֮��ΪTrue
            _TStr::Append(ptr, ptr2 - ptr);
            ptr = ptr2 + 1;
          SEQUENCE:
            switch(*ptr)
            {
            case '\0':
              goto FUNC_RET;
            case '%':
              _TStr::Append((TChar)'%');
              break;
            case 'l':
              ptr++;
              if(nLong < 2) {
                nLong++;
                goto SEQUENCE;
              }
              break;
            case 's':
              _TStr::Append((TChar*)va_arg(arglist, TChar*), bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
              break;
            case 'c':
              _TStr::Append((TChar)va_arg(arglist, int/*TChar*/));
              break;

            case 'd':
            case 'i':
              if(nLong == 2) { // long long(64bits)
                i64 va_value = va_arg(arglist, i64);

                if(va_value >= 0 && bForceSign) {
                  buffer[0] = '+';
                  //MyTraits::Integer64ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
                  clstd::l64tox(va_value, buffer + 1, MAX_DIGITS - 1, 10, 0);
                }
                else if(va_value >= 0 && bSpace) {
                  buffer[0] = 0x20;
                  //MyTraits::Integer64ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
                  clstd::l64tox(va_value, buffer + 1, MAX_DIGITS - 1, 10, 0);
                }
                else {
                  //MyTraits::Integer64ToString(buffer, MAX_DIGITS, va_value, 0);
                  clstd::l64tox(va_value, buffer, MAX_DIGITS, 10, 0);
                }

              }
              else { // int(32bits)
                int va_value = va_arg(arglist, int);

                if(va_value >= 0 && bForceSign) {
                  buffer[0] = '+';
                  //MyTraits::Integer32ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
                  clstd::ltox(va_value, buffer + 1, MAX_DIGITS - 1, 10, 0);
                }
                else if(va_value >= 0 && bSpace) {
                  buffer[0] = 0x20;
                  //MyTraits::Integer32ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
                  clstd::ltox(va_value, buffer + 1, MAX_DIGITS - 1, 10, 0);
                }
                else {
                  //MyTraits::Integer32ToString(buffer, MAX_DIGITS, va_value, 0);
                  clstd::ltox(va_value, buffer, MAX_DIGITS, 10, 0);
                }
              }

              ASSERT(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20 ||
                (buffer[0] >= '0' && buffer[0] <= '9'));

              if(bPrecision)
              {
                // "%5.3d", 1  = "  001"
                // "%3.5d", 1  = "00001"
                // "%5.3d", -1 = " -001"
                // "%3.5d", -1 = "-00001"

                if(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20) {
                  this->_AppendFormat(buffer, 1, buffer + 1, nWidth, nPrecision);
                }
                else {
                  this->_AppendFormat(buffer, nWidth, nPrecision);
                }
              }
              else
              {
                if(bZeroPrefix && nWidth > 0)
                {
                  if(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20) {
                    _TStr::Append(buffer[0]);
                    _TStr::Append(buffer + 1, '0', nWidth - 1);
                  }
                  else {
                    _TStr::Append(buffer, '0', nWidth);
                  }
                }
                else
                {
                  _TStr::Append(buffer, ' ', nWidth);
                }
              }
              break;

            case 'o':
            {
              unsigned long va_value = va_arg(arglist, unsigned long);
              if(bPound && va_value) {
                buffer[0] = '0'; // ����ǰ׺
                //MyTraits::OctalToString(buffer + 1, MAX_DIGITS - 1, va_value);
                clstd::ultox(va_value, buffer + 1, MAX_DIGITS - 1, 8);
              }
              else {
                //MyTraits::OctalToString(buffer, MAX_DIGITS, va_value);
                clstd::ultox(va_value, buffer, MAX_DIGITS, 8);
              }

              if(bPrecision) {
                _AppendFormat(buffer, nWidth, nPrecision);
              }
              else {
                _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
              }
              break;
            }
            case 'u':
            {
              if(nLong == 2) {
                //MyTraits::Unsigned64ToString(buffer, MAX_DIGITS, va_arg(arglist, u64), 0);
                clstd::ul64tox(va_arg(arglist, u64), buffer, MAX_DIGITS, 10);
              }
              else {
                //MyTraits::Unsigned32ToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long), 0);
                clstd::ultox(va_arg(arglist, unsigned long), buffer, MAX_DIGITS, 10);
              }

              if(bPrecision) {
                _AppendFormat(buffer, nWidth, nPrecision);
              }
              else {
                _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
              }
              break;
            }

            case 'f':
            case 'F':
            {
              double va_value = va_arg(arglist, double);
              int avail = 0;
              //if(nPrecision)
              {
                if( ! bPrecision) {
                  nPrecision = nDefaultDoublePrecision;
                }

                if(bForceSign && *(i64*)&va_value >= 0) {
                  buffer[0] = '+';
                  //avail = 1 + MyTraits::FloatToString(buffer + 1, MAX_DIGITS - 1, nPrecision, (float)va_value, 'F');
                  avail = 1 + clstd::ftox((float)va_value, buffer + 1, MAX_DIGITS - 1, nPrecision, 'F');
                }
                else if(bSpace && *(i64*)&va_value >= 0) {
                  buffer[0] = 0x20;
                  //avail = 1 + MyTraits::FloatToString(buffer + 1, MAX_DIGITS - 1, nPrecision, (float)va_value, 'F');
                  avail = 1 + clstd::ftox((float)va_value, buffer + 1, MAX_DIGITS - 1, nPrecision, 'F');
                }
                else {
                  //avail = MyTraits::FloatToString(buffer, MAX_DIGITS, nPrecision, (float)va_value, 'F');
                  avail = clstd::ftox((float)va_value, buffer, MAX_DIGITS, nPrecision, 'F');
                }

                //const _TCh* pDot = MyTraits::StringSearchChar(buffer, '.');
                //if(pDot != NULL) {
                //  int nn = nPrecision + 1; // ����'.'�ĸ���
                //  while(nn-- && *++pDot != '\0'); // û�����Ƿֺţ�
                //  *(_TCh*)pDot = '\0';
                //}
              }

              if(!bPound && buffer[avail - 1] == '.') {
                buffer[avail - 1] = '\0';
              }
              //else
              //{
              //  if(bForceSign && va_value >= 0) {
              //    buffer[0] = '+';
              //    MyTraits::FloatToString(buffer + 1, MAX_DIGITS - 1, 0, (float)va_value, 'F');
              //  }
              //  else {
              //    MyTraits::FloatToString(buffer, MAX_DIGITS, 0, (float)va_value, 'F');
              //  }
              //}
              _TStr::Append(buffer);
              break;
            }

            case 'b':
              //MyTraits::BinaryToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));
              clstd::ultox(va_arg(arglist, unsigned long), buffer, MAX_DIGITS, 2);
              _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
              break;

            case 'X':
            case 'x':
            {
              unsigned long va_value = va_arg(arglist, unsigned long);

              if(va_value)
              {
                buffer[0] = '0'; buffer[1] = *ptr;
                if(*ptr == 'X') {
                  //MyTraits::HexToUpperString(buffer + 2, MAX_DIGITS - 2, va_value);
                  clstd::ultox(va_value, buffer + 2, MAX_DIGITS - 2, 16, 1);
                }
                else {
                  //MyTraits::HexToLowerString(buffer + 2, MAX_DIGITS - 2, va_value);
                  clstd::ultox(va_value, buffer + 2, MAX_DIGITS - 2, 16);
                }

                if(bPrecision)
                {
                  if(bPound) {
                    _AppendFormat(buffer, 2, buffer + 2, nWidth, nPrecision);
                  }
                  else {
                    _AppendFormat(buffer + 2, nWidth, nPrecision);
                  }
                }
                else if(bPound && bZeroPrefix)
                {
                  _TStr::Append(buffer, 2);

                  // ��Ϊǰ�����ǰ׺������趨�˿�ȣ�����Ҫ�ѿ�����������ַ�
                  if(nWidth > 2) {
                    _TStr::Append(buffer + 2, '0', nWidth - 2);
                  }
                  else if(nWidth < -2) {
                    _TStr::Append(buffer + 2, 0x20, nWidth + 2);
                  }
                  else {
                    _TStr::Append(buffer + 2);
                  }
                }
                else if(bPound)
                {
                  _TStr::Append(buffer, 0x20, nWidth);
                }
                else if(bZeroPrefix)
                {
                  _TStr::Append(buffer + 2, (nWidth > 0 ? '0' : 0x20), nWidth);
                }
                else {
                  _TStr::Append(buffer + 2, 0x20, nWidth);
                }
              }
              else
              {
                if(bPrecision)
                {
                  if(nWidth > nPrecision) {
                    _TStr::Append((TChar)0x20, nWidth - nPrecision);
                  }

                  _TStr::Append('0', nPrecision);

                  if(-nWidth > nPrecision) {
                    _TStr::Append((TChar)0x20, -nWidth - nPrecision);
                  }
                }
                else
                {
                  // ���� bZeroPerfix
                  if(nWidth) {
                    buffer[0] = '0'; buffer[1] = '\0';
                    _TStr::Append(buffer, (bZeroPrefix && nWidth > 0 ? '0' : (TChar)0x20), nWidth);
                  }
                  else {
                    _TStr::Append('0');
                  }
                }
              }
              break;
            }

            case '*':
              if(bPrecision) {
                nPrecision = (int)va_arg(arglist, int);
              }
              else {
                nWidth = (int)va_arg(arglist, int);
              }
              ptr++;
              goto SEQUENCE;

            case '0':
              bZeroPrefix = TRUE;
              ptr++;
              goto SEQUENCE;

            case '+':
              bForceSign = TRUE;
              ptr++;
              goto SEQUENCE;

            case '-':
              bLeftAlign = TRUE;
              ptr++;
              goto SEQUENCE;

            case '#':
              bPound = TRUE;
              ptr++;
              goto SEQUENCE;

            case 0x20: // space
              bSpace = TRUE;
              ptr++;
              goto SEQUENCE;

            case '.':  // "%.3f"
              bPrecision = TRUE;
              ptr++;
              goto SEQUENCE;

            default:
              if(*ptr >= '0' && *ptr <= '9')  // "%8d"
              {
                i = 0;
                while(1)
                {
                  if(*ptr >= '0' && *ptr <= '9') {
                    buffer[i++] = *ptr;
                  }
                  else if(*ptr == '\0') {
                    goto FUNC_RET;
                  }
                  else if(i >= sizeof(buffer)) {
                    break;
                  }
                  else if(*ptr == 'd' || *ptr == 'i' || *ptr == 'u' || *ptr == 'o' || *ptr == 'X' || *ptr == 'x' || *ptr == '.' || *ptr == 'f')
                  {
                    buffer[i] = '\0';

                    if(bPrecision) {
                      //nPrecision = MyTraits::StringToInteger32(buffer);
                      nPrecision = clstd::xtoi(buffer);
                    }
                    else {
                      //nWidth = MyTraits::StringToInteger32(buffer);
                      nWidth = clstd::xtoi(buffer);
                      if(bLeftAlign) {
                        nWidth = -nWidth;
                      }
                    }
                    goto SEQUENCE;
                  }
                  else
                    break;
                  ptr++;
                }
              } // if
              break;
            } // switch
          }
          ptr++;
        }

      FUNC_RET:
        va_end(arglist);
        return *this;
      }

      //////////////////////////////////////////////////////////////////////////

      void _AppendSpace(int len, int nPrefixLen, int nWidth, int nPrecision)
      {
        if(nWidth > nPrecision)
        {
          if(nPrecision > len) {
            if(nWidth - nPrecision > nPrefixLen) {
              _TStr::Append(0x20, nWidth - nPrecision - nPrefixLen);
            }
          }
          else if(nWidth > len) {
            if(nWidth - len > nPrefixLen) {
              _TStr::Append(0x20, nWidth - len - nPrefixLen);
            }
          }
        }
      }

      //************************************
      // Method:    _AppendFormat
      // Qualifier:
      // Parameter: szPrefix      ǰ׺���ţ�����'+'��'-'��'0x'
      // Parameter: nPrefixLen    ǰ׺����
      // Parameter: szNumeric     �����ַ�
      // Parameter: nWidth        ��ֵ�����Ҷ��룬��ֵ�����
      // Parameter: nPrecision
      //************************************
      void _AppendFormat(LPCSTR szPrefix, int nPrefixLen, LPCSTR szNumeric, int nWidth, int nPrecision)
      {
        int len = (int)strlenT(szNumeric);
        _AppendSpace(len, nPrefixLen, nWidth, nPrecision);
        _TStr::Append(szPrefix, nPrefixLen);
        _TStr::Append(szNumeric, '0', nPrecision);
        _AppendSpace(len, nPrefixLen, -nWidth, nPrecision);
      }

      //////////////////////////////////////////////////////////////////////////

      void _AppendSpace(int len, int nWidth, int nPrecision)
      {
        if(nWidth > nPrecision)
        {
          if(nPrecision > len) {
            _TStr::Append(0x20, nWidth - nPrecision);
          }
          else if(nWidth > len) {
            _TStr::Append(0x20, nWidth - len);
          }
        }
      }

      void _AppendFormat(LPCSTR szNumeric, int nWidth, int nPrecision)
      {
        int len = (int)strlenT(szNumeric);

        _AppendSpace(len, nWidth, nPrecision);
        _TStr::Append(szNumeric, '0', nPrecision);
        _AppendSpace(len, -nWidth, nPrecision);
      }
    };


    //////////////////////////////////////////////////////////////////////////

    template<class _TStrUtf8>
    _TStrUtf8& ConvertToUtf8T(_TStrUtf8& strUtf8, const wch* szUnicode, size_t nUnicode)
    {
      strUtf8.Clear();
      for(size_t i = 0; i < nUnicode; i++)
      {
        u16 c = szUnicode[i];
        if(c <= 0x7F) {
          strUtf8.Append((typename _TStrUtf8::TChar)c);
        }
        else if(c < 0x7FF) {
          strUtf8.Append((typename _TStrUtf8::TChar)0xC0 | (0x1F & (c >> 6)));
          strUtf8.Append((typename _TStrUtf8::TChar)0x80 | (0x3F & (c)));
        }
        else {
          strUtf8.Append((typename _TStrUtf8::TChar)0xE0 | (0x0F & (c >> 12)));
          strUtf8.Append((typename _TStrUtf8::TChar)0x80 | (0x3F & (c >> 6)));
          strUtf8.Append((typename _TStrUtf8::TChar)0x80 | (0x3F & (c)));
        }
      }
      return strUtf8;
    }

    //////////////////////////////////////////////////////////////////////////

    template<class _TStrUnicode>
    _TStrUnicode& ConvertFromUtf8(_TStrUnicode& strUnicode, const ch* szUtf8, size_t cbUtf8)
    {
      strUnicode.Clear();

      for(size_t i = 0; i < cbUtf8; i++)
      {
        u8 c = szUtf8[i];

        if(c & 0x80)
        {
          if(c & 0x40)
          {
            if(++i >= cbUtf8) {
              break;
            }
            u8 c1 = szUtf8[i];
            if((c1 & 0xC0) != 0x80) {
              break;
            }

            if(c & 0x20)
            {
              if(++i >= cbUtf8) {
                break;
              }
              u8 c2 = szUtf8[i];
              if((c2 & 0xC0) != 0x80) {
                break;
              }

              if(c & 0x10)
              {
                // 1111xxxx ... ��֧��
                break;
              }
              else
              {
                // 1110xxxx 10xxxxxx 10xxxxxx
                strUnicode.Append((typename _TStrUnicode::TChar)
                  (((u16)c & 0x0F) << 12) |
                  (((u16)c1 & 0x3F) << 6) |
                  ((u16)c2 & 0x3F)
                );
              }
            }
            else
            {
              // 110xxxxx 10xxxxxx
              strUnicode.Append((typename _TStrUnicode::TChar)((((u16)c & 0x1F) << 6) | ((u16)c1 & 0x3F)));
            }
          }
          else {
            // "1000-0000" code error
            break;
          }
        }
        else {
          // 0xxxxxxx
          strUnicode.Append((typename _TStrUnicode::TChar)c);
        }
      }

      return strUnicode;
    }

    //////////////////////////////////////////////////////////////////////////
  } // namespace StringCommon
} // namespace clstd

#endif // _STRING_FORMATTED_H_