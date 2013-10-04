#include "Zenderer/Graphics/Quad.hpp"

using namespace zen;
using gfx::zQuad;

zQuad::zQuad(asset::zAssetManager& Mgr, const math::rect_t& Size) :
    zPolygon(Mgr), m_inv(false), m_rep(false)
{
    m_BoundingBox = Size;
}

zQuad::zQuad(asset::zAssetManager& Mgr, const uint16_t w, const uint16_t h) :
    zPolygon(Mgr), m_inv(false), m_rep(false)
{
    m_BoundingBox = math::rect_t(0, 0, w, h);
}

zQuad::zQuad(const zQuad& Copy) : zPolygon(Copy)
{
    m_inv = Copy.m_inv;
    m_rep = Copy.m_rep;
}

zQuad::~zQuad()
{
}

gfx::zPolygon& zQuad::Create(const bool triangulate)
{
    if(m_DrawData.Vertices == nullptr)
    {
        m_DrawData.Vertices = new gfxcore::vertex_t[4];
        m_DrawData.vcount = 4;
    }

    if(!m_inv)
    {
        this->LoadRegularVertices();
        this->LoadRegularTC();
    }
    else
    {
        this->LoadInvertedVertices();
        this->LoadInvertedTC();
    }

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        m_DrawData.Vertices[i].color = m_Color;

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

    m_Tris.reserve(m_DrawData.icount);
    for(uint8_t i = 0; i < m_DrawData.icount; ++i)
        m_Tris.push_back(m_DrawData.Vertices[m_DrawData.Indices[i]].position);

    return (*this);
}

void zQuad::Resize(const math::vectoru16_t& Size)
{
    this->Resize(Size.x, Size.y);
}

void zQuad::Resize(const uint16_t w, const uint16_t h)
{
    // Offload()'ed already.
    if(!this->IsModifiable()) return;

    m_BoundingBox.w = w;
    m_BoundingBox.h = h;
}

void zQuad::SetInverted(const bool flag)
{
    m_inv = flag;
}

void zQuad::SetRepeating(const bool flag)
{
    m_rep = flag;
}

void zQuad::LoadRegularVertices()
{
    m_DrawData.Vertices[0].position = math::vector_t(0, 0);
    m_DrawData.Vertices[1].position = math::vector_t(m_BoundingBox.w, 0);
    m_DrawData.Vertices[2].position = math::vector_t(m_BoundingBox.w, m_BoundingBox.h);
    m_DrawData.Vertices[3].position = math::vector_t(0, m_BoundingBox.h);
}

void zQuad::LoadInvertedVertices()
{
    m_DrawData.Vertices[0].position = math::vector_t(0, 0);
    m_DrawData.Vertices[1].position = math::vector_t(0, -m_BoundingBox.h);
    m_DrawData.Vertices[2].position = math::vector_t(m_BoundingBox.w, -m_BoundingBox.h);
    m_DrawData.Vertices[3].position = math::vector_t(m_BoundingBox.w, 0);
}

void zQuad::LoadRegularTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;

    if(m_rep)
    {
        uint16_t w = m_Material.GetTexture().GetWidth();
        uint16_t h = m_Material.GetTexture().GetHeight();

        tc_w = m_BoundingBox.w / real_t(w);
        tc_h = m_BoundingBox.h / real_t(h);
    }

    m_DrawData.Vertices[0].tc = math::vector_t(0.0, tc_h);
    m_DrawData.Vertices[1].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, 0.0);
    m_DrawData.Vertices[3].tc = math::vector_t(0.0, 0.0);
}

void zQuad::LoadInvertedTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;

    if(m_rep)
    {
        uint16_t w = m_Material.GetTexture().GetWidth();
        uint16_t h = m_Material.GetTexture().GetHeight();

        tc_w = m_BoundingBox.w / real_t(w);
        tc_h = m_BoundingBox.h / real_t(h);
    }

    m_DrawData.Vertices[0].tc = math::vector_t(0.0, 0.0);
    m_DrawData.Vertices[1].tc = math::vector_t(0.0, tc_h);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[3].tc = math::vector_t(tc_w, 0.0);
}
