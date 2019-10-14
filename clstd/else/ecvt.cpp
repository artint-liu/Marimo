// 代码主要来自 2.11BSD, 做了模板化的修改
/*
 *  ecvt converts to decimal
 *  the number of digits is specified by ndigit
 *  decpt is set to the position of the decimal point
 *  sign is set to 0 for positive, 1 for negative
 */
#include <math.h>
#include "../clstd.h"
//#define  NDIG  80

//char  *cvt();
namespace clstd
{
  template<typename _TCh, int _NDIG>
  static _TCh* cvt(_TCh* buf, double arg, int ndigits, int *decpt, int *sign, int eflag);

  template<typename _TCh, int _NDIG>
  _TCh* ecvt(_TCh* buf, double arg, int ndigits, int *decpt, int *sign)
  {
    return(cvt<_TCh, _NDIG>(buf, arg, ndigits, decpt, sign, 1));
  }

  template<typename _TCh, int _NDIG>
  _TCh* fcvt(_TCh* buf, double arg, int ndigits, int *decpt, int *sign)
  {
    return(cvt<_TCh, _NDIG>(buf, arg, ndigits, decpt, sign, 0));
  }

  template<typename _TCh, int _NDIG> // buf[_NDIG]
  static _TCh*
    cvt(_TCh* buf, double arg, int ndigits, int *decpt, int *sign, int eflag)
  {
    int r2;
    double fi, fj;
    _TCh *p, *p1;
    //static _TCh buf[NDIG];
    //double modf();
    STATIC_ASSERT(_NDIG >= 80);

    if(ndigits < 0) {
      ndigits = 0;
    }

    if(ndigits >= _NDIG - 1) {
      ndigits = _NDIG - 2;
    }

    r2 = 0;
    *sign = 0;
    p = &buf[0];
    if(*(i64*)&arg < 0) { // -0.0 情况下 sign=1
      *sign = 1;
      arg = -arg;
    }
    arg = modf((double)arg, (double*)&fi);
    p1 = &buf[_NDIG];
    /*
     * Do integer part
     */
    if(fi != 0) {
      p1 = &buf[_NDIG];
      while(p1 >= buf && fi != 0) {
        fj = modf(fi / 10.0, &fi);
        *--p1 = (int)((fj + .03) * 10) + '0';
        r2++;
      }
      while(p1 < &buf[_NDIG])
        *p++ = *p1++;
    }
    else if(arg > 0) {
      while((fj = arg * 10) < 1) {
        arg = fj;
        r2--;
      }
    }
    p1 = &buf[ndigits];
    if(eflag == 0)
      p1 += r2;
    *decpt = r2;
    if(p1 < &buf[0]) {
      buf[0] = '\0';
      return(buf);
    }
    while(p <= p1 && p < &buf[_NDIG]) {
      arg *= 10;
      arg = modf(arg, &fj);
      *p++ = (int)fj + '0';
    }
    if(p1 >= &buf[_NDIG]) {
      buf[_NDIG - 1] = '\0';
      return(buf);
    }
    p = p1;
    *p1 += 5;
    while(*p1 > '9') {
      *p1 = '0';
      if(p1 > buf)
        ++*--p1;
      else {
        *p1 = '1';
        (*decpt)++;
        if(eflag == 0) {
          if(p > buf)
            *p = '0';
          p++;
        }
      }
    }
    *p = '\0';
    return(buf);
  }

  template wch* fcvt<wch, 310>(wch* buf, double arg, int ndigits, int *decpt, int *sign);
  template  ch* fcvt< ch, 310>(ch* buf, double arg, int ndigits, int *decpt, int *sign);
  template wch* ecvt<wch, 310>(wch* buf, double arg, int ndigits, int *decpt, int *sign);
  template  ch* ecvt< ch, 310>(ch* buf, double arg, int ndigits, int *decpt, int *sign);
} // namespace clstd
