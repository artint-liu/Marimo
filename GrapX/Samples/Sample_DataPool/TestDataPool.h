#ifndef _TEST_DATAPOOL_H_
#define _TEST_DATAPOOL_H_

using namespace Marimo;

extern GXLPCWSTR s_szExampleString[];
void EnumerateVariables(DataPool* pDataPool);
void EnumeratePtrControl(DataPool* pDataPool);
void EnumeratePtrControl2(DataPool* pDataPool);

void CompareVariable(const DataPoolVariable& var1, const DataPoolVariable& var2);
void CompareDataPool(DataPool* pDataPoolA, DataPool* pDataPoolB);

#define ENUM_DATAPOOL(x)   EnumerateVariables(x)
//#define ENUM_DATAPOOL(x)  EnumeratePtrControl(x)
//#define ENUM_DATAPOOL(x)  EnumeratePtrControl2(x)

typedef DataPoolUtility::named_iterator DataPoolIterator;
typedef DataPoolUtility::named_element_iterator DataPoolElementIterator;

#endif // _TEST_DATAPOOL_H_