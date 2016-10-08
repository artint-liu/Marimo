// parameter of ImmGetCompositionString
#define GXGCS_COMPREADSTR                 0x0001
#define GXGCS_COMPREADATTR                0x0002
#define GXGCS_COMPREADCLAUSE              0x0004
#define GXGCS_COMPSTR                     0x0008
#define GXGCS_COMPATTR                    0x0010
#define GXGCS_COMPCLAUSE                  0x0020
#define GXGCS_CURSORPOS                   0x0080
#define GXGCS_DELTASTART                  0x0100
#define GXGCS_RESULTREADSTR               0x0200
#define GXGCS_RESULTREADCLAUSE            0x0400
#define GXGCS_RESULTSTR                   0x0800
#define GXGCS_RESULTCLAUSE                0x1000

extern "C"
{
  GXLONG GXDLLAPI gxImmGetCompositionStringW(GXHIMC,GXDWORD,GXLPVOID,GXDWORD);
  GXHIMC GXDLLAPI gxImmGetContextW(GXHWND);
  GXBOOL GXDLLAPI gxImmReleaseContextW(GXHWND,GXHIMC);
}

typedef struct tagGXIMECHARPOSITION {
  GXDWORD       dwSize;
  GXDWORD       dwCharPos;
  GXPOINT       pt;
  GXUINT        cLineHeight;
  GXRECT        rcDocument;
} GXIMECHARPOSITION, *GXLPIMECHARPOSITION;

#define gxImmGetCompositionString  gxImmGetCompositionStringW
#define gxImmGetContext            gxImmGetContextW
#define gxImmReleaseContext        gxImmReleaseContextW
