#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

#include <Smart/smartstream.h>
#include <Smart/Stock.h>

// 平台相关
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataInfrastructure.H"
#include "clPathFile.H"

// 私有头文件
#include "User/GXWindow.h"
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXKernel.H"
#include "Controls/GXUICtrlBase.h"
#include "Controls/GXUIStatic.h"
#include "Controls/GXUIButton.h"
#include "Controls/GXUIToolbar.h"
#include "Controls/GXUIList.h"
#include "Controls/GXUISlider.h"
#include "Controls/PropertySheet.h"
#include "Controls/PropertyList.h"
#include "User/UILayoutMgr.h"
#include "GrapX/gxDevice.H"

extern GXWNDCLASSEX WndClassEx_Menu;
extern GXWNDCLASSEX WndClassEx_MyButton;
extern GXWNDCLASSEX WndClassEx_MyEdit;
extern GXWNDCLASSEX WndClassEx_MyEdit_1_3_30;
extern GXWNDCLASSEX WndClassEx_MyListbox;
extern GXWNDCLASSEX WndClassEx_MyStatic;
extern GXWNDCLASSEX WndClassEx_GXUIEdit_1_3_30;
extern GXBOOL LoadMenuTemplateFromStockW(clStockW* pSmart, GXLPCWSTR szName, clBuffer* pBuffer);
extern GXLRESULT GXCALLBACK CommDialogWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
extern GXLRESULT GXCALLBACK CommDialogWndProcEx(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);

struct DLGEXTRA
{
  GXLRESULT   lMsgResult;
  GXDLGPROC   pDlgProc;
  GXDWORD_PTR dwUserData;
  DLGLOG*     sLog;
};

static GXWNDCLASSEX WndClassEx_Dialog = { sizeof(GXWNDCLASSEX), GXCS_DBLCLKS | GXCS_PARENTDC, CommDialogWndProc, 0L, sizeof(DLGEXTRA),
  (GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, GXIDC_ARROW), NULL, NULL,
  GXWC_DIALOGW, NULL };

static GXWNDCLASSEX WndClassEx_DialogEx = { sizeof(GXWNDCLASSEX), GXCS_DBLCLKS | GXCS_PARENTDC, CommDialogWndProcEx, 0L, sizeof(DLGEXTRA),
  NULL, NULL, gxLoadCursor(NULL, GXIDC_ARROW), NULL, NULL,
  GXWC_DIALOGEXW, NULL };

//extern long g_SystemMetrics[];

#pragma pack(2)
typedef struct __tagGXDLGTEMPLATEEX{   
   GXWORD   dlgVer; 
   GXWORD   signature; 
   GXDWORD  helpID; 
   GXDWORD  exStyle; 
   GXDWORD  style; 
   GXWORD   cDlgItems; 
   short  x; 
   short  y; 
   short  cx; 
   short  cy; 
//     sz_Or_Ord menu;         // name or ordinal of a menu resource
//     sz_Or_Ord windowClass;  // name or ordinal of a window class
//     GXWCHAR  title[titleLen]; // title string of the dialog box
} GXDLGTEMPLATEEX, *LPGXDLGTEMPLATEEX; 

typedef struct __tagGXDLGTEMPLATEEX_1{
  short  pointsize;       // only if DS_SETFONT flag is set
  short  weight;          // only if DS_SETFONT flag is set
  short  bItalic;         // only if DS_SETFONT flag is set
//     GXWCHAR  font[fontLen];   // typeface name, if DS_SETFONT is set
} GXDLGTEMPLATEEX_1, *LPGXDLGTEMPLATEEX_1;

typedef struct __tagGXDLGITEMTEMPLATEEX{  
  GXDWORD  helpID; 
  GXDWORD  exStyle; 
  GXDWORD  style; 
  short  x; 
  short  y; 
  short  cx; 
  short  cy; 
  GXWORD   id; 
  //sz_Or_Ord windowClass; // name or ordinal of a window class
  //sz_Or_Ord title;       // title string or ordinal of a resource
  //GXWORD   extraCount;     // bytes of following creation data
}GXDLGITEMTEMPLATEEX, *LPGXDLGITEMTEMPLATEEX; 

#pragma pack(4)

GXHWND gxIntCreateDialogFromFileW(GXHINSTANCE  hInstance, GXLPCWSTR lpFilename, GXLPCWSTR lpDlgName, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);

// 如果是字符串则返回 TRUE
GXBOOL GetVariableData(GXLPBYTE lpBytes, GXLPINT pRetLen)
{
  GXLPSHORT  pwData = (GXLPSHORT)lpBytes;
  GXINT    nLen = 1;
  if(*pwData == 0)
  {
    *pRetLen = 1;
    return FALSE;
  }
  else if(*pwData == -1)
  {
    *pRetLen = 2;
    return FALSE;
  }
  while(*pwData != 0)
  {
    nLen++;
    pwData++;
  };
  *pRetLen = (GXINT)nLen;
  return TRUE;
}

namespace DlgXM
{
  class DlgSmartFile : public clStockW
  {
  public:
    typedef clStockW::Section Section;
    typedef clStockW::ATTRIBUTE ATTRIBUTE;
    typedef clvector<GXTBBUTTON> TBButtonArray;
  private:
    //GXHRESULT LoadLayoutBinaryItem(HANDLE hHandle, GXUI::DlgLayout::BINARYITEM* pBinaryItemDesc);
    //GXHRESULT LoadLayoutBinary(HANDLE hHandle, GXUI::DlgLayout::BINARY* pBinaryDesc);
    GXHRESULT LoadLayoutPanel     (clStockW::Section hHandle, GXUI::DLGPANEL* pPanel);
    GXHRESULT LoadTBButtonItem    (clStockW::Section hHandle, GXTBBUTTON& sTBButton);
  public:
    GXHRESULT GetBasicParam       (clStockW::Section hHandle, DLGBASICPARAMW* pwbp, GXDefinitionArrayW* pDefinitions = NULL);
    GXHRESULT GetFontParam        (clStockW::Section hHandle, DLGFONTPARAMW* pwfp);
    GXHRESULT LoadBtnSpriteCfg    (clStockW::Section hHandle, GXLPCWSTR szSect, DLGBTNSPRITE* pBtnSprite);  // hHandle 是父句柄
    GXHRESULT LoadSliderSpriteCfg (clStockW::Section hHandle, GXLPCWSTR szSect, DLGSLIDERSPRITE* pSliderSprite);  // hHandle 是父句柄
    GXHRESULT LoadLayout          (clStockW::Section hHandle, GXUI::DLGPANEL* pPanel);
    GXHRESULT LoadTBButton        (clStockW::Section hHandle, TBButtonArray& aTBButton);
  };

  GXHRESULT DlgSmartFile::GetBasicParam(StockW::Section hHandle, DLGBASICPARAMW* pwbp, GXDefinitionArrayW* pDefinitions)
  {
    clStringW strStyle;
    clStringW strExStyle;
    ATTRIBUTE val;

    if(pDefinitions &&  ! pDefinitions->empty()) {
      pDefinitions->clear();
    }

    if(hHandle.FirstKey(val))
    {
      do 
      {
        clStringW strKeyName = val.KeyName();
        if(strKeyName == L"Left") {
          pwbp->regn.left = GetPixelSizeFromMarkW(val.ToString());
        }
        else if(strKeyName == L"Top") {
          pwbp->regn.top = GetPixelSizeFromMarkW(val.ToString());
        }
        else if(strKeyName == L"Width") {
          pwbp->regn.width = GetPixelSizeFromMarkW(val.ToString());
        }
        else if(strKeyName == L"Height") {
          pwbp->regn.height = GetPixelSizeFromMarkW(val.ToString());
        }
        else if(strKeyName == L"Caption" || strKeyName == L"Text") {
          pwbp->strCaption = val.ToString();
        }
        else if(strKeyName == L"Menu") {
          pwbp->strMenu = val.ToString();
        }
        else if(strKeyName == L"Name") {
          pwbp->strName = val.ToString();
        }
        else if(strKeyName == L"Style") {
          strStyle = val.ToString();
        }
        else if(strKeyName == L"ExStyle") {
          strExStyle = val.ToString();
        }
        else if(pDefinitions != NULL)
        {
          GXDefinitionW def;
          def.Name = strKeyName;
          def.Value = val.ToString();
          pDefinitions->push_back(def);
        }
      } while (val.NextKey());
    }

    //FindClose(hKey);
    pwbp->strStyle = strStyle;
    pwbp->strExStyle = strExStyle;

    //pwbp->regn.left   = GetPixelSizeFromMarkW(FindKeyAsString(hHandle, L"Left", L"0"));
    //pwbp->regn.top    = GetPixelSizeFromMarkW(FindKeyAsString(hHandle, L"Top", L"0"));
    //pwbp->regn.width  = GetPixelSizeFromMarkW(FindKeyAsString(hHandle, L"Width", L"128"));
    //pwbp->regn.height = GetPixelSizeFromMarkW(FindKeyAsString(hHandle, L"Height", L"128"));
    //pwbp->strCaption  = FindKeyAsString(hHandle, L"Caption", L"");
    //pwbp->strName     = FindKeyAsString(hHandle, L"Name", L"");

    //strStyle    = FindKeyAsString(hHandle, L"Style", L"");
    //strExStyle  = FindKeyAsString(hHandle, L"ExStyle", L"");

    if(strStyle.GetLength())  {
      pwbp->dwStyle = DlgXM::ParseCombinedFlags(strStyle, L"GXWS_", DlgXM::CMC_WndStyle);
    }

    if(strExStyle.GetLength())  {
      pwbp->dwExStyle = DlgXM::ParseCombinedFlags(strExStyle, L"GXWS_EX_", DlgXM::CMC_WndExStyle);
    }
    return 0;
  }

  GXHRESULT DlgSmartFile::GetFontParam(StockW::Section hHandle, DLGFONTPARAMW* pwfp)
  {
    pwfp->strFontName = hHandle.GetKeyAsString(L"FontName", L"");
    pwfp->nFontSize   = hHandle.GetKeyAsInteger(L"FontSize", 0);

    return 0;
  }

  GXHRESULT DlgSmartFile::LoadBtnSpriteCfg(StockW::Section hHandle, GXLPCWSTR szSect, DLGBTNSPRITE* pBtnSprite)  // hHandle 是父句柄
  {
    StockW::Section hSprite = hHandle.Open(szSect);
    if(hSprite.IsValid())
    {
      pBtnSprite->strResource = hSprite.GetKeyAsString(L"Resource", L"");
      pBtnSprite->strNormal   = hSprite.GetKeyAsString(L"Normal", L"");
      pBtnSprite->strHover    = hSprite.GetKeyAsString(L"Hover", L"");
      pBtnSprite->strPressed  = hSprite.GetKeyAsString(L"Pressed", L"");
      pBtnSprite->strDisable  = hSprite.GetKeyAsString(L"Disable", L"");
      pBtnSprite->strDefault  = hSprite.GetKeyAsString(L"Default", L"");

      //FindClose(hSprite);
      return GX_OK;
    }
    return -1;
  }

  GXHRESULT DlgSmartFile::LoadSliderSpriteCfg(StockW::Section hHandle, GXLPCWSTR szSect, DLGSLIDERSPRITE* pBtnSprite)  // hHandle 是父句柄
  {
    StockW::Section hSprite = hHandle.Open(szSect);
    if(hSprite.IsValid())
    {
      pBtnSprite->strResource   = hSprite.GetKeyAsString(L"Resource", L"");
      pBtnSprite->strHandle     = hSprite.GetKeyAsString(L"Handle", L"");
      pBtnSprite->strEmpty      = hSprite.GetKeyAsString(L"EmptyBar", L"");
      pBtnSprite->strFull       = hSprite.GetKeyAsString(L"FullBar", L"");
      pBtnSprite->strDial       = hSprite.GetKeyAsString(L"Dial", L"");
      pBtnSprite->strVertEmpty  = hSprite.GetKeyAsString(L"VertEmptyBar", L"");
      pBtnSprite->strVertFull   = hSprite.GetKeyAsString(L"VertFullBar", L"");
      pBtnSprite->strVertDial   = hSprite.GetKeyAsString(L"VertDial", L"");

      //FindClose(hSprite);
      return GX_OK;
    }
    return -1;
  }

  GXHRESULT DlgSmartFile::LoadLayoutPanel(StockW::Section hHandle, GXUI::DLGPANEL* pPanel)
  {
    ATTRIBUTE Value;
    //HANDLE hKey = FindFirstKey(hHandle, Value);
    const int nScaleMaxIdx = sizeof(pPanel->fScale) / sizeof(float);
    if(hHandle.FirstKey(Value)) {
      int nScaleIdx = 0;
      STATIC_ASSERT(nScaleMaxIdx == 2);
      do 
      {
        const clStringW& strKey = Value.KeyName();
        if(strKey == L"Name") {
          pPanel->strName = Value.ToString();
          //pPanel->dwStyle |= LPS_WNDITEM;
        }
        else if(strKey == L"Style")
        {
          pPanel->dwStyle = DlgXM::ParseCombinedFlags(Value.ToString(), 
            L"LPS_", DlgXM::CMC_LayoutPanel);
        }
        else if(strKey == L"Scale" && nScaleIdx < nScaleMaxIdx)
        {
          const clStringW& strValue = Value.ToString();
          if(strValue.Back() == L'%') {
            pPanel->fScale[nScaleIdx] = (float)clstd::_xstrtof((const wch*)strValue) * 0.01f;
            clClamp(0.0f, 1.0f, &pPanel->fScale[nScaleIdx]);
            nScaleIdx++;
          }
        }
        else if(strKey == L"List")
        {
          clStringArrayW aWndNames;
          clstd::ResolveString(Value.ToString(), ';', aWndNames);
          pPanel->aPanels.reserve(aWndNames.size());

          for(clStringArrayW::iterator it = aWndNames.begin();
            it != aWndNames.end(); ++it)
          {
            if(it->IsEmpty()) {
              continue;
            }
            else if(*it == L"\\n" && ! pPanel->aPanels.empty()) {
              pPanel->aPanels.back().dwStyle |= LPS_INT_RETURN;
              continue;
            }

            GXUI::DLGPANEL WndItem;
            WndItem.strName = *it;
            WndItem.dwStyle = LPS_WNDITEM;
            pPanel->aPanels.push_back(WndItem);
          }
        }
        //else if(strKey == L"AlignType")
        //{
        //  clStringW strValue = Value.ToString();
        //  strValue.MakeUpper();
        //  if(strValue == L"FIXEDFIRST") {
        //    pPanel->eAlignType = GXUI::PAT_FixedFirst;
        //  }
        //  else if(strValue == L"FIXEDSECOND") {
        //    pPanel->eAlignType = GXUI::PAT_FixedSecond;
        //  }
        //  else if(strValue == L"ALIGNFIRST") {
        //    pPanel->eAlignType = GXUI::PAT_AlignFirst;
        //  }
        //  else if(strValue == L"ALIGNSECOND") {
        //    pPanel->eAlignType = GXUI::PAT_AlignSecond;
        //  }
        //}
      } while(Value.NextKey());
    }
    const GXDWORD dwType = pPanel->dwStyle & LPS_TYPEMASK;
    if(dwType != LPS_WNDITEM)
    {
      Section hItemSect = hHandle.Open(NULL);
      if(hItemSect.IsValid())
      {
        do 
        {
          clStringW strName = hItemSect.SectionName();
          GXUI::DLGPANEL Panel;
          //Panel.eType      = GXUI::PT_Unknown;
          //Panel.eAlignType = GXUI::PAT_AlignFirst;
          for(int i = 0; i < nScaleMaxIdx; i++) {
            Panel.fScale[i] = 0.5f;
          }
          Panel.dwStyle = 0;
          if(strName == L"Item" || strName == L"Panel")
          {
            if(GXSUCCEEDED(LoadLayoutPanel(hItemSect, &Panel))) {
              pPanel->aPanels.push_back(Panel);
            }
          }

          if((dwType == LPS_HBINARYPANEL || dwType == LPS_VBINARYPANEL) &&
            pPanel->aPanels.size() >= 2) {
              break;
          }

        } while(hItemSect.NextSection());
      }
      //FindClose(hItemSect);
    }
    //FindClose(hKey);
    return GX_OK;
  }

  GXHRESULT DlgSmartFile::LoadLayout(StockW::Section hHandle, GXUI::DLGPANEL* pPanel)
  {
    Section hSect = hHandle.Open(NULL);
    GXHRESULT hval = GX_FAIL;
    if(hSect.IsValid())
    {
      do 
      {
        clStringW strName = hSect.SectionName();
        if(strName == L"Panel")
        {
          if(GXSUCCEEDED(LoadLayoutPanel(hSect, pPanel))) {
            hval = GX_OK;
          }
          break;
        }
      } while(hSect.NextSection());
      //FindClose(hSect);
    }
    return GX_OK;
  }

  GXHRESULT DlgSmartFile::LoadTBButtonItem(StockW::Section hHandle, GXTBBUTTON& sTBButton)
  {
    ATTRIBUTE Value;
    if(hHandle.FirstKey(Value)) {
      do 
      {
        const clStringW& strKey = Value.KeyName();
        if(strKey == L"Bitmap") {
          sTBButton.iBitmap = Value.ToInt();
        }
        else if(strKey == L"Command")
        {
          clStringW* pStr = new(&sTBButton.idCommand) clStringW();
          *pStr = Value.ToString();
        }
        else if(strKey == L"Text")
        {
          clStringW* pStr = new(&sTBButton.iString) clStringW();
          *pStr = Value.ToString();
        }
        else if(strKey == L"Style")
        {
          clStringW strStyle = Value.ToString();
          sTBButton.fsStyle = (GXBYTE)DlgXM::ParseCombinedFlags(strStyle, L"BTNS_", DlgXM::CMC_ToolbarBtnStyle);
        }
        else if(strKey == L"State")
        {
          clStringW strState = Value.ToString();
          sTBButton.fsState = (GXBYTE)DlgXM::ParseCombinedFlags(strState, L"TBSTATE_", DlgXM::CMC_ToolbarState);
        }
      } while(Value.NextKey());
      //FindClose(hKey);
    }
    return GX_OK;
  }

  GXHRESULT DlgSmartFile::LoadTBButton(StockW::Section hHandle, TBButtonArray& aTBButton)
  {
    StockW::Section hButtons = hHandle.Open(L"Button");
    if(hButtons.IsValid())
    {
      do {
        GXTBBUTTON btn = {-1};
        LoadTBButtonItem(hButtons, btn);
        aTBButton.push_back(btn);        
      }while(hButtons.NextSection());
      //FindClose(hButtons);
    }
    return GX_OK;
  }
} // namespace DlgXM

GXHWND GXDLLAPI gxCreateDialogParamW(
                GXHINSTANCE hInstance,
                GXLPCWSTR   lpTemplate,
                GXHWND      hParent,
                GXDLGPROC   lpDialogFunc,
                GXLPARAM    lParam
                )
{
  if(hInstance == NULL || (IS_PTR(lpTemplate) && lpTemplate[0] == '@'))  {
    return gxIntCreateDialogFromFileW(NULL, lpTemplate, _T("Dialog"), hParent, lpDialogFunc, lParam);
  }

  GXHRSRC   hRsc;
  GXHGLOBAL hGlobal;
  GXLPBYTE  lpBytes;
  GXHWND    hDialogFrame;
  GXINT     nVariableLen;

  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());//_gxGetStationFromInstance(_hInstance);
  // lpStation->hInstApp

  VERIFY(hRsc = gxFindResourceW(hInstance, lpTemplate, GXRT_DIALOG));
  if(hRsc == NULL)
    return NULL;
  
  VERIFY(hGlobal = gxLoadResource(hInstance, hRsc));
  if(hGlobal == NULL)
    return NULL;

  VERIFY(lpBytes = (GXLPBYTE)gxLockResource(hGlobal));
  if(lpBytes == NULL)
    return NULL;
  
  LPGXDLGTEMPLATEEX   lpDlgTemplate = (LPGXDLGTEMPLATEEX)lpBytes;
  LPGXDLGTEMPLATEEX_1 lpDlgTemplate_1;

  GXPOINT  ptPos, ptBaseUnit;
  GXSIZE  ptSize;
  GXUSHORT idMenu;
  GXLONG lRet = gxGetDialogBaseUnits();
  ptBaseUnit.x = GXLOWORD(lRet);
  ptBaseUnit.y = GXHIWORD(lRet);


  // 计算像素尺寸
  ptPos.x    = (ptBaseUnit.x * lpDlgTemplate->x) / 4;
  ptPos.y    = (ptBaseUnit.y * lpDlgTemplate->y) / 8;
  ptSize.cx  = (GXLONG)(ptBaseUnit.x * lpDlgTemplate->cx) / 4;
  ptSize.cy  = (GXLONG)(ptBaseUnit.y * lpDlgTemplate->cy) / 8;

  if(lpDlgTemplate->style & GXDS_CENTER)
  {
    //D3DPRESENT_PARAMETERS& d3dpp = lpStation->d3dpp;
    ptPos.x = (lpStation->nWidth  - ptSize.cx) >> 1;
    ptPos.y = (lpStation->nHeight - ptSize.cy) >> 1;
  }

  //pDialogFrame = (CGXFrame*)GXNEW GXBYTE[100];
  //pDialogFrame:CGXFrame(pFrameParent, ptPos.x, ptPos.y, ptSize.cx + (FRAME_NC_LEFT + FRAME_NC_RIGHT), ptSize.cy + (FRAME_NC_TOP + FRAME_NC_BOTTOM));

  gxRegisterClassExW(&WndClassEx_Dialog);

  TRACE("struct align: %d\n",sizeof(GXDLGTEMPLATEEX));
  lpBytes += sizeof(GXDLGTEMPLATEEX);


  GetVariableData(lpBytes, &nVariableLen);
  ASSERT(nVariableLen == 2 || nVariableLen == 1);
  TRACE("menu length:%d\n",nVariableLen);
  idMenu = ((GXUSHORT*)lpBytes)[nVariableLen - 1];
  lpBytes += nVariableLen * sizeof(GXSHORT);

  GXLPARAM aParam[2] = {
    lParam,            // 用户参数
    (GXLPARAM)lpDialogFunc    // 对话框处理函数
  };
  hDialogFrame = gxCreateWindowExW( lpDlgTemplate->exStyle, WndClassEx_Dialog.lpszClassName,
    (GXLPCWSTR)L"", lpDlgTemplate->style, 
    ptPos.x, ptPos.y, ptSize.cx + (FRAME_NC_EDGE_LEFT + FRAME_NC_EDGE_RIGHT), ptSize.cy + (FRAME_NC_EDGE_CAPTION + FRAME_NC_EDGE_BOTTOM), 
    hParent, (GXHMENU)idMenu, hInstance, (GXLPVOID)&aParam);

  if(hDialogFrame == NULL) {
    return NULL;
  }
  //gxSetWindowLongW(hDialogFrame, 0, (GXLONG_PTR)lpDialogFunc);
  //gxUnregisterClassW(COMMONDIALOG, NULL);
  //pDialogFrame = GXNEW CFrame(pFrameParent, ptPos.x, ptPos.y, ptSize.cx + (FRAME_NC_LEFT + FRAME_NC_RIGHT), ptSize.cy + (FRAME_NC_TOP + FRAME_NC_BOTTOM));
  GXWND_PTR(hDialogFrame)->m_uState |= WIS_ISDIALOG;


  
  GetVariableData(lpBytes, &nVariableLen);
  TRACE("class name length:%d\n",nVariableLen);

  // 得到窗口标题
  lpBytes += nVariableLen * sizeof(GXSHORT);
  if(GetVariableData(lpBytes, &nVariableLen) != FALSE)
  {
    TRACEW(L"caption is: %s, length:%d\n", (GXLPWCHAR)lpBytes,nVariableLen);
    gxSetWindowText(hDialogFrame, (GXLPWCHAR)lpBytes);
  }

  lpBytes += (nVariableLen * sizeof(GXSHORT));
  lpDlgTemplate_1 = (LPGXDLGTEMPLATEEX_1) lpBytes;
  lpBytes += sizeof(GXDLGTEMPLATEEX_1);
  TRACE("point size:%d\n",lpDlgTemplate_1->pointsize);
  TRACE("sizeof(GXDLGTEMPLATEEX_1) = %d\n",sizeof(GXDLGTEMPLATEEX_1));

  if(GetVariableData(lpBytes, &nVariableLen) != FALSE)
  {
    TRACEW(L"font face name:%s, length: %d\n", (GXLPWCHAR)lpBytes,nVariableLen);
  }

  lpBytes = (GXLPBYTE)BYTE_ALIGN_4((GXLONG_PTR)lpBytes + nVariableLen * sizeof(GXSHORT));    // 可能会有Bug,因为Lock的内存有可能不是4字节对齐
  //////////////////////////////////////////////////////////////////////////
  for(GXINT i = 0; i < lpDlgTemplate->cDlgItems; i++)
  {
    LPGXDLGITEMTEMPLATEEX  lpDlgItem = (LPGXDLGITEMTEMPLATEEX)lpBytes;
    GXLPSHORT       pShort;
    GXHWND          hControl = NULL;
    TRACE("x:%d, y:%d, cx:%d, cy:%d\n",lpDlgItem->x, lpDlgItem->y, lpDlgItem->cx, lpDlgItem->cy);
    ptPos.x    = (ptBaseUnit.x * lpDlgItem->x) / 4/* + FRAME_NC_EDGE_LEFT*/;
    ptPos.y    = (ptBaseUnit.y * lpDlgItem->y) / 8/* + FRAME_NC_EDGE_CAPTION*/;
    ptSize.cx  = (ptBaseUnit.x * lpDlgItem->cx) / 4;
    ptSize.cy  = (ptBaseUnit.y * lpDlgItem->cy) / 8;

    lpBytes = (GXLPBYTE)BYTE_ALIGN_4((GXLONG_PTR)lpBytes + sizeof(GXDLGITEMTEMPLATEEX));
    TRACE("sizeof(GXDLGITEMTEMPLATEEX) = %d\n",sizeof(GXDLGITEMTEMPLATEEX));

    GetVariableData(lpBytes, &nVariableLen);
    pShort = (GXLPSHORT)lpBytes;
    //if(*pShort == -1)
    if(nVariableLen == 2)
    {
      TRACE("Class type:");
      GXSHORT dType = pShort[1];
      GXLPSHORT pTitle;
      GXBOOL bId = FALSE;

      // 定位到控件标题
      pTitle = (GXLPSHORT)BYTE_ALIGN_4((GXLONG_PTR)pShort+2);
      if(*pTitle == (GXSHORT)0xffff)
      {
        bId = TRUE;
        pTitle++;
      }
      else if(*pTitle != 0)
      {
        //if(pControl != NULL)
        //  pControl->gxSetWindowText((GXLPCWSTR)pShort);
        TRACEW(L"control title:%s\n",pTitle);
        //pTitle += (GXSTRLEN((GXLPCWSTR)pTitle) + 1);
      }


      switch(dType)
      {
      case 0x0080:  // Button
        TRACE("Button\n");
        hControl = gxCreateWindowExW(
          NULL, WndClassEx_MyButton.lpszClassName, (GXLPCWSTR)pTitle, lpDlgItem->style, 
          (int)ptPos.x, (int)ptPos.y, (int)ptSize.cx, (int)ptSize.cy, 
          hDialogFrame, (GXHMENU)lpDlgItem->id, NULL, 0);
        gxInvalidateRect(hControl, NULL, FALSE);
        break;
      case 0x0081:  // Edit
        TRACE("Edit\n");
        hControl = gxCreateWindowExW(
          NULL, WndClassEx_MyEdit.lpszClassName, (GXLPCWSTR)pTitle, lpDlgItem->style, 
          ptPos.x, ptPos.y, ptSize.cx, ptSize.cy, 
          hDialogFrame, (GXHMENU)lpDlgItem->id, NULL, 0);
        gxInvalidateRect(hControl, NULL, FALSE);
        break;
      case 0x0082:  // Static
        TRACE("Static\n");
        //pControl = (CGXFrame*)GXNEW CStatic(pDialogFrame, ptPos.x, ptPos.y, ptSize.cx, ptSize.cy, lpDlgItem->id);
        hControl = gxCreateWindowExW(
          NULL, WndClassEx_MyStatic.lpszClassName, (bId ? ((GXLPCWSTR)*pTitle):((GXLPCWSTR)pTitle)), lpDlgItem->style, 
          (int)ptPos.x, (int)ptPos.y, (int)ptSize.cx, (int)ptSize.cy, 
          hDialogFrame, (GXHMENU)lpDlgItem->id, NULL, 0);
        gxInvalidateRect(hControl, NULL, FALSE);
        break;
      case 0x0083:  // List box
        TRACE("List box\n");
        hControl = gxCreateWindowExW(
          NULL, WndClassEx_MyListbox.lpszClassName, (GXLPCWSTR)pTitle, lpDlgItem->style, 
          ptPos.x, ptPos.y, ptSize.cx, ptSize.cy, hDialogFrame, (GXHMENU)lpDlgItem->id, NULL, 0);
        gxInvalidateRect(hControl, NULL, FALSE);
        break;
      case 0x0084:  // Scroll bar
        TRACE("Scroll bar\n");
        break;
      case 0x0085:  // Combo box
        TRACE("Combo box\n");
        break;
      default:
        ASSERT(FALSE);
      }
      pShort += 2;
    }
    else if(nVariableLen == 1)
    {
      pShort++;
    }
    else
    {
      pShort += nVariableLen;
      TRACEW(L"ComCtrl Class:%d\n", lpBytes);
      hControl = gxCreateWindowExW(NULL, (GXLPCWSTR)lpBytes, (GXLPCWSTR)L"", lpDlgItem->style, ptPos.x, ptPos.y, ptSize.cx, ptSize.cy, hDialogFrame, (GXHMENU)lpDlgItem->id, NULL, 0);
      //ASSERT(FALSE);
    }
    // 定位到控件标题
    pShort = (GXLPSHORT)BYTE_ALIGN_4((GXLONG_PTR)pShort);
    if(*pShort == (GXSHORT)0xffff)
    {
      //ASSERT(FALSE);
      pShort+=2;
    }
    else if(*pShort == 0)
    {
      pShort++;
    }
    else
    {
      //if(pControl != NULL)
      //  pControl->gxSetWindowText((GXLPCWSTR)pShort);
      //TRACEW(L"control title:%s\n",pShort);
      pShort += (GXSTRLEN((GXLPCWSTR)pShort) + 1);
    }

    // 定位到控件附加字节
    pShort = (GXLPSHORT)BYTE_ALIGN_4((GXLONG_PTR)(pShort + (*pShort + 1)));
    lpBytes = (GXLPBYTE)pShort;

  }

  //gxSetWindowLongW(hDialogFrame, GXGWL_WNDPROC, (GXLONG_PTR)lpDialogFunc);  // 为啥要完全替换呢?
  gxSendMessageW(hDialogFrame, GXWM_INITDIALOG, NULL, lParam);
  return hDialogFrame;
}
//////////////////////////////////////////////////////////////////////////
//template<typename clString> struct stdext::less
//{
//  bool operator()(const clString& _Left, const clString& _Right) const
//  {
//    return _Left.compare(_Right) < 0;
//  }
//};
//struct stdext::hash_compare<clString, stdext::less<clString>>
//{
//  size_t operator()(const clString& str) const
//  {
//    return str.GetHash();
//  }
//};

//struct less
//{
//  bool operator()(const clStringW& _Left, const clStringW& _Right) const
//  {
//    return (_Left.Compare(_Right) < 0);
//  }
//};





typedef DlgXM::DlgSmartFile::Section Section;

GXHWND CreateDialogItem_Label( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGFONTPARAMW &dfp ) 
{
  GXHWND hItemWnd = NULL;
  DlgXM::DLGFONTPARAMW  dfpItem;

  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  clStringW strLayout = hDlgItem.GetKeyAsString(L"Layout", L"");
  if(strLayout.GetLength() > 0)
  {
    dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(strLayout, L"GXUISS_", DlgXM::CMC_StaticLabel);
  }

  clstd::TranslateEscapeCharacter(dbpItem.strCaption);

  GXUI::StaticLabel* pLabel = (GXUI::StaticLabel*)GXUI::Static::Create(hInstance, hDlgWnd, GXUI::Static::Label, dbpItem.strName, &dbpItem, &aDefinitions);
  hItemWnd = pLabel->Get();
  //gxSetWindowText(hItemWnd, dbpItem.strCaption);

  // 设置颜色
  clStringW strColor = hDlgItem.GetKeyAsString(L"Color", L"");
  if(strColor.GetLength() > 0)
  {
    pLabel->SetColor(DlgXM::GetColorFromMarkW(strColor));
  }

  file.GetFontParam(hDlgItem, &dfpItem);
  dfpItem.Inherit(&dfp);

  if(dfp.IsAvailable()) {
    pLabel->SetFont(dfpItem.strFontName, dfpItem.nFontSize);
  }

  return hItemWnd;
}

GXHWND CreateDialogItem_Rectangle( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGFONTPARAMW &dfp ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;

  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  GXUI::StaticRectangle* pRectangle = (GXUI::StaticRectangle*)GXUI::Static::Create(hInstance, hDlgWnd, GXUI::Static::Rectangle, dbpItem.strName, &dbpItem, &aDefinitions);
  hItemWnd = pRectangle->Get();
  gxSetWindowText(hItemWnd, dbpItem.strCaption);

  // 设置颜色
  clStringW strColor = hDlgItem.GetKeyAsString(L"Color", L"");
  if(strColor.GetLength() > 0)
  {
    pRectangle->SetColor(DlgXM::GetColorFromMarkW(strColor));
  }

  file.GetFontParam(hDlgItem, &dfpItem);
  dfpItem.Inherit(&dfp);

  if(dfp.IsAvailable()) {
    pRectangle->SetFont(dfpItem.strFontName, dfpItem.nFontSize);
  }          
  return hItemWnd;
}

GXHWND CreateDialogItem_Sprite( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;

  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);
  if( ! dbpItem.strStyle.IsEmpty()) {
    dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(dbpItem.strStyle, L"GXUISS_", DlgXM::CMC_StaticLabel);
  }

  GXUI::StaticSprite* pSprite = (GXUI::StaticSprite*)GXUI::Static::Create(hInstance, hDlgWnd, GXUI::Static::Sprite, dbpItem.strName, &dbpItem, &aDefinitions);
  hItemWnd = pSprite->Get();
  gxSetWindowText(hItemWnd, dbpItem.strCaption);

  clStringW strSpriteInfo = hDlgItem.GetKeyAsString(L"Sprite", L"");
  if(strSpriteInfo.GetLength() > 0)
  {
    clStringW strSpriteFile;
    clStringW strSprite;

    if(strSpriteInfo.DivideBy(L':', strSpriteFile, strSprite) != clStringW::npos &&
      strSpriteFile.IsNotEmpty() && strSprite.IsNotEmpty())
    {
      pSprite->SetSpriteByFilenameW(strSpriteFile);
      pSprite->SetByNameW(strSprite);
    }
  }
  return hItemWnd;
}

GXHWND CreateDialogItem_Button( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGBTNSPRITE sDlgTemplateBtnSprite, DlgXM::DLGFONTPARAMW dfp ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;

  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  clStringW strLayout = hDlgItem.GetKeyAsString(L"Layout", L"");
  if(strLayout.GetLength() > 0) {
    dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(strLayout, L"GXUIBS_", DlgXM::CMC_ButtonStyle);
  }

  GXUI::Button* pButton = GXUI::Button::Create(hInstance, hDlgWnd, dbpItem.strCaption, dbpItem.dwStyle, dbpItem.strName, &dbpItem.regn, &aDefinitions);
  hItemWnd = pButton->Get();

  // 载入Sprite配置
  DlgXM::DLGBTNSPRITE sDlgBtnSprite = sDlgTemplateBtnSprite;

  file.LoadBtnSpriteCfg(hDlgItem, L"Sprite", &sDlgBtnSprite);
  pButton->SetSprite(sDlgBtnSprite);


  file.GetFontParam(hDlgItem, &dfpItem);
  dfpItem.Inherit(&dfp);

  // 设置字体
  if(dfpItem.IsAvailable()) {
    pButton->SetFont(dfpItem.strFontName, dfpItem.nFontSize);
  }
  return hItemWnd;
}

GXHWND CreateDialogItem_WETree( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHWND hDlgWnd, GXHINSTANCE hInstance ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;

  file.GetBasicParam(hDlgItem, &dbpItem);

  hItemWnd = gxCreateWindowEx(dbpItem.dwExStyle, GXWE_TREEVIEWW, dbpItem.strCaption, 
    dbpItem.dwStyle | GXWS_VISIBLE | GXWS_CHILD, 
    dbpItem.regn.left, dbpItem.regn.top, dbpItem.regn.width, dbpItem.regn.height, hDlgWnd, NULL, hInstance, NULL);
  return hItemWnd;
}

GXHWND CreateDialogItem_WEListView( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHWND hDlgWnd, GXHINSTANCE hInstance ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  file.GetBasicParam(hDlgItem, &dbpItem);

  hItemWnd = gxCreateWindowEx(dbpItem.dwExStyle, GXWE_LISTVIEWW, dbpItem.strCaption, 
    dbpItem.dwStyle | GXWS_VISIBLE | GXWS_CHILD, 
    dbpItem.regn.left, dbpItem.regn.top, dbpItem.regn.width, dbpItem.regn.height, hDlgWnd, NULL, hInstance, NULL);
  return hItemWnd;
}

GXHWND CreateDialogItem_WEEdit( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHWND hDlgWnd, GXHINSTANCE hInstance ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  file.GetBasicParam(hDlgItem, &dbpItem);

  //clStringW strLayout = file.FindKeyAsString(hDlgItem, L"Layout", L"");
  //if(strLayout.GetLength() > 0) {
  //  dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(strLayout, L"GXUIBS_", DlgXM::CMC_ButtonStyle);
  //}

  hItemWnd = gxCreateWindowEx(dbpItem.dwExStyle, GXWE_EDITW_1_3_30, dbpItem.strCaption, 
    dbpItem.dwStyle | GXWS_VISIBLE | GXWS_CHILD, 
    dbpItem.regn.left, dbpItem.regn.top, dbpItem.regn.width, dbpItem.regn.height, hDlgWnd, NULL, hInstance, NULL);

  //GXUI::StaticRectangle* pEdit = (GXUI::StaticRectangle*)GXUI::Static::Create(hInstance, hDlgWnd, GXUI::Static::Rectangle, dbpItem.strName, &dbpItem);
  //hItemWnd = pEdit->Get();
  //gxSetWindowText(hItemWnd, dbpItem.strCaption);

  //GXUI::Button* pButton = GXUI::Button::Create(hInstance, hDlgWnd, dbpItem.strCaption, dbpItem.dwStyle, dbpItem.strName, &dbpItem.regn);
  //hItemWnd = pButton->Get();

  //// 载入Sprite配置
  //DlgXM::DLGBTNSPRITE sDlgBtnSprite = sDlgTemplateBtnSprite;

  //file.LoadBtnSpriteCfg(hDlgItem, L"Sprite", &sDlgBtnSprite);
  //pButton->SetSprite(sDlgBtnSprite.strResource, sDlgBtnSprite.strNormal, 
  //  sDlgBtnSprite.strHover, sDlgBtnSprite.strPressed, 
  //  sDlgBtnSprite.strDisable, sDlgBtnSprite.strDefault);

  // 设置字体
  //if(dfp.IsAvailable()) {
  //  pEdit->SetFont(dfp.strFontName, dfp.nFontSize);
  //}
  return hItemWnd;
}

GXHWND CreateDialogItem_PropSheet( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  file.GetBasicParam(hDlgItem, &dbpItem);

  auto pForm = GXUIEXT::PropertySheet::Form::Create(
    hInstance, hDlgWnd, dbpItem.strName, &dbpItem, NULL);
  hItemWnd = pForm->Get();
  return hItemWnd;
}

GXHWND CreateDialogItem_PropList( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  file.GetBasicParam(hDlgItem, &dbpItem);

  auto pForm = GXUIEXT::PropertyList::Form::Create(
    hInstance, hDlgWnd, dbpItem.strName, &dbpItem, NULL);
  hItemWnd = pForm->Get();
  return hItemWnd;
}

GXHWND CreateDialogItem_Toolbar( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGFONTPARAMW &dfp ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  typedef DlgXM::DlgSmartFile::TBButtonArray TBButtonArray;
  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  TBButtonArray aTBButtons;
  file.LoadTBButton(hDlgItem, aTBButtons);

  dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(dbpItem.strStyle, L"TBSTYLE_", DlgXM::CMC_ToolbarStyle);

  GXUI::Toolbar* pToolbar = GXUI::Toolbar::Create(hInstance, hDlgWnd, &dbpItem, &aDefinitions);
  if(pToolbar) {
    for(TBButtonArray::iterator itTBBtn = aTBButtons.begin();
      itTBBtn != aTBButtons.end(); ++itTBBtn)
    {
      pToolbar->AddButton(&*itTBBtn);

      // 释放内部的 clStringW
      if(itTBBtn->idCommand) {
        clStringW& str = *(clStringW*)&itTBBtn->idCommand;
        str.~clStringW();
      }
      else if(itTBBtn->iString) {
        clStringW& str = *(clStringW*)&itTBBtn->iString;
        str.~clStringW();
      }
    }
    pToolbar->CalcButtonSize(0);
  }
  hItemWnd = pToolbar->Get();

  // 设置字体
  if(dfp.IsAvailable()) {
    pToolbar->SetFont(dfp.strFontName, dfp.nFontSize);
  }
  return hItemWnd;
}

GXHWND CreateDialogItem_Slide( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGSLIDERSPRITE sDlgTemplateSldSprite ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(dbpItem.strStyle, L"GXUISLDS_", DlgXM::CMC_SliderStyle);
  GXUI::Slider* pSlider = GXUI::Slider::Create(hInstance, hDlgWnd, &dbpItem, &aDefinitions);
  hItemWnd = pSlider->Get();

  // 载入Sprite配置
  DlgXM::DLGSLIDERSPRITE sDlgSldSprite = sDlgTemplateSldSprite;

  file.LoadSliderSpriteCfg(hDlgItem, L"Sprite", &sDlgSldSprite);
  pSlider->SetSprite(sDlgSldSprite);
  return hItemWnd;
}

GXHWND CreateDialogItem_Edit( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, GXHWND hDlgWnd, GXHINSTANCE hInstance, DlgXM::DLGFONTPARAMW dfp ) 
{
  GXHWND hItemWnd;
  DlgXM::DLGFONTPARAMW  dfpItem;
  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);
  //file.GetBasicParam(hDlgItem, &dbpItem);

  clStringW strEditStyle = hDlgItem.GetKeyAsString(L"EditStyle", L"");
  if(strEditStyle.IsNotEmpty()) {
    dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(strEditStyle, L"GXES_", DlgXM::CMC_EditStyle);
  }

  hItemWnd = gxCreateWindowEx(dbpItem.dwExStyle, GXUICLASSNAME_EDIT, dbpItem.strCaption, 
    dbpItem.dwStyle | GXWS_VISIBLE | GXWS_CHILD, 
    dbpItem.regn.left, dbpItem.regn.top, dbpItem.regn.width, dbpItem.regn.height, hDlgWnd, NULL, hInstance, NULL);

  for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
    it != aDefinitions.end(); ++it)
  {
    if(it->Name == L"DataPool") {
      MOVariable Var;
      clStringA strExpression = (GXLPCWSTR)it->Value;
      MODataPool::FindVariable(NULL, &Var, strExpression);
      gxSendMessage(hItemWnd, GXWM_DATAPOOLOPERATION, DPO_SETVARIABLE, (GXLPARAM)&Var);
    }
  }

  gxSendMessage(hItemWnd, GXWM_SOLVEDEFINITION, 0, (GXLPARAM)&aDefinitions);

  file.GetFontParam(hDlgItem, &dfpItem);
  dfpItem.Inherit(&dfp);
  if(dfpItem.IsAvailable())
  {
    GXLOGFONTW LogFont;
    memset(&LogFont, 0, sizeof(GXLOGFONTW));
    LogFont.lfHeight = dfpItem.nFontSize;
    GXSTRCPYN<GXWCHAR>(LogFont.lfFaceName, dfpItem.strFontName, GXLF_FACESIZE);
    GXHFONT hFont = gxCreateFontIndirectW(&LogFont);
    if(hFont) {
      gxSendMessage(hItemWnd, GXWM_SETFONT, (GXWPARAM)hFont, 0);
    }
  }
  return hItemWnd;
}

GXHWND CreateDialogItem_List( DlgXM::DlgSmartFile &file, Section hDlgItem, DlgXM::DLGBASICPARAMW &dbpItem, const clStringW& strItemTemplate, GXHINSTANCE hInstance, GXHWND hDlgWnd, DlgXM::DLGFONTPARAMW &dfp ) 
{
  GXHWND hItemWnd = NULL;        
  DlgXM::DLGFONTPARAMW  dfpItem;
  GXDefinitionArrayW aDefinitions;
  file.GetBasicParam(hDlgItem, &dbpItem, &aDefinitions);

  dbpItem.dwStyle |= DlgXM::ParseCombinedFlags(dbpItem.strStyle, L"GXLBS_", DlgXM::CMC_StdListBox);

  GXUI::List* pList = strItemTemplate.IsEmpty()
     ? GXUI::List::Create(hInstance, hDlgWnd, &dbpItem, &aDefinitions)
     : GXUI::List::CreateRich(hInstance, hDlgWnd, &dbpItem, &aDefinitions);

  hItemWnd = pList->Get();

  // 设置字体
  if(dfp.IsAvailable()) {
    pList->SetFont(dfp.strFontName, dfp.nFontSize);
  }
  return hItemWnd;
}

//////////////////////////////////////////////////////////////////////////
struct RICHLIST_PARAM
{
  GXHWND  hWnd;
  clStringW strItemTemplate;
};

typedef cllist<RICHLIST_PARAM> RichListParams;
//////////////////////////////////////////////////////////////////////////

GXHWND gxIntCreateDialogFromFileW(
  GXHINSTANCE hInstance,
  GXLPCWSTR   lpFilename,
  GXLPCWSTR   lpDlgName,
  GXHWND      hParent,
  GXDLGPROC   lpDialogFunc,
  GXLPARAM    lParam
  )
{
  ASSERT(hInstance == NULL);
  

  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());

  GXLPCWSTR szDlgFilename = lpFilename[0] == '@' ? &lpFilename[1] : lpFilename;

  DlgXM::DlgSmartFile file;
  Section hDlgSect;
  Section hDlgItem;
  clStringW strDialogSection = clStringW(L"Dialogs/") + lpDlgName;
  clStringW strDialogFile = lpStation->ConvertAbsPathW(szDlgFilename);

  //RichListParams sRichListParams; // 参数队列，用来在InitDialog消息之后初始化Rich List Box控件

  if( ! file.LoadW(strDialogFile))
  {
    TRACEW(L"Error gxIntCreateDialogFromFileW, 无法加载文件(%s).\n", lpFilename);
    return NULL;
  }

  // 注册对话框资源
  gxRegisterClassExW(&WndClassEx_DialogEx);

  hDlgSect = file.Open(strDialogSection);
  if( ! hDlgSect.IsValid())  {
    TRACE("Error gxIntCreateDialogFromFileW, 指定的段不存在.\n");
    return NULL;
  }
  GXUI::DLGPANEL          DlgPanel;
  DlgXM::DLGBASICPARAMW   wbp;
  DlgXM::DLGFONTPARAMW    dfp;
  DlgXM::DLGBTNSPRITE     sDlgTemplateBtnSprite;
  DlgXM::DLGSLIDERSPRITE  sDlgTemplateSldSprite;
  file.GetBasicParam(hDlgSect, &wbp);
  file.GetFontParam(hDlgSect, &dfp);
  
  //GXLPARAM aParam[2] = {
  //  lParam,                  // 用户参数
  //  (GXLPARAM)lpDialogFunc  // 对话框处理函数
  //};

  GXHWND hDlgWnd = NULL;
  GXHMENU hMenu = NULL;
  Section hTemplate;

  // 创建对话框储存结构
  DLGLOG* pDlgLog = new DLGLOG;
  if(pDlgLog == NULL) {
    return hDlgWnd;
  }
  pDlgLog->pDlgProc = lpDialogFunc;
  pDlgLog->strName = wbp.strName;
  pDlgLog->lParam = lParam;

  {
    // 获得对话框扩展样式
    GXDWORD dwDlgStyle = DlgXM::ParseCombinedFlags(wbp.strStyle, L"GXDS_", DlgXM::CMC_DlgStyle);

    // 处理"居中"样式, 此时忽略原来参数中的 left 和 top 参数
    if (TEST_FLAG(dwDlgStyle, GXDS_CENTER))
    {
      //GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
      wbp.regn.left = (lpStation->nWidth - wbp.regn.width) / 2;
      wbp.regn.top = (lpStation->nHeight - wbp.regn.height) / 2;
    }
  }

  if(wbp.strMenu.IsNotEmpty())
  {
    clBuffer bufMenu;
    if(LoadMenuTemplateFromStockW(&file, clStringW("Menus/") + wbp.strMenu, &bufMenu))
    {
      hMenu = gxLoadMenuIndirectW(bufMenu.GetPtr());
      ASSERT(hMenu != NULL);
    }
  }

  // 创建 Dialog 窗口
  hDlgWnd = gxCreateWindowExW( wbp.dwExStyle, WndClassEx_DialogEx.lpszClassName,
    wbp.strCaption, wbp.dwStyle, 
    wbp.regn.left, wbp.regn.top, wbp.regn.width, wbp.regn.height, 
    hParent, hMenu, GXGetInstance(GXINSTTYPE_APP), (GXLPVOID)pDlgLog);
  GXWND_PTR(hDlgWnd)->m_uState |= WIS_ISDIALOGEX;

  if(hDlgWnd == NULL) {
    //goto FUNC_RET;
    return hDlgWnd;
  }

  // 加载对话框模板资源
  hTemplate = file.Open(L"Template");
  if(hTemplate.IsValid())
  {
    Section hButton = hTemplate.Open(L"Button");
    if(hButton.IsValid())
    {
      file.LoadBtnSpriteCfg(hButton, L"Sprite", &sDlgTemplateBtnSprite);
      sDlgTemplateBtnSprite.strResource = lpStation->ConvertAbsPathW(sDlgTemplateBtnSprite.strResource);
      //file.FindClose(hButton);
    }

    Section hSlider = hTemplate.Open(L"Slide");
    if(hSlider.IsValid())
    {
      file.LoadSliderSpriteCfg(hSlider, L"Sprite", &sDlgTemplateSldSprite);
      sDlgTemplateSldSprite.strResource = lpStation->ConvertAbsPathW(sDlgTemplateSldSprite.strResource);
      //file.FindClose(hSlider);
    }

    //file.FindClose(hTemplate);
  }

  hDlgItem = file.Open(strDialogSection).Open(NULL);
  //SP_HANDLE hDlgItem = file.FindFirstSection(hDlgSect, NULL, NULL);

  if(hDlgItem.IsValid())
  {
    do 
    {
      DlgXM::DLGBASICPARAMW dbpItem;
      GXHWND hItemWnd = NULL;

      clStringW strItemName = hDlgItem.SectionName();

      //////////////////////////////////////////////////////////////////////////
      if(strItemName == L"Label") // GXUI Label
      {
        hItemWnd = CreateDialogItem_Label(file, hDlgItem, dbpItem, hInstance, hDlgWnd, dfp);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"Rectangle") // GXUI Rectangle
      {
        hItemWnd = CreateDialogItem_Rectangle(file, hDlgItem, dbpItem, hInstance, hDlgWnd, dfp);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"Sprite") // GXUI Sprite
      {
        hItemWnd = CreateDialogItem_Sprite(file, hDlgItem, dbpItem, hInstance, hDlgWnd);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"Button") // GXUI Button
      {
        hItemWnd = CreateDialogItem_Button(file, hDlgItem, dbpItem, hInstance, hDlgWnd, sDlgTemplateBtnSprite, dfp);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"WETree") // Wine Tree
      {
        hItemWnd = CreateDialogItem_WETree(file, hDlgItem, dbpItem, hDlgWnd, hInstance);
      }
      else if(strItemName == L"WEListView")
      {
        hItemWnd = CreateDialogItem_WEListView(file, hDlgItem, dbpItem, hDlgWnd, hInstance);
      }
      //////////////////////////////////////////////////////////////////////////
      
      else if(strItemName == L"WEEdit") // Wine Edit
      {
        hItemWnd = CreateDialogItem_WEEdit(file, hDlgItem, dbpItem, hDlgWnd, hInstance);
      }
      //////////////////////////////////////////////////////////////////////////
      
      else if(strItemName == L"Edit") // GXUI Edit
      {
        hItemWnd = CreateDialogItem_Edit(file, hDlgItem, dbpItem, hDlgWnd, hInstance, dfp);
      }
      //////////////////////////////////////////////////////////////////////////
      
      else if(strItemName == L"PropSheet") // Property Sheet
      {
        hItemWnd = CreateDialogItem_PropSheet(file, hDlgItem, dbpItem, hInstance, hDlgWnd);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"PropList") // PropList
      {
        hItemWnd = CreateDialogItem_PropList(file, hDlgItem, dbpItem, hInstance, hDlgWnd);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"Toolbar")
      {
        hItemWnd = CreateDialogItem_Toolbar(file, hDlgItem, dbpItem, hInstance, hDlgWnd, dfp);
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"List") // GXUI List Box
      {
        RICHLIST_PARAM sParam;
        sParam.strItemTemplate = hDlgItem.GetKeyAsString(L"ItemTemplate", L"");
        if(sParam.strItemTemplate.IsNotEmpty() && clpathfile::IsFileSpecW(sParam.strItemTemplate)) {
          sParam.strItemTemplate = clStringW(lpFilename) + L":" + sParam.strItemTemplate;
        }

        hItemWnd = CreateDialogItem_List(file, hDlgItem, dbpItem, sParam.strItemTemplate, hInstance, hDlgWnd, dfp);

        if(sParam.strItemTemplate.IsNotEmpty()) {
          gxSendMessageW(hItemWnd, GXLB_SETITEMTEMPLATE, NULL, (GXLPARAM)(GXLPCWSTR)sParam.strItemTemplate);
          //sParam.hWnd = hItemWnd;
          //sRichListParams.push_back(sParam);
        }
      }
      //////////////////////////////////////////////////////////////////////////
      else if(strItemName == L"Slide")
      {
        hItemWnd = CreateDialogItem_Slide(file, hDlgItem, dbpItem, hInstance, hDlgWnd, sDlgTemplateSldSprite);
      }
      //////////////////////////////////////////////////////////////////////////
      // Layout
      else if(strItemName == L"Layout")
      {
        GXHWND hItemWnd = NULL;
        DlgXM::DLGFONTPARAMW  dfpItem;

        const int nScaleMaxIdx = sizeof(DlgPanel.fScale) / sizeof(float);
        DlgPanel.dwStyle = 0;
        for(int i = 0; i < nScaleMaxIdx; i++) {
          DlgPanel.fScale[i] = 0.5f;
        }
        file.LoadLayout(hDlgItem, &DlgPanel);
        continue; // 跳过 AddItem 语句
      }

      pDlgLog->AddItem(dbpItem.strName, hItemWnd);
    } while(hDlgItem.NextSection());

    if(DlgPanel.aPanels.size() != 0)
    {
      ASSERT(pDlgLog->pLayout == NULL);
      pDlgLog->pLayout = new GXUI::Layout(hDlgWnd);
      if(GXFAILED(pDlgLog->pLayout->Initialize(&DlgPanel)))
      {
        delete pDlgLog->pLayout;
        pDlgLog->pLayout = NULL;
      }
    }
  } // if(hDlgItem != NULL)

  //for(auto it = sRichListParams.begin(); it != sRichListParams.end(); ++it) {
  //  gxSendMessageW(it->hWnd, GXLB_SETITEMTEMPLATE, NULL, (GXLPARAM)(GXLPCWSTR)it->strItemTemplate);
  //}

  // #.对于复杂控件，某些控件消息可能先于GXWM_INITDIALOG发生
  // #.在GXWM_INITDIALOG消息发送前，Dialog资源中的控件应该都已经准备好了
  gxSendMessageW(hDlgWnd, GXWM_INITDIALOG, NULL, lParam);


  //for(auto it = sRichListParams.begin(); it != sRichListParams.end(); ++it) {
  //  gxSendMessageW(it->hWnd, GXLB_SETITEMTEMPLATE, NULL, (GXLPARAM)(GXLPCWSTR)it->strItemTemplate);
  //}

//FUNC_RET:
  //if(hDlgItem != NULL) {
  //  file.FindClose(hDlgItem);
  //}
  //if(hDlgSect != NULL) {
  //  file.FindClose(hDlgSect);
  //}
  return hDlgWnd;
}


//////////////////////////////////////////////////////////////////////////
int GXDLLAPI gxDialogBoxParamW(
           GXHINSTANCE  hInstance,      // handle to application instance
           GXLPCWSTR    lpTemplateName, // identifies dialog box template
           GXHWND       hWndParent,     // handle to owner window
           GXDLGPROC    lpDialogFunc,   // pointer to dialog box procedure  
           GXLPARAM     dwInitParam     // initialization value
           )
{
  GXMSG gxmsg;
  GXHWND hDlg = gxCreateDialogParamW((GXHINSTANCE)hInstance, lpTemplateName, NULL, lpDialogFunc, dwInitParam);
  if( ! hDlg) {
    return (int)-1;
  }

  gxShowWindow(hDlg, GXSW_SHOWNORMAL);
//#ifdef _ENABLE_STMT
//  ASSERT(STMT::CheckMainTask() == FALSE);
//#endif // #ifdef _ENABLE_STMT
  if(hWndParent != NULL) {
    gxEnableWindow(hWndParent, FALSE);
  }
  GXINT_PTR ptr = -1;
  while(gxGetMessage(&gxmsg, NULL))
  {
    if((hDlg != gxmsg.hwnd && ( ! gxIsChild(hDlg, gxmsg.hwnd))) && 
      gxmsg.message >= GXWM_LBUTTONDOWN && gxmsg.message <= GXWM_MBUTTONUP)
    {
      gxMessageBeep(0);
      continue;
    }
    gxDispatchMessageW(&gxmsg);

    if((GXWND_PTR(hDlg)->m_uState & WIS_ENDDIALOG) != 0) {
      ptr = gxGetWindowLong(hDlg, GXDWL_MSGRESULT);
      break;
    }
  }
  gxDestroyWindow(hDlg);

  if(hWndParent != NULL) {
    gxEnableWindow(hWndParent, TRUE);
  }

  TRACE("Exit gxDialogBoxParamW\n");
  return (int)ptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLGLOG::DLGLOG() 
 : cbSize   (sizeof(DLGLOG))
 , pDlgProc (NULL)
 , pLayout  (NULL)
{
}

DLGLOG::~DLGLOG()
{
  pLayout->Finalize();
  SAFE_DELETE(pLayout);
}

GXBOOL DLGLOG::AddItem(GXLPCWSTR szName, GXHWND hWnd)
{
    if(szName != NULL && szName[0] != '\0')
    {
        NameToWndDict::iterator it = CtrlItemDict.find(szName);
        if(it == CtrlItemDict.end())
        {
            CtrlItemDict[szName] = hWnd;
            return TRUE;
        }
    }
    return FALSE;
}

GXHWND DLGLOG::GetItem(GXLPCWSTR szName) const
{
    if(szName != NULL && szName[0] != '\0')
    {
        NameToWndDict::const_iterator it = CtrlItemDict.find(szName);
        if(it != CtrlItemDict.end())
        {
            return it->second;
        }
    }
    return NULL;
}
#endif // _DEV_DISABLE_UI_CODE