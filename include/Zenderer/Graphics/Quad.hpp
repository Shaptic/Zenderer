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

#include "Zenderer/Math/Shapes.hpp"
#include "Zenderer/CoreGraphics/Drawable.hpp"

namespace zen
{
namespace gfx
{
    /// A four-sided quadrilateral primitive.
    class ZEN_API CQuad : public gfxcore::CDrawable
    {
    public:
        CQuad(const math::rect_t& Size);
        CQuad(const uint16_t w, const uint16_t h);
        ~CQuad();

        CDrawable& Create();
        void AttachMaterial(const gfx::material_t* pMaterial);

    private:
        math::rect_t m_Size;
    };

}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__QUAD_HPP

/** @} **/
