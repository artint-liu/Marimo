#include "GrapX.h"
#include "GrapX.Hxx"

#include "clStringSet.h"
#include "clStaticStringsDict.h"

#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataPoolIterator.h"
#include "GrapX/GXKernel.h"

#include "DataPoolImpl.h"
#include "DataPoolBuildTime.h"

#include "GXStation.h"
using namespace clstd;

namespace Marimo
{
  typedef DataPoolVariable Variable;
  DataPoolTypeClass operator&(DataPoolTypeClass a, DataPoolTypeClass b);
  b32 operator==(DataPoolTypeClass a, GXUINT b);

  namespace DataPoolInternal
  {
    GXBOOL IntCreateSubPool(DataPool** ppSubPool, DataPoolImpl* pReference);
  } // namespace DataPoolInternal


  DataPoolImpl_SubPool::DataPoolImpl_SubPool(DataPoolImpl* pDataPool)
    : m_pReference(pDataPool)
  {
    m_buffer.Resize(pDataPool->GetRootSize(), TRUE);    
    //TRACE("sizeof(DataPoolImpl_SubPool):%u\n", sizeof(DataPoolImpl_SubPool));
  }

#define _HIDING_CODE_
  typedef DataPoolImpl::VARIABLE VARIABLE;
  typedef DataPoolImpl::STRUCT_DESC STRUCT_DESC;
  //#define VARIABLE          DataPoolImpl::VARIABLE

#define m_Name            m_pReference->m_Name
#define m_Buffer          m_pReference->m_Buffer
#define m_nNumOfTypes     m_pReference->m_nNumOfTypes
#define m_nNumOfStructs   m_pReference->m_nNumOfStructs
#define m_nNumOfVar       m_pReference->m_nNumOfVar
#define m_nNumOfMember    m_pReference->m_nNumOfMember
#define m_nNumOfEnums     m_pReference->m_nNumOfEnums
#define m_cbHashBuckets   m_pReference->m_cbHashBuckets
#define m_aTypes          m_pReference->m_aTypes
#define m_aStructs        m_pReference->m_aStructs
#define m_aHashAlgos      m_pReference->m_aHashAlgos
#define m_aHashBuckets    m_pReference->m_aHashBuckets
#define m_aGSIT           m_pReference->m_aGSIT
#define m_aVariables      m_pReference->m_aVariables
#define m_aMembers        m_pReference->m_aMembers
#define m_aEnums          m_pReference->m_aEnums
#define m_pNamesTabBegin  m_pReference->m_pNamesTabBegin
#define m_pNamesTabEnd    m_pReference->m_pNamesTabEnd
#define m_dwRuntimeFlags  m_pReference->m_dwRuntimeFlags
#define m_FixedDict       m_pReference->m_FixedDict
#define m_WatchableArray  m_pReference->m_WatchableArray
#define m_ImpulsingSet    m_pReference->m_ImpulsingSet
#define m_FixedDict       m_pReference->m_FixedDict
#define m_WatchableArray  m_pReference->m_WatchableArray

#define m_VarBuffer       m_buffer
#define DataPoolImpl      DataPoolImpl_SubPool
#define VARIABLE_DATAPOOL_OBJECT m_pReference
#include "DataPoolImpl.cpp"

  //////////////////////////////////////////////////////////////////////////

  GXBOOL DataPoolImpl_SubPool::CreateSubPool (DataPool** ppSubPool)
  {
    return DataPoolInternal::IntCreateSubPool(ppSubPool, m_pReference);
  }

} // namespace Marimo