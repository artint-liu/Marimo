#ifndef _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_
#define _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_

class GXImage;
class GXCanvas;
class GXGraphics;

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
private:
  clStringW         m_strImageFile;
  GXImage*          m_pImage;

  clStringSetA      m_NameSet; // TODO: 这个将来可以取消，使用压实的一大块内存储存字符串序列

  ModuleArray       m_aModules;
  
  FrameArray        m_aFrames;
  FrameModuleArray  m_aFrameModules;

  AnimationArray    m_aAnimations;
  AnimFrameArray    m_aAnimFrames;

  NameDict          m_SpriteDict;
  //int IntGetSpriteCount() const;
protected:
  virtual ~MOSpriteImpl();
public:
  MOSpriteImpl();
  //GXBOOL Initialize(GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc);
  //GXBOOL Initialize(GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount);


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef          ();
  virtual GXHRESULT Release         ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const override;
  GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const override;
  GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const override;

  GXVOID    PaintModule3H        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const override;
  GXVOID    PaintModule3V        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const override;
  GXVOID    PaintModule3x3       (GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawCenter, GXLPCRECT rect) const override;

  GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const override;
  GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const override;
  GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;

  GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const override;
  GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const override;
  GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const override;
  GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const override;

  GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const override;
  GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const override;
  GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;
  GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const override;
  GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const override;
  GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const override;

  GXINT     Find                 (ID id) const override;
  GXINT     Find                 (GXLPCSTR szName) const override;

  GXSIZE_T  GetModuleCount       () const override;
  GXSIZE_T  GetFrameCount        () const override;
  GXSIZE_T  GetAnimationCount    () const override;
  GXSIZE_T  GetAnimFrameCount    (GXINT nIndex) const override;

  GXBOOL    GetModule            (GXINT nIndex, MODULE* pModule) const override;
  GXBOOL    GetFrame             (GXINT nIndex, FRAME* pFrame) const override;
  GXSIZE_T  GetFrameModule       (GXINT nIndex, FRAME_UNIT* pFrameModule, GXSIZE_T nCount) const override;
  GXBOOL    GetAnimation         (GXINT nIndex, ANIMATION* pAnimation) const override;
  GXSIZE_T  GetAnimFrame         (GXINT nIndex, ANIM_UNIT* pAnimFrame, GXSIZE_T nCount) const override;

  GXBOOL    GetModuleRect        (GXINT nIndex, GXLPRECT rcSprite) const override;  // 获得Module在Image中的位置
  GXBOOL    GetModuleRegion      (GXINT nIndex, GXLPREGN rgSprite) const override;
  GXBOOL    GetFrameBounding     (GXINT nIndex, GXLPRECT lprc) const override;
  GXBOOL    GetAnimBounding      (GXINT nIndex, GXLPRECT lprc) const override;
  Type      GetBounding          (ID id, GXLPRECT lprc) const override; // 对于Module，返回值的left和top都应该是0
  Type      GetBounding          (ID id, GXLPREGN lprg) const override;

  GXSIZE_T  GetImageCount        () const override;  // 含有的图片数量
  GXBOOL    GetImage             (GXImage** pImage, GXINT index) const override;
  clStringW GetImageFileW        (GXINT index) const override;
  clStringA GetImageFileA        (GXINT index) const override;

  //virtual GXHRESULT SaveW             (GXLPCWSTR szFilename) const;


};


#endif // _IMPLEMENT_MARIMO_SPRITE_HEADER_FILE_