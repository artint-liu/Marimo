#ifndef _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_
#define _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_

class GXImage;
class GXCanvas;
class GXGraphics;

class GXSpriteDescImpl : public GXSpriteDesc
{
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXSprite::ANIM_FRAME>    AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;

protected:
  clStringW         m_strImageFile;
  clStringSetA      m_NameSet;      // 用来储存字符串的集合
  ModuleArray       m_aModules;
  FrameArray        m_aFrames;
  FrameModuleArray  m_aFrameModules;
  AnimationArray    m_aAnimations;
  AnimFrameArray    m_aAnimFrames;

public:
  virtual ~GXSpriteDescImpl();
  GXSPRITE_DESCW ToDesc();

public:
  friend GXHRESULT IntLoadSpriteDesc(clstd::StockA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc);
  friend GXHRESULT IntLoadModules   (clstd::StockA& ss, GXSpriteDescImpl* pDescObj);
  friend GXHRESULT IntLoadFrames    (clstd::StockA& ss, GXSpriteDescImpl* pDescObj);
  friend GXHRESULT IntLoadAnimations(clstd::StockA& ss, GXSpriteDescImpl* pDescObj) ;
};

class GXSpriteImpl : public GXSprite
{
public:
  //struct MODULE
  //{
  //  clStringA name;
  //  Regn      regn;
  //};

  //typedef clvector<MODULE>      ModuleArray;
  struct IDATTR{
    GXSprite::Type type;
    union {
      GXUINT               index;
      GXSprite::MODULE*    pModel;
      GXSprite::FRAME*     pFrame;
      GXSprite::ANIMATION* pAnination;
    };
  };

  typedef clmap<clStringA, IDATTR>          NameDict;
  typedef clmap<ID, IDATTR>                 IDDict;

  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXSprite::ANIM_FRAME>    AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;
private:
  clStringW         m_strImageFile;
  GXImage*          m_pImage;

  clStringSetA      m_NameSet; // TODO: 这个将来可以取消，使用压实的一大块内存储存字符串序列

  ModuleArray       m_aModules;
  
  FrameArray        m_aFrames;
  FrameModuleArray  m_aFrameModules;

  AnimationArray    m_aAnimations;
  AnimFrameArray    m_aAnimFrames;

  NameDict          m_NameDict;
  IDDict            m_IDDict;
  int IntGetSpriteCount() const;
protected:
  virtual ~GXSpriteImpl();
public:
  GXSpriteImpl();
  GXBOOL Initialize(GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc);
  GXBOOL Initialize(GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount);


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef          ();
  virtual GXHRESULT Release         ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //virtual GXHRESULT SaveW             (GXLPCWSTR szFilename) const;

  GXSTDIMPLEMENT(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintModule3H        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3V        (GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3x3       (GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXINT nAnimIndex, GXINT nFrameIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const);

  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);

  GXSTDIMPLEMENT(GXINT     Find                 (ID id, GXOUT Type* pType) const); // pType 可以设置为NULL, 不返回类型
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXLPCSTR  FindName             (ID id) const);           // 用 ID 查找 Name
  GXSTDIMPLEMENT(ID        FindID               (GXLPCSTR szName) const); // 用 Name 查找 ID
  GXSTDIMPLEMENT(ID        FindID               (GXLPCWSTR szName) const); // 用 Name 查找 ID

  virtual GXSIZE_T  GetModuleCount    () const;
  virtual GXSIZE_T  GetFrameCount     () const;
  virtual GXSIZE_T  GetAnimationCount () const;
  //virtual GXBOOL    GetNameA          (IndexType eType, GXUINT nIndex, clStringA* pstrName) const;

  virtual GXBOOL    GetModule           (GXINT nIndex, MODULE* pModule) const;
  virtual GXBOOL    GetFrame            (GXINT nIndex, FRAME* pFrame) const;
  virtual GXUINT    GetFrameModule      (GXINT nIndex, FRAME_MODULE* pFrameModule, int nCount) const;
  virtual GXBOOL    GetAnimation        (GXINT nIndex, ANIMATION* pAnimation) const;
  virtual GXUINT    GetAnimFrame        (GXINT nIndex, ANIM_FRAME* pAnimFrame, int nCount) const;

  
  virtual GXBOOL    GetModuleRect     (GXINT nIndex, GXRECT *rcSprite) const;
  virtual GXBOOL    GetModuleRegion   (GXINT nIndex, REGN *rgSprite) const;
  virtual GXBOOL    GetFrameBounding  (GXINT nIndex, GXRECT* lprc) const;
  virtual GXBOOL    GetAnimBounding   (GXINT nIndex, GXRECT* lprc) const;

  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPRECT lprc) const;
  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPREGN lprg) const;

  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

  virtual GXHRESULT GetImage          (GXImage** ppImage);
  virtual clStringW GetImageFileW     () const;
  virtual clStringA GetImageFileA     () const;

  //virtual int FindByNameA             (GXLPCSTR szName) const;
  //virtual int FindByNameW             (GXLPCWSTR szName) const;

  template<class _TArray, class _TDesc>
  void Add(_TArray& aArray, Type type, _TDesc& desc);

  const IDATTR* IntFind(ID id) const;
  const IDATTR* IntFind(GXLPCSTR szName) const;
  GXINT AttrToIndex(const IDATTR* pAttr) const;
  void  IntGetBounding(const IDATTR* pAttr, GXREGN* lprg) const;

  friend GXHRESULT GXDLLAPI GXCreateSprite          (GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN*  aRegion, GXINT nCount, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteEx        (GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteArray     (GXGraphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileW (GXGraphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileA (GXGraphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW       (GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescA       (GXLPCSTR szSpriteFile, GXSpriteDescObj** ppDesc);
};


#endif // _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_