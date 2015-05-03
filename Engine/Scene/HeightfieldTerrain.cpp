#include "GrapX.H"
#include "clTree.H"
#include "clTransform.h"
#include "clImage.h"
#include "GameEngine.h"

#include "GXUser.H"

#include "GUnknown.H"
#include "GResource.H"
#include "GPrimitive.h"
#include "GTexture.H"
#include "3D/gvNode.h"
#include "3D/gvMesh.h"
#include "HeightfieldTerrain.h"
#include "gxUtility.h"


HeightfieldTerrainBlock::HeightfieldTerrainBlock( GXGraphics* pGraphics )
  : GVMesh(pGraphics, Code)
  , m_nVertCountX(0)
  , m_nVertCountY(0)
  , m_fStride(0)
{
  m_ptID.x = 0;
  m_ptID.y = 0;
}

GXBOOL HeightfieldTerrainBlock::Initialize( GXGraphics* pGraphics, const BLOCKDESC* pDesc )
  //const float3& vStartPos, int nSegmentX, int nSegmentY, float fStride, 
  //const float* pHeightfieldData, int cbHFDPitch, const float3* pInputNormals, int cbNormalsPitch, GXDWORD dwFlags)
{
  typedef clvector<float3> Float3Array;
  typedef clvector<float2> Float2Array;
  typedef clvector<GXColor32> Color32Array;

  //m_nVertCountX = nSegmentX;
  //m_nVertCountY = nSegmentY;
  m_fStride = pDesc->fStride;
  m_ptID = pDesc->ptID;

  GVMESHDATA MeshData = {0};

  Float3Array   aNormals;
  Float3Array   aVertices;
  Float2Array   aTexcoords;
  IndicesArray  aIndices;
  Color32Array  aColors;

  const GXBOOL bEditableMesh = TEST_FLAG(pDesc->dwFlags, HeightfieldTerrain::CreationFlag_EditableMesh);

  size_t nSize;
  float3 vPos = pDesc->vPos;
  if(bEditableMesh) {
    m_nVertCountX = pDesc->nSegmentX + 3;
    m_nVertCountY = pDesc->nSegmentY + 3;
    aIndices.reserve((pDesc->nSegmentX + 2) * (pDesc->nSegmentY + 2) * 6);
    vPos.x -= m_fStride;
    vPos.z -= m_fStride;
  }
  else {
    m_nVertCountX = pDesc->nSegmentX + 1;
    m_nVertCountY = pDesc->nSegmentY + 1;    
    aIndices.reserve(pDesc->nSegmentX * pDesc->nSegmentY * 6);
  }

  nSize = m_nVertCountX * m_nVertCountY;
  aVertices.reserve(nSize);
  aTexcoords.reserve(nSize);
  aNormals.reserve(nSize);
  aColors.reserve(nSize);

  const float fInvStride = 1.0f / pDesc->fStride;

  // 这里构建基础顶点
  for(GXINT y = 0; y < m_nVertCountY; y++)
  {
    float v = (float)y * fInvStride;
    for(GXINT x = 0; x < m_nVertCountX; x++)
    {
      aNormals.push_back(float3::AxisY);
      aVertices.push_back(vPos);
      aTexcoords.push_back(float2((float)x * fInvStride, v));
      vPos.x += pDesc->fStride;
      aColors.push_back(-1);
    }
    vPos.x -= m_fStride * m_nVertCountX;
    vPos.z += pDesc->fStride;
  }

  // @.这里如果有高度数据，则拷贝一次
  // @.分带法线拷贝和没有法线的拷贝
  // @.没有法线的拷贝之后会计算一次法线
  if(pDesc->pHeightfield) {
    const float* pData = pDesc->pHeightfield;
    const float3* pNormalsSrc = pDesc->pNormals;
    float3* pVertices = &aVertices.front();
    float3* pNormals = &aNormals.front();

    if(pDesc->pNormals) // 带法线拷贝
    {
      for(GXINT y = 0; y < m_nVertCountY; y++) {
        for(GXINT x = 0; x < m_nVertCountX; x++) {
          pVertices->y += *pData++;
          *pNormals++ = *pNormalsSrc++;
          pVertices++;
        }

        pData = (float*)((GXLPBYTE)pData + pDesc->cbHFPitch - sizeof(float) * m_nVertCountX);
        pNormalsSrc = (float3*)((GXLPBYTE)pNormalsSrc + pDesc->cbNormalsPitch - sizeof(float3) * m_nVertCountX);
      }

      // 按照四边形网格方式填充索引
      if(bEditableMesh) {
        PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(pDesc->nSegmentX, pDesc->nSegmentY, pDesc->nSegmentX + 4, aIndices, pDesc->nSegmentX + 3);
      }
      else {
        PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(pDesc->nSegmentX, pDesc->nSegmentY, 0, aIndices);
      }
    }
    else // 不带法线的拷贝
    {
      for(GXINT y = 0; y < m_nVertCountY; y++) {
        for(GXINT x = 0; x < m_nVertCountX; x++) {
          pVertices->y += *pData++;
          pVertices++;
        }
        pData = (float*)((GXLPBYTE)pData + pDesc->cbHFPitch - sizeof(float) * m_nVertCountX);
      }
      // 按照四边形网格方式填充索引
      if(bEditableMesh) {        
        //PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(pDesc->nSegmentX + 2, pDesc->nSegmentY + 2, 0, aIndices, pDesc->nSegmentX + 3);

        if(pDesc->pTerrain)
        {
          int nICount;
          VIndex* pIndices = pDesc->pTerrain->GetExtendedMeshIndex(&nICount);
          mesh::CalculateNormals(aNormals, aVertices, pIndices, nICount / 3);
        }
        else {
          mesh::CalculateNormals(aNormals, aVertices, aIndices);
        }

        //aIndices.clear();
        PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(pDesc->nSegmentX, pDesc->nSegmentY, pDesc->nSegmentX + 4, aIndices, pDesc->nSegmentX + 3);
      }
      else {
        PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(pDesc->nSegmentX, pDesc->nSegmentY, 0, aIndices);
        mesh::CalculateNormals(aNormals, aVertices, aIndices);
      }
    }
  }


  MeshData.nVertexCount = aVertices.size();
  MeshData.nIndexCount  = aIndices.size();
  MeshData.pIndices     = &aIndices.front();
  MeshData.pVertices    = &aVertices.front();
  MeshData.pTexcoord0   = &aTexcoords.front();
  MeshData.pNormals     = &aNormals.front();
  MeshData.pColors32    = &aColors.front();


  // 暂时禁止纹理坐标
  //MeshData.pTexcoord0 = &aTexcoords.front();

  m_nPrimiCount = aIndices.size() / 3;
  m_nVertCount = aVertices.size();

  return IntCreateMesh(pGraphics, &MeshData);
}

//////////////////////////////////////////////////////////////////////////
GXHRESULT HeightfieldTerrainBlock::Create( GXGraphics* pGraphics, HeightfieldTerrainBlock** ppTerrain, const BLOCKDESC* pDesc )
{
  if(pDesc->cbSize != sizeof(BLOCKDESC)) {
    return GX_FAIL;
  }

  HeightfieldTerrainBlock* pTerrain = new HeightfieldTerrainBlock(pGraphics);
  if( ! InlCheckNewAndIncReference(pTerrain)) {
    return GX_FAIL;
  }

  BLOCKDESC desc = *pDesc;
  //desc.nSegmentX += 2;
  //desc.nSegmentY += 2;
  //desc.dwFlags = 0;

  if( ! pTerrain->Initialize(pGraphics, &desc))
    //->vPos, pDesc->nSegmentX, pDesc->nSegmentY, pDesc->fStride, 
    //  pDesc->pHeightfield, pDesc->cbHFPitch, pDesc->pNormals, pDesc->cbNormalsPitch, pDesc->dwFlags))
  {
    pTerrain->Release();
    pTerrain = NULL;
    return GX_FAIL;
  }

  pTerrain->CombineFlags(GVNF_STATIC);
  *ppTerrain = pTerrain;
  return GX_OK;
}

GXHRESULT HeightfieldTerrain::CreateGroup( GXGraphics* pGraphics, HeightfieldTerrain** ppTerrain, const GROUPDESC* pDesc )
{
  if(pDesc->cbSize != sizeof(GROUPDESC)) {
    return GX_FAIL;
  }

  int xCount = 0;
  int yCount = 0;

  if(TEST_FLAG(pDesc->dwFlags, CreationFlag_EditableMesh)) {
    xCount = (pDesc->nWidth  - 3) / pDesc->nBlockSegmentX;
    yCount = (pDesc->nHeight - 3) / pDesc->nBlockSegmentY;
  }
  else {
    xCount = (pDesc->nWidth  - 1) / pDesc->nBlockSegmentX;
    yCount = (pDesc->nHeight - 1) / pDesc->nBlockSegmentY;
  }

  HeightfieldTerrain* pNewTerrain = new HeightfieldTerrain(pGraphics, xCount, yCount, *pDesc);
  if( ! InlCheckNewAndIncReference(pNewTerrain)) {
    return GX_FAIL;
  }

  HeightfieldTerrainBlock::BLOCKDESC sBlock = {sizeof(HeightfieldTerrainBlock::BLOCKDESC)};
  sBlock.cbHFPitch = pDesc->cbPitch;
  sBlock.vPos      = pDesc->vPos;
  sBlock.nSegmentX = pDesc->nBlockSegmentX;
  sBlock.nSegmentY = pDesc->nBlockSegmentY;
  sBlock.fStride   = pDesc->fStride;
  sBlock.dwFlags   = pDesc->dwFlags;
  sBlock.pTerrain  = pNewTerrain;


  //
  // 预先计算Normals
  // 因为Block之间的法线是关联的，分布到Block后再计算，
  // 相邻Block上同一个位置的顶点计算出来的法线无法完全重合
  float3 vPos = 0.0f;
  typedef clvector<float3> Float3Array;
  typedef clvector<float2> Float2Array;

  Float3Array    aNormals;
  Float3Array    aVertices;
  Indices32Array aIndices;
  const float* pHFData = pDesc->pHeightfieldData;
  const int nTotalSize = pDesc->nWidth * pDesc->nHeight;
  aVertices.reserve(nTotalSize);
  aNormals.reserve(nTotalSize);
  aIndices.reserve(nTotalSize * 6);


  // 计算整体的法线
  if(TEST_FLAG(pDesc->dwFlags, CreationFlag_EditableMesh)) {
    // 这么写是为了标志判断的统一，防止看走眼
  }
  else {
    for(GXINT y = 0; y < pDesc->nHeight; y++)
    {
      pHFData = (const float*)((GXLPBYTE)pDesc->pHeightfieldData + y * pDesc->cbPitch);
      for(GXINT x = 0; x < pDesc->nWidth; x++)
      {
        aNormals.push_back(float3::AxisY);

        vPos.y = *pHFData++;
        aVertices.push_back(vPos);
        vPos.x += pDesc->fStride;
      }
      vPos.x = 0.0f;
      vPos.z += pDesc->fStride;
    }

    PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW32(pDesc->nWidth - 1, pDesc->nHeight - 1, 0, aIndices);
    mesh::CalculateNormals(aNormals, aVertices, aIndices);
    sBlock.cbNormalsPitch = pDesc->nWidth * sizeof(float3);
  }

  const int nBlockSegSizeX = pDesc->nBlockSegmentX;
  const int nBlockSegSizeY = pDesc->nBlockSegmentY;

  clStringA strName;

  // 按照分块创建
  for(int y = 0; y < yCount; y++)
  {
    sBlock.pHeightfield = (float*)((GXLPBYTE)pDesc->pHeightfieldData + (nBlockSegSizeY * pDesc->cbPitch) * y);
    sBlock.ptID.y = y;

    if( ! aNormals.empty()) {
      sBlock.pNormals = (float3*)((GXLPBYTE)&aNormals.front() + (nBlockSegSizeY * pDesc->nWidth * sizeof(float3)) * y);
    }

    for(int x = 0; x < xCount; x++)
    {
      HeightfieldTerrainBlock* pTrnBlock = NULL;
      sBlock.ptID.x = x;
      if(GXFAILED(HeightfieldTerrainBlock::Create(pGraphics, &pTrnBlock, &sBlock))) {
        pNewTerrain->Release();
        return GX_FAIL;
      }

      pNewTerrain->m_aBlocks[y * xCount + x] = pTrnBlock;
      pTrnBlock->AddRef();

      strName.Format("HFTerrainBlock %d %d", sBlock.ptID.x, sBlock.ptID.y);

      pTrnBlock->SetName(strName);
      pTrnBlock->SetParent(pNewTerrain);
      sBlock.vPos.x += pDesc->fStride * (float)pDesc->nBlockSegmentX;
      sBlock.pHeightfield += nBlockSegSizeX;

      if(sBlock.pNormals) {
        sBlock.pNormals += nBlockSegSizeX;
      }
    }
    sBlock.vPos.x = pDesc->vPos.x;
    sBlock.vPos.z += pDesc->fStride * (float)pDesc->nBlockSegmentY;
  }
  pNewTerrain->CalculateAABB();
  *ppTerrain = pNewTerrain;
  return GX_OK;
}

GXHRESULT HeightfieldTerrain::CreateFromTexture( GXGraphics* pGraphics, HeightfieldTerrain** ppTerrain, GTexture* pTexture, const float3& vPos, int nBlockSegmentX, int nBlockSegmentY, float fStride, float fHeightScaling, CREATIONFLAGS dwFlags)
{
  GTexture::LOCKEDRECT lr;
  if( ! pTexture->LockRect(&lr, NULL, NULL)) {
    return GX_FAIL;
  }

  GROUPDESC sGroup = {sizeof(GROUPDESC)};

  const int nWidth = pTexture->GetWidth();
  const int nHeight = pTexture->GetHeight();

  sGroup.vPos           = vPos;
  sGroup.nWidth         = nWidth + 1;
  sGroup.nHeight        = nHeight + 1;
  sGroup.nBlockSegmentX = nBlockSegmentX;
  sGroup.nBlockSegmentY = nBlockSegmentY;
  sGroup.fStride        = fStride;
  sGroup.dwFlags        = dwFlags;

  GXUINT nPixelStride = GetBytesOfGraphicsFormat(pTexture->GetFormat());
  GXLPBYTE lpLine = NULL;

  // 标准模式下会向右扩一个数值，使（n+1）个顶点完成n个分段
  // 编辑模式下在此基础上左右还会各向外扩充一个数值，是边缘也存在毗邻数据，用来计算平滑的法线
  if(TEST_FLAG(dwFlags, CreationFlag_EditableMesh)) {
    sGroup.nWidth  += 2;
    sGroup.nHeight += 2;
    sGroup.cbPitch = sizeof(float) * (nWidth + 3);
    sGroup.pHeightfieldData = new float[(nWidth + 3) * (nHeight + 3)];
    lpLine = (GXLPBYTE)sGroup.pHeightfieldData;

    // 这个lpLine是去掉了扩充之后的高度数据的开始位置
    // 这里要跳过一整行+第2行第1个扩充数值
    memset(lpLine, 0, sGroup.cbPitch + sizeof(float));
    lpLine += sGroup.cbPitch + sizeof(float);
  }
  else {
    sGroup.cbPitch = sizeof(float) * (nWidth + 1);
    sGroup.pHeightfieldData = new float[(nWidth + 1) * (nHeight + 1)];
    lpLine = (GXLPBYTE)sGroup.pHeightfieldData;
  }

  // 位图数据转换为浮点数据
  float* lpDest = NULL;
  for(int y = 0; y < nHeight; y++)
  {
    GXLPBYTE lpBmpData = (GXLPBYTE)lr.pBits + lr.Pitch * y;
    lpDest = (float*)(lpLine + sGroup.cbPitch * y);
    for(int x = 0; x < nWidth; x++)
    {
      *lpDest = *lpBmpData * ((1.0f / 255.0f) * fHeightScaling);
      lpBmpData += nPixelStride;
      lpDest++;
    }
    *lpDest = *(lpDest - 1);

    // @.CreationFlag_EditableMesh 下清零
    // @.否则会被后来的数值覆盖
    lpDest++;
    *lpDest = 0;
    lpDest++;
    *lpDest = 0;
  }
  memset(lpDest + 1, 0, sGroup.cbPitch);

  pTexture->UnlockRect();

  GXHRESULT result = CreateGroup(pGraphics, ppTerrain, &sGroup);
  SAFE_DELETE_ARRAY(sGroup.pHeightfieldData);

  return result;
}

HeightfieldTerrain::HeightfieldTerrain( GXGraphics* pGraphics, int xCount, int yCount, const GROUPDESC& d )
  : GVMesh(pGraphics, Code)
  , m_vPos(d.vPos)
  , m_xCount(xCount)
  , m_yCount(yCount)
  , m_fBlockExtX(d.nBlockSegmentX * d.fStride)
  , m_fBlockExtY(d.nBlockSegmentY * d.fStride)
  , m_dwFlags(d.dwFlags)
{
  m_aBlocks = new HeightfieldTerrainBlock*[xCount * yCount];
  memset(m_aBlocks, 0, sizeof(HeightfieldTerrainBlock*) * xCount * yCount);

  if(TEST_FLAG(m_dwFlags, CreationFlag_EditableMesh)){
    PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCCW16(d.nBlockSegmentX + 2, d.nBlockSegmentY + 2, 0, m_aIndices, d.nBlockSegmentX + 3);
  }
}

void HeightfieldTerrain::BrushHeight( const float3& vPos, clstd::ImageFilterF* pBrushImage)
{
  GXRECT rect;
  const int nWidth = pBrushImage->GetWidth();
  const int nHeight = pBrushImage->GetHeight();
  const float* pBrush = (float*)pBrushImage->GetPixelPtr(0, 0);

  float3 vHalfExt((float)(nWidth >> 1) + 2, 0, (float)(nHeight >> 1) + 2);
  PosToSeat((GXPOINT*)&rect, vPos - vHalfExt);
  PosToSeat((GXPOINT*)&rect.right, vPos + vHalfExt);
  
  //rect.left   = pt.x - (nWidth >> 1);
  //rect.top    = pt.y - (nHeight >> 1);
  //rect.right  = rect.left + nWidth;
  //rect.bottom = rect.top + nHeight;
  rect.left   = clMax(0, rect.left);
  rect.top    = clMax(0, rect.top);
  rect.right  = clMin(m_xCount - 1, rect.right);
  rect.bottom = clMin(m_yCount - 1, rect.bottom);

  for(int y = rect.top; y <= rect.bottom; y++)
  {
    for(int x = rect.left; x <= rect.right; x++)
    {
      HeightfieldTerrainBlock* pBlock = GetBlockBySeatUnsafe(x, y);
      pBlock->SetHeight(this, vPos, nWidth, nHeight, pBrush);
      pBlock->CalculateAABB();
    }
  }
}

GXBOOL HeightfieldTerrain::RayTrace( const Ray& ray, NODERAYTRACE* pRayTrace )
{
  return GVMesh::RayTrace(ray, pRayTrace);
}

HeightfieldTerrain::~HeightfieldTerrain()
{
  int nCount = m_xCount * m_yCount;
  for(int i = 0; i < nCount; i++)
  {
    SAFE_RELEASE(m_aBlocks[i]);
  }
  SAFE_DELETE_ARRAY(m_aBlocks);
}

HeightfieldTerrainBlock* HeightfieldTerrain::GetBlockByPosUnsafe( const float3& vPos )
{
  GXPOINT pt;
  PosToSeat(&pt, vPos);
  return GetBlockBySeatUnsafe(pt.x, pt.y);
}

HeightfieldTerrainBlock* HeightfieldTerrain::GetBlockBySeatUnsafe( int x, int y )
{
  return m_aBlocks[y * m_xCount + x];
}

void HeightfieldTerrain::PosToSeat( GXPOINT* pt, const float3& vPos )
{
  pt->x = (int)((vPos.x - m_vPos.x) / m_fBlockExtX);
  pt->y = (int)((vPos.z - m_vPos.z) / m_fBlockExtY);
}

VIndex* HeightfieldTerrain::GetExtendedMeshIndex( int* nCount )
{
  if(m_aIndices.empty()) {
    return NULL;
  }

  if(nCount) {
    *nCount = (int)m_aIndices.size();
  }
  return &m_aIndices.front();
}

void HeightfieldTerrain::BrushMask( const float3& vPos, GTexture* pBrushTex )
{

}

//////////////////////////////////////////////////////////////////////////

void HeightfieldTerrainBlock::SetColor( const float3& vPos )
{
  AABB aabb;
  GetAbsoluteAABB(aabb);
  float3 vLocal = vPos - aabb.vMin;
  int x = (int)(vLocal.x / m_fStride);
  int y = (int)(vLocal.z / m_fStride);
  
  if(x >= 0 && x < m_nVertCountX && y >= 0 && y < m_nVertCountY)
  {
    PrimitiveUtility::SetUnifiedDiffuse(m_pPrimitive, 0xffffffff, TRUE);
    GXINT nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_COLOR, 0);
    GXUINT nVertStride = m_pPrimitive->GetVertexStride();
    GXDWORD* dwColor = (GXDWORD*)((GXLPBYTE)m_pPrimitive->GetVerticesBuffer() + (m_nVertCountX * y + x) * nVertStride + nOffset);
    *dwColor = 0xff00ff00;
    m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
  }
}

void HeightfieldTerrainBlock::SetHeight(HeightfieldTerrain* pTerrain, const float3& vPos, int nWidth, int nHeight, const float* pBrush)
{
  AABB aabb;
  GetAbsoluteAABB(aabb);
  float3 vLocal = vPos - aabb.vMin;
  int x = (int)floor(vLocal.x / m_fStride + 0.5f) - (nWidth  >> 1);
  int y = (int)floor(vLocal.z / m_fStride + 0.5f) - (nHeight >> 1);

  GXRECT rcBrush = {x, y, x + nWidth, y + nHeight};
  GXRECT rcVert = {0, 0, m_nVertCountX, m_nVertCountY};
  GXRECT rcClip;

  gxIntersectRect(&rcClip, &rcBrush, &rcVert);

  int xBrush = clMax(-x, 0);
  int yBrush = clMax(-y, 0);
  const float* pClippedBrush = pBrush + yBrush * nWidth + xBrush;
  
  int xCopy = rcClip.right - rcClip.left;
  int yCopy = rcClip.bottom - rcClip.top;

  x = rcClip.left;
  y = rcClip.top;

  if(x >= 0 && x < m_nVertCountX && y >= 0 && y < m_nVertCountY)
  {
    GXINT nVerticesOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
    GXINT nNormalOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_NORMAL, 0);
    GXUINT nVertStride = m_pPrimitive->GetVertexStride();
    GXLPBYTE lpVerticesData = (GXLPBYTE)m_pPrimitive->GetVerticesBuffer();
    float3* pVertex = (float3*)(lpVerticesData + (m_nVertCountX * y + x) * nVertStride + nVerticesOffset);

    for(int yl = 0; yl < yCopy; yl++)
    {
      for(int xl = 0; xl < xCopy; xl++)
      {
        pVertex->y += *pClippedBrush;
        pVertex = (float3*)((GXLPBYTE)pVertex + nVertStride);
        pClippedBrush++;
      }

      pVertex = (float3*)((GXLPBYTE)pVertex + (m_nVertCountX - xCopy) * nVertStride);
      pClippedBrush += (nWidth - xCopy);
    }

    //pVertex->y += 0.1f;

    int nICount;
    VIndex* pIndices = pTerrain->GetExtendedMeshIndex(&nICount);
    if(pIndices) {
      mesh::CalculateNormals((float3*)(lpVerticesData + nNormalOffset), (float3*)(lpVerticesData + nVerticesOffset), m_pPrimitive->GetVerticesCount(), 
        pIndices, nICount / 3, nVertStride, nVertStride);
    }
    else {
      mesh::CalculateNormals((float3*)(lpVerticesData + nNormalOffset), (float3*)(lpVerticesData + nVerticesOffset), m_pPrimitive->GetVerticesCount(), 
        (VIndex*)m_pPrimitive->GetIndicesBuffer(), m_pPrimitive->GetIndexCount() / 3, nVertStride, nVertStride);
    }
    m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
  }
}
