#include "clstd.h"
#include "clBuffer.H"
#include "clCompress.h"

#include "third_party/zlib/include/zlib.h"

namespace clstd
{
  // compress 和 uncompress 参数使用uLongf, 要保证和包装参数中clsize长度相同或更大
  STATIC_ASSERT(sizeof(clsize) == sizeof(uLongf) || 
    sizeof(clsize) == sizeof(uLongf) * 2);

  //////////////////////////////////////////////////////////////////////////
  clsize CompressBound(clsize cbData)
  {
    return compressBound((uLong)cbData);
  }

  int CompressBuffer(CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen)
  {
    return compress((Bytef*)pDest, (uLongf*)destLen, (const Bytef*)pSource, (uLong)sourceLen);
  }

  int UncompressBuffer(CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen)
  {
    return uncompress((Bytef*)pDest, (uLongf*)destLen, (const Bytef*)pSource, (uLong)sourceLen);
  }

  int CompressBuffer(Buffer* pBuffer, CLLPCVOID ptr, clsize nSize)
  {
    //得到要保存压缩结果的最小buffer大小
    clsize nMinLength = compressBound((uLong)nSize);
    pBuffer->Resize(nMinLength, FALSE);
    int result = compress((Bytef*)pBuffer->GetPtr(),
      (uLongf*)&nMinLength, (Bytef*)ptr, (uLong)nSize);

    if (result == Z_OK) {
      pBuffer->Resize(nMinLength, FALSE);
    }
    return result;
  }

  Buffer* CompressBuffer(CLLPCVOID ptr, clsize nSize)
  {
    Buffer* pBuffer = new Buffer(16);
    int result = CompressBuffer(pBuffer, ptr, nSize);
    if(result != Z_OK) {
      delete pBuffer;
      pBuffer = NULL;
    }
    return pBuffer;
  }

  Buffer* CompressBuffer(BufferBase* pSourceBuffer)
  {
    return CompressBuffer(pSourceBuffer->GetPtr(), pSourceBuffer->GetSize());
  }

  FixedBuffer* UncompressBuffer(BufferBase* pSourceBuffer, clsize nDestSize)
  {
    int err = Z_OK;
    FixedBuffer* pBuffer = new FixedBuffer(nDestSize);

    //pBuffer->Resize(nDestSize, FALSE);
    err = uncompress((Bytef*)pBuffer->GetPtr(), (uLongf*)&nDestSize, 
      (const Bytef*)pSourceBuffer->GetPtr(), (uLong)pSourceBuffer->GetSize());

    if(err != Z_OK) {
      delete pBuffer;
      pBuffer = NULL;
    }
    return pBuffer;
  }
} // namespace clstd
