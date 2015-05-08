#ifndef _GRAPVR_SEQUENCE_H_
#define _GRAPVR_SEQUENCE_H_

// Background   1000
// Geometry     2000
// Alpha test   2500
// Transparent  3000
// Overlay      4000
// Max          4095

// Render queue - 参考Unity3D文档的定义
enum RenderQueue {
  RenderQueue_Background  = 1000,
  RenderQueue_Geometry    = 2000,
  RenderQueue_AlphaTest   = 2500,
  RenderQueue_Transparent = 3000,
  RenderQueue_Overlay     = 4000,
  RenderQueue_Max         = 4095,
};

class GXDLL GVSequence : public GUnknown
{
public:
  typedef clvector<GVRENDERDESC> RenderDescArray;
  typedef clmap<GXUINT, RenderDescArray> RenderQueue;
protected:
  const static int c_nNumRQSlot = 4096;

  //RenderDescArray   m_aRenderDesc;
  //RenderDescArray   m_aRenderDesc2; // 这个需要排序
  RenderQueue       m_mapRenderQueue;

  // 这个是临时的，目前仅分了5级渲染顺序，还没想好怎么设计渲染序列这个问题， 实际上应该是4096级，并且材质排序的
  const static int c_nNeedRefactorRenderDescCount = 6;
  RenderDescArray   m_aRenderDesc[6];
  // </Comment>

//void*             m_aRenderQueueSlot[c_nNumRQSlot]; // void* 是临时的,没想好是啥类型

protected:
  GVSequence();
  virtual ~GVSequence();
public:
  void  Clear ();
  int   Add   (GVRENDERDESC* pDesc);

  int   GetArrayCount ();

  const RenderDescArray&
        GetArray      (int nIndex);
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
  virtual GXHRESULT Release ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  static  GXHRESULT Create  (GVSequence** ppSequence);
};

#endif // _GRAPVR_SEQUENCE_H_