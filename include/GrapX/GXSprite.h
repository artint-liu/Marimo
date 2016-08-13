#ifndef _GRAPH_X_SPRITE_HEADER_FILE_
#define _GRAPH_X_SPRITE_HEADER_FILE_

class GXCanvas;
class GXGraphics;
class GXImage;

class GXSprite : public GUnknown
{
public:
  enum IndexType  // 索引类型
  {
    IndexType_Module    = 0,  // Module类型索引
    IndexType_Frame     = 1,  // Frame类型索引
    IndexType_Animation = 2,  // 动画类型索引
    IndexType_Unified   = 3,  // 统一类型索引
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
public:
  //GXSTDINTERFACE(GXHRESULT SaveW              (GXLPCWSTR szFilename) const);
                                              
  GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDINTERFACE(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
//GXSTDINTERFACE(GXVOID    PaintModuleH         (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nWidth) const);
//GXSTDINTERFACE(GXVOID    PaintModuleV         (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT nHeight) const);
                                                
  GXSTDINTERFACE(GXLONG    PaintModule3H        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDINTERFACE(GXLONG    PaintModule3V        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDINTERFACE(GXVOID    PaintModule3x3       (GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const);

  GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT idxAnim, GXINT idxFrame, GXINT x, GXINT y) const);

  GXSTDINTERFACE(GXVOID    PaintSprite          (GXCanvas *pCanvas, GXINT nUnifiedIndex, GXINT nMinorIndex, GXINT x, GXINT y, float xScale, float yScale) const); // x,y canvas坐标，不受Scale影响。
//GXSTDINTERFACE(GXVOID    PaintByUniformIndex  (GXCanvas *pCanvas, GXINT nIndex, GXINT nMinorIndex, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
                                          
  GXSTDINTERFACE(GXSIZE_T  GetModuleCount       () const);
  GXSTDINTERFACE(GXSIZE_T  GetFrameCount        () const);
  GXSTDINTERFACE(GXSIZE_T  GetAnimationCount    () const);
  GXSTDINTERFACE(GXBOOL    GetNameA             (IndexType eType, GXUINT nIndex, clStringA* pstrName) const);

  GXSTDINTERFACE(GXBOOL    GetModule            (GXINT nIndex, MODULE* pModule) const);
  GXSTDINTERFACE(GXBOOL    GetFrame             (GXINT nIndex, FRAME* pFrame) const);
  GXSTDINTERFACE(GXUINT    GetFrameModule       (GXINT nIndex, FRAME_MODULE* pFrameModule, int nCount) const);
  GXSTDINTERFACE(GXBOOL    GetAnimation         (GXINT nIndex, ANIMATION* pAnimation) const);
  GXSTDINTERFACE(GXUINT    GetAnimFrame         (GXINT nIndex, ANIM_FRAME* pAnimFrame, int nCount) const);
  
  GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXINT nIndex, GXRECT *rcSprite) const);  // 获得Module在Image中的位置
  GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXINT nIndex, GXREGN *rgSprite) const);
  GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(GXBOOL    GetSpriteBounding    (GXINT nUnifiedIndex, GXRECT* lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDINTERFACE(GXBOOL    GetSpriteBounding    (GXINT nUnifiedIndex, GXREGN* lprg) const);

  GXSTDINTERFACE(GXHRESULT GetImage             (GXImage** pImage));
  GXSTDINTERFACE(clStringW GetImageFileW        () const);
  GXSTDINTERFACE(clStringA GetImageFileA        () const);
                                          
  GXSTDINTERFACE(int       FindByNameA          (GXLPCSTR szName) const);
  GXSTDINTERFACE(int       FindByNameW          (GXLPCWSTR szName) const);
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