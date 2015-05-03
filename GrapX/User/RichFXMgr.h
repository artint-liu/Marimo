#if 0
//////////////////////////////////////////////////////////////////////////
//
// ��Ч������
// ������ GXStation, ��Ҫ���� Windows �ĸ�����Чʵ��, Ҳ���ûص�������д������ 
// Window ����Ч, ������"rich"����.
//
#ifndef _RICH_FX_MANAGER_H_
#define _RICH_FX_MANAGER_H_

//#include <vector>

// RichFX �ص�ʱ�Ĳ���
struct RFXPROCPARAM
{
  GXDWORD  dwId;
  GXDWORD  dwTime;    // ��ע�Ὺʼ�����ڵ�ʱ��,��0��ʼ, ���Ϊ0xffffffff, 
            // ��˵�����Ѿ��ﵽ���������ڵ����, ����������
  GXDWORD  dwElapse;  // ע��ʱ�����ĳ���ʱ��
  GXDWORD  dwFlags;  // ע��ʱ�õı�־
  GXLPARAM  lParam;    // �û�����
};

// RichFX �Ļص�����
// RichFX������������ע��ʱ������dwElapse, ��������ص���������false����������������Ч
typedef GXBOOL (GXCALLBACK* RichFXProc)(const RFXPROCPARAM* pParam);

// ע���õ�RichFX����
struct RICHFX
{
  GXDWORD      dwElapse;
  GXDWORD      dwFlags;
  RichFXProc    lpCallBack;
  GXLPARAM      lParam;
};

struct RICHFXITEM
{
  RICHFX    RichFX;
  GXDWORD    dwId;
  GXDWORD    dwBegin;
  GXDWORD    dwTime;
};


class RichFXMgr
{
  typedef clvector<RICHFXITEM> RichFXArray;
private:
  GXLPSTATION    m_lpStation;
  RichFXArray    m_aRichFX;
  GXDWORD      m_dwTime;
  GXDWORD      m_dwIdCounter;
public:
  RichFXMgr(GXLPSTATION lpStation);
  GXBOOL  TickRender    (GXDWORD dwDeltaTime);
  GXDWORD  Register    (const RICHFX* pDesc);
};

#endif // _RICH_FX_MANAGER_H_
#endif // #if 0