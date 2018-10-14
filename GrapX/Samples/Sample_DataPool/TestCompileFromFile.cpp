#include <tchar.h>
#include <Marimo.H>
#include "GrapX/DataPool.H"
#include "TestObject.h"
#include "clPathFile.h"
#include "TestDataPool.h"
#define HUGE_NUMBER 10000


// ������Ҫ�������Խϴ������ʹ�����

void TestCompileFromFile()
{
  DataPool* pDataPool = NULL;
  
  clStringW strMainFile = L"Test\\DataPool\\main.txt";
  clpathfile::CombineAbsPathT(strMainFile);
  clstd::TimeTrace tt;
  tt.Begin();
  GXHRESULT hval = DataPool::CompileFromFileW(&pDataPool, NULL, strMainFile, NULL);
  ASSERT(GXSUCCEEDED(hval));
  tt.End().Dump("����ʱ��");

  if(GXSUCCEEDED(hval))
  {
    MOVariable varEssence;
    MOVariable varEquipType;
    MOVariable varEquipMask;
    GXBOOL bval;
    bval = pDataPool->QueryByExpression("equipment_essence_raw", &varEssence);
    ASSERT(bval);

    bval = pDataPool->QueryByExpression("equipment_essence_raw.equip_type", &varEquipType);
    ASSERT( ! bval); // ���Է��ʶ�̬����ĳ�Ա����
    

    bval = pDataPool->QueryByExpression("equipment_essence_raw[0].equip_type", &varEquipType);
    ASSERT(bval);

    // ����ö��
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_WEAPON");
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    
    // ö�ٽ���
    varEquipType.Set("EQUIPTYPE_ARMOR");
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_ARMOR");
    
    varEquipType.Set("2");      // ʮ�������ֽ���
    TRACE("equipment_essence_raw.equip_type = %s\n", varEquipType.ToStringA());
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_FASHION");

    varEquipType.Set("0b11");   // ���������ֽ���
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_FACE");

    varEquipType.Set("0x04");   // 16�������ֽ���
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_HANGMATTER");

    varEquipType.Set("05");     // 8�������ֽ���
    ASSERT(varEquipType.ToStringA() == "EQUIPTYPE_CLOAK");


    bval = pDataPool->QueryByExpression("equipment_essence_raw.equip_mask", &varEquipMask);
    ASSERT( ! bval); // ���Է��ʶ�̬����ĳ�Ա����

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

    // ���Լ��Ƚϣ����ԱȽ��㷨
    CompareDataPool(pDataPool, pDataPool);

    // �ͼ����ļ��Ƚϣ������ļ���д�㷨
    CompareDataPool(pDataPool, pDataPoolReadOnly);
    CompareDataPool(pDataPool, pDataPoolFromFile);

    ENUM_DATAPOOL(pDataPool);
    ENUM_DATAPOOL(pDataPoolReadOnly);

    SAFE_RELEASE(pDataPool);
    SAFE_RELEASE(pDataPoolReadOnly);
    SAFE_RELEASE(pDataPoolFromFile);

  }
}
