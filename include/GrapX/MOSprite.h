#ifndef _MARIMO_SPRITE_HEADER_FILE_
#define _MARIMO_SPRITE_HEADER_FILE_

namespace Marimo : public GUnknown
{
  class Sprite
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
      GXINT    id;    // 0 是无效id
      GXLPCSTR name;
      GXREGN   regn;
    };

    struct FRAME
    {
      GXINT    id;
      GXLPCSTR name;
      GXINT    start;   // 在aFrameDescs数组中的开始位置
      GXINT    count;
    };

    struct ANIMATION
    {
      GXINT    id;
      GXLPCSTR name;
      GXINT    start;
      GXINT    count;
      GXUINT   rate;
    };

    struct FRAME_UNIT   // 描述单一的Module在Frame中的位置和旋转
    {
      GXUINT  nModuleIdx;
      GXDWORD rotate;      // 旋转翻转标志
      GXREGN  regn;
    };

    typedef GXUINT    ANIM_UNIT;
    typedef GXINT     ID;         // id 不能为 0, module/frame/animation 不能相同
    typedef GXUINT    TIME_T;

  public:
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const);

    GXSTDINTERFACE(GXLONG    PaintModule3H        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
    GXSTDINTERFACE(GXLONG    PaintModule3V        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
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

    GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXINT nIndex, GXLPCRECT rcSprite) const);  // 获得Module在Image中的位置
    GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXINT nIndex, GXLPCREGN rgSprite) const);
    GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXINT nIndex, GXLPCRECT lprc) const);
    GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXINT nIndex, GXLPCRECT lprc) const);
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPCRECT lprc) const); // 对于Module，返回值的left和top都应该是0
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPCREGN lprg) const);

    GXSTDINTERFACE(GXSIZE_T  GetImageCount        () const);  // 含有的图片数量
    GXSTDINTERFACE(GXHRESULT GetImage             (GXImage** pImage, GXINT index) const);
    GXSTDINTERFACE(clStringW GetImageFileW        (GXINT index) const);
    GXSTDINTERFACE(clStringA GetImageFileA        (GXINT index) const);
  };
} // namespace Marimo

typedef Marimo::Sprite MOSprite;

#endif // #ifndef _MARIMO_SPRITE_HEADER_FILE_