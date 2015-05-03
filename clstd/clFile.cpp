#include <string>

#include "clStd.H"
#include "clString.H"
#include "clmemory.h"
#include "clFile.H"
#include "clBuffer.H"
#include "clUtility.H"

#pragma warning(disable:4355)

namespace clstd
{
#if defined(_WINDOWS) || defined(_WIN32)

  STATIC_ASSERT(File::DA_Read  == GENERIC_READ);
  STATIC_ASSERT(File::DA_Write  == GENERIC_WRITE);

  STATIC_ASSERT(File::SM_Delete  == FILE_SHARE_DELETE);
  STATIC_ASSERT(File::SM_Read  == FILE_SHARE_READ);
  STATIC_ASSERT(File::SM_Write  == FILE_SHARE_WRITE);

  STATIC_ASSERT(File::CD_CreateNew      == CREATE_NEW);
  STATIC_ASSERT(File::CD_CreateAlways    == CREATE_ALWAYS);
  STATIC_ASSERT(File::CD_OpenExisting    == OPEN_EXISTING);
  STATIC_ASSERT(File::CD_OpenAlways      == OPEN_ALWAYS);
  STATIC_ASSERT(File::CD_TruncateExisting  == TRUNCATE_EXISTING);

  STATIC_ASSERT(File::A_Archive    == FILE_ATTRIBUTE_ARCHIVE);
  STATIC_ASSERT(File::A_Compressed  == FILE_ATTRIBUTE_COMPRESSED);
  STATIC_ASSERT(File::A_Hidden    == FILE_ATTRIBUTE_HIDDEN);
  STATIC_ASSERT(File::A_Normal    == FILE_ATTRIBUTE_NORMAL);
  STATIC_ASSERT(File::A_Offline    == FILE_ATTRIBUTE_OFFLINE);
  STATIC_ASSERT(File::A_ReadOnly  == FILE_ATTRIBUTE_READONLY);
  STATIC_ASSERT(File::A_System    == FILE_ATTRIBUTE_SYSTEM);
  STATIC_ASSERT(File::A_Temporary  == FILE_ATTRIBUTE_TEMPORARY);

  STATIC_ASSERT(FILEATTRIBUTE_READONLY   == FILE_ATTRIBUTE_READONLY);
  STATIC_ASSERT(FILEATTRIBUTE_HIDDEN     == FILE_ATTRIBUTE_HIDDEN);
  STATIC_ASSERT(FILEATTRIBUTE_SYSTEM     == FILE_ATTRIBUTE_SYSTEM);
  STATIC_ASSERT(FILEATTRIBUTE_DIRECTORY  == FILE_ATTRIBUTE_DIRECTORY);

#endif // #if defined(_WINDOWS) || defined(_WIN32)

  void CreateFileCreationDistributionToStdOpenCode(
    File::CreationDistribution eCreationDistribution, 
    File::DesiredAccess eDesiredAccess, 
    ch* szOpenMode, s32 nLength)
  {
    memset(szOpenMode, 0, nLength);

    switch(eCreationDistribution)
    {
    case File::CD_CreateNew:
      ASSERT(0);
      break;
    case File::CD_CreateAlways:
      szOpenMode[0] = 'w';
      szOpenMode[1] = 'b';
      if(eDesiredAccess & File::DA_Read)
        szOpenMode[2] = '+';
      break;
    case File::CD_OpenExisting:
      szOpenMode[0] = 'r';
      szOpenMode[1] = 'b';
      if(eDesiredAccess & File::DA_Write)
        szOpenMode[2] = '+';
      break;
    case File::CD_OpenAlways:
      ASSERT(0);
      break;
    case File::CD_TruncateExisting:
      ASSERT(0);
      break;
    }
  }

  File::File()
    : m_hFile(NULL)
    //, m_TextStream(this)
  {
  }

  File::~File()
  {
    Close();
  }

  b32 File::OpenExistingA(CLLPCSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileA(pszFileName, DA_Read, SM_Read, CD_OpenExisting, A_Archive);
    return bval;
  }

  b32 File::CreateAlwaysA(CLLPCSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileA(pszFileName, DA_Write, SM_Read, CD_CreateAlways, A_Archive);
    return bval;
  }

  b32 File::CreateNewA(CLLPCSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileA(pszFileName, DA_Write, SM_Read, CD_CreateNew, A_Archive);
    return bval;
  }

  b32 File::CreateFileA(
    CLLPCSTR        pszFileName, 
    DesiredAccess      eDesiredAccess, 
    ShareMode        eShareMode, 
    CreationDistribution  eCreationDistribution, 
    FlagsAndAttributes    eFlagAttr)
  {
    Close();
#ifdef _WINDOWS
    m_hFile = ::CreateFileA(pszFileName, eDesiredAccess, eShareMode, NULL, 
      eCreationDistribution, eFlagAttr, NULL);
    return (m_hFile != INVALID_HANDLE_VALUE);
#else
    // TODO: ���û���Թ�
    ch szOpenMode[8];
    CreateFileCreationDistributionToStdOpenCode(
      eCreationDistribution, eDesiredAccess, szOpenMode, sizeof(szOpenMode));
    m_hFile = fopen(pszFileName, szOpenMode);
    return (m_hFile != NULL);
#endif // _WINDOWS

  }
  b32 File::OpenExistingW(CLLPCWSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileW(pszFileName, DA_Read, SM_Read, CD_OpenExisting, A_Archive);
    return bval;
  }

  b32 File::CreateAlwaysW(CLLPCWSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileW(pszFileName, DA_Write, SM_Read, CD_CreateAlways, A_Archive);
    return bval;
  }

  b32 File::CreateNewW(CLLPCWSTR pszFileName)
  {
    Close();
    const b32 bval = CreateFileW(pszFileName, DA_Write, SM_Read, CD_CreateNew, A_Archive);
    return bval;
  }

  b32 File::CreateFileW(
    CLLPCWSTR        pszFileName, 
    DesiredAccess      eDesiredAccess, 
    ShareMode        eShareMode, 
    CreationDistribution  eCreationDistribution, 
    FlagsAndAttributes    eFlagAttr)
  {
    Close();
#ifdef _WINDOWS
    m_hFile = ::CreateFileW(pszFileName, eDesiredAccess, eShareMode, NULL, 
      eCreationDistribution, eFlagAttr, NULL);
    return (m_hFile != INVALID_HANDLE_VALUE);
#else
    // TODO: ���û���Թ�
    ch szOpenMode[8];
    CreateFileCreationDistributionToStdOpenCode(
      eCreationDistribution, eDesiredAccess, szOpenMode, sizeof(szOpenMode));

    clStringA strFile(pszFileName);
    m_hFile = fopen(strFile, szOpenMode);
    return (m_hFile != NULL);
#endif // _WINDOWS
  }

  void File::Close()
  {
    if(m_hFile != NULL)
    {
#ifdef _WINDOWS
      CloseHandle(m_hFile);
#else
      fclose(m_hFile);
#endif // _WINDOWS
      m_hFile = NULL;
    }
  }

  u32 File::GetPointer()
  {
    return SetPointer(0, 1);
  }

  u32 File::SetPointer(u32 uMove, u32 uMode)
  {
#ifdef _WINDOWS
    return ::SetFilePointer(m_hFile, uMove, NULL, uMode);
#else
    return fseek(m_hFile, uMove, uMode);
    ASSERT(0);
#endif // #ifdef _WINDOWS
  }

  u64 File::GetPointer64()
  {
    return SetPointer64(0, 1);
  }

  u64 File::SetPointer64(u64 uMove, u32 uMode)
  {
#ifdef _WINDOWS
    union {
      struct {
        LONG lLow;
        LONG lHigh;
      };
      u64 v;
    }d;
    d.v = uMove;
    d.lLow = ::SetFilePointer(m_hFile, d.lLow, &d.lHigh, uMode);
    return d.v;
#else
    return fseek(m_hFile, (long)uMove, uMode);
#endif // #ifdef _WINDOWS
  }


  u32 File::GetSize(u32* pdwFileSizeHight)
  {
#ifdef _WINDOWS
    return ::GetFileSize(m_hFile, (LPDWORD)pdwFileSizeHight);
#else
    u32 dwLength;
    long current = ftell(m_hFile);
    fseek(m_hFile, 0, SEEK_END);
    dwLength = ftell(m_hFile);
    fseek(m_hFile, current, SEEK_SET);
    if(pdwFileSizeHight != NULL)
      *pdwFileSizeHight = 0;
    return dwLength;
#endif
  }

  void File::GetTime(TIME* lpCreationTime, TIME* lpLastAccessTime, TIME* lpLastWriteTime)
  {
#ifdef _WINDOWS
    FILETIME sCreationTime;
    FILETIME sLastAccessTime;
    FILETIME sLastWriteTime;
    GetFileTime(m_hFile, &sCreationTime, &sLastAccessTime, &sLastWriteTime);
    if(lpCreationTime)
    {
      lpCreationTime->dwHighDateTime = sCreationTime.dwHighDateTime;
      lpCreationTime->dwLowDateTime  = sCreationTime.dwLowDateTime;
    }
    if(lpLastAccessTime)
    {
      lpLastAccessTime->dwHighDateTime = sLastAccessTime.dwHighDateTime;
      lpLastAccessTime->dwLowDateTime  = sLastAccessTime.dwLowDateTime;
    }
    if(lpLastWriteTime)
    {
      lpLastWriteTime->dwHighDateTime = sLastWriteTime.dwHighDateTime;
      lpLastWriteTime->dwLowDateTime  = sLastWriteTime.dwLowDateTime;
    }
#else
#error not implement!
#endif // #ifdef _WINDOWS
  }

  b32 File::Read(CLLPVOID lpBuffer, u32 nNumOfBytesToRead,  u32* lpNumberOfBytesRead)
  {
    u32 dwNumRead;
#ifdef _WINDOWS
    const b32 bRet = ::ReadFile(m_hFile, lpBuffer, nNumOfBytesToRead, (LPDWORD)&dwNumRead, NULL);
#else
    dwNumRead = fread(lpBuffer, nNumOfBytesToRead, 1, m_hFile);
    ASSERT(dwNumRead == 1); // ������ֶ�����˵����������
    dwNumRead = nNumOfBytesToRead;
    const b32 bRet = (dwNumRead != 0);
#endif // _WINDOWS
    if(lpNumberOfBytesRead != NULL)
      *lpNumberOfBytesRead = dwNumRead;
    return bRet;
  }

  b32 File::Write(CLLPCVOID lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten)
  {
    u32 dwNumWrite;
#ifdef _WINDOWS
    const b32 bRet = ::WriteFile(m_hFile, lpBuffer, nNumberOfBytesToWrite, (LPDWORD)&dwNumWrite, NULL);
#else
    dwNumWrite = fwrite(lpBuffer, nNumberOfBytesToWrite, 1, m_hFile);
    const b32 bRet = (dwNumWrite != 0);
#endif // _WINDOWS
    if(lpNumberOfBytesWritten != NULL)
      *lpNumberOfBytesWritten = dwNumWrite;
    return bRet;
  }

  //int File::Writef(const tch* format, ...)
  //{
  //  va_list arglist;

  //  va_start(arglist, format);
  //  
  //  u32 dwNumWrite;
  //  clString buffer;
  //  buffer.VarFormat(format, arglist);
  //  Write(buffer, (u32)buffer.GetLength(), &dwNumWrite);

  //  va_end(arglist);
  //  return (int)dwNumWrite;
  //}


  int File::WritefA(const ch* format, ...)
  {
    va_list arglist;

    va_start(arglist, format);

    u32 dwNumWrite;
    clStringA buffer;
    buffer.VarFormat(format, arglist);
    Write(buffer, (u32)buffer.GetLength(), &dwNumWrite);

    va_end(arglist);
    return (int)dwNumWrite;
  }

  int File::WritefW(const wch* format, ...)
  {
    va_list arglist;

    va_start(arglist, format);

    u32 dwNumWrite;
    clStringW buffer;
    buffer.VarFormat(format, arglist);
    Write(buffer, (u32)buffer.GetLength(), &dwNumWrite);

    va_end(arglist);
    return (int)dwNumWrite;
  }


  b32 File::MapToBuffer(CLBYTE** ppBuffer, int nFileOffset, int cbSize, u32* pcbSize)
  {
    u32 dwSizeLow, dwSizeHigh;
    CLBYTE* pBuffer = NULL;

    if(m_hFile == NULL || ppBuffer == NULL || pcbSize == NULL)
      goto FALSE_RET;

    if(cbSize < 0) {  // ���Ϊ��, ��˵��(u32)cbSize > 2GB
      goto FALSE_RET;
    }

    dwSizeLow = GetSize(&dwSizeHigh);

    // ���ӳ��ȫ���ļ����ļ����� 2GB,�򷵻� FALSE
    if(cbSize == 0 && (dwSizeHigh != 0 && dwSizeLow > 0x80000000))
      goto FALSE_RET;

    if(dwSizeLow == 0)
      goto FALSE_RET;

    cbSize = cbSize == 0 ? dwSizeLow :cbSize;

    pBuffer = new CLBYTE[cbSize];
    if(pBuffer == NULL)
      goto FALSE_RET;

    if(Read(pBuffer, cbSize, pcbSize) == FALSE)
    {
      SAFE_DELETE_ARRAY(pBuffer);
      goto FALSE_RET;
    }
    *ppBuffer = pBuffer;
    //*pdwSize = dwSizeLow;
    return TRUE;

FALSE_RET:
    if(pBuffer != NULL) {
      *ppBuffer = NULL;
    }
    if(pcbSize != NULL) {
      *pcbSize = 0;
    }
    return FALSE;
  }

  b32 File::MapToBuffer(clBuffer** ppBuffer, int nFileOffset, int cbSize)
  {
    CLBYTE*   pBuffer    = NULL;
    u32       dwSize     = 0;
    clBuffer* pBufferObj = NULL;

    if(MapToBuffer(&pBuffer, nFileOffset, cbSize, &dwSize) == FALSE || ppBuffer == NULL)
      return FALSE;

    pBufferObj = new clBuffer;
    pBufferObj->Append(pBuffer, dwSize);

    *ppBuffer = pBufferObj;
    SAFE_DELETE(pBuffer);
    dwSize = 0;
    return TRUE;
  }

  b32 File::ReadToBuffer(clBuffer* pBuffer, int nFileOffset, int cbSize)
  {
    u32 dwSizeHigh;
    const i32 nReadSize = cbSize == 0 ? GetSize(&dwSizeHigh) 
      : clMin((i32)(GetSize(&dwSizeHigh) - (u32)nFileOffset), cbSize);

    // return false if file size great than 2GB.
    if(nReadSize <= 0 || dwSizeHigh != 0) {
      return FALSE;
    }

    u32 nNumBytes = 0;
    pBuffer->Resize(nReadSize, FALSE);

    return Read(pBuffer->GetPtr(), nReadSize, &nNumBytes) && (nReadSize == nNumBytes);
  }

  //File::TextStream& File::GetTextStream()
  //{
  //  return m_TextStream;
  //}

  //File::TextStream::TextStream(File* pFile)
  //  : m_pFile(pFile)
  //{
  //}

  //File::TextStream&  File::TextStream::operator <<(const wch* pszString)
  //{
  //  m_pFile->WritefW(pszString);
  //  return *this;
  //}

  //File::TextStream&  File::TextStream::operator <<(const ch* pszString)
  //{
  //  m_pFile->WritefA(pszString);
  //  return *this;
  //}

  //File::TextStream&  File::TextStream::operator <<(const int nValue)
  //{
  //  m_pFile->WritefW(_T("%d"), nValue);
  //  return *this;
  //}

  //File::TextStream&  File::TextStream::operator <<(const float fValue)
  //{
  //  m_pFile->WritefW(_T("%f"), fValue);
  //  return *this;
  //}

  //File::TextStream&  File::TextStream::operator <<(const clString& str)
  //{
  //  m_pFile->WritefW(str);
  //  return *this;
  //}
  //////////////////////////////////////////////////////////////////////////
#if defined(_WINDOWS) || defined(_WIN32)
  FindFile::FindFile()
    : hFind(INVALID_HANDLE_VALUE)
  {
    InlSetZeroT(wfd);
  }

  FindFile::FindFile(CLLPCSTR szFilename)
  {
    NewFindA(szFilename);
  }

  FindFile::FindFile(CLLPCWSTR szFilename)
  {
    NewFindW(szFilename);
  }

  b32 FindFile::NewFindW(CLLPCWSTR szFilename)
  {
    if(hFind != INVALID_HANDLE_VALUE) {
      FindClose(hFind);
    }
    hFind = FindFirstFileW(szFilename, &wfd);
    return (hFind != INVALID_HANDLE_VALUE);
  }

  b32 FindFile::NewFindA(CLLPCSTR szFilename)
  {
    clStringW strFilename = szFilename;
    return NewFindW(strFilename);
  }

  b32 FindFile::GetFileW(FINDFILEDATAW* FindFileData)
  {
    if(hFind == INVALID_HANDLE_VALUE) {
      return FALSE;
    }
    clstd::strcpyn(FindFileData->Filename, wfd.cFileName, MAX_PATH);
    FindFileData->dwAttributes  = wfd.dwFileAttributes;
    FindFileData->nFileSizeHigh = wfd.nFileSizeHigh;
    FindFileData->nFileSizeLow  = wfd.nFileSizeLow;
    if( ! FindNextFile(hFind, &wfd))
    {
      FindClose(hFind);
      hFind = INVALID_HANDLE_VALUE;
    }
    return TRUE;
  }

  b32 FindFile::GetFileA(FINDFILEDATAA* FindFileData)
  {
    FINDFILEDATAW ffdW;
    const b32 bval = GetFileW(&ffdW);
    WideCharToMultiByte(CP_ACP, NULL, ffdW.Filename, -1, FindFileData->Filename, MAX_PATH, NULL, NULL);
    FindFileData->dwAttributes  = ffdW.dwAttributes;
    FindFileData->nFileSizeHigh = ffdW.nFileSizeHigh;
    FindFileData->nFileSizeLow  = ffdW.nFileSizeLow;
    return bval;
  }

#else
#endif // #if defined(_WINDOWS) || defined(_WIN32)
} // namespace clstd