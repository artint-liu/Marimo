if( ! pTexture) {
  return FALSE;
}

GXUINT uBaseIndex = PrepareBatch(CF_Textured, 4, 6, (GXLPARAM)pTexture);
CHECK_LOCK;

for(int i = 0; i < 4; i++)
{
  m_lpLockedVertex[m_uVertCount + i].z = 0;
  m_lpLockedVertex[m_uVertCount + i].w = 1;
  m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)m_dwTexVertColor;
}
//GXUINT nTexWidth;
//GXUINT nTexHeight;
GXSIZE sDimension;
pTexture->GetDimension(&sDimension);
const GXFLOAT fInvTexWidth  = 1.0f / (GXFLOAT)sDimension.cx;
const GXFLOAT fInvTexHeight = 1.0f / (GXFLOAT)sDimension.cy;

#ifdef GLES2_CANVAS_IMPL
const GXFLOAT fLeft   = fInvTexWidth  * rcSrc->left;
const GXFLOAT fTop    = 1 - (fInvTexHeight * rcSrc->top);
const GXFLOAT fRight  = fInvTexWidth  * (rcSrc->left + rcSrc->width);
const GXFLOAT fBottom = 1 - (fInvTexHeight * (rcSrc->top  + rcSrc->height));
#elif defined(D3D9_CANVAS_IMPL)
const GXFLOAT fHalfPixelWidth  = 0.5f * fInvTexWidth;
const GXFLOAT fHalfPixelHeight = 0.5f * fInvTexHeight;

const GXFLOAT fLeft   = fInvTexWidth  * rcSrc->left + fHalfPixelWidth;
const GXFLOAT fTop    = fInvTexHeight * rcSrc->top  + fHalfPixelHeight;
const GXFLOAT fRight  = fInvTexWidth  * (rcSrc->left + rcSrc->width)  + fHalfPixelWidth;
const GXFLOAT fBottom = fInvTexHeight * (rcSrc->top  + rcSrc->height) + fHalfPixelHeight;
#elif defined(D3D11_CANVAS_IMPL)
const GXFLOAT fLeft   = fInvTexWidth  * rcSrc->left;
const GXFLOAT fTop    = fInvTexHeight * rcSrc->top;
const GXFLOAT fRight  = fInvTexWidth  * (rcSrc->left + rcSrc->width);
const GXFLOAT fBottom = fInvTexHeight * (rcSrc->top  + rcSrc->height);
#else
#error 需要定义inl的环境
#endif

m_lpLockedVertex[m_uVertCount    ].SetTexcoord(fLeft, fTop);
m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(fRight, fTop);
m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(fRight, fBottom);
m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(fLeft, fBottom);