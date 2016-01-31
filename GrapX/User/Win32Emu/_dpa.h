#ifndef _GRAP_X_DYNAMIC_POINTER_ARRAY_H_
#define _GRAP_X_DYNAMIC_POINTER_ARRAY_H_

typedef int    (GXCALLBACK *GXPFNDPAENUMCALLBACK)(void *p, void *pData);
typedef int    (GXCALLBACK *GXPFNDPACOMPARE)(void *p1, void *p2, GXLPARAM lParam);
typedef GXLPVOID  (GXCALLBACK *GXPFNDPAMERGE)(GXDWORD,GXLPVOID,GXLPVOID,GXLPARAM);

GXHDPA   GXDLLAPI gxDPA_Create          (int cItemGrow);
GXBOOL   GXDLLAPI gxDPA_Destroy         (GXHDPA hdpa);
GXLPVOID GXDLLAPI gxDPA_DeletePtr       (GXHDPA hdpa, int i);
GXBOOL   GXDLLAPI gxDPA_DeleteAllPtrs   (GXHDPA hdpa);
void     GXDLLAPI gxDPA_EnumCallback    (GXHDPA hdpa, GXPFNDPAENUMCALLBACK pfnCB, void *pData);
void     GXDLLAPI gxDPA_DestroyCallback (GXHDPA hdpa, GXPFNDPAENUMCALLBACK pfnCB, void *pData);
GXBOOL   GXDLLAPI gxDPA_SetPtr          (GXHDPA hdpa, int i, void *p);
int      GXDLLAPI gxDPA_InsertPtr       (GXHDPA hdpa, int i, void *p);
GXLPVOID GXDLLAPI gxDPA_GetPtr          (GXHDPA hdpa, GXINT_PTR i);
GXBOOL   GXDLLAPI gxDPA_Grow            (const GXHDPA hdpa, GXINT nGrow);
GXBOOL   GXDLLAPI gxDPA_Sort            (const GXHDPA hdpa, GXPFNDPACOMPARE pfnCompare, GXLPARAM lParam);
GXINT    GXDLLAPI gxDPA_Search          (const GXHDPA hdpa, GXLPVOID pFind, GXINT nStart,GXPFNDPACOMPARE pfnCompare, GXLPARAM lParam, GXUINT uOptions);
GXINT    GXDLLAPI gxDPA_GetPtrIndex     (const GXHDPA hdpa, GXLPVOID p);

#define gxDPA_GetPtrCount(hdpa)  (*(GXINT*)(hdpa))

#define GXDPAM_NOSORT             0x0001
#define GXDPAM_INSERT             0x0004
#define GXDPAM_DELETE             0x0008

#define GXDPAS_SORTED             0x0001
#define GXDPAS_INSERTBEFORE       0x0002
#define GXDPAS_INSERTAFTER        0x0004

#endif // _GRAP_X_DYNAMIC_POINTER_ARRAY_H_