#ifndef _GRAPX_SHADER_MANAGER_H_
#define _GRAPX_SHADER_MANAGER_H_

#if defined(_WIN32) || defined(_WINDOWS)
//////////////////////////////////////////////////////////////////////////
//
// Shader管理器
// 隶属于 GXGraphics 对象, 用于管理缓存的 CShader 类.
//
#if 0
#include <hash_map>

class GXGraphics;
class GXShaderMgr;
class clBuffer;

// Shader装载行为
enum SHADERLOAD
{
  SL_CACHEONLY,     // 只装载Cache文件, 如果不存在则失败, 不监视Source更新
  SL_SOURCEONLY,    // 如果Source存在, 则先更新Cache再载入, 如果Source不存在则返回失败, 在运行时监视Source更新
  SL_CACHEFIRST,    // 如果Cache存在, 则忽略Source(不检查一致性)直接载入, 否则编译Source后载入, 在运行时监视Source更新
  SL_SOURCEFIRST,   // 检查Source和Cache, 如果都存在并且一致, 则载入Cache, 如果不一致则更新Cache, 否则只载入存在的文件, 在运行时监视Source更新
};

// Shader Manager Attribute 设置掩码
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
      GXLONG cbOffset;  // ShaderBuf中的偏移，如果为-1说明这个constant是shader私有的
      GXLONG cbMaxSize; // constant的大小，cbOffset为-1之前是所有已加载shader并且使用该名字的constant最大值
                        // 不为-1时说明已经设置为全局constant，这个值不再改变。
    };

    typedef clhash_map<clStringA, NAMEDESC> NameDict;

  private:
    GXGraphics* m_pGraphics;
    GXLONG      m_nTotalCount;  // 所有的ConstName数量
    NameDict    m_NameDict;
    GXLONG      m_cbCanvasUniform;  // Canvas-Uniform 的大小

  protected:
    void Cleanup();

  public:
    ShaderConstName(GXGraphics* pGraphics);
    virtual ~ShaderConstName();

  public:
    GXINT_PTR AllocHandle       (GXLPCSTR szName, GXUniformType eExpect); // GXUniformType， 如果指定的名字存在，则忽略eExpect。如果不存在则按照期望类型创建，GXUB_UNDEFINED则返回失败
    GXINT_PTR AddName           (GShader* pShader, GXLPCSTR szName, GXLONG cbSize);
    int       RemoveName        (GShader* pShader, GXLPCSTR szName);

    inline GXSIZE_T GetSize() const;
  };

  // Canvas3D 调用 AllocHandle 之后需要调用这个方法来检查自身的
  // CanvasUniform Buffer 是否太小，如果是就需要广播其他Canvas3D
  // 来调整Buffer大小
  GXSIZE_T ShaderConstName::GetSize() const
  {
    return (GXSIZE_T)m_cbCanvasUniform;
  }


} // namespace Marimo

#endif // _GRAPX_SHADER_MANAGER_H_
