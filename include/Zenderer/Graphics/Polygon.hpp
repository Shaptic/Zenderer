/**
 * @file
 *  Zenderer/CoreGraphics/Drawable.hpp - A vertex-agnostic convex primitive.
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

#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/Math/Shapes.hpp"

namespace zen
{
namespace obj { class ZEN_API zEntity; }
namespace gfx
{
    /// An arbitrary n-vertex convex polygon.
    class ZEN_API zPolygon// : public gfxcore::zDrawable
    {
    public:
        zPolygon(asset::zAssetManager& Assets, const size_t preload = 0);
        ~zPolygon();

        zPolygon(const zPolygon& Copy);
        zPolygon(zPolygon&& Move);

        /**
         * Adds a vertex to the polygon at the specified position.
         *  Vertices form triangles from the first vertex added, and thus
         *  will not look right if there is any notion of concavity.
         *
         * @param   Position    The location to create the next vertex at
         **/
        void AddVertex(const math::vector_t& Position);

        /**
         * Creates the polygon from the internally stored vertices.
         * @pre     >= 3 vertices have been added to the polygon.
         * @post    No temporary vertices are stored until AddVertex()
         *          is called again.
         **/
        virtual zPolygon& Create();

        /**
         * Moves the drawable to a certain location.
         *  This doesn't rely on any vertex data, but rather uses
         *  the model-view matrix to translate the object, thus
         *  there is a default implementation.
         *
         * @param   Position    (x, y, z) coordinates where you want the object
         **/
        virtual void Move(const math::vector_t& Position);

        /// @overload
        virtual void Move(const real_t x, const real_t y, const real_t z = 1.0);

        /**
         * Attaches a material to render on top of the primitive.
         *  This really shouldn't be allowed on simple primitives, but it's
         *  here if you need it. Keep in mind that this will override any
         *  color settings you've made.
         *  Likely this will only work well on quadrilateral primitives due
         *  to difficulties setting texture coordinates on other shapes.
         *
         * @param   Material   The texture you want rendered
         *
         * @pre     Create() or Draw() haven't been called yet.
         *
         * @note    I promise the given material won't be modified.
         **/
        void AttachMaterial(gfx::zMaterial& Material);

        /// Reverts to using the default material.
        void RemoveMaterial();

        /**
         * Draws the primitive on-screen.
         *  This implements the technique described above. If there is no
         *  "owner" of the primitive (meaning no scene has set the internal
         *  data), it will automatically create a zVertexArray instance,
         *  a model-view matrix, and will use the default shader set.
         *  This data will be re-used time after time on subsequent Draw()
         *  calls, not recreated every time.
         *
         * @param   is_bound    Have we bound things? (VAO, material, etc.)
         *
         * @return  `true` if drawing was successful, `false` otherwise.
         **/
        bool Draw(const bool is_bound = false);

        /**
         * Shortcut to prevent loading simple objects manually.
         *  This DOES NOT delete any internal vertex data unless
         *  explicitly specified. Thus, this can be called multiple
         *  times with various vertex settings to set up different
         *  primitives in the given vertex array.
         *
         * @param   VAO         The vertex array to store data into
         * @param   preserve    Should we keep our local vertex data?
         **/
        void LoadIntoVAO(gfxcore::zVertexArray& VAO,
                         const bool preserve = true);

        virtual inline std::vector<math::vector_t> Triangulate() const
        { return math::triangulate(m_Verts); }

        /**
         * Overrides default index creation for the added vertices.
         *  Indices are considered valid if none of them are larger than
         *  the number of vertices. This function does *not* check for that
         *  in order to save speed.
         *
         * @param   Indices The indices to override defaults with
         **/
        void SetIndices(const std::vector<gfxcore::index_t>& Indices);

        /// Sets the vertex color of the *temporary* buffer.
        void SetColor(const color4f_t& Color);

        inline const math::vector_t& GetPosition() const
        { return m_Position; }

        inline real_t GetX() const { return m_Position.x; }
        inline real_t GetY() const { return m_Position.y; }

        /// Calculates maximum height for the current vertices.
        uint16_t GetH() const;

        /// Calculates maximum width for the current vertices.
        uint16_t GetW() const;

        /// Request to see if we can change the internal vertices or not.
        bool IsModifiable() const;

        const gfx::zMaterial& GetMaterial() const;

        /// For setting things implicitly.
        friend class ZEN_API obj::zEntity;

    private:
        asset::zAssetManager&   m_Assets;
        math::matrix4x4_t*      mp_MVMatrix;
        gfxcore::zVertexArray*  mp_VAO;
        gfxcore::index_t        m_offset;

    protected:
        virtual void MapTexCoords() { ZEN_ASSERTM(false, "not implemented"); }

        std::vector<math::vector_t> m_Verts;
        gfx::zMaterial      m_Material;
        math::vector_t      m_Position;
        gfxcore::DrawBatch  m_DrawData;
        color4f_t           m_Color;
        bool                m_internal;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__POLYGON_HPP

/**
 * @class zen::gfx::zPolygon
 * @details
 *  This is a class representing an arbitrary convex polygon that can
 *  accept any number of vertices. It does NOT work with concave polygons,
 *  meaning there can be no way to connect any two vertices with a line that
 *  goes outside of the polygon. The polygon will be generated, of course,
 *  since there is no check for concavity, but it will simply not look
 *  correct when rendered on-screen.
 *
 *  The polygon is generated by drawing triangles to each individual vertex,
 *  beginning from the first vertex added.
 *
 *  Polygons are created a little differently than pre-baked primitives
 *  such as gfx::zQuad, they store vertices in a temporary buffer until
 *  Create() is called. Thus calls to the various `Set*()` methods are
 *  overidden in order to use the temporary buffer rather than the true
 *  internal vertex buffer as specified by the base class gfxcore::zDrawable.
 *
 * @example Polygons
 * @section A Variety of Polgyons
 * @subsection quad Simple Quad
 *  This can obviously be replicated through zen::gfx::zQuad, but is here
 *  for example's sake. We will compare the creation of the two.
 *
 *  @code
 *  // Assuming a manager, etc has already been defined prior.
 *  gfx::zPolygon PolyQuad(Manager);
 *  PolyQuad.AddVertex(0,   0);
 *  PolyQuad.AddVertex(32,  0);
 *  PolyQuad.AddVertex(32,  32);
 *  PolyQuad.AddVertex(0,   32);
 *  PolyQuad.SetColor(color4f_t(1, 0, 0));
 *  PolyQuad.Create();
 *
 *  // Equivalent quad-creating code.
 *  gfx::zQuad RealQuad(32, 32);
 *  RealQuad.SetColor(color4f_t(1, 0, 0));
 *  RealQuad.Create();
 *  @endcode
 *
 *  The benefit, of course, of using the polygon interface vs. the quad
 *  interface is that it eliminates the requirement of being axis-aligned,
 *  thus not rotated and in a rectangular form. With the polygon, we
 *  can make rhombuses and other arbitrary four-sided shapes.
 *
 * @subsection circle   Approximated Circle
 *  Now we will create a circle, or an approximation there-of. It's impossible
 *  of course to create a perfect circle since it's composed of lines, but
 *  we approximate with 360 sides :)
 *
 *  @code
 *  // Assuming a manager, etc has already been defined prior.
 *  gfx::zPolygon Circle(Manager);
 *
 *  real_t radius = 32;
 *  Circle.AddVertex(32, 32);   // Center of the circle
 *  for(uint16_t i = 0; i < 360; ++i)
 *  {
 *      Circle.AddVertex(sin(math::rad(i)) * radius,
 *                       cos(math::rad(i)) * radius);
 *  }
 *
 *  Circle.Create();
 *  @endcode
 **/

/** @} **/
