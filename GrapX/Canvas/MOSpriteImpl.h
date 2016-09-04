#ifndef _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_
#define _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_

class GXImage;
class GXCanvas;
class GXGraphics;

namespace Marimo
{
  struct SPRITE_DESC_LOADER : SPRITE_DESC
  {
    enum {
      HAS_LOADED_RIGHT = 0x0001,
      HAS_LOADED_BOTTOM = 0x0002,
    };
    clStringA               strImageDir;
    clstd::StringSetA       StringSet;
    b32 Load(clstd::StockA* pStock, GXLPCSTR szSection);
    b32 LoadModule(const clstd::StockA::Section& sect);
    b32 LoadFrame(const clstd::StockA::Section& sect);
    b32 LoadAnimation(const clstd::StockA::Section& sect);

    void TranslateFrameUnit();
    void TranslateAnimUnit();

    template <class _TUnit>
    void AddToDict(const _TUnit& desc, const TYPEIDX& ti)
    {
      if (desc.id != 0) {
        sIDDict.insert(clmake_pair(desc.id, ti));
      }
      if (desc.name != NULL && desc.name[0] != '\0') {
        sNameDict.insert(clmake_pair(desc.name, ti));
      }
    }

    template <class _TUnit>
    b32 LoadCommon(_TUnit& dest, const clstd::StockA::ATTRIBUTE& attr)
    {
      if (attr.key == "id") {
        dest.id = attr.ToInt();
      }
      else if (attr.key == "name") {
        dest.name = StringSet.add(attr.ToString());
      }
      else {
        return FALSE;
      }
      return TRUE;
    }

    b32 LoadRegion(GXREGN& regn, const clstd::StockA::ATTRIBUTE& attr, GXDWORD& dwFlag);
  };

  class MOSpriteImpl : public Marimo::Sprite
  {
  public:
    //struct MODULE
    //{
    //  clStringA name;
    //  Regn      regn;
    //};

    //typedef clvector<MODULE>      ModuleArray;
    typedef clmap<clStringA, int>             NameDict;

    typedef clvector<MOSprite::MODULE>       ModuleArray;
    typedef clvector<MOSprite::FRAME>        FrameArray;
    typedef clvector<MOSprite::FRAME_UNIT>   FrameModuleArray;
    typedef clvector<MOSprite::ANIMATION>    AnimationArray;
    typedef clvector<MOSprite::ANIM_UNIT>    AnimFrameArray;
    typedef clstd::StringSetA                clStringSetA;
    typedef clvector<GXImage*>               ImageArray;
  private:
    clStringW           m_strImageFile;
    ImageArray          m_ImageArray;
    SPRITE_DESC_LOADER  m_loader;
    //GXImage*          m_pImage;

    //clStringSetA      m_NameSet; // TODO: 这个将来可以取消，使用压实的一大块内存储存字符串序列

    //ModuleArray       m_aModules;

    //FrameArray        m_aFrames;
    //FrameModuleArray  m_aFrameModules;

    //AnimationArray    m_aAnimations;
    //AnimFrameArray    m_aAnimFrames;

    //NameDict          m_SpriteDict;
    //int IntGetSpriteCount() const;
  protected:
    virtual ~MOSpriteImpl();
  public:
    MOSpriteImpl();
    GXBOOL Initialize(GXGraphics* pGraphics, const SPRITE_DESC_LOADER* pDesc);
    //GXBOOL Initialize(GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount);


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef();
    virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXVOID    PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const override;
    GXVOID    PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const override;
    GXVOID    PaintModule(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const override;

    GXVOID    PaintModule3H(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const override;
    GXVOID    PaintModule3V(GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const override;
    GXVOID    PaintModule3x3(GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawCenter, GXLPCRECT rect) const override;

    GXVOID    PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const override;
    GXVOID    PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const override;
    GXVOID    PaintFrame(GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;

    GXVOID    PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const override;
    GXVOID    PaintAnimationFrame(GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const override;
    GXVOID    PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const override;
    GXVOID    PaintAnimationByTime(GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const override;

    GXVOID    Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const override;
    GXVOID    Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const override;
    GXVOID    Paint(GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;
    GXVOID    Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const override;
    GXVOID    Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const override;
    GXVOID    Paint(GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;

    GXINT     Find(ID id, Type* pType) const override;
    GXINT     Find(GXLPCSTR szName, Type* pType) const override;
    GXSTDIMPLEMENT(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType = NULL) const);
    GXSTDIMPLEMENT(GXLPCSTR  FindName             (ID id) const);           // 用 ID 查找 Name
    GXSTDIMPLEMENT(ID        FindID               (GXLPCSTR szName) const); // 用 Name 查找 ID
    GXSTDIMPLEMENT(ID        FindID               (GXLPCWSTR szName) const); // 用 Name 查找 ID

    GXSIZE_T  GetModuleCount    () const override;
    GXSIZE_T  GetFrameCount     () const override;
    GXSIZE_T  GetAnimationCount () const override;
    GXSIZE_T  GetAnimFrameCount (GXUINT nIndex) const override;

    GXBOOL    GetModule     (GXUINT nIndex, MODULE* pModule) const override;
    GXBOOL    GetFrame      (GXUINT nIndex, FRAME* pFrame) const override;
    GXSIZE_T  GetFrameModule(GXUINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const override;
    GXBOOL    GetAnimation  (GXUINT nIndex, ANIMATION* pAnimation) const override;
    GXSIZE_T  GetAnimFrame  (GXUINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const override;

    GXBOOL    GetModuleRect     (GXUINT nIndex, GXLPRECT rcSprite) const override;  // 获得Module在Image中的位置
    GXBOOL    GetModuleRegion   (GXUINT nIndex, GXLPREGN rgSprite) const override;
    GXBOOL    GetFrameBounding  (GXUINT nIndex, GXLPRECT lprc) const override;
    GXBOOL    GetAnimBounding   (GXUINT nIndex, GXLPRECT lprc) const override;

    template<typename _TID>
    MOSprite::Type GetBoundingT(_TID id, GXLPRECT lprc) const;
    template<typename _TID>
    MOSprite::Type GetBoundingT(_TID id, GXLPREGN lprg) const;

    GXSTDIMPLEMENT(Type      GetBounding(ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDIMPLEMENT(Type      GetBounding(ID id, GXLPREGN lprg) const);
    GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
    GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

    GXSIZE_T  GetImageCount() const override;  // 含有的图片数量
    GXBOOL    GetImage(GXImage** pImage, GXUINT index) const override;
    clStringW GetImageFileW(GXUINT index) const override;
    clStringA GetImageFileA(GXUINT index) const override;

    //virtual GXHRESULT SaveW             (GXLPCWSTR szFilename) const;
  };

} // namespace Marimo


#endif // _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_