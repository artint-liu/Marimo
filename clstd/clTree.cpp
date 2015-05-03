//#include "stdafx.h"
//#include <clTypes.H>
#include "clMemory.h"
#include "clStd.H"
#include "clTree.H"

namespace clstd
{
  Tree::Tree()
    :m_pParent(NULL)
    ,m_pFirstChild(NULL)
    ,m_pPrev(NULL)
    ,m_pNext(NULL)
  {
  }

  Tree::~Tree()
  {
    ASSERT(m_pFirstChild == NULL && m_pNext == NULL);
  }

  bool Tree::PickNode()
  {
    if(m_pPrev == NULL)
    {
      if(m_pParent != NULL)
        m_pParent->m_pFirstChild = m_pNext;
      if(m_pNext != NULL)
        m_pNext->m_pPrev = NULL;
    }
    else
    {
      m_pPrev->m_pNext = m_pNext;
      if(m_pNext != NULL)
      {
        m_pNext->m_pPrev = m_pPrev;
      }
    }
    m_pParent = NULL;
    m_pPrev = NULL;
    m_pNext = NULL;
    return true;
  }

  Tree* Tree::GetParent() const
  {
    return m_pParent;
  }
  Tree* Tree::GetFirstChild() const
  {
    return m_pFirstChild;
  }
  Tree* Tree::GetNext() const
  {
    return m_pNext;
  }
  Tree* Tree::GetPrev() const
  {
    return m_pPrev;
  }

  Tree* Tree::SetParent(Tree *pNewParent, bool bLast)
  {
    PickNode();
    if(pNewParent == NULL)
      return NULL;
    m_pParent = pNewParent;
    if(bLast == false)
    {
      m_pNext = pNewParent->m_pFirstChild;
      if(pNewParent->m_pFirstChild != NULL)
      {
        pNewParent->m_pFirstChild->m_pPrev = this;
      }
      pNewParent->m_pFirstChild = this;
    }
    else
    {
      Tree *pItem = pNewParent->m_pFirstChild;
      if(pItem == NULL)
      {
        pNewParent->m_pFirstChild = this;
        return pNewParent;
      }
      while(pItem->m_pNext != NULL)  pItem = pItem->m_pNext;
      pItem->m_pNext = this;
      m_pPrev = pItem;
    }
    return pNewParent;
  }

  // ɾ���ڵ�
  bool Tree::DestroyNode()
  {
    while(m_pFirstChild != NULL)
    {
      m_pFirstChild->DestroyNode();
    }
    PickNode();
    Tree *pThis = this;
    SAFE_DELETE(pThis);
    return true;
  }

  // bool Tree::InsertAfter(Tree *pAfter, bool bForward);
  // �����ڵ�˳��
  // ����:  pAfter    ����Ĳο��ڵ�
  //      bForward  �������bForwardΪfalse����ǰ�ڵ���뵽Ŀ��ڵ�ĺ��棬������뵽��ǰ�ڵ��ǰ��
  // ��ע: ��������˳����Next����Ϊ���棬��֮��ǰ
  bool Tree::InsertAfter(Tree *pAfter, bool bForward)
  {
    if(pAfter == NULL)
      return false;
    PickNode();

    if(bForward == false)
    {
      m_pParent = pAfter->m_pParent;
      m_pNext = pAfter->m_pNext;
      m_pPrev = pAfter;

      if(pAfter->m_pNext != NULL)
        pAfter->m_pNext->m_pPrev = this;

      pAfter->m_pNext = this;

    }
    else
    {
      m_pParent = pAfter->m_pParent;
      m_pNext = pAfter;
      m_pPrev = pAfter->m_pPrev;

      if(pAfter->m_pPrev != NULL)
        pAfter->m_pPrev->m_pNext = this;
      else
        pAfter->m_pParent->m_pFirstChild = this;

      pAfter->m_pPrev = this;
    }
    return true;
  }
}
