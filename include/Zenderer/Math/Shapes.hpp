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

#include <vector>
#include <array>

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Utilities/Assert.hpp"
#include "MathCore.hpp"
#include "Vector.hpp"

namespace zen
{
namespace math
{
    /// A triangle representation.
    typedef std::array<vector_t, 3> tri_t;

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
               const zVector<uint32_t>& Size) :
            tl(Pos), br(Pos + Size) {}

        aabb_t(const rect_t& Data) :
            tl(Data.x, Data.y), br(Data.x + Data.w, Data.y + Data.h) {}

        /// Detects collision with another AABB.
        bool collides(const aabb_t& b) const;

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
        bool collides(const tri_t& tri) const;

        vector_t tl;      ///< Top-left point.
        vector_t br;      ///< Bottom-right point.
    };

    static inline
    bool collides(const tri_t& A, const tri_t& b)
    {
        ZEN_ASSERTM(false, "not implemented");
        return false;
    }

    /**
     * Finds orientation of a given set of points, which form a polygon.
     * @param   Polygon     A set of points forming a simple polygon.
     * @return `true` if clockwise, `false` if counter-clockwise.
     * @see    http://debian.fmi.uni-sofia.bg/~sergei/cgsr/docs/clockwise.htm
     **/
    bool orientation(const std::vector<vector_t>& Polygon);
    bool orientation(const tri_t& Tri);     ///< @overload

    /**
     * Tests if a point is contained with a triangle.
     *  This algorithm uses a formula for Barycentric coordinates.
     *
     * @param   V   The point to test to see if it's in `T`
     * @param   T   Test if this triangle contains `V`
     *
     * @return  `true` if `V` is within `T`, `false` otherwise.
     *
     * @see     http://stackoverflow.com/a/13301035
     **/
    bool triangle_test(const vector_t& V, const tri_t& T);

    /**
     * Triangulates a set of vertices into triples, which form triangles.
     *
     * @param   Polygon     A list of vertices forming a simple polygon
     *
     * @pre     `Polygon` must have at least 3 points in it.
     *
     * @return  A list of vertices that, when taken in triplets, form triangles.
     *
     * @see http://abitwise.blogspot.com/2013/09/triangulating-concave-and-convex.html
     * @see http://gist.github.com/Ruskiy69/6526805
     *
     * @note    Algorithmic complexity: O(n^3)
     **/
    std::vector<vector_t> triangulate(std::vector<vector_t> Polygon);
}   // namespace math
}   // namespace zen

#endif // ZENDERER__MATH__SHAPES_HPP

/** @} **/
