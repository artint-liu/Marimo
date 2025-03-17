#ifndef _CLSTD_STRING_POOL_H_
#define _CLSTD_STRING_POOL_H_

#ifndef _CL_BUFFER_H_
# error include clBuffer.h first
#endif

// 字符串池
// 用于避免字符串分配内存导致的内存碎片化, 但是需要注意Map仍然可能导致内存碎片化

namespace clstd
{
    template<class _TString, class _TMap = clmap<_TString, size_t> >
    class StringPoolT : _TMap // 不用public继承
    {
        MemBuffer m_buffer;
    public:
        size_t Add(typename _TString::LPCSTR str) // 添加字符串，并返回偏移
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

        typename _TString::LPCSTR Ptr(size_t offset) const // 偏移转换为地址，注意：如果执行Add之后，地址可能失效
        {
            return static_cast<_TString::LPCSTR>(static_cast<u8*>(m_buffer.GetPtr()) + offset);
        }
    };
}

#endif // _CLSTD_STRING_POOL_H_