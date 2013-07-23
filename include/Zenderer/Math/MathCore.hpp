/**
 * @file
 *  Zenderer/Math/MathCore.hpp - Defines various functions that are used
 *  throughout the rest of the math component.
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
 *  This group contains essentials for manipulation of objects throughout
 *  the rendering engine. Actions such as moving an entity are performed
 *  with a combination of user-level contribution of `math::vector2_t`, and
 *  a low-level manipulation of `math::matrix4x4_t`. Collision detection
 *  and physical reactions require the `math::rect_t` object, which itself
 *  might depend on `math::vector2_t`, as well. Do you want to locate the angle
 *  for firing a shot? Well then you likely need `math::rad` and
 *  `math::deg` to get sensible values for trigonometric functions.\n
 *  As you can see, the `Math` group is at the very core of the engine.
 *  It's critical enough to be a part of `Core`, but can do enough on its
 *  own that it deserves its own group.
 *
 * @{
 **/

#ifndef ZENDERER__MATH__MATH_CORE_HPP
#define ZENDERER__MATH__MATH_CORE_HPP

#include <cmath>

#include "Zenderer/Core/Types.hpp"

// Undefine these function macros so we can make our own and
// not have them conflict.
#ifdef max
  #undef max
#endif // max

#ifdef min
  #undef min
#endif // min

namespace zen
{
    // Forward declaration (defined in Core/Types.hpp)
    template<typename T>
    ZEN_API void clamp(T&, const T&, const T&);

/// Defines math-related functions and objects used throughout @a Zenderer.
namespace math
{
    /// Famous mathematical constant.
    static const real_t PI = 3.141592653;

    /**
     * Returns a value that is a portion of the way between Start and End
     *  The weight is clamped to the range [0, 1].
     *
     * @param   Start   Starting value
     * @param   End     Ending value
     * @param   weight  Percentage in between values
     *
     * @return  Value between start and ending values.
     **/
    template<typename T>
    ZEN_API T lerp(const T& Start, const T& End, real_t weight);

    /**
     * Tests to see if a value is in a given range (inclusive).
     *  The type can be any type supporting the <= and >= operators.
     *
     * @param   n       Value to test
     * @param   minimum Minimum range (inclusive)
     * @param   maximum Maximum range (inclusive)
     *
     * @return  `true`  if T in [minimum, maximum], and
     *          `false` if not.
     **/
    template<typename T>
    ZEN_API bool in_range(const T& n, const T& minimum, const T& maximum);

    /// Returns the maximum of two values (first value if same).
    template<typename T>
    ZEN_API const T& max(const T& a, const T& b);

    /// Returns the minimum of two values (first value if same).
    template<typename T>
    ZEN_API const T& min(const T& a, const T& b);

    /// Converts degrees to radians.
    ZEN_API real_t rad(const real_t degrees);

    /// Converts radians to degrees.
    ZEN_API real_t deg(const real_t radians);

    /// Compares two floating point values within a certain range.
    ZEN_API bool compf(const real_t, const real_t,
                       const real_t threshold = 0.0001);

    // Definitions for the functions defined above.
    #include "MathCore.inl"
}   // namespace math
}   // namespace zen

#endif // ZENDERER__MATH__MATH_CORE_HPP

/** @} **/
