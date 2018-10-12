namespace GrapX
{
  class GraphicsBaseImpl : public GXGraphics
  {
  private:
    GAllocator*             m_pRgnAllocator;

  protected:
    GraphicsBaseImpl();
    virtual ~GraphicsBaseImpl();

    GXHRESULT CreateRectRgn(GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom) override;
    GXHRESULT CreateRectRgnIndirect(GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount) override;
    GXHRESULT CreateRoundRectRgn(GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse) override;
    
  };
} // namespace GrapX