#ifndef _GRAPX_RESOURCE_DATA_MANAGER_
#define _GRAPX_RESOURCE_DATA_MANAGER_

#define GXRESDATA_STRMAP      1
#define GXRESDATA_PVOIDMAP    2
//#define GXRESDATA_HASHMAP    2

class GrapXResData
{
public:
  virtual GXBOOL SetPtr(GXLPVOID ptr, GXLPVOID pHandle) = NULL;
  virtual GXBOOL GetPtr(GXLPVOID ptr, GXLPVOID* ppHandle) = NULL;
  virtual GXBOOL Remove(GXLPVOID ptr) = NULL;
  virtual GXVOID Release() = NULL;
  virtual GXSIZE_T Count() const = NULL;
};

typedef void* HGXRESDATA;

GrapXResData* GrapXResData_Create(GXUINT uType);
GXVOID GrapXResData_Destroy(GrapXResData* pResData);
//GXBOOL GrapXResData_SetPtr(HGXRESDATA hResData, GXLPVOID ptr, GXLPVOID pHandle);
//GXBOOL GrapXResData_GetPtr(HGXRESDATA hResData, GXLPVOID ptr, GXLPVOID* ppHandle);
//GXBOOL GrapXResData_Remove(HGXRESDATA hResData, GXLPVOID ptr);
//GXSIZE_T GrapXResData_Count(HGXRESDATA hResData);


#endif // _GRAPX_RESOURCE_DATA_MANAGER_