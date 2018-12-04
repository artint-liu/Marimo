#ifndef _GRAPX_RESOURCE_MANAGER_H_
#define _GRAPX_RESOURCE_MANAGER_H_

namespace GrapX
{
  class GResource;
}
struct GXRASTERIZERDESC;
struct GXBLENDDESC;
struct GXDEPTHSTENCILDESC;
struct GXSAMPLERDESC;
namespace GrapX
{
  namespace Internal
  {
    class GXResourceMgr
    {
    public:
      typedef clmap<clStringW, GResource*>  ResNameDict;
      typedef clmap<GXDWORD, ResNameDict>   CateDict;
      typedef clmap<GResource*, GRESKETCH>  ResDict;

    private:
      CateDict      m_CategoryDict;
      ResDict       m_ResourceDict;

    public:
      GXResourceMgr ();
      ~GXResourceMgr();

      GXLRESULT Initialize();
      GXLRESULT Finalize  ();

      //GXHRESULT       BroadcastDeviceEvent  (DeviceEvent eEvent);
      GXHRESULT       BroadcastScriptCommand    (GRESCRIPTDESC* pDesc);
      GXHRESULT       BroadcastCategoryMessage  (GXDWORD dwCategoryId, GRESCRIPTDESC* pDesc);
      GResource*      Find                      (LPCRESKETCH pDesc) const;
      GXHRESULT       Find                      (GResource** ppResource, LPCRESKETCH pDesc) const;
      LPCRESKETCH     Find                      (GResource* pResource) const;
      GXHRESULT       RegisterUnfeatured        (GResource* pResource);
      GXHRESULT       Register                  (LPCRESKETCH pDesc, GResource* pResource);
      GXHRESULT       Unregister                (GResource* pResource);
    };

    namespace ResourceSketch
    {
      // Texture 2D 和 Texture 3D 都用这个
      GXLRESULT GenerateTexture       (GRESKETCH* pDesc, GXLPCWSTR pSrcFile,
        GXUINT Width, GXUINT Height, GXUINT Depth,
        GXUINT MipLevels, GXFormat Format, GXDWORD Usage,
        GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey);

      GXLRESULT GenerateTextureNameA  (GRESKETCH* pDesc, GXLPCSTR szName);
      GXLRESULT GenerateImage         (GRESKETCH* pDesc, GXLPCWSTR lpwszFilename);
      //GXLRESULT GenerateFontW         (GRESKETCH* pDesc, const GXLPLOGFONTW lpLogFont);
      GXLRESULT GenerateFontA         (GRESKETCH* pDesc, const GXLPLOGFONTA lpLogFont);
      GXLRESULT GenerateShaderElementA(GRESKETCH* pDesc, const MOSHADER_ELEMENT_SOURCE* pSdrElementSrc, const GXDEFINITION* pMacros);
      //GXLRESULT GenerateShaderFileW   (GRESFEATUREDESC* pDesc, GXLPCWSTR szFilename, const GXDEFINITION* pMacros);
      GXLRESULT GenerateMaterialDescW (GRESKETCH* pDesc, GXLPCWSTR szShaderDesc);
      GXLRESULT GenerateVertexDecl    (GRESKETCH* pDesc, LPCGXVERTEXELEMENT lpVertexElement);
      GXLRESULT GenerateSpriteW(GRESKETCH* pDesc, GXLPCWSTR pszSpriteFile);
      //GXLRESULT GenerateSpriteA(GRESKETCH* pDesc, GXLPCSTR pszSpriteFile);

      GXLRESULT GenerateRasterizerState   (GRESKETCH* pDesc, const GXRASTERIZERDESC* pStateDesc);
      GXLRESULT GenerateBlendState        (GRESKETCH* pDesc, const GXBLENDDESC* pStateDesc);
      GXLRESULT GenerateDepthStencilState (GRESKETCH* pDesc, const GXDEPTHSTENCILDESC* pStateDesc);
      GXLRESULT GenerateSamplerState      (GRESKETCH* pDesc, const GXSAMPLERDESC* pStateDesc);
    }

    /*
    class MOResourceExtraStorage
    {
    public:
      typedef clhash_map<GResource*, LPVOID>  ResDict;
    private:
      ResDict m_ResourceToData;
    public:
    };
    //*/
  } // namespace Internal
} // namespace GrapX

#endif // _GRAPX_RESOURCE_MANAGER_H_
