#ifndef _MARIMO_ENGINE_HEIGHTFIELD_TERRAIN_H_
#define _MARIMO_ENGINE_HEIGHTFIELD_TERRAIN_H_

namespace clstd
{
  class ImageFilterF;
} // namespace clstd

class HeightfieldTerrain;

class GAMEENGINE_DLL HeightfieldTerrainBlock : public GVMesh
{
public:
  struct BLOCKDESC
  {
    GXSIZE_T      cbSize;
    float3        vPos;           // 初始位置，这个位置是网格的一个角落，不是中心位置
    int           nSegmentX;      // X轴分段数
    int           nSegmentY;      // Y轴分段数
    float         fStride;        // 每个段的步幅
    const float*  pHeightfield;   // 高度场数据
    int           cbHFPitch;      // pHeightfieldData每行的长度(bytes)
    const float3* pNormals;
    int           cbNormalsPitch;
    GXPOINT       ptID;           // 位置ID，这个可能是行列的整数坐标，也有可能是行列索引，具体由所有者决定
    GXDWORD       dwFlags;        // 参考CREATIONFLAGS
    HeightfieldTerrain* pTerrain;
    // #.pHeightfield 高度场数据
    // 1.标准模式下这个数据长度应该是(nSegmentX + 1) * (nSegmentY + 1)
    // 2.CreationFlag_EditableMesh模式下长度是(nSegmentX + 3) * (nSegmentY + 3)
    //   边缘只会用与计算法线，不会用于构建模型。
  };


protected:
  GXPOINT m_ptID;
  int m_nVertCountX;
  int m_nVertCountY;
  float m_fStride;  // 每个段的步幅

protected:
  HeightfieldTerrainBlock(GXGraphics* pGraphics);

  // pNormals数据需要在pHeightfieldData也有效的前提下才能使用
  GXBOOL Initialize(GXGraphics* pGraphics, const BLOCKDESC* pDesc);

public:
  void SetColor(const float3& vPos);  // 临时
  void SetHeight(HeightfieldTerrain* pTerrain, const float3& vPos, int nWidth, int nHeight, const float* pBrush);  // 临时
public:
  static const GXDWORD Code = GXMAKEFOURCC('H','T','R','B');
  //************************************
  // Method:    Create
  // FullName:  HeightfieldTerrain::Create
  // Returns:   GXHRESULT
  // Qualifier:
  // Parameter: HeightfieldTerrain** ppTerrain  地形对象
  //************************************
  static GXHRESULT Create(GXGraphics* pGraphics, HeightfieldTerrainBlock** ppTerrain, const BLOCKDESC* pDesc);
};

//////////////////////////////////////////////////////////////////////////

class GAMEENGINE_DLL HeightfieldTerrain : public GVMesh
{
public:
  enum CREATIONFLAGS
  {
    CreationFlag_EditableMesh = 0x00000001,  // 可编辑网格

    // CreationFlag_EditableMesh
    // 编辑模式下，所有使用的网格会向外扩充一格，方便法线计算
  };

  struct GROUPDESC
  {
    GXSIZE_T  cbSize;
    float3    vPos;
    int       nWidth;
    int       nHeight;
    int       nBlockSegmentX;
    int       nBlockSegmentY;
    float     fStride;
    const float* pHeightfieldData; // 长度应该是 nWidth * nHeight
    int       cbPitch;
    CREATIONFLAGS dwFlags;
  };
  static const GXDWORD Code = GXMAKEFOURCC('H','T','R','N');

protected:
  float3    m_vPos;
  //int       m_nWidth;
  //int       m_nHeight;
  //int       m_nBlockSegmentX;
  //int       m_nBlockSegmentY;
  int       m_xCount; 
  int       m_yCount;
  float     m_fBlockExtX;
  float     m_fBlockExtY;
  IndicesArray  m_aIndices; // 编辑模式下用来计算法线的索引

  //float     m_fStride;
  CREATIONFLAGS m_dwFlags;
  HeightfieldTerrainBlock** m_aBlocks;

protected:
  HeightfieldTerrain(GXGraphics* pGraphics, int xCount, int yCount, const GROUPDESC& d);
  virtual ~HeightfieldTerrain();

  void PosToSeat(GXPOINT* pt, const float3& vPos);
  HeightfieldTerrainBlock* GetBlockBySeatUnsafe(int x, int y);
  HeightfieldTerrainBlock* GetBlockByPosUnsafe(const float3& vPos);

public:
  void BrushHeight(const float3& vPos, clstd::ImageFilterF* pBrush);
  void BrushMask(const float3& vPos, GTexture* pBrushTex);
  VIndex* GetExtendedMeshIndex(int* nCount);

  virtual GXBOOL  RayTrace        (const Ray& ray, NODERAYTRACE* pRayTrace);

public:
  static GXHRESULT CreateGroup(GXGraphics* pGraphics, HeightfieldTerrain** ppTerrain, const GROUPDESC* pDesc);
  static GXHRESULT CreateFromTexture(GXGraphics* pGraphics, HeightfieldTerrain** ppTerrain, GTexture* pTexture, const float3& vPos, int nBlockSegmentX, int nBlockSegmentY, float fStride, float fHeightScaling, CREATIONFLAGS dwFlags);
};

#endif // #ifndef _MARIMO_ENGINE_HEIGHTFIELD_TERRAIN_H_