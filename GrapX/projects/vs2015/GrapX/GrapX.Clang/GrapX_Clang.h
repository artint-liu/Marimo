#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GrapX_Clang_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GrapX_Clang_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GrapX_Clang_EXPORTS
#define GrapX_Clang_API __declspec(dllexport)
#else
#define GrapX_Clang_API __declspec(dllimport)
#endif

// This is an example of a class exported from the GrapX_Clang.dll
class GrapX_Clang_API CGrapX_Clang
{
public:
    CGrapX_Clang();
    // TODO: add your methods here.
};

// This is an example of an exported variable
extern GrapX_Clang_API int nGrapX_Clang;

// This is an example of an exported function.
GrapX_Clang_API int fnGrapX_Clang(void);
