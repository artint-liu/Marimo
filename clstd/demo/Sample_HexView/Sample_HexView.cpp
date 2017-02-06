// Sample_FileView.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include <clstd.h>
#include <clUtility.h>
#include <clFile.h>

#pragma comment(lib, "clstd_d.lib")

int main(int argc, char* argv[])
{
  if(argc != 2) {
    printf("HexView <filename>");
    return -1;
  }

  clstd::File file;
  if( ! file.OpenExisting(argv[1])) {
    CLOG_ERROR("can not open file(%s).", argv[1]);
    return -2;
  }

  const size_t cbFileSizeLimit = 100 * 1024 * 1024;
  u32 nFileSize = 0;
  if(file.GetSize(&nFileSize) > cbFileSizeLimit) {
    CLOG_ERROR("file is too large to show(size:%l).", ((u64)nFileSize << 32) | file.GetSize(NULL));
    return -3;
  }

  const size_t cbViewSize = 512;
  clstd::MemBuffer buf;
  if(file.ReadToBuffer(&buf, 0, cbViewSize)) {
    clstd::MemBuffer HexBuffer;
    HexBuffer.Resize(clstd::ViewMemory16(NULL, 0, buf.GetPtr(), cbViewSize, buf.GetPtr()), FALSE);
    clstd::ViewMemory16((ch*)HexBuffer.GetPtr(), HexBuffer.GetSize(), buf.GetPtr(), cbViewSize, buf.GetPtr());
    printf((ch*)HexBuffer.GetPtr());
  }
	return 0;
}

