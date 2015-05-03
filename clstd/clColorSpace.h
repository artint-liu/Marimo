#ifndef _CLSTD_COLOR_SPACE_H_
#define _CLSTD_COLOR_SPACE_H_

#ifndef _CL_STD_CODE_
#error Must be include "clstd.h" first.
#endif // #ifndef _CL_STD_CODE_

struct _float4x4;

namespace clstd
{
  namespace Color
  {
    enum {
      AliceBlue             = 0xF0F8FF, // ����˹��
      AntiqueWhite          = 0xFAEBD7, // �Ŷ���
      Aqua                  = 0x00FFFF, // ǳ��ɫ
      Aquamarine            = 0x7FFFD4, // ����ɫ
      Azure                 = 0xF0FFFF, // ����ɫ
      Beige                 = 0xF5F5DC, // ��ɫ
      Bisque                = 0xFFE4C4, // �ۻ�ɫ
      Black                 = 0x000000, // ��ɫ
      Blanchedalmond        = 0xFFEBCD, // ����ɫ
      Blue                  = 0x0000FF, // ��ɫ
      BlueViolet            = 0x8A2BE2, // ������ɫ
      Brown                 = 0xA52A2A, // ��ɫ
      BurlyWood             = 0xDEB887, // ʵľɫ
      CadetBlue             = 0x5F9EA0, // ����ɫ
      Chartreuse            = 0x7FFF00, // ����ɫ
      Chocolate             = 0xD2691E, // �ɿ���ɫ
      Coral                 = 0xFF7F50, // ɺ��ɫ
      CornflowerBlue        = 0x6495ED, // ����ɫ
      Cornsilk              = 0xFFF8DC, // �׳�ɫ
      Crimson               = 0xDC143C, // �����ɫ
      Cyan                  = 0x00FFFF, // ��ɫ
      DarkBlue              = 0x00008B, // ����ɫ
      DarkCyan              = 0x008B8B, // ����ɫ
      DarkGoldenrod         = 0xB8860B, // �����ɫ
      DarkGray              = 0xA9A9A9, // ����ɫ
      DarkGreen             = 0x006400, // ����ɫ
      DarkGrey              = 0xA9A9A9, // ����ɫ
      DarkKhaki             = 0xBDB76B, // ���ƺ�ɫ
      DarkMagenta           = 0x8B008B, // �����
      DarkOliveGreen        = 0x556B2F, // �������
      DarkOrange            = 0xFF8C00, // ���ۻ�ɫ
      DarkOrchid            = 0x9932CC, // ����ɫ
      DarkRed               = 0x8B0000, // ����ɫ
      DarkSalmon            = 0xE9967A, // ����ɫ
      DarkSeaGreen          = 0x8FBC8F, // ������ɫ
      DarkSlateBlue         = 0x483D8B, // ������ɫ
      DarkSlateGray         = 0x2F4F4F, // ���߻�ɫ
      DarkSlateGrey         = 0x2F4F4F, // ���߻�ɫ
      DarkTurquoise         = 0x00CED1, // ����ʯ��
      DarkViolet            = 0x9400D3, // ��������ɫ
      DeepPink              = 0xFF1493, // ��ۺ�ɫ
      DeepSkyBlue           = 0x00BFFF, // ������ɫ
      DimGray               = 0x696969, // ����ɫ
      DimGrey               = 0x696969, // ����ɫ
      DodgerBlue            = 0x1E90FF, // ����ɫ
      FireBrick             = 0xB22222, // ��שɫ
      FloralWhite           = 0xFFFAF0, // ����ɫ
      ForestGreen           = 0x228B22, // ɭ����
      Fuchsia               = 0xFF00FF, // �Ϻ�ɫ
      Gainsboro             = 0xDCDCDC, // ����ɫ
      GhostWhite            = 0xF8F8FF, // �����
      Gold                  = 0xFFD700, // ��ɫ
      Goldenrod             = 0xDAA520, // ������ɫ
      Gray                  = 0x808080, // ��ɫ
      Green                 = 0x008000, // ��ɫ
      GreenYellow           = 0xADFF2F, // ����ɫ
      Grey                  = 0x808080, // ��ɫ
      Honeydew              = 0xF0FFF0, // ��ɫ
      HotPink               = 0xFF69B4, // �ȷۺ�ɫ
      IndianRed             = 0xCD5C5C, // ӡ�ڰ���
      Indigo                = 0x4B0082, // ����ɫ
      Ivory                 = 0xFFFFF0, // ����ɫ
      Khaki                 = 0xF0E68C, // �ƺ�ɫ
      Lavender              = 0xE6E6FA, // ����ɫ
      LavenderBlush         = 0xFFF0F5, // ���Ϻ�
      LawnGreen             = 0x7CFC00, // ����ɫ
      LemonChiffon          = 0xFFFACD, // ���ʳ�ɫ  
      LightBlue             = 0xADD8E6, // ����ɫ    
      LightCoral            = 0xF08080, // ��ɺ��ɫ  
      LightCyan             = 0xE0FFFF, // ����ɫ    
      LightGoldenrodYellow  = 0xFAFAD2, // �����ɫ  
      LightGray             = 0xD3D3D3, // ����ɫ    
      LightGreen            = 0x90EE90, // ����ɫ    
      LightGrey             = 0xD3D3D3, // ����ɫ    
      LightPink             = 0xFFB6C1, // ���ۺ�ɫ  
      LightSalmon           = 0xFFA07A, // ����ɫ    
      LightSeaGreen         = 0x20B2AA, // ������ɫ  
      LightSkyBlue          = 0x87CEFA, // ������ɫ  
      LightSlateGray        = 0x778899, // ������    
      LightSlateGrey        = 0x778899, // ������    
      LightSteelBlue        = 0xB0C4DE, // ������ɫ  
      LightYellow           = 0xFFFFE0, // ����ɫ      
      Lime                  = 0x00FF00, // ���ɫ      
      LimeGreen             = 0x32CD32, // ����ɫ      
      Linen                 = 0xFAF0E6, // ����ɫ      
      Magenta               = 0xFF00FF, // ����ɫ      
      Maroon                = 0x800000, // ��ɫ        
      MediumAquamarine      = 0x66CDAA, // ����ɫ      
      MediumBlue            = 0x0000CD, // ����ɫ      
      MediumOrchid          = 0xBA55D3, // �з���ɫ    
      MediumPurple          = 0x9370DB, // ����ɫ      
      MediumSeaGreen        = 0x3CB371, // �к���      
      MediumSlateBlue       = 0x7B68EE, // �а���ɫ    
      MediumSpringGreen     = 0x00FA9A, // �д���ɫ    
      MediumTurquoise       = 0x48D1CC, // ���̱�ʯ    
      MediumVioletRed       = 0xC71585, // ��������ɫ   
      MidnightBlue          = 0x191970, // �л���ɫ    
      Mintcream             = 0xF5FFFA, // ����ɫ      
      Mistyrose             = 0xFFE4E1, // ǳõ��ɫ    
      Moccasin              = 0xFFE4B5, // ¹Ƥɫ      
      NavajoWhite           = 0xFFDEAD, // ���߰�      
      Navy                  = 0x000080, // ����ɫ    
      Oldlace               = 0xFDF5E6, // �ϻ�ɫ    
      Olive                 = 0x808000, // ���ɫ    
      Olivedrab             = 0x6B8E23, // ���̺�ɫ  
      Orange                = 0xFFA500, // ��ɫ      
      OrangeRed             = 0xFF4500, // ���ɫ    
      Orchid                = 0xDA70D6, // ����ɫ    
      PaleGoldenrod         = 0xEEE8AA, // ������ɫ  
      PaleGreen             = 0x98FB98, // ����ɫ    
      PaleTurquoise         = 0xAFEEEE, // �Ա�ʯ��  
      PaleVioletRed         = 0xDB7093, // ��������ɫ 
      Papayawhip            = 0xFFEFD5, // ��ľɫ    
      PeachPuff             = 0xFFDAB9, // ��ɫ      
      Peru                  = 0xCD853F, // ��³ɫ    
      Pink                  = 0xFFC0CB, // �ۺ�ɫ    
      Plum                  = 0xDDA0DD, // ����ɫ    
      PowderBlue            = 0xB0E0E6, // ����ɫ    
      Purple                = 0x800080, // ��ɫ      
      Red                   = 0xFF0000, // ��ɫ      
      RosyBrown             = 0xBC8F8F, // ��õ���   
      RoyalBlue             = 0x4169E1, // �ʼ���     
      SaddleBrown           = 0x8B4513, // �غ�ɫ     
      Salmon                = 0xFA8072, // ����ɫ     
      SandyBrown            = 0xF4A460, // ɳ��ɫ     
      SeaGreen              = 0x2E8B57, // ����ɫ     
      SeaShell              = 0xFFF5EE, // ����ɫ     
      Sienna                = 0xA0522D, // ��ɫ      
      Silver                = 0xC0C0C0, // ��ɫ      
      SkyBlue               = 0x87CEEB, // ����ɫ
      SlateBlue             = 0x6A5ACD, // ʯ��ɫ
      SlateGray             = 0x708090, // ��ʯɫ
      SlateGrey             = 0x708090, // ��ʯɫ
      Snow                  = 0xFFFAFA, // ѩ��ɫ
      SpringGreen           = 0x00FF7F, // ����ɫ
      SteelBlue             = 0x4682B4, // ����ɫ
      Tan                   = 0xD2B48C, // ��ɫ      
      Teal                  = 0x008080, // ˮѼɫ     
      Thistle               = 0xD8BFD8, // ��ɫ      
      Tomato                = 0xFF6347, // ������ɫ   
      Turquoise             = 0x40E0D0, // ����ɫ     
      Violet                = 0xEE82EE, // ������ɫ   
      Wheat                 = 0xF5DEB3, // ǳ��ɫ     
      White                 = 0xFFFFFF, // ��ɫ      
      WhiteSmoke            = 0xF5F5F5, // �̰�ɫ     
      Yellow                = 0xFFFF00, // ��ɫ      
    };
  } // namespace color

  namespace Internal
  {
    void BuildRGB2HueMat();
    void BuildHue2RGBMat();
    void InitializeColorSpace();
  } // namespace Internal


  void rgb2hsv(float r, float g, float b, float* h, float* s, float* v);
  void hsv2rgb(float h, float s, float v, float* r, float* g, float* b);
  void rgb2yuv(float r, float g, float b, float* y, float* u, float* v);
  void yuv2rgb(float y, float u, float v, float* r, float* g, float* b);
  float getgray(float r, float g, float b);
  void GetHueMagicMat(_float4x4* pMagic);  // rgb to hue
  void GetRGBMagicMat(_float4x4* pMagic);  // hue to rgb

  //struct COLOR_AHSV;
  //struct COLOR_ARGB;
  //struct COLOR_RGBA_F;
  //struct COLOR_ABGR_F;
  struct COLOR_HSVA_F;
  struct COLOR_YUVA_F;
  
  //namespace Internal
  //{
  //  struct COLOR_RGBA_F;
  //}

  namespace Internal
  {
    struct COLOR_ARGB
    {
      union {
        struct {
          u8  b, g, r, a;
        };
        u32 argb;
        u32 data;
        u8  m[4];
      };
    };

    struct COLOR_ABGR
    {
      union {
        struct {
          u8 r, g, b, a;
        };
        u32 abgr;
        u32 data;
        u8  m[4];
      };
    };

    struct COLOR_RGBA_F
    {
      float r, g, b, a;
    };

    struct COLOR_ABGR_F
    {
      float a, b, g, r;
    };

    template<class _TDst, class _TSrc> 
    inline void _Assign(_TDst& dst, const _TSrc& src)
    {
      dst.r = src.r;
      dst.g = src.g;
      dst.b = src.b;
      dst.a = src.a;
    }
    template<class _TDst, class _TSrc> 
    inline void _UnpackColorValue(_TDst& dst, const _TSrc& src)
    {
      const float fInverseFactor = 1.0f / 255.0f;
      dst.a = (float)(src.a & 0xff) * fInverseFactor;
      dst.r = (float)(src.r & 0xff) * fInverseFactor;
      dst.g = (float)(src.g & 0xff) * fInverseFactor;
      dst.b = (float)(src.b & 0xff) * fInverseFactor;
    }
    template<class _TDst, class _TSrc> 
    inline void _PackColorValue(_TDst& dst, const _TSrc& src)
    {
      dst.r = src.r < 0 ? 0 : (src.r > 1.0f ? 0xff : (u8)(src.r * 255.0f + 0.5f));
      dst.g = src.g < 0 ? 0 : (src.g > 1.0f ? 0xff : (u8)(src.g * 255.0f + 0.5f));
      dst.b = src.b < 0 ? 0 : (src.b > 1.0f ? 0xff : (u8)(src.b * 255.0f + 0.5f));
      dst.a = src.a < 0 ? 0 : (src.a > 1.0f ? 0xff : (u8)(src.a * 255.0f + 0.5f));
    }

    //template<class _Ty>
    //inline void _MulF(_Ty& dst, const _Ty& src, const _Ty& val)
    //{
    //  dst.r   src.r * val.r;
    //  dst.g   src.g * val.g;
    //  dst.b   src.b * val.b;
    //  dst.a   src.a * val.a;
    //}

  } // namespace Internal


  template<class _TDataDef> struct _COLOR_RGBTEMPL : public _TDataDef
  {
    _COLOR_RGBTEMPL(){}

    _COLOR_RGBTEMPL(const Internal::COLOR_ABGR& abgr)
    {
      Internal::_Assign(*this, abgr);
    }
    
    _COLOR_RGBTEMPL(const Internal::COLOR_ARGB& argb)
    {
      Internal::_Assign(*this, argb)
    }

    _COLOR_RGBTEMPL(const Internal::COLOR_ABGR_F& clr)
    {
      Internal::_PackColorValue(*this, clr);
    }

    _COLOR_RGBTEMPL(const Internal::COLOR_RGBA_F& clr)
    {
      Internal::_PackColorValue(*this, clr);
    }

    _COLOR_RGBTEMPL& set(const Internal::COLOR_ARGB& argb)
    {
      Internal::_Assign(*this, argb);
      return *this;
    }

    _COLOR_RGBTEMPL& set(u8 a, u8 r, u8 g, u8 b)
    {
      this->a = a;
      this->r = r;
      this->g = g;
      this->b = b;
      return *this;
    }

    _COLOR_RGBTEMPL& set(u32 aarrggbb)
    {
      a = (aarrggbb >> 24) & 0xff;
      r = (aarrggbb >> 16) & 0xff;
      g = (aarrggbb >> 8) & 0xff;
      b = aarrggbb & 0xff;
      return *this;
    }

    u32 RandomizeRGB()  // �������һ����ɫ
    {
      r = clrand() & 0xff;
      g = clrand() & 0xff;
      b = clrand() & 0xff;
      return data;
    }

    u32 RandomizeAll()  // �������һ����ɫ��Alpha
    {
      RandomizeRGB();
      a = clrand() & 0xff;
      return data;
    }

    operator u32()
    {
      return data;
    }
  };

  template<class _TDataDef> struct _COLOR_RGBTEMPL_F : public _TDataDef
  {
    _COLOR_RGBTEMPL_F()
    {
      r = g = b = a = 1;
    }
    _COLOR_RGBTEMPL_F(u32 aarrggbb)
    {
      set(aarrggbb);
    }

    // ��������������ΪSet����
    _COLOR_RGBTEMPL_F(const Internal::COLOR_ARGB& value)
    {
      Internal::_UnpackColorValue(*this, value);
    }

    _COLOR_RGBTEMPL_F(const Internal::COLOR_RGBA_F& value)
    {
      Internal::_Assign(*this, value);
    }

    _COLOR_RGBTEMPL_F(const Internal::COLOR_ABGR_F& value)
    {
      Internal::_Assign(*this, value);
    }

    _COLOR_RGBTEMPL_F(const COLOR_HSVA_F& hsv)
    {
      hsv2rgb(hsv.h, hsv.s, hsv.v, &r, &g, &b);
      a = hsv.a;
    }

    _COLOR_RGBTEMPL_F(const COLOR_YUVA_F& yuv)
    {
      yuv2rgb(yuv.y, yuv.u, yuv.v, &r, &g, &b);
      a = yuv.a;
    }

    _COLOR_RGBTEMPL_F(float r, float g, float b, float a)
    {
      this->r = r;
      this->g = g;
      this->b = b;
      this->a = a;
    }

    _COLOR_RGBTEMPL_F operator+(const _COLOR_RGBTEMPL_F& rgb) const
    {
      return _COLOR_RGBTEMPL_F(r + rgb.r, g + rgb.g, b + rgb.b, a + rgb.a);
    }

    _COLOR_RGBTEMPL_F operator+(float v) const
    {
      return _COLOR_RGBTEMPL_F(r + v, g + v, b + v, a + v);
    }

    _COLOR_RGBTEMPL_F operator-(const _COLOR_RGBTEMPL_F& rgb) const
    {
      return _COLOR_RGBTEMPL_F(r - rgb.r, g - rgb.g, b - rgb.b, a - rgb.a);
    }

    _COLOR_RGBTEMPL_F operator-(float v) const
    {
      return _COLOR_RGBTEMPL_F(r - v, g - v, b - v, a - v);
    }

    _COLOR_RGBTEMPL_F operator*(float v) const
    {
      return _COLOR_RGBTEMPL_F(r * v, g * v, b * v, a * v);
    }

    _COLOR_RGBTEMPL_F operator/(float v) const
    {
      const float fInverseValue = 1.0f / v;
      return _COLOR_RGBTEMPL_F(r * fInverseValue, g * fInverseValue, b * fInverseValue, a * fInverseValue);
    }
    
    /////

    _COLOR_RGBTEMPL_F& operator+=(const _COLOR_RGBTEMPL_F& rgb)
    {
      r += rgb.r;
      g += rgb.g;
      b += rgb.b;
      a += rgb.a;
      return *this;
    }

    _COLOR_RGBTEMPL_F& operator+=(float v)
    {
      r += v;
      g += v;
      b += v;
      a += v;
      return *this;
    }

    _COLOR_RGBTEMPL_F& operator-=(const _COLOR_RGBTEMPL_F& rgb)
    {      
      r -= rgb.r;
      g -= rgb.g;
      b -= rgb.b;
      a -= rgb.a;
      return *this;
    }

    _COLOR_RGBTEMPL_F& operator-=(float v)
    {
      r -= v;
      g -= v;
      b -= v;
      a -= v;
      return *this;
    }

    _COLOR_RGBTEMPL_F& operator*=(float v)
    {
      r *= v;
      g *= v;
      b *= v;
      a *= v;
      return *this;
    }

    _COLOR_RGBTEMPL_F& operator/=(float v)
    {
      const float fInverseValue = 1.0f / v;
      r *= fInverseValue;
      g *= fInverseValue;
      b *= fInverseValue;
      a *= fInverseValue;
      return *this;
    }

    b32 operator==(const _COLOR_RGBTEMPL_F& rgb) CLCONST
    {
      return (r == rgb.r && g == rgb.g && b == rgb.b && a == rgb.a);
    }

    b32 operator!=(const _COLOR_RGBTEMPL_F& rgb) CLCONST
    {
      return (r != rgb.r || g != rgb.g || b != rgb.b || a != rgb.a);
    }

    //operator Internal::COLOR_ARGB () const
    //{
    //  Internal::COLOR_ARGB t;
    //  t.r = r < 0 ? 0 : (r > 1.0f ? 0xff : (u8)(r * 255.0f + 0.5f));
    //  t.g = g < 0 ? 0 : (g > 1.0f ? 0xff : (u8)(g * 255.0f + 0.5f));
    //  t.b = b < 0 ? 0 : (b > 1.0f ? 0xff : (u8)(b * 255.0f + 0.5f));
    //  t.a = a < 0 ? 0 : (a > 1.0f ? 0xff : (u8)(a * 255.0f + 0.5f));
    //  return t;
    //}
    _COLOR_RGBTEMPL_F& set(float r, float g, float b, float a)
    {
      this->r = r;
      this->g = g;
      this->b = b;
      this->a = a;
      return *this;
    }

    _COLOR_RGBTEMPL_F& set(u32 aarrggbb)
    {
      const float fInverseFactor = 1.0f / 255.0f;
      a = (float)((aarrggbb >> 24) & 0xff) * fInverseFactor;
      r = (float)((aarrggbb >> 16) & 0xff) * fInverseFactor;
      g = (float)((aarrggbb >> 8) & 0xff) * fInverseFactor;
      b = (float)(aarrggbb & 0xff) * fInverseFactor;
      return *this;
    }

    u32 ARGB()
    {
      Internal::COLOR_ARGB ret;
      Internal::_PackColorValue(ret, *this);
      return ret.data;
    }

    u32 ABGR()
    {
      Internal::COLOR_ABGR ret;
      Internal::_PackColorValue(ret, *this);
      return ret.data;
    }

    float GetGray() const
    {
      return getgray(r, g, b);
    }
  };

  typedef _COLOR_RGBTEMPL_F<Internal::COLOR_RGBA_F> COLOR_RGBA_F;
  typedef _COLOR_RGBTEMPL_F<Internal::COLOR_ABGR_F> COLOR_ABGR_F;

  typedef _COLOR_RGBTEMPL<Internal::COLOR_ARGB> COLOR_ARGB;
  typedef _COLOR_RGBTEMPL<Internal::COLOR_ABGR> COLOR_ABGR;

  // ����������ļ��ɷ���, ������Ҫ����������ʵ�ֹ��캯��
  //class COLOR_RGBA_F : public _COLOR_RGBTEMPL_F<Internal::COLOR_RGBA_F>
  //{
  //};

  //class COLOR_ABGR_F : public _COLOR_RGBTEMPL_F<Internal::COLOR_ABGR_F>
  //{
  //};

  //struct COLOR_ARGB : _COLOR_RGBTEMPL_F<Internal::COLOR_ARGB>
  //{
  //};

  //struct COLOR_ABGR : _COLOR_RGBTEMPL_F<Internal::COLOR_ABGR>
  //{
  //};
  struct COLOR_AHSV
  {
    u8 v, s, h, a;
  };

  struct COLOR_AYUV
  {
    u8 v, u, y, a;
  };

  struct COLOR_HSVA_F
  {
    COLOR_HSVA_F();
    COLOR_HSVA_F(const COLOR_RGBA_F& rgb);
    COLOR_HSVA_F(const COLOR_ABGR_F& rgb);
    COLOR_HSVA_F(u32 aarrggbb);

    float h, s, v, a;
  };


  //typedef _COLOR_RGBTEMPL_F<Internal::COLOR_RGBA_F> COLOR_RGBA_F;

  //struct COLOR_RGBA_F : public _COLOR_RGBBASE_F<Internal::COLOR_RGBA_F>
  //{
  //  COLOR_RGBA_F();
  //  COLOR_RGBA_F(u32 packedClr);  // 0xaarrggbb
  //  COLOR_RGBA_F(float r, float g, float b, float a);
  //  COLOR_RGBA_F operator*(float value) const;
  //  COLOR_RGBA_F operator/(float value) const;
  //  COLOR_RGBA_F operator-(const COLOR_RGBA_F& rgb) const;

  //  operator COLOR_ARGB () const;

  //  float GetGray() const;
  //  float r, g, b, a;
  //};

  //struct COLOR_ABGR_F
  //{
  //  COLOR_ABGR_F(const COLOR_HSVA_F& hsv);

  //  operator COLOR_ARGB () const;

  //  float GetGray() const;

  //  float a, b, g, r;
  //};

  struct COLOR_YUVA_F
  {
    float GetGray() const;

    float y, u, v, a;
  };
 
} // namespace clstd

#endif // _CLSTD_COLOR_SPACE_H_