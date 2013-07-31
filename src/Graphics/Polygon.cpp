#include "Zenderer/Graphics/Polygon.hpp"

using namespace zen;
using gfx::CPolygon;

CPolygon::CPolygon(asset::CAssetManager& Assets) :
    CDrawable(Assets)
{
    m_Verts.clear();
}

CPolygon::~CPolygon()
{
    for(auto& i : m_Verts) delete i;
    m_Verts.clear();
}

void CPolygon::AddVertex(const math::vector_t& Position)
{
    gfxcore::vertex_t* V = new gfxcore::vertex_t;
    V->position = Position;
    m_Verts.push_back(V);
}

gfxcore::CDrawable& CPolygon::Create()
{
    if(m_Verts.size() <= 2) return (*this);

    uint16_t tris = (m_Verts.size() - 2) * 3;
    gfxcore::index_t* indices = new gfxcore::index_t[tris];

    for(uint16_t i = 0; i < tris; i += 3)
    {
        uint16_t x = i / 3;
        indices[i] = 0;
        indices[i+1] = x + 1;
        indices[i+2] = x + 2;
    }

    m_DrawData.Vertices = new gfxcore::vertex_t[m_Verts.size()];
    m_DrawData.vcount   = m_Verts.size();
    m_DrawData.Indices  = indices;
    m_DrawData.icount   = tris;

    math::vector_t First(m_Verts[0]->position);
    for(size_t i = 0; i < m_Verts.size(); ++i)
    {
        m_DrawData.Vertices[i].position = m_Verts[i]->position;
        m_DrawData.Vertices[i].color    = m_Verts[i]->color;
        m_DrawData.Vertices[i].tc       = m_Verts[i]->tc;
    }

    for(auto& i : m_Verts) delete i;
    m_Verts.clear();

    return (*this);
}

void CPolygon::SetColor(const color4f_t& Color)
{
    for(auto& i : m_Verts) i->color = Color;
}
