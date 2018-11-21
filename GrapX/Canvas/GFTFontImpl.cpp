// 全局头文件
#include <GrapX.h>
#include "User/GrapX.Hxx"

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXKernel.h"

// 平台相关
// 私有头文件
//#include <vector>
//#include <hash_map>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftwinfnt.h>

#include "GrapX/GXUser.h"

#include "GFTFontImpl.h"

#ifdef _DEBUG
#  pragma comment(lib, "freetype2411ST_D.lib")
#else
#  pragma comment(lib, "freetype2411ST.lib")
#endif // _DEBUG


#define DEFAULT_FONT_TEX_SIZE_X    256
#define DEFAULT_FONT_TEX_SIZE_Y    256
#define FONT_INFLATE        2
//clString IOSGetResourceDir();
//////////////////////////////////////////////////////////////////////////
GXVOID _GFTFont::FontDescToCharDesc(const FTFONTDESC &fm, LPCHARDESC lpCD)
{
  lpCD->pTex         = m_aFontTex[fm.idxTex];
  lpCD->rgSrc.left   = fm.xGrid;
  lpCD->rgSrc.top    = fm.yGrid;
  lpCD->rgSrc.width  = fm.nWidth;
  lpCD->rgSrc.height = fm.nHeight;// + fm.yPos;
  lpCD->ptDest.x     = fm.xPos;
  lpCD->ptDest.y     = fm.yPos;
  lpCD->nAdvWidth    = fm.nAdvWidth;
//  lpCD->rgDest.height= fm.nHeight;
}
//////////////////////////////////////////////////////////////////////////

GXBOOL _GFTFont::CreateFont(const GXULONG nWidth, const GXULONG nHeight, const GXCHAR *pFileName)
{
  if( FT_Init_FreeType( &m_Library ) )
  {
    return FALSE;
  }
  //clStringA strResource = pFileName;
  //m_pGraphics->ConvertToAbsPathW(strResource);
  clStringA strFileName = pFileName;
  m_pGraphics->ConvertToAbsolutePathA(strFileName);
  if( FT_New_Face(m_Library, strFileName, 0, &m_Face) )
  {
    return FALSE;
  }
  FT_Select_Charmap(m_Face, FT_ENCODING_UNICODE);

  //GXUINT uLen = (GXUINT)GXSTRLEN(pFileName) + 1;
  //m_pFaceName = new GXCHAR[uLen];
  //GXSTRCPY(m_pFaceName, pFileName);
  m_strFaceName = pFileName;
  m_nWidth = nWidth == 0 ? (nHeight + FONT_INFLATE) : (nWidth + FONT_INFLATE);
  m_nHeight = nHeight + FONT_INFLATE;

  FT_UInt nFontHeight = (FT_UInt)nHeight;
  if(nFontHeight == 0)
    nFontHeight = 12;
  else if(nFontHeight & 0x80000000)
    nFontHeight = -((int)nFontHeight);

  FT_Set_Pixel_Sizes(m_Face, (FT_UInt)nWidth, nFontHeight);

  m_pmapCode2FMatrix = new CharToFontDesc;

  //FT_Matrix     matrix;              /* transformation matrix */
  //FT_Vector     pen;

  //matrix.xx = 1 << 16;
  //matrix.xy = 0;
  //matrix.yy = 1 << 16;
  //matrix.yx = 0;

  //pen.x = 0;
  //pen.y = 0;

  //FT_Set_Transform( m_Face, &matrix, &pen );
  //m_aFontTex = new GTextureArray;

  IntCreateTexture();

  return TRUE;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT _GFTFont::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT _GFTFont::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT _GFTFont::Invoke(GRESCRIPTDESC* pDesc)
{
  INVOKE_DESC_CHECK(pDesc);
  // TODO: 设备重置处理属于平台相关的处理,应该放到D3D9对象中
  switch(pDesc->dwCmdCode)
  {
  case RC_ResetDevice:
    ASSERT(m_aFontTex.size() == 1);
    break;
  case RC_LostDevice:
    if(m_aFontTex.size() > 1)
    {
      // 清除纹理缓冲阵列,只保留一个纹理以省掉第一次重建
      for(GTextureArray::iterator it = m_aFontTex.begin() + 1;
        it != m_aFontTex.end(); ++it)
        (*it)->Release();
      m_aFontTex.erase(m_aFontTex.begin() + 1, m_aFontTex.end());
    }
    m_pmapCode2FMatrix->clear();
    m_ptPen.x = 0;
    m_ptPen.y = 0;
    break;
  case RC_ResizeDevice:
    break;
  }
  return GX_OK;
}

GXBOOL _GFTFont::GetDescW(GXLPLOGFONTW lpLogFont) const
{
  lpLogFont->lfHeight          = m_LogFont.lfHeight;
  lpLogFont->lfWidth            = m_LogFont.lfWidth;
  lpLogFont->lfEscapement      = m_LogFont.lfEscapement;
  lpLogFont->lfOrientation    = m_LogFont.lfOrientation;
  lpLogFont->lfWeight          = m_LogFont.lfWeight;
  lpLogFont->lfItalic          = m_LogFont.lfItalic;
  lpLogFont->lfUnderline        = m_LogFont.lfUnderline;
  lpLogFont->lfStrikeOut        = m_LogFont.lfStrikeOut;
  lpLogFont->lfCharSet          = m_LogFont.lfCharSet;
  lpLogFont->lfOutPrecision    = m_LogFont.lfOutPrecision;
  lpLogFont->lfClipPrecision    = m_LogFont.lfClipPrecision;
  lpLogFont->lfQuality          = m_LogFont.lfQuality;
  lpLogFont->lfPitchAndFamily  = m_LogFont.lfPitchAndFamily;
  gxMultiByteToWideChar(GXCP_ACP, 0, m_LogFont.lfFaceName, -1, lpLogFont->lfFaceName, GXLF_FACESIZE);
  return TRUE;
}
GXBOOL _GFTFont::GetDescA(GXLPLOGFONTA lpLogFont) const
{
  *lpLogFont = m_LogFont;
  return TRUE;
}

_GFTFont::_GFTFont(GrapX::Graphics* pGraphics, const GXLPLOGFONTA lpLogFont)
  : GXFont        ()
  //, m_pFaceName   (NULL)
  , m_pGraphics   (pGraphics)
  , m_Library     (NULL)
  , m_Face        (NULL)
  , m_nWidth      (0)
  , m_nHeight     (0)
  , m_pmapCode2FMatrix(NULL)
{
  m_LogFont = *lpLogFont;
  m_ptPen.x = 0;
  m_ptPen.y = 0;

  AddRef();
}


_GFTFont::~_GFTFont()
{
  for(GTextureArray::iterator it = m_aFontTex.begin(); 
    it != m_aFontTex.end(); ++it)
  {
    //GXWCHAR buffer[1024];
    //wsprintfW(buffer, L"%08x.png", (*it));
    //(*it)->SaveToFile(buffer, L"png");
    SAFE_RELEASE(*it);
  }
  m_aFontTex.clear();
  //SAFE_DELETE(m_aFontTex);

  SAFE_DELETE(m_pmapCode2FMatrix);
  //SAFE_DELETE(m_pFaceName);
  if(m_Face) {
    FT_Done_Face(m_Face);
    m_Face = NULL;
  }

  if(m_Library) {
    FT_Done_FreeType(m_Library);
    m_Library = NULL;
  }

  // m_strFaceName 为空说明没用创建成功，此时也没有注册到管理器上
  if(m_strFaceName.IsNotEmpty()) {
    m_pGraphics->UnregisterResource(this);
  }
}


GXBOOL _GFTFont::IntCreateTexture()
{
  GrapX::GTexture* pTexture = NULL;

  //pTexture = GXCreateTexture(DEFAULT_FONT_TEX_SIZE_X, DEFAULT_FONT_TEX_SIZE_Y, 
  //  1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT);
  if(GXFAILED(m_pGraphics->CreateTexture(&pTexture, NULL, DEFAULT_FONT_TEX_SIZE_X, DEFAULT_FONT_TEX_SIZE_Y, 
    GXFMT_A8, GXResUsage::Default, 1)))
  {
    CLBREAK;
    return FALSE;
  }
#if 0
  GTexture::MAPPEDRECT mapped;
  pTexture->MapRect(&mapped, NULL, GXResMap::Write);
  memset(mapped.pBits, 0x80, DEFAULT_FONT_TEX_SIZE_X * DEFAULT_FONT_TEX_SIZE_Y);
  pTexture->UnmapRect();
#else
  //pTexture->Clear(0x80808080);
#endif

  m_aFontTex.push_back(pTexture);
  //pTexture->AddRef();
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
GXVOID _GFTFont::UpdateTexBuffer(GXUINT idxTex, LPGXREGN prgDest, unsigned char* pBuffer)
{
  //GTexture::MAPPEDRECT mapped;
  GXRECT rect;
  ASSERT(prgDest->width > 0 && prgDest->height > 0);
  gxRegnToRect((GXRECT*)&rect, prgDest);
  GrapX::GTexture* pTexture = m_aFontTex[idxTex];

#if 0
  pTexture->MapRect(&mapped, &rect, GXResMap::Write);

  unsigned char* pBits = (unsigned char*)mapped.pBits;
  for(GXLONG i = 0; i < prgDest->height; i++)
  {
    memcpy(pBits, (const void*)&pBuffer[i * prgDest->width], prgDest->width);
    pBits += mapped.Pitch;
  }
  pTexture->UnmapRect();
#else
  pTexture->UpdateRect(&rect, pBuffer, prgDest->width);
#endif
}

GXBOOL _GFTFont::QueryCharDescFromCache(GXWCHAR ch, LPCHARDESC pCharDesc)
{
  CharToFontDesc::iterator it = m_pmapCode2FMatrix->find(ch);
  if(it == m_pmapCode2FMatrix->end())
    return FALSE;

  FTFONTDESC &fm = it->second;
  FontDescToCharDesc(fm, pCharDesc);
  return TRUE;
}

GXINT _GFTFont::QueryCharDesc(GXWCHAR ch, LPCHARDESC pCharDesc)
{
  if(QueryCharDescFromCache(ch, pCharDesc) != FALSE)
    return TRUE;
  else
  {
    FT_Error fterr;
    fterr = FT_Load_Char(m_Face, ch, FT_LOAD_RENDER|FT_LOAD_FORCE_AUTOHINT|
      FT_LOAD_TARGET_NORMAL);
    ASSERT(fterr == 0);

    FT_Glyph glyph;
    FT_Get_Glyph(m_Face->glyph, &glyph );
    ASSERT(fterr == 0);

    FT_Render_Glyph(m_Face->glyph, FT_RENDER_MODE_NORMAL); 
    fterr = FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1);
    ASSERT(fterr == 0);

    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;

    GXREGN rgDest;
    long Ascender = m_Face->size->metrics.ascender >> 6;

    if(m_ptPen.x + (bitmap.width + FONT_INFLATE) >= DEFAULT_FONT_TEX_SIZE_X)
    {
      m_ptPen.x = 0;
      m_ptPen.y += m_nHeight;
    }
    if(m_ptPen.y + m_nHeight >= DEFAULT_FONT_TEX_SIZE_Y)
    {
      if(IntCreateTexture() == FALSE)
        return FALSE;
      m_ptPen.y = 0;
    }

    FTFONTDESC fm;
    fm.idxTex    = (unsigned char)(m_aFontTex.size() - 1);
    fm.xGrid     = (unsigned short)(m_ptPen.x);
    fm.yGrid     = (unsigned short)(m_ptPen.y);
    fm.nWidth    = bitmap.width;
    fm.nHeight   = bitmap.rows;
    fm.xPos      = bitmap_glyph->left;
    fm.yPos      = (short)(Ascender - bitmap_glyph->top);
    fm.nAdvWidth = (unsigned short)(glyph->advance.x >> 16);

    m_ptPen.x += (bitmap.width + FONT_INFLATE);

    (*m_pmapCode2FMatrix)[ch] = fm;

    ASSERT(bitmap.pitch == bitmap.width);
    rgDest.left   = fm.xGrid;
    rgDest.top    = fm.yGrid;
    rgDest.width  = bitmap.width;
    rgDest.height = bitmap.rows;

    if(rgDest.width > 0 && rgDest.height > 0)
      UpdateTexBuffer(fm.idxTex, &rgDest, bitmap.buffer);

    FontDescToCharDesc(fm, pCharDesc);
    FT_Done_Glyph( glyph );
  }     
  return TRUE;
}

GXINT _GFTFont::QueryCharWidth(GXWCHAR ch)
{
  FT_Glyph  glyph;

  CharToFontDesc::iterator it = m_pmapCode2FMatrix->find(ch);
  if(it != m_pmapCode2FMatrix->end())
  {
    return (it->second.nAdvWidth);
  }
  if(FT_Load_Char(m_Face, ch, FT_LOAD_RENDER|FT_LOAD_FORCE_AUTOHINT|FT_LOAD_TARGET_NORMAL) != 0 || 
    FT_Get_Glyph( m_Face->glyph, &glyph ) != 0)
    return 0L;
  GXINT nRet = (glyph->advance.x >> 16);
  FT_Done_Glyph(glyph);
  return nRet;
}

GXLONG _GFTFont::GetMetricsHeight() const
{
  return m_Face->size->metrics.height >> 6;
}
GXLONG _GFTFont::GetWidth() const
{
  return m_nWidth - FONT_INFLATE;
}

GXLONG _GFTFont::GetHeight() const
{
  return m_nHeight - FONT_INFLATE;
}

GXBOOL _GFTFont::GetMetricW(GXLPTEXTMETRICW lptm) const
{
  FT_WinFNT_HeaderRec WinFntHeaderRec;
  memset(&WinFntHeaderRec, 0, sizeof(WinFntHeaderRec));
  //FT_Get_WinFNT_Header(m_Face, &WinFntHeaderRec);


  lptm->tmHeight          = m_Face->size->metrics.height >> 6;
  lptm->tmExternalLeading = 0;    // FIXME;
  lptm->tmAveCharWidth    = m_Face->size->metrics.height >> 6;    // FIXME;
  lptm->tmMaxCharWidth    = m_Face->size->metrics.max_advance >> 6;
#ifndef _DEV_DISABLE_UI_CODE
  lptm->tmPitchAndFamily  = GXTMPF_TRUETYPE;    // FIXME
#else
  lptm->tmPitchAndFamily = 1;
#endif // _DEV_DISABLE_UI_CODE

  //lptm->tmHeight        = m_nHeight;  // TODO: 这个值不太对 WinFntHeaderRec.pixel_height;
  //lptm->tmAscent        = WinFntHeaderRec.ascent;
  //lptm->tmDescent        ;
  //lptm->tmInternalLeading    = WinFntHeaderRec.internal_leading;
  //lptm->tmExternalLeading    = WinFntHeaderRec.external_leading;
  //lptm->tmAveCharWidth          = WinFntHeaderRec.avg_width;
  //lptm->tmMaxCharWidth          = WinFntHeaderRec.max_width;
  //lptm->tmWeight        = WinFntHeaderRec.weight;
  //lptm->tmOverhang            ;
  //lptm->tmDigitizedAspectX  ;
  //lptm->tmDigitizedAspectY  ;
  //lptm->tmFirstChar      = WinFntHeaderRec.first_char;
  //lptm->tmLastChar              = WinFntHeaderRec.last_char;
  //lptm->tmDefaultChar      = WinFntHeaderRec.default_char;
  //lptm->tmBreakChar      = WinFntHeaderRec.break_char;
  //lptm->tmItalic        = WinFntHeaderRec.italic;
  //lptm->tmUnderlined      = WinFntHeaderRec.underline;
  //lptm->tmStruckOut      = WinFntHeaderRec.strike_out;
  //lptm->tmPitchAndFamily    = WinFntHeaderRec.pitch_and_family;
  //lptm->tmCharSet        = WinFntHeaderRec.charset;
  return TRUE;
}

GXLPVOID _GFTFont::GetTexture(GXUINT idx) const
{
  if(idx >= m_aFontTex.size())
    return NULL;
  return (GXLPVOID)m_aFontTex[idx];
}

//////////////////////////////////////////////////////////////////////////
