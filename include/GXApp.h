#ifndef GRAPX_APPLICATION_H_
#define GRAPX_APPLICATION_H_

class IGXPlatform;
class GXGraphics;
class GXImage;
class ILogger;
class GXApp;
struct GXAPP_DESC;
enum GXPlaformIdentity;

enum GXAppDescStyle  // GXAPP_DESC::dwStyle
{
  GXADS_SIZABLE      = 0x00000001,  // 窗口可以改变大小
  GXADS_WAITFORVSYNC = 0x00000002,  // 等待垂直同步
  GXADS_LAZY         = 0x00000004,  // 收到系统重绘时更新
  GXADS_ACTIVE       = 0x0000000C,  // 活动时主动更新，非活动状态根据系统重绘通知更新
  GXADS_REALTIME     = 0x0000001C,  // 尽可能快地不断更新
};


struct GXAPP_DESC
{
  GXSIZE_T            cbSize;     // 数据长度
  GXLPCWSTR           lpName;     // 窗口的标题名
  GXUINT              nWidth;     // 显示区域的宽度, 如果为0,表示让系统自己选择窗口尺寸, 此时窗口一定是可以改变大小的.
  GXUINT              nHeight;    // 显示区域的高度
  GXDWORD             dwStyle;    // 样式, 参见 GXAppDescStyle
  GXPlaformIdentity   idPlatform; // 平台id
  ILogger*            pLogger;    // 日志输出
  GXDEFINITION*       pParameter; // 参数
};

struct GXAPPACTIONINFO
{
  GXHWND    hUIHoverWnd;   // 鼠标滑过的窗口
  GXDWORD   dwAction;      // 标识动作的值
  GXWPARAM  Keys;          // 所有动作的掩码 GXMK_CONTROL, GXMK_LBUTTON, ...
  GXLPARAM  Data;          // 数据
  GXPOINT   ptCursor;      // 位置
};

struct GXAPPKEYINFO
{
  GXHWND    hUIFocusWnd;    // UI焦点窗口
  GXDWORD   dwAction;       // 标识动作的值 CHAR KEYDOWN KEYUP
  GXDWORD   dwKey;          // 所有动作的掩码
};

class GXDLL GXApp
{
protected:
  GXGraphics*   m_pGraphics;
  IGXPlatform*  m_pIPlatform;
public:
  GXHRESULT     Go                 (GXAPP_DESC* pDesc);
  GXGraphics*   GetGraphicsUnsafe  ();

  virtual GXHRESULT OnCreate    ();
  virtual GXHRESULT OnDestroy   ();
  virtual GXHRESULT OnResizing  (GXSIZE* sizeNew);
  virtual GXHRESULT Render      ();
  virtual GXHRESULT ActionBegin (GXAPPACTIONINFO* pActionInfo);
  virtual GXHRESULT ActionMove  (GXAPPACTIONINFO* pActionInfo);
  virtual GXHRESULT ActionEnd   (GXAPPACTIONINFO* pActionInfo);
  virtual GXHRESULT ActionExtra (GXAPPACTIONINFO* pActionInfo); // 扩展信息
  virtual GXHRESULT KeyMessage  (GXAPPKEYINFO* pKeyInfo);
};

#if defined(_WIN32) || defined(_WINDOWS)
GXBOOL GXDLLAPI MOUICreatePlatformSelectedDlg(HINSTANCE hInstance, GXAPP_DESC* pDesc);
#endif // #if defined(_WIN32) || defined(_WINDOWS)
#endif // GRAPX_APPLICATION_H_