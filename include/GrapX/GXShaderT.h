#ifndef _GRAPX_SHADER_T_H_
#define _GRAPX_SHADER_T_H_

namespace UVShader
{
  class CodeParser;
  enum UniformModifier;
  enum VariableStorageClass;
}

namespace GrapX
{
  namespace ShaderT
  {
    class CodeInstance
    {
      clstd::MemBuffer m_buffer;

    protected:
      void Dump(const UVShader::CodeParser& parser);
      void DumpVariableModifier(UVShader::UniformModifier modifier);
      void DumpVariableStorageClass(UVShader::VariableStorageClass storage_class);
      void WriteFormat(GXLPCSTR szFormat, ...);
      
    public:
      CodeInstance();
      ~CodeInstance();

      GXBOOL Init(const UVShader::CodeParser& parser);
      GXBOOL SaveToFile(GXLPCWSTR szFilename);
    };

    class ShaderObject // ∂‘œÛ
    {
    private:
      clStringW                   m_strFilename;
      const clstd::StockA&        m_stock;
      const clstd::TextLines<ch>& m_lines;

    private:
      GXBOOL InitPass(const clstd::StockA::Section& sect_pass);

    public:
      ShaderObject(const clStringW& strFilename, const clstd::StockA& stock, const clstd::TextLines<ch>& lines);
      virtual ~ShaderObject();

      GXBOOL InitObject(const clstd::StockA::Section& sect);
    };

    class GXDLL Script
    {
      typedef cllist<ShaderObject*> shader_list_t;
    private:
      clStringW            m_strFilename;
      clstd::StockA        m_stock;
      clstd::TextLines<ch> m_lines;
      shader_list_t        m_ShaderList;

    public:
      Script();
      virtual ~Script();

      GXBOOL Load(GXLPCWSTR szFilename);
      GXBOOL Load(GXLPCSTR szFilename);
      GXBOOL Load(const clstd::BufferBase& buf, GXLPCWSTR szFilename);
    };
  } // namespace ShaderT
} // namespace GrapX

#endif // _GRAPX_SHADER_T_H_