/**
 * @file
 *  Zenderer/Math/Vector.hpp - A vector class representing a position
 *  in 3D (or 2D) space, fully supporting the mathematical operations
 *  typical of such a structure.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.1.3
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

#ifndef IRON_CLAD__MATH__VECTOR_2_HPP
#define IRON_CLAD__MATH__VECTOR_2_HPP

#include <cmath>
#include <iostream>

#include "Zenderer/Core/Types.hpp"
#include "MathCore.hpp"

namespace zen
{
namespace math
{
    /**
     * Forward declaration of a 4x4 matrix for use in
     * 2D vector translation.
     **/
    class ZEN_API matrix4x4_t;

    /**
     * Represents a point in 3D rectangular-coordinate space.
     *  Supports all vector operations such as cross products,
     *  dot products, movement, scaling, and rotation.
     *
     *  By default, components are real number values, but it can also
     *  be instantiated with integers if necessary. See vectori_t for this.
     *
     * @todo Add support for translation via matrices.
     * @todo Write vectori_t;
     *
     * @see USE_DOUBLE_PRECISION
     **/
    struct ZEN_API vectorf_t
    {
        /// The publicly-accessible vector components.
        real_t x, y, z;

        /// Default constructor that moves the vector to the origin.
        vectorf_t() : x(0), y(0), z(0) {}

        /// Constructor for any coordinate space (defaults to 2D).
        vectorf_t(real_t x, real_t y, real_t z = 0) : x(x), y(y), z(z) {}

        /// Copies vector components from one to another.
        vectorf_t(const vectorf_t& C) : x(C.x), y(C.y), z(C.z) {}

        /// Assigns one vector to another.
        inline vectorf_t& operator=(const vectorf_t& Copy);

        /// Compares two vectors for equivalence.
        inline bool operator==(const vectorf_t& Other) const;

        /// Opposite of vectorf_t::operator==()
        inline bool operator!=(const vectorf_t& Other) const;

        // Begin mathematical vector operations.

        /**
         * Calculates the cross product of two vectors.
         *  The cross product of two vectors is a vector that is
         *  perpendicular to both operators. e.g. i = j ^ k.
         *
         * @param   vectorf_t&   The vector to cross with
         *
         * @return  A 3D vector normal to both vectors.
         **/
        inline vectorf_t operator^(const vectorf_t& Other) const;

        /**
         * Calculates the dot product of two vectors.
         *  The dot product is useful in finding the angle between two
         *  vectors. The formula for that is as follows:    \n
         *      cos(T) = (A . B) / (|| A || * || B ||)      \n
         *  The actual operation is defined as such:        \n
         *      A = (x1, y1, z1)
         *      B = (x2, y2, z2)
         *      A . B = x1*x2 + y1*y2 + z1*z2
         *
         * @param   The vector to dot with.
         *
         * @return  The dot product as a scalar.
         **/
        inline real_t operator*(const vectorf_t& Other) const;

        /**
         * Multiplies each component by a scalar factor.
         * @param   real_t  The scalar
         * @return  A 2D resultant vector.
         **/
        inline vectorf_t operator*(const real_t scalar) const;

        /**
         * Divides each component by a scalar factor.
         * @param   real_t  The scalar
         * @return  A 2D resultant vector.
         **/
        inline vectorf_t operator/(const real_t scalar) const;

        /**
         * Adds a given vector to the current vector, returning the result.
         * @param   vectorf_t    The other vector
         * @return  A 2D resultant vector.
         **/
        inline vectorf_t operator+(const vectorf_t& Other) const;

        /**
         * Adds a value to both components of the current vector.
         * @param   real_t  The value to add
         * @return  A 2D resultant vector.
         **/
        inline vectorf_t operator+(const real_t value) const;

        /**
         * Subtracts a given vector from the current vector, returning the result.
         * @param   vectorf_t    The other vector
         * @return  A 2D resultant vector.
         **/
        inline vectorf_t operator-(const vectorf_t& Other) const;

        /**
         * Normalizes the current vector.
         *  This *DOES* modify the current vector, and makes it into
         *  a unit vector version of itself.
         **/
        inline void Normalize();

        /// Returns the current vectors magnitude, or 'norm'.
        inline real_t Magnitude() const;

        /**
         * Rotates the current vector using the rotation matrix.
         *  The rotation matrix (in right-hand Cartesian plane)
         *  is defined as being
         *  | x | | cos(d), -sin(d) |
         *  | y | | sin(d),  cos(d) |
         *
         *  But in the OpenGL coordinate system, the origin is in
         *  the top-left, as opposed to bottom-left, as shown above.
         *  So rotations are actually inverted and the matrix is
         *  | x | | cos(d),  sin(d) |
         *  | y | | -sin(d), cos(d) |
         *
         *  So the final rotation in OpenGL would be:
         *      x =  x * cos(d) + y * sin(d)
         *      y = -x * sin(d) + y * cos(d)
         *
         * @param   real_t  The rotation angle in radians.
         *
         * @info    The coordinate system adjustment was removed.
         **/
        inline void Rotate(const real_t radians);

        /**
         * Translates the current vector by a matrix.
         * @param   matrix4x4_t&    Translation matrix
         **/
        void Translate(const matrix4x4_t& trans_mat);

        /**
         * Returns a scalar cross product value between two 2D vectors.
         *  Given a vector v = <x1, y1> and a vector w = <x2, y2>, their
         *  cross-product is determined as <0, 0, x1*y2 - y1*x2>.
         *  So, this method returns the third component.
         *
         *  This value can be used to determine which side of a vector
         *  another vector is on. If the return value is negative, the
         *  "Other" vector is on the left (going ccw). If positive,
         *  it's on the right (going c/w). This can also be done by
         *  examining the dot product.
         *
         * @param   vectorf_t&   Vector to test cross product on
         *
         * @return  2D cross product (z-component of 3D cross).
         *
         * @see     operator*(const vectorf_t&)
         **/
        inline real_t Cross2D(const vectorf_t& Other) const;

        /**
         * Returns a normalized version of the current vector.
         * @see vectorf_t::Normalize()
         **/
        inline vectorf_t GetNormalized() const;

        /// Outputs the vector in the form `<x, y, z>`
        static friend std::ostream& operator<<(std::ostream& out,
                                               const vectorf_t& Other);
    };

    #include "Vector.inl"

    typedef vectorf_t vector_t;
}   // namespace math
}   // namespace ic

#endif // IRON_CLAD__MATH__VECTOR_2_HPP

/** @} **/
