#ifndef _GRAPHX_PRIMITIVE_H_
#define _GRAPHX_PRIMITIVE_H_

class GVertexDeclaration;

class GPrimitive : public GResource
{
//public:
//  enum ResEnum // TODO: 去掉
//  {
//    ResourceIndices,
//    ResourceVertices,
//    ResourceAll,
//  };

public:
  GPrimitive(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType){}
  GXSTDINTERFACE(GXHRESULT  AddRef  ());

  GXSTDINTERFACE(GXGraphics*GetGraphicsUnsafe ());

  GXSTDINTERFACE(GXUINT     GetVertexCount    ());
  GXSTDINTERFACE(GXUINT     GetVertexStride   ());
  GXSTDINTERFACE(GXLPVOID   MapVertexBuffer   (GXResMap eMap));
  GXSTDINTERFACE(GXBOOL     UnmapVertexBuffer (GXLPVOID lpMappedBuffer));

  // 如果不包含Index buffer, 则下列索引相关函数返回0或者Null
  GXSTDINTERFACE(GXUINT     GetIndexCount     ());
  GXSTDINTERFACE(GXUINT     GetIndexStride    ());
  GXSTDINTERFACE(GXLPVOID   MapIndexBuffer    (GXResMap eMap));
  GXSTDINTERFACE(GXBOOL     UnmapIndexBuffer  (GXLPVOID lpMappedBuffer));

  //GXSTDINTERFACE(GXBOOL     UpdateResouce     (ResEnum eRes));  // 将内存数据更新到设备上
  GXSTDINTERFACE(GXHRESULT  GetVertexDeclaration(GVertexDeclaration** ppDeclaration));
  GXSTDINTERFACE(GXINT      GetElementOffset    (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc = NULL));
};

namespace GrapX
{
  namespace PrimitiveUtility
  {
    // 锁定顶点的辅助对象，超出作用域自动解锁
    class GXDLL MapVertices
    {
    protected:
      GPrimitive* m_pPrimitive;
      GXLPVOID    m_ptr;

    public:
      MapVertices(GPrimitive* pPrimitive, GXResMap eMap);
      ~MapVertices();
      GXLPVOID GetPtr() const;
    };


    // 锁定顶点索引，超出作用域自动解锁
    class GXDLL MapIndices
    {
    protected:
      GPrimitive* m_pPrimitive;
      GXLPVOID    m_ptr;

    public:
      MapIndices(GPrimitive* pPrimitive, GXResMap eMap);
      ~MapIndices();
      GXLPVOID GetPtr() const;
    };

  } // namespace PrimitiveUtility
} // namespace GrapX

//
// Primitive - Vertex 
//
//class GPrimitiveV : public GPrimitive
//{
//public:
//  GPrimitiveV() : GPrimitive(0, RESTYPE_PRIMITIVE){}
//
//  GXSTDINTERFACE(GXHRESULT AddRef        ());
//  GXSTDINTERFACE(GXHRESULT Release       ());
//
//  //GXSTDINTERFACE(GXBOOL    EnableDiscard (GXBOOL bDiscard));
//  //GXSTDINTERFACE(GXBOOL    IsDiscardable ());
//  //GXSTDINTERFACE(GXLPVOID  Lock          (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
//  //  GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE)));
//  //GXSTDINTERFACE(GXBOOL    Unlock        ());
//};

//
// Primitive - Vertex & Index
//
//class GPrimitiveVI : public GPrimitive
//{
//public:
//  GPrimitiveVI() : GPrimitive(0, RESTYPE_INDEXED_PRIMITIVE){}
//
//  GXSTDINTERFACE(GXHRESULT AddRef        ());
//  GXSTDINTERFACE(GXHRESULT Release       ());
//
//  //GXSTDINTERFACE(GXBOOL    EnableDiscard (GXBOOL bDiscard));
//  //GXSTDINTERFACE(GXBOOL    IsDiscardable ());
//  //GXSTDINTERFACE(GXBOOL    Lock          (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
//  //  GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
//  //  GXLPVOID* ppVertexData, GXWORD** ppIndexData,
//  //  GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE)));
//  //GXSTDINTERFACE(GXBOOL    Unlock        ());
//  //GXSTDINTERFACE(GXUINT    GetIndicesCount ());
//  //GXSTDINTERFACE(GXLPVOID  GetIndicesBuffer());
//};
//
// EnableDiscard()
// 确定是否可以在LostDevice时丢弃, 默认为TRUE
//
#endif // _GRAPHX_PRIMITIVE_H_