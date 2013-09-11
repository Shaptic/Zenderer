#include "Zenderer/Graphics/Polygon.hpp"

using namespace zen;
using gfx::zPolygon;

zPolygon::zPolygon(asset::zAssetManager& Assets) :
    zDrawable(Assets)
{
    m_Verts.clear();
}

zPolygon::~zPolygon()
{
    m_Verts.clear();
}

zPolygon::zPolygon(const zPolygon& Copy) : zDrawable(Copy)
{
    m_Size = Copy.m_Size;
    m_Verts.clear();
    m_Verts.reserve(Copy.m_Verts.size());
    std::copy(Copy.m_Verts.begin(), Copy.m_Verts.end(), m_Verts.begin());
}

zPolygon::zPolygon(const zPolygon&& Move) : zDrawable(Move)
{
    m_Size = Copy.m_Size;
    m_Verts = std::move(Move.m_Verts);
}

void zPolygon::AddVertex(const math::vector_t& Position)
{
    m_Verts.emplace_back(Position);
}

gfxcore::zDrawable& zPolygon::Create()
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
        m_DrawData.Vertices[i].position = m_Verts[i];
        m_DrawData.Vertices[i].color    = m_Color;
    }

    for(auto& i : m_Verts) delete i;
    m_Verts.clear();

    return (*this);
}

void zPolygon::SetColor(const color4f_t& Color)
{
    m_Color = Color;
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        m_DrawData.Vertices[i].color = Color;
}

void zPolygon::SetIndices(const std::vector<gfxcore::index_t>& Indices)
{
    if(m_DrawData.Indices != nullptr && m_DrawData.icount > 0)
        delete[] m_DrawData.Indices;

    m_DrawData.icount = Indices.size();
    m_DrawData.Indices = new gfxcore::index_t[m_DrawData.icount];
    std::copy(Indices.begin(), Indices.end(), m_DrawData.Indices);
}

uint16_t zPolygon::GetH() const
{
    if(!m_Verts.size() || !m_DrawData.vcount) return 0;
    
    // Calculate lowest and highest y-values.
    real_t low, high;
    if(m_DrawData.vcount > 0)
        low = high = m_DrawData.Vertices[0].position.y;
    else
        low = high = m_Verts[0].y;
    
    for(auto& i : m_Verts)
    {
        low  = math::min<real_t>(low, i.y);
        high = math::max<real_t>(high, i.y);
    }
    
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        low  = math::min<real_t>(low, m_DrawData.Vertices[i].position.y);
        high = math::max<real_t>(high, m_DrawData.Vertices[i].position.y);
    }
    
    return high - low;
}

uint16_t zPolygon::GetW() const
{
    if(!m_Verts.size() || !m_DrawData.vcount) return 0;
    
    // Calculate lowest and highest x-values.
    real_t left, right;
    if(m_DrawData.vcount > 0)
        left = right = m_DrawData.Vertices[0].position.x;
    else
        left = right = m_Verts[0].x;
    
    for(auto& i : m_Verts)
    {
        left  = math::min<real_t>(left,  i.x);
        right = math::max<real_t>(right, i.x);
    }
    
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        left  = math::min<real_t>(left,  m_DrawData.Vertices[i].position.x);
        right = math::max<real_t>(right, m_DrawData.Vertices[i].position.x);
    }
    
    return high - low;
}
