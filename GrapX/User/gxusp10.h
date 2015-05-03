/*
 * Copyright (C) 2005 Steven Edwards
 * Copyright (C) 2005 Vijay Kiran Kamuju
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __USP10_H
#define __USP10_H

//#include <windows.h>
/* FIXME: #include <specstrings.h> */

#ifdef __cplusplus
extern "C" {
#endif

/** ScriptStringAnalyse */
#define  GXSSA_PASSWORD         0x00000001
#define  GXSSA_TAB              0x00000002
#define  GXSSA_CLIP             0x00000004
#define  GXSSA_FIT              0x00000008
#define  GXSSA_DZWG             0x00000010
#define  GXSSA_FALLBACK         0x00000020
#define  GXSSA_BREAK            0x00000040
#define  GXSSA_GLYPHS           0x00000080
#define  GXSSA_RTL              0x00000100
#define  GXSSA_GCP              0x00000200
#define  GXSSA_HOTKEY           0x00000400
#define  GXSSA_METAFILE         0x00000800
#define  GXSSA_LINK             0x00001000
#define  GXSSA_HIDEHOTKEY       0x00002000
#define  GXSSA_HOTKEYONLY       0x00002400
#define  GXSSA_FULLMEASURE      0x04000000
#define  GXSSA_LPKANSIFALLBACK  0x08000000
#define  GXSSA_PIDX             0x10000000
#define  GXSSA_LAYOUTRTL        0x20000000
#define  GXSSA_DONTGLYPH        0x40000000 
#define  GXSSA_NOKASHIDA        0x80000000 

/** StringIsComplex */
#define  GXSIC_COMPLEX     1
#define  GXSIC_ASCIIDIGIT  2
#define  GXSIC_NEUTRAL     4

/** ScriptGetCMap */
#define GXSGCM_RTL  0x00000001

/** ScriptApplyDigitSubstitution */
#define GXSCRIPT_DIGITSUBSTITUTE_CONTEXT      0
#define GXSCRIPT_DIGITSUBSTITUTE_NONE         1
#define GXSCRIPT_DIGITSUBSTITUTE_NATIONAL     2
#define GXSCRIPT_DIGITSUBSTITUTE_TRADITIONAL  3

#define GXSCRIPT_UNDEFINED  0

#define GXUSP_E_SCRIPT_NOT_IN_FONT MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,0x200)

typedef enum tag_GXSCRIPT_JUSTIFY {
  GXSCRIPT_JUSTIFY_NONE           = 0,
  GXSCRIPT_JUSTIFY_ARABIC_BLANK   = 1,
  GXSCRIPT_JUSTIFY_CHARACTER      = 2,
  GXSCRIPT_JUSTIFY_RESERVED1      = 3,
  GXSCRIPT_JUSTIFY_BLANK          = 4,
  GXSCRIPT_JUSTIFY_RESERVED2      = 5,
  GXSCRIPT_JUSTIFY_RESERVED3      = 6,
  GXSCRIPT_JUSTIFY_ARABIC_NORMAL  = 7,
  GXSCRIPT_JUSTIFY_ARABIC_KASHIDA = 8,
  GXSCRIPT_JUSTIFY_ARABIC_ALEF    = 9,
  GXSCRIPT_JUSTIFY_ARABIC_HA      = 10,
  GXSCRIPT_JUSTIFY_ARABIC_RA      = 11,
  GXSCRIPT_JUSTIFY_ARABIC_BA      = 12,
  GXSCRIPT_JUSTIFY_ARABIC_BARA    = 13,
  GXSCRIPT_JUSTIFY_ARABIC_SEEN    = 14,
  GXSCRIPT_JUSTIFY_ARABIC_SEEN_M  = 15,
} GXSCRIPT_JUSTIFY;

typedef struct tag_GXSCRIPT_CONTROL {
  GXDWORD uDefaultLanguage	:16;
  GXDWORD fContextDigits		:1;
  GXDWORD fInvertPreBoundDir	:1;
  GXDWORD fInvertPostBoundDir	:1;
  GXDWORD fLinkStringBefore	:1;
  GXDWORD fLinkStringAfter	:1;
  GXDWORD fNeutralOverride	:1;
  GXDWORD fNumericOverride	:1;
  GXDWORD fLegacyBidiClass	:1;
  GXDWORD fMergeNeutralItems	:1;
  GXDWORD fReserved		:7;
} GXSCRIPT_CONTROL;

typedef struct {
  GXDWORD langid			:16;
  GXDWORD fNumeric		:1;
  GXDWORD fComplex		:1;     
  GXDWORD fNeedsWordBreaking	:1;     
  GXDWORD fNeedsCaretInfo		:1;
  GXDWORD bCharSet		:8;
  GXDWORD fControl		:1;
  GXDWORD fPrivateUseArea		:1;
  GXDWORD fNeedsCharacterJustify	:1;
  GXDWORD fInvalidGlyph		:1;
  GXDWORD fInvalidLogAttr		:1;
  GXDWORD fCDM			:1;
  GXDWORD fAmbiguousCharSet	:1;
  GXDWORD fClusterSizeVaries	:1;
  GXDWORD fRejectInvalid		:1;
} GXSCRIPT_PROPERTIES;

typedef struct tag_GXSCRIPT_STATE {
  GXWORD uBidiLevel		:5;
  GXWORD fOverrideDirection	:1;
  GXWORD fInhibitSymSwap		:1;
  GXWORD fCharShape		:1;
  GXWORD fDigitSubstitute		:1;
  GXWORD fInhibitLigate		:1;
  GXWORD fDisplayZWG		:1;
  GXWORD fArabicNumContext	:1;
  GXWORD fGcpClusters		:1;
  GXWORD fReserved		:1;
  GXWORD fEngineReserved		:2;
} GXSCRIPT_STATE;

typedef struct tag_GXSCRIPT_ANALYSIS {
  GXWORD eScript			:10;
  GXWORD fRTL			:1;
  GXWORD fLayoutRTL		:1;
  GXWORD fLinkBefore		:1;
  GXWORD fLinkAfter		:1;
  GXWORD fLogicalOrder		:1;
  GXWORD fNoGlyphIndex		:1;
  GXSCRIPT_STATE 	s;
} GXSCRIPT_ANALYSIS;

typedef struct tag_GXSCRIPT_ITEM {
  int iCharPos;
  GXSCRIPT_ANALYSIS a;
} GXSCRIPT_ITEM;

typedef struct tag_GXSCRIPT_DIGITSUBSTITUTE {
  GXDWORD NationalDigitLanguage		:16;
  GXDWORD TraditionalDigitLanguage	:16;
  GXDWORD DigitSubstitute			:8;
  GXDWORD dwReserved;
} GXSCRIPT_DIGITSUBSTITUTE;

typedef struct tag_GXSCRIPT_FONTPROPERTIES {
  int   cBytes;
  GXWORD wgBlank;
  GXWORD wgDefault;
  GXWORD wgInvalid;
  GXWORD wgKashida;
  int iKashidaWidth;
} GXSCRIPT_FONTPROPERTIES;

typedef struct tag_GXSCRIPT_TABDEF {
  int cTabStops;
  int iScale;
  int *pTabStops;
  int iTabOrigin;
} GXSCRIPT_TABDEF;

typedef struct tag_GXSCRIPT_VISATTR {
  GXWORD uJustification   :4;
  GXWORD fClusterStart    :1;
  GXWORD fDiacritic       :1;
  GXWORD fZeroWidth       :1;
  GXWORD fReserved        :1;
  GXWORD fShapeReserved   :8;
} GXSCRIPT_VISATTR;

typedef struct tag_GXSCRIPT_LOGATTR {
  GXBYTE    fSoftBreak      :1;
  GXBYTE    fWhiteSpace     :1;
  GXBYTE    fCharStop       :1;
  GXBYTE    fWordStop       :1;
  GXBYTE    fInvalid        :1;
  GXBYTE    fReserved       :3;
} GXSCRIPT_LOGATTR;

typedef const GXSCRIPT_LOGATTR* GXLPCSCRIPT_LOGATTR;
typedef void *GXSCRIPT_CACHE;
typedef void *GXSCRIPT_STRING_ANALYSIS; 

#ifndef LSDEFS_DEFINED
typedef struct tagGXGOFFSET {
  GXLONG  du;
  GXLONG  dv;
} GXGOFFSET;
#endif

typedef GXULONG GXOPENTYPE_TAG;

typedef struct tagGXOPENTYPE_FEATURE_RECORD
{
    GXOPENTYPE_TAG tagFeature;
    GXLONG         lParameter;
} GXOPENTYPE_FEATURE_RECORD;

typedef struct tagGXSCRIPT_GLYPHPROP
{
    GXSCRIPT_VISATTR sva;
    GXWORD           reserved;
} GXSCRIPT_GLYPHPROP;

typedef struct tagGXSCRIPT_CHARPROP
{
    GXWORD fCanGlyphAlone  :1;
    GXWORD reserved        :15;
} GXSCRIPT_CHARPROP;

typedef struct tagGXTEXTRANGE_PROPERTIES
{
    GXOPENTYPE_FEATURE_RECORD *potfRecords;
    GXINT                     cotfRecords;
} GXTEXTRANGE_PROPERTIES;

/* Function Declarations */

GXHRESULT GXDLLAPI gxScriptApplyDigitSubstitution(const GXSCRIPT_DIGITSUBSTITUTE* psds, 
                                            GXSCRIPT_CONTROL* psc, GXSCRIPT_STATE* pss);
//GXHRESULT GXDLLAPI gxScriptApplyLogicalWidth(const int *piDx, int cChars, int cGlyphs, const GXWORD *pwLogClust,
//                                       const GXSCRIPT_VISATTR *psva, const int *piAdvance,
//                                       const GXSCRIPT_ANALYSIS *psa, GXABC *pABC, int *piJustify);
//GXHRESULT GXDLLAPI gxScriptRecordDigitSubstitution(LCID Locale, SCRIPT_DIGITSUBSTITUTE *psds);
GXHRESULT GXDLLAPI gxScriptItemize(const GXWCHAR *pwcInChars, int cInChars, int cMaxItems, 
                             const GXSCRIPT_CONTROL *psControl, const GXSCRIPT_STATE *psState, 
                             GXSCRIPT_ITEM *pItems, int *pcItems);
GXHRESULT GXDLLAPI gxScriptGetCMap(GXHDC hdc, GXSCRIPT_CACHE *psc, const GXWCHAR *pwcInChars, int cChars,
                             GXDWORD dwFlags, GXWORD *pwOutGlyphs);
GXHRESULT GXDLLAPI gxScriptGetFontProperties(GXHDC hdc, GXSCRIPT_CACHE *psc, GXSCRIPT_FONTPROPERTIES *sfp);
//GXHRESULT GXDLLAPI gxScriptGetGlyphABCWidth(GXHDC hdc, GXSCRIPT_CACHE *psc, GXWORD wGlyph, ABC *pABC);
GXHRESULT GXDLLAPI gxScriptGetLogicalWidths(const GXSCRIPT_ANALYSIS *psa, int cChars, int cGlyphs,
                                      const int *piGlyphWidth, const GXWORD *pwLogClust,
                                      const GXSCRIPT_VISATTR *psva, int *piDx);
GXHRESULT GXDLLAPI gxScriptGetProperties(const GXSCRIPT_PROPERTIES ***ppSp, int *piNumScripts);
GXHRESULT GXDLLAPI gxScriptStringAnalyse(GXHDC hdc, 
				   const void *pString, 
				   int cString, 
				   int cGlyphs,
				   int iCharset,
				   GXDWORD dwFlags,
				   int iReqWidth,
				   GXSCRIPT_CONTROL *psControl,
				   GXSCRIPT_STATE *psState,
				   const int *piDx,
				   GXSCRIPT_TABDEF *pTabdef,
				   const GXBYTE *pbInClass,
				   GXSCRIPT_STRING_ANALYSIS *pssa);
GXHRESULT GXDLLAPI gxScriptStringValidate(GXSCRIPT_STRING_ANALYSIS ssa);
GXHRESULT GXDLLAPI gxScriptStringFree(GXSCRIPT_STRING_ANALYSIS *pssa);
GXHRESULT GXDLLAPI gxScriptFreeCache(GXSCRIPT_CACHE *psc);
GXHRESULT GXDLLAPI gxScriptIsComplex(const GXWCHAR* pwcInChars, int cInChars, GXDWORD dwFlags);
GXHRESULT GXDLLAPI gxScriptJustify(const GXSCRIPT_VISATTR *psva, const int *piAdvance, int cGlyphs,
                             int iDx, int iMinKashida, int *piJustify);
GXHRESULT GXDLLAPI gxScriptLayout(int cRuns, const GXBYTE *pbLevel, int *piVisualToLogical, int *piLogicalToVisual);
GXHRESULT GXDLLAPI gxScriptShape(GXHDC hdc, GXSCRIPT_CACHE *psc, const GXWCHAR *pwcChars, int cChars, int cMaxGlyphs,
                           GXSCRIPT_ANALYSIS *psa, GXWORD *pwOutGlyphs, GXWORD *pwLogClust, GXSCRIPT_VISATTR *psva, int *pcGlyphs);
//GXHRESULT GXDLLAPI gxScriptPlace(GXHDC hdc, GXSCRIPT_CACHE *psc, const GXWORD *pwGlyphs, int cGlyphs, const GXSCRIPT_VISATTR *psva,
//                           GXSCRIPT_ANALYSIS *psa, int *piAdvance, GXGOFFSET *pGoffset, ABC *pABC );
GXHRESULT GXDLLAPI gxScriptBreak(const GXWCHAR *pwcChars, int cChars, const GXSCRIPT_ANALYSIS *psa, GXSCRIPT_LOGATTR *psla);
GXHRESULT GXDLLAPI gxScriptCacheGetHeight(GXHDC hdc, GXSCRIPT_CACHE *psc, GXLONG *tmHeight);
GXHRESULT GXDLLAPI gxScriptCPtoX(int iCP, GXBOOL fTrailing, int cChars, int cGlyphs, const GXWORD *pwLogClust, const GXSCRIPT_VISATTR *psva,
                           const int *piAdvance, const GXSCRIPT_ANALYSIS *psa, int *piX);
GXHRESULT GXDLLAPI gxScriptXtoCP(int iX, int cChars, int cGlyphs, const GXWORD *pwLogClust, const GXSCRIPT_VISATTR *psva,
                           const int *piAdvance, const GXSCRIPT_ANALYSIS *psa, int *piCP, int *piTrailing);
GXHRESULT GXDLLAPI gxScriptStringCPtoX(GXSCRIPT_STRING_ANALYSIS ssa, int icp, GXBOOL fTrailing, int *pX);
GXHRESULT GXDLLAPI gxScriptStringXtoCP(GXSCRIPT_STRING_ANALYSIS ssa, int iX, int *piCh , int *piTrailing);
GXHRESULT GXDLLAPI gxScriptStringGetLogicalWidths(GXSCRIPT_STRING_ANALYSIS ssa, int *piDx);
GXHRESULT GXDLLAPI gxScriptStringGetOrder(GXSCRIPT_STRING_ANALYSIS ssa, GXUINT *puOrder);
GXHRESULT GXDLLAPI gxScriptStringOut(GXSCRIPT_STRING_ANALYSIS ssa, int iX, int iY, GXUINT uOptions, const GXRECT *prc,
                               int iMinSel, int iMaxSel, GXBOOL fDisabled);
GXHRESULT GXDLLAPI gxScriptTextOut(const GXHDC hdc, GXSCRIPT_CACHE *psc, int x, int y, GXUINT fuOptions, const GXRECT *lprc,
                             const GXSCRIPT_ANALYSIS *psa, const GXWCHAR *pwcReserved, int iReserved, const GXWORD *pwGlyphs,
                             int cGlyphs, const int *piAdvance, const int *piJustify, const GXGOFFSET *pGoffset);
GXLPCINT GXDLLAPI gxScriptString_pcOutChars(GXSCRIPT_STRING_ANALYSIS ssa);
GXLPCSCRIPT_LOGATTR GXDLLAPI ScriptString_pLogAttr(GXSCRIPT_STRING_ANALYSIS ssa);
GXLPSIZE GXDLLAPI gxScriptString_pSize(GXSCRIPT_STRING_ANALYSIS ssa);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __USP10_H */
