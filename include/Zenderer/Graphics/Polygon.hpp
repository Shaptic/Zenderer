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
        CPolygon(asset::CAssetManager& Assets);
        ~CPolygon();

        void AddVertex(const math::vector_t& Position);
        gfxcore::CDrawable& Create();

        void SetColor(const color4f_t& Color);

    private:
        std::vector<gfxcore::vertex_t*> m_Verts;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__POLYGON_HPP

/** @} **/
