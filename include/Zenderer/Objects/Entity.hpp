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
        enum class ErrorType : uint16_t
        {
            BAD_PAIR,
            BAD_POSITION,
            BAD_MATERIAL,
            NO_TEXTURE
        };
        
    public:
        CEntity(asset::CAssetManager& Assets) : 
            m_Assets(Assets), m_Log(util::CLog::GetEngineLog()) {}
        virtual ~CEntity();

        bool LoadFromFile(const string_t& filename);
        bool LoadFromTexture(const string_t& filename);
        
        bool AddPrimitive(const gfx::CQuad& Prim);
        bool Create();
        bool Optimize() { ZEN_ASSERTM(false, "not implemented"); return false; }
        bool Draw(bool is_bound = false);
        
        void Move(const real_t x, const real_t y, const real_t z = 1.0)
        {
            ZEN_ASSERTM(false, "not implemented");
        }

        friend class gui::CFont;
        friend class gfx::CScene;
        
    protected:
        void Destroy();
        bool FileError(const string_t& filename,
                       const string_t& line, const uint32_t line_no,
                       const ErrorType& Err = ErrorType::BAD_PAIR);
        
        asset::CAssetManager&           m_Assets;
        util::CLog&                     m_Log;
        
        math::vector_t                  m_Position;
        string_t                        m_filename;
        std::vector<gfx::CQuad*>        mp_allPrims;
        std::vector<gfx::CMaterial*>    mp_allMaterials; 
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
 *  if you've got some CPU cycles to spare.
 *
 * @see specs.html#ZEnt
 **/

/** @} **/
