/**
 * @file
 *  Zenderer/Math/Shapes.hpp - Defines various shape structures.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup Math
 * @{
 **/

#ifndef ZENDERER__MATH__SHAPES_HPP
#define ZENDERER__MATH__SHAPES_HPP

#include "Zenderer/Core/Types.hpp"
#include "MathCore.hpp"
#include "Vector.hpp"

namespace zen
{
namespace math
{
    /// Represents a rectangle with position and dimensions.
    struct ZEN_API rect_t
    {
        rect_t() : x(0.0), y(0.0), w(0), h(0) {}
        rect_t(real_t x, real_t y, uint32_t w, uint32_t h) :
            x(x), y(y), w(w), h(h) {}

        real_t x, y;
        uint32_t w, h;
    };

    /// Represents a circle with position and radius.
    struct ZEN_API circle_t
    {
        circle_t() : x(0.0), y(0.0), r(0.0) {}
        circle_t(real_t x, real_t y, real_t r) : x(x), y(y), r(r) {}

        inline bool collides(const circle_t& Other) const
        {
            real_t d = distance(Other.x, Other.y, x, y, false);
            return d <= (r + Other.r) * (r + Other.r);
        }

        real_t x, y;
        real_t r;
    };

    /// Represents an axis-aligned bounding box.
    struct ZEN_API aabb_t
    {
        aabb_t() {}

        aabb_t(const vector_t& Pos,
               const Vector<uint32_t>& Size) :
            tl(Pos), br(Pos + Size) {}

        aabb_t(const rect_t& Data) :
            tl(Data.x, Data.y), br(Data.x + Data.w, Data.y + Data.h) {}

        /// Detects collision with another AABB.
        inline bool collides(const aabb_t& b) const
        {
            return !(tl.x > b.br.x || br.x < b.tl.x ||
                     tl.y > b.br.y || br.y < b.tl.y);
        }

        /**
         * Detects collision with a triangle.
         *  This uses the separation of axis theorem (SAT) to perform
         *  collision detection. It tests the 3 axes of the triangle
         *  in turn, exiting the function as soon as a collision is
         *  impossible.
         *
         *  Worst case, the algorithm will perform:
         *      19 multiplications
         *      33 addition / subtractions
         *      25 comparisons
         *
         *  Best case, the algorithm will perform:
         *      7 multiplications
         *      27 addition / subtractions
         *      4 comparisons
         *
         * @see http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/
         **/
        inline bool collides(const vector_t tri[3])
        {
            math::vector_t halfsize((br - tl) * 0.5);
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

        math::vector_t tl;      ///< Top-left point.
        math::vector_t br;      ///< Bottom-right point.
    };
}   // namespace math
}   // namespace zen

#endif // ZENDERER__MATH__SHAPES_HPP

/** @} **/
