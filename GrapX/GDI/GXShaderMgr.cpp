#if defined(_WIN32) || defined(_WINDOWS)
#include <GrapX.h>
#include "User/GrapX.Hxx"

#include <clPathFile.h>
//#include <clString.h>

#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"

#include <GDI/GXShaderMgr.h>

#if 0
#include <xhash>

ShaderDirMonitor::ShaderDirMonitor(GXShaderMgr* pShaderMgr)
  : m_pShaderMgr    (pShaderMgr)
  , m_hDir      (NULL)
  , m_hThread      (NULL)
  , m_dwIdTrackThread  (NULL)
{
  memset(&m_Overlapped, 0, sizeof(m_Overlapped));
}

GXHRESULT ShaderDirMonitor::Initialize(GXLPCWSTR lpDir)
{
  m_strSourceDir = lpDir;
  m_Overlapped.hEvent = CreateEvent(NULL, FALSE, NULL, NULL);

  m_hDir = CreateFile(
    m_strSourceDir,                    // pointer to the file name
    FILE_LIST_DIRECTORY,                // access (read-write) mode
    FILE_SHARE_READ|FILE_SHARE_DELETE,          // share mode
    NULL,                        // security descriptor
    OPEN_EXISTING,                    // how to create
    FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,  // file attributes
    NULL                        // file with attributes to copy
    );

  m_Overlapped.hEvent = CreateEvent(NULL, FALSE, NULL, NULL);
  m_hThread = CreateThread(NULL, 0, TrackDirChangeThread, this, NULL, &m_dwIdTrackThread);
  return GX_OK;
}

GXHRESULT ShaderDirMonitor::Destroy()
{
  CloseHandle(m_hDir);
  SetEvent(m_Overlapped.hEvent);
  WaitForSingleObject(m_hThread, INFINITE);
  CloseHandle(m_Overlapped.hEvent);
  CloseHandle(m_hThread);
  return GX_OK;
}

GXDWORD GXDLLAPI ShaderDirMonitor::TrackDirChangeThread(GXLPVOID lParam)
{
  ShaderDirMonitor* pThis = (ShaderDirMonitor*)lParam;
  GXBYTE buffer[4 * 1024];
  GXDWORD dwBytes;

  while(ReadDirectoryChangesW(pThis->m_hDir, buffer, sizeof(buffer), TRUE, 
    FILE_NOTIFY_CHANGE_LAST_WRITE, &dwBytes, &pThis->m_Overlapped, NULL) != 0)
  {
    GXDWORD dwResult = WaitForSingleObject(pThis->m_Overlapped.hEvent, INFINITE);
    if(dwResult == WAIT_OBJECT_0)
    {
      FILE_NOTIFY_INFORMATION* pFni = (FILE_NOTIFY_INFORMATION*)buffer;

      FILE_ACTION_ADDED;
      //__asm nop
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////


GXShaderMgr::GXShaderMgr(GXGraphics* pGraphics)
  : m_pGraphics    (pGraphics)
{
}

GXHRESULT GXShaderMgr::Initialize()
{
  InitializeCriticalSection(&m_ClsAccess);
  return GX_OK;
}

GXHRESULT GXShaderMgr::Destroy()
{
  DeleteCriticalSection(&m_ClsAccess);
  return GX_OK;
}

GXHRESULT GXShaderMgr::LoadVertexShader(GXLPCWSTR lpVertexShaderSource)
{
  return GX_OK;
}

GXHRESULT GXShaderMgr::LoadPixelShader(GXLPCWSTR lpPixelShaderSource)
{
  return GX_OK;
}

GXHRESULT GXShaderMgr::LoadFileToBuffer(GXLPCWSTR lpAnyFile, clBuffer** ppBuffer)
{
  GXDWORD dwHash = (GXDWORD)stdext::hash_value(lpAnyFile);
  GXWCHAR buffer[16];
  clString strDir;
  clString strFilename;
  clString strMainFilename;
  clString strExtension;
  clString strCacheFile;

  clFile  File;

  SplitPath(lpAnyFile, &strDir, &strFilename);
  clsize p = clpathfile::FindExtensionW(strFilename);
  strMainFilename = strFilename.SubString(0, p + 1);
  strExtension = strFilename.SubString(p + 1, strFilename.GetLength() - p);
  //GetFileExtension(strFilename, &strMainFilename, &strExtension);

  wsprintf(buffer, L"%08X%s", dwHash, strExtension);
  strCacheFile = m_strCacheDir + buffer;

  switch(m_eLoad)
  {
  case SL_CACHEONLY:
    {
      if(File.OpenExistingW(strCacheFile) == FALSE)
        return GX_FAIL;
      File.MapToBuffer(ppBuffer);
    }
    break;
  case SL_CACHEFIRST:
    break;
  case SL_SOURCEONLY:
    break;
  case SL_SOURCEFIRST:
    break;
  }
  return GX_OK;
}

GXHRESULT GXShaderMgr::GetAttribute(SHADERMGRATTR* pShaderMgrAttr)
{
  if(pShaderMgrAttr == NULL)
    return GX_FAIL;

  if(pShaderMgrAttr->dwMask & SMAM_SHADERLOAD)
    pShaderMgrAttr->eLoad = m_eLoad;

  if(pShaderMgrAttr->dwMask & SMAM_CACHEDIR)
    pShaderMgrAttr->strCacheDir = m_strCacheDir;

  return GX_OK;
}

GXHRESULT GXShaderMgr::SetAttribute(SHADERMGRATTR* pShaderMgrAttr)
{
  if(pShaderMgrAttr == NULL)
    return GX_FAIL;

  if(pShaderMgrAttr->dwMask & SMAM_SHADERLOAD)
    m_eLoad = pShaderMgrAttr->eLoad;

  if(pShaderMgrAttr->dwMask & SMAM_CACHEDIR)
    m_strCacheDir = pShaderMgrAttr->strCacheDir;

  return GX_OK;
}
#endif // defined(_WIN32) || defined(_WINDOWS)
#endif // #if 0
namespace Marimo
{
  ShaderConstName::ShaderConstName(GXGraphics* pGraphics)
    : m_pGraphics   (pGraphics)
    , m_nTotalCount (0)
    , m_cbCanvasUniform (sizeof(float4x4))  // 这个地方是要避免AllocHandle返回0
  {
  }

  ShaderConstName::~ShaderConstName()
  {
  }

  GXINT_PTR ShaderConstName::AddName(GShader* pShader, GXLPCSTR szName, GXLONG cbSize)
  {
    NameDict::iterator it = m_NameDict.find(szName);
    if(it != m_NameDict.end()) {

      if(cbSize > it->second.cbMaxSize)
      {
        if(it->second.cbOffset == -1) {
          it->second.cbMaxSize = cbSize;
        }
        else {  // 已经是Global Constant，需要扩展后面的Buffer
                // 如果else增加了扩展，去掉cbMaxSize关于不会改变的说明
        }
      }
      ++m_nTotalCount;
      ++it->second.nCount;
      return it->second.cbOffset;
    }

    NAMEDESC sDesc;
    sDesc.nCount    = 1;
    sDesc.cbOffset  = -1;
    sDesc.cbMaxSize = cbSize;
    m_NameDict[szName] = sDesc;
    ++m_nTotalCount;

    // 针对Canvas-Uniform只增不减的问题，这里设置一个断言，如果达到64KB，
    // 作为一个警告会触发这个断言，后续应该考虑可以在合适的时机清理一次
    // Canvas-Uniform缓冲。参考Cleanup()中的注意事项
    ASSERT(m_cbCanvasUniform < 65536);
    return -1;
  }

  int ShaderConstName::RemoveName(GShader* pShader, GXLPCSTR szName)
  {
    int result = 0;
    NameDict::iterator it = m_NameDict.find(szName);
    if(it != m_NameDict.end()) {
      --m_nTotalCount;
      ASSERT(m_nTotalCount >= 0);
      result = --it->second.nCount;

      // 如果引用归0并且是私有变量，就清除掉
      if(result == 0 && it->second.cbOffset < 0) {
        m_NameDict.erase(it);
      }
    }
    return result;
  }

  void ShaderConstName::Cleanup()
  {
    // 执行清理的条件
    // 1. shader全都释放掉
    // 2. 没想好怎么确定 canvas 3D 不再使用 global const 了
    // 3. Effect不属于Material，但是在这个类中注册了名字，但是内置Effect是由Graphics创建的，这可能导致用于达不到（1）的条件
    ASSERT(m_nTotalCount == 0);
#ifdef _DEBUG
    for(NameDict::iterator it = m_NameDict.begin();
      it != m_NameDict.end(); ++it) {
        ASSERT(it->second.nCount == 0);
    }
#endif // #ifdef _DEBUG
    m_NameDict.clear();
    m_cbCanvasUniform = 0;
  }

  GXINT_PTR ShaderConstName::AllocHandle(GXLPCSTR szName, GXUniformType eExpect)
  {
    NameDict::iterator it = m_NameDict.find(szName);
    if(it != m_NameDict.end()) {
      if(it->second.cbOffset >= 0) {  // 直接返回
        return (GXINT_PTR)it->second.cbOffset;
      }
      else { // 在constant buffer上分配一个空间
        NAMEDESC& sDesc = it->second;
        sDesc.cbOffset = m_cbCanvasUniform;
        m_cbCanvasUniform += ALIGN_16(sDesc.cbMaxSize);
        
        // 通知所有shader，如果使用这个名字则把constant从私有改为全局
        GRESCRIPTDESC sScriptDesc = {NULL};
        sScriptDesc.dwCmdCode = RC_MarkCanvsUniform;
        sScriptDesc.lParam    = (GXLPARAM)szName;
        sScriptDesc.wParam    = sDesc.cbOffset;
        m_pGraphics->BroadcastCategoryCommand(RCC_Shader, &sScriptDesc);

        return (GXINT_PTR)sDesc.cbOffset;
      }
    }

    if(eExpect == GXUB_UNDEFINED) {
      return -1;
    }

    NAMEDESC sDesc;
    sDesc.nCount    = 0;
    sDesc.cbOffset  = m_cbCanvasUniform;

    switch(eExpect)
    {
    case GXUB_FLOAT:
      sDesc.cbMaxSize = sizeof(float);
      break;
    case GXUB_FLOAT2:
      sDesc.cbMaxSize = sizeof(float2);
      break;
    case GXUB_FLOAT3:
      sDesc.cbMaxSize = sizeof(float3);
      break;
    case GXUB_FLOAT4:
      sDesc.cbMaxSize = sizeof(float4);
      break;
    case GXUB_MATRIX4:
      sDesc.cbMaxSize = sizeof(float4x4);
      break;
    case GXUB_SAMPLER2D:
    case GXUB_SAMPLER3D:
    default:
      return -1;
    }

    m_NameDict[szName] = sDesc;

    m_cbCanvasUniform += ALIGN_16(sDesc.cbMaxSize);
    return (GXINT_PTR)sDesc.cbOffset;
  }

} // namespace Marimo
