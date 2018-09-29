#include <GrapX.h>
#include <User/GrapX.Hxx>
#include "GrapX/GXImm.h"
//#include "GXStation.h"
//#include <User/GXWindow.h>


GXLONG GXDLLAPI gxImmGetCompositionStringW(
               GXHIMC hIMC,   
               GXDWORD dwIndex,   
               GXLPVOID lpBuf,   
               GXDWORD dwBufLen  
               )
{
  ASSERT(FALSE);
  return NULL;
}

GXHIMC GXDLLAPI gxImmGetContextW(
           GXHWND hWnd  
           )
{
  ASSERT(FALSE);
  return NULL;
}

GXBOOL GXDLLAPI gxImmReleaseContextW(
             GXHWND hWnd,   
             GXHIMC hIMC  
             )
{
  ASSERT(FALSE);
  return FALSE;
}
