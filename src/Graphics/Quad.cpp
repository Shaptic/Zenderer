#include "Zenderer/Graphics/Quad.hpp"

using namespace zen;
using gfx::CQuad;

CQuad::CQuad(const math::rect_t& Size) : m_Size(Size)
{}

CQuad::CQuad(const uint16_t w, const uint16_t h) : m_Size(0, 0, w, h)
{}

CQuad::~CQuad()
{
    if(m_DrawData.Vertices != nullptr) delete m_DrawData.Vertices;
    if(m_DrawData.Indices  != nullptr) delete m_DrawData.Indices;
}

gfxcore::CDrawable& CQuad::Create()
{
    if(m_DrawData.Vertices == nullptr)
    {
        m_DrawData.Vertices = new gfxcore::vertex_t[4];
        m_DrawData.vcount = 4;
    }

    m_DrawData.Vertices[0].position = math::vector_t();
    m_DrawData.Vertices[1].position = math::vector_t(m_Size.w, 0.0);
    m_DrawData.Vertices[2].position = math::vector_t(m_Size.w, m_Size.h);
    m_DrawData.Vertices[3].position = math::vector_t(0.0, m_Size.h);

    if(m_DrawData.Indices == nullptr)
    {
        m_DrawData.Indices = new gfxcore::index_t[6];
        m_DrawData.icount = 6;
    }

    m_DrawData.Indices[0] = 0;
    m_DrawData.Indices[1] = 1;
    m_DrawData.Indices[2] = 3;
    m_DrawData.Indices[3] = 3;
    m_DrawData.Indices[4] = 1;
    m_DrawData.Indices[5] = 2;

    m_DrawData.Vertices[0].tc = math::vector_t(0.0, 0.0);
    m_DrawData.Vertices[1].tc = math::vector_t(1.0, 0.0);
    m_DrawData.Vertices[2].tc = math::vector_t(1.0, 1.0);
    m_DrawData.Vertices[3].tc = math::vector_t(0.0, 1.0);

    return (*this);
}

void CQuad::AttachMaterial(const gfx::material_t* pMaterial)
{
    ZEN_ASSERT(pMaterial);
    if(m_DrawData.Vertices == nullptr) return;
    mp_Material = pMaterial;
}

