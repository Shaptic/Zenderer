/**
 * @file
 *  Zenderer/Graphics/Quad.hpp - A drawable primitive with 4 vertices.
 *
 * @author      George (@_Shaptic)
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
#include "Zenderer/Graphics/Polygon.hpp"

namespace zen
{
namespace gfx
{
    enum class Axis : uint8_t { NONE, X, Y, Z };
    MAKE_ENUM_BITFLAG(Axis);

    /// A four-sided quadrilateral primitive.
    class ZEN_API zQuad : public gfx::zPolygon
    {
    public:
        zQuad(asset::zAssetManager&, const math::rect_t& Size);
        zQuad(asset::zAssetManager&, const uint16_t w, const uint16_t h);
        zQuad(asset::zAssetManager&, const GLuint handle,
              const EffectType Effect = gfx::EffectType::NO_EFFECT);
        zQuad(asset::zAssetManager&, gfxcore::zTexture& Texture,
              const EffectType Effect = gfx::EffectType::NO_EFFECT);
        zQuad(const zQuad& Copy);

        ~zQuad();

        /// @copydoc    gfx::zPolygon::Create()
        gfx::zPolygon& Create(const bool triangluate = true);

        /// @todo   Make it work properly when `zQuad` is inverted.
        //void Move(const real_t x, const real_t y, const real_t z = 1.0);
        //void Move(const math::vector_t& Position);

        /**
         * Resizes the quad to a new dimension.
         * @param   Size    New quad size (in pixels)
         * @pre     `Draw()` has not been called yet.
         **/
        void Resize(const math::vectoru16_t& Size);
        void Resize(const uint16_t w, const uint16_t h); ///< @overload

        virtual bool Collides(const zPolygon& Other, math::cquery_t* q) const;
        bool Collides(const zQuad& Other, math::cquery_t* q) const;
        bool Collides(const math::aabb_t& other);

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
        virtual void SetInverted(const bool flag);

        /**
         * Repeat texture or stretch to fit.
         *  When a material is attached to the drawable instance,
         *  and the size of the drawable is larger than the texture size,
         *  it will be stretched by default. You can set it to repeat the
         *  texture pattern across the drawable with this method.
         *
         * @param   flag    `true` makes it repeat, `false` stretches
         *
         * @pre     Neither Draw() nor Create() have been called yet.
         **/
        void SetRepeating(const bool flag);

        /**
         * Flips the quadrilateral on a certain axis (or combination of axes).
         *  Passing `gfx::Axis::NONE` will remove all flipping.
         *  Flips do not "stack," meaning call to flip on Y followed by a call
         *  to flip on X will only flip on X. Use bitwise operators to combine
         *  flips, instead.
         *
         * @param   axes    Axis (or axes) to flip on
         **/
        void FlipOn(const Axis& axes);

    private:
        void LoadRegularVertices();     // Standard quad
        void LoadInvertedVertices();    // Inverted quad using -y instead of 0
        void LoadRegularTC();           // Tex-coords to match standard quad
        void LoadInvertedTC();          // Tex-coords to match inverted quad

        Axis m_flips;
        bool m_inv, m_rep;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__QUAD_HPP

/**
 * @class zen::gfx::zQuad
 * @details
 *  Quads are the essential drawing primitive behind @a Zenderer. They are used
 *  internally by zen::obj::zEntity instances, which just attach materials to
 *  one or more of them and draw them on-screen using the standard
 *  implementation.
 **/

/** @} **/
