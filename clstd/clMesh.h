#ifndef _CLSTD_MESH_H_
#define _CLSTD_MESH_H_

// 这个是clstd下的设备无关Mesh处理类
// Mesh数据完全在内存中，不与显卡等硬件设备关联，处理操作也依赖于cpu进行

namespace clstd
{
  class Mesh
  {
  protected:
  public:
    Mesh();
    virtual ~Mesh();

  public:
  };
} // namespace clstd

#endif // _CLSTD_MESH_H_