#ifndef _GRAPX_SHADER_T_H_
#define _GRAPX_SHADER_T_H_

namespace GrapX
{
  namespace ShaderT
  {
    class ShaderObject // ∂‘œÛ
    {
    public:
      ShaderObject();
      virtual ~ShaderObject();
    };

    class GXDLL Script
    {
      typedef cllist<ShaderObject*> shader_list_t;
    private:
      clstd::StockA        m_stock;
      clstd::TextLines<ch> m_lines;
      shader_list_t        m_ShaderList;

    public:
      Script();
      virtual ~Script();

      GXBOOL Load(GXLPCWSTR szFilename);
      GXBOOL Load(GXLPCSTR szFilename);
      GXBOOL Load(const clstd::BufferBase& buf);
    };
  } // namespace ShaderT
} // namespace GrapX

#endif // _GRAPX_SHADER_T_H_