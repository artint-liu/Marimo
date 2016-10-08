#ifndef _GRAPHX_PRIMITIVE_H_
#define _GRAPHX_PRIMITIVE_H_

// <过时注释>
// 1.使用了 Primitive 这个名字 是为了避免和以后的Stream重名
//    因为 Stream 可能会从 Graphics3D 创建出来

//////////////////////////////////////////////////////////////////////////
// TODO: 追加数据将来单独分离一个Collector类
// 如果追加数据会溢出缓冲,则返回 FALSE
//GXBOOL  Append  (GXLPVOID lpPrimitiveData, GXUINT uElementCount);
//GXBOOL  Append  (GXLPVOID lpPrimitiveData, GXUINT uElementCount, WORD* pIndices, GXUINT uIndexCount);
//GXBOOL  Reset  ();
// </过时注释>

class GVertexDeclaration;

class GPrimitive : public GResource
{
public:
  enum ResEnum
  {
    ResourceIndices,
    ResourceVertices,
    ResourceAll,
  };

public:
  //enum Type
  //{
  //  VertexOnly = 0,
  //  Indexed = 1,
  //};
  GPrimitive(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType){}
  GXSTDINTERFACE(GXHRESULT  AddRef  ());
  //GXSTDINTERFACE(Type       GetType ());

  GXSTDINTERFACE(GXLPVOID   GetVerticesBuffer ());
  GXSTDINTERFACE(GXUINT     GetVerticesCount  ());
  GXSTDINTERFACE(GXUINT     GetVertexStride   ());
  GXSTDINTERFACE(GXBOOL     UpdateResouce     (ResEnum eRes));  // 将内存数据更新到设备上
  GXSTDINTERFACE(GXHRESULT  GetVertexDeclaration(GVertexDeclaration** ppDeclaration));
  GXSTDINTERFACE(GXGraphics*GetGraphicsUnsafe ());
  GXSTDINTERFACE(GXINT      GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc = NULL));
};

//
// Primitive - Vertex 
//
class GPrimitiveV : public GPrimitive
{
public:
  GPrimitiveV() : GPrimitive(0, RESTYPE_PRIMITIVE){}

  GXSTDINTERFACE(GXHRESULT AddRef        ());
  GXSTDINTERFACE(GXHRESULT Release       ());

  GXSTDINTERFACE(GXBOOL    EnableDiscard (GXBOOL bDiscard));
  GXSTDINTERFACE(GXBOOL    IsDiscardable ());
  GXSTDINTERFACE(GXLPVOID  Lock          (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
    GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE)));
  GXSTDINTERFACE(GXBOOL    Unlock        ());
};

//
// Primitive - Vertex & Index
//
class GPrimitiveVI : public GPrimitive
{
public:
  GPrimitiveVI() : GPrimitive(0, RESTYPE_INDEXED_PRIMITIVE){}

  GXSTDINTERFACE(GXHRESULT AddRef        ());
  GXSTDINTERFACE(GXHRESULT Release       ());

  GXSTDINTERFACE(GXBOOL    EnableDiscard (GXBOOL bDiscard));
  GXSTDINTERFACE(GXBOOL    IsDiscardable ());
  GXSTDINTERFACE(GXBOOL    Lock          (GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, 
    GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
    GXLPVOID* ppVertexData, GXWORD** ppIndexData,
    GXDWORD dwFlags = (GXLOCK_DISCARD | GXLOCK_NOOVERWRITE)));
  GXSTDINTERFACE(GXBOOL    Unlock        ());
  GXSTDINTERFACE(GXUINT    GetIndexCount ());

  GXSTDINTERFACE(GXLPVOID  GetIndicesBuffer());
};
//
// EnableDiscard()
// 确定是否可以在LostDevice时丢弃, 默认为TRUE
//
#endif // _GRAPHX_PRIMITIVE_H_