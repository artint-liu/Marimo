#ifndef _TREE_H_
#define _TREE_H_

namespace clstd
{
  class Tree
  {
  protected:
    Tree  *  m_pParent;
    Tree  *  m_pFirstChild;
    Tree  *  m_pPrev;
    Tree  *  m_pNext;

    bool  PickNode      ();  // 从Tree中摘除节点
  public:
    Tree* GetParent     () const;
    Tree* GetFirstChild () const;
    Tree* GetNext       () const;
    Tree* GetPrev       () const;
    Tree*  SetParent     (Tree *pNewParent, bool bLast = false);
    bool  DestroyNode   ();
    bool  InsertAfter   (Tree *pAfter, bool bForward);
  public:
    Tree();
    virtual ~Tree();
  };

  template<class _T>
  class treeT
  {
  protected:
    _T*  m_pParent;
    _T*  m_pFirstChild;
    _T*  m_pPrev;
    _T*  m_pNext;

    bool PickNode()  // 从Tree中摘除节点
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
  public:
    _T* GetParent() const
    {
      return m_pParent;
    }
    
    _T* GetFirstChild() const
    {
      return m_pFirstChild;
    }

    _T* GetNext() const
    {
      return m_pNext;
    }
    
    _T* GetPrev() const
    {
      return m_pPrev;
    }
    
    _T* SetParent(_T* pNewParent, bool bLast = false)
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
          pNewParent->m_pFirstChild->m_pPrev = static_cast<_T*>(this);
        }
        pNewParent->m_pFirstChild = static_cast<_T*>(this);
      }
      else
      {
        _T* pItem = pNewParent->m_pFirstChild;
        if(pItem == NULL)
        {
          pNewParent->m_pFirstChild = static_cast<_T*>(this);
          return pNewParent;
        }
        while(pItem->m_pNext != NULL)  pItem = pItem->m_pNext;
        pItem->m_pNext = static_cast<_T*>(this);
        m_pPrev = pItem;
      }
      return pNewParent;
    }

    bool IsAncestorOf(const _T* pChild) const // 如果参数是自己返回 false
    {
      const _T* pNode = pChild->m_pParent;

      while(pNode)
      {
        if(pNode == this) {
          return true;
        }
        pNode = pNode->m_pParent;
      }
      return false;
    }

    bool DestroyNode()
    {
      while(m_pFirstChild != NULL)
      {
        m_pFirstChild->DestroyNode();
      }
      PickNode();
      _T* pThis = static_cast<_T*>(this);
      SAFE_DELETE(pThis);
      return true;
    }
    bool InsertAfter(_T* pAfter, bool bForward)
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
          pAfter->m_pNext->m_pPrev = static_cast<_T*>(this);

        pAfter->m_pNext = static_cast<_T*>(this);

      }
      else
      {
        m_pParent = pAfter->m_pParent;
        m_pNext = pAfter;
        m_pPrev = pAfter->m_pPrev;

        if(pAfter->m_pPrev != NULL)
          pAfter->m_pPrev->m_pNext = static_cast<_T*>(this);
        else
          pAfter->m_pParent->m_pFirstChild = static_cast<_T*>(this);

        pAfter->m_pPrev = static_cast<_T*>(this);
      }
      return true;
    }
  public:
    treeT() : m_pParent(NULL), m_pFirstChild(NULL), m_pPrev(NULL), m_pNext(NULL){}
    virtual ~treeT(){}
  };

}
typedef clstd::Tree Tree;
#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _TREE_H_