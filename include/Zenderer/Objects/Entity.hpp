/**
 * @file
 *  Zenderer/Objects/Entity.hpp - A generic entity class wrapping drawable
 *  primitives, materials, and easy sprite-like functionality together.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.0
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
 * @addtogroup Objects
 *  A group containing renderable objects with a variety of controllable
 *  options and features like animation and collision detection.
 *  These classes are a step above the simple primitive rendering scheme
 *  set up by the architecture in zen::gfxcore::CDrawable, and are the most
 *  likely to face direct user interaction through the application.
 *
 * @{
 **/

#ifndef ZENDERER__OBJECTS__ENTITY_HPP
#define ZENDERER__OBJECTS__ENTITY_HPP

#include <regex>

#include "Zenderer/Math/Math.hpp"
#include "Zenderer/CoreGraphics/Drawable.hpp"
#include "Zenderer/Graphics/Material.hpp"
#include "Zenderer/Graphics/Quad.hpp"
#include "Zenderer/Utilities/INIParser.hpp"

namespace zen
{
namespace gfx { class ZEN_API CScene; }
namespace gui { class ZEN_API CFont;  }

/// A group of sprite-like objects that operate at a high level of abstraction.
namespace obj
{
    /// A base class for all "in-game" objects.
    class ZEN_API CEntity
    {
    protected:
        // Internal error types.
        enum class ZEN_API ErrorType : uint16_t
        {
            BAD_PAIR,
            BAD_POSITION,
            BAD_MATERIAL,
            NO_TEXTURE
        };

    public:
        explicit CEntity(asset::CAssetManager& Assets);
        virtual ~CEntity();

        /**
         * Creates an entity mesh from a file.
         *  There is a detailed specification for well-formed loading of
         *  single and multi-primitive entity loading with unique material
         *  attachments for each. This will implicitly call `Optimize()`
         *  when loading is complete.
         *
         * @param   filename    The path to the entity file.
         *
         * @return  `true`  if loaded successfully, and
         *          `false` otherwise. There is no retrievable error, but
         *                  something appropriate will be logged.
         *
         * @see     The entity file specification page [here](index.html)
         **/
        bool LoadFromFile(const string_t& filename);

        /**
         * Creates an entity from a texture file.
         *  This will simply make a quad entity the exact width and height
         *  as the provided texture returns.
         *
         * @param   filename    Path to texture
         *
         * @return  `true`  if the image and entity loaded successfully,
         *          `false` otherwise. There is no retrievable error, but
         *                  something appropriate will be logged.
         **/
        bool LoadFromTexture(const string_t& filename);

        /**
         * Creates an entity from an existing primitive, or adds it.
         *  This will store a copy of the primitive locally, to prevent bad
         *  references occuring later. Thus it's advisable to `delete` heap
         *  memory when done here, or ensure that the given primitive will go
         *  off the stack soon, to prevent uneccessary memory duplication.
         *
         * @param   Prim    A primitive with a non-default material attached
         *
         * @return  `true`  if the primitive loaded with a valid textre, and
         *          `false` otherwise.
         *
         * @note    Currently, multiple calls to this method will stack
         *          primitive on top of one another, since there is no position
         *          specification on them.
         **/
        bool AddPrimitive(const gfx::CQuad& Prim);

        /**
         * Merges primitive data with shared materials.
         *  If multiple internal quads share the same material data, this will
         *  merge their vertex buffer data together in order to minimize draw
         *  calls and state changes later when rendering.
         *
         * @return  `true`  if any optimization was done, `false` otherwise.
         *
         * @warning Not implemented.
         **/
        bool Optimize();

        /**
         * Draws the entity on-screen.
         *  Since entities are just collections of one or more primitives, this
         *  will simply call `gfx::CQuad::Draw` on each individual internal
         *  primitive.
         *
         *  If `is_bound` is `true`, the draw call will only do drawing (go
         *  figure). Otherwise, it will create an internal vertex array (on
         *  first draw), bind its local material, set up the model-view matrix,
         *  etc, and then do drawing. This allows for various scenarios and
         *  prevents the user from having to set up this data on their own if
         *  they just want to get something on-screen.
         *
         * @param   is_bound    Is there already buffer / material data bound?
         *
         *
         **/
        bool Draw(bool is_bound = false);

        /**
         * Moves the entity to the specified position.
         *  In future revisions, this will adjust all internal primitives
         *  accordingly, once the default primitive stacking behavior is
         *  removed.
         *
         * @param   Pos     The coordinate to place the entity at
         *
         * @todo    Support a variety of primitive depths.
         **/
        void Move(const math::vector_t& Pos);

        /// @overload
        void Move(const real_t x, const real_t y, const real_t z = 1.0);

        /// Transforms the entity with a shear.
        inline void Shear(const math::vector_t& Angles) { m_MV.Shear(Angles); }

        /// Transforms the entity with scaling factors.
        inline void Scale(const math::vector_t& Factors){ m_MV.Scale(Factors);}

        /**
         * Inverts the vertex and texture coordinates of internal primitives.
         *  This will do nothing for existing primitives and data, and only
         *  applies to things that will be loaded after this call.
         *
         * @pre     Nothing has been loaded into the entity.
         *
         * @see     zen::gfx::CQuad::SetInverted()
         **/
        inline void Invert() { m_inv = true; }

        /**
         * Offloads the internal primitives to the given GPU buffer.
         *  If `keep` is specified (the default), the primitives will maintain
         *  their data and just offload a copy of it. Otherwise, local data
         *  will be permanently deleted until more data is added.
         *
         * @param   VAO     The vertex array to offload data to
         * @param   keep    Keep the vertex data locally? (optional=`true`)
         *
         * @note    When permanently offloaded with `keep=false`, later adding
         *          more primitives or reloading more data is untested and
         *          may cause problems.
         *
         * @warning The data will stay on the GPU, occupying precious video
         *          memory, until the `gfxcore::CVertexArray` is cleared.
         *
         * @see     zen::gfxcore::CVertexArray::Clear()
         **/
        void Offload(gfxcore::CVertexArray& VAO, const bool keep = true);

        /// Checks if the entity has offloaded any vertex data to a GPU buffer.
        bool Offloaded() const;

        /// Sets the depth of the entity, for shadows or masking later on.
        void SetDepth(uint16_t depth);

        /// Retrieves an immutable reference to the transformation matrix.
        const math::matrix4x4_t& GetTransformation() const;
        math::vector_t GetPosition() const;

        uint32_t GetSortFlag() const;

        /// Returns an iterator to the start of the internal primitive list.
        std::vector<gfx::CQuad*>::const_iterator cbegin() const;

        /// Returns an iterator to the end of the internal primitive list.
        std::vector<gfx::CQuad*>::const_iterator cend() const;

        friend class ZEN_API gui::CFont;
        friend class ZEN_API gfx::CScene;

    protected:
        void Destroy();
        bool FileError(const string_t& filename,
                       const string_t& line, const uint32_t line_no,
                       const ErrorType& Err = ErrorType::BAD_PAIR);

        asset::CAssetManager&       m_Assets;
        util::CLog&                 m_Log;

        math::matrix4x4_t           m_MV;
        std::vector<gfx::CQuad*>    mp_allPrims;
        string_t                    m_filename;
        uint16_t                    m_depth;
        uint32_t                    m_sort;
        bool                        m_inv;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__OBJECTS__ENTITY_HPP

/**
 * @class zen::obj::CEntity
 * @details
 *  These differ from generic primitives in the sense that they have a lot more
 *  functionality abstracted away, as well as additional functions like
 *  animation and physics reactions (in inheriting classes). They do use the
 *  zen::gfx::CQuad class at their core, but function at a much higher level.
 *
 *  They can also be loaded from files (see the spec) and can
 *  contain multiple primitive instances.
 *
 *  When creating an entity from multiple primitives, a copy of the primitive
 *  data will be stored internally in order to prevent a bad reference later. Thus
 *  it's recommended that if you will be creating via `AddPrimitive()` that you
 *  either dynamically allocate the primitives and then `delete` later, or you
 *  ensure that they go out of scope soon to prevent useless data duplication.
 *
 *  The call to `Optimize()` is not necessary, but is recommended to merge
 *  identical primitives together if they use the same material, especially
 *  if you've got some CPU cycles to spare, because it will save render state
 *  switches down the road.
 *
 * @see specs.html
 *
 * @todo    Allow for additional primitives not to stack up.
 **/

/** @} **/
