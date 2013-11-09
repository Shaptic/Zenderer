/**
 * @file
 *  Zenderer/Math/zVector.hpp - A vector class representing a position
 *  in 3D (or 2D) space, fully supporting the mathematical operations
 *  typical of such a structure.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.5.1
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
#include <ostream>

#include "Zenderer/Core/Types.hpp"
#include "MathCore.hpp"

namespace zen
{
namespace math
{
    /// Forward declaration of a 4x4 matrix for use in 2D vector translation.
    class ZEN_API matrix4x4_t;

    /// Represents a point in 3D rectangular-coordinate space.
    template<typename T>
    struct ZEN_API zVector
    {
        /// The publicly-accessible vector components.
        T x, y, z;

        /// Default constructor that moves the vector to the origin.
        zVector() : x(0), y(0), z(1) {}

        /// Constructor for any coordinate space (defaults to 2D).
        zVector(T x, T y, T z = 1) : x(x), y(y), z(z) {}

        /// Copies vector components from one to another.
        template<typename U>
        zVector(const zVector<U>& C) : x(C.x), y(C.y), z(C.z) {}

        /// Assigns one vector to another.
        template<typename U> inline
        zVector<T>& operator=(const zVector<U>& Copy);

        /// Compares two vectors for equivalence.
        template<typename U> inline
        bool operator==(const zVector<U>& Other) const;

        /// Opposite of zVector::operator==()
        template<typename U> inline
        bool operator!=(const zVector<U>& Other) const;

        // Begin mathematical vector operations.

        /**
         * Calculates the cross product of two vectors.
         *  The cross product of two vectors is a vector that is
         *  perpendicular to both operators. e.g. i = j ^ k.
         *
         * @param   Other   The vector to cross with
         *
         * @return  A 3D vector normal to both vectors.
         **/
        template<typename U> inline
        zVector<T> operator^(const zVector<U>& Other) const;

        /**
         * Calculates the dot product of two vectors.
         *  The dot product is useful in finding the angle between two
         *  vectors. The formula for that is as follows:
         *
         *  cos(&Theta;) = (A &sdot; B) / (||A|| * ||B||)
         *
         *  Where A and B are vectors and ||A|| is the magnitude of A.
         *  The actual operation is defined as such:
         *
         *  A = (x1, y1, z1)
         *  B = (x2, y2, z2)
         *  A &sdot; B = x1 * x2 + y1 * y2 + z1 * z2
         *
         * @param   Other   The vector to dot with.
         *
         * @return  The dot product as a scalar.
         **/
        template<typename U> inline
        real_t operator*(const zVector<U>& Other) const;

        /**
         * Multiplies each component by a scalar factor.
         * @param   scalar  The component scaling factor
         * @return  A 2D resultant vector.
         **/
        inline zVector<T> operator*(const real_t scalar) const;

        /**
         * Divides each component by a scalar factor.
         * @param   scalar  The component scaling factor
         * @return  A 2D resultant vector.
         **/
        inline zVector<T> operator/(const real_t scalar) const;

        /**
         * Adds a given vector to the current vector, returning the result.
         * @param   Other   The vector to add (component-wise)
         * @return  A 2D resultant vector.
         **/
        template<typename U> inline
        zVector<T> operator+(const zVector<U>& Other) const;

        template<typename U> inline
        zVector<T>& operator+=(const zVector<U>& Other);

        /**
         * Adds a value to both components of the current vector.
         * @param   value   The value to add
         * @return  A 2D resultant vector.
         **/
        inline zVector<T> operator+(const real_t value) const;

        /**
         * Subtracts a given vector from the current vector, returning the result.
         * @param   Other   The vector to subtract from the current vector
         * @return  A 2D resultant vector.
         **/
        template<typename U> inline
        zVector<T> operator-(const zVector<U>& Other) const;

        /**
         * Normalizes the current vector.
         *  This *DOES* modify the current vector, and makes it into
         *  a unit vector version of itself.
         **/
        inline void Normalize();

        /// Returns the current vectors magnitude, or 'norm'.
        /// magnitude = &radic;(x<sup>2</sup> + y<sup>2</sup> + z<sup>2</sup>)
        inline real_t Magnitude() const;

        /**
         * Rotates the current vector CCW using the rotation matrix.
         *  The rotation matrix (in right-hand Cartesian plane)
         *  is defined as being
         *  | x | | cos(&Theta;), -sin(&Theta;) |
         *  | y | | sin(&Theta;),  cos(&Theta;) |
         *
         *  But in the OpenGL coordinate system, the origin is in
         *  the top-left, as opposed to bottom-left, as shown above.
         *  So rotations are actually inverted and the matrix is
         *  | x | | cos(&Theta;),  sin(&Theta;) |
         *  | y | | -sin(&Theta;), cos(&Theta;) |
         *
         *  So the final rotation in OpenGL would be:
         *      x =  x * cos(&Theta;) + y * sin(&Theta;)
         *      y = -x * sin(&Theta;) + y * cos(&Theta;)
         *
         * @param   radians     The rotation angle in radians.
         *
         * @note    The coordinate system adjustment was removed.
         **/
        inline void Rotate(const real_t radians);

        /**
         * Translates the current vector by a matrix.
         * @param   TransMat    Translation matrix
         **/
        void Translate(const matrix4x4_t& TransMat);

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
         * @param   Other   zVector to test cross product on
         *
         * @return  2D cross product (z-component of 3D cross).
         *
         * @see     operator*(const zVector&)
         **/
        template<typename U> inline
        real_t Cross2D(const zVector<U>& Other) const;

        /**
         * Returns a normalized version of the current vector.
         * @see     zVector::Normalize()
         **/
        inline zVector<T> GetNormalized() const;

        /// Outputs the vector in the form `<x, y, z>`
        template<typename U> friend
        std::ostream& operator<<(std::ostream& out,
                                 const zVector<U>& Other);

        /**
         * Finds the distance between two points (optional `sqrt`).
         * @todo    Investigate potential problems between a signed
         *          and unsigned `zVector<R>` or `zVector<S>`.
         **/
        template<typename R, typename S> static
        real_t distance(const zVector<R>& A, const zVector<S>& B,
                        const bool do_sqrt = false);
    };

    #include "Vector.inl"

    /// A shortcut for the default vector implementation in the engine.
    using vector_t = zVector<real_t>;

    /// Floating-point vector shortcut.
    using vectorf_t = zVector<real_t>;

    /// 16-bit unsigned integral vector shortcut.
    using vectoru16_t = zVector<uint16_t>;

    /// Standard 32-bit integral vector shortcut.
    using vectori_t = zVector<int32_t>;

}   // namespace math
}   // namespace ic

#endif // IRON_CLAD__MATH__VECTOR_2_HPP

/**
 * @class zen::math::vector_t
 * @details
 *  Supports all vector operations such as cross products,
 *  dot products, movement, scaling, and rotation.
 *
 *  The class is templated to store any sort of POD values that
 *  you'd like, such as `float`, `int`, `uint32_t`, etc.
 *  Beware that any operations between mixed vector types will
 *  return the type of the vector on the left-hand side of the
 *  operation. e.g.:
 *
 *  @code
 *  zVector<int> A(1, 2, 3);
 *  zVector<float> B(1.1, 2.2, 3.3);
 *
 *  zVector<float> C = A + B;    // INVALID!
 *  zVector<int>   C = A + B;    // VALID
 *  @endcode
 *
 *  Thus if you need a higher level of precision, keep that vector
 *  instance on the right-hand side.
 *
 *  There is a built-in shortcut to `vector_t` that is a floating-point
 *  vector representation that is used throughout the engine.
 *
 * @todo    Add support for translation via matrices.
 *
 * @note    There is support for creating translation matrices via
 *          vectors but not vice-versa.
 *
 * @see     @ref USE_DOUBLE_PRECISION
 **/

/** @} **/
