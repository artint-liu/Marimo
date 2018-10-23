#include "GrapX.h"
//#include "GrapX/GUnknown.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GXUser.h"
#include "GrapX/MOConsoleStaff.h"

struct KEYCONTEXT
{
  GXLPCWSTR szVirKeyName;
  int       nVirKeyValue;
};

static KEYCONTEXT s_aKeyMap[] = {
  {_CLTEXT("VK_OEM_3"),   GXVK_OEM_3},
  {_CLTEXT("VK_ESCAPE"),  GXVK_ESCAPE},
  {_CLTEXT("VK_TAB"),     GXVK_TAB},
  {_CLTEXT("VK_F1"),      GXVK_F1},
  {_CLTEXT("VK_F2"),      GXVK_F2},
  {_CLTEXT("VK_F3"),      GXVK_F3},
  {_CLTEXT("VK_F4"),      GXVK_F4},
  {_CLTEXT("VK_F5"),      GXVK_F5},
  {_CLTEXT("VK_F6"),      GXVK_F6},
  {_CLTEXT("VK_F7"),      GXVK_F7},
  {_CLTEXT("VK_F8"),      GXVK_F8},
  {_CLTEXT("VK_F9"),      GXVK_F9},
  {_CLTEXT("VK_F10"),     GXVK_F10},
  {_CLTEXT("VK_F11"),     GXVK_F11},
  {_CLTEXT("VK_F12"),     GXVK_F12},
  {_CLTEXT("VK_F13"),     GXVK_F13},
  {_CLTEXT("VK_F14"),     GXVK_F14},
  {_CLTEXT("VK_F15"),     GXVK_F15},
  {_CLTEXT("VK_F16"),     GXVK_F16},
  {_CLTEXT("VK_F17"),     GXVK_F17},
  {_CLTEXT("VK_F18"),     GXVK_F18},
  {_CLTEXT("VK_F19"),     GXVK_F19},
  {_CLTEXT("VK_F20"),     GXVK_F20},
  {_CLTEXT("VK_F21"),     GXVK_F21},
  {_CLTEXT("VK_F22"),     GXVK_F22},
  {_CLTEXT("VK_F23"),     GXVK_F23},
  {_CLTEXT("VK_F24"),     GXVK_F24},
  {_CLTEXT("VK_LEFT"),    GXVK_LEFT},
  {_CLTEXT("VK_UP"),      GXVK_UP},
  {_CLTEXT("VK_RIGHT"),   GXVK_RIGHT},
  {_CLTEXT("VK_DOWN"),    GXVK_DOWN},
  {_CLTEXT("VK_DELETE"),  GXVK_DELETE},
  {_CLTEXT("VK_NUMPAD0"), GXVK_NUMPAD0},
  {_CLTEXT("VK_NUMPAD1"), GXVK_NUMPAD1},
  {_CLTEXT("VK_NUMPAD2"), GXVK_NUMPAD2},
  {_CLTEXT("VK_NUMPAD3"), GXVK_NUMPAD3},
  {_CLTEXT("VK_NUMPAD4"), GXVK_NUMPAD4},
  {_CLTEXT("VK_NUMPAD5"), GXVK_NUMPAD5},
  {_CLTEXT("VK_NUMPAD6"), GXVK_NUMPAD6},
  {_CLTEXT("VK_NUMPAD7"), GXVK_NUMPAD7},
  {_CLTEXT("VK_NUMPAD8"), GXVK_NUMPAD8},
  {_CLTEXT("VK_NUMPAD9"), GXVK_NUMPAD9},

  {NULL, 0},
};

class MOFPSKeyboardStaffImpl : public MOKeyboardStaff
{
  static STAFFCAPSDESC s_aCapDesc[];
  struct KEYRESPONSE
  {
    int             nCmdIndex;
    IConsoleStaff*  pStaff;
    clStringW       strCmdLine;
    GXBOOL          bParams;   // strCommand是单一命令的话FALSE，如果是带参数命令则为TRUE
  };
  typedef clhash_map<int, KEYRESPONSE> KeyResponseDict;

  KeyResponseDict m_KeyDict;

private:
  virtual ~MOFPSKeyboardStaffImpl()
  {
    for(KeyResponseDict::iterator it = m_KeyDict.begin();
      it != m_KeyDict.end(); ++it)
    {
      SAFE_RELEASE(it->second.pStaff);
    }
    m_KeyDict.clear();
  }
public:
  GXHRESULT AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  LPCSTAFFCAPSDESC GetCapacity() const
  {
    return s_aCapDesc;
  }

  GXHRESULT Execute(int nCmdIndex, const clStringW* argv, int argc)
  {
    switch(nCmdIndex)
    {
    case 0:
      {
        ASSERT(argv[0] == _CLTEXT("bind"));
        if(argc == 1)
        {
          clStringW strCommandLine;
          for(KeyResponseDict::iterator it = m_KeyDict.begin();
            it != m_KeyDict.end(); ++it) {
              GetBindCommandLine(strCommandLine, it);
              MOLogW(strCommandLine);
          }
        }
        else if(argc == 2)
        {
          clStringW strKey = argv[1];
          strKey.MakeUpper();
          Bind(strKey, "");
        }
        else if(argc >= 2)
        {
          clStringW strKey = argv[1];
          clStringW strCmd = argv[2];
          strKey.MakeUpper();
          Bind(strKey, strCmd);
        }
      }
      break;

    case 1:
      {
        ASSERT(argv[0] == _CLTEXT("unbind"));
        if(argc != 2) {
          MOLogW(_CLTEXT("unbind <virtual-key>\n"));
          break;
        }
        int nVirKey = 0;
        clStringW strKey = argv[1];
        strKey.MakeUpper();
        if(strKey.GetLength() == 1) {
          nVirKey = (int)strKey[0];
        }
        else {
          nVirKey = GetVirtualKeyValue(strKey);
        }

        KeyResponseDict::iterator it = m_KeyDict.find(nVirKey);
        if(it == m_KeyDict.end()) {
          MOLogW(_CLTEXT("\"%s\" doesn't bind anything.\n"), (GXLPCWSTR)argv[1]);
          break;
        }

        SAFE_RELEASE(it->second.pStaff);
        m_KeyDict.erase(it);
        MOLogW(_CLTEXT("Unbind \"%s\".\n"), (GXLPCWSTR)argv[1]);
      }
    }
    return GX_OK;
  }

  GXBOOL OnKeyDown(int nVirKey)
  {
    if(gxGetFocus() != NULL) {
      return GX_OK;
    }
    KeyResponseDict::iterator it = m_KeyDict.find(nVirKey);
    if(it != m_KeyDict.end())
    {
      CallCommand(&it->second);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL OnKeyUp(int nVirKey)
  {
    KeyResponseDict::iterator it = m_KeyDict.find(-nVirKey);
    if(it != m_KeyDict.end())
    {
      CallCommand(&it->second);
      return TRUE;
    }
    return FALSE;
  }

  void CallCommand(const KEYRESPONSE* pKeyResponse)
  {
    // 如果命令带参数就要分解到参数列表中
    if(pKeyResponse->bParams) {
      clStringArrayW aArgs;
      clstd::ParseCommandLine<GXWCHAR>(pKeyResponse->strCmdLine, aArgs);
      pKeyResponse->pStaff->Execute(pKeyResponse->nCmdIndex, &aArgs.front(), (int)aArgs.size());
    }
    else {
      pKeyResponse->pStaff->Execute(pKeyResponse->nCmdIndex, &pKeyResponse->strCmdLine, 1);
    }
  }

  int GetVirtualKeyValue(const clStringW& strVirKey)
  {
    for(int i = 0; s_aKeyMap[i].szVirKeyName != NULL; i++) {
      if(strVirKey == s_aKeyMap[i].szVirKeyName) {
        return s_aKeyMap[i].nVirKeyValue;
      }
    }
    return 0;
  }

  void GetVirtualKeyStringW(clStringW& strVirKey, int nVirKey)
  {
    for(int i = 0; s_aKeyMap[i].szVirKeyName != NULL; i++) {
      if(s_aKeyMap[i].nVirKeyValue == nVirKey) {
        strVirKey = s_aKeyMap[i].szVirKeyName;
        return;
      }
    }
    strVirKey = (GXWCHAR)nVirKey;
  }

  void ReleaseBindStaff(int nVirKey)
  {
    KeyResponseDict::iterator it = m_KeyDict.find(nVirKey);
    if(it != m_KeyDict.end()) {
      SAFE_RELEASE(it->second.pStaff);
    }
  }

  void Bind(const clStringW& strKey, const clStringW& strCmdLine)
  {
    int nVirKey = 0;
    if(strKey.GetLength() == 1) {
      nVirKey = (int)strKey[0];
    }
    else {
      nVirKey = GetVirtualKeyValue(strKey);
    }

    if(nVirKey == 0) {
      return;
    }

    // 一个参数情况，显示Key的绑定信息
    if(strCmdLine.IsEmpty())
    {
      KeyResponseDict::iterator it = m_KeyDict.find(nVirKey);

      // FIXME: 应该能把 VirtualKey 还原为 VK_XXX
      if(it == m_KeyDict.end()) {
        MOLogW(_CLTEXT("\"%c\" doesn't bind anything.\r\n"), nVirKey);
      }
      else {
        MOLogW(_CLTEXT("\"%c\" bind with \"%s\"\r\n"), nVirKey, (GXLPCWSTR)it->second.strCmdLine);
      }
    }
    else
    {
      clStringArrayW aArgs;
      KEYRESPONSE kr;

      kr.strCmdLine = strCmdLine;
      kr.strCmdLine.TrimLeft('\"');
      kr.strCmdLine.TrimRight('\"');

      clstd::ParseCommandLine<GXWCHAR>(kr.strCmdLine, aArgs);
      kr.bParams = aArgs.size() > 1;      

      // KeyDown和KeyUp特殊处理的情况
      if(strCmdLine[0] == '+')
      {
        KEYRESPONSE krUp;
        krUp.strCmdLine = strCmdLine;
        krUp.strCmdLine[0] = '-';

        clStringA strNegArg0 = (GXLPCWSTR)aArgs[0];
        strNegArg0[0] = '-';

        if(MOGetConsoleCmdInfoA(clStringA(aArgs[0]), &kr.nCmdIndex, &kr.pStaff) &&
          MOGetConsoleCmdInfoA(clStringA(strNegArg0), &krUp.nCmdIndex, &krUp.pStaff)) {

            // 如果键位已经被占用就先释放掉Staff对象
            ReleaseBindStaff(nVirKey);
            ReleaseBindStaff(-nVirKey);

            m_KeyDict[-nVirKey] = krUp;
            m_KeyDict[nVirKey] = kr;
        }
      }
      else
      {
        if(MOGetConsoleCmdInfoA(clStringA(aArgs[0]), &kr.nCmdIndex, &kr.pStaff)) {
          ReleaseBindStaff(nVirKey);  // 释放掉Staff对象
          m_KeyDict[nVirKey] = kr;
        }
      }
    }
  } // Bind()

  GXBOOL GetBindCommandLine(clStringW& strCommandLine, KeyResponseDict::const_iterator& it)
  {
    if(it->first < 0) {
      return FALSE;
    }

    clStringW strKey;
    const KEYRESPONSE& sKey = it->second;
    GetVirtualKeyStringW(strKey, it->first);

    // 对空格键特殊处理
    if(strKey[0] == ' ') {
      strKey = "\" \"";
    }

    if(sKey.bParams) {
      strCommandLine.Format(_CLTEXT("bind %s \"%s\"\r\n"), (GXLPCWSTR)strKey, (GXLPCWSTR)it->second.strCmdLine);
    }
    else {
      strCommandLine.Format(_CLTEXT("bind %s %s\r\n"), (GXLPCWSTR)strKey, (GXLPCWSTR)it->second.strCmdLine);
    }
    return TRUE;
  }

  GXBOOL SaveToFileW (GXLPCWSTR szFilename)
  {
    clstd::File file;
    if( ! file.CreateAlways(szFilename)) {
      CLOG_ERROR("%s : Can not create file(%s).\n", __FUNCTION__, (GXLPCSTR)clStringA(szFilename));
      return FALSE;
    }

    clStringW strCommandLine;
    for(KeyResponseDict::iterator it = m_KeyDict.begin();
      it != m_KeyDict.end(); ++it) {
        if(GetBindCommandLine(strCommandLine, it))
        {
          file.Write((GXLPCWSTR)strCommandLine, (u32)(strCommandLine.GetLength() * sizeof(GXWCHAR)));
        }
    }
    return TRUE;
  }
}; // class MOFPSKeyboardStaffImpl : public MOKeyboardStaff

STAFFCAPSDESC MOFPSKeyboardStaffImpl::s_aCapDesc[] = {
  {_CLTEXT("bind"),   _CLTEXT("Bind a key")},
  {_CLTEXT("unbind"), _CLTEXT("Unbind a key")},
  {NULL},
};


GXHRESULT GXDLLAPI MOCreateKeyboardStaff(MOKeyboardStaff** ppStaff)
{
  MOFPSKeyboardStaffImpl* pStaff = new MOFPSKeyboardStaffImpl;
  if( ! InlCheckNewAndIncReference(pStaff)) {
    return GX_FAIL;
  }

  *ppStaff = pStaff;
  return GX_OK;
}