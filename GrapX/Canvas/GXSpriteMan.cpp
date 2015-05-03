//
// Sprite Manufacture
// Sprite �����ຯ��
//
// ȫ��ͷ�ļ�
#include <GrapX.H>

// ��׼�ӿ�
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GXImage.H"
#include "Include/GXCanvas.H"
#include "Include/GXSprite.H"
#include "Include/GXGraphics.H"
#include "Include/GXKernel.H"

// ˽��ͷ�ļ�
#include <clstd/clPathFile.h>
#include <clstd/clUtility.H>
#include <clstd/clStringSet.H>
#include <clstd/Smart/SmartStream.h>
#include <clstd/Smart/SmartProfile.h>
#include <clstd/Smart/SmartStock.h>
#include <Canvas/GXSpriteImpl.H>

//using namespace std;
//////////////////////////////////////////////////////////////////////////
extern "C" GXBOOL GXDLLAPI gxSetRegn(GXLPREGN lprg, GXINT xLeft, GXINT yTop, GXINT xWidth, GXINT yHeight);
GXHRESULT IntLoadSpriteDesc(SmartProfileA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc);
//////////////////////////////////////////////////////////////////////////

GXHRESULT GXDLLAPI GXCreateSprite(GXGraphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount, GXSprite** ppSprite)
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

extern "C" GXHRESULT GXDLLAPI GXCreateSpriteEx(GXGraphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite)
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

GXHRESULT GXDLLAPI GXCreateSpriteArray(GXGraphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite)
{
  GXImage* pImage = pGraphics->CreateImageFromFile(szTextureFile);
  if( ! pImage) {
    return GX_E_OPEN_FAILED;
  }
  int nWidth = pImage->GetWidth();
  int nHeight = pImage->GetHeight();

  clvector<GXREGN> aRegn;
  GXREGN regn;

  for(int y = yStart; y <= nHeight - nTileHeight; y += nTileHeight + yGap)
  {
    for(int x = xStart; x <= nWidth - nTileWidth; x += nTileWidth + xGap)
    {
      gxSetRegn(&regn, x, y, nTileWidth, nTileHeight);
      aRegn.push_back(regn);
    }
  }
  GXHRESULT hval = GXCreateSprite(pGraphics, szTextureFile, &aRegn.front(), aRegn.size(), ppSprite);
  SAFE_RELEASE(pImage);
  return hval;
}

GXHRESULT GXDLLAPI GXCreateSpriteFromFileW(GXGraphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite)
{
  SmartProfileA ss;
  GXHRESULT hval = GX_FAIL;

  if( ! ss.LoadW(szSpriteFile)) {
    return GX_E_OPEN_FAILED;
  }
  
  //
  // sprite ֧��[Sprite]��[SpriteArray]���ָ�ʽ������
  // ĿǰΪֹ��������и�ʽ������ͬһ�ļ������ȶ�ȡ[SpriteArray]������
  // δ�����ų�����ͬʱ��ȡ���ָ�ʽ���ϲ�Ϊͬһ��Sprite����Ŀ���
  //

  SmartProfileA::HANDLE hSpriteArray = ss.FindFirstSection(NULL, NULL, NULL, "SpriteArray");
  if(hSpriteArray)
  {
    clStringW strImageFile;
    strImageFile    = ss.FindKeyAsString(hSpriteArray, "File", "");
    int xStart      = ss.FindKeyAsInteger(hSpriteArray, "Left", 0);
    int yStart      = ss.FindKeyAsInteger(hSpriteArray, "Top", 0);
    int nTileWidth  = ss.FindKeyAsInteger(hSpriteArray, "TileWidth", 0);
    int nTileHeight = ss.FindKeyAsInteger(hSpriteArray, "TileHeight", 0);
    int nGapWidth   = ss.FindKeyAsInteger(hSpriteArray, "GapWidth", 0);
    int nGapHeight  = ss.FindKeyAsInteger(hSpriteArray, "GapHeight", 0);

    clStringW strImageFileW = szSpriteFile;//;
    strImageFileW.Replace(clpathfile::ViceSlash(), clpathfile::Slash());
    clpathfile::RemoveFileSpecW(strImageFileW);
    clpathfile::CombinePathW(strImageFileW, strImageFileW, clStringW(strImageFile));
    hval = GXCreateSpriteArray(pGraphics, strImageFileW, xStart, yStart, nTileWidth, nTileHeight, nGapWidth, nGapHeight, ppSprite);
    ss.FindClose(hSpriteArray);
  } // if(hSpriteArray)
  else
  {
    GXSpriteDesc* pSpriteDesc = NULL;
    hval = IntLoadSpriteDesc(ss, szSpriteFile, &pSpriteDesc);
    if(GXSUCCEEDED(hval)) {
      hval = GXCreateSpriteEx(pGraphics, &pSpriteDesc->ToDesc(), ppSprite);
    }
    SAFE_DELETE(pSpriteDesc);
  }
  return hval;
}

//class GXSpriteDescObjImpl : public GXSpriteDescObj
//{
//  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
//  friend GXHRESULT IntLoadSpriteDesc(SmartProfileA& ss, GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
//  friend GXHRESULT IntLoadModules( SmartProfileA &ss, GXSpriteDescObjImpl* pDescObj );
//  friend GXHRESULT IntLoadFrames( SmartProfileA &ss, GXSpriteDescObjImpl* pDescObj );
//  friend GXHRESULT IntLoadAnimations( SmartProfileA &ss, GXSpriteDescObjImpl* pDescObj ) ;
//};

GXHRESULT IntLoadModules( SmartProfileA &ss, GXSpriteDescImpl* pDescObj )
{
  GXHRESULT hval = GX_OK;
  // ��������
  //*ppSprite = NULL;
  //GXSpriteImpl* pSprite = new GXSpriteImpl;
  //if( ! InlCheckNewAndIncReference(pSprite)) {
  //  return GX_FAIL;
  //}

  //pSprite->m_pImage = pGraphics->CreateImageFromFile(pDescObj->m_strImageFile);
  //pSprite->m_strImageFile = strImageFile;

  //if( ! pSprite->m_pImage) {
  //  // TODO: ��ΪRemoveFileSpec
  //  clStringW strRelImagePath = szSpriteFile;
  //  strRelImagePath.Replace(clpathfile::ViceSlash(), clpathfile::Slash());
  //  clsize nFilename = clpathfile::FindFileNameW(strRelImagePath);
  //  strRelImagePath.Replace(nFilename, -1, strImageFile);
  //  pSprite->m_pImage = pGraphics->CreateImageFromFile(strRelImagePath);
  //}

  //clvector<REGN> aRegns;
  SmartProfileA::HANDLE hModule = ss.FindFirstSection(NULL, FALSE, "Sprite\\Module", "rect");


  // �ο�TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if(hModule == NULL) {
    hModule = ss.FindFirstSection(NULL, FALSE, "Image\\Module", "rect");
  }


  if(hModule != NULL)
  {

    GXSprite::MODULE sModule;
    clStringA strModuleName;
    SmartProfileA::VALUE valueLeft;
    SmartProfileA::VALUE valueTop;
    SmartProfileA::VALUE valueRight;
    SmartProfileA::VALUE valueBottom;

    do 
    {
      //sSprite.name.Clear();
      strModuleName.Clear();
      sModule.name = NULL;
      sModule.id   = 0;

      if( ss.FindKey(hModule, "left", valueLeft) == TRUE &&
        ss.FindKey(hModule, "top", valueTop) == TRUE &&
        ss.FindKey(hModule, "right", valueRight) == TRUE &&
        ss.FindKey(hModule, "bottom", valueBottom) == TRUE)
      {
        sModule.regn.left   = valueLeft.ToInt();
        sModule.regn.top    = valueTop.ToInt();
        sModule.regn.width  = valueRight.ToInt() - sModule.regn.left;
        sModule.regn.height = valueBottom.ToInt() - sModule.regn.top;

        strModuleName = ss.FindKeyAsString(hModule, "name", "");

        // ������ֲ�Ϊ������뵽�����в������ṹ��
        if(strModuleName.IsNotEmpty()) {
          sModule.name = pDescObj->m_NameSet.add(strModuleName);
        }

        pDescObj->m_aModules.push_back(sModule);
      }
    } while(ss.FindNextSection(hModule) == TRUE);
    //*ppSprite = pSprite;
    hval = GX_OK;
  }
  ss.FindClose(hModule);
  return hval;
}

GXHRESULT IntLoadFrames( SmartProfileA &ss, GXSpriteDescImpl* pDescObj ) 
{
  GXHRESULT hval = GX_OK;
  SmartProfileA::HANDLE hFrame = ss.FindFirstSection(NULL, FALSE, "Sprite", "Frame");

  // �ο�TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if(hFrame == NULL) {
    hFrame = ss.FindFirstSection(NULL, FALSE, "Image", "Frame");
  }

  if(hFrame != NULL)
  {
    SmartProfileA::VALUE valueName;
    //SmartProfileA::VALUE valueId;

    //typedef clvector<GXSPRITEFRAME> FrameArray;
    //typedef clvector<GXSPRITEFRAME_MODULEDESC> FrameModuleArray;

    auto& aFrames = pDescObj->m_aFrames;
    auto& aFrameModulesArray = pDescObj->m_aFrameModules;
    GXSprite::FRAME sFrame = {0};

    do {
      sFrame.name = NULL;
      sFrame.start = (GXINT)aFrameModulesArray.size();

      if(ss.FindKey(hFrame, "name", valueName)) {
        sFrame.name = pDescObj->m_NameSet.add(valueName.ToString());
      }

      sFrame.id = ss.FindKeyAsInteger(hFrame, "id", 0);

      SmartProfileA::HANDLE hFrameModule = ss.FindFirstSection(hFrame, FALSE, NULL, "module");
      if(hFrameModule)
      {
        GXSprite::FRAME_MODULE sFrameModule = {0};
        do {
          //SmartProfileA::VALUE valueIndex;
          //SmartProfileA::VALUE valueFlags;
          //SmartProfileA::VALUE valueLeft;
          //SmartProfileA::VALUE valueTop;

          sFrameModule.nModuleIdx = ss.FindKeyAsInteger(hFrameModule, "index", -1);
          sFrameModule.rotate     = ss.FindKeyAsInteger(hFrameModule, "flags", -1);
          sFrameModule.offset.x   = ss.FindKeyAsInteger(hFrameModule, "x", 0);
          sFrameModule.offset.y   = ss.FindKeyAsInteger(hFrameModule, "y", 0);

          TRACE("index:%d flags:%d x:%d y:%d\n", sFrameModule.nModuleIdx, sFrameModule.rotate, sFrameModule.offset.x, sFrameModule.offset.y);
          // push back
          aFrameModulesArray.push_back(sFrameModule);

        } while(ss.FindNextSection(hFrameModule));
        ss.FindClose(hFrameModule);
      }

      // push back
      sFrame.count = (GXINT)aFrameModulesArray.size() - sFrame.start;
      aFrames.push_back(sFrame);
      sFrame.start = (GXINT)aFrameModulesArray.size();

    }while(ss.FindNextSection(hFrame));

    ss.FindClose(hFrame);
  }
  return hval;
}

GXHRESULT IntLoadAnimations( SmartProfileA &ss, GXSpriteDescImpl* pDescObj ) 
{
  GXHRESULT hval = GX_OK;
  SmartProfileA::HANDLE hAnim = ss.FindFirstSection(NULL, FALSE, "Sprite", "Animation");

  // �ο�TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if(hAnim == NULL) {
    hAnim = ss.FindFirstSection(NULL, FALSE, "Image", "Animation");
  }

  if(hAnim != NULL)
  {
    SmartProfileA::VALUE valueName;
    GXSprite::ANIMATION sAnimation = {0};

    auto& aAnimations = pDescObj->m_aAnimations;
    auto& aAnimFrames = pDescObj->m_aAnimFrames;

    do {
      sAnimation.name = NULL;
      if(ss.FindKey(hAnim, "name", valueName)) {
        sAnimation.name = pDescObj->m_NameSet.add(valueName.ToString());
      }

      sAnimation.rate  = ss.FindKeyAsInteger(hAnim, "rate", 0);
      sAnimation.id    = ss.FindKeyAsInteger(hAnim, "id", 0);
      sAnimation.start = (GXINT)aAnimFrames.size();
      sAnimation.count = ss.FindKeyAsInteger(hAnim, "count", 0);
      aAnimFrames.reserve(aAnimFrames.size() + sAnimation.count);

      clStringA strFrames = ss.FindKeyAsString(hAnim, "frames", "");
      clStringArrayA aFrameList;
      if(strFrames.IsNotEmpty())
      {
        clstd::ResolveString(strFrames, ',', aFrameList);

        // �����βΪ����ȥ��
        while(aFrameList.back().IsEmpty()) {
          aFrameList.erase(aFrameList.end() - 1);
        }
      }

      // ����ļ���¼��ʵ�ʶ���������Ƿ����
      if(aFrameList.size() != sAnimation.count) {
        CLOG_WARNING(__FUNCTION__": Number of animation frames is not match the record.(%s,%d)\r\n", 
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

    }while(ss.FindNextSection(hAnim));

    ss.FindClose(hAnim);
  }
  return hval;
}

GXHRESULT IntLoadSpriteDesc(SmartProfileA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  GXSpriteDescImpl* pDescObj = NULL;
  GXHRESULT hval = GX_OK;

  pDescObj = new GXSpriteDescImpl;
  if(pDescObj == NULL) {
    CLOG_ERROR(__FUNCTION__": Out of memory!\r\n");
    return GX_ERROR_OUROFMEMORY;
  }


  SmartProfileA::HANDLE hSprite = ss.FindFirstSection(NULL, NULL, NULL, "Sprite");

  // TODO: ֮ǰSprite����������Image��, ������ΪSprite
  // �������˼���д�������Sprite�β��������Զ�ȡ�͵�Image�ζ���
  // �Ժ�ȥ��
  // TAG:{B1363AEA-3BB3-4E2E-9C90-55A3CAF07E78}
  if(hSprite == NULL) {
    hSprite = ss.FindFirstSection(NULL, NULL, NULL, "Image");
  }


  if(hSprite)
  {
    //
    // ��ȡ�����ļ���
    //
    SmartProfileA::VALUE valueFile;
    if(hSprite != NULL)
    {
      if(ss.FindKey(hSprite, "File", valueFile) == TRUE) {
        clStringW strSpriteDir = szSpriteFile;
        clpathfile::RemoveFileSpecW(strSpriteDir);
        pDescObj->m_strImageFile = AS2WS(FromProfileString<clStringA>(valueFile.ToString()));
        clpathfile::CombinePathW(pDescObj->m_strImageFile, strSpriteDir, pDescObj->m_strImageFile);
      }
    }
    ss.FindClose(hSprite);

    // ��ȡModule����
    IntLoadModules(ss, pDescObj);

    // ��ȡ Frame
    IntLoadFrames(ss, pDescObj);

    // ��ȡ Animation
    IntLoadAnimations(ss, pDescObj);

  } // if(hSprite != NULL)



  *ppDesc = pDescObj;

  return hval;
}

GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  SmartProfileA ss;

  if( ! ss.LoadW(szSpriteFile)) {
    return GX_E_OPEN_FAILED;
  }
  
  return IntLoadSpriteDesc(ss, szSpriteFile, ppDesc);
}

GXHRESULT GXDLLAPI GXCreateSpriteFromFileA(GXGraphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite)
{
  return GXCreateSpriteFromFileW(pGraphics, AS2WS(szSpriteFile), ppSprite);
}

GXHRESULT GXDLLAPI GXLoadSpriteDescA(GXLPCSTR szSpriteFile, GXSpriteDesc** ppDesc)
{
  return GXLoadSpriteDescW(AS2WS(szSpriteFile), ppDesc);
}

GXSpriteDescImpl::~GXSpriteDescImpl()
{
  // ���������֤�����������ض�ģ�������
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
    file.WritefA("%*sid=\"%d\";\r\n", nTab, "", d.id);
  }
  if(d.name != NULL && d.name[0] != '\0') {
    file.WritefA("%*sname=\"%s\";\r\n", nTab, "", d.name);
  }
}

GXBOOL GXDLLAPI ___GXSaveSpriteToFileW___(GXLPCWSTR szFilename, const GXSPRITE_DESCW* pDesc)
{
  clFile file;

  if(pDesc->szImageFile == NULL || GXSTRLEN(pDesc->szImageFile) == 0) {
    CLOG_ERROR(__FUNCTION__": Texture filename is empty.\r\n");
    return FALSE;
  }

  if( ! IntCheckSpriteDesc(pDesc)) {
    CLOG_ERROR(__FUNCTION__": Bad input argument.\r\n");
  }

  if( ! file.CreateAlwaysW(szFilename)) {
    CLOG_ERROR(__FUNCTION__": Can't create file.\r\n");
    return FALSE;
  }

  file.WritefA("smart{version = \"0.0.1.0\";}\r\n");
  file.WritefA("Sprite {\r\n"); // <Sprite>

  clStringW strRelativePath;
  clpathfile::RelativePathToW(strRelativePath, szFilename, FALSE, pDesc->szImageFile, FALSE);
  clStringA strFilenameA(strRelativePath);
  file.WritefA("  File=\"%s\";\r\n", strFilenameA);
  file.WritefA("  Module {\r\n"); // <Module>

  //
  // Module
  //
  if(pDesc->aModules != NULL) // ����Ҫ�ж�pDesc->nNumOfModules������ѭ���ж���
  {
    for(GXUINT i = 0; i < pDesc->nNumOfModules; i++)
    {
      GXSprite::MODULE& m = pDesc->aModules[i];
      file.WritefA("    rect {\r\n"); // <rect>
      IntSaveSpriteNamdAndId(file, 6, m);
      file.WritefA("      left=%d; top=%d;\r\n", m.regn.left, m.regn.top);
      file.WritefA("      right=%d; bottom=%d;\r\n", m.regn.left + m.regn.width, m.regn.top + m.regn.height);
      file.WritefA("    };\r\n"); // </rect>
    }
  }
  file.WritefA("  };\r\n"); // </Module>

  //
  // Frame�б���Ϣ
  // 
  if(pDesc->aFrames != NULL && pDesc->nNumOfFrameModules > 0 && pDesc->aFrameModules != NULL)
  {
    for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
    {
      GXSprite::FRAME& f = pDesc->aFrames[i];
      file.WritefA("  Frame {\r\n"); // <frame>
      IntSaveSpriteNamdAndId(file, 4, f);
      //file.WritefA("    start=\"%d\"; count=\"%d\";\r\n", f.start, f.count);

      // Frame������Ϣ
      for(GXINT n = 0; n < f.count; n++)
      {
        GXSprite::FRAME_MODULE& fd = pDesc->aFrameModules[n + f.start];
        file.WritefA("    module {\r\n");
        file.WritefA("      index=\"%d\"; flags=\"%d\"; x=\"%d\"; y=\"%d\";\r\n", 
          fd.nModuleIdx, fd.rotate, fd.offset.x, fd.offset.y);
        file.WritefA("    };\r\n");
      }
      file.WritefA("  };\r\n"); // </frame>
    }
  }

  //
  // Animation�б���Ϣ
  //
  if(pDesc->aAnimations != NULL && pDesc->nNumOfAnimFrames > 0 && pDesc->aAnimFrames != NULL)
  {
    clStringA strFrameList;
    for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
    {
      GXSprite::ANIMATION& a = pDesc->aAnimations[i];
      file.WritefA("    Animation {\r\n"); // <animation>
      IntSaveSpriteNamdAndId(file, 6, a);
      file.WritefA("      rate=\"%d\"; start=\"%d\"; count=\"%d\";\r\n", a.rate, a.start, a.count);

      strFrameList.Clear();
      for(GXINT n = 0; n < a.count; n++) {
        strFrameList.AppendFormat("%d,", pDesc->aAnimFrames[n + a.start]);
      }
      strFrameList.TrimRight(',');
      file.WritefA("      frames=\"%s\"\r\n", strFrameList);
      file.WritefA("    };\r\n"); // </animation>
    }
  }

  file.WritefA("};\r\n"); // </Sprite>
  return TRUE;

}

GXBOOL GXDLLAPI GXSaveSpriteToFileW(GXLPCWSTR szFilename, const GXSPRITE_DESCW* pDesc)
{
  //clFile file;
  SmartStockA stock;

  if(pDesc->szImageFile == NULL || GXSTRLEN(pDesc->szImageFile) == 0) {
    CLOG_ERROR(__FUNCTION__": Texture filename is empty.\r\n");
    return FALSE;
  }

  if( ! IntCheckSpriteDesc(pDesc)) {
    CLOG_ERROR(__FUNCTION__": Bad input argument.\r\n");
  }

  auto pSmartSection = stock.Create("smart");
  pSmartSection->SetKey("version", "0.0.1.0");
  stock.CloseSection(pSmartSection);

  auto pSpriteSection = stock.Create("Sprite");

  clStringW strRelativePath;
  clpathfile::RelativePathToW(strRelativePath, szFilename, FALSE, pDesc->szImageFile, FALSE);
  clStringA strFilenameA(strRelativePath);

  pSpriteSection->SetKey("File", strFilenameA);
  stock.CloseSection(pSpriteSection);

  //
  // Module
  //
  if(pDesc->aModules != NULL) // ����Ҫ�ж�pDesc->nNumOfModules������ѭ���ж���
  {
    for(GXUINT i = 0; i < pDesc->nNumOfModules; i++)
    {
      GXSprite::MODULE& m = pDesc->aModules[i];
      auto pRectSection = stock.Create("Sprite/Module/rect");
      pRectSection->SetKey("name",    m.name);
      pRectSection->SetKey("id",      clStringA(m.id));
      pRectSection->SetKey("left",    clStringA(m.regn.left));
      pRectSection->SetKey("top",     clStringA(m.regn.top));
      pRectSection->SetKey("right",   clStringA(m.regn.left + m.regn.width));
      pRectSection->SetKey("bottom",  clStringA(m.regn.top + m.regn.height));
      stock.CloseSection(pRectSection);
    }
  }

  //
  // Frame�б���Ϣ
  // 
  if(pDesc->aFrames != NULL && pDesc->nNumOfFrameModules > 0 && pDesc->aFrameModules != NULL)
  {
    for(GXUINT i = 0; i < pDesc->nNumOfFrames; i++)
    {
      GXSprite::FRAME& f = pDesc->aFrames[i];
      auto pFrameSection = stock.Create("Sprite/Frame");
      pFrameSection->SetKey("name",   f.name);
      pFrameSection->SetKey("id",     clStringA(f.id));
      pFrameSection->SetKey("start",  clStringA(f.start));
      pFrameSection->SetKey("count",  clStringA(f.count));

      // Frame������Ϣ
      for(GXINT n = 0; n < f.count; n++)
      {
        GXSprite::FRAME_MODULE& fd = pDesc->aFrameModules[n + f.start];
        auto pFrameModuleSection = stock.CreateChild(pFrameSection, "module");
        pFrameModuleSection->SetKey("index",  clStringA(fd.nModuleIdx));
        pFrameModuleSection->SetKey("flags",  clStringA(fd.rotate));
        pFrameModuleSection->SetKey("x",      clStringA(fd.offset.x));
        pFrameModuleSection->SetKey("y",      clStringA(fd.offset.y));
        stock.CloseSection(pFrameModuleSection);
      }
      stock.CloseSection(pFrameSection);
      //file.WritefA("  };\r\n"); // </frame>
    }
  }

  //
  // Animation�б���Ϣ
  //
  if(pDesc->aAnimations != NULL && pDesc->nNumOfAnimFrames > 0 && pDesc->aAnimFrames != NULL)
  {
    clStringA strFrameList;
    for(GXUINT i = 0; i < pDesc->nNumOfAnimations; i++)
    {
      GXSprite::ANIMATION& a = pDesc->aAnimations[i];
      auto pAnimSection = stock.Create("Sprite/Animation");
      pAnimSection->SetKey("name",   a.name);
      pAnimSection->SetKey("id",     clStringA(a.id));
      pAnimSection->SetKey("rate",   clStringA(a.rate));
      pAnimSection->SetKey("start",  clStringA(a.start));
      pAnimSection->SetKey("count",  clStringA(a.count));

      strFrameList.Clear();
      for(GXINT n = 0; n < a.count; n++) {
        strFrameList.AppendFormat("%d,", pDesc->aAnimFrames[n + a.start]);
      }
      pAnimSection->SetKey("frames", strFrameList);
      stock.CloseSection(pAnimSection);
    }
  }

  if( ! stock.SaveW(szFilename))
  {
    CLOG_ERROR(__FUNCTION__": Can not save file(%s).\r\n", szFilename);
    return FALSE;
  }

  return TRUE;
}
//////////////////////////////////////////////////////////////////////////