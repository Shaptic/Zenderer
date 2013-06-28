/**
 * @file
 *  Zenderer/Graphics/Quad.hpp - A drawable primitive with 4 vertices.
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

#ifndef ZENDERER__GRAPHICS__QUAD_HPP
#define ZENDERER__GRAPHICS__QUAD_HPP

#include "Zenderer/CoreGraphics/Drawable.hpp"

namespace zen
{
namespace gfx
{
    class ZEN_API CQuad : public CDrawable
    {
    public:
        CQuad(const math::rect_t& Size) : m_Size(Size) {}
        CQuad(const uint16_t w, const uint16_t h) : m_Size(0, 0, w, h) {}
        
        ~CQuad();
        
        void Create()
        {
            // Aliases.
            vertex_t* vertexList = m_DrawData.Vertices;
            index_t* indexList = m_DrawData.Indices;
            
            if(vertexList == nullptr)
            {
                vertexList = new vertex_t[4];
                m_DrawData.vcount = 4;
            }
            
            vertexList[0].position = math::vector_t();
            vertexList[1].position = math::vector_t(m_Size.w, 0.0);
            vertexList[2].position = math::vector_t(m_Size.w, m_Size.h);
            vertexList[3].position = math::vector_t(0.0, m_Size.h);
            
            if(indexList == nullptr)
            {
                indexList = new index_t[6];
                m_DrawData.icount = 6;
            }
            
            indexList[0] = 0;
            indexList[1] = 1;
            indexList[2] = 3;
            indexList[3] = 3;
            indexList[4] = 1;
            indexList[5] = 2;
        }
        
        void AttachMaterial(const gfx::material_t* pMaterial)
        {
            vertex_t* vertexList = m_DrawData.Vertices;
            if(vertexList == nullptr) return;
            
            vertexList[0].tc = math::vector_t(0.0, 0.0);
            vertexList[1].tc = math::vector_t(1.0, 0.0);
            vertexList[2].tc = math::vector_t(1.0, 1.0);
            vertexList[3].tc = math::vector_t(0.0, 1.0);
        }
 
    private: 
        math::rect_t m_Size;
    };

}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__QUAD_HPP

/** @} **/
