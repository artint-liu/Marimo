#ifndef _GRAPH_X_SPRITE_HEADER_FILE_
#define _GRAPH_X_SPRITE_HEADER_FILE_

class GXCanvas;
class GXGraphics;
class GXImage;

class GXSprite : public GUnknown
{
public:
  enum Type  // 索引类型
  {
    Type_Empty     = 0,  // Module类型索引
    Type_Module    = 1,  // Module类型索引
    Type_Frame     = 2,  // Frame类型索引
    Type_Animation = 3,  // 动画类型索引
    Type_Unified   = 4,  // 统一类型索引
  };

  struct MODULE
  {
    GXUINT   id;    // 0 是无效id
    GXLPCSTR name;
    GXREGN   regn;
  };

  struct FRAME
  {
    GXUINT   id;
    GXLPCSTR name;
    GXINT    start;   // 在aFrameDescs数组中的开始位置
    GXINT    count;
  };

  struct ANIMATION
  {
    GXUINT   id;
    GXLPCSTR name;
    GXUINT   rate;
    GXINT    start;
    GXINT    count;
  };

  struct FRAME_MODULE   // 描述单一的Module在Frame中的位置和旋转
  {
    GXUINT  nModuleIdx;
    GXDWORD rotate;      // 旋转翻转标志
    GXPOINT offset;
  };

  typedef GXUINT ANIM_FRAME; // 描述一段动画的帧序列
  typedef GXUINT    TIME_T;
  typedef GXUINT    ID;         // id 不能为 0, module/frame/animation 不能相同


public:
  //GXSTDINTERFACE(GXHRESULT SaveW              (GXLPCWSTR szFilename) const);
                                              
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

  GXSTDINTERFACE(GXINT     Find                 (ID id, GXOUT Type* pType = NULL) const); // pType 可以设置为NULL, 不返回类型
  GXSTDINTERFACE(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType = NULL) const);
  GXSTDINTERFACE(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType = NULL) const);
  GXSTDINTERFACE(GXLPCSTR  FindName             (ID id) const);           // 用 ID 查找 Name
  GXSTDINTERFACE(ID        FindID               (GXLPCSTR szName) const); // 用 Name 查找 ID
  GXSTDINTERFACE(ID        FindID               (GXLPCWSTR szName) const); // 用 Name 查找 ID

  GXSTDINTERFACE(GXSIZE_T  GetModuleCount       () const);
  GXSTDINTERFACE(GXSIZE_T  GetFrameCount        () const);
  GXSTDINTERFACE(GXSIZE_T  GetAnimationCount    () const);
  //GXSTDINTERFACE(GXBOOL    GetNameA             (IndexType eType, GXUINT nIndex, clStringA* pstrName) const);

  GXSTDINTERFACE(GXBOOL    GetModule            (GXINT nIndex, MODULE* pModule) const);
  GXSTDINTERFACE(GXBOOL    GetFrame             (GXINT nIndex, FRAME* pFrame) const);
  GXSTDINTERFACE(GXUINT    GetFrameModule       (GXINT nIndex, FRAME_MODULE* pFrameModule, int nCount) const);
  GXSTDINTERFACE(GXBOOL    GetAnimation         (GXINT nIndex, ANIMATION* pAnimation) const);
  GXSTDINTERFACE(GXUINT    GetAnimFrame         (GXINT nIndex, ANIM_FRAME* pAnimFrame, int nCount) const);
  
  GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXINT nIndex, GXRECT *rcSprite) const);  // 获得Module在Image中的位置
  GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXINT nIndex, GXREGN *rgSprite) const);
  GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPREGN lprg) const);
  GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
  GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

  //GXSTDINTERFACE(GXBOOL    GetSpriteBounding    (GXINT nUnifiedIndex, GXRECT* lprc) const); // 对于Module，返回值的left和top都应该是0
  //GXSTDINTERFACE(GXBOOL    GetSpriteBounding    (GXINT nUnifiedIndex, GXREGN* lprg) const);

  GXSTDINTERFACE(GXHRESULT GetImage             (GXImage** pImage));
  GXSTDINTERFACE(clStringW GetImageFileW        () const);
  GXSTDINTERFACE(clStringA GetImageFileA        () const);
                                          
  //GXSTDINTERFACE(int       FindByNameA          (GXLPCSTR szName) const);
  //GXSTDINTERFACE(int       FindByNameW          (GXLPCWSTR szName) const);
};

struct GXSPRITE_DESCW
{
  typedef GXSprite::MODULE        MODULE;
  typedef GXSprite::FRAME         FRAME;
  typedef GXSprite::FRAME_MODULE  FRAME_MODULE;
  typedef GXSprite::ANIMATION     ANIMATION;
  typedef GXSprite::ANIM_FRAME    ANIM_FRAME;

  GXUINT        cbSize;
  GXLPCWSTR     szImageFile;

  GXUINT        nNumOfModules;
  MODULE*       aModules;

  GXUINT        nNumOfFrames;
  FRAME*        aFrames;

  GXUINT        nNumOfFrameModules;
  FRAME_MODULE* aFrameModules;


  GXUINT        nNumOfAnimations;
  ANIMATION*    aAnimations;

  // 用于动画的帧列表
  GXUINT        nNumOfAnimFrames;
  ANIM_FRAME*   aAnimFrames;
};



class GXSpriteDesc
{
public:
  GXSTDINTERFACE(GXSPRITE_DESCW ToDesc());
  virtual ~GXSpriteDesc(){};
};

extern "C"
{
  GXBOOL    GXDLLAPI GXSaveSpriteToFileW      (GXLPCWSTR szFilename, const GXSPRITE_DESCW* pDesc);
  GXHRESULT GXDLLAPI GXCreateSprite           (GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount, GXSprite** ppSprite);
  GXHRESULT GXDLLAPI GXCreateSpriteEx         (GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite);
  GXHRESULT GXDLLAPI GXCreateSpriteArray      (GXGraphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite);
  GXHRESULT GXDLLAPI GXCreateSpriteFromFileW  (GXGraphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite);
  GXHRESULT GXDLLAPI GXCreateSpriteFromFileA  (GXGraphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite);
  GXHRESULT GXDLLAPI GXLoadSpriteDescW        (GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc);
  GXHRESULT GXDLLAPI GXLoadSpriteDescA        (GXLPCSTR szSpriteFile, GXSpriteDesc** ppDesc);
};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // end of _GRAPH_X_SPRITE_HEADER_FILE_