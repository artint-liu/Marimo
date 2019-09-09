//
// Sprite Manufacture
// Sprite 生产类函数
//
// 全局头文件
#include <GrapX.h>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXSprite.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXKernel.h"

// 私有头文件
#include <clPathFile.h>
#include <clUtility.h>
#include <clStringSet.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStock.h>
#include <Canvas/GXSpriteImpl.h>

using namespace clstd;
//////////////////////////////////////////////////////////////////////////
extern "C" GXBOOL GXDLLAPI gxSetRegn(GXLPREGN lprg, GXINT xLeft, GXINT yTop, GXINT xWidth, GXINT yHeight);
GXHRESULT IntLoadSpriteDesc(clstd::StockA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc);
//////////////////////////////////////////////////////////////////////////

GXHRESULT GXDLLAPI GXCreateSprite(GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount, GXSprite** ppSprite)
{
  *ppSprite = NULL;
  GXSpriteImpl* pSpriteImpl = new GXSpriteImpl;
  if( ! InlCheckNewAndIncReference(pSpriteImpl)) {
    return GX_FAIL;
  }

  if( ! pSpriteImpl->Initialize(pGraphics, szTextureFile, arrayRegion, nCount))
  {
    SAFE_RELEASE(pSpriteImpl);
    return GX_FAIL;
  }

  *ppSprite = pSpriteImpl;
  return GX_OK;
}

extern "C" GXHRESULT GXDLLAPI GXCreateSpriteEx(GrapX::Graphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite)
{
  GXSpriteImpl* pSpriteImpl = new GXSpriteImpl;
  if( ! InlCheckNewAndIncReference(pSpriteImpl)) {
    return GX_ERROR_OUROFMEMORY;
  }

  if( ! pSpriteImpl->Initialize(pGraphics, pDesc)) {
    SAFE_RELEASE(pSpriteImpl);
    return GX_FAIL;
  }

  *ppSprite = pSpriteImpl;
  return GX_OK;
}

GXHRESULT GXDLLAPI GXCreateSpriteArray(GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite)
{
  GrapX::Texture* pTexture = NULL;
  if(GXFAILED(pGraphics->CreateTextureFromFile(&pTexture, szTextureFile, GXResUsage::Default))) {
    return GX_E_OPEN_FAILED;
  }
  //int nWidth  = pTexture->GetWidth();
  //int nHeight = pTexture->GetHeight();
  GXSIZE sDimension;
  pTexture->GetDimension(&sDimension);

  clvector<GXREGN> aRegn;
  GXREGN regn;

  for(int y = yStart; y <= sDimension.cy - nTileHeight; y += nTileHeight + yGap)
  {
    for(int x = xStart; x <= sDimension.cx - nTileWidth; x += nTileWidth + xGap)
    {
      gxSetRegn(&regn, x, y, nTileWidth, nTileHeight);
      aRegn.push_back(regn);
    }
  }
  GXHRESULT hval = GXCreateSprite(pGraphics, szTextureFile, &aRegn.front(), aRegn.size(), ppSprite);
  SAFE_RELEASE(pTexture);
  return hval;
}

GXHRESULT GXDLLAPI GXCreateSpriteFromFileW(GrapX::Graphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite)
{
  clstd::StockA ss;
  GXHRESULT hval = GX_FAIL;

  if( ! ss.LoadFromFile(szSpriteFile)) {
    CLOG_ERRORW(_CLTEXT("%s : Can not open file(\"%s\")."), __FUNCTIONW__, szSpriteFile);
    return GX_E_OPEN_FAILED;
  }
  
  //
  // sprite 支持[Sprite]和[SpriteArray]两种格式的描述
  // 目前为止，如果两中格式储存在同一文件中优先读取[SpriteArray]的数据
  // 未来不排除增加同时读取两种格式并合并为同一个Sprite对象的可能
  //

  clstd::StockA::Section hSpriteArray = ss.OpenSection("SpriteArray");
  if(hSpriteArray)
  {
    clStringW strImageFile;
    strImageFile    = hSpriteArray.GetKeyAsString("File", "");
    int xStart      = hSpriteArray.GetKeyAsInteger("Left", 0);
    int yStart      = hSpriteArray.GetKeyAsInteger("Top", 0);
    int nTileWidth  = hSpriteArray.GetKeyAsInteger("TileWidth", 0);
    int nTileHeight = hSpriteArray.GetKeyAsInteger("TileHeight", 0);
    int nGapWidth   = hSpriteArray.GetKeyAsInteger("GapWidth", 0);
    int nGapHeight  = hSpriteArray.GetKeyAsInteger("GapHeight", 0);

    clStringW strImageFileW = szSpriteFile;//;
    strImageFileW.Replace(clpathfile::ViceSlash(), clpathfile::Slash());
    clpathfile::RemoveFileSpec(strImageFileW);
    clpathfile::CombinePath(strImageFileW, strImageFileW, clStringW(strImageFile));
    hval = GXCreateSpriteArray(pGraphics, strImageFileW, xStart, yStart, nTileWidth, nTileHeight, nGapWidth, nGapHeight, ppSprite);
    //ss.FindClose(hSpriteArray);
  } // if(hSpriteArray)
  else
  {
    GXSpriteDesc* pSpriteDesc = NULL;
    hval = IntLoadSpriteDesc(ss, szSpriteFile, &pSpriteDesc);
    if(GXSUCCEEDED(hval)) {
      GXSPRITE_DESCW sDesc = pSpriteDesc->ToDesc();
      hval = GXCreateSpriteEx(pGraphics, &sDesc, ppSprite);
      if (GXFAILED(hval)) {
        CLOG_ERRORW(_CLTEXT("%s : (\"%s\").\n"), __FUNCTIONW__, szSpriteFile);
      }
    }
    SAFE_DELETE(pSpriteDesc);
  }
  return hval;
}


GXHRESULT IntLoadModules(StockA& ss, GXSpriteDescImpl* pDescObj )
{
  GXHRESULT hval = GX_OK;
  // 创建对象
  //*ppSprite = NULL;
  //GXSpriteImpl* pSprite = new GXSpriteImpl;
  //if( ! InlCheckNewAndIncReference(pSprite)) {
  //  return GX_FAIL;
  //}

  //pSprite->m_pImage = pGraphics->CreateImageFromFile(pDescObj->m_strImageFile);
  //pSprite->m_strImageFile = strImageFile;

  //if( ! pSprite->m_pImage) {
  //  // TODO: 改为RemoveFileSpec
  //  clStringW strRelImagePath = szSpriteFile;
  //  strRelImagePath.Replace(clpathfile::ViceSlash(), clpathfile::Slash());
  //  clsize nFilename = clpathfile::FindFileNameW(strRelImagePath);
  //  strRelImagePath.Replace(nFilename, -1, strImageFile);
  //  pSprite->m_pImage = pGraphics->CreateImageFromFile(strRelImagePath);
  //}

  //clvector<REGN> aRegns;
  StockA::Section hModule = ss.OpenSection("Sprite/Module/rect");


  // 参考TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if( ! hModule) {
    hModule = ss.OpenSection("Image/Module/rect");
  }


  if(hModule)
  {
    GXSprite::MODULE sModule;
    clStringA strModuleName;
    StockA::ATTRIBUTE valueLeft;
    StockA::ATTRIBUTE valueTop;
    StockA::ATTRIBUTE valueRight;
    StockA::ATTRIBUTE valueBottom;

    do 
    {
      //sSprite.name.Clear();
      strModuleName.Clear();
      sModule.name = NULL;
      sModule.id   = 0;

      if( hModule.GetKey("left", valueLeft) == TRUE &&
        hModule.GetKey("top", valueTop) == TRUE &&
        hModule.GetKey("right", valueRight) == TRUE &&
        hModule.GetKey("bottom", valueBottom) == TRUE)
      {
        sModule.regn.left   = valueLeft.ToInt();
        sModule.regn.top    = valueTop.ToInt();
        sModule.regn.width  = valueRight.ToInt() - sModule.regn.left;
        sModule.regn.height = valueBottom.ToInt() - sModule.regn.top;

        strModuleName = hModule.GetKeyAsString("name", "");

        // 如果名字不为空则加入到集合中并赋给结构体
        if(strModuleName.IsNotEmpty()) {
          sModule.name = pDescObj->m_NameSet.add(strModuleName);
        }

        pDescObj->m_aModules.push_back(sModule);
      }
    } while(hModule.NextSection());
    //*ppSprite = pSprite;
    hval = GX_OK;
  }
  //ss.FindClose(hModule);
  return hval;
}

GXHRESULT IntLoadFrames(StockA& ss, GXSpriteDescImpl* pDescObj ) 
{
  GXHRESULT hval = GX_OK;
  StockA::Section hFrame = ss.OpenSection("Sprite/Frame");

  // 参考TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if( ! hFrame) {
    hFrame = ss.OpenSection("Image/Frame");
  }

  if(hFrame)
  {
    StockA::ATTRIBUTE valueName;

    auto& aFrames = pDescObj->m_aFrames;
    auto& aFrameModulesArray = pDescObj->m_aFrameModules;
    GXSprite::FRAME sFrame = {0};

    do {
      sFrame.name = NULL;
      sFrame.start = (GXINT)aFrameModulesArray.size();

      if(hFrame.GetKey("name", valueName)) {
        sFrame.name = pDescObj->m_NameSet.add(valueName.ToString());
      }

      sFrame.id = hFrame.GetKeyAsInteger("id", 0);

      StockA::Section hFrameModule = hFrame.Open("module");
      if(hFrameModule)
      {
        GXSprite::FRAME_MODULE sFrameModule = {0};
        do {

          sFrameModule.nModuleIdx = hFrameModule.GetKeyAsInteger("index", -1);
          sFrameModule.rotate     = hFrameModule.GetKeyAsInteger("flags", -1);
          sFrameModule.offset.x   = hFrameModule.GetKeyAsInteger("x", 0);
          sFrameModule.offset.y   = hFrameModule.GetKeyAsInteger("y", 0);

          TRACE("index:%u flags:%lu x:%ld y:%ld\n", sFrameModule.nModuleIdx, sFrameModule.rotate, sFrameModule.offset.x, sFrameModule.offset.y);
          // push back
          aFrameModulesArray.push_back(sFrameModule);

        } while(hFrameModule.NextSection());
        //ss.FindClose(hFrameModule);
      }

      // push back
      sFrame.count = (GXINT)aFrameModulesArray.size() - sFrame.start;
      aFrames.push_back(sFrame);
      sFrame.start = (GXINT)aFrameModulesArray.size();

    }while(hFrame.NextSection());

    //ss.FindClose(hFrame);
  }
  return hval;
}

GXHRESULT IntLoadAnimations(StockA &ss, GXSpriteDescImpl* pDescObj ) 
{
  GXHRESULT hval = GX_OK;
  StockA::Section hAnim = ss.OpenSection("Sprite/Animation");

  // 参考TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if( ! hAnim) {
    hAnim = ss.OpenSection("Image/Animation");
  }

  if(hAnim)
  {
    StockA::ATTRIBUTE valueName;
    GXSprite::ANIMATION sAnimation = {0};

    auto& aAnimations = pDescObj->m_aAnimations;
    auto& aAnimFrames = pDescObj->m_aAnimFrames;

    do {
      sAnimation.name = NULL;
      if(hAnim.GetKey("name", valueName)) {
        sAnimation.name = pDescObj->m_NameSet.add(valueName.ToString());
      }

      sAnimation.rate  = hAnim.GetKeyAsInteger("rate", 0);
      sAnimation.id    = hAnim.GetKeyAsInteger("id", 0);
      sAnimation.start = (GXINT)aAnimFrames.size();
      sAnimation.count = hAnim.GetKeyAsInteger("count", 0);
      aAnimFrames.reserve(aAnimFrames.size() + sAnimation.count);

      clStringA strFrames = hAnim.GetKeyAsString("frames", "");
      clStringArrayA aFrameList;
      if(strFrames.IsNotEmpty())
      {
        clstd::ResolveString(strFrames, ',', aFrameList);

        // 如果结尾为空则去掉
        while(aFrameList.back().IsEmpty()) {
          aFrameList.erase(aFrameList.end() - 1);
        }
      }

      // 检查文件记录与实际读入的数量是否相符
      if(aFrameList.size() != sAnimation.count) {
        CLOG_WARNING("%s : Number of animation frames is not match the record.(%s,%d)\r\n", __FUNCTION__,
          sAnimation.name != NULL ? sAnimation.name : "", sAnimation.id);
      }

      for(auto it = aFrameList.begin(); it != aFrameList.end(); ++it)
      {
        aAnimFrames.push_back(it->ToInteger());
      }

      // push back
      sAnimation.count = (GXINT)aAnimFrames.size() - sAnimation.start;
      aAnimations.push_back(sAnimation);
      sAnimation.start = (GXINT)aAnimFrames.size();

    }while(hAnim.NextSection());

    //ss.FindClose(hAnim);
  }
  return hval;
}

GXHRESULT IntLoadSpriteDesc(StockA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  GXSpriteDescImpl* pDescObj = NULL;
  GXHRESULT hval = GX_OK;

  pDescObj = new GXSpriteDescImpl;
  if(pDescObj == NULL) {
    CLOG_ERROR("%s : Out of memory!\r\n", __FUNCTION__);
    return GX_ERROR_OUROFMEMORY;
  }


  StockA::Section hSprite = ss.OpenSection("Sprite");

  // TODO: 之前Sprite描述定义在Image段, 后来改为Sprite
  // 这里用了兼容写法，如果Sprite段不存在则尝试读取就的Image段定义
  // 以后去掉
  // TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if( ! hSprite) {
    hSprite = ss.OpenSection("Image");
  }


  if(hSprite)
  {
    //
    // 读取纹理文件名
    //
    StockA::ATTRIBUTE valueFile;
    if(hSprite)
    {
      if(hSprite.GetKey("File", valueFile) == TRUE) {
        clStringW strSpriteDir = szSpriteFile;
        clpathfile::RemoveFileSpec(strSpriteDir);
        pDescObj->m_strImageFile = AS2WS(valueFile.ToString());
        clpathfile::CombinePath(pDescObj->m_strImageFile, strSpriteDir, pDescObj->m_strImageFile);
      }
    }
    //ss.FindClose(hSprite);

    // 读取Module数据
    IntLoadModules(ss, pDescObj);

    // 读取 Frame
    IntLoadFrames(ss, pDescObj);

    // 读取 Animation
    IntLoadAnimations(ss, pDescObj);

  } // if(hSprite != NULL)



  *ppDesc = pDescObj;

  return hval;
}

GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  StockA ss;

  if( ! ss.LoadFromFile(szSpriteFile)) {
    return GX_E_OPEN_FAILED;
  }
  
  return IntLoadSpriteDesc(ss, szSpriteFile, ppDesc);
}

GXHRESULT GXDLLAPI GXCreateSpriteFromFileA(GrapX::Graphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite)
{
  return GXCreateSpriteFromFileW(pGraphics, AS2WS(szSpriteFile), ppSprite);
}

GXHRESULT GXDLLAPI GXLoadSpriteDescA(GXLPCSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  return GXLoadSpriteDescW(AS2WS(szSpriteFile), ppDesc);
}

GXSpriteDescImpl::~GXSpriteDescImpl()
{
  // 这个用来保证析构函数在特定模块里调用
}

GXSPRITE_DESCW GXSpriteDescImpl::ToDesc()
{
  GXSPRITE_DESCW sDesc = {0};
  sDesc.cbSize = sizeof(GXSPRITE_DESCW);
  sDesc.szImageFile = m_strImageFile;

  sDesc.nNumOfModules = (GXUINT)m_aModules.size();
  sDesc.aModules = &m_aModules.front();


  sDesc.nNumOfFrames = (GXUINT)m_aFrames.size();
  if(sDesc.nNumOfFrames) {
    sDesc.aFrames = &m_aFrames.front();
  }

  sDesc.nNumOfFrameModules = (GXUINT)m_aFrameModules.size();
  if(sDesc.nNumOfFrameModules) {
    sDesc.aFrameModules = &m_aFrameModules.front();
  }


  sDesc.nNumOfAnimations = (GXUINT)m_aAnimations.size();
  if(sDesc.nNumOfAnimations) {
    sDesc.aAnimations = &m_aAnimations.front();
  }

  sDesc.nNumOfAnimFrames = (GXUINT)m_aAnimFrames.size();
  if(sDesc.nNumOfAnimFrames) {
    sDesc.aAnimFrames = &m_aAnimFrames.front();
  }

  return sDesc;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL IntCheckSpriteDesc(const GXSPRITE_DESCW* pDesc)
{
  if(pDesc->cbSize != sizeof(GXSPRITE_DESCW)) {
    return FALSE;
  }
  else if(pDesc->aModules == NULL && (pDesc->aFrames != NULL || pDesc->aAnimations != NULL)) {
    return FALSE;
  }
  else if(pDesc->aFrames == NULL && pDesc->aAnimations != NULL) {
    return FALSE;
  }

  for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
  {
    GXUINT nEndModule = pDesc->aFrames[i].start + pDesc->aFrames[i].count;
    if(nEndModule > pDesc->nNumOfFrameModules) {
      CLOG_ERROR("Frame-desc index out of range.\r\n");
      return FALSE;
    }
  }

  for(GXUINT i = 0; i < pDesc->nNumOfFrameModules; i++)
  {
    if(pDesc->aFrameModules[i].nModuleIdx >= pDesc->nNumOfModules) {
      CLOG_ERROR("Module index in frame-desc out of range.\r\n");
      return FALSE;
    }
  }

  for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
  {
    GXUINT nEndFrame = pDesc->aAnimations[i].start + pDesc->aAnimations[i].count;
    if(nEndFrame > pDesc->nNumOfAnimFrames) {
      CLOG_ERROR("Animation index out of range.\r\n");
      return FALSE;
    }
  }

  for(GXUINT i = 0; i < pDesc->nNumOfAnimFrames; i++)
  {
    if(pDesc->aAnimFrames[i] >= pDesc->nNumOfFrames) {
      CLOG_ERROR("Frame index out of range.\r\n");
      return FALSE;
    }
  }
  return TRUE;
}

template<class _DESCT>
void IntSaveSpriteNamdAndId(clFile& file, int nTab, const _DESCT& d)
{
  if(d.id != 0) {
    file.Writef("%*sid=\"%d\";\r\n", nTab, "", d.id);
  }
  if(d.name != NULL && d.name[0] != '\0') {
    file.Writef("%*sname=\"%s\";\r\n", nTab, "", d.name);
  }
}

GXBOOL GXDLLAPI ___GXSaveSpriteToFileW___(GXLPCWSTR szFilename, const GXSPRITE_DESCW* pDesc)
{
  clFile file;

  if(pDesc->szImageFile == NULL || GXSTRLEN(pDesc->szImageFile) == 0) {
    CLOG_ERROR("%s : Texture filename is empty.\r\n", __FUNCTION__);
    return FALSE;
  }

  if( ! IntCheckSpriteDesc(pDesc)) {
    CLOG_ERROR("%s : Bad input argument.\r\n", __FUNCTION__);
  }

  if( ! file.CreateAlways(szFilename)) {
    CLOG_ERROR("%s : Can't create file.\r\n", __FUNCTION__);
    return FALSE;
  }

  file.Writef("smart{version = \"0.0.1.0\";}\r\n");
  file.Writef("Sprite {\r\n"); // <Sprite>

  clStringW strRelativePath;
  clpathfile::RelativePathTo(strRelativePath, szFilename, FALSE, pDesc->szImageFile, FALSE);
  clStringA strFilenameA(strRelativePath);
  file.Writef("  File=\"%s\";\r\n", (const char*)strFilenameA);
  file.Writef("  Module {\r\n"); // <Module>

  //
  // Module
  //
  if(pDesc->aModules != NULL) // 不需要判断pDesc->nNumOfModules，下面循环判断了
  {
    for(GXUINT i = 0; i < pDesc->nNumOfModules; i++)
    {
      GXSprite::MODULE& m = pDesc->aModules[i];
      file.Writef("    rect {\r\n"); // <rect>
      IntSaveSpriteNamdAndId(file, 6, m);
      file.Writef("      left=%d; top=%d;\r\n", m.regn.left, m.regn.top);
      file.Writef("      right=%d; bottom=%d;\r\n", m.regn.left + m.regn.width, m.regn.top + m.regn.height);
      file.Writef("    };\r\n"); // </rect>
    }
  }
  file.Writef("  };\r\n"); // </Module>

  //
  // Frame列表信息
  // 
  if(pDesc->aFrames != NULL && pDesc->nNumOfFrameModules > 0 && pDesc->aFrameModules != NULL)
  {
    for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
    {
      GXSprite::FRAME& f = pDesc->aFrames[i];
      file.Writef("  Frame {\r\n"); // <frame>
      IntSaveSpriteNamdAndId(file, 4, f);
      //file.Writef("    start=\"%d\"; count=\"%d\";\r\n", f.start, f.count);

      // Frame描述信息
      for(GXUINT n = 0; n < f.count; n++)
      {
        GXSprite::FRAME_MODULE& fd = pDesc->aFrameModules[n + f.start];
        file.Writef("    module {\r\n");
        file.Writef("      index=\"%d\"; flags=\"%d\"; x=\"%d\"; y=\"%d\";\r\n",
          fd.nModuleIdx, fd.rotate, fd.offset.x, fd.offset.y);
        file.Writef("    };\r\n");
      }
      file.Writef("  };\r\n"); // </frame>
    }
  }

  //
  // Animation列表信息
  //
  if(pDesc->aAnimations != NULL && pDesc->nNumOfAnimFrames > 0 && pDesc->aAnimFrames != NULL)
  {
    clStringA strFrameList;
    for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
    {
      GXSprite::ANIMATION& a = pDesc->aAnimations[i];
      file.Writef("    Animation {\r\n"); // <animation>
      IntSaveSpriteNamdAndId(file, 6, a);
      file.Writef("      rate=\"%d\"; start=\"%d\"; count=\"%d\";\r\n", a.rate, a.start, a.count);

      strFrameList.Clear();
      for(GXUINT n = 0; n < a.count; n++) {
        strFrameList.AppendFormat("%d,", pDesc->aAnimFrames[n + a.start]);
      }
      strFrameList.TrimRight(',');
      file.Writef("      frames=\"%s\"\r\n", (const char*)strFrameList);
      file.Writef("    };\r\n"); // </animation>
    }
  }

  file.Writef("};\r\n"); // </Sprite>
  return TRUE;

}

GXBOOL GXDLLAPI GXSaveSpriteToFileW(GXLPCWSTR szFilename, const GXSPRITE_DESCW* pDesc)
{
  //clFile file;
  clStockA stock;

  if(pDesc->szImageFile == NULL || GXSTRLEN(pDesc->szImageFile) == 0) {
    CLOG_ERROR("%s : Texture filename is empty.\r\n", __FUNCTION__);
    return FALSE;
  }

  if( ! IntCheckSpriteDesc(pDesc)) {
    CLOG_ERROR("%s : Bad input argument.\r\n", __FUNCTION__);
  }

  clStockA::Section pSmartSection = stock.CreateSection("smart");
  pSmartSection.SetKey("version", "0.0.1.0");
  //stock.CloseSection(pSmartSection);

  auto pSpriteSection = stock.CreateSection("Sprite");

  clStringW strRelativePath;
  clpathfile::RelativePathTo(strRelativePath, szFilename, FALSE, pDesc->szImageFile, FALSE);
  clStringA strFilenameA(strRelativePath);

  pSpriteSection.SetKey("File", strFilenameA);
  //stock.CloseSection(pSpriteSection);

  //
  // Module
  //
  if(pDesc->aModules != NULL) // 不需要判断pDesc->nNumOfModules，下面循环判断了
  {
    for(GXUINT i = 0; i < pDesc->nNumOfModules; i++)
    {
      GXSprite::MODULE& m = pDesc->aModules[i];
      auto pRectSection = stock.CreateSection("Sprite/Module/rect");
      pRectSection.SetKey("name",    m.name);
      pRectSection.SetKey("id",      clStringA(m.id));
      pRectSection.SetKey("left",    clStringA(m.regn.left));
      pRectSection.SetKey("top",     clStringA(m.regn.top));
      pRectSection.SetKey("right",   clStringA(m.regn.left + m.regn.width));
      pRectSection.SetKey("bottom",  clStringA(m.regn.top + m.regn.height));
      //stock.CloseSection(pRectSection);
    }
  }

  //
  // Frame列表信息
  // 
  if(pDesc->aFrames != NULL && pDesc->nNumOfFrameModules > 0 && pDesc->aFrameModules != NULL)
  {
    for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
    {
      GXSprite::FRAME& f = pDesc->aFrames[i];
      auto pFrameSection = stock.CreateSection("Sprite/Frame");
      pFrameSection.SetKey("name",   f.name);
      pFrameSection.SetKey("id",     clStringA(f.id));
      pFrameSection.SetKey("start",  clStringA(f.start));
      pFrameSection.SetKey("count",  clStringA(f.count));

      // Frame描述信息
      for(GXUINT n = 0; n < f.count; n++)
      {
        GXSprite::FRAME_MODULE& fd = pDesc->aFrameModules[n + f.start];
        clStockA::Section pFrameModuleSection = stock.CreateSection(&pFrameSection, "module");
        pFrameModuleSection.SetKey("index",  clStringA(fd.nModuleIdx));
        pFrameModuleSection.SetKey("flags",  clStringA(fd.rotate));
        pFrameModuleSection.SetKey("x",      clStringA(fd.offset.x));
        pFrameModuleSection.SetKey("y",      clStringA(fd.offset.y));
        //stock.CloseSection(pFrameModuleSection);
      }
      //stock.CloseSection(pFrameSection);
      //file.Writef("  };\r\n"); // </frame>
    }
  }

  //
  // Animation列表信息
  //
  if(pDesc->aAnimations != NULL && pDesc->nNumOfAnimFrames > 0 && pDesc->aAnimFrames != NULL)
  {
    clStringA strFrameList;
    for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
    {
      GXSprite::ANIMATION& a = pDesc->aAnimations[i];
      auto pAnimSection = stock.CreateSection("Sprite/Animation");
      pAnimSection.SetKey("name",   a.name);
      pAnimSection.SetKey("id",     clStringA(a.id));
      pAnimSection.SetKey("rate",   clStringA(a.rate));
      pAnimSection.SetKey("start",  clStringA(a.start));
      pAnimSection.SetKey("count",  clStringA(a.count));

      strFrameList.Clear();
      for(GXUINT n = 0; n < a.count; n++) {
        strFrameList.AppendFormat("%d,", pDesc->aAnimFrames[n + a.start]);
      }
      pAnimSection.SetKey("frames", strFrameList);
      //stock.CloseSection(pAnimSection);
    }
  }

  if( ! stock.SaveToFile(szFilename))
  {
    CLOG_ERROR("%s : Can not save file(%s).\r\n", __FUNCTION__, (const char*)szFilename);
    return FALSE;
  }

  return TRUE;
}
//////////////////////////////////////////////////////////////////////////