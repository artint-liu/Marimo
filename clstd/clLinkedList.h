#ifndef _CLSTD_LINKED_LIST_H_
#define _CLSTD_LINKED_LIST_H_

namespace clstd
{
  // _Ty 至少要包含pNext成员变量
  template<class _Ty>
  class LinkedList
  {
    _Ty* m_pFront;
    _Ty* m_pBack;
    size_t m_count;

  public:
    LinkedList();
    LinkedList(_Ty* pItem);

    void    push_back(_Ty* pItem);
    _Ty*    pop_front();
    _Ty*    front() const;
    size_t  size() const;
    b32     empty() const;
    void    clear();
    b32     erase(const _Ty* pItem);
    _Ty*    find(const _Ty* pItem); // 返回值是pItem的前序Item，如果返回pItem本身表示pItem是第一个
  };

} // namespace clstd

#endif // _CLSTD_LINKED_LIST_H_