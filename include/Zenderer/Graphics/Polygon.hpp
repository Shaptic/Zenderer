/**
 * @file
 *  Zenderer/CoreGraphics/Polygon.hpp - A vertex-agnostic convex primitive.
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

#include <algorithm>

#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/Math/Shapes.hpp"

namespace zen
{
namespace obj { class ZEN_API zEntity; }
namespace gfx
{
    /// An arbitrary n-vertex convex polygon.
    class ZEN_API zPolygon
    {
    public:
        zPolygon(asset::zAssetManager& Assets, const size_t preload = 0);
        virtual ~zPolygon();

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
         *  If `do_triangulation` is set to `false`, the internal
         *  triangulation storage will simply default to the contents
         *  of the current temporary vertices.
         *  This comes in handy if, for example, you wish to directly
         *  implement a concave polygon (without using zConcavePolygon).
         *  In this case, you would set custom indices, then Create() the
         *  polygon without triangulation, since the vertices you've given
         *  would already form triangles.
         *  This type of processing is used by zen::lvl::zLevelLoader.
         *
         * @param   do_triangulation    Should we triangulate?
         *
         * @pre     >= 3 vertices have been added to the polygon.
         * @post    No temporary vertices are stored until AddVertex()
         *          is called again.
         **/
        virtual zPolygon& Create(const bool do_triangulation = true);

        /**
         * Moves the drawable to a certain location.
         *  This doesn't rely on any vertex data, but rather uses
         *  the model-view matrix to translate the object, thus
         *  there is a default implementation.
         *
         * @param   Position    (x, y) coordinates where you want the object
         **/
        virtual void Move(const math::vector_t& Position);

        /// @overload
        virtual void Move(const real_t x, const real_t y);

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

        /**
         * Attempts collision detection with another polygon.
         *  This function is O(n^2), so its preferable to do a generic bounding
         *  box collision detection prior to this.
         *
         * @param   Other   Polygon to check collision with
         * @param   poi     Optional point of impact specifier.
         *
         * @return  `true`  if this polygon collides with the given parameter,
         *          `false` otherwise.
         **/
        virtual bool Collides(const zPolygon& Other, math::vector_t* poi = nullptr);
        virtual bool Collides(const math::aabb_t& other);   ///< @overload

        /**
         * Overrides default index creation for the added vertices.
         *  Indices are considered valid if none of them are larger than
         *  the number of vertices. This function does *not* check for that
         *  in order to save speed.
         *
         * @param   Indices The indices to override defaults with
         **/
        void SetIndices(const std::vector<gfxcore::index_t>& Indices);

        /// Sets the vertex color of the created vertices.
        void SetColor(const color4f_t& Color);
        void SetColor(const real_t r, const real_t g,
                      const real_t b, const real_t a = 1.0); ///< @overload

        inline const std::vector<math::vector_t>&
        GetTriangulation() const { return m_Tris; }

        inline math::vector_t GetPosition() const
        { return math::vector_t(m_BoundingBox.x, m_BoundingBox.y); }

        inline const math::rect_t&
        GetBoundingBox() const { return m_BoundingBox; }

        inline real_t GetX() const { return m_BoundingBox.x; }
        inline real_t GetY() const { return m_BoundingBox.y; }

        /// Gets preset maximum height for the current vertices.
        inline uint16_t GetH() const { return m_BoundingBox.h; }

        /// Gets preset width for the current vertices.
        inline uint16_t GetW() const { return m_BoundingBox.w; }

        /// Calculates the lowest X value for the polygon.
        int16_t CalcX() const;

        /// Calculates the lowest Y value for the polygon.
        int16_t CalcY() const;

        /// Calculates maximum height for the current vertices.
        /// Caches it for retrieval via GetH().
        uint16_t CalcH();

        /// Calculates width for the current vertices.
        /// Caches it for retrieval via GetW().
        uint16_t CalcW();

        int GetLowPoint() const;
        int GetLeftPoint() const;

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

        std::vector<math::vector_t> m_Verts, m_Tris;
        gfx::zMaterial      m_Material;
        math::rect_t        m_BoundingBox;
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
 *  Polygons present the basis for inheriting classes that provide pre-baked
 *  vertex configurations, such as gfx::zQuad. Vertices are stored in a
 *  temporary buffer until Create() is called.
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
 *  gfx::zPolygon Circle(Manager, 360);
 *
 *  real_t radius = 32;
 *  Circle.AddVertex(32, 32);   // Center of the circle
 *  for(uint16_t i = 0; i < 360; ++i)
 *  {
 *      Circle.AddVertex(std::sin(math::rad(i)) * radius,
 *                       std::cos(math::rad(i)) * radius);
 *  }
 *
 *  Circle.Create();
 *  @endcode
 **/

/** @} **/
