/**
 * @file
 *  Zenderer/CoreGraphics/Drawable.hpp - A vertex-agnostic primitive.
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

#ifndef ZENDERER__GRAPHICS__POLYGON_HPP
#define ZENDERER__GRAPHICS__POLYGON_HPP

#include "Zenderer/CoreGraphics/Drawable.hpp"

namespace zen
{
namespace gfx
{
    class ZEN_API CPolygon : public gfxcore::CDrawable
    {
    public:
        CPolygon(asset::CAssetManager& Assets) :
            CDrawable(Assets) {}
        ~CPolygon()
        {
            for(auto& i : m_Verts) delete i;
            m_Verts.clear();
        }

        void AddVertex(const math::vector_t& Position)
        {
            gfxcore::vertex_t* V = new gfxcore::vertex_t;
            V->position = Position;

            m_Verts.push_back(V);
        }

        gfxcore::CDrawable& Create()
        {
            uint16_t tris = m_Verts.size() - 2;
            gfxcore::index_t* indices = new gfxcore::index_t[tris * 3];

            for(uint16_t i = 0; i < tris; ++i)
            {
                indices[i] = 0;
                indices[i+1] = i + 1;
                indices[i+2] = i + 2;
            }
            indices[m_Verts.size() - 1] = 0;

            m_DrawData.Vertices = new gfxcore::vertex_t[m_Verts.size()];
            m_DrawData.vcount   = m_Verts.size();
            m_DrawData.Indices  = indices;
            m_DrawData.icount   = tris * 3;

            for(size_t i = 0; i < m_Verts.size(); ++i)
            {
                m_DrawData.Vertices[i].position = m_Verts[i]->position;
                m_DrawData.Vertices[i].color    = m_Verts[i]->color;
                m_DrawData.Vertices[i].tc       = m_Verts[i]->tc;
            }

            for(auto& i : m_Verts) delete i;
            m_Verts.clear();

            return (*this);
        }

        void SetColor(const color4f_t& Color)
        {
            auto i = m_Verts.begin(), j = m_Verts.end();
            for( ; i != j; ++i) (*i)->color = Color;
        }

    private:
        std::vector<gfxcore::vertex_t*> m_Verts;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__POLYGON_HPP

/** @} **/
