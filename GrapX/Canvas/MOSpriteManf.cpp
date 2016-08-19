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
#include <Smart/Stock.h>
#include <Canvas/MOSpriteImpl.H>
#include "clStringSet.h"

namespace Marimo
{

  struct SPRITE
  {
    enum {
      HAS_LOADED_RIGHT = 0x0001,
      HAS_LOADED_BOTTOM = 0x0002,
    };
    clStringArrayA aFiles;
    clstd::StringSetA StringSet;
    Sprite::ModuleArray aModules;
    Sprite::FrameArray aFrames;
    Sprite::AnimationArray aAnims;
    Sprite::FrameUnitArray aFrameUnits;
    Sprite::AnimUnitArray aAnimUnits;

    b32 LoadModule(const clstd::StockA::Section& sect);
    b32 LoadFrame(const clstd::StockA::Section& sect);
    b32 LoadAnimation(const clstd::StockA::Section& sect);

    template <class _TUnit>
    b32 LoadCommon(_TUnit& dest, const clstd::StockA::ATTRIBUTE& attr)
    {
      if(attr.key == "id") {
        dest.id = attr.ToInt();
      }
      else if(attr.key == "name") {
        dest.name = StringSet.add(attr.ToString());
      }
      else {
        return FALSE;
      }
      return TRUE;
    }

    b32 LoadRegion(GXREGN& regn, const clstd::StockA::ATTRIBUTE& attr, GXDWORD& dwFlag)
    {
      if(attr.key == "x" || attr.key == "left") {
        regn.left = attr.ToInt();
      }
      else if(attr.key == "y" || attr.key == "top") {
        regn.top = attr.ToInt();
      }
      else if(attr.key == "right") {
        SET_FLAG(dwFlag, HAS_LOADED_RIGHT);
        regn.width = attr.ToInt();
      }
      else if(attr.key == "bottom") {
        SET_FLAG(dwFlag, HAS_LOADED_BOTTOM);
        regn.height = attr.ToInt();
      }
      else if(attr.key == "width") {
        RESET_FLAG(dwFlag, HAS_LOADED_RIGHT);
        regn.width = attr.ToInt();
      }
      else if(attr.key == "height") {
        RESET_FLAG(dwFlag, HAS_LOADED_BOTTOM);
        regn.height = attr.ToInt();
      }
      else {
        return FALSE;
      }
      return TRUE;
    }
  };

  b32 SPRITE::LoadModule(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "module")
    {
      Sprite::MODULE module;
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

      aModules.push_back(module);
      return TRUE;
    }
    return FALSE;
  }

  b32 SPRITE::LoadFrame(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "frame")
    {
      Sprite::FRAME frame;
      for (sect.FirstKey(attr); attr; ++attr)
      {
        if(LoadCommon(frame, attr)) {
          continue;
        }
      }

      for(auto unit_sect = sect.Open(NULL); unit_sect; ++unit_sect)
      {
        Sprite::FRAME_UNIT unit;
        if(unit_sect.SectionName() == "module")
        {
          clstd::StockA::ATTRIBUTE unit_attr;
          GXDWORD dwFlag = 0;
          for(unit_sect.FirstKey(unit_attr); unit_attr; ++unit_attr)
          {
            if(LoadRegion(unit.regn, unit_attr, dwFlag)) {
              continue;
            }
          }
          aFrameUnits.push_back(unit);
        }
      }
      
      aFrames.push_back(frame);
    }
    return FALSE;
  }

  b32 SPRITE::LoadAnimation(const clstd::StockA::Section& sect)
  {
    clstd::StockA::ATTRIBUTE attr;

    if(sect.SectionName() == "animation")
    {
      Sprite::ANIMATION anim;
      for (sect.FirstKey(attr); attr; ++attr)
      {
        if(LoadCommon(anim, attr)) {
          continue;
        }
        else if(attr.key == "frames")
        {
          clStringListA str_list;
          clstd::ResolveString(attr.value.ToString(), ',', str_list);
          for(auto it = str_list.begin(); it != str_list.end(); ++it)
          {
            aAnimUnits.push_back(it->ToInteger());
          }
        }
      }
      aAnims.push_back(anim);
    }
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL GXDLLAPI Sprite::CreateFromStockA(Sprite** ppSprite, clstd::StockA* pStock, GXLPCSTR szSection)
  {
    MOSpriteImpl* pSprite = new MOSpriteImpl;
    if( ! InlCheckNewAndIncReference(pSprite)) {
      return FALSE;
    }

    clstd::StockA::Section main_sect = pStock->Open(szSection);
    if( ! main_sect) {
      CLOG_ERROR("Can not open \"%s\" section.\n", szSection);
      return FALSE;
    }

    SPRITE spr_desc;
    clstd::StockA::ATTRIBUTE attr;

    for(main_sect.FirstKey(attr); attr; ++attr) {
      if(attr.key == "image") {
        spr_desc.aFiles.push_back(attr.value.ToString());
      }
    }

    for(auto sect = main_sect.Open(NULL); sect; ++sect)
    {
      spr_desc.LoadModule(sect) ||
      spr_desc.LoadFrame(sect) ||
      spr_desc.LoadAnimation(sect);
    }

    return FALSE;
  }
} // namespace Marimo