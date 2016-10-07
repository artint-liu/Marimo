//
// Sprite Manufacture
// Sprite 生产类函数
//
// 全局头文件
#include <GrapX.H>

// 标准接口
#include "GrapX/GResource.H"
#include "GrapX/GXImage.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/MOSprite.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXKernel.H"

// 私有头文件
#include <clPathFile.h>
#include <clUtility.H>
#include <clStringSet.H>
#include <Smart/SmartStream.h>
#include <clStock.h>
#include "MOSpriteImpl.h"

namespace Marimo
{


  b32 SPRITE_DESC_LOADER::LoadRegion(GXREGN& regn, const clstd::StockA::ATTRIBUTE& attr, GXDWORD& dwFlag)
  {
    if (attr.key == "x" || attr.key == "left") {
      regn.left = attr.ToInt();
    }
    else if (attr.key == "y" || attr.key == "top") {
      regn.top = attr.ToInt();
    }
    else if (attr.key == "right") {
      SET_FLAG(dwFlag, HAS_LOADED_RIGHT);
      regn.width = attr.ToInt();
    }
    else if (attr.key == "bottom") {
      SET_FLAG(dwFlag, HAS_LOADED_BOTTOM);
      regn.height = attr.ToInt();
    }
    else if (attr.key == "width") {
      RESET_FLAG(dwFlag, HAS_LOADED_RIGHT);
      regn.width = attr.ToInt();
    }
    else if (attr.key == "height") {
      RESET_FLAG(dwFlag, HAS_LOADED_BOTTOM);
      regn.height = attr.ToInt();
    }
    else {
      return FALSE;
    }
    return TRUE;
  }



  b32 SPRITE_DESC_LOADER::LoadModule(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "module")
    {
      Sprite::MODULE module = {0};
      GXDWORD dwFlag = 0;
      for (sect.FirstKey(attr); attr; ++attr)
      {
        if(LoadCommon(module, attr)) {
          continue;
        }
        else if(LoadRegion(module.regn, attr, dwFlag)) {
          continue;
        }
      }

      // 记录顺序无关
      if(TEST_FLAG(dwFlag, HAS_LOADED_RIGHT)) {
        module.regn.width = module.regn.width - module.regn.left;
      }

      if(TEST_FLAG(dwFlag, HAS_LOADED_BOTTOM)) {
        module.regn.height = module.regn.height - module.regn.top;
      }

      TYPEIDX ti = {Sprite::Type_Module};
      ti.index = (GXUINT)aModules.size();
      AddToDict(module, ti);

      aModules.push_back(module);
      return TRUE;
    }
    return FALSE;
  }

  b32 SPRITE_DESC_LOADER::LoadFrame(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "frame")
    {
      Sprite::FRAME frame = {0};
      for (sect.FirstKey(attr); attr; ++attr)
      {
        if(LoadCommon(frame, attr)) {
          continue;
        }
      }

      frame.begin = (GXUINT)aFrameUnits.size();
      for(auto unit_sect = sect.Open(NULL); unit_sect; ++unit_sect)
      {
        Sprite::FRAME_UNIT unit = {0};
        gxSetRegn(&unit.regn, -1, -1, -1, -1);

        if(unit_sect.SectionName() == "module")
        {
          clstd::StockA::ATTRIBUTE unit_attr;
          GXDWORD dwFlag = 0;
          for(unit_sect.FirstKey(unit_attr); unit_attr; ++unit_attr)
          {
            if(LoadRegion(unit.regn, unit_attr, dwFlag)) {
              continue;
            }
            else if(unit_attr.key == "id") {
              unit.nModuleIdx = unit_attr.ToInt(); // ID暂存到index中
            }
          }
          aFrameUnits.push_back(unit);
        }
      }
      frame.end = (GXUINT)aFrameUnits.size();
      
      TYPEIDX ti = {Sprite::Type_Frame};
      ti.index = (GXUINT)aFrames.size();
      AddToDict(frame, ti);

      aFrames.push_back(frame);
    }
    return FALSE;
  }

  b32 SPRITE_DESC_LOADER::LoadAnimation(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "animation")
    {
      Sprite::ANIMATION anim = {0};
      for (sect.FirstKey(attr); attr; ++attr)
      {
        if(LoadCommon(anim, attr)) {
          continue;
        }
        else if(attr.key == "frames")
        {
          clStringListA str_list;
          clstd::ResolveString(attr.value.ToString(), ',', str_list);
          
          anim.begin = (GXUINT)aAnimUnits.size();
          for(auto it = str_list.begin(); it != str_list.end(); ++it) {
            aAnimUnits.push_back(it->ToInteger());
          }
          anim.end = (GXUINT)aAnimUnits.size();
        }
      }

      TYPEIDX ti = {Sprite::Type_Animation};
      ti.index = (GXUINT)aAnims.size();
      AddToDict(anim, ti);

      aAnims.push_back(anim);
    }
    return FALSE;
  }

  void SPRITE_DESC_LOADER::TranslateFrameUnit()
  {
    std::for_each(aFrameUnits.begin(), aFrameUnits.end(), [&](Sprite::FRAME_UNIT& unit) {
      auto it = sIDDict.find(unit.nModuleIdx);
      if(it == sIDDict.end() || it->second.type != Sprite::Type_Module) {
        unit.nModuleIdx = -1;
      }
      else {
        unit.nModuleIdx = it->second.index;
        if(unit.regn.width <= 0) {
          unit.regn.width = aModules[unit.nModuleIdx].regn.width;
        }
        if(unit.regn.height <= 0) {
          unit.regn.height = aModules[unit.nModuleIdx].regn.height;
        }
      }
    });
  }

  void SPRITE_DESC_LOADER::TranslateAnimUnit()
  {
    std::for_each(aAnimUnits.begin(), aAnimUnits.end(), [&](Sprite::ANIM_UNIT& unit){
      auto it = sIDDict.find(unit);
      if(it == sIDDict.end() || it->second.type != Sprite::Type_Frame) {
        unit = -1;
      }
      else {
        unit = it->second.index;
      }
    });
  }

  //////////////////////////////////////////////////////////////////////////

  b32 SPRITE_DESC_LOADER::Load(clstd::StockA* pStock, GXLPCSTR szSection)
  {
    clstd::StockA::Section main_sect = pStock->Open(szSection);
    if (!main_sect) {
      CLOG_ERROR("Can not open \"%s\" section.\n", szSection);
      return FALSE;
    }

    clstd::StockA::ATTRIBUTE attr;

    for (main_sect.FirstKey(attr); attr; ++attr) {
      if (attr.key == "image") {
        aFiles.push_back(attr.value.ToString());
      }
    }

    for (auto sect = main_sect.Open(NULL); sect; ++sect)
    {
      LoadModule(sect) ||
        LoadFrame(sect) ||
        LoadAnimation(sect);
    }

    TranslateFrameUnit();
    TranslateAnimUnit();
    return TRUE;
  }

  SPRITE_DESC* SPRITE_DESC::Create(clstd::StockA* pStock, GXLPCSTR szSection)
  {
    SPRITE_DESC_LOADER* pLoader = new SPRITE_DESC_LOADER;
    if( ! pLoader->Load(pStock, szSection)) {
      delete pLoader;
      pLoader = NULL;
    }
    return pLoader;
  }

  void SPRITE_DESC::Destroy(SPRITE_DESC* pDesc)
  {
    SPRITE_DESC_LOADER* pLoader = static_cast<SPRITE_DESC_LOADER*>(pDesc);
    SAFE_DELETE(pLoader);
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL GXDLLAPI Sprite::CreateFromStockA(Sprite** ppSprite, GXGraphics* pGraphics, clstd::StockA* pStock, GXLPCSTR szImageDir, GXLPCSTR szSection)
  {
    MOSpriteImpl* pSprite = new MOSpriteImpl;
    if( ! InlCheckNewAndIncReference(pSprite)) {
      return FALSE;
    }

    SPRITE_DESC_LOADER spr;
    spr.strImageDir = szImageDir;

    if (spr.Load(pStock, szSection)) {
      if (pSprite->Initialize(pGraphics, &spr)) {
        *ppSprite = pSprite;
        return TRUE;
      }
    }

    SAFE_RELEASE(pSprite);
    return FALSE;
  }

  GXBOOL GXDLLAPI Sprite::CreateFromStockFileA(Sprite** ppSprite, GXGraphics* pGraphics, GXLPCSTR szFilename, GXLPCSTR szSection)
  {
    clstd::StockA stock;
    if( ! stock.LoadA(szFilename)) {
      CLOG_ERROR("%s : Can not open stock file(\"%s\").\n", __FUNCTION__, szFilename);
      return FALSE;
    }
    clsize pos = clpathfile::FindFileNameA(szFilename);
    clStringA strImageDir;
    if(pos != 0) {
      strImageDir.Append(szFilename, pos);
    }
    return CreateFromStockA(ppSprite, pGraphics, &stock, strImageDir, szSection);
  }
} // namespace Marimo