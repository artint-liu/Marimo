// clstdX_Clang.cpp : Defines the exported functions for the static library.
//

#include "clstdX_Clang.h"

// This is an example of an exported variable
int nclstdX_Clang=0;

// This is an example of an exported function.
int fnclstdX_Clang(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see clstdX_Clang.h for the class definition
CclstdX_Clang::CclstdX_Clang()
{
    return;
}
