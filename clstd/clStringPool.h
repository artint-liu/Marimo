#ifndef _CLSTD_STRING_POOL_H_
#define _CLSTD_STRING_POOL_H_

#ifndef _CL_BUFFER_H_
# error include clBuffer.h first
#endif

// �ַ�����
// ���ڱ����ַ��������ڴ浼�µ��ڴ���Ƭ��, ������Ҫע��Map��Ȼ���ܵ����ڴ���Ƭ��

namespace clstd
{
    template<class _TString, class _TMap = clmap<_TString, size_t> >
    class StringPoolT : _TMap // ����public�̳�
    {
        MemBuffer m_buffer;
    public:
        size_t Add(typename _TString::LPCSTR str) // ����ַ�����������ƫ��
        {
            auto it = _TMap::find(str);
            if (it != _TMap::end())
            {
                return it->second;
            }
            size_t offset = m_buffer.GetSize();
            size_t size = clstd::strlenT(str);
            m_buffer.Append(str, size + 1);
            _TMap::insert(clmake_pair(str, offset));
            return offset;
        }

        typename _TString::LPCSTR Ptr(size_t offset) const // ƫ��ת��Ϊ��ַ��ע�⣺���ִ��Add֮�󣬵�ַ����ʧЧ
        {
            return static_cast<_TString::LPCSTR>(static_cast<u8*>(m_buffer.GetPtr()) + offset);
        }
    };
}

#endif // _CLSTD_STRING_POOL_H_