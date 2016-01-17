// GrapX_Clang.cpp : Defines the exported functions for the DLL application.
//

#include "GrapX_Clang.h"

// This is an example of an exported variable
GrapX_Clang_API int nGrapX_Clang=0;

// This is an example of an exported function.
GrapX_Clang_API int fnGrapX_Clang(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see GrapX_Clang.h for the class definition
CGrapX_Clang::CGrapX_Clang()
{
    return;
}
