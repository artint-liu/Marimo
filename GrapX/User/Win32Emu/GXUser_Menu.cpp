#include "GrapX.H"
#include "User/GrapX.Hxx"
#include "Smart/smartstream.h"
#include <clTokens.h>
#include "clStock.h"

//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
//#include "Include/GXFont.H"
//#include "Include/GXGraphics.H"
//#include "Include/GXUser.H"
//#include "Include/GXCanvas.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "Controls/GXUICtrlBase.h"

typedef clhash_map<clStringA, GXDWORD> TIdentifyDefination;

#pragma pack(1)
struct GXMENUEX_TEMPLATE_ITEM {
  GXMENUEX_TEMPLATE_ITEM_HEADER head;
  GXWCHAR szText[1024];

  GXBOOL IsValid()
  {
    return head.menuId != 0 || GXSTRLEN(szText);
  }
  GXSIZE_T GetSize()
  {
    GXSIZE_T nSize = sizeof(GXMENUEX_TEMPLATE_ITEM_HEADER) + (GXSTRLEN(szText) + 1) * sizeof(GXWCHAR);
    return ALIGN_4(nSize);
  }
  void Clear()
  {
    memset(this, 0, sizeof(GXMENUEX_TEMPLATE_ITEM_HEADER) + sizeof(GXWCHAR));
  }
};

STATIC_ASSERT(sizeof(GXMENUEX_TEMPLATE_ITEM) == 3 * sizeof(GXDWORD) + sizeof(GXWORD) + 1024 * sizeof(GXWCHAR));

GXBOOL IntEnumMenuKeys(
  clStockW* pSmart, 
  clStockW::Section hPopup, 
  const TIdentifyDefination& DefTable,
  GXMENUEX_TEMPLATE_ITEM& MenuItem)
{
  //clStockW::Section hKeys;
  clStockW::ATTRIBUTE val;  

  int nTextLength = 0;
  //InlSetZeroT(MenuItem);
  MenuItem.Clear();

  if(hPopup.FirstKey(val))
  {
    do {
      clStringW strKeyName = val.KeyName();
      if(strKeyName == L"Text")
      {
        //TRACEW(L"%s\n", val.ToString());
        GXSTRCPYN<wch>(MenuItem.szText, val.ToString(), 1024);
        nTextLength = GXSTRLEN(MenuItem.szText);
      }
      else if(strKeyName == L"Name")
      {
        //TRACEW(L"%s\n", val.ToString());
        TIdentifyDefination::const_iterator it = DefTable.find(clStringA(val.ToString()));
        MenuItem.head.menuId = it != DefTable.end() ? it->second : 0;
      }
      else if(strKeyName == L"State")
      {
        //TRACEW(L"%s\n", val.ToString());
        MenuItem.head.dwState = DlgXM::ParseCombinedFlags(val.ToString(), L"GXMFS_", DlgXM::CMC_MenuState);
      }
    } while(val.NextKey());
  }
  return TRUE;
}

GXBOOL LoadMenuTemplateFromStockSectionW(
  clStockW* pSmart, 
  clStockW::Section hPopup, 
  clBuffer* pBuffer, 
  const TIdentifyDefination& DefTable)
{
  GXMENUEX_TEMPLATE_ITEM MenuItem;
  GXBOOL bValidPopup;
  GXSIZE_T nLastOffset = 0;

  clStockW::Section hItems = hPopup.Open(NULL);
  while(hItems)
  {
    clStringW strSectName = hItems.SectionName();
    
    if(strSectName == L"MenuItem")
    {
      IntEnumMenuKeys(pSmart, hItems, DefTable, MenuItem);

      if(bValidPopup && MenuItem.IsValid()) {
        nLastOffset = pBuffer->GetSize();
        pBuffer->Append(&MenuItem, MenuItem.GetSize());
      }
    }
    else if(strSectName == L"Popup")
    {
      IntEnumMenuKeys(pSmart, hItems, DefTable, MenuItem);

      bValidPopup = MenuItem.IsValid();
      GXDWORD dwHelpId = 0;
      if(bValidPopup) {
        MenuItem.head.bResInfo |= 0x01;
        nLastOffset = pBuffer->GetSize();
        pBuffer->Append(&MenuItem, MenuItem.GetSize());
        pBuffer->Append(&dwHelpId, sizeof(dwHelpId));
      }

      LoadMenuTemplateFromStockSectionW(pSmart, hItems, pBuffer, DefTable);
      MenuItem.Clear();
    }

    if( ! hItems.NextSection()) {
      GXMENUEX_TEMPLATE_ITEM* pLastItem = (GXMENUEX_TEMPLATE_ITEM*)((GXLPBYTE)pBuffer->GetPtr() + nLastOffset);
      ASSERT(pLastItem->head.bResInfo == 0x01 || pLastItem->head.bResInfo == 0x00);
      pLastItem->head.bResInfo |= 0x80;
      //pSmart->CloseHandle(hItems);
      break;
    }
  }
  return TRUE;
}

GXBOOL LoadIdentifyDefinationW(GXLPCWSTR szFilename, GXLPCWSTR szSmartPath, TIdentifyDefination& DefTable)
{
  clStockW sp;
  if(sp.LoadW(szFilename))
  {
    clStockW::Section hDef = sp.Open(szSmartPath);
    if(hDef) {
      return FALSE;
    }

    clStockW::ATTRIBUTE val;
    //clStockW::Section hEnumKey = sp.FindFirstKey(hDef, val);
    if(hDef.FirstKey(val))
    {
      do {
        DefTable[clStringA(val.KeyName())] = val.ToInt();

        //if( ! sp.FindNextKey(hEnumKey, val)) {
        //  //sp.CloseHandle(hEnumKey);
        //  break;
        //}
      } while(val.NextKey());
    }
    //sp.CloseHandle(hDef);
    return TRUE;
  }
  return FALSE;
}

GXBOOL LoadMenuTemplateFromStockW(clStockW* pSmart, GXLPCWSTR szName, clBuffer* pBuffer)
{
  clStockW::Section hSect = NULL;
  hSect = pSmart->Open(szName);
  if(hSect)
  {
    clStockW::ATTRIBUTE val;
    TIdentifyDefination DefTable;
    GXBOOL bval = TRUE;
    if(hSect.GetKey(L"Defination", val))
    {
      clStringW strDesc = val.ToString();
      clStringW strFilename;
      clStringW strSmartPath;
      strDesc.DivideBy(L'|', strFilename, strSmartPath);
      bval = LoadIdentifyDefinationW(strFilename, strSmartPath, DefTable);
      //TRACEW(L"%s\n", val.ToString());
    }

    if(bval) {
      GXMENUEX_TEMPLATE_HEADER header;
      header.wVersion = 1;
      header.wOffset = 4; // MSDN: If the first item definition immediately follows the dwHelpId member, this member should be 4.
      header.dwHelpId = 0;
      pBuffer->Append(&header, sizeof(header));
      bval = LoadMenuTemplateFromStockSectionW(pSmart, hSect, pBuffer, DefTable);
    }

    //pSmart->CloseHandle(hSect);
    return bval;
  }
  return FALSE;
}