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
    float3        vPos;           // ��ʼλ�ã����λ���������һ�����䣬��������λ��
    int           nSegmentX;      // X��ֶ���
    int           nSegmentY;      // Y��ֶ���
    float         fStride;        // ÿ���εĲ���
    const float*  pHeightfield;   // �߶ȳ�����
    int           cbHFPitch;      // pHeightfieldDataÿ�еĳ���(bytes)
    const float3* pNormals;
    int           cbNormalsPitch;
    GXPOINT       ptID;           // λ��ID��������������е��������꣬Ҳ�п��������������������������߾���
    GXDWORD       dwFlags;        // �ο�CREATIONFLAGS
    HeightfieldTerrain* pTerrain;
    // #.pHeightfield �߶ȳ�����
    // 1.��׼ģʽ��������ݳ���Ӧ����(nSegmentX + 1) * (nSegmentY + 1)
    // 2.CreationFlag_EditableMeshģʽ�³�����(nSegmentX + 3) * (nSegmentY + 3)
    //   ��Եֻ��������㷨�ߣ��������ڹ���ģ�͡�
  };


protected:
  GXPOINT m_ptID;
  int m_nVertCountX;
  int m_nVertCountY;
  float m_fStride;  // ÿ���εĲ���

protected:
  HeightfieldTerrainBlock(GXGraphics* pGraphics);

  // pNormals������Ҫ��pHeightfieldDataҲ��Ч��ǰ���²���ʹ��
  GXBOOL Initialize(GXGraphics* pGraphics, const BLOCKDESC* pDesc);

public:
  void SetColor(const float3& vPos);  // ��ʱ
  void SetHeight(HeightfieldTerrain* pTerrain, const float3& vPos, int nWidth, int nHeight, const float* pBrush);  // ��ʱ
public:
  static const GXDWORD Code = GXMAKEFOURCC('H','T','R','B');
  //************************************
  // Method:    Create
  // FullName:  HeightfieldTerrain::Create
  // Returns:   GXHRESULT
  // Qualifier:
  // Parameter: HeightfieldTerrain** ppTerrain  ���ζ���
  //************************************
  static GXHRESULT Create(GXGraphics* pGraphics, HeightfieldTerrainBlock** ppTerrain, const BLOCKDESC* pDesc);
};

//////////////////////////////////////////////////////////////////////////

class GAMEENGINE_DLL HeightfieldTerrain : public GVMesh
{
public:
  enum CREATIONFLAGS
  {
    CreationFlag_EditableMesh = 0x00000001,  // �ɱ༭����

    // CreationFlag_EditableMesh
    // �༭ģʽ�£�����ʹ�õ��������������һ�񣬷��㷨�߼���
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
    const float* pHeightfieldData; // ����Ӧ���� nWidth * nHeight
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
  IndicesArray  m_aIndices; // �༭ģʽ���������㷨�ߵ�����

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