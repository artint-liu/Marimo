#ifndef _MARIMO_SPRITE_HEADER_FILE_
#define _MARIMO_SPRITE_HEADER_FILE_

class GXImage;

namespace Marimo
{
  class Sprite : public GUnknown
  {
  public:
    enum Type
    {
      Type_Error,
      Type_Module,
      Type_Frame,
      Type_Animation,
    };

    struct MODULE
    {
      GXUINT   id;    // 0 是无效id
      GXLPCSTR name;
      GXREGN   regn;  // regn.top 的值确定使用哪个Image
    };

    struct FRAME
    {
      GXUINT   id;
      GXLPCSTR name;
      GXINT    begin;   // 在aFrameDescs数组中的开始位置
      GXINT    end;
    };

    struct ANIMATION
    {
      GXUINT   id;
      GXLPCSTR name;
      GXUINT   rate;
      GXINT    begin;
      GXINT    end;
    };

    struct FRAME_UNIT   // 描述单一的Module在Frame中的位置和旋转
    {
      GXUINT  nModuleIdx;
      GXDWORD rotate;      // 旋转翻转标志
      GXREGN  regn;
    };

    typedef GXUINT    ANIM_UNIT;
    typedef GXUINT    TIME_T;
    typedef GXUINT    ID;         // id 不能为 0, module/frame/animation 不能相同

    typedef clvector<MODULE>      ModuleArray;
    typedef clvector<FRAME>       FrameArray;
    typedef clvector<ANIMATION>   AnimationArray;
    typedef clvector<FRAME_UNIT>  FrameUnitArray;
    typedef clvector<ANIM_UNIT>   AnimUnitArray;

  public:
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const);

    GXSTDINTERFACE(GXVOID    PaintModule3H        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
    GXSTDINTERFACE(GXVOID    PaintModule3V        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
    GXSTDINTERFACE(GXVOID    PaintModule3x3       (GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawCenter, GXLPCRECT rect) const);

    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
    
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const);

    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
       
    GXSTDINTERFACE(GXINT     Find                 (ID id) const);
    GXSTDINTERFACE(GXINT     Find                 (GXLPCSTR szName) const);

    GXSTDINTERFACE(GXSIZE_T  GetModuleCount       () const);
    GXSTDINTERFACE(GXSIZE_T  GetFrameCount        () const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimationCount    () const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimFrameCount    (GXINT nIndex) const);

    GXSTDINTERFACE(GXBOOL    GetModule            (GXINT nIndex, MODULE* pModule) const);
    GXSTDINTERFACE(GXBOOL    GetFrame             (GXINT nIndex, FRAME* pFrame) const);
    GXSTDINTERFACE(GXSIZE_T  GetFrameModule       (GXINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const);
    GXSTDINTERFACE(GXBOOL    GetAnimation         (GXINT nIndex, ANIMATION* pAnimation) const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimFrame         (GXINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const);

    GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXINT nIndex, GXLPRECT rcSprite) const);  // 获得Module在Image中的位置
    GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXINT nIndex, GXLPREGN rgSprite) const);
    GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPREGN lprg) const);

    GXSTDINTERFACE(GXSIZE_T  GetImageCount        () const);  // 含有的图片数量
    GXSTDINTERFACE(GXBOOL    GetImage             (GXImage** pImage, GXUINT index) const);
    GXSTDINTERFACE(clStringW GetImageFileW        (GXINT index) const);
    GXSTDINTERFACE(clStringA GetImageFileA        (GXINT index) const);

    static GXBOOL GXDLLAPI CreateFromStockA       (Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szSection = "sprite");
    //static GXBOOL GXDLLAPI CreateFromStockW       (Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szSection = "sprite");
    static GXBOOL GXDLLAPI CreateFromStockFileA   (Sprite** ppSprite, GXGraphics* pGraphics, GXLPCSTR szFilename, GXLPCSTR szSection = "sprite");
    //static GXBOOL GXDLLAPI CreateFromStockFileW   (Sprite** ppSprite, GXGraphics* pGraphics, GXLPCSTR szFilename, GXLPCSTR szSection = "sprite");
  };

  //
  // 用于在加载/储存时使用的描述结构
  //
  struct GXDLL SPRITE_DESC
  {
    struct TYPEIDX{
      Sprite::Type type;
      union {
        GXUINT             index;
        Sprite::MODULE*    pModel;
        Sprite::FRAME*     pFrame;
        Sprite::ANIMATION* pAnination;
      };
    };

    typedef clmap<Sprite::ID, TYPEIDX>  IDDict;
    typedef clmap<GXLPCSTR, TYPEIDX>    NameDict;

    clStringArrayA          aFiles;
    Sprite::ModuleArray     aModules;
    Sprite::FrameArray      aFrames;
    Sprite::AnimationArray  aAnims;
    Sprite::FrameUnitArray  aFrameUnits;
    Sprite::AnimUnitArray   aAnimUnits;

    IDDict                  sIDDict;
    NameDict                sNameDict;

    static SPRITE_DESC* Create  (clstd::StockA* pStock, GXLPCSTR szSection);
    static void         Destroy (SPRITE_DESC* pDesc);
  };

} // namespace Marimo

typedef Marimo::Sprite MOSprite;

#endif // #ifndef _MARIMO_SPRITE_HEADER_FILE_