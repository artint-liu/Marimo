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
      GXUINT   id;    // 0 ����Чid
      GXLPCSTR name;
      GXREGN   regn;  // regn.top ��ֵȷ��ʹ���ĸ�Image
    };

    struct FRAME
    {
      GXUINT   id;
      GXLPCSTR name;
      GXUINT   begin;   // ��aFrameDescs�����еĿ�ʼλ��
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

    struct FRAME_UNIT   // ������һ��Module��Frame�е�λ�ú���ת
    {
      GXUINT  nModuleIdx;
      GXDWORD rotate;      // ��ת��ת��־
      GXREGN  regn;
    };

    typedef GXUINT    ANIM_UNIT;
    typedef GXUINT    TIME_T;
    typedef GXUINT    ID;         // id ����Ϊ 0, module/frame/animation ������ͬ

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
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXUINT nIndex, GXLPCREGN lpRegn) const); // ����REGN������ʾ����(left,top)��ԭ��λ�ã�λ�����ģ�(width,height)�ǲο��ߴ�
    GXSTDINTERFACE(GXVOID    PaintFrame           (GXCanvas *pCanvas, GXUINT nIndex, GXLPCRECT lpRect) const); // Frame��ʾ��RECT�У���������������ƫ��
    
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCREGN lpRegn) const); // ͬPaintFrame��REGN����
    GXSTDINTERFACE(GXVOID    PaintAnimationFrame  (GXCanvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCRECT lpRect) const); // ͬPaintFrame��RECT����
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn) const); // ͬPaintFrame��REGN����
    GXSTDINTERFACE(GXVOID    PaintAnimationByTime (GXCanvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCRECT lpRect) const); // ͬPaintFrame��RECT����

    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
    GXSTDINTERFACE(GXVOID    Paint                (GXCanvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
       
    GXSTDINTERFACE(GXINT     Find                 (ID id, GXOUT Type* pType = NULL) const); // pType ��������ΪNULL, ����������
    GXSTDINTERFACE(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType = NULL) const);
    GXSTDINTERFACE(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType = NULL) const);
    GXSTDINTERFACE(GXLPCSTR  FindName             (ID id) const);           // �� ID ���� Name
    GXSTDINTERFACE(ID        FindID               (GXLPCSTR szName) const); // �� Name ���� ID
    GXSTDINTERFACE(ID        FindID               (GXLPCWSTR szName) const); // �� Name ���� ID

    GXSTDINTERFACE(GXINT     PackIndex            (Type type, GXUINT index) const);   // ����ͬ���͵��������Ϊͳһ���͵�����
    GXSTDINTERFACE(GXINT     UnpackIndex          (GXUINT nUniqueIndex, Type* pType) const); // ��ͳһ�������Ϊ���ͺ���������

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

    GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXUINT nIndex, GXLPRECT rcSprite) const);  // ���Module��Image�е�λ��
    GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXUINT nIndex, GXLPREGN rgSprite) const);
    GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXUINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXUINT nIndex, GXLPRECT lprc) const);
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
    GXSTDINTERFACE(Type      GetBounding          (ID id, GXLPREGN lprg) const);
    GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
    GXSTDINTERFACE(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
    GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // ����Module������ֵ��left��top��Ӧ����0
    GXSTDINTERFACE(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

    GXSTDINTERFACE(GXSIZE_T  GetImageCount        () const);  // ���е�ͼƬ����
    GXSTDINTERFACE(GXBOOL    GetImage             (GXImage** pImage, GXUINT index) const);
    GXSTDINTERFACE(clStringW GetImageFileW        (GXUINT index) const);
    GXSTDINTERFACE(clStringA GetImageFileA        (GXUINT index) const);

    static GXBOOL GXDLLAPI CreateFromStockA       (Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szImageDir, GXLPCSTR szSection = "sprite");
    //static GXBOOL GXDLLAPI CreateFromStockW       (Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szSection = "sprite");
    static GXBOOL GXDLLAPI CreateFromStockFileA   (Sprite** ppSprite, GXGraphics* pGraphics, GXLPCSTR szFilename, GXLPCSTR szSection = "sprite");
    //static GXBOOL GXDLLAPI CreateFromStockFileW   (Sprite** ppSprite, GXGraphics* pGraphics, GXLPCSTR szFilename, GXLPCSTR szSection = "sprite");
  };

  //
  // �����ڼ���/����ʱʹ�õ������ṹ
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