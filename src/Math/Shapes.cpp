#include <algorithm>
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
    math::vector_t halfsize((br - tl) * static_cast<real_t>(0.5f));
    math::vector_t center(tl + halfsize);

    math::vector_t verts[3] = {
        tri[0] - center,
        tri[1] - center,
        tri[2] - center
    };

    math::vector_t edges[3] = {
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

bool math::collides(const tri_t& A, const tri_t& B, cquery_t* q)
{
    line_t aseg[3] = {
        { A[0], A[1] },
        { A[1], A[2] },
        { A[0], A[2] }
    };

    line_t bseg[3] = {
        { B[0], B[1] },
        { B[1], B[2] },
        { B[0], B[2] }
    };

    for(uint8_t i = 0; i < 3; ++i)
    {
        for(uint8_t j = 0; j < 3; ++j)
        {
            if(math::collides(aseg[i], bseg[j], q))
            {
                if(q != nullptr)
                {
                    q->line1 = std::move(aseg[i]);
                    q->line2 = std::move(bseg[j]);
                }

                return true;
            }
        }
    }

    return false;
}

bool math::collides(const line_t& A, const tri_t& B, cquery_t* q)
{
    line_t bseg[3] = {
        { B[0], B[1] },
        { B[1], B[2] },
        { B[0], B[2] }
    };

    for(uint8_t i = 0; i < 3; ++i)
    {
        if(math::collides(A, bseg[i], q))
        {
            if(q != nullptr)
            {
                q->line1 = std::move(A);
                q->line2 = std::move(bseg[i]);
            }

            return true;
        }
    }

    return false;
}

bool math::collides(const line_t& a, const line_t& b, cquery_t* q)
{
    // A = Q -> Q + S
    // B = P -> P + R
    math::vector_t r = b[1] - b[0];
    math::vector_t s = a[1] - a[0];
    math::vector_t z = a[0] - b[0];

    real_t zxr = glm::cross(z, r).length();

    // Co-linear. Check for overlap.
    if(compf(zxr, 0.0))
    {
        bool ret = ((b[0].x - a[0].x < 0 != b[0].x - a[1].x < 0) ||
                    (b[0].y - a[0].y < 0 != b[0].y - a[1].y < 0));

        if(ret && q != nullptr)
        {
            q->collision = q->colinear = true;
            q->point = a[0];
        }

        return ret;
    }

    real_t d = glm::cross(r, s).length();
    if(compf(d, 0.0))   // Parallel lines.
    {
        if(q != nullptr) q->collision = false;
        return false;
    }

    real_t t = glm::cross(z, s).length() / d;
    real_t u = zxr / d;

    bool collision = in_range<real_t>(t, 0, 1) && in_range<real_t>(u, 0, 1);
    if(q != nullptr)
    {
        q->collision = collision;
        q->point = b[0] + (r * t);
        q->edge_case = (compf(t, 0) || compf(t, 1) || compf(u, 0) || compf(u, 1));
    }

    return collision;
}

// Helper functions for triangulation algorithm.

real_t area(const std::vector<math::vector_t>& contour)
{
    size_t n = contour.size();

    real_t A = 0.0f;
    for(int p = n - 1, q = 0; q < n; p = q++)
    {
        A += contour[p].x * contour[q].y - contour[q].x * contour[p].y;
    }

    return A * 0.5f;
}

bool in_triangle(float Ax, float Ay, float Bx, float By,
                 float Cx, float Cy, float Px, float Py)

{
    float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    float cCROSSap, bCROSScp, aCROSSbp;

    ax = Cx - Bx;  ay = Cy - By;
    bx = Ax - Cx;  by = Ay - Cy;
    cx = Bx - Ax;  cy = By - Ay;
    apx = Px - Ax;  apy = Py - Ay;
    bpx = Px - Bx;  bpy = Py - By;
    cpx = Px - Cx;  cpy = Py - Cy;

    aCROSSbp = ax*bpy - ay*bpx;
    cCROSSap = cx*apy - cy*apx;
    bCROSScp = bx*cpy - by*cpx;

    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

bool clip(const std::vector<math::vector_t>& contour,
          int u, int v, int w, int n, int* V)
{
    int p;
    float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

    Ax = contour[V[u]].x;
    Ay = contour[V[u]].y;

    Bx = contour[V[v]].x;
    By = contour[V[v]].y;

    Cx = contour[V[w]].x;
    Cy = contour[V[w]].y;

    if(0.000001 > (((Bx - Ax) * (Cy - Ay)) - ((By - Ay) * (Cx - Ax))))
        return false;

    for(p = 0; p<n; p++)
    {
        if((p == u) || (p == v) || (p == w)) continue;
        Px = contour[V[p]].x;
        Py = contour[V[p]].y;

        if(in_triangle(Ax, Ay, Bx, By, Cx, Cy, Px, Py))
            return false;
    }

    return true;
}

std::vector<math::vector_t> math::triangulate(
    const std::vector<math::vector_t>& contour)
{
    std::vector<math::vector_t> result;

    /* allocate and initialize list of Vertices in polygon */

    int n = contour.size();
    if(n < 3) return contour;

    int *V = new int[n];

    /* we want a counter-clockwise polygon in V */

    if(0.0f < area(contour))
        for(int v = 0; v < n; v++) V[v] = v;
    else
        for(int v = 0; v < n; v++) V[v] = (n - 1) - v;

    int nv = n;

    /*  remove nv-2 Vertices, creating 1 triangle every time */
    int count = 2 * nv;

    for(int m = 0, v = nv - 1; nv > 2;)
    {
        /* if we loop, it is probably a non-simple polygon */
        ZEN_ASSERTM(!(0 >= (count--)), "non-simple polygons unsupported");

        /* three consecutive vertices in current polygon, <u,v,w> */
        int u = v; if(nv <= u) u = 0;     /* previous */
        v = u + 1; if(nv <= v) v = 0;     /* new v    */
        int w = v + 1; if(nv <= w) w = 0;     /* next     */

        if(clip(contour, u, v, w, nv, V))
        {
            int a, b, c, s, t;

            /* true names of the vertices */
            a = V[u]; b = V[v]; c = V[w];

            /* output Triangle */
            result.push_back(contour[a]);
            result.push_back(contour[b]);
            result.push_back(contour[c]);

            m++;

            /* remove v from remaining polygon */
            for(s = v, t = v + 1; t<nv; s++, t++) V[s] = V[t]; nv--;

            /* reset error detection counter */
            count = 2 * nv;
        }
    }

    delete V;
    return result;
}

real_t math::slope(const math::line_t& line)
{
    if (compf(line[1].x, line[0].x))
        return std::numeric_limits<float>::quiet_NaN();

    return (line[1].y - line[0].y) / (line[1].x - line[0].x);
}