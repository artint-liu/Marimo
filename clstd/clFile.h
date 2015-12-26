#ifndef _CL_FILE_H_
#define _CL_FILE_H_

#define FILEATTRIBUTE_READONLY   0x00000001
#define FILEATTRIBUTE_HIDDEN     0x00000002
#define FILEATTRIBUTE_SYSTEM     0x00000004
#define FILEATTRIBUTE_DIRECTORY  0x00000010

class clBuffer;
namespace clstd
{
  class File
  {
  public:
    enum DesiredAccess
    {
      DA_Query  = 0,          // Specifies device query access to the object. An application can query device attributes without accessing the device.
      DA_Read   = (0x80000000L),    // Specifies read access to the object. Data can be read from the file and the file pointer can be moved. Combine with GENERIC_WRITE for read-write access. 
      DA_Write  = (0x40000000L),    // Specifies write access to the object. Data can be written to the file and the file pointer can be moved. Combine with GENERIC_READ for read-write access. 
    };
    enum ShareMode
    {
      SM_Delete = (0x00000004),      // Windows NT only: Subsequent open operations on the object will succeed only if delete access is requested. 
      SM_Read   = (0x00000001),      // Subsequent open operations on the object will succeed only if read access is requested. 
      SM_Write  = (0x00000002),      // Subsequent open operations on the object will succeed only if write access is requested. 
    };

    enum CreationDistribution
    {
      CD_CreateNew        = 1,    // Creates a new file. The function fails if the specified file already exists.
      CD_CreateAlways     = 2,    // Creates a new file. The function overwrites the file if it exists.
      CD_OpenExisting     = 3,    // Opens the file. The function fails if the file does not exist.
      // See the Remarks section for a discussion of why you should use the OPEN_EXISTING flag if you are using the CreateFile function for devices, including the console.
      CD_OpenAlways       = 4,    // Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDistribution were CREATE_NEW.
      CD_TruncateExisting = 5,    // Opens the file. Once opened, the file is truncated so that its size is zero bytes. The calling process must open the file with at least GENERIC_WRITE access. The function fails if the file does not exist.

    };

    enum FlagsAndAttributes
    {
      A_Archive     = 0x00000020,  // The file should be archived. Applications use this attribute to mark files for backup or removal.
      A_Compressed  = 0x00000800,  // The file or directory is compressed. For a file, this means that all of the data in the file is compressed. For a directory, this means that compression is the default for newly created files and subdirectories.
      A_Hidden      = 0x00000002,  // The file is hidden. It is not to be included in an ordinary directory listing.
      A_Normal      = 0x00000080,  // The file has no other attributes set. This attribute is valid only if used alone.
      A_Offline     = 0x00001000,  // The data of the file is not immediately available. Indicates that the file data has been physically moved to offline storage.
      A_ReadOnly    = 0x00000001,  // The file is read only. Applications can read the file but cannot write to it or delete it.
      A_System      = 0x00000004,  // The file is part of or is used exclusively by the operating system.
      A_Temporary   = 0x00000100,  // The file is being used for temporary storage. File systems attempt to keep all of the data in memory for quicker access rather than flushing the data back to mass storage. A temporary file should be deleted by the application as soon as it is no longer needed.
    };

    struct TIME
    {
      u32 dwLowDateTime; 
      u32 dwHighDateTime; 
    };

    // TODO: 以后改为 FileTextStream ...
    //template<class _TString>
    //class TextStreamT
    //{
    //  friend class File;
    //private:
    //  File*    m_pFile;
    //public:
    //  TextStream(File* pFile);
    //  File::TextStream&  operator <<(const wch*);
    //  File::TextStream&  operator <<(const ch*);
    //  File::TextStream&  operator <<(const int);
    //  File::TextStream&  operator <<(const float);
    //  File::TextStream&  operator <<(const _TString&);
    //};
  private:
#ifdef _WINDOWS
    HANDLE    m_hFile;
#else
    FILE*    m_hFile;
#endif
    //TextStream  m_TextStream;
  public:
    File();
    ~File();

    b32  OpenExistingA  (CLLPCSTR pszFileName);
    b32  CreateAlwaysA  (CLLPCSTR pszFileName);    // Creates a new file. The function overwrites the file if it exists.
    b32  CreateNewA     (CLLPCSTR pszFileName);
    b32  CreateFileA    (CLLPCSTR pszFileName, 
      DesiredAccess eDesiredAccess, 
      ShareMode eShareMode, 
      CreationDistribution eCreationDistribution, 
      FlagsAndAttributes eFlagAttr);

    b32  OpenExistingW  (CLLPCWSTR pszFileName);
    b32  CreateAlwaysW  (CLLPCWSTR pszFileName);
    b32  CreateNewW     (CLLPCWSTR pszFileName);
    b32  CreateFileW    (CLLPCWSTR pszFileName, 
      DesiredAccess eDesiredAccess, 
      ShareMode eShareMode, 
      CreationDistribution eCreationDistribution, 
      FlagsAndAttributes eFlagAttr);

    void  Close       ();

    u32  GetPointer   ();
    u32  SetPointer   (u32 uMove, u32 uMode);
    u64  GetPointer64 ();
    u64  SetPointer64 (u64 uMove, u32 uMode);
    u32  GetSize      (u32* pdwFileSizeHight);
    void GetTime      (TIME* lpCreationTime, TIME* lpLastAccessTime, TIME* lpLastWriteTime);
    b32  Read         (CLLPVOID lpBuffer, u32 nNumOfBytesToRead,  u32* lpNumberOfBytesRead = NULL);
    b32  Write        (CLLPCVOID lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten = NULL);

    //int  Writef       (const tch* format, ...);
    int  WritefA      (const ch* format, ...);
    int  WritefW      (const wch* format, ...);

    b32  ReadToBuffer (clBuffer* pBuffer, int nFileOffset = 0, int cbSize = 0);
    b32  MapToBuffer  (CLBYTE** pBuffer, int nFileOffset, int cbSize, u32* pcbSize); // 从nFileOffset偏移开始读cbSize（0表示读到文件末尾）字节到pBuffer缓冲中，实际读入大小是pcbSize
    b32  MapToBuffer  (clBuffer** ppBuffer, int nFileOffset = 0, int cbSize = 0);
    //TextStream& 
    //    GetTextStream ();  
  };

  //////////////////////////////////////////////////////////////////////////
  struct FINDFILEDATAW
  {
    CLWCHAR Filename[MAX_PATH];
    CLDWORD dwAttributes;
    u32     nFileSizeHigh;
    u32     nFileSizeLow;
  };

  struct FINDFILEDATAA
  {
    CLCHAR  Filename[MAX_PATH];
    CLDWORD dwAttributes;
    u32     nFileSizeHigh;
    u32     nFileSizeLow;
  };

  class FindFile
  {
#if defined(_WINDOWS) || defined(_WIN32)
    HANDLE          hFind;
    WIN32_FIND_DATA wfd;
#else
#endif // #if defined(_WINDOWS) || defined(_WIN32)
  public:
    FindFile();
    FindFile(CLLPCSTR szFilename);
    FindFile(CLLPCWSTR szFilename);

    b32 NewFindW(CLLPCWSTR szFilename);
    b32 NewFindA(CLLPCSTR szFilename);

    b32 GetFileW(FINDFILEDATAW* FindFileData);
    b32 GetFileA(FINDFILEDATAA* FindFileData);
  };

} // namespace clstd

typedef clstd::File clFile;

//#ifdef _UNICODE
//#define OpenExisting  OpenExistingW
//#define CreateAlways  CreateAlwaysW
//#define CreateNew     CreateNewW
//#define CreateFile    CreateFileW
//#else
//#define OpenExisting  OpenExistingA
//#define CreateAlways  CreateAlwaysA
//#define CreateNew     CreateNewA
//#define CreateFile    CreateFileA
//#endif

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CL_FILE_H_