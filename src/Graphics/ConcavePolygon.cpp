#include "Zenderer/Graphics/Polygon.hpp"

using namespace zen;
using gfx::zConcavePolygon;

template<typename InputIt, typename T> inline
size_t find_index_of(InputIt begin, InputIt end, const T& elem)
{
    size_t index = 0;
    for(InputIt i = begin, j = end; i != j; ++i, ++index)
    {
        if(*i == elem) return index;
    }
    
    return static_cast<size_t>(-1);
}

zConcavePolygon::zConcavePolygon(asset::zAssetManager& Assets,
                                 const size_t preload) :
    zPolygon(Assets, preload) {}

zConcavePolygon::~zConcavePolygon() {}

zPolygon& zConcavePolygon::Create()
{
    if(m_Verts.size() <= 2) return (*this);
    
    std::vector<math::vector_t> tris = math::triangulate(m_Verts);
    std::vector<math::vector_t> verts;

    // We minimize the vertices we offload by only storing those
    // that are unique. This process is O(n^2) but who cares?
    uint16_t index = 0;
    uint16_t count = (m_Verts.size() - 2) * 3;
    
    if(m_DrawData.Indices != nullptr || m_DrawData.icount > 0) 
        delete[] m_DrawData.Indices;

    gfxcore::index_t* indices = new gfxcore::index_t[count];

    for(auto& i : tris)
    {
        int idx = find_index_of(verts.begin(), verts.end(), i);
        if(idx > 0) indices[index++] = idx;
        else
        {
            verts.push_back(i);
            indices[index] = index++;
        }
    }
    
    m_DrawData.Indices  = indices;
    m_DrawData.icount   = tris;

    m_DrawData.Vertices = new gfxcore::vertex_t[m_Verts.size()];
    m_DrawData.vcount   = verts.size();

    for(size_t i = 0; i < m_Verts.size(); ++i)
    {
        m_DrawData.Vertices[i].position = std::move(verts[i]);
        m_DrawData.Vertices[i].color    = m_Color;
    }

    m_Tris = std::move(tris);
    m_Verts.clear();
    return (*this);
}
