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
      GXUINT   begin;   // 在aFrameDescs数组中的开始位置
      GXUINT   end;
    };

    struct ANIMATION
    {
      GXUINT   id;
      GXLPCSTR name;
      GXUINT   rate;
      GXUINT   begin;
      GXUINT   end;
    };

    struct FRAME_UNIT   // 描述单一的Module在Frame中的位置和旋转
    {
      GXUINT  nModuleIdx;
      GXDWORD rotate;   // 旋转翻转标志
      GXREGN  regn;
    };

    struct ANIM_UNIT    // 描述一段动画的帧序列
    {
      GXUINT frame;     // 帧索引
      GXUINT duration;  // 持续时间
    };

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

    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXUINT nIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXUINT nIndex, GXLPCREGN lpRegn) const); // 这里REGN不是显示区域，(left,top)是原点位置，位于中心，(width,height)是参考尺寸
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXUINT nIndex, GXLPCRECT lpRect) const); // Frame显示在RECT中，而忽略它本身的偏移
    
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCREGN lpRegn) const); // 同PaintFrame中REGN含义
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCRECT lpRect) const); // 同PaintFrame中RECT含义
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXINT x, GXINT y));
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn)); // 同PaintFrame中REGN含义
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCRECT lpRect)); // 同PaintFrame中RECT含义

    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
       
    GXSTDINTERFACE(GXINT     Find                 (ID id, GXOUT Type* pType = NULL) const); // pType 可以设置为NULL, 不返回类型
    GXSTDINTERFACE(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType = NULL) const);
    GXSTDINTERFACE(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType = NULL) const);
    GXSTDINTERFACE(GXLPCSTR  FindName             (ID id) const);           // 用 ID 查找 Name
    GXSTDINTERFACE(ID        FindID               (GXLPCSTR szName) const); // 用 Name 查找 ID
    GXSTDINTERFACE(ID        FindID               (GXLPCWSTR szName) const); // 用 Name 查找 ID

    GXSTDINTERFACE(GXINT     PackIndex            (Type type, GXUINT index) const);   // 将不同类型的索引打包为统一类型的索引
    GXSTDINTERFACE(GXINT     UnpackIndex          (GXUINT nUniqueIndex, Type* pType) const); // 将统一索引拆解为类型和类型索引

    GXSTDINTERFACE(GXSIZE_T  GetModuleCount       () const);
    GXSTDINTERFACE(GXSIZE_T  GetFrameCount        () const);
    GXSTDINTERFACE(GXSIZE_T  GetFrameModuleCount  (GXUINT nFrameIndex) const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimationCount    () const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimFrameCount    (GXUINT nAnimIndex) const);

    GXSTDINTERFACE(GXBOOL    GetModule            (GXUINT nIndex, MODULE* pModule) const);
    GXSTDINTERFACE(GXBOOL    GetFrame             (GXUINT nIndex, FRAME* pFrame) const);
    GXSTDINTERFACE(GXSIZE_T  GetFrameModule       (GXUINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const);
    GXSTDINTERFACE(GXBOOL    GetAnimation         (GXUINT nIndex, ANIMATION* pAnimation) const);
    GXSTDINTERFACE(GXSIZE_T  GetAnimFrame         (GXUINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const);

    GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXUINT nIndex, GXLPRECT rcSprite) const);  // 获得Module在Image中的位置
    GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXUINT nIndex, GXLPREGN rgSprite) const);
    GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXUINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXUINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPREGN lprg) const);
    GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
    GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

    GXSTDINTERFACE(GXSIZE_T  GetImageCount        () const);  // 含有的图片数量
    GXSTDINTERFACE(GXBOOL    GetImage             (GXImage** pImage, GXUINT index) const);
    GXSTDINTERFACE(clStringW GetImageFileW        (GXUINT index) const);
    GXSTDINTERFACE(clStringA GetImageFileA        (GXUINT index) const);

    static GXBOOL GXDLLAPI CreateFromStockA       (Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szImageDir, GXLPCSTR szSection = "sprite");
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

    enum Caps
    {
      Caps_VariableRate = 0x00000001, // 动画中帧与帧间的时间间隔是变化的
    };

    clStringArrayA          aFiles;
    GXDWORD                 dwCapsFlags; // 参考 Caps 定义
    Sprite::ModuleArray     aModules;
    Sprite::FrameArray      aFrames;
    Sprite::AnimationArray  aAnims;
    Sprite::FrameUnitArray  aFrameUnits;
    Sprite::AnimUnitArray   aAnimUnits;

    IDDict                  sIDDict;
    NameDict                sNameDict;

    SPRITE_DESC() : dwCapsFlags(0) {}
    virtual ~SPRITE_DESC() {}

    static SPRITE_DESC* Create  (clstd::StockA* pStock, GXLPCSTR szSection);
    static void         Destroy (SPRITE_DESC* pDesc);
  };

} // namespace Marimo

typedef Marimo::Sprite MOSprite;

#endif // #ifndef _MARIMO_SPRITE_HEADER_FILE_