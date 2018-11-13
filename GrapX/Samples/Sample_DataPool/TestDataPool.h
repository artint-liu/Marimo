#ifndef _TEST_DATAPOOL_H_
#define _TEST_DATAPOOL_H_

using namespace Marimo;

extern GXLPCWSTR s_szExampleString[];
size_t EnumerateVariables(DataPool* pDataPool);
size_t EnumeratePtrControl(DataPool* pDataPool);
void EnumeratePtrControl2(DataPool* pDataPool);
void TestSlidingArray(DataPool* pDataPool, DataPoolVariable& varArray, GXLPCSTR szVarFormat);

void CompareVariable(const DataPoolVariable& var1, const DataPoolVariable& var2);
size_t CompareDataPool(DataPool* pDataPoolA, DataPool* pDataPoolB);

#define ENUM_DATAPOOL(x)   EnumerateVariables(x)
//#define ENUM_DATAPOOL(x)  EnumeratePtrControl(x)
//#define ENUM_DATAPOOL(x)  EnumeratePtrControl2(x)

typedef DataPoolUtility::named_iterator DataPoolIterator;
typedef DataPoolUtility::named_element_iterator DataPoolElementIterator;

#endif // _TEST_DATAPOOL_H_