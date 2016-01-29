#include "../clstd.h"
#include "../clString.H"
#include "../clFile.H"
#include "SmartRepository.h"
#include "../clUtility.H"
#include "../clBuffer.H"
#include "../clCompress.h"

// SSDB = Simple Storage DataBase
// SZDB = Smart Zlib-compressed DataBase
#define ROOT_NAME "<Root>"
#define SSDB_MAGIC CLMAKEFOURCC('S','S','D','B')  // 普通文件头
#define SZDB_MAGIC CLMAKEFOURCC('S','Z','D','B')  // 带有zlib压缩的头
#define ZNODECC    CLMAKEFOURCC('Z','L','I','B')  // 节点 - 使用的压缩算法标记

namespace clstd
{
  typedef SmartRepository::SmartNode SmartNode;     // 只在源文件中有效,为了方便写代码

  struct ZHEADER
  {
    u32 dwZCode;  // 压缩算法标志
    u32 nSize;    // 压缩前数据的长度
  };

#ifdef ENABLE_HASH_NAME
  STATIC_ASSERT(sizeof(SmartRepository::KEYDESC) == sizeof(u32) * 3);
#else
  STATIC_ASSERT(sizeof(SmartRepository::KEYDESC) == sizeof(u32) * 3);
#endif // #ifdef ENABLE_HASH_NAME

  //////////////////////////////////////////////////////////////////////////
  //[Node数据结构]:
  //  [HEADER]              记录标志头和有多少数据键
  //  [KEYDESC + KeyName]   非Node类型的键1
  //  [KEYDESC + KeyName]   非Node类型的键2
  //  ...
  //  [KEYDESC + KeyName]   非Node类型的键n
  //
  //  [KEYDESC + KeyName]   Node类型的键1
  //    [Node数据结构]
  //  [KEYDESC + KeyName]   Node类型的键2
  //    [Node数据结构]
  //  ...
  //  [KEYDESC + KeyName]   Node类型的键n
  //    [Node数据结构]
  //
  //  [Buffer]              Variable类型的数据池
  //////////////////////////////////////////////////////////////////////////

  SmartRepository::SmartRepository()
    : pBuffer(NULL)
  {
  }

  SmartRepository::~SmartRepository()
  {
    // 析构不做保存动作, 因为文件可能是读取
    ClearRecursive(this);
  }

  b32 SmartRepository::ClearRecursive(SmartNode* pSmart)
  {
    b32 bval = TRUE;
    for(KeyDict::iterator it = pSmart->Keys.begin();
      it != pSmart->Keys.end(); ++it) {
        if(it->second.eType == KT_Node) {
          bval = ClearRecursive(it->second.buf.pSmart);
          ASSERT(it->second.buf.pSmart->pBuffer == NULL);
          delete it->second.buf.pSmart;
          it->second.buf.pSmart = NULL;
        }
    }
    SAFE_DELETE(pSmart->pBuffer);
    pSmart->Keys.clear();
    return TRUE;
  }

  b32 SmartRepository::LoadA(CLLPCSTR szFile)
  {
    clStringW strFile = szFile;
    return LoadW(strFile);
  }

  b32 SmartRepository::SaveA(CLLPCSTR szFile)
  {
    clStringW strFile = szFile;
    return SaveW(strFile);
  }

  b32 SmartRepository::LoadW(CLLPCWSTR szFile)
  {
    ClearRecursive(this);
    clFile file;
    if( ! file.OpenExistingW(szFile)) {
      // 文件打开失败通过返回值体现,返回值已经足够体现错误信息,这里不输出LOG.
      return FALSE;
    }
    return LoadRecursive(this, ROOT_NAME, file);
  }

  b32 SmartRepository::SaveW(CLLPCWSTR szFile)
  {
    clFile file;
    if( ! file.CreateAlwaysW(szFile)) {
      return FALSE; // 返回值已经足够体现错误信息,这里不输出LOG.
    }
    return SaveRecursive(this, ROOT_NAME, file);
  }

  b32 SmartRepository::LoadSubNodeA(CLLPCSTR szFile, CLLPCSTR szKeys)
  {
    clStringW strFile = szFile;
    return LoadSubNodeW(strFile, szKeys);
  }

  b32 SmartRepository::LoadSubNodeW(CLLPCWSTR szFile, CLLPCSTR szKeys)
  {
    CLBREAK; // 没实现呢!
    return FALSE;
  }

  b32 SmartRepository::LoadRecursive(SmartNode* pSmart, CLLPCSTR szSmartNodeName, clFile& file)
  {
    HEADER Header;
    if( ! file.Read(&Header, sizeof(HEADER))) {
      CLOG_ERROR("%s: Can not read data header(%s).\n", __FUNCTION__, szSmartNodeName);
      return FALSE;
    }
    if(Header.dwMagic != SSDB_MAGIC && Header.dwMagic != SZDB_MAGIC) {
      CLOG_ERROR("%s: Header sign does not match(%s).\n", __FUNCTION__, szSmartNodeName);
      return FALSE;
    }

    u32 nDataLength = 0;
    for(u32 i = 0; i < Header.dwNumKeys; i++)
    {
      KEYDESC KeyDesc;
      ch szNameBuf[256];
      if( ! file.Read(&KeyDesc, sizeof(KEYDESC))) {
        CLOG_ERROR("%s: Can not read key-desc(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }

      if(KeyDesc.nKeyLength == 0) {
        CLOG_ERROR("%s: Bad key name length(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }

      if( ! file.Read(szNameBuf, KeyDesc.nKeyLength)) {
        CLOG_ERROR("%s: Can not read key name(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }

      szNameBuf[KeyDesc.nKeyLength] = '\0'; // 安全结尾

      if(KeyDesc.eType == KT_Node) {
        KeyDesc.buf.pSmart = new SmartNode;
        if( ! LoadRecursive(KeyDesc.buf.pSmart, szNameBuf, file)) {
          CLOG_ERROR("%s: Failed to load key-SmartNode[%s(%s)].\n", __FUNCTION__, szSmartNodeName, szNameBuf);
          return FALSE;
        }
      }
      else if(KeyDesc.eType == KT_Varible) {
        nDataLength += KeyDesc.v.cbSize;
      }

      pSmart->Keys[szNameBuf] = KeyDesc;
    }

    ASSERT(pSmart->pBuffer == NULL);

    if(nDataLength == 0) {
      return TRUE;
    }

    // 最后最后,读出Variable用的Buffer
    if(Header.dwMagic == SZDB_MAGIC)
    {
      ZHEADER ZHeader;
      if( ! file.Read(&ZHeader, sizeof(ZHEADER))) {
        CLOG_ERROR("%s: Can not read Z-Header data(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }

      if(ZHeader.dwZCode != ZNODECC || ZHeader.nSize == 0) {
        CLOG_ERROR("%s: Bad Z-Header data(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }

      clFixedBuffer* pZBuf = new clFixedBuffer(ZHeader.nSize);
      if( ! file.Read(pZBuf->GetPtr(), (u32)pZBuf->GetSize())) {
        CLOG_ERROR("%s: Can not load compressed data(%s).\n", __FUNCTION__, szSmartNodeName);
        SAFE_DELETE(pZBuf);
        return FALSE;
      }

      clFixedBuffer* pDataBuf = clstd::UncompressBuffer(pZBuf, nDataLength);
      SAFE_DELETE(pZBuf); // 不再用了, 直接释放

      if(pDataBuf == NULL)
      {
        CLOG_ERROR("%s: Failed to uncompressed data, "
          "this may have wrong data-length or bad compressed data(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }
      pSmart->m_bCompressed = TRUE;
      pSmart->pBuffer = new Buffer;
      pSmart->pBuffer->Append(pDataBuf->GetPtr(), pDataBuf->GetSize());
      SAFE_DELETE(pDataBuf);
    }
    else
    {
      ASSERT(Header.dwMagic == SSDB_MAGIC);
      pSmart->pBuffer = new Buffer;
      pSmart->pBuffer->Resize(nDataLength, FALSE);
      if( ! file.Read(pSmart->pBuffer->GetPtr(), (u32)pSmart->pBuffer->GetSize())) {
        CLOG_ERROR("%s: Can not load buffer data(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }
    }
    return TRUE;
  }

  b32 SmartRepository::SaveRecursive(SmartNode* pSmart, CLLPCSTR szSmartNodeName, clFile& file)
  {
    HEADER Header;
    Header.dwMagic   = m_bCompressed ? SZDB_MAGIC : SSDB_MAGIC;
    Header.dwNumKeys = (u32)pSmart->Keys.size();
    if( ! file.Write(&Header, sizeof(HEADER))) {
      CLOG_ERROR("%s: Can not write data header(%s).\n", __FUNCTION__, szSmartNodeName);
      return FALSE;
    }

    // 第一次写入所有非SmartNode数据
    u32 nDataLength = 0;
    for(KeyDict::iterator it = pSmart->Keys.begin();
      it != pSmart->Keys.end(); ++it) {
        // 先跳过SmartNode类型的数据
        if(it->second.eType == KT_Node) {
          continue;
        }

        // 写入Key
        b32 bval = file.Write(&(it->second), sizeof(KEYDESC));
        if(it->first.IsNotEmpty()) {
          bval = bval && file.Write((it->first).GetBuffer(), (u32)(it->first).GetLength());
        }
        if( ! bval) {
          CLOG_ERROR("%s: Can not write key-desc(%s).\n", __FUNCTION__, szSmartNodeName);
          return FALSE;
        }
        if(it->second.eType == KT_Varible) {
          nDataLength += it->second.v.cbSize; // 只有Variable才需要记录长度,这个要和Buffer的记录相符!
        }
    }

    // 检查变长数据的记录长度和Buffer的长度
    if((nDataLength == 0 && pSmart->pBuffer != NULL) || 
      (nDataLength != 0 && pSmart->pBuffer == NULL) || 
      (pSmart->pBuffer != NULL && nDataLength != pSmart->pBuffer->GetSize()))
    {
      CLOG_ERROR("%s: Keys record length does not match the buffer size(%s).\n", __FUNCTION__, szSmartNodeName);
      return FALSE;
    }

    // 第二阶段写入SmartNode数据
    for(KeyDict::iterator it = pSmart->Keys.begin();
      it != pSmart->Keys.end(); ++it) {
        if(it->second.eType != KT_Node) {
          continue;
        }
        KEYDESC SmartNodeKey = it->second;
        SmartNodeKey.v.nOffset = CLMAKEFOURCC('G','R','U','P'); // 这两个暂时没用上
        SmartNodeKey.v.cbSize  = CLMAKEFOURCC('E','M','P','Y');
        // 写入Key
        b32 bval = file.Write(&SmartNodeKey, sizeof(KEYDESC));
        bval = bval && file.Write((it->first).GetBuffer(), (u32)(it->first).GetLength());
        if( ! bval) {
          CLOG_ERROR("%s: Can not write key-desc(%s).\n", __FUNCTION__, szSmartNodeName);
          return FALSE;
        }

        SmartNode* pChildSmartNode = it->second.buf.pSmart;
        if(pChildSmartNode->pBuffer != NULL && pChildSmartNode->pBuffer->GetSize() > 0) {
          if( ! SaveRecursive(pChildSmartNode, it->first, file)) {
            CLOG_ERROR("%s: Failed to write SmartNode[%s(%s)].", __FUNCTION__,
              szSmartNodeName, (CLLPCSTR)static_cast<clStringA>(it->first));
            return FALSE;
          }
        }
        else {
          CLOG_WARNING("%s: There is an empty SmartNode[%s(%s)].\n", __FUNCTION__,
            szSmartNodeName, (CLLPCSTR)(it->first));
        }
    }

    // 全是 KT_Octet 类型键值可能会没有 Buffer 数据.
    if(pSmart->pBuffer == NULL || pSmart->pBuffer->GetSize() == 0) {
      return TRUE;
    }

    // 数据buffer放在最后面
    if(m_bCompressed)
    {
      b32 bval = TRUE;
      Buffer* pZBuf = clstd::CompressBuffer(pSmart->pBuffer);
      if(pZBuf == NULL || pZBuf->GetSize() == 0) {
        CLOG_ERROR("%s: Failed to compress buffer(%s).\n", __FUNCTION__, szSmartNodeName);
        bval = FALSE;
      }
      else
      {
        ZHEADER ZHeader;
        ZHeader.dwZCode = ZNODECC;
        ZHeader.nSize   = (u32)pZBuf->GetSize();

        if( ! file.Write(&ZHeader, sizeof(ZHEADER)) ||
          ! file.Write(pZBuf->GetPtr(), (u32)pZBuf->GetSize())) {
            CLOG_ERROR("%s: Can not write compressed buffer data(%s).\n", __FUNCTION__, szSmartNodeName);
            bval = FALSE;
        }
      }

      SAFE_DELETE(pZBuf);
      if( ! bval) {
        return bval;
      }
    }
    else
    {
      if( ! file.Write(pSmart->pBuffer->GetPtr(), (u32)pSmart->pBuffer->GetSize()) ) {
        CLOG_ERROR("%s: Can not write buffer data(%s).\n", __FUNCTION__, szSmartNodeName);
        return FALSE;
      }
    }
    return TRUE;
  }

  SmartNode* SmartRepository::CreateNode(CLLPCSTR szKeys)
  {
    clStringArrayA aPathKeys;
    ResolveString<clStringA>(szKeys, '\\', aPathKeys);

    // 检查键值不能大于255字节
    for(clStringArrayA::iterator itKeyName = aPathKeys.begin();
      itKeyName != aPathKeys.end(); ++itKeyName) {
        if(itKeyName->GetLength() > 255 || itKeyName->IsEmpty()) {
          CLOG_ERROR("%s: Key name(%s) is too long.\n", __FUNCTION__, (CLLPCSTR)(*itKeyName));
          return FALSE;
        }
    }

    SmartNode* pSmart = &*this;
    for(clStringArrayA::iterator itKeyName = aPathKeys.begin();
      itKeyName != aPathKeys.end(); ++itKeyName)
    {
      KeyDict::iterator itKeyDesc = pSmart->Keys.find(*itKeyName);
      if(itKeyDesc == pSmart->Keys.end()) {
        KEYDESC KeyDesc;
        KeyDesc.nKeyLength = itKeyName->GetLength();
        KeyDesc.eType  = KT_Node;
        KeyDesc.Flags  = 0;
        KeyDesc.buf.pSmart = new SmartNode;
        //KeyDesc.buf.pSmart->pBuffer = new clBuffer;
        pSmart->Keys[*itKeyName] = KeyDesc;
        pSmart = KeyDesc.buf.pSmart;
      }
      else if(itKeyDesc->second.eType == KT_Node) {
        pSmart = itKeyDesc->second.buf.pSmart;
      }
      else return NULL;
    }
    return pSmart;
  }

  SmartNode* SmartRepository::GetNode(CLLPCSTR szKeys) CLCONST
  {
    clStringArrayA aPathKeys;
    ResolveString<clStringA>(szKeys, '\\', aPathKeys);
    const SmartNode* pSmart = this;
    for(clStringArrayA::iterator itKeyName = aPathKeys.begin();
      itKeyName != aPathKeys.end(); ++itKeyName)
    {
      KeyDict::const_iterator itKeyDesc = pSmart->Keys.find(*itKeyName);
      if(itKeyDesc != pSmart->Keys.end() && itKeyDesc->second.eType == KT_Node) {
        pSmart = itKeyDesc->second.buf.pSmart;
      }
      else return NULL;
    }
    return (SmartNode*)pSmart;
  }

  u32 SmartRepository::SetFlags(u32 dwFlags)
  {
    const u32 uPrev = m_bCompressed;
    m_bCompressed = TEST_FLAG(dwFlags, SRF_COMPRESSED) ? 1 : 0;

    if(TEST_FLAG(dwFlags, SRF_APPLYCHILDNODE))
    {
      for(KeyDict::iterator it = Keys.begin();
        it != Keys.end(); ++it) {
          if(it->second.eType == KT_Node) {
            it->second.buf.pSmart->SetFlags(dwFlags);
          }
      } // for
    }
    return uPrev;
  }

  s32 SmartRepository::GetLength(CLCONST SmartNode* pSmart, CLLPCSTR szKey) CLCONST
  {
    pSmart = pSmart == NULL ? this : pSmart;
    KeyDict::const_iterator it = pSmart->Keys.find(szKey);
    if(it == pSmart->Keys.end()) {
      return 0;
    }

    if(it->second.eType == KT_Octet) {
      return sizeof(u32) * 2;
    }
    else if(it->second.eType == KT_Varible) {
      return it->second.v.cbSize;
    }
    else if(it->second.eType == KT_Node) {
      return 0; // SmartNode 
    }
    return -1;
  }

  b32 SmartRepository::Write(SmartNode* pSmart, CLLPCSTR szKey, CLLPCVOID lpData, u32 cbSize)
  {
    pSmart = pSmart == NULL ? this : pSmart;
    if(cbSize == 0) {
      return FALSE;
    }
    const clsize nKeyLength = clstd::strlenT(szKey);
    if(/*nKeyLength < 0 || */nKeyLength > 255) {
      CLOG_ERROR("%s: Key name(%s) is too long.", __FUNCTION__, szKey);
    }

    if(pSmart->pBuffer == NULL) {
      pSmart->pBuffer = new Buffer;
    }

    KeyDict::iterator it = pSmart->Keys.find(szKey);
    if(it != pSmart->Keys.end()) {
      // 含有该键值则修改
      KEYDESC& Desc = it->second;
      if(Desc.eType == KT_Varible)
      {
        pSmart->pBuffer->Replace(Desc.v.nOffset, Desc.v.cbSize, lpData, cbSize);
      }
      else if(Desc.eType == KT_Octet)
      {
        ASSERT(0); // FIXME: 验证后去掉
        Desc.eType     = KT_Octet;
        Desc.v.cbSize  = cbSize;
        Desc.v.nOffset = (u32)pSmart->pBuffer->GetSize();
        pSmart->pBuffer->Append(lpData, cbSize);
      }
      else
        return FALSE;
    }
    else
    {
      KEYDESC Desc;
      Desc.nKeyLength = nKeyLength;
      Desc.eType      = KT_Varible;
      Desc.Flags      = 0;
      Desc.v.cbSize   = cbSize;
      Desc.v.nOffset  = (u32)pSmart->pBuffer->GetSize();

      pSmart->pBuffer->Append(lpData, cbSize);
      pSmart->Keys[szKey] = Desc;
    }
    return TRUE;
  }

  s32 SmartRepository::Read(CLCONST SmartNode* pSmart, CLLPCSTR szKey, CLLPVOID lpData, u32 cbSize) CLCONST
  {
    s32 ret = 0;
    pSmart = pSmart == NULL ? this : pSmart;

    KeyDict::const_iterator it = pSmart->Keys.find(szKey);
    if(it == pSmart->Keys.end()) {
      return ret;
    }
    CLCONST KEYDESC& Desc = it->second;
    if(Desc.eType == KT_Varible)
    {
      ret = clMin(Desc.v.cbSize, cbSize);
      memcpy(lpData, (u8*)pSmart->pBuffer->GetPtr() + Desc.v.nOffset, ret);
    }
    else if(Desc.eType == KT_Octet)
    {
      ASSERT(cbSize >= 8); // FIXME: Just copy the specify size!
      *(u32*)lpData = Desc.o.dwLow;
      *((u32*)lpData + 1) = Desc.o.dwHigh;
      ret = 8;
    }
    return ret;
  }

  b32 SmartRepository::Write64(SmartNode* pSmart, CLLPCSTR szKey, u32 dwLow, u32 dwHigh)
  {
    pSmart = pSmart == NULL ? this : pSmart;
    const clsize nKeyLength = clstd::strlenT(szKey);
    if(/*nKeyLength < 0 || */nKeyLength > 255) {
      CLOG_ERROR("%s: Key name(%s) is too long.", __FUNCTION__, szKey);
    }

    KeyDict::iterator itRep = pSmart->Keys.find(szKey);
    if(itRep != pSmart->Keys.end()) {
      // 含有该键值则修改
      KEYDESC& Desc = itRep->second;
      if(Desc.eType == KT_Varible)
      {
        ASSERT(0); // FIXME: 验证后去掉
        // 删除 pBuffer 已有的数据
        pSmart->pBuffer->Replace(Desc.v.nOffset, Desc.v.cbSize, NULL, 0);
        // 重定位表中其他键值的数据
        //const u32 cbDataSize = Desc.v.cbSize;
        for(KeyDict::iterator itLoop = pSmart->Keys.begin(); 
          itLoop != pSmart->Keys.end(); ++itLoop) {
            if(itLoop->second.eType == KT_Varible && itLoop->second.v.nOffset > Desc.v.nOffset) {
              itLoop->second.v.nOffset -= Desc.v.cbSize;
            }
        }
        // 设置为 Octet 数据
        Desc.eType    = KT_Octet;
        Desc.o.dwHigh = dwHigh;
        Desc.o.dwLow  = dwLow;
      }
      else if(Desc.eType == KT_Octet)
      {
        Desc.o.dwHigh = dwHigh;
        Desc.o.dwLow  = dwLow;
      }
      else {
        return FALSE;
      }
    }
    else
    {
      KEYDESC Desc;
      Desc.nKeyLength = nKeyLength;
      Desc.eType      = KT_Octet;
      Desc.Flags      = 0;
      Desc.o.dwLow    = dwLow;
      Desc.o.dwHigh   = dwHigh;

      pSmart->Keys[szKey] = Desc;
    }
    return TRUE;
  }

  b32 SmartRepository::Read64(CLCONST SmartNode* pSmart, CLLPCSTR szKey, u32* dwLow, u32* dwHigh) CLCONST
  {
    pSmart = pSmart == NULL ? this : pSmart;

    KeyDict::const_iterator it = pSmart->Keys.find(szKey);
    if(it == pSmart->Keys.end()) {
      return FALSE;
    }

    if(it->second.eType == KT_Octet) {
      if(dwLow != NULL) {
        *dwLow = it->second.o.dwLow;
      }
      if(dwHigh != NULL) {
        *dwHigh = it->second.o.dwHigh;
      }
      return TRUE;
    }
    return FALSE;
  }

  s32 SmartRepository::ReadToBuffer(CLCONST SmartNode* pSmart, CLLPCSTR szKey, Buffer* pBuffer) CLCONST
  {
    s32 ret = 0;
    pSmart = pSmart == NULL ? this : pSmart;

    KeyDict::const_iterator it = pSmart->Keys.find(szKey);
    if(it == pSmart->Keys.end()) {
      return ret;
    }
    CLCONST KEYDESC& Desc = it->second;
    if(Desc.eType == KT_Varible)
    {
      pBuffer->Resize(Desc.v.cbSize, FALSE);
      ret = Desc.v.cbSize;
      memcpy(pBuffer->GetPtr(), (u8*)pSmart->pBuffer->GetPtr() + Desc.v.nOffset, ret);
    }
    else if(Desc.eType == KT_Octet)
    {
      pBuffer->Resize(8, FALSE);
      *(u32*)pBuffer->GetPtr() = Desc.o.dwLow;
      *((u32*)pBuffer->GetPtr() + 1) = Desc.o.dwHigh;
      ret = 8;
    }
    return ret;

  }

  b32 SmartRepository::WriteStringW(SmartNode* pSmart, CLLPCSTR szKey, CLLPCWSTR szString)
  {
    return Write(pSmart, szKey, szString, (u32)clstd::strlenT(szString) * sizeof(wch));
  }

  b32 SmartRepository::WriteStringA(SmartNode* pSmart, CLLPCSTR szKey, CLLPCSTR  szString)
  {
    return Write(pSmart, szKey, szString, (u32)clstd::strlenT(szString));
  }

  b32 SmartRepository::WriteStringW(SmartNode* pSmart, CLLPCSTR szKey, const clStringW& strString)
  {
    return Write(pSmart, szKey, strString, (u32)strString.GetLength() * sizeof(wch));
  }

  b32 SmartRepository::WriteStringA(SmartNode* pSmart, CLLPCSTR szKey, const clStringA& strString)
  {
    return Write(pSmart, szKey, strString, (u32)strString.GetLength());
  }

  s32 SmartRepository::ReadStringW(SmartNode* pSmart, CLLPCSTR szKey, clStringW& strString)
  {
    s32 nLength = GetLength(pSmart, szKey);
    wch* pBuffer = strString.GetBuffer(nLength + 1);
    s32 val = Read(pSmart, szKey, pBuffer, nLength);
    pBuffer[nLength / sizeof(wch)] = '\0';
    strString.ReleaseBuffer();
    return val;
  }

  s32 SmartRepository::ReadStringA(SmartNode* pSmart, CLLPCSTR szKey, clStringA& strString)
  {
    s32 nLength = GetLength(pSmart, szKey);
    ch* pBuffer = strString.GetBuffer(nLength + 1);
    s32 val = Read(pSmart, szKey, pBuffer, nLength);
    pBuffer[nLength] = '\0';
    strString.ReleaseBuffer();
    return val;
  }

  b32 SmartRepository::WriteNumeric(SmartNode* pSmart, CLLPCSTR szKey, s32 nNum)
  {
    return Write64(pSmart, szKey, nNum, 0);
  }

  s32 SmartRepository::ReadNumeric(SmartNode* pSmart, CLLPCSTR szKey)
  {
    s32 nNum;
    if(Read64(pSmart, szKey, (u32*)&nNum, NULL)) {
      return nNum;
    }
    if(Read(pSmart, szKey, &nNum, sizeof(s32)) == sizeof(s32)) {
      return nNum;
    }
    return 0;
  }
} // using namespace clstd;

//*/