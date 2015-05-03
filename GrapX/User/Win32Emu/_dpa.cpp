#ifndef _DPA_
/*
* Dynamic pointer array (GXDPA) implementation
*
* Copyright 1998 Eric Kohl
*           1998 Juergen Schmied <j.schmied@metronet.de>
*           2000 Eric Kohl for CodeWeavers
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
*
* NOTES
*     These functions were involuntarily documented by Microsoft in 2002 as
*     the outcome of an anti-trust suit brought by various U.S. governments.
*     As a result the specifications on MSDN are inaccurate, incomplete 
*     and misleading. A much more complete (unofficial) documentation is
*     available at:
*
*     http://members.ozemail.com.au/~geoffch/samples/win32/shell/comctl32  
*/

#define COBJMACROS

//#include <stdarg.h>
//#include <limits.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "winuser.h"
//#include "commctrl.h"
//#include "objbase.h"
//
//#include "comctl32.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(dpa);

#include <GrapX.H>
#include <User/GrapX.Hxx>
#include "Include/GXUser.H"
#include "Include/GXKernel.H"
#include "Include/GXGDI.H"
#include "Include/GXImm.h"
#include "GXCommCtrl.h"

#include <stdarg.h>
#include <stdlib.h>

#include <User/WineComm.H>
#include <User/Win32Emu/_dpa.H>

#ifdef TRACE
#undef TRACE
#define TRACE
#endif // TRACE

#ifdef TRACEA
#undef TRACEA
#define TRACEA
#endif // TRACEA

#ifdef TRACEW
#undef TRACEW
#define TRACEW
#endif // TRACEW


struct _GXDPA
{
  GXINT    nItemCount;
  GXLPVOID *ptrs;
  GXHANDLE hHeap;
  GXINT    nGrow;
  GXINT    nMaxCount;
};

typedef struct _STREAMDATA
{
  GXDWORD dwSize;
  GXDWORD dwData2;
  GXDWORD dwItems;
} STREAMDATA, *PSTREAMDATA;

typedef struct _LOADDATA
{
  GXINT   nCount;
  GXLPVOID ptr;
} LOADDATA, *LPLOADDATA;

//typedef GXHRESULT (CALLBACK *DPALOADPROC)(LPLOADDATA,IStream*,GXLPARAM);


/**************************************************************************
* gxDPA_LoadStream [COMCTL32.9]
*
* Loads a dynamic pointer array from a stream
*
* PARAMS
*     phDpa    [O] pointer to a handle to a dynamic pointer array
*     loadProc [I] pointer to a callback function
*     pStream  [I] pointer to a stream
*     lParam   [I] application specific value
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE 
*
* NOTES
*     No more information available yet!
*/
//GXHRESULT GXDLLAPI gxDPA_LoadStream (GXHDPA *phDpa, DPALOADPROC loadProc,
//                               IStream *pStream, GXLPARAM lParam)
//{
//    GXHRESULT errCode;
//    LARGE_INTEGER position;
//    ULARGE_INTEGER newPosition;
//    STREAMDATA  streamData;
//    LOADDATA loadData;
//    GXULONG ulRead;
//    GXHDPA hDpa;
//    GXLPVOID *ptr;
//
//    FIXME ("phDpa=%p loadProc=%p pStream=%p lParam=%lx\n",
//           phDpa, loadProc, pStream, lParam);
//
//    if (!phDpa || !loadProc || !pStream)
//        return E_INVALIDARG;
//
//    *phDpa = (GXHDPA)NULL;
//
//    position.QuadPart = 0;
//
//    /*
//     * Zero out our streamData
//     */
//    memset(&streamData,0,sizeof(STREAMDATA));
//
//    errCode = IStream_Seek (pStream, position, STREAM_SEEK_CUR, &newPosition);
//    if (errCode != GX_OK)
//        return errCode;
//
//    errCode = IStream_Read (pStream, &streamData, sizeof(STREAMDATA), &ulRead);
//    if (errCode != GX_OK)
//        return errCode;
//
//    FIXME ("dwSize=%u dwData2=%u dwItems=%u\n",
//           streamData.dwSize, streamData.dwData2, streamData.dwItems);
//
//    if ( ulRead < sizeof(STREAMDATA) ||
//    lParam < sizeof(STREAMDATA) ||
//        streamData.dwSize < sizeof(STREAMDATA) ||
//        streamData.dwData2 < 1) {
//        errCode = GX_FAIL;
//    }
//
//    if (streamData.dwItems > (UINT_MAX / 2 / sizeof(GXVOID*))) /* 536870911 */
//        return E_OUTOFMEMORY;
//
//    /* create the dpa */
//    hDpa = gxDPA_Create (streamData.dwItems);
//    if (!hDpa)
//        return E_OUTOFMEMORY;
//
//    if (!gxDPA_Grow (hDpa, streamData.dwItems))
//        return E_OUTOFMEMORY;
//
//    /* load data from the stream into the dpa */
//    ptr = hDpa->ptrs;
//    for (loadData.nCount = 0; loadData.nCount < streamData.dwItems; loadData.nCount++) {
//        errCode = (loadProc)(&loadData, pStream, lParam);
//        if (errCode != GX_OK) {
//            errCode = S_FALSE;
//            break;
//        }
//
//        *ptr = loadData.ptr;
//        ptr++;
//    }
//
//    /* set the number of items */
//    hDpa->nItemCount = loadData.nCount;
//
//    /* store the handle to the dpa */
//    *phDpa = hDpa;
//    FIXME ("new hDpa=%p, errorcode=%x\n", hDpa, errCode);
//
//    return errCode;
//}


/**************************************************************************
* gxDPA_SaveStream [COMCTL32.10]
*
* Saves a dynamic pointer array to a stream
*
* PARAMS
*     hDpa     [I] handle to a dynamic pointer array
*     loadProc [I] pointer to a callback function
*     pStream  [I] pointer to a stream
*     lParam   [I] application specific value
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE 
*
* NOTES
*     No more information available yet!
*/
//GXHRESULT GXDLLAPI gxDPA_SaveStream (const GXHDPA hDpa, DPALOADPROC loadProc,
//                  IStream *pStream, GXLPARAM lParam)
//{
//
//  FIXME ("hDpa=%p loadProc=%p pStream=%p lParam=%lx\n",
//    hDpa, loadProc, pStream, lParam);
//
//  return GX_FAIL;
//}


/**************************************************************************
* gxDPA_Merge [COMCTL32.11]
*
* Merge two dynamic pointers arrays.
*
* PARAMS
*     hdpa1       [I] handle to a dynamic pointer array
*     hdpa2       [I] handle to a dynamic pointer array
*     dwFlags     [I] flags
*     pfnCompare  [I] pointer to sort function
*     pfnMerge    [I] pointer to merge function
*     lParam      [I] application specific value
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE 
*
* NOTES
*     No more information available yet!
*/
GXBOOL GXDLLAPI gxDPA_Merge (const GXHDPA hdpa1, const GXHDPA hdpa2, GXDWORD dwFlags,
              GXPFNDPACOMPARE pfnCompare, GXPFNDPAMERGE pfnMerge,
              GXLPARAM lParam)
{
  GXINT nCount;
  GXLPVOID *pWork1, *pWork2;
  GXINT nResult, i;
  GXINT nIndex;

  TRACE("%p %p %08x %p %p %08lx)\n",
    hdpa1, hdpa2, dwFlags, pfnCompare, pfnMerge, lParam);

  //if (IsBadWritePtr (hdpa1, sizeof(*hdpa1)))
  //  return FALSE;

  //if (IsBadWritePtr (hdpa2, sizeof(*hdpa2)))
  //  return FALSE;

  //if (IsBadCodePtr ((FARPROC)pfnCompare))
  //  return FALSE;

  //if (IsBadCodePtr ((FARPROC)pfnMerge))
  //  return FALSE;

  if (!(dwFlags & GXDPAM_NOSORT)) {
    TRACE("sorting dpa's!\n");
    if (hdpa1->nItemCount > 0)
      gxDPA_Sort (hdpa1, pfnCompare, lParam);
    TRACE ("dpa 1 sorted!\n");
    if (hdpa2->nItemCount > 0)
      gxDPA_Sort (hdpa2, pfnCompare, lParam);
    TRACE ("dpa 2 sorted!\n");
  }

  if (hdpa2->nItemCount < 1)
    return TRUE;

  TRACE("hdpa1->nItemCount=%d hdpa2->nItemCount=%d\n",
    hdpa1->nItemCount, hdpa2->nItemCount);


  /* working but untrusted implementation */

  pWork1 = &(hdpa1->ptrs[hdpa1->nItemCount - 1]);
  pWork2 = &(hdpa2->ptrs[hdpa2->nItemCount - 1]);

  nIndex = hdpa1->nItemCount - 1;
  nCount = hdpa2->nItemCount - 1;

  do
  {
    if (nIndex < 0) {
      if ((nCount >= 0) && (dwFlags & GXDPAM_INSERT)) {
        /* Now insert the remaining new items into GXDPA 1 */
        TRACE("%d items to be inserted at start of GXDPA 1\n",
          nCount+1);
        for (i=nCount; i>=0; i--) {
          GXLPVOID ptr;

          ptr = (pfnMerge)(3, *pWork2, NULL, lParam);
          if (!ptr)
            return FALSE;
          gxDPA_InsertPtr (hdpa1, 0, ptr);
          pWork2--;
        }
      }
      break;
    }
    nResult = (pfnCompare)(*pWork1, *pWork2, lParam);
    TRACE("compare result=%d, dpa1.cnt=%d, dpa2.cnt=%d\n",
      nResult, nIndex, nCount);

    if (nResult == 0)
    {
      GXLPVOID ptr;

      ptr = (pfnMerge)(1, *pWork1, *pWork2, lParam);
      if (!ptr)
        return FALSE;

      nCount--;
      pWork2--;
      *pWork1 = ptr;
      nIndex--;
      pWork1--;
    }
    else if (nResult > 0)
    {
      /* item in GXDPA 1 missing from GXDPA 2 */
      if (dwFlags & GXDPAM_DELETE)
      {
        /* Now delete the extra item in DPA1 */
        GXLPVOID ptr;

        ptr = gxDPA_DeletePtr (hdpa1, hdpa1->nItemCount - 1);

        (pfnMerge)(2, ptr, NULL, lParam);
      }
      nIndex--;
      pWork1--;
    }
    else
    {
      /* new item in GXDPA 2 */
      if (dwFlags & GXDPAM_INSERT)
      {
        /* Now insert the new item in GXDPA 1 */
        GXLPVOID ptr;

        ptr = (pfnMerge)(3, *pWork2, NULL, lParam);
        if (!ptr)
          return FALSE;
        gxDPA_InsertPtr (hdpa1, nIndex+1, ptr);
      }
      nCount--;
      pWork2--;
    }

  }
  while (nCount >= 0);

  return TRUE;
}


/**************************************************************************
* gxDPA_Destroy [COMCTL32.329]
*
* Destroys a dynamic pointer array
*
* PARAMS
*     hdpa [I] handle (pointer) to the pointer array
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/
GXBOOL GXDLLAPI gxDPA_Destroy (const GXHDPA hdpa)
{
  TRACE("(%p)\n", hdpa);

  if (!hdpa)
    return FALSE;

  if (hdpa->ptrs && (!gxHeapFree (hdpa->hHeap, 0, hdpa->ptrs)))
    return FALSE;

  return gxHeapFree (hdpa->hHeap, 0, hdpa);
}


/**************************************************************************
* gxDPA_Grow [COMCTL32.330]
*
* Sets the growth amount.
*
* PARAMS
*     hdpa  [I] handle (pointer) to the existing (source) pointer array
*     nGrow [I] number of items by which the array grows when it's too small
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/
GXBOOL GXDLLAPI gxDPA_Grow (const GXHDPA hdpa, GXINT nGrow)
{
  TRACE("(%p %d)\n", hdpa, nGrow);

  if (!hdpa)
    return FALSE;

  hdpa->nGrow = max(8, nGrow);

  return TRUE;
}


/**************************************************************************
* gxDPA_Clone [COMCTL32.331]
*
* Copies a pointer array to an other one or creates a copy
*
* PARAMS
*     hdpa    [I] handle (pointer) to the existing (source) pointer array
*     hdpaNew [O] handle (pointer) to the destination pointer array
*
* RETURNS
*     Success: pointer to the destination pointer array.
*     Failure: NULL
*
* NOTES
*     - If the 'hdpaNew' is a NULL-Pointer, a copy of the source pointer
*       array will be created and it's handle (pointer) is returned.
*     - If 'hdpa' is a NULL-Pointer, the original implementation crashes,
*       this implementation just returns NULL.
*/
GXHDPA GXDLLAPI gxDPA_Clone (const GXHDPA hdpa, const GXHDPA hdpaNew)
{
  GXINT nNewItems, nSize;
  GXHDPA hdpaTemp;

  if (!hdpa)
    return NULL;

  TRACE("(%p %p)\n", hdpa, hdpaNew);

  if (!hdpaNew) {
    /* create a new GXDPA */
    hdpaTemp = (GXHDPA)gxHeapAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY,
      sizeof(*hdpaTemp));
    hdpaTemp->hHeap = hdpa->hHeap;
    hdpaTemp->nGrow = hdpa->nGrow;
  }
  else
    hdpaTemp = hdpaNew;

  if (hdpaTemp->ptrs) {
    /* remove old pointer array */
    gxHeapFree (hdpaTemp->hHeap, 0, hdpaTemp->ptrs);
    hdpaTemp->ptrs = NULL;
    hdpaTemp->nItemCount = 0;
    hdpaTemp->nMaxCount = 0;
  }

  /* create a new pointer array */
  nNewItems = hdpaTemp->nGrow *
    (((hdpa->nItemCount - 1) / hdpaTemp->nGrow) + 1);
  nSize = nNewItems * sizeof(GXLPVOID);
  hdpaTemp->ptrs = (GXLPVOID*)gxHeapAlloc (hdpaTemp->hHeap, GXHEAP_ZERO_MEMORY, nSize);
  hdpaTemp->nMaxCount = nNewItems;

  /* clone the pointer array */
  hdpaTemp->nItemCount = hdpa->nItemCount;
  memmove (hdpaTemp->ptrs, hdpa->ptrs,
    hdpaTemp->nItemCount * sizeof(GXLPVOID));

  return hdpaTemp;
}


/**************************************************************************
* gxDPA_GetPtr [COMCTL32.332]
*
* Retrieves a pointer from a dynamic pointer array
*
* PARAMS
*     hdpa   [I] handle (pointer) to the pointer array
*     nIndex [I] array index of the desired pointer
*
* RETURNS
*     Success: pointer
*     Failure: NULL
*/
GXLPVOID GXDLLAPI gxDPA_GetPtr (const GXHDPA hdpa, GXINT_PTR nIndex)
{
  TRACE("(%p %d)\n", hdpa, nIndex);

  if (!hdpa)
    return NULL;
  if (!hdpa->ptrs) {
    WARN("no pointer array.\n");
    return NULL;
  }
  if ((nIndex < 0) || (nIndex >= hdpa->nItemCount)) {
    WARN("not enough pointers in array (%d vs %d).\n",nIndex,hdpa->nItemCount);
    return NULL;
  }

  TRACE("-- %p\n", hdpa->ptrs[nIndex]);

  return hdpa->ptrs[nIndex];
}


/**************************************************************************
* gxDPA_GetPtrIndex [COMCTL32.333]
*
* Retrieves the index of the specified pointer
*
* PARAMS
*     hdpa   [I] handle (pointer) to the pointer array
*     p      [I] pointer
*
* RETURNS
*     Success: index of the specified pointer
*     Failure: -1
*/
GXINT GXDLLAPI gxDPA_GetPtrIndex (const GXHDPA hdpa, GXLPVOID p)
{
  GXINT i;

  if (!hdpa || !hdpa->ptrs)
    return -1;

  for (i = 0; i < hdpa->nItemCount; i++) {
    if (hdpa->ptrs[i] == p)
      return i;
  }

  return -1;
}


/**************************************************************************
* gxDPA_InsertPtr [COMCTL32.334]
*
* Inserts a pointer into a dynamic pointer array
*
* PARAMS
*     hdpa [I] handle (pointer) to the array
*     i    [I] array index
*     p    [I] pointer to insert
*
* RETURNS
*     Success: index of the inserted pointer
*     Failure: -1
*/
GXINT GXDLLAPI gxDPA_InsertPtr (const GXHDPA hdpa, GXINT i, GXLPVOID p)
{
  TRACE("(%p %d %p)\n", hdpa, i, p);

  if (!hdpa || i < 0) return -1;

  /* append item if index is out of bounds */
  i = min(hdpa->nItemCount, i);

  /* create empty spot at the end */
  if (!gxDPA_SetPtr(hdpa, hdpa->nItemCount, 0)) return -1;

  if (i != hdpa->nItemCount - 1)
    memmove (hdpa->ptrs + i + 1, hdpa->ptrs + i, 
    (hdpa->nItemCount - i - 1) * sizeof(GXLPVOID));

  hdpa->ptrs[i] = p;
  return i;
}


/**************************************************************************
* gxDPA_SetPtr [COMCTL32.335]
*
* Sets a pointer in the pointer array
*
* PARAMS
*     hdpa [I] handle (pointer) to the pointer array
*     i    [I] index of the pointer that will be set
*     p    [I] pointer to be set
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/
GXBOOL GXDLLAPI gxDPA_SetPtr (const GXHDPA hdpa, GXINT i, GXLPVOID p)
{
  GXLPVOID *lpTemp;  // TODO:为什么这个地方要用双重指针呢？

  TRACE("(%p %d %p)\n", hdpa, i, p);

  if (!hdpa || i < 0)
    return FALSE;

  if (hdpa->nItemCount <= i) {
    /* within the old array */
    if (hdpa->nMaxCount <= i) {
      /* resize the block of memory */
      GXINT nNewItems =
        hdpa->nGrow * ((((i+1) - 1) / hdpa->nGrow) + 1);
      GXINT nSize = nNewItems * sizeof(GXLPVOID);

      if (hdpa->ptrs)
        lpTemp = (GXLPVOID*)gxHeapReAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY, hdpa->ptrs, nSize);
      else
        lpTemp = (GXLPVOID*)gxHeapAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY, nSize);

      if (!lpTemp)
        return FALSE;

      hdpa->nMaxCount = nNewItems;
      hdpa->ptrs = lpTemp;
    }
    hdpa->nItemCount = i+1;
  }

  /* put the new entry in */
  hdpa->ptrs[i] = p;

  return TRUE;
}


/**************************************************************************
* gxDPA_DeletePtr [COMCTL32.336]
*
* Removes a pointer from the pointer array.
*
* PARAMS
*     hdpa [I] handle (pointer) to the pointer array
*     i    [I] index of the pointer that will be deleted
*
* RETURNS
*     Success: deleted pointer
*     Failure: NULL
*/
GXLPVOID GXDLLAPI gxDPA_DeletePtr (const GXHDPA hdpa, GXINT i)
{
  GXLPVOID *lpDest, *lpSrc, lpTemp = NULL;
  GXINT  nSize;

  TRACE("(%p %d)\n", hdpa, i);

  if ((!hdpa) || i < 0 || i >= hdpa->nItemCount)
    return NULL;

  lpTemp = hdpa->ptrs[i];

  /* do we need to move ?*/
  if (i < hdpa->nItemCount - 1) {
    lpDest = hdpa->ptrs + i;
    lpSrc = lpDest + 1;
    nSize = (hdpa->nItemCount - i - 1) * sizeof(GXLPVOID);
    TRACE("-- move dest=%p src=%p size=%x\n",
      lpDest, lpSrc, nSize);
    memmove (lpDest, lpSrc, nSize);
  }

  hdpa->nItemCount --;

  /* free memory ?*/
  if ((hdpa->nMaxCount - hdpa->nItemCount) >= hdpa->nGrow) {
    GXINT nNewItems = max(hdpa->nGrow * 2, hdpa->nItemCount);
    nSize = nNewItems * sizeof(GXLPVOID);
    lpDest = (GXLPVOID*)gxHeapReAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY,
      hdpa->ptrs, nSize);
    if (!lpDest)
      return NULL;

    hdpa->nMaxCount = nNewItems;
    hdpa->ptrs = lpDest;
  }

  return lpTemp;
}


/**************************************************************************
* gxDPA_DeleteAllPtrs [COMCTL32.337]
*
* Removes all pointers and reinitializes the array.
*
* PARAMS
*     hdpa [I] handle (pointer) to the pointer array
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/
GXBOOL GXDLLAPI gxDPA_DeleteAllPtrs (const GXHDPA hdpa)
{
  TRACE("(%p)\n", hdpa);

  if (!hdpa)
    return FALSE;

  if (hdpa->ptrs && (!gxHeapFree (hdpa->hHeap, 0, hdpa->ptrs)))
    return FALSE;

  hdpa->nItemCount = 0;
  hdpa->nMaxCount = hdpa->nGrow * 2;
  hdpa->ptrs = (GXLPVOID*)gxHeapAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY,
    hdpa->nMaxCount * sizeof(GXLPVOID));

  return TRUE;
}


/**************************************************************************
* gxDPA_QuickSort [Internal]
*
* Ordinary quicksort (used by gxDPA_Sort).
*
* PARAMS
*     lpPtrs     [I] pointer to the pointer array
*     l          [I] index of the "left border" of the partition
*     r          [I] index of the "right border" of the partition
*     pfnCompare [I] pointer to the compare function
*     lParam     [I] user defined value (3rd parameter in compare function)
*
* RETURNS
*     NONE
*/
static GXVOID gxDPA_QuickSort (GXLPVOID *lpPtrs, GXINT l, GXINT r,
               GXPFNDPACOMPARE pfnCompare, GXLPARAM lParam)
{
  GXINT m;
  GXLPVOID t;

  TRACE("l=%i r=%i\n", l, r);

  if (l==r)    /* one element is always sorted */
    return;
  if (r<l)     /* oops, got it in the wrong order */
  {
    gxDPA_QuickSort(lpPtrs, r, l, pfnCompare, lParam);
    return;
  }
  m = (l+r)/2; /* divide by two */
  gxDPA_QuickSort(lpPtrs, l, m, pfnCompare, lParam);
  gxDPA_QuickSort(lpPtrs, m+1, r, pfnCompare, lParam);

  /* join the two sides */
  while( (l<=m) && (m<r) )
  {
    if(pfnCompare(lpPtrs[l],lpPtrs[m+1],lParam)>0)
    {
      t = lpPtrs[m+1];
      memmove(&lpPtrs[l+1],&lpPtrs[l],(m-l+1)*sizeof(lpPtrs[l]));
      lpPtrs[l] = t;

      m++;
    }
    l++;
  }
}


/**************************************************************************
* gxDPA_Sort [COMCTL32.338]
*
* Sorts a pointer array using a user defined compare function
*
* PARAMS
*     hdpa       [I] handle (pointer) to the pointer array
*     pfnCompare [I] pointer to the compare function
*     lParam     [I] user defined value (3rd parameter of compare function)
*
* RETURNS
*     Success: TRUE
*     Failure: FALSE
*/
GXBOOL GXDLLAPI gxDPA_Sort (const GXHDPA hdpa, GXPFNDPACOMPARE pfnCompare, GXLPARAM lParam)
{
  if (!hdpa || !pfnCompare)
    return FALSE;

  TRACE("(%p %p 0x%lx)\n", hdpa, pfnCompare, lParam);

  if ((hdpa->nItemCount > 1) && (hdpa->ptrs))
    gxDPA_QuickSort (hdpa->ptrs, 0, hdpa->nItemCount - 1,
    pfnCompare, lParam);

  return TRUE;
}


/**************************************************************************
* gxDPA_Search [COMCTL32.339]
*
* Searches a pointer array for a specified pointer
*
* PARAMS
*     hdpa       [I] handle (pointer) to the pointer array
*     pFind      [I] pointer to search for
*     nStart     [I] start index
*     pfnCompare [I] pointer to the compare function
*     lParam     [I] user defined value (3rd parameter of compare function)
*     uOptions   [I] search options
*
* RETURNS
*     Success: index of the pointer in the array.
*     Failure: -1
*/
GXINT GXDLLAPI gxDPA_Search (const GXHDPA hdpa, GXLPVOID pFind, GXINT nStart,
            GXPFNDPACOMPARE pfnCompare, GXLPARAM lParam, GXUINT uOptions)
{
  if (!hdpa || !pfnCompare || !pFind)
    return -1;

  TRACE("(%p %p %d %p 0x%08lx 0x%08x)\n",
    hdpa, pFind, nStart, pfnCompare, lParam, uOptions);

  if (uOptions & GXDPAS_SORTED) {
    /* array is sorted --> use binary search */
    GXINT l, r, x, n;
    GXLPVOID *lpPtr;

    l = (nStart == -1) ? 0 : nStart;
    r = hdpa->nItemCount - 1;
    lpPtr = hdpa->ptrs;
    while (r >= l) {
      x = (l + r) / 2;
      n = (pfnCompare)(pFind, lpPtr[x], lParam);
      if (n == 0)
        return x;
      else if (n < 0)
        r = x - 1;
      else /* (n > 0) */
        l = x + 1;
    }
    if (uOptions & (GXDPAS_INSERTBEFORE|GXDPAS_INSERTAFTER)) return l;
  }
  else {
    /* array is not sorted --> use linear search */
    GXLPVOID *lpPtr;
    GXINT  nIndex;

    nIndex = (nStart == -1)? 0 : nStart;
    lpPtr = hdpa->ptrs;
    for (; nIndex < hdpa->nItemCount; nIndex++) {
      if ((pfnCompare)(pFind, lpPtr[nIndex], lParam) == 0)
        return nIndex;
    }
  }

  return -1;
}


/**************************************************************************
* gxDPA_CreateEx [COMCTL32.340]
*
* Creates a dynamic pointer array using the specified size and heap.
*
* PARAMS
*     nGrow [I] number of items by which the array grows when it is filled
*     hHeap [I] handle to the heap where the array is stored
*
* RETURNS
*     Success: handle (pointer) to the pointer array.
*     Failure: NULL
*
* NOTES
*     The gxDPA_ functions can be used to create and manipulate arrays of
*     pointers.
*/
GXHDPA GXDLLAPI gxDPA_CreateEx (GXINT nGrow, GXHANDLE hHeap)
{
  GXHDPA hdpa;

  TRACE("(%d %p)\n", nGrow, hHeap);

  if (hHeap)
    hdpa = (GXHDPA)gxHeapAlloc (hHeap, GXHEAP_ZERO_MEMORY, sizeof(*hdpa));
  else
    hdpa = (GXHDPA)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, sizeof(*hdpa));

  if (hdpa) {
    hdpa->nGrow = max(8, nGrow);
    hdpa->hHeap = hHeap ? hHeap : gxGetProcessHeap();
    hdpa->nMaxCount = hdpa->nGrow * 2;
    hdpa->ptrs = (GXLPVOID*)gxHeapAlloc (hdpa->hHeap, GXHEAP_ZERO_MEMORY,
      hdpa->nMaxCount * sizeof(GXLPVOID));
  }

  TRACE("-- %p\n", hdpa);

  return hdpa;
}


/**************************************************************************
* gxDPA_Create [COMCTL32.328]
*
* Creates a dynamic pointer array.
*
* PARAMS
*     nGrow [I] number of items by which the array grows when it is filled
*
* RETURNS
*     Success: handle (pointer) to the pointer array.
*     Failure: NULL
*
* NOTES
*     The gxDPA_ functions can be used to create and manipulate arrays of
*     pointers.
*/
GXHDPA GXDLLAPI gxDPA_Create (GXINT nGrow)
{
  return gxDPA_CreateEx( nGrow, 0 );
}


/**************************************************************************
* gxDPA_EnumCallback [COMCTL32.385]
*
* Enumerates all items in a dynamic pointer array.
*
* PARAMS
*     hdpa     [I] handle to the dynamic pointer array
*     enumProc [I]
*     lParam   [I]
*
* RETURNS
*     none
*/
GXVOID GXDLLAPI gxDPA_EnumCallback (GXHDPA hdpa, GXPFNDPAENUMCALLBACK enumProc,
                 GXLPVOID lParam)
{
  GXINT i;

  TRACE("(%p %p %p)\n", hdpa, enumProc, lParam);

  if (!hdpa)
    return;
  if (hdpa->nItemCount <= 0)
    return;

  for (i = 0; i < hdpa->nItemCount; i++) {
    if ((enumProc)(hdpa->ptrs[i], lParam) == 0)
      return;
  }

  return;
}


/**************************************************************************
* gxDPA_DestroyCallback [COMCTL32.386]
*
* Enumerates all items in a dynamic pointer array and destroys it.
*
* PARAMS
*     hdpa     [I] handle to the dynamic pointer array
*     enumProc [I]
*     lParam   [I]
*
* RETURNS
*     none
*/
void GXDLLAPI gxDPA_DestroyCallback (GXHDPA hdpa, GXPFNDPAENUMCALLBACK enumProc,
                  GXLPVOID lParam)
{
  TRACE("(%p %p %p)\n", hdpa, enumProc, lParam);

  gxDPA_EnumCallback (hdpa, enumProc, lParam);
  gxDPA_Destroy (hdpa);
}
#endif // _DPA_