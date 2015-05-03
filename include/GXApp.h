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
  GXADS_SIZABLE      = 0x00000001,  // ���ڿ��Ըı��С
  GXADS_WAITFORVSYNC = 0x00000002,  // �ȴ���ֱͬ��
  GXADS_LAZY         = 0x00000004,  // �յ�ϵͳ�ػ�ʱ����
  GXADS_ACTIVE       = 0x0000000C,  // �ʱ�������£��ǻ״̬����ϵͳ�ػ�֪ͨ����
  GXADS_REALTIME     = 0x0000001C,  // �����ܿ�ز��ϸ���
};


struct GXAPP_DESC
{
  GXSIZE_T            cbSize;     // ���ݳ���
  GXLPCWSTR           lpName;     // ���ڵı�����
  GXUINT              nWidth;     // ��ʾ����Ŀ��, ���Ϊ0,��ʾ��ϵͳ�Լ�ѡ�񴰿ڳߴ�, ��ʱ����һ���ǿ��Ըı��С��.
  GXUINT              nHeight;    // ��ʾ����ĸ߶�
  GXDWORD             dwStyle;    // ��ʽ, �μ� GXAppDescStyle
  GXPlaformIdentity   idPlatform; // ƽ̨id
  ILogger*            pLogger;    // ��־���
  GXDEFINITION*       pParameter; // ����
};

struct GXAPPACTIONINFO
{
  GXHWND    hUIHoverWnd;   // ��껬���Ĵ���
  GXDWORD   dwAction;      // ��ʶ������ֵ
  GXWPARAM  Keys;          // ���ж��������� GXMK_CONTROL, GXMK_LBUTTON, ...
  GXLPARAM  Data;          // ����
  GXPOINT   ptCursor;      // λ��
};

struct GXAPPKEYINFO
{
  GXHWND    hUIFocusWnd;    // UI���㴰��
  GXDWORD   dwAction;       // ��ʶ������ֵ CHAR KEYDOWN KEYUP
  GXDWORD   dwKey;          // ���ж���������
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
  virtual GXHRESULT ActionExtra (GXAPPACTIONINFO* pActionInfo); // ��չ��Ϣ
  virtual GXHRESULT KeyMessage  (GXAPPKEYINFO* pKeyInfo);
};

#if defined(_WIN32) || defined(_WINDOWS)
GXBOOL GXDLLAPI MOUICreatePlatformSelectedDlg(HINSTANCE hInstance, GXAPP_DESC* pDesc);
#endif // #if defined(_WIN32) || defined(_WINDOWS)
#endif // GRAPX_APPLICATION_H_