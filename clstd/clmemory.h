#ifndef _CL_MEMORY_MANAGENT_
#define _CL_MEMORY_MANAGENT_

#ifdef _WINDOWS
#pragma warning(disable : 4291)
#endif // #ifdef _WINDOWS

//void* operator new(unsigned int nSize, const char *file, int line);
//void* operator new[](unsigned int nSize, const char *file, int line);
//void* operator new(unsigned int nSize);
//void* operator new[](unsigned int nSize);
//void operator delete(void* p);
//void operator delete[](void* p);

#define NEW            new//(__FILE__,__LINE__)
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)      if((x) != NULL) {delete x; x = 0;}
#endif // SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)  if((x) != NULL) {delete[]x; x = 0;}
#endif // SAFE_DELETE_ARRAY

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  if((x) != NULL) {(x)->Release(); (x) = NULL; }
#endif // SAFE_RELEASE

bool CheckMemLeak();

#endif // _CL_MEMORY_MANAGENT_