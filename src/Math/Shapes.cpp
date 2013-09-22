#include "Zenderer/Math/Shapes.hpp"

using namespace zen;
using namespace math;

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

bool math::collides(const tri_t& A, const tri_t& B)
{
    line_t aseg[3] = {
        { A[0], A[1] },
        { A[1], A[2] },
        { A[2], A[0] }
    };

    line_t bseg[3] = {
        { B[0], B[1] },
        { B[1], B[2] },
        { B[2], B[0] }
    };

    for(uint8_t i = 0; i < 3; ++i)
    {
        for(uint8_t j = 0; j < 3; ++j)
        {
            if(math::collides(aseg[i], bseg[i])) return true;
        }
    }

    return false;
}

bool math::collides(const line_t& a, const line_t& b)
{
    // A = Q -> Q + S
    // B = P -> P + R
    vector_t r = b[1] - b[0];
    vector_t s = a[1] - a[0];
    vector_t z = a[0] - b[0];

    real_t zxr = z.Cross2D(r);

    // Co-linear. Check for overlap.
    if(compf(zxr, 0.0))
    {
        return ((b[0].x - a[0].x < 0 != b[0].x - a[1].x < 0) ||
                (b[0].y - a[0].y < 0 != b[0].y - a[1].y < 0));
    }

    real_t d = r.Cross2D(s);
    if(compf(d, 0.0)) return false;     // Parallel lines.

    real_t t = z.Cross2D(s) / d;
    real_t u = zxr / d;

    return in_range<real_t>(t, 0, 1) && in_range<real_t>(u, 0, 1);
}

bool math::orientation(const tri_t& Tri)
{
    return (Tri[1].x - Tri[0].x) * (Tri[2].y - Tri[0].y) -
           (Tri[2].x - Tri[0].x) * (Tri[1].y - Tri[0].y) > 0.0;
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

    if(Polygon.size() <= 3) return Polygon;

    // Determine polygon's orientation via top-left-most vertex.
    vector_t left = Polygon[0];
    size_t index = 0;
    for(size_t i = 0; i < Polygon.size(); ++i)
    {
        if(Polygon[i].x < left.x ||
          (compf(Polygon[i].x, left.x) && Polygon[i].y < left.y))
        {
            index = i;
            left = Polygon[i];
        }
    }

    tri_t tri = {
        Polygon[(index > 0) ? index - 1 : Polygon.size() - 1],
        Polygon[index],
        Polygon[(index + 1 < Polygon.size()) ? index + 1 : 0]
    };

    bool ccw = orientation(tri);

    // We know there will be vertex_count - 2 triangles made.
    triangles.reserve(Polygon.size() - 2);

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
            uint16_t n = (index + 1 < Polygon.size()) ? index + 1 : 0;

            tri[0] = Polygon[p]; tri[1] = i; tri[2] = Polygon[n];

            if(orientation(tri) != ccw)
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
                    if(&v == &Polygon[p] ||
                       &v == &Polygon[n] ||
                       &v == &Polygon[index]) continue;

                    if(triangle_test(v, tri))
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
