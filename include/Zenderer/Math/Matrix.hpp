/**
 * @file
 *  Zenderer/Math/Matrix.hpp - A 4x4 matrix class wrapper for handling
 *  various graphical math operations.
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

#ifndef ZENDERER__MATH__MATRIX_HPP
#define ZENDERER__MATH__MATRIX_HPP

#include "Vector.hpp"

namespace zen
{
namespace math
{
    /// 4x4 matrix representation.
    class ZEN_API matrix4x4_t
    {
    public:
        matrix4x4_t();
        matrix4x4_t(const real_t values[4][4]);
        matrix4x4_t(const real_t** ppvalues);
        matrix4x4_t(const matrix4x4_t& Copy);

        matrix4x4_t& operator=(const matrix4x4_t& Copy);

        /**
         * Provides the matrix's elements at that row.
         *  See the following examples:
         *
         *    matrix[0][3]  // 3rd element of row 0 (r0, c3)
         *    matrix[3][1]  // 1st element of row 3 (r3, c1)
         *    matrix[2][1]  // 1st element of row 2 (r2, c1)
         *
         *  The index value is automatically clamped at [0, 3].
         *
         * @param   index   Row index to access
         * @return  The `real_t` array at that index.
         **/
        real_t* operator[](uint8_t index);

        /// Multiple two matrices together.
        matrix4x4_t operator*(matrix4x4_t& Other) const;

        /// Translates the matrix by a vector.
        inline void Translate(const math::vector_t& V)
        {
            m_values[0][3] = V.x;
            m_values[1][3] = V.y;
            m_values[2][3] = V.z;
        }

        /// Adjusts the translation matrix by a vector.
        inline void TranslateAdj(const math::vector_t& DV)
        {
            m_values[0][3] += DV.x;
            m_values[1][3] += DV.y;
            m_values[2][3] += DV.z;
        }

        /**
         * Stretches the matrix by degree-angles in different directions.
         *  For future reference, x-direction shearing angles are as
         *  follows (in degrees):
         *
         *          45.0 or 225.0    0       -45.0 or 135.0
         *           /               |              \
         *          /                |               \
         *
         * @param   Angles  Vector containing x, y, and z shear angles (in deg)
         *
         * @note    Only 2D shears are supported (X and Y directions).
         * @todo    Add 3D shearing.
         **/
        inline void Shear(const math::vector_t& Angles)
        {
            if(!compf(Angles.x, 0.0f))
                m_values[0][1] = -std::tan(rad(Angles.x));

            if(!compf(Angles.y, 0.0f))
                m_values[1][0] = -std::tan(rad(Angles.y));
        }
        
        /**
         * Scales the matrix by some scalar values.
         *  If a matrix with a scaling factor of (1, 2, 0) is applied
         *  to a (2, 3, 5) vector, for example, the resulting 
         *  vector would be (2, 6, 0).
         *  Scaling is a 1 to 1 ratio by default.
         *
         * @param   Factors     Scaling values in x, y, z direction.
         **/
        inline void Scale(const math::vector_t& Factors)
        {
            m_values[0][0] *= Factors.x;
            m_values[1][1] *= Factors.y;
            m_values[2][2] *= Factors.z;
        }
        
        /**
         * Performs a 2D rotation about a set of angles.
         *  There is no support for rotation in all directions in 3D
         *  space, but since @a Zenderer is (so far) exclusively a 2D
         *  engine, this isn't necessary. Rotation in 2D can be considered
         *  as rotation about the z-axis in 3D space, since the z-axis pops
         *  out toward/away from the screen, taking this axis and turning it
         *  would simply be turning the screen.
         *
         * @note    Only rotations on the 2D xy-plane are supported;
         *          this is rotation about the Z axis.
         *
         * @see     zen::math::Vector::Rotate
         * @see     http://people.cs.clemson.edu/~dhouse/courses/401/notes/affines-matrices.pdf
         **/
        inline void Rotation(const real_t degrees)
        {
            real_t r = rad(degrees);
            real_t c = std::cos(r);
            real_t s = std::sin(r);
            
            m_values[0][0] =  c;
            m_values[0][1] = -s;
            m_values[1][0] =  s;
            m_values[1][1] =  c;
        }

        inline const real_t* GetPointer() const
        { return reinterpret_cast<const real_t*>(m_values); }

        /**
         * Creates a 2D projection matrix.
         *
         * @param   width   Projection width
         * @param   height  Projection height
         * @param   max_z   Maximum projection depth (positive integer)
         * @param   min_z   Minimum projection depth (positive integer)
         *
         * @return  The requested projection matrix.
         **/
        static inline matrix4x4_t Projection2D(
            const uint16_t width,   const uint16_t height,
            const uint16_t max_z,   const uint16_t min_z)
        {
            matrix4x4_t result;

            result[0][0] =  2.0f / width;
            result[0][3] = -1.0f;
            result[1][1] = -2.0f / height;
            result[1][3] =  1.0f;
            result[2][2] = -2.0f / (min_z - max_z);
            result[3][3] =  1.0f;

            return result;
        }

        static matrix4x4_t CreateIdentityMatrix();
        static const matrix4x4_t& GetIdentityMatrix();

    private:
        real_t m_values[4][4];
    };
}   // namespace math
}   // namespace zen

#endif // ZENDERER__MATH__MATRIX_HPP

/**
 * @class zen::math::matrix4x4_t
 * @details
 *  This is in place to support various linear transformation
 *  operations required for moving vertices around in OpenGL
 *  terms.
 **/

/** @} **/
