#ifndef _DEV_DISABLE_UI_CODE
/* Device Parameters for GetDeviceCaps() */
//#define GXDRIVERVERSION 0     /* Device driver version                    */
//#define GXTECHNOLOGY    2     /* Device classification                    */
//#define GXHORZSIZE      4     /* Horizontal size in millimeters           */
//#define GXVERTSIZE      6     /* Vertical size in millimeters             */
//#define GXHORZRES       8     /* Horizontal width in pixels               */
//#define GXVERTRES       10    /* Vertical height in pixels                */
#define GXBITSPIXEL     12    /* Number of bits per pixel                 */

/* Region Flags */
#define GXERROR               0
#define GXNULLREGION          1
#define GXSIMPLEREGION        2
#define GXCOMPLEXREGION       3

/* CombineRgn() Styles */
#define GXRGN_AND             1
#define GXRGN_OR              2
#define GXRGN_XOR             3
#define GXRGN_DIFF            4
#define GXRGN_COPY            5
#define GXRGN_MIN             GXRGN_AND
#define GXRGN_MAX             GXRGN_COPY

/* Object Definitions for EnumObjects() */
#define GXOBJ_PEN             1
#define GXOBJ_BRUSH           2
#define GXOBJ_DC              3
#define GXOBJ_METADC          4
#define GXOBJ_PAL             5
#define GXOBJ_FONT            6
#define GXOBJ_BITMAP          7
#define GXOBJ_REGION          8
#define GXOBJ_METAFILE        9
#define GXOBJ_MEMDC           10
#define GXOBJ_EXTPEN          11
#define GXOBJ_ENHMETADC       12
#define GXOBJ_ENHMETAFILE     13
#define GXOBJ_COLORSPACE      14
#define GXGDI_OBJ_LAST        GXOBJ_COLORSPACE


/* Ternary raster operations */
#define GXSRCCOPY             (GXDWORD)0x00CC0020 /* dest = source                   */
#define GXSRCPAINT            (GXDWORD)0x00EE0086 /* dest = source OR dest           */
#define GXSRCAND              (GXDWORD)0x008800C6 /* dest = source AND dest          */
#define GXSRCINVERT           (GXDWORD)0x00660046 /* dest = source XOR dest          */
#define GXSRCERASE            (GXDWORD)0x00440328 /* dest = source AND (NOT dest )   */
#define GXNOTSRCCOPY          (GXDWORD)0x00330008 /* dest = (NOT source)             */
#define GXNOTSRCERASE         (GXDWORD)0x001100A6 /* dest = (NOT src) AND (NOT dest) */
#define GXMERGECOPY           (GXDWORD)0x00C000CA /* dest = (source AND pattern)     */
#define GXMERGEPAINT          (GXDWORD)0x00BB0226 /* dest = (NOT source) OR dest     */
#define GXPATCOPY             (GXDWORD)0x00F00021 /* dest = pattern                  */
#define GXPATPAINT            (GXDWORD)0x00FB0A09 /* dest = DPSnoo                   */
#define GXPATINVERT           (GXDWORD)0x005A0049 /* dest = pattern XOR dest         */
#define GXDSTINVERT           (GXDWORD)0x00550009 /* dest = (NOT dest)               */
#define GXBLACKNESS           (GXDWORD)0x00000042 /* dest = BLACK                    */
#define GXWHITENESS           (GXDWORD)0x00FF0062 /* dest = WHITE                    */
#define GXNOMIRRORBITMAP      (GXDWORD)0x80000000 /* Do not Mirror the bitmap in this call */
#define GXCAPTUREBLT          (GXDWORD)0x40000000 /* Include layered windows */

/* Brush Styles */
#define GXBS_SOLID            0
#define GXBS_NULL             1
#define GXBS_HOLLOW           GXBS_NULL
#define GXBS_HATCHED          2
#define GXBS_PATTERN          3
#define GXBS_INDEXED          4
#define GXBS_DIBPATTERN       5
#define GXBS_DIBPATTERNPT     6
#define GXBS_PATTERN8X8       7
#define GXBS_DIBPATTERN8X8    8
#define GXBS_MONOPATTERN      9

/* Hatch Styles */
#define GXHS_HORIZONTAL       0       /* ----- */
#define GXHS_VERTICAL         1       /* ||||| */
#define GXHS_FDIAGONAL        2       /* \\\\\ */
#define GXHS_BDIAGONAL        3       /* ///// */
#define GXHS_CROSS            4       /* +++++ */
#define GXHS_DIAGCROSS        5       /* xxxxx */
#define GXHS_API_MAX          12

/* Pen Styles */
#define GXPS_SOLID            0
#define GXPS_DASH             1       /* -------  */
#define GXPS_DOT              2       /* .......  */
#define GXPS_DASHDOT          3       /* _._._._  */
#define GXPS_DASHDOTDOT       4       /* _.._.._  */
#define GXPS_NULL             5
#define GXPS_INSIDEFRAME      6
#define GXPS_USERSTYLE        7
#define GXPS_ALTERNATE        8
#define GXPS_STYLE_MASK       0x0000000F

#define GXPS_ENDCAP_ROUND     0x00000000
#define GXPS_ENDCAP_SQUARE    0x00000100
#define GXPS_ENDCAP_FLAT      0x00000200
#define GXPS_ENDCAP_MASK      0x00000F00

#define GXPS_JOIN_ROUND       0x00000000
#define GXPS_JOIN_BEVEL       0x00001000
#define GXPS_JOIN_MITER       0x00002000
#define GXPS_JOIN_MASK        0x0000F000

#define GXPS_COSMETIC         0x00000000
#define GXPS_GEOMETRIC        0x00010000
#define GXPS_TYPE_MASK        0x000F0000


/* Stock Logical Objects */
#define GXWHITE_BRUSH         0
#define GXLTGRAY_BRUSH        1
#define GXGRAY_BRUSH          2
#define GXDKGRAY_BRUSH        3
#define GXBLACK_BRUSH         4
#define NULL_BRUSH          5
#define GXHOLLOW_BRUSH        NULL_BRUSH
#define GXWHITE_PEN           6
#define GXBLACK_PEN           7
#define NULL_PEN            8
//#define GXOEM_FIXED_FONT      10
//#define GXANSI_FIXED_FONT     11
//#define GXANSI_VAR_FONT       12
#define GXSYSTEM_FONT         13
//#define GXDEVICE_DEFAULT_FONT 14
#define GXDEFAULT_PALETTE     15
//#define GXSYSTEM_FIXED_FONT   16
#define GXDC_BRUSH            18
#define GXDC_PEN              19

/* constants for the biCompression field */
#define GXBI_RGB        0L

/* DIB color table identifiers */

#define GXDIB_RGB_COLORS      0 /* color table in RGBs */
//#define GXDIB_PAL_COLORS      1 /* color table in palette indices */

/* Binary raster ops */
//#define GXR2_BLACK            1   /*  0       */
//#define GXR2_NOTMERGEPEN      2   /* DPon     */
//#define GXR2_MASKNOTPEN       3   /* DPna     */
//#define GXR2_NOTCOPYPEN       4   /* PN       */
//#define GXR2_MASKPENNOT       5   /* PDna     */
//#define GXR2_NOT              6   /* Dn       */
#define GXR2_XORPEN           7   /* DPx      */
//#define GXR2_NOTMASKPEN       8   /* DPan     */
//#define GXR2_MASKPEN          9   /* DPa      */
//#define GXR2_NOTXORPEN        10  /* DPxn     */
//#define GXR2_NOP              11  /* D        */
//#define GXR2_MERGENOTPEN      12  /* DPno     */
//#define GXR2_COPYPEN          13  /* P        */
//#define GXR2_MERGEPENNOT      14  /* PDno     */
//#define GXR2_MERGEPEN         15  /* DPo      */
//#define GXR2_WHITE            16  /*  1       */
//#define GXR2_LAST             16

/* Device Parameters for GetDeviceCaps() */
#define GXDRIVERVERSION 0     /* Device driver version                    */
#define GXTECHNOLOGY    2     /* Device classification                    */
#define GXHORZSIZE      4     /* Horizontal size in millimeters           */
#define GXVERTSIZE      6     /* Vertical size in millimeters             */
#define GXHORZRES       8     /* Horizontal width in pixels               */
#define GXVERTRES       10    /* Vertical height in pixels                */
#define GXBITSPIXEL     12    /* Number of bits per pixel                 */
#define GXPLANES        14    /* Number of planes                         */
#define GXNUMBRUSHES    16    /* Number of brushes the device has         */
#define GXNUMPENS       18    /* Number of pens the device has            */
#define GXNUMMARKERS    20    /* Number of markers the device has         */
#define GXNUMFONTS      22    /* Number of fonts the device has           */
#define GXNUMCOLORS     24    /* Number of colors the device supports     */
#define GXPDEVICESIZE   26    /* Size required for device descriptor      */
#define GXCURVECAPS     28    /* Curve capabilities                       */
#define GXLINECAPS      30    /* Line capabilities                        */
#define GXPOLYGONALCAPS 32    /* Polygonal capabilities                   */
#define GXTEXTCAPS      34    /* Text capabilities                        */
#define GXCLIPCAPS      36    /* Clipping capabilities                    */
#define GXRASTERCAPS    38    /* Bitblt capabilities                      */
#define GXASPECTX       40    /* Length of the X leg                      */
#define GXASPECTY       42    /* Length of the Y leg                      */
#define GXASPECTXY      44    /* Length of the hypotenuse                 */

#define GXLOGPIXELSX    88    /* Logical pixels/inch in X                 */
#define GXLOGPIXELSY    90    /* Logical pixels/inch in Y                 */

#define GXSIZEPALETTE   104   /* Number of entries in physical palette    */
#define GXNUMRESERVED   106   /* Number of reserved entries in palette    */
#define GXCOLORRES      108   /* Actual color resolution                  */

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
  GXINT       GXDLLAPI gxGetObject              (GXHGDIOBJ hgdiobj, GXINT cbBuffer, GXLPVOID lpvObject);
  GXDWORD     GXDLLAPI gxGetObjectType          (GXHGDIOBJ hgdiobj);  

  GXHGDIOBJ   GXDLLAPI gxSelectObject           (GXHDC hdc, GXHGDIOBJ hgdiobj);
  GXBOOL      GXDLLAPI gxExtTextOutW            (GXHDC hdc, GXINT,GXINT,GXUINT,GXCONST GXRECT*,GXLPCWSTR,GXUINT,GXCONST GXINT *);
  GXHBRUSH    GXDLLAPI gxCreateSolidBrush       (GXCOLORREF);
  GXCOLORREF  GXDLLAPI gxSetBkColor             (GXHDC hdc, GXCOLORREF crBack);
  GXCOLORREF  GXDLLAPI GXSetBkColor             (GXHDC hdc, GXCOLORREF crBack); // GrapX 特有，支持透明色
  GXBOOL      GXDLLAPI gxDeleteDC               (GXHDC hdc);

  GXINT       GXDLLAPI gxGetClipRgn             (GXHDC hdc, GXHRGN hRgn);
  GXBOOL      GXDLLAPI gxBitBlt                 (GXHDC hdc, GXINT, GXINT, GXINT, GXINT, GXHDC, GXINT, GXINT, GXDWORD);
  GXBOOL      GXDLLAPI gxDeleteObject           (GXHGDIOBJ hgdiobj);  

  GXBOOL      GXDLLAPI gxStretchBlt             (GXHDC hdc, GXINT, GXINT, GXINT, GXINT, GXHDC, GXINT, GXINT, GXINT, GXINT, GXDWORD);

  GXHDC       GXDLLAPI gxCreateCompatibleDC     (GXHDC hdc);
  GXCOLORREF  GXDLLAPI gxSetTextColor           (GXHDC hdc, GXCOLORREF crColor);

  GXDWORD     GXDLLAPI gxSetLayout              (GXHDC hdc, GXDWORD dwLayout);
  GXDWORD     GXDLLAPI gxGetLayout              (GXHDC hdc);
  GXBOOL      GXDLLAPI gxDPtoLP                 (GXHDC hdc, GXLPPOINT lpPoints, GXINT nCount);
  GXINT       GXDLLAPI gxSelectClipRgn          (GXHDC hdc, GXHRGN hrgn);

  GXINT       GXDLLAPI gxIntersectClipRect      (GXHDC hdc, GXINT nLeftRect, GXINT nTopRect, GXINT nRightRect, GXINT nBottomRect);
  GXHRGN      GXDLLAPI gxCreateRectRgn          (GXINT nLeftRect, GXINT nTopRect, GXINT nRightRect, GXINT nBottomRect);  
  GXHPEN      GXDLLAPI gxExtCreatePen           (GXDWORD dwPenStyle,GXDWORD dwWidth,GXCONST GXLOGBRUSH *lplb,GXDWORD dwStyleCount, GXCONST GXDWORD *lpStyle);
  GXHPEN      GXDLLAPI gxCreatePen              (int fnPenStyle,int nWidth,GXCOLORREF crColor);


  GXHRGN      GXDLLAPI gxCreateRectRgnIndirect  (GXCONST GXRECT *lprc);
  GXBOOL      GXDLLAPI gxGetTextExtentPointW    (GXHDC hdc, GXLPCWSTR, GXINT, GXLPSIZE);
  GXBOOL      GXDLLAPI gxSetWindowOrgEx         (GXHDC hdc, GXINT, GXINT, GXLPPOINT);

  GXHRGN      GXDLLAPI gxCreateRectRgnIndirect  (GXCONST GXRECT *lprc);
  GXBOOL      GXDLLAPI gxGetTextExtentPointW    (GXHDC hdc, GXLPCWSTR, GXINT, GXLPSIZE);
  GXBOOL      GXDLLAPI gxSetWindowOrgEx         (GXHDC hdc, GXINT, GXINT, GXLPPOINT);
  GXBOOL      GXDLLAPI gxOffsetWindowOrgEx      (GXHDC hdc, int nXOffset, int nYOffset, GXLPPOINT lpPoint);

  GXBOOL      GXDLLAPI gxTextOutW               (GXHDC hdc, GXINT, GXINT, GXLPCWSTR, GXINT);
  GXBOOL      GXDLLAPI gxPatBlt                 (GXHDC hdc, GXINT, GXINT, GXINT, GXINT, GXDWORD);
  GXINT       GXDLLAPI gxSetBkMode              (GXHDC hdc, GXINT);
  GXBOOL      GXDLLAPI gxSetRectRgn             (GXHRGN hrgn, GXINT nLeft, GXINT nTop, GXINT nRight, GXINT nBottom);
  GXINT       GXDLLAPI gxGetObjectW             (GXHGDIOBJ hgdiobj, GXINT, GXLPVOID);
  GXCOLORREF  GXDLLAPI gxGetTextColor           (GXHDC hdc);
  GXCOLORREF  GXDLLAPI gxGetBkColor             (GXHDC hdc);
  GXINT       GXDLLAPI gxGetBkMode              (GXHDC hdc);
  GXHGDIOBJ   GXDLLAPI gxGetStockObject         (int fnObject);
  GXHFONT     GXDLLAPI gxCreateFontIndirectW    (GXCONST GXLOGFONTW* lplf);
  GXINT       GXDLLAPI gxCombineRgn             (GXHRGN hrgn, GXHRGN hrgnSrc1, GXHRGN hrgnSrc2, GXINT fnCombineMode);
  GXBOOL      GXDLLAPI gxGetTextMetricsW        (GXHDC hdc, GXLPTEXTMETRICW);
  GXINT       GXDLLAPI gxGetClipBox             (GXHDC hdc, GXLPRECT);
  GXHGDIOBJ   GXDLLAPI gxGetCurrentObject       (GXHDC hdc, GXUINT);
  GXBOOL      GXDLLAPI gxGetTextExtentPoint32W  (GXHDC hdc, GXLPCWSTR, GXINT,GXLPSIZE);

  GXBOOL      GXDLLAPI gxInvertRect             (GXHDC hdc, const GXRECT* lprc);
  GXBOOL      GXDLLAPI gxRectangle              (GXHDC hdc, GXINT nLeftRect, GXINT nTopRect, GXINT nRightRect, GXINT nBottomRect);
  GXBOOL      GXDLLAPI gxMoveToEx               (GXHDC hdc, int X, int Y, LPGXPOINT lpPoint);
  GXBOOL      GXDLLAPI gxLineTo                 (GXHDC hdc, int nXEnd, int nYEnd);
  GXBOOL      GXDLLAPI gxSetViewportOrgEx       (GXHDC hdc, int X, int Y, LPGXPOINT lpPoint);

  GXHBITMAP   GXDLLAPI gxCreateCompatibleBitmap (GXHDC hdc,int nWidth,int nHeight);
  GXHBITMAP   GXDLLAPI gxCreateBitmap           (int nWidth,int nHeight,GXUINT cPlanes,GXUINT cBitsPerPel,GXCONST GXVOID *lpvBits);
  GXHBRUSH    GXDLLAPI gxCreatePatternBrush     (GXHBITMAP hbmp);

  GXCOLORREF  GXDLLAPI gxSetPixel               (GXHDC hdc, int X, int Y, GXCOLORREF crColor);
  GXCOLORREF  GXDLLAPI gxGetPixel               (GXHDC hdc, int XPos, int nYPos);
  GXHBITMAP   GXDLLAPI gxCreateDIBSection       (GXHDC hdc, GXCONST GXBITMAPINFO *pbmi, GXUINT iUsage, GXVOID *ppvBits, GXHANDLE hSection, GXDWORD dwOffset);
  GXUINT      GXDLLAPI gxGetPaletteEntries      (GXHPALETTE hpal, GXUINT iStartIndex, GXUINT nEntries, GXLPPALETTEENTRY lppe);
  int         GXDLLAPI gxGetDeviceCaps          (GXHDC hdc, int nIndex);
  GXBOOL      GXDLLAPI gxRectVisible            (GXHDC hdc, GXCONST GXRECT *lprc);

  int         GXDLLAPI gxSetROP2                (GXHDC hdc, int fnDrawMode);
  GXBOOL      GXDLLAPI gxPolyPolyline           (GXHDC hdc, GXCONST GXPOINT *lppt, GXCONST GXDWORD *lpdwPolyPoints, GXDWORD cCount);

  GXVOID      _gxInitDefGXDC(GXHDC);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _DEV_DISABLE_UI_CODE