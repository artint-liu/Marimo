#ifndef _CLSTD_COMPRESS_H_
#define _CLSTD_COMPRESS_H_

namespace clstd
{
  //b32 Compress(PROCESSBUFFER* pProcessBuffer);
  //b32 Uncompress(PROCESSBUFFER* pProcessBuffer);
  clsize        CompressBound    (clsize cbData);

  int           CompressBuffer   (CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen);
  int           UncompressBuffer (CLLPVOID pDest, clsize* destLen, CLLPCVOID pSource, clsize sourceLen);

  int           CompressBuffer   (clBuffer* pBuffer, CLLPCVOID ptr, clsize nSize);
  clBuffer*     CompressBuffer   (CLLPCVOID ptr, clsize nSize);
  clBuffer*     CompressBuffer   (clBufferBase* pSourceBuffer);

  FixedBuffer*  UncompressBuffer (clBufferBase* pSourceBuffer, clsize nDestSize);  // nDestSize ��ѹ��ĳ���, ���������ʵ�ʳ��Ȳ���, ��ʧ��.
} // namespace clstd


#endif // #ifndef _CLSTD_COMPRESS_H_