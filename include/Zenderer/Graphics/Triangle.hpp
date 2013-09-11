/**
 * @file
 *  Zenderer/Graphics/Triangle.hpp - A drawable primitive with 3 vertices.
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
 * @addtogroup Graphics
 * @{
 **/

#ifndef ZENDERER__GRAPHICS__TRIANGLE_HPP
#define ZENDERER__GRAPHICS__TRIANGLE_HPP

#include <array>

#include "Zenderer/Math/Shapes.hpp"
#include "Zenderer/CoreGraphics/Drawable.hpp"

namespace zen
{
namespace gfx
{
    /// A basic three-vertex primitive.
    class ZEN_API zTriangleQuad : public gfxcore::zDrawable
    {
    public:
        zTriangle(asset::CAssetManager& Mgr,
                  const math::vector_t& A,
                  const math::vector_t& B,
                  const math::vector_t& C) :
            zDrawable(Mgr) /*, m_Verts({ A, B, C })*/
        {
            m_Verts[0] = A;
            m_Verts[1] = B;
            m_Verts[2] = C;

            this->LoadSize();
        }

        zTriangle(const zTriangle& Copy)
        {
            this->operator=(Copy);
        }
        
        zTriangle& operator=(const zTriangle& Copy)
        {
            std::copy(Copy.m_Verts.begin(), Copy.m_Verts.end(), &m_Verts[0]);
            this->LoadSize();
            return *this;
        }
        
        ~zTriangle() {}

        virtual zDrawable& Create()
        {
            if(m_DrawData.Vertices == nullptr)
            {
                m_DrawData.Vertices = new gfxcore::vertex_t[3];
                m_DrawData.vcount = 3;
            }

            this->LoadVertexData();

            if(m_DrawData.Indices == nullptr)
            {
                m_DrawData.Indices = new gfxcore::index_t[3];
                m_DrawData.icount = 3;
            }

            m_DrawData.Indices[0] = 0;
            m_DrawData.Indices[1] = 1;
            m_DrawData.Indices[2] = 2;

            return (*this);
        }

        inline uint16_t GetW() const { return m_Size.x; }
        inline uint16_t GetH() const { return m_Size.y; }

    private:
        void LoadVertexData()
        {
            std::copy(&m_Verts[0], &m_Verts[2], m_DrawData.Vertices);
        }

        void LoadSize()
        {
            // Find left-most vertex.
            real_t l = math::min(math::min(m_Verts[0].x, m_Verts[1].x),
                                 m_Verts[2].x);

            // Find right-most vertex.
            real_t r = math::max(math::max(m_Verts[0].x, m_Verts[1].x),
                                 m_Verts[2].x);

            // Find bottom-most vertex.
            real_t b = math::min(math::min(m_Verts[0].y, m_Verts[1].y),
                                 m_Verts[2].y);

            // Find top-most vertex.
            real_t t = math::max(math::max(m_Verts[0].y, m_Verts[1].y),
                                 m_Verts[2].y);

            m_Size.x = r - l;
            m_Size.y = b - t;
        }

        math::tri_t         m_Verts;
        math::vectoru16_t   m_Size;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__TRIANGLE_HPP

/**
 * @class zen::gfx::zTriangle
 * @details
 *  Triangles are a low-level drawing primitive provided for convenience
 *  to users of @a Zenderer. They are not actually used in any part of the
 *  engine other than the triangulation module and collision detection.
 *
 * @see gfx::triangulate()
 **/

/** @} **/
