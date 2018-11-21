#ifndef _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_
#define _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_

//class GXImage;
namespace GrapX
{
  class GXCanvas;
  class Graphics;
}

class GXSpriteDescImpl : public GXSpriteDesc
{
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXUINT>                  AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;

protected:
  clStringW         m_strImageFile;
  clStringSetA      m_NameSet;      // ���������ַ����ļ���
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
      GXSprite::ANIMATION* pAnimation;
    };
  };

  typedef clmap<clStringA, IDATTR>          NameDict;
  typedef clmap<ID, IDATTR>                 IDDict;

  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXUINT>                  AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;
private:
  clStringW         m_strImageFile;
  //GXImage*          m_pImage;
  GrapX::GTexture*  m_pTexture;

  clStringSetA      m_NameSet; // TODO: �����������ȡ����ʹ��ѹʵ��һ����ڴ洢���ַ�������

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
  GXBOOL Initialize(GrapX::Graphics* pGraphics, const GXSPRITE_DESCW* pDesc);
  GXBOOL Initialize(GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount);


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef          ();
  virtual GXHRESULT Release         ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //virtual GXHRESULT SaveW             (GXLPCWSTR szFilename) const;

  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::GXCanvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::GXCanvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintModule3H        (GrapX::GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3V        (GrapX::GXCanvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3x3       (GrapX::GXCanvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::GXCanvas *pCanvas, GXUINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::GXCanvas *pCanvas, GXUINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::GXCanvas *pCanvas, GXUINT nIndex, GXLPCRECT lpRect) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCRECT lpRect) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXINT x, GXINT y));
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn));
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCRECT lpRect));

  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);

  GXSTDIMPLEMENT(GXINT     Find                 (ID id, GXOUT Type* pType) const); // pType ��������ΪNULL, ����������
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXLPCSTR  FindName             (ID id) const);           // �� ID ���� Name
  GXSTDIMPLEMENT(ID        FindID               (GXLPCSTR szName) const); // �� Name ���� ID
  GXSTDIMPLEMENT(ID        FindID               (GXLPCWSTR szName) const); // �� Name ���� ID

  GXSTDIMPLEMENT(GXINT     PackIndex            (Type type, GXUINT index) const);   // ����ͬ���͵��������Ϊͳһ���͵�����
  GXSTDIMPLEMENT(GXINT     UnpackIndex          (GXUINT nUniqueIndex, Type* pType) const); // ��ͳһ�������Ϊ���ͺ���������

  GXSTDIMPLEMENT(GXSIZE_T  GetModuleCount       () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetFrameCount        () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetFrameModuleCount  (GXUINT nFrameIndex) const);
  GXSTDIMPLEMENT(GXSIZE_T  GetAnimationCount    () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetAnimFrameCount    (GXUINT nAnimIndex) const);
  //virtual GXBOOL    GetNameA          (IndexType eType, GXUINT nIndex, clStringA* pstrName) const;

  GXSTDIMPLEMENT(GXBOOL    GetModule            (GXUINT nIndex, MODULE* pModule) const);
  GXSTDIMPLEMENT(GXBOOL    GetFrame             (GXUINT nIndex, FRAME* pFrame) const);
  GXSTDIMPLEMENT(GXUINT    GetFrameModule       (GXUINT nIndex, FRAME_MODULE* pFrameModule, GXSIZE_T nCount) const);
  GXSTDIMPLEMENT(GXBOOL    GetAnimation         (GXUINT nIndex, ANIMATION* pAnimation) const);
  GXSTDIMPLEMENT(GXUINT    GetAnimFrame         (GXUINT nIndex, ANIM_FRAME* pAnimFrame, GXSIZE_T nCount) const);

  
  virtual GXBOOL    GetModuleRect     (GXUINT nIndex, GXRECT *rcSprite) const;
  virtual GXBOOL    GetModuleRegion   (GXUINT nIndex, REGN *rgSprite) const;
  virtual GXBOOL    GetFrameBounding  (GXUINT nIndex, GXRECT* lprc) const;
  virtual GXBOOL    GetAnimBounding   (GXUINT nIndex, GXRECT* lprc) const;

  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPRECT lprc) const;
  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPREGN lprg) const;

  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

  GXHRESULT GetTexture        (GrapX::GTexture** ppTexture) override;
  clStringW GetImageFileW     () const override;
  clStringA GetImageFileA     () const override;

  //virtual int FindByNameA             (GXLPCSTR szName) const;
  //virtual int FindByNameW             (GXLPCWSTR szName) const;

  template<class _TArray, class _TDesc>
  void Add(_TArray& aArray, Type type, _TDesc& desc);

  const IDATTR* IntFind(ID id) const;
  const IDATTR* IntFind(GXLPCSTR szName) const;
  GXINT AttrToIndex(const IDATTR* pAttr) const;
  void  IntGetBounding(const IDATTR* pAttr, GXREGN* lprg) const;

  friend GXHRESULT GXDLLAPI GXCreateSprite          (GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN*  aRegion, GXINT nCount, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteEx        (GrapX::Graphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteArray     (GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileW (GrapX::Graphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileA (GrapX::Graphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW       (GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescA       (GXLPCSTR szSpriteFile, GXSpriteDescObj** ppDesc);
};


#endif // _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_