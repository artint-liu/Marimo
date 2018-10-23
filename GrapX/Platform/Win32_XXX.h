#ifndef _WIN32_ALL_GRAPHICS_INTERFACE_BASE_H_
#define _WIN32_ALL_GRAPHICS_INTERFACE_BASE_H_

class ILogger;
class IMOPlatform_Win32Base : public IGXPlatform
{
  friend class GXUIMsgThread;

protected:
  HINSTANCE       m_hInstance;
  HWND            m_hWnd;
  clStringArrayW  m_aDropFiles;
  clStringW       m_strRootDir;
  ILogger*        m_pLogger;
  GXUpdateRate    m_eUpdateRate;
  //GXDWORD         m_dwAppDescStyle;

protected:
  // 这个因为头文件包含问题, 要注意不支持析构函数的调用
  IMOPlatform_Win32Base();
  virtual ~IMOPlatform_Win32Base();

  virtual GXHRESULT Finalize      (GXINOUT GXGraphics** ppGraphics);
  virtual GXHRESULT MainLoop      ();
  virtual GXHRESULT QueryFeature  (GXDWORD dwFeatureCode, GXVOID** ppUnknown);

protected:
  static LRESULT  CALLBACK  WndProc     (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  GXLRESULT                 CreateWnd   (LPWSTR lpClassName, WNDPROC pWndProc, GXAPP_DESC* pDesc, GXApp* pApp);
public:
  GXLRESULT                 AppHandle   (GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
};

void ResolverMacroStringToD3DMacro(GXLPCSTR szMacros, GXDefinitionArray& aMacros);

#endif // _WIN32_ALL_GRAPHICS_INTERFACE_BASE_H_