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

        virtual CDrawable& Create();
        virtual void AttachMaterial(gfx::CMaterial* pMaterial);
        
        /** 
         * Resizes the quad to a new dimension.
         *
         * @param   w   New width
         * @param   h   New height
         *
         * @pre     Draw() has not been called yet.
         **/
        void Resize(const math::vectoru16_t& Size);
        
        /// @overload
        void Resize(const uint16_t w, const uint16_t h);
        
        /**
         * Makes the vertices have non-zero y values.
         *  This is used (by me) to support skewing the upper vertices
         *  of a quad to simulate waving effects and such. By default,
         *  the quad's vertices are laid out like this:
         *
         *       (0,0) _________________ (w,0)
         *            |                 |
         *            |                 |
         *            |                 |
         *            |_________________|
         *       (0,h)                   (w,h)
         *
         *  As such, when skew operations are performed in the x-direction,
         *  they only modify the bottom vertices since they have non-zero 
         *  y-values. Calling this method will arrange the vertices like so:
         *
         *       (0,-h) _________________ (w,-h)
         *             |                 |
         *             |                 |
         *             |                 |
         *             |_________________|
         *       (0,0)                    (w,0)
         *
         *  Thus allowing for the upper vertices to be skewed properly,
         *  but the bottom ones will not, naturally.
         *
         * @param   flag    `true`  will make the upper vertices skewable,
         *                  `false` will create the quad as normal
         *
         * @pre     Neither Draw() nor Create() have been called yet.
         **/
        void SetInverted(const bool flag);
        
        /**
         * Repeat texture or stretch to fit.
         *  When a material is attached to the drawable instance, 
         *  and the size of the drawable is larger than the texture size,
         *  it will be stretched by default. You can set it to repeat the 
         *  texture pattern across the drawable with this method. 
         *
         * @bool    Flag    `true` makes it repeat, `false` stretches
         *
         * @pre     Neither Draw() nor Create() have been called yet.
         **/
        void SetRepeating(const bool flag);

    private:
        void LoadRegularVertices();     // Standard quad
        void LoadInvertedVertices();    // Inverted quad using -y instead of 0
        void LoadRegularTC();           // Tex-coords to match standard quad
        void LoadInvertedTC();          // Tex-coords to match inverted quad
        
        math::rect_t m_Size;
        bool m_inv, m_rep;
    };

}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__QUAD_HPP

/** @} **/
