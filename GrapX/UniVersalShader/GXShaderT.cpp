// 全局头文件
#include <GrapX.h>
#include <clTokens.h>
#include <clStock.h>
#include <clTextLines.h>
#include <GrapX/GXShaderT.h>

namespace GrapX
{
  namespace ShaderT
  {
    Script::Script()
      : m_stock(clstd::StockA::StockFlag_Version2)
    {
    }

    Script::~Script()
    {
    }

    GXBOOL Script::Load(GXLPCWSTR szFilename)
    {
      clstd::File file;
      clstd::MemBuffer buf;
      if(_CL_NOT_(file.OpenExisting(szFilename))) {
        return FALSE;
      }

      if(_CL_NOT_(file.ReadToBuffer(&buf))) {
        return FALSE;
      }

      return Load(buf);
    }

    GXBOOL Script::Load(GXLPCSTR szFilename)
    {
      clStringW strFilename = szFilename;
      return Load(strFilename);
    }

    GXBOOL Script::Load(const clstd::BufferBase& buf)
    {
      if(_CL_NOT_(m_stock.Set(&buf))) {
        return FALSE;
      }

      m_lines.Generate(reinterpret_cast<ch*>(buf.GetPtr()), buf.GetSize());
      clstd::StockA::Section sect = m_stock.OpenSection("shader");
      if(sect)
      {
        do {
          TRACE("shader:%s\n", sect.name.ToString());
        } while (sect.NextSection());
      }

      return FALSE;
    }
  } // namespace ShaderT
} // namespace GrapX

