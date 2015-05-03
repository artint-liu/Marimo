#include "GrapX.H"
#include "User/GrapX.Hxx"
#include "clstd/Smart/smartstream.h"
#include "clstd/Smart/SmartProfile.h"

#include "Include/GUnknown.H"
#include "Include/GResource.H"
//#include "Include/GXFont.H"
//#include "Include/GXGraphics.H"
//#include "Include/GXUser.H"
//#include "Include/GXCanvas.H"
#include "Include/DataPool.H"
#include "Include/DataPoolVariable.H"
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
  SmartProfileW* pSmart, 
  SmartProfileW::HANDLE hPopup, 
  const TIdentifyDefination& DefTable,
  GXMENUEX_TEMPLATE_ITEM& MenuItem)
{
  SmartProfileW::HANDLE hKeys;
  SmartProfileW::VALUE val;  

  int nTextLength = 0;
  //InlSetZeroT(MenuItem);
  MenuItem.Clear();

  hKeys = pSmart->FindFirstKey(hPopup, val);
  while(hKeys != NULL) {
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
    if( ! pSmart->FindNextKey(hKeys, val)) {
      pSmart->CloseHandle(hKeys);
      break;
    }
  }
  return TRUE;
}

GXBOOL LoadMenuTemplateFromSmartProfileSectionW(
  SmartProfileW* pSmart, 
  SmartProfileW::HANDLE hPopup, 
  clBuffer* pBuffer, 
  const TIdentifyDefination& DefTable)
{
  GXMENUEX_TEMPLATE_ITEM MenuItem;
  GXBOOL bValidPopup;
  GXSIZE_T nLastOffset = 0;

  SmartProfileW::HANDLE hItems = pSmart->FindFirstSection(hPopup, FALSE, NULL, NULL);
  while(hItems != NULL)
  {
    clStringW strSectName = hItems->itSection.ToString();
    
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

      LoadMenuTemplateFromSmartProfileSectionW(pSmart, hItems, pBuffer, DefTable);
      MenuItem.Clear();
    }

    if( ! pSmart->FindNextSection(hItems)) {
      GXMENUEX_TEMPLATE_ITEM* pLastItem = (GXMENUEX_TEMPLATE_ITEM*)((GXLPBYTE)pBuffer->GetPtr() + nLastOffset);
      ASSERT(pLastItem->head.bResInfo == 0x01 || pLastItem->head.bResInfo == 0x00);
      pLastItem->head.bResInfo |= 0x80;
      pSmart->CloseHandle(hItems);
      break;
    }
  }
  return TRUE;
}

GXBOOL LoadIdentifyDefinationW(GXLPCWSTR szFilename, GXLPCWSTR szSmartPath, TIdentifyDefination& DefTable)
{
  SmartProfileW sp;
  if(sp.LoadW(szFilename))
  {
    SmartProfileW::HANDLE hDef = sp.OpenSection(szSmartPath);
    if(hDef == NULL) {
      return FALSE;
    }

    SmartProfileW::VALUE val;
    SmartProfileW::HANDLE hEnumKey = sp.FindFirstKey(hDef, val);
    while(hEnumKey != NULL)
    {
      DefTable[clStringA(val.KeyName())] = val.ToInt();

      if( ! sp.FindNextKey(hEnumKey, val)) {
        sp.CloseHandle(hEnumKey);
        break;
      }
    }
    sp.CloseHandle(hDef);
    return TRUE;
  }
  return FALSE;
}

GXBOOL LoadMenuTemplateFromSmartProfileW(SmartProfileW* pSmart, GXLPCWSTR szName, clBuffer* pBuffer)
{
  SmartProfileW::HANDLE hSect = NULL;
  hSect = pSmart->OpenSection(szName);
  if(hSect != NULL)
  {
    SmartProfileW::VALUE val;
    TIdentifyDefination DefTable;
    GXBOOL bval = TRUE;
    if(pSmart->FindKey(hSect, L"Defination", val))
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
      bval = LoadMenuTemplateFromSmartProfileSectionW(pSmart, hSect, pBuffer, DefTable);
    }

    pSmart->CloseHandle(hSect);
    return bval;
  }
  return FALSE;
}