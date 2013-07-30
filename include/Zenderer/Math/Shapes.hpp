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

        aabb_t(const math::vector_t& Pos,
               const math::Vector<uint32_t>& Size) :
            pos(Pos), xw(Size.x / 2, Pos.y), yw(Pos.x, Size.y / 2) {}

        aabb_t(const math::rect_t& Data) :
            pos(Data.x, Data.y), xw(Data.w / 2, Data.y), yw(Data.x, Data.h / 2) {}

        inline bool collides(const aabb_t& Other) const
        {
            if(std::abs(pos.x - Other.pos.x) > (xw.x + Other.xw.x) ||
               std::abs(pos.x - Other.pos.x) > (xw.x + Other.xw.x))
                return false;

            return true;
        }

        math::vector_t pos;
        math::Vector<uint32_t> xw;
        math::Vector<uint32_t> yw;
    };
}   // namespace math
}   // namespace zen

#endif // ZENDERER__MATH__SHAPES_HPP

/** @} **/
