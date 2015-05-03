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
      AliceBlue             = 0xF0F8FF, // 艾利斯兰
      AntiqueWhite          = 0xFAEBD7, // 古董白
      Aqua                  = 0x00FFFF, // 浅绿色
      Aquamarine            = 0x7FFFD4, // 碧绿色
      Azure                 = 0xF0FFFF, // 天蓝色
      Beige                 = 0xF5F5DC, // 米色
      Bisque                = 0xFFE4C4, // 桔黄色
      Black                 = 0x000000, // 黑色
      Blanchedalmond        = 0xFFEBCD, // 白杏色
      Blue                  = 0x0000FF, // 蓝色
      BlueViolet            = 0x8A2BE2, // 紫罗兰色
      Brown                 = 0xA52A2A, // 褐色
      BurlyWood             = 0xDEB887, // 实木色
      CadetBlue             = 0x5F9EA0, // 军兰色
      Chartreuse            = 0x7FFF00, // 黄绿色
      Chocolate             = 0xD2691E, // 巧可力色
      Coral                 = 0xFF7F50, // 珊瑚色
      CornflowerBlue        = 0x6495ED, // 菊兰色
      Cornsilk              = 0xFFF8DC, // 米绸色
      Crimson               = 0xDC143C, // 暗深红色
      Cyan                  = 0x00FFFF, // 青色
      DarkBlue              = 0x00008B, // 暗蓝色
      DarkCyan              = 0x008B8B, // 暗青色
      DarkGoldenrod         = 0xB8860B, // 暗金黄色
      DarkGray              = 0xA9A9A9, // 暗灰色
      DarkGreen             = 0x006400, // 暗绿色
      DarkGrey              = 0xA9A9A9, // 暗灰色
      DarkKhaki             = 0xBDB76B, // 暗黄褐色
      DarkMagenta           = 0x8B008B, // 暗洋红
      DarkOliveGreen        = 0x556B2F, // 暗橄榄绿
      DarkOrange            = 0xFF8C00, // 暗桔黄色
      DarkOrchid            = 0x9932CC, // 暗紫色
      DarkRed               = 0x8B0000, // 暗红色
      DarkSalmon            = 0xE9967A, // 暗肉色
      DarkSeaGreen          = 0x8FBC8F, // 暗海兰色
      DarkSlateBlue         = 0x483D8B, // 暗灰蓝色
      DarkSlateGray         = 0x2F4F4F, // 暗瓦灰色
      DarkSlateGrey         = 0x2F4F4F, // 暗瓦灰色
      DarkTurquoise         = 0x00CED1, // 暗宝石绿
      DarkViolet            = 0x9400D3, // 暗紫罗兰色
      DeepPink              = 0xFF1493, // 深粉红色
      DeepSkyBlue           = 0x00BFFF, // 深天蓝色
      DimGray               = 0x696969, // 暗灰色
      DimGrey               = 0x696969, // 暗灰色
      DodgerBlue            = 0x1E90FF, // 闪兰色
      FireBrick             = 0xB22222, // 火砖色
      FloralWhite           = 0xFFFAF0, // 花白色
      ForestGreen           = 0x228B22, // 森林绿
      Fuchsia               = 0xFF00FF, // 紫红色
      Gainsboro             = 0xDCDCDC, // 淡灰色
      GhostWhite            = 0xF8F8FF, // 幽灵白
      Gold                  = 0xFFD700, // 金色
      Goldenrod             = 0xDAA520, // 金麒麟色
      Gray                  = 0x808080, // 灰色
      Green                 = 0x008000, // 绿色
      GreenYellow           = 0xADFF2F, // 黄绿色
      Grey                  = 0x808080, // 灰色
      Honeydew              = 0xF0FFF0, // 蜜色
      HotPink               = 0xFF69B4, // 热粉红色
      IndianRed             = 0xCD5C5C, // 印第安红
      Indigo                = 0x4B0082, // 靛青色
      Ivory                 = 0xFFFFF0, // 象牙色
      Khaki                 = 0xF0E68C, // 黄褐色
      Lavender              = 0xE6E6FA, // 淡紫色
      LavenderBlush         = 0xFFF0F5, // 淡紫红
      LawnGreen             = 0x7CFC00, // 草绿色
      LemonChiffon          = 0xFFFACD, // 柠檬绸色  
      LightBlue             = 0xADD8E6, // 亮蓝色    
      LightCoral            = 0xF08080, // 亮珊瑚色  
      LightCyan             = 0xE0FFFF, // 亮青色    
      LightGoldenrodYellow  = 0xFAFAD2, // 亮金黄色  
      LightGray             = 0xD3D3D3, // 亮灰色    
      LightGreen            = 0x90EE90, // 亮绿色    
      LightGrey             = 0xD3D3D3, // 亮灰色    
      LightPink             = 0xFFB6C1, // 亮粉红色  
      LightSalmon           = 0xFFA07A, // 亮肉色    
      LightSeaGreen         = 0x20B2AA, // 亮海蓝色  
      LightSkyBlue          = 0x87CEFA, // 亮天蓝色  
      LightSlateGray        = 0x778899, // 亮蓝灰    
      LightSlateGrey        = 0x778899, // 亮蓝灰    
      LightSteelBlue        = 0xB0C4DE, // 亮钢兰色  
      LightYellow           = 0xFFFFE0, // 亮黄色      
      Lime                  = 0x00FF00, // 酸橙色      
      LimeGreen             = 0x32CD32, // 橙绿色      
      Linen                 = 0xFAF0E6, // 亚麻色      
      Magenta               = 0xFF00FF, // 红紫色      
      Maroon                = 0x800000, // 粟色        
      MediumAquamarine      = 0x66CDAA, // 中绿色      
      MediumBlue            = 0x0000CD, // 中兰色      
      MediumOrchid          = 0xBA55D3, // 中粉紫色    
      MediumPurple          = 0x9370DB, // 中紫色      
      MediumSeaGreen        = 0x3CB371, // 中海蓝      
      MediumSlateBlue       = 0x7B68EE, // 中暗蓝色    
      MediumSpringGreen     = 0x00FA9A, // 中春绿色    
      MediumTurquoise       = 0x48D1CC, // 中绿宝石    
      MediumVioletRed       = 0xC71585, // 中紫罗兰色   
      MidnightBlue          = 0x191970, // 中灰兰色    
      Mintcream             = 0xF5FFFA, // 薄荷色      
      Mistyrose             = 0xFFE4E1, // 浅玫瑰色    
      Moccasin              = 0xFFE4B5, // 鹿皮色      
      NavajoWhite           = 0xFFDEAD, // 纳瓦白      
      Navy                  = 0x000080, // 海军色    
      Oldlace               = 0xFDF5E6, // 老花色    
      Olive                 = 0x808000, // 橄榄色    
      Olivedrab             = 0x6B8E23, // 深绿褐色  
      Orange                = 0xFFA500, // 橙色      
      OrangeRed             = 0xFF4500, // 红橙色    
      Orchid                = 0xDA70D6, // 淡紫色    
      PaleGoldenrod         = 0xEEE8AA, // 苍麒麟色  
      PaleGreen             = 0x98FB98, // 苍绿色    
      PaleTurquoise         = 0xAFEEEE, // 苍宝石绿  
      PaleVioletRed         = 0xDB7093, // 苍紫罗兰色 
      Papayawhip            = 0xFFEFD5, // 番木色    
      PeachPuff             = 0xFFDAB9, // 桃色      
      Peru                  = 0xCD853F, // 秘鲁色    
      Pink                  = 0xFFC0CB, // 粉红色    
      Plum                  = 0xDDA0DD, // 洋李色    
      PowderBlue            = 0xB0E0E6, // 粉蓝色    
      Purple                = 0x800080, // 紫色      
      Red                   = 0xFF0000, // 红色      
      RosyBrown             = 0xBC8F8F, // 褐玫瑰红   
      RoyalBlue             = 0x4169E1, // 皇家蓝     
      SaddleBrown           = 0x8B4513, // 重褐色     
      Salmon                = 0xFA8072, // 鲜肉色     
      SandyBrown            = 0xF4A460, // 沙褐色     
      SeaGreen              = 0x2E8B57, // 海绿色     
      SeaShell              = 0xFFF5EE, // 海贝色     
      Sienna                = 0xA0522D, // 赭色      
      Silver                = 0xC0C0C0, // 银色      
      SkyBlue               = 0x87CEEB, // 天蓝色
      SlateBlue             = 0x6A5ACD, // 石蓝色
      SlateGray             = 0x708090, // 灰石色
      SlateGrey             = 0x708090, // 灰石色
      Snow                  = 0xFFFAFA, // 雪白色
      SpringGreen           = 0x00FF7F, // 春绿色
      SteelBlue             = 0x4682B4, // 钢兰色
      Tan                   = 0xD2B48C, // 茶色      
      Teal                  = 0x008080, // 水鸭色     
      Thistle               = 0xD8BFD8, // 蓟色      
      Tomato                = 0xFF6347, // 西红柿色   
      Turquoise             = 0x40E0D0, // 青绿色     
      Violet                = 0xEE82EE, // 紫罗兰色   
      Wheat                 = 0xF5DEB3, // 浅黄色     
      White                 = 0xFFFFFF, // 白色      
      WhiteSmoke            = 0xF5F5F5, // 烟白色     
      Yellow                = 0xFFFF00, // 黄色      
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

    u32 RandomizeRGB()  // 随机产生一个颜色
    {
      r = clrand() & 0xff;
      g = clrand() & 0xff;
      b = clrand() & 0xff;
      return data;
    }

    u32 RandomizeAll()  // 随机产生一个颜色和Alpha
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

    // 如果编译有歧义改为Set函数
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

  // 不能用下面的集成方法, 这样还要重新声明和实现构造函数
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