#include "Zenderer/Math/Shapes.hpp"

using namespace zen::math;

bool aabb_t::collides(const aabb_t& b) const
{
    return !(tl.x > b.br.x || br.x < b.tl.x ||
             tl.y > b.br.y || br.y < b.tl.y);
}

bool aabb_t::collides(const tri_t& tri) const
{
    vector_t halfsize((br - tl) * 0.5);
    vector_t center(tl + halfsize);

    vector_t verts[3] = {
        tri[0] - center,
        tri[1] - center,
        tri[2] - center
    };

    vector_t edges[3] = {
        verts[1] - verts[0],
        verts[2] - verts[1],
        verts[0] - verts[2]
    };

    /*
     * Axis tests entail the following:
     *
     * determinant:
     * |  edge.y  edge.x  |
     * |  vert.y  vert.x  |
     *
     * radius check:
     * <edge.y, edge.x> DOT halfsize
     *
     * determining min/max from determinant
     *
     * if min > radius or max < -radius there is no collision
     */

    // First axis test

    real_t p1 = edges[0].y * verts[1].x - edges[0].x * verts[1].y;
    real_t p2 = edges[0].y * verts[2].x - edges[0].x * verts[2].y;
    real_t r  = std::abs(edges[0].y) * halfsize.x +
                std::abs(edges[0].x) * halfsize.y;

    real_t mn = 0.0, mx = 0.0;
    if(p2 < p1) { mn = p2; mx = p1; }
    else        { mn = p1; mx = p2; }

    if(mn > r || mx < -r) return false;

    // Second axis test

    p1  = edges[1].y * verts[0].x - edges[1].x * verts[0].y;
    p2  = edges[1].y * verts[1].x - edges[1].x * verts[1].y;
    r   = std::abs(edges[1].y) * halfsize.x +
          std::abs(edges[1].x) * halfsize.y;

    if(p1 < p2) { mn = p1; mx = p2; }
    else        { mn = p2; mx = p1; }

    if(mn > r || mx < -r) return false;

    // Third axis test

    p1  = edges[2].y * verts[1].x - edges[2].x * verts[1].y;
    p2  = edges[2].y * verts[2].x - edges[2].x * verts[2].y;
    r   = std::abs(edges[2].y) * halfsize.x +
          std::abs(edges[2].x) * halfsize.y;

    if(p2 < p1) { mn = p2; mx = p1; }
    else        { mn = p1; mx = p2; }

    if(mn > r || mx < -r) return false;

    // min/max check

    mn = mx = verts[0].x;
    mn = min<real_t>(min<real_t>(mn, verts[1].x), verts[2].x);
    mx = max<real_t>(max<real_t>(mx, verts[1].x), verts[2].x);

    if(mn > halfsize.x || mx < -halfsize.x) return false;

    mn = mx = verts[0].y;
    mn = min<real_t>(min<real_t>(mn, verts[1].y), verts[2].y);
    mx = max<real_t>(max<real_t>(mx, verts[1].y), verts[2].y);

    if(mn > halfsize.y || mx < -halfsize.y) return false;

    return true;
}

bool math::orientation(const std::vector<vector_t>& Polygon)
{
    ZEN_ASSERTM(Polygon.size() >= 3, "not a polygon");
    
    uint16_t count = 0;
    for(uint16_t i = 0; i < Polygon.size(); ++i)
    {
        uint16_t j = (i + 1 < Polygon.size()) ? i + 1 : 0;
        uint16_t k = (i + 2 < Polygon.size()) ? i + 2 : 1;
        real_t z = (Polygon[j].x - Polygon[i].x) * (Polygon[k].y - Polygon[j].y)
                 - (Polygon[j].y - Polygon[i].y) * (Polygon[k].x - Polygon[j].x);
        
        count += (z > 0) ? 1 : -1;
    }
    
    ZEN_ASSERTM(count != 0, "not a simple polygon");
    return (count > 0);
}

bool math::orientation(const tri_t& Tri)
{
    return (Tri[1].x - Tri[0].x) * (Tri[2].y - Tri[0].y) -
           (Tri[2].x - Tri[0].x) * (Tri[1].y - Tri[0].y);
}

bool math::triangle_test(const vector_t& V, const tri_t& T)
{
    real_t denom = (T[1].y - T[2].y) * (T[0].x - T[2].x)
                 + (T[2].x - T[1].x) * (T[0].y - T[2].y);

    // Avoid division by zero.
    if(compf(denom, 0.0)) return true;
    denom = 1.0 / denom;
    
    real_t alpha = denom * ((T[1].y - T[2].y) * (V.x - T[2].x) +
                            (T[2].x - T[1].x) * (V.y - T[2].y));
    if(alpha < 0) return false;
 
    real_t beta  = denom * ((T[2].y - T[0].y) * (V.x - T[2].x) +
                            (T[0].x - T[2].x) * (V.y - T[2].y));

    return (beta > 0 || alpha + beta >= 1);
}

std::vector<vector_t> math::triangulate(std::vector<vector_t> Polygon)
{
    std::vector<uint16_t> reflex;
    std::vector<vector_t> triangles;    
    
    // Determine entire polygon orientation
    bool ort = orientation(Polygon);
    
    // We know there will be vertex_count - 2 triangles made.
    triangles.reserve(Polygon.size() - 2);
 
    if(Polygon.size() == 3) return Polygon;
    while(Polygon.size() >= 3)
    {
        reflex.clear();
        int16_t eartip = -1, index = -1;
        tri_t tri;

        for(auto& i : Polygon)
        {
            ++index;
            if(eartip >= 0) break;
            
            uint16_t p = (index > 0) ? index - 1 : Polygon.size() - 1;
            uint16_t n = (index < Polygon.size()) ? index + 1 : 0;
            
            tri[0] = Polygon[p]; tri[1] = i; tri[2] = Polygon[n];
            
            if(orientation(tri) != ort)
            {
                reflex.emplace_back(index);
                continue;
            }
            
            bool ear = true;
            for(auto& j : reflex)
            {
                if(j == p || j == n) continue;
                if(triangle_test(Polygon[j], tri))
                {
                    ear = false;
                    break;
                }
            }
            
            if(ear)
            {
                auto j = Polygon.begin() + index + 1,
                     k = Polygon.end();
                 
                for( ; j != k; ++j)
                {
                    auto& v = *j;
                    if(&v == &Polygon[p] || &v == &Polygon[n]) continue;
                    if(in_triangle(v, tri))
                    {
                        ear = false;
                        break;
                    }
                }
            }
            
            if(ear) eartip = index;
        }
        
        if(eartip < 0) break;
        
        // Create the triangulated piece.
        for(const auto& i : tri) triangles.push_back(i);
        
        // Clip the ear from the polygon.
        Polygon.erase(std::find(Polygon.begin(), Polygon.end(), tri[1]));
    }
    
    return triangles;
}