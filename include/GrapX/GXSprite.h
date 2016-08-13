#ifndef _GRAPH_X_SPRITE_HEADER_FILE_
#define _GRAPH_X_SPRITE_HEADER_FILE_

class GXCanvas;
class GXGraphics;
class GXImage;

class GXSprite : public GUnknown
{
public:
  enum IndexType  // ��������
  {
    IndexType_Module    = 0,  // Module��������
    IndexType_Frame     = 1,  // Frame��������
    IndexType_Animation = 2,  // ������������
    IndexType_Unified   = 3,  // ͳһ��������
  };

  struct MODULE
  {
    GXINT    id;    // 0 ����Чid
    GXLPCSTR name;
    GXREGN   regn;
  };

  struct FRAME
  {
    GXINT    id;
    GXLPCSTR name;
    GXINT    start;   // ��aFrameDescs�����еĿ�ʼλ��
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

  struct FRAME_MODULE   // ������һ��Module��Frame�е�λ�ú���ת
  {
    GXUINT  nModuleIdx;
    GXDWORD rotate;      // ��ת��ת��־
    GXPOINT offset;
  };

  typedef GXUINT ANIM_FRAME; // ����һ�ζ�����֡����
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

  GXSTDINTERFACE(GXVOID    PaintSprite          (GXCanvas *pCanvas, GXINT nUnifiedIndex, GXINT nMinorIndex, GXINT x, GXINT y, float xScale, float yScale) const); // x,y canvas���꣬����ScaleӰ�졣
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
  
  GXSTDINTERFACE(GXBOOL    GetModuleRect        (GXINT nIndex, GXRECT *rcSprite) const);  // ���Module��Image�е�λ��
  GXSTDINTERFACE(GXBOOL    GetModuleRegion      (GXINT nIndex, GXREGN *rgSprite) const);
  GXSTDINTERFACE(GXBOOL    GetFrameBounding     (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(GXBOOL    GetAnimBounding      (GXINT nIndex, GXRECT* lprc) const);
  GXSTDINTERFACE(GXBOOL    GetSpriteBounding    (GXINT nUnifiedIndex, GXRECT* lprc) const); // ����Module������ֵ��left��top��Ӧ����0
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

  // ���ڶ�����֡�б�
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