#include <tchar.h>
#include <Marimo.H>
#include "GrapX/DataPool.H"
#include "TestObject.h"
#include "clPathFile.h"
#include "TestDataPool.h"
#define HUGE_NUMBER 10000


// 这里主要用来测试较大的数组使用情况

void TestCompileFromFile()
{
  DataPool* pDataPool = NULL;
  
  clStringW strMainFile = L"Test\\DataPool\\main.txt";
  clpathfile::CombineAbsPathT(strMainFile);
  clstd::TimeTrace tt;
  tt.Begin();
  GXHRESULT hval = DataPool::CompileFromFileW(&pDataPool, NULL, strMainFile, NULL);
  ASSERT(GXSUCCEEDED(hval));
  tt.End().Dump("编译时间");

  if(GXSUCCEEDED(hval))
  {
    MOVariable varEssence;
    MOVariable varEquipType;
    MOVariable varEquipMask;
    GXBOOL bval;
    bval = pDataPool->QueryByExpression("equipment_essence_raw", &varEssence);
    ASSERT(bval);

    bval = pDataPool->QueryByExpression("equipment_essence_raw.equip_type", &varEquipType);
    ASSERT( ! bval); // 尝试访问动态数组的成员变量
    

    bval = pDataPool->QueryByExpression("equipment_essence_raw[0].equip_type", &varEquipType);
    ASSERT(bval);

    // 测试枚举
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_WEAPON");
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    
    // 枚举解析
    varEquipType.Set("EQUIPTYPE_ARMOR");
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_ARMOR");
    
    varEquipType.Set("2");      // 十进制数字解析
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_FASHION");

    varEquipType.Set("0b11");   // 二进制数字解析
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_FACE");

    varEquipType.Set("0x04");   // 16进制数字解析
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_HANGMATTER");

    varEquipType.Set("05");     // 8进制数字解析
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_CLOAK");


    bval = pDataPool->QueryByExpression("equipment_essence_raw.equip_mask", &varEquipMask);
    ASSERT( ! bval); // 尝试访问动态数组的成员变量

    bval = pDataPool->QueryByExpression("equipment_essence_raw[0].equip_mask", &varEquipMask);
    ASSERT(bval);
    varEquipMask.Set(0xf);
    TRACE("equipment_essence_raw.equip_mask = %s\n", varEquipMask.ToStringA());

#ifdef _X86
    GXLPCWSTR szFilename = L"Test\\TestCompileFromFile.DPL";
#else
    GXLPCWSTR szFilename = L"Test\\TestCompileFromFile_x64.DPL";
#endif // #ifdef _X86
    pDataPool->SaveW(szFilename);

    DataPool* pDataPoolReadOnly = NULL;
    DataPool::CreateFromFileW(&pDataPoolReadOnly, NULL, szFilename, DataPoolCreation_ReadOnly);

    DataPool* pDataPoolFromFile = NULL;
    DataPool::CreateFromFileW(&pDataPoolFromFile, NULL, szFilename, DataPoolCreation_Default);

    //pDataPool->begin()

    // 和自己比较，测试比较算法
    CompareDataPool(pDataPool, pDataPool);

    // 和加载文件比较，测试文件读写算法
    CompareDataPool(pDataPool, pDataPoolReadOnly);
    CompareDataPool(pDataPool, pDataPoolFromFile);

    ENUM_DATAPOOL(pDataPool);
    ENUM_DATAPOOL(pDataPoolReadOnly);

    SAFE_RELEASE(pDataPool);
    SAFE_RELEASE(pDataPoolReadOnly);
    SAFE_RELEASE(pDataPoolFromFile);

  }
}
