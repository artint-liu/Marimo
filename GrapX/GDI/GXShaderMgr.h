#ifndef _GRAPX_SHADER_MANAGER_H_
#define _GRAPX_SHADER_MANAGER_H_

#if defined(_WIN32) || defined(_WINDOWS)
//////////////////////////////////////////////////////////////////////////
//
// Shader������
// ������ GXGraphics ����, ���ڹ������ CShader ��.
//
#if 0
#include <hash_map>

class GXGraphics;
class GXShaderMgr;
class clBuffer;

// Shaderװ����Ϊ
enum SHADERLOAD
{
  SL_CACHEONLY,     // ֻװ��Cache�ļ�, �����������ʧ��, ������Source����
  SL_SOURCEONLY,    // ���Source����, ���ȸ���Cache������, ���Source�������򷵻�ʧ��, ������ʱ����Source����
  SL_CACHEFIRST,    // ���Cache����, �����Source(�����һ����)ֱ������, �������Source������, ������ʱ����Source����
  SL_SOURCEFIRST,   // ���Source��Cache, ��������ڲ���һ��, ������Cache, �����һ�������Cache, ����ֻ������ڵ��ļ�, ������ʱ����Source����
};

// Shader Manager Attribute ��������
enum SHADERMGRATTRMASK
{
  SMAM_SHADERLOAD = 0x00000001,
  SMAM_CACHEDIR = 0x00000002,
};

struct SHADERMGRATTR
{
  GXDWORD    dwMask;
  SHADERLOAD  eLoad;
  clString  strCacheDir;
};

class ShaderDirMonitor
{
private:
  GXShaderMgr*  m_pShaderMgr;
  OVERLAPPED    m_Overlapped;
  HANDLE        m_hDir;
  HANDLE        m_hThread;
  GXDWORD       m_dwIdTrackThread;
  clString      m_strSourceDir;

  static GXDWORD GXDLLAPI TrackDirChangeThread(GXLPVOID lParam);
public:
  ShaderDirMonitor  (GXShaderMgr* pShaderMgr);
  GXHRESULT Initialize  (GXLPCWSTR lpDir);
  GXHRESULT Destroy    ();
};

class GXShaderMgr
{
  typedef clhash_map<GXDWORD, ShaderDirMonitor>  ShaderDirMonitorMap;
private:
  GXGraphics*      m_pGraphics;
  SHADERLOAD      m_eLoad;
  clString      m_strCacheDir;
  CRITICAL_SECTION  m_ClsAccess;
  ShaderDirMonitorMap  m_mapShaderDir;

  GXHRESULT LoadFileToBuffer  (GXLPCWSTR lpAnyFile, clBuffer** ppBuffer);

public:
  GXShaderMgr          (GXGraphics* pGraphics);

  GXHRESULT Initialize      ();
  GXHRESULT Destroy        ();

  GXHRESULT LoadVertexShader  (GXLPCWSTR lpVertexShaderSource);
  GXHRESULT LoadPixelShader    (GXLPCWSTR lpPixelShaderSource);

  GXHRESULT GetAttribute    (SHADERMGRATTR* pShaderMgrAttr);
  GXHRESULT SetAttribute    (SHADERMGRATTR* pShaderMgrAttr);
};

#endif // defined(_WIN32) || defined(_WINDOWS)
#endif // #if 0

namespace Marimo
{
  class ShaderConstName
  {
  private:
    struct NAMEDESC
    {
      GXLONG nCount;
      GXLONG cbOffset;  // ShaderBuf�е�ƫ�ƣ����Ϊ-1˵�����constant��shader˽�е�
      GXLONG cbMaxSize; // constant�Ĵ�С��cbOffsetΪ-1֮ǰ�������Ѽ���shader����ʹ�ø����ֵ�constant���ֵ
                        // ��Ϊ-1ʱ˵���Ѿ�����Ϊȫ��constant�����ֵ���ٸı䡣
    };

    typedef clhash_map<clStringA, NAMEDESC> NameDict;

  private:
    GXGraphics* m_pGraphics;
    GXLONG      m_nTotalCount;  // ���е�ConstName����
    NameDict    m_NameDict;
    GXLONG      m_cbCanvasUniform;  // Canvas-Uniform �Ĵ�С

  protected:
    void Cleanup();

  public:
    ShaderConstName(GXGraphics* pGraphics);
    virtual ~ShaderConstName();

  public:
    GXINT_PTR AllocHandle       (GXLPCSTR szName, GXUniformType eExpect); // GXUniformType�� ���ָ�������ִ��ڣ������eExpect����������������������ʹ�����GXUB_UNDEFINED�򷵻�ʧ��
    GXINT_PTR AddName           (GShader* pShader, GXLPCSTR szName, GXLONG cbSize);
    int       RemoveName        (GShader* pShader, GXLPCSTR szName);

    inline GXSIZE_T GetSize() const;
  };

  // Canvas3D ���� AllocHandle ֮����Ҫ���������������������
  // CanvasUniform Buffer �Ƿ�̫С������Ǿ���Ҫ�㲥����Canvas3D
  // ������Buffer��С
  GXSIZE_T ShaderConstName::GetSize() const
  {
    return (GXSIZE_T)m_cbCanvasUniform;
  }


} // namespace Marimo

#endif // _GRAPX_SHADER_MANAGER_H_
