#include "GrapX.H"

#include "Smart/smartstream.h"
#include "smart/SmartRepository.h"
#include "clUtility.H"

//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.H"
#include "GrapX/GShader.H"
#include "GrapX/gxUtility.h"

#include "GrapX/GrapVR.H"

#define MESHFMT_VERTEXDECL   "%s@VertexDeclaration"
#define MESHFMT_ASMVERTICES  "%s@AssembledVertices"
#define MESHFMT_INDICES      "%s@Indices"
#define MESHFMT_PRIMCOUNT    "%s@PrimitiveCount"
#define MESHFMT_STARTINDEX   "%s@StartIndex"

namespace RepoUtility
{
  GXHRESULT SavePrimitive(clSmartRepository* pStorage, GXLPCSTR szDomain, GPrimitiveVI* pPrimitive, int nStartIndex, int nNumPrimi)
  {
    clStringA strMeshVertexDecl;
    clStringA strMeshAsmVertices;
    clStringA strMeshIndices;
    clStringA strMeshPrimCount;
    clStringA strMeshStartIndex;

    strMeshVertexDecl.Format(MESHFMT_VERTEXDECL, szDomain);
    strMeshAsmVertices.Format(MESHFMT_ASMVERTICES, szDomain);
    strMeshIndices.Format(MESHFMT_INDICES, szDomain);
    strMeshPrimCount.Format(MESHFMT_PRIMCOUNT, szDomain);
    strMeshStartIndex.Format(MESHFMT_STARTINDEX, szDomain);

    GVertexDeclaration* pVertexDecl = NULL;
    if(GXSUCCEEDED(pPrimitive->GetVertexDeclaration(&pVertexDecl))) {
      GXLPCVERTEXELEMENT  lpVertexElement = pVertexDecl->GetVertexElement();
      GXUINT              nElementCount   = MOGetDeclCount(lpVertexElement);
      pStorage->WriteStructArrayT(NULL, strMeshVertexDecl, 
        *lpVertexElement, (nElementCount + 1));
      SAFE_RELEASE(pVertexDecl);
    }

    pStorage->Write(NULL, strMeshAsmVertices, pPrimitive->GetVerticesBuffer(), 
      pPrimitive->GetVertexStride() * pPrimitive->GetVerticesCount());

    pStorage->Write(NULL, strMeshIndices, pPrimitive->GetIndicesBuffer(), 
      pPrimitive->GetIndexCount() * sizeof(VIndex));

    pStorage->Write64(NULL, strMeshPrimCount, nNumPrimi, 0);
    pStorage->Write64(NULL, strMeshStartIndex, nStartIndex, 0);

    return GX_OK;
  }

  GXHRESULT LoadPrimitive(clSmartRepository* pStorage, GXLPCSTR szDomain, GXVERTEXELEMENT* pVertElement, clBuffer* pVertices, clBuffer* pIndices, GXSIZE_T& nStartIndex, GXSIZE_T& nNumPrimi)
  {
    clStringA strMeshVertexDecl;
    clStringA strMeshAsmVertices;
    clStringA strMeshIndices;
    clStringA strMeshPrimCount;
    clStringA strMeshStartIndex;

    strMeshVertexDecl.Format(MESHFMT_VERTEXDECL, szDomain);
    strMeshAsmVertices.Format(MESHFMT_ASMVERTICES, szDomain);
    strMeshIndices.Format(MESHFMT_INDICES, szDomain);
    strMeshPrimCount.Format(MESHFMT_PRIMCOUNT, szDomain);
    strMeshStartIndex.Format(MESHFMT_STARTINDEX, szDomain);

    //InlSetZeroT(VertexElement);
    s32 nReadSize = pStorage->ReadStructArrayT(NULL, strMeshVertexDecl, *pVertElement, 64);
    if(nReadSize < 0) {
      ASSERT(0);
    }

    //s32 cbVertSize = pStorage->GetLength(NULL, strMeshAsmVertices);
    //s32 nIndexCount = pStorage->GetLength(NULL, strMeshIndices) / sizeof(VIndex);

    //GXBYTE* pVertices = NULL;
    //VIndex* pIndices  = NULL;
    GXBOOL bval = TRUE;
    //try
    {
      //pVertices = new GXBYTE[cbVertSize];
      //pIndices = new VIndex[nIndexCount];
      //pVertices->Resize(cbVertSize, FALSE);
      //pIndices->Resize(sizeof(VIndex) * nIndexCount, FALSE);

      nReadSize = pStorage->ReadToBuffer(NULL, strMeshAsmVertices, pVertices);
      if(nReadSize != pVertices->GetSize()) {
        ASSERT(0);
      }
      nReadSize = pStorage->ReadToBuffer(NULL, strMeshIndices, pIndices);
      if(nReadSize != pIndices->GetSize())
      {
        ASSERT(0);
      }

      //int nPrimiCount;
      //int nStartIndex;
      pStorage->Read64(NULL, strMeshPrimCount, (u32*)&nNumPrimi, 0);
      pStorage->Read64(NULL, strMeshStartIndex, (u32*)&nStartIndex, 0);

      //bval = IntCreatePrimitive(pGraphics, nPrimiCount, VertexElement, pVertices, 
      //  nPrimiCount * 3, pIndices, nIndexCount);

      //if( ! pStorage->ReadStructT(NULL, MESH_TRANSFORM, matLocal)) {
      //  matLocal.identity();
      //}
      return GX_OK;
    }
  }
} // namespace RepoUtility

namespace ObjMeshUtility
{
  size_t CALLBACK ObjSymbolProc(SmartStreamA::iterator& it, clsize uRemain, u32_ptr lParam)
  {
    if(it.marker[0] == '#') {
      u32 i = 0;
      while(i < uRemain) {
        if(it.marker[i] == 0x0d || it.marker[i] == 0x0a) {
          it.length = i;
          return i;
        }
        i++;
      }
    }
    return it.length;
  }

  GXBOOL LoadFromFileA(GXLPCSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs)
  {
    return LoadFromFileW(clStringW(szFilename), pTransform, aMeshs);
  }

  GXBOOL LoadFromFileW(GXLPCWSTR szFilename, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs)
  {
    clFile file;
    GXBOOL bval = FALSE;
    if(szFilename == NULL) {
      return bval;
    }
    if(file.OpenExisting(szFilename)) {
      clBuffer* pBuffer = NULL;
      file.MapToBuffer(&pBuffer);
      bval = LoadFromMemory(pBuffer, pTransform, aMeshs);
      SAFE_DELETE(pBuffer);
    }
    return bval;
  }

  GXBOOL LoadFromMemory(const clBufferBase* pBuffer, CFloat4x4* pTransform, GXOUT PrimaryMeshsArray& aMeshs)
  {
    //clStringA strFilename = m_strName;

    SmartStreamA ss;
    ss.Initialize((const ch*)pBuffer->GetPtr(), (u32)pBuffer->GetSize());

    ss.SetFlags(SmartStreamA::F_SYMBOLBREAK);

    // 设置符号表
    u32 SemiTable[128];
    ss.GetCharSemantic(SemiTable, 0, 128);
    //SemiTable[0x0d] |= SmartStreamA::M_GAP;
    //SemiTable[0x0a] |= SmartStreamA::M_GAP;
    SemiTable['.'] |= SmartStreamA::M_LABEL;
    SemiTable['-'] |= SmartStreamA::M_LABEL;
    SemiTable['/'] |= SmartStreamA::M_LABEL;
    SemiTable['\n'] = SmartStreamA::M_SYMBOL;
    ss.SetCharSemantic(SemiTable, 0, 128);


    //GShader* pDefault = NULL;
    //pGraphics->CreateShaderFromFileA(&pDefault, "Resource\\SimpleShader_PosTexNor.txt");
    //ASSERT(pDefault != NULL);

    // 设置解析的回调函数
    ss.SetIteratorCallBack(ObjSymbolProc, NULL);

    typedef clvector<float3> VertexArray;
    typedef clvector<float3> TexcoordArray;
    typedef clvector<float3> NormalArray;
    //typedef clhash_map<string, int> VertexDict;
    //typedef clvector<VIndex> FaceArray; // TODO: 不能叫Face，或者单独声明一个Face结构

    VertexArray   aVertices;
    TexcoordArray aTexcoords;
    NormalArray   aNormals;
    //clvector<GXVERTEX_P3T2N3F> aMeshVertices;
    //VertexDict    MeshVertDict;
    //FaceArray     aFaces;
    int           nFaceIdx = 0;
    cllist<clStringA> NameList;
    INDEXEDTRIANGLE face;
    PRIMARYMESH     me;

    float3 v;
    for(SmartStreamA::iterator it = ss.begin();
      it != ss.end(); ++it)
    {
      if(it == "v" || it == "vn" || it == "vt") 
      {
        const ch* szMarker = it.marker;
        for(int i = 0; i < 3; i++)
        {
          ++it;
          clStringA strMarker = it.ToString();
          v.m[i] = (float)clstd::xtof((const ch*)strMarker);
          //TRACE("%s=%f\t", strMarker, v.m[i]);
        }
        //TRACE("\n");

        if(szMarker[1] == ' ') {
          if(pTransform != NULL) {
            v *= (*pTransform);
          }
          aVertices.push_back(v);
        }
        else if(szMarker[1] == 'n') {
          // TODO: 法线没做变换,需要修正
          //if(pTransform != NULL) {
          //  v *= (*pTransform);
          //}
          v.normalize();
          aNormals.push_back(v);
        }
        else if(szMarker[1] == 't') {
          aTexcoords.push_back(v);
        }
        else {
          TRACE("%s\n", szMarker);
        }
      }
      else if(it == "g") {
        ++it;
        NameList.push_back(it.ToString());
        //m_strName = ;
      }
      else if(it == "s") {
        ++it;
        // 不知道这是啥
      }
      else if(it == "f") {
        VIndex m[4];
        int i;
        for(i = 0; i < 4; i++)
        {
          ++it;
          m[i] = nFaceIdx;

          clStringArrayA aString;
          GXVERTEX_P3T2N3F Vertex;

          if(it == '\n') { break; }
          clStringA str = it.ToString();
          clstd::ResolveString(str, '/', aString);
          ASSERT(aString.size() == 3);

          int nVertIdx   = clstd::xtoi((const char*)aString[0]) - 1;
          int nUVIdx     = clstd::xtoi((const char*)aString[1]) - 1;
          int nNormalIdx = clstd::xtoi((const char*)aString[2]) - 1;
          Vertex.pos = aVertices[nVertIdx];
          if(nUVIdx < 0) {
            Vertex.texcoord.set(0,0);
          }
          else {
            Vertex.texcoord = aTexcoords[nUVIdx].xy();
          }
          Vertex.normal = aNormals[nNormalIdx];
          //aMeshVertices.push_back(Vertex);
          me.aVertices.push_back(Vertex);
          nFaceIdx++;
        }

        // 这里检测INDEXEDTRIANGLE本质是VIndex[3]数组, 虽然不严谨，但至少长度应该相等
        STATIC_ASSERT(sizeof(INDEXEDTRIANGLE) == sizeof(VIndex) * 3);
        me.aFaces.push_back(*(INDEXEDTRIANGLE*)&m);
        if(i == 4) {
          face.a = m[0];
          face.b = m[2];
          face.c = m[3];
          me.aFaces.push_back(face);
        }
      }
      else if(it.marker[0] == '#') {
        continue;
      }
      else if(it == "mtllib") {
        do{
          ++it;
        }while( ! (it == "\n"));
      }
      else if(it == "usemtl") {
        ++it;

        if(( ! me.aVertices.empty()) && ( ! me.aFaces.empty()))
        {
          //GVMesh* pSubMesh = NULL;
          //GVMesh::CreateUserPrimitive(pGraphics, 
          //  aFaces.size() / 3, MOGetSysVertexDecl(GXVD_P3T2N3F), 
          //  &aMeshVertices.front(), aMeshVertices.size(),
          //  &aFaces.front(), aFaces.size(), &pSubMesh);

          if( ! NameList.empty())
          {
            //pSubMesh->m_strName = NameList.front();
            me.Name = NameList.front();
            NameList.pop_front();
          }
          //pSubMesh->SetParent(this);
          //CLOG("Add Obj mesh:\"%s\"\n", pSubMesh->m_strName);
          aMeshs.push_back(me);

          //aMeshVertices.clear();
          //aFaces.clear();
          me.Name.Clear();
          me.aVertices.clear();
          me.aFaces.clear();
          nFaceIdx = 0;
        }
      }
      else if(it == "\n")
      {
        //++it;
      }
      else {
        //ASSERT(0);
        CLOG_WARNING("%s: Unsupport cmd label(\"%s\").\n", __FUNCTION__, (GXLPCSTR)it.ToString());
      }
    }

    // Obj 文件读取结束后,如果列表里还有数据
    // 则按情况处理,如果之前的Mesh都按照子模型创建
    // 则剩余数据按照子模型创建
    //if(m_pFirstChild != NULL)
    //{
    //  m_dwFlags |= GVNF_CONTAINER;  // 标志这个对象是容器对象

    //  // 这是子Mesh的最后一个了
    //  if(aMeshVertices.size() > 0 && aFaces.size() > 0)
    //  {
    //    GVMesh* pSubMesh = NULL;
    //    GVMesh::CreateUserPrimitive(pGraphics, 
    //      aFaces.size() / 3, MOGetSysVertexDecl(GXVD_P3T2N3F), &aMeshVertices.front(), 
    //      aMeshVertices.size(), &aFaces.front(), aFaces.size(), &pSubMesh);

    //    //GXLPMATERIALINST pMtlInst = NULL;
    //    ////pGraphics->CreateMaterial(&pMtlInst, pDefault);
    //    //pSubMesh->SetMaterialInst(pMtlInst, FALSE);
    //    //SAFE_RELEASE(pMtlInst);

    //    if( ! NameList.empty())
    //    {
    //      pSubMesh->m_strName = NameList.front();
    //      NameList.pop_front();
    //    }
    //    pSubMesh->SetParent(this);
    //    CLOG("Add Obj mesh:\"%s\"\n", pSubMesh->m_strName);
    //  }

    //  // 更新父对象的AABB
    //  GVMesh* pChild = (GVMesh*)m_pFirstChild;
    //  while(pChild != NULL)
    //  {
    //    m_aabbLocal.vMin.Min(pChild->m_aabbLocal.vMin);
    //    m_aabbLocal.vMax.Max(pChild->m_aabbLocal.vMax);
    //    pChild = (GVMesh*)(pChild->m_pNext);
    //  }
    //  //m_aabbLocal.UpdateCenterExtent();

    //  //SAFE_RELEASE(pDefault);
    //  m_strName = strFilename;
    //  return TRUE;
    //}
    //else
    //{
      // 如果之前没有子Mesh,则剩余数据就放在当前节点对象里面
      //if(aMeshVertices.size() > 0 && aFaces.size() > 0)
    //{
    //    IntCreatePrimitive(pGraphics, aFaces.size() / 3, 
    //      MOGetSysVertexDecl(GXVD_P3T2N3F), &aMeshVertices.front(), aMeshVertices.size(),
    //      &aFaces.front(), aFaces.size());

    //    if( ! NameList.empty())
    //    {
    //      m_strName = NameList.front();
    //    }

    //    //m_pMtlInst = NULL;
    //    //pGraphics->CreateMaterial(&m_pMtlInst, pDefault);
    //    //SAFE_RELEASE(pDefault);
    //    return TRUE;
    //  }
    //  else {  // 空文件
    //    //SAFE_RELEASE(pDefault);
    //    return FALSE;
    //  }
    //}  


    if(( ! me.aVertices.empty()) && ( ! me.aFaces.empty()))
    {
      if( ! NameList.empty()) {
        me.Name = NameList.front();
      }
      aMeshs.push_back(me);
    }
    return ! aMeshs.empty();
  }

  GXBOOL SavePrimitive(clBuffer* pBuffer, GXLPCSTR szName, GVRENDERDESC* pDesc, GXINOUT u32& nVertBase)
  {
    if(pDesc->pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
    {
      GPrimitiveVI* pPrimitive = static_cast<GPrimitiveVI*>(pDesc->pPrimitive);
      GXINT nVertOffset = pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
      GXLPVOID pVertices = pPrimitive->GetVerticesBuffer();
      VIndex* pIndices = (VIndex*)pPrimitive->GetIndicesBuffer();

      ASSERT(pDesc->ePrimType == GXPT_TRIANGLELIST);

      int nStride = pPrimitive->GetVertexStride();
      int nNumVerts = pPrimitive->GetVerticesCount();
      int nNumFaces = pPrimitive->GetIndexCount() / 3;
      clStringA line;
      if(szName) {
        line.Format("g %x\n", pPrimitive);
      }
      else {
        line = clStringA('g') + szName + "\n";
      }

      pBuffer->Append(line.GetBuffer(), line.GetLength());

      for(int i = 0; i < nNumVerts; i++)
      {
        float3 v = *(float3*)((GXLPBYTE)pVertices + nStride * i + nVertOffset);
        //v = v * pDesc->matWorld;
        line.Format("v %f %f %f\n", v.x, v.y, v.z);
        pBuffer->Append(line.GetBuffer(), line.GetLength());
      }

      if(nVertBase <= 0) { // obj 文件顶点索引从1开始
        nVertBase = 1;
      }

      for(int i = 0; i < nNumFaces; i++)
      {
        line.Format("f %d %d %d\n", pIndices[i * 3] + nVertBase, pIndices[i * 3 + 1] + nVertBase, pIndices[i * 3 + 2] + nVertBase);
        pBuffer->Append(line.GetBuffer(), line.GetLength());
      }

      nVertBase += nNumVerts;
      return TRUE;
    }


    return FALSE;
  }

  //PrimaryMeshsArray* PrimaryMeshsArray::Create()
  //{
  //  return new PrimaryMeshsArray;
  //}

  //void PrimaryMeshsArray::Destroy(PrimaryMeshsArray* pMesh)
  //{
  //  delete pMesh;
  //}

  PrimaryMeshsArray::PrimaryMeshsArray()
    : m_pMeshs(NULL)
  {
    m_pMeshs = new PrimaryMeshsArray_t;
  }

  PrimaryMeshsArray::~PrimaryMeshsArray()
  {
    SAFE_DELETE(m_pMeshs);
  }

  void PrimaryMeshsArray::push_back(const PRIMARYMESH& m)
  {
    m_pMeshs->push_back(m);
  }

  GXBOOL PrimaryMeshsArray::empty() const
  {
    return m_pMeshs->empty();
  }

  size_t PrimaryMeshsArray::size() const
  {
    return m_pMeshs->size();
  }

  PrimaryMeshsArray::iterator PrimaryMeshsArray::begin() const
  {
    return m_pMeshs->begin();
  }

  PrimaryMeshsArray::iterator PrimaryMeshsArray::end() const
  {
    return m_pMeshs->end();
  }

  PRIMARYMESH& PrimaryMeshsArray::operator[](int index)
  {
    return (*m_pMeshs)[index];
  }

  const PRIMARYMESH& PrimaryMeshsArray::operator[](int index) const
  {
    return (*m_pMeshs)[index];
  }
} // namespace ObjMeshUtility

GXBOOL GXDLL PrimitiveUtility::SetUnifiedDiffuse( GPrimitive* pPrimitive, GXColor32 crDiffuse, GXBOOL bManualUpdate )
{
  GXVERTEXELEMENT element;
  GXINT nOffset = pPrimitive->GetElementOffset(GXDECLUSAGE_COLOR, 0, &element);
  if(nOffset < 0) {
    CLOG_ERROR("%s : Primitive doesn't has diffuse element.\r\n", __FUNCTION__);
    return FALSE;
  }

  ASSERT(element.Type == GXDECLTYPE_D3DCOLOR);

  GXLPBYTE lpColors = (GXLPBYTE)pPrimitive->GetVerticesBuffer();
  int nNumVertices = pPrimitive->GetVerticesCount();
  int nStride = pPrimitive->GetVertexStride();

  if(lpColors == NULL || nNumVertices == 0 || nStride <= 0) {
    if(lpColors == NULL) {
      CLOG_ERROR("%s : Can't get vertices buffer.\r\n", __FUNCTION__);
    }
    if(nNumVertices == 0) {
      CLOG_ERROR("%s : Number of vertices is empty.\r\n", __FUNCTION__);
    }
    if(nStride <= 0) {
      CLOG_ERROR("%s : Bad stride number.\r\n", __FUNCTION__);
    }
    return FALSE;
  }

  lpColors += nOffset;
  for(int i = 0; i < nNumVertices; i++)
  {
    *(GXColor32*)lpColors = crDiffuse;
    lpColors += nStride;
  }

  if( ! bManualUpdate) {
    pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
  }
  return TRUE;
}

#include "clImage.h"
#include "GrapX/GTexture.H"

GXDLL clstd::Image* TextureUtility::CreateImage( GTexture* pTexture )
{
  GTexture::LOCKEDRECT lr;
  if( ! pTexture->LockRect(&lr, NULL, 0)) {
    CLOG_ERROR("%s : Can't lock texture.\r\n", __FUNCTION__);
    return NULL;
  }
  char fmt[8];
  int nDepth;
  if( ! TextureFormatToClstdImageFormat(fmt, &nDepth, pTexture->GetFormat())) {
    CLOG_ERROR("%s : Unsupport texture format.\r\n", __FUNCTION__);
    return NULL;
  }

  auto pImage = new clstd::Image();

  if( ! pImage->Set(pTexture->GetWidth(), pTexture->GetHeight(), fmt, nDepth, lr.pBits, lr.Pitch))
  {
    CLOG_ERROR("%s : Failed to set image.\r\n", __FUNCTION__);
    SAFE_DELETE(pImage);
  }

  return pImage;
}

GXBOOL GXDLL TextureUtility::TextureFormatToClstdImageFormat(char* fmt, int* nDepth, GXFormat eFmt)
{
  switch(eFmt)
  {
  case GXFMT_R8G8B8:        *(GXDWORD*)fmt = *(GXDWORD*)"BGR ";    *nDepth = 8;    break;
  case GXFMT_A8R8G8B8:      *(GXDWORD*)fmt = *(GXDWORD*)"BGRA";    *nDepth = 8;    break;
  case GXFMT_X8R8G8B8:      *(GXDWORD*)fmt = *(GXDWORD*)"BGRX";    *nDepth = 8;    break;
  case GXFMT_A8:            *(GXDWORD*)fmt = *(GXDWORD*)"A   ";    *nDepth = 8;    break;
  case GXFMT_A8B8G8R8:      *(GXDWORD*)fmt = *(GXDWORD*)"RGBA";    *nDepth = 8;    break;
  case GXFMT_X8B8G8R8:      *(GXDWORD*)fmt = *(GXDWORD*)"RGBX";    *nDepth = 8;    break;
  case GXFMT_R32F:          *(GXDWORD*)fmt = *(GXDWORD*)"R   ";    *nDepth = 32;    break;
  case GXFMT_G32R32F:       *(GXDWORD*)fmt = *(GXDWORD*)"RG  ";    *nDepth = 32;    break;
  case GXFMT_A32B32G32R32F: *(GXDWORD*)fmt = *(GXDWORD*)"RGBA";    *nDepth = 32;    break;

  default:
    return FALSE;
  }
  return TRUE;
}
