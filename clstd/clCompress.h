#ifndef _CLSTD_COMPRESS_H_
#define _CLSTD_COMPRESS_H_

namespace clstd
{
  //b32 Compress(PROCESSBUFFER* pProcessBuffer);
  //b32 Uncompress(PROCESSBUFFER* pProcessBuffer);
  clsize        CompressBound    (clsize cbData);

  int           CompressBuffer   (CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen);
  int           UncompressBuffer (CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen);

  int           CompressBuffer   (Buffer* pBuffer, CLLPCVOID ptr, clsize nSize);
  Buffer*     CompressBuffer   (CLLPCVOID ptr, clsize nSize);
  Buffer*     CompressBuffer   (BufferBase* pSourceBuffer);

  FixedBuffer*  UncompressBuffer (BufferBase* pSourceBuffer, clsize nDestSize);  // nDestSize 解压后的长度, 如果与数据实际长度不符, 会失败.
} // namespace clstd


#endif // #ifndef _CLSTD_COMPRESS_H_