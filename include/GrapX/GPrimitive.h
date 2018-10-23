#ifndef _GRAPHX_PRIMITIVE_H_
#define _GRAPHX_PRIMITIVE_H_

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
  GPrimitive(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType){}
  GXSTDINTERFACE(GXHRESULT  AddRef  ());

  GXSTDINTERFACE(GXLPVOID   GetVerticesBuffer ());
  GXSTDINTERFACE(GXUINT     GetVerticesCount  ());
  GXSTDINTERFACE(GXUINT     GetVertexStride   ());
  GXSTDINTERFACE(GXUINT     GetIndicesCount   ()); // 如果返回0，表示这个不含Index Buffer
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
  GXSTDINTERFACE(GXUINT    GetIndicesCount ());
  GXSTDINTERFACE(GXLPVOID  GetIndicesBuffer());
};
//
// EnableDiscard()
// 确定是否可以在LostDevice时丢弃, 默认为TRUE
//
#endif // _GRAPHX_PRIMITIVE_H_