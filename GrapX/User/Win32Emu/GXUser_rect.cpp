#include <GrapX.H>
#include <User/GrapX.Hxx>
//////////////////////////////////////////////////////////////////////////
extern "C"
{
  GXBOOL GXDLLAPI gxIsRectEmpty(
    GXCONST GXRECT *lprc   // address of structure with rectangle
    )
  {
    return (lprc == NULL || (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom));
  }

  GXBOOL GXDLLAPI gxUnionRect(
    GXLPRECT lprcDst,         // address of structure for union
    GXCONST GXRECT *lprcSrc1, // address of structure with first rectangle
    GXCONST GXRECT *lprcSrc2  // address of structure with second rectangle
    )
  {
    if(lprcSrc1 == NULL || lprcSrc1->left >= lprcSrc1->right || lprcSrc1->top >= lprcSrc1->bottom) {
      if(lprcSrc2 == NULL || lprcSrc2->left >= lprcSrc2->right || lprcSrc2->top >= lprcSrc2->bottom) {
        lprcDst->left   = 0;
        lprcDst->top    = 0;
        lprcDst->right  = 0;
        lprcDst->bottom = 0;
        return 0;
      }

      *lprcDst = *lprcSrc2;
      return 1;
    }
    else if(lprcSrc2 == NULL || lprcSrc2->left >= lprcSrc2->right || lprcSrc2->top >= lprcSrc2->bottom) {
      *lprcDst = *lprcSrc1;
      return 1;
    }

    lprcDst->left   = lprcSrc1->left   < lprcSrc2->left   ? lprcSrc1->left   : lprcSrc2->left;
    lprcDst->right  = lprcSrc1->right  > lprcSrc2->right  ? lprcSrc1->right  : lprcSrc2->right;
    lprcDst->top    = lprcSrc1->top    < lprcSrc2->top    ? lprcSrc1->top    : lprcSrc2->top;
    lprcDst->bottom = lprcSrc1->bottom > lprcSrc2->bottom ? lprcSrc1->bottom : lprcSrc2->bottom;

    //if(  lprcDst->left >= lprcDst->right  || 
    //  lprcDst->top >= lprcDst->bottom  )
    //{
    //  lprcDst->left  = 0;
    //  lprcDst->top  = 0;
    //  lprcDst->right  = 0;
    //  lprcDst->bottom  = 0;
    //  return 0;
    //}
    return 1;
  }

  GXBOOL GXDLLAPI gxUnionRegn(GXLPREGN lpOut, GXLPCREGN lprg1, GXLPCREGN lprg2)
  {
    if(lprg1 == NULL) {
      if(lprg2 == NULL) {
        lpOut->left   = 0;
        lpOut->top    = 0;
        lpOut->width  = 0;
        lpOut->height = 0;
        return 0;
      }
      else {
        *lpOut = *lprg2;
        return 1;
      }
    }
    else if(lprg2 == NULL) {
      *lpOut = *lprg1;
      return 1;
    }

    // 防止lpOut与lprg1/lprg2地址一样时left/top被提前改写
    const GXLONG right1 = lprg1->left + lprg1->width;
    const GXLONG right2 = lprg2->left + lprg2->width;
    const GXLONG bottom1 = lprg1->top + lprg1->height;
    const GXLONG bottom2 = lprg2->top + lprg2->height;

    lpOut->left   = clMin(lprg1->left, lprg2->left);
    lpOut->top    = clMin(lprg1->top,  lprg2->top);
    lpOut->width  = clMax(right1, right2) - lpOut->left;
    lpOut->height = clMax(bottom1, bottom2) - lpOut->top;
    return 1;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL GXDLLAPI gxIntersectRect(
    GXLPRECT lprcDst,      // address of structure for intersection
    GXCONST GXRECT *lprcSrc1,  // address of structure with first rectangle
    GXCONST GXRECT *lprcSrc2   // address of structure with second rectangle
    )
  {
    if(lprcSrc1 == NULL || lprcSrc2 == NULL) {
      lprcDst->left   = 0;
      lprcDst->top    = 0;
      lprcDst->right  = 0;
      lprcDst->bottom = 0;
      return 0;
    }

    lprcDst->left   = lprcSrc1->left    > lprcSrc2->left    ? lprcSrc1->left    : lprcSrc2->left;
    lprcDst->right  = lprcSrc1->right   < lprcSrc2->right   ? lprcSrc1->right   : lprcSrc2->right;
    lprcDst->top    = lprcSrc1->top     > lprcSrc2->top     ? lprcSrc1->top     : lprcSrc2->top;
    lprcDst->bottom = lprcSrc1->bottom  < lprcSrc2->bottom  ? lprcSrc1->bottom  : lprcSrc2->bottom;

    if(  lprcDst->left >= lprcDst->right || 
      lprcDst->top >= lprcDst->bottom)
    {
      lprcDst->left   = 0;
      lprcDst->top    = 0;
      lprcDst->right  = 0;
      lprcDst->bottom = 0;
      return 0;
    }
    return 1;
  }

  GXBOOL GXDLLAPI gxIsRegnEmpty(GXCONST GXREGN *lprg)
  {
    return lprg == NULL || lprg->width <= 0 || lprg->height <= 0;
  }

#ifdef ENABLE_ASSEMBLE
  __declspec(naked)GXBOOL GXDLLAPI gxSetRect(
    GXLPRECT lprc,  // address of structure with rectangle to set
    GXINT xLeft,  // left side
    GXINT yTop,  // top side
    GXINT xRight,  // right side
    GXINT yBottom   // bottom side
    )
  {
    __asm mov eax, [esp + 4]
    __asm mov ecx, [esp + 8]
    __asm mov edx, [esp + 12]
    __asm mov [eax], ecx
    __asm mov [eax + 4], edx
    __asm mov ecx, [esp + 16]
    __asm mov edx, [esp + 20]
    __asm mov [eax + 8], ecx
    __asm mov [eax + 12], edx
    __asm xor eax, eax
    __asm inc eax
    __asm ret 20
  }
#else
  GXBOOL GXDLLAPI gxSetRect(
    GXLPRECT lprc,  // address of structure with rectangle to set
    GXINT xLeft,  // left side
    GXINT yTop,  // top side
    GXINT xRight,  // right side
    GXINT yBottom   // bottom side
    )
  {
    lprc->left    = xLeft;
    lprc->top    = yTop;
    lprc->right    = xRight;
    lprc->bottom  = yBottom;
    return TRUE;
  }
#endif  // ENABLE_ASSEMBLE
  GXBOOL GXDLLAPI gxSetRegn(GXLPREGN lprg,GXINT xLeft,GXINT yTop,GXINT xWidth,GXINT yHeight)
  {
    lprg->left   = xLeft;
    lprg->top    = yTop;
    lprg->width  = xWidth;
    lprg->height = yHeight;
    return TRUE;
  }

#ifdef ENABLE_ASSEMBLE
  __declspec(naked)GXBOOL GXDLLAPI gxCopyRect(
    GXLPRECT lprcDst,  // pointer to structure for destination rectangle
    GXCONST GXRECT *lprcSrc   // pointer to structure with source rectangle
    )
  {
    __asm push esi
    __asm push edi
    __asm mov edi, [esp + 3 * 4]
    __asm mov esi, [esp + 4 * 4]
    __asm mov eax, [esi]
    __asm mov ecx, [esi + 4]
    __asm mov edx, [esi + 4 * 2]
    __asm mov [edi], eax
    __asm mov [edi + 4], ecx
    __asm mov eax, [esi + 4 * 3]
    __asm mov [edi + 4 * 2], edx
    __asm mov [edi + 4 * 3], eax
    __asm pop edi
    __asm pop esi
    __asm xor eax, eax
    __asm inc eax
    __asm ret 8
  }
#else
  GXBOOL GXDLLAPI gxCopyRect(
    GXLPRECT lprcDst,  // pointer to structure for destination rectangle
    GXCONST GXRECT *lprcSrc   // pointer to structure with source rectangle
    )
  {
    *lprcDst = *lprcSrc;
    return TRUE;
  }
#endif  // ENABLE_ASSEMBLE

#ifdef ENABLE_ASSEMBLE
  __declspec(naked)GXBOOL GXDLLAPI gxPtInRect(
    GXCONST GXRECT *lprc,  // address of structure with rectangle
    GXPOINT pt        // structure with point
    )
  {
    __asm mov ecx, [esp + 4]      // lprc
    __asm mov eax, [esp + 4 * 2]  // right
    __asm mov edx, [esp + 4 * 2]
    __asm sub eax, [ecx]          // left
    __asm sub edx, [ecx + 4 * 2]  // right
    __asm xor eax, edx
    __asm bt eax, 31
    __asm jnc false_ret

    __asm mov eax, [esp + 4 * 3]
    __asm mov edx, [esp + 4 * 3]
    __asm sub eax, [ecx + 4 * 1]  // top
    __asm sub edx, [ecx + 4 * 3]  // bottom
    __asm xor eax, edx
    __asm bt eax, 31
    __asm jnc false_ret

    __asm xor eax, eax
    __asm inc eax
    __asm ret 12
false_ret:
    __asm xor eax, eax
    __asm ret 12
  }
#else
  GXBOOL GXDLLAPI gxPtInRect(
    GXCONST GXRECT *lprc,  // address of structure with rectangle
    GXPOINT pt        // structure with point
    )
  {
    return (
      lprc->left <= pt.x && 
      lprc->right > pt.x &&
      lprc->top <= pt.y &&
      lprc->bottom > pt.y );
  }
#endif // ENABLE_ASSEMBLE

  GXBOOL GXDLLAPI gxPtInRegn(GXCONST GXREGN *lprg,GXPOINT pt)
  {
    return (
      lprg->left <= pt.x &&
      lprg->left + lprg->width > pt.x &&
      lprg->top <= pt.y &&
      lprg->top + lprg->height > pt.y );
  }

#ifdef ENABLE_ASSEMBLE
  __declspec(naked)GXBOOL GXDLLAPI gxEqualRect(
    GXCONST GXRECT *lprc1,  // pointer to structure with first rectangle
    GXCONST GXRECT *lprc2   // pointer to structure with second rectangle
    )
  {
    __asm push esi
    __asm push edi
    __asm mov esi, [esp + 4 * 3]
    __asm mov edi, [esp + 4 * 4]
    __asm mov eax, [esi]
    __asm mov edx, [esi + 4]
    __asm sub eax, [edi]
    __asm sub edx, [edi + 4]
    __asm mov ecx, [esi + 4 * 2]
    __asm add eax, edx
    __asm mov edx, [esi + 4 * 3]
    __asm sub ecx, [edi + 4 * 2]
    __asm sub edx, [edi + 4 * 3]
    __asm add eax, ecx
    __asm add eax, edx

    // 下面三行从VC编译结果上抄的，比较玄妙
                      // eax = 0  | eax != 0
    __asm neg eax     // CF = 0   | CF = 1
    __asm sbb eax,eax // eax = 0  | eax = 0xffffffff
    __asm inc eax     // eax = 1  | eax = 0

    __asm pop edi
    __asm pop esi
    __asm ret 8
  }
#else
  GXBOOL GXDLLAPI gxEqualRect(
    GXCONST GXRECT *lprc1,  // pointer to structure with first rectangle
    GXCONST GXRECT *lprc2   // pointer to structure with second rectangle
    )
  {
    return (
      lprc1->left    - lprc2->left + 
      lprc1->top    - lprc2->top +
      lprc1->right  - lprc2->right +
      lprc1->bottom  - lprc2->bottom) == 0;
  }
#endif // ENABLE_ASSEMBLE

  GXBOOL GXDLLAPI gxOffsetRect(
    GXLPRECT lprc,  // pointer to structure with rectangle
    GXINT dx,    // horizontal offset
    GXINT dy     // vertical offset
    )
  {
    lprc->left    += dx;
    lprc->top    += dy;
    lprc->right    += dx;
    lprc->bottom    += dy;
    return TRUE;
  }
  GXBOOL GXDLLAPI gxInflateRect(
    GXLPRECT lprc,  // address of rectangle
    GXINT dx,       // amount to increase or decrease width
    GXINT dy        // amount to increase or decrease height
    )
  {
    //ASSERT(FALSE);
    //TRACE_UNACHIEVE("=== gxInflateRect ===\n");
    lprc->left   -= dx;
    lprc->top    -= dy;
    lprc->right  += dx;
    lprc->bottom += dy;
    return TRUE;
  }

  GXVOID GXDLLAPI gxRegnToRect(GXLPRECT lprc, GXCONST GXLPREGN lpregn)
  {
    lprc->left = lpregn->left;
    lprc->top = lpregn->top;
    lprc->right = lpregn->left + lpregn->width;
    lprc->bottom = lpregn->top + lpregn->height;
  }

  GXVOID GXDLLAPI gxRectToRegn(GXLPREGN lpregn, GXCONST GXLPRECT lprc)
  {
    lpregn->left   = lprc->left;
    lpregn->top    = lprc->top;
    lpregn->width  = (lprc->right - lprc->left);
    lpregn->height = (lprc->bottom - lprc->top);
  }
  GXBOOL GXDLLAPI gxSetRectEmpty  (
    GXLPRECT lprc   // address of structure with rectangle set to empty
    )
  {
    lprc->left   = 0;
    lprc->top    = 0;
    lprc->right  = 0;
    lprc->bottom = 0;
    return TRUE;
  }

  GXBOOL GXDLLAPI gxSetRegnEmpty(GXLPREGN lprc)
  {
    lprc->left   = 0;
    lprc->top    = 0;
    lprc->width  = 0;
    lprc->height = 0;
    return TRUE;
  }

  LPGXREGN GXDLLAPI gxLerpRegn(LPGXREGN lprg, LPGXREGN lprg1, LPGXREGN lprg2, float fLerp)
  {
    lprg->left   = (GXLONG)((lprg2->left   - lprg1->left  ) * fLerp) + lprg1->left;
    lprg->top    = (GXLONG)((lprg2->top    - lprg1->top   ) * fLerp) + lprg1->top;
    lprg->width  = (GXLONG)((lprg2->width  - lprg1->width ) * fLerp) + lprg1->width;
    lprg->height = (GXLONG)((lprg2->height - lprg1->height) * fLerp) + lprg1->height;
    return lprg;
  }
} // extern "C"