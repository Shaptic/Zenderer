/**
 * @file
 *  Zenderer/Graphics/Scene.hpp - A high-level graphics rendering interface
 *  intended to abstract away all low-level rendering details.
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

#ifndef ZENDERER__GRAPHICS__SCENE_HPP
#define ZENDERER__GRAPHICS__SCENE_HPP

#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/CoreGraphics/Sorter.hpp"

#include "Window.hpp"
#include "RenderTarget.hpp"
#include "Effect.hpp"
#include "Light.hpp"
#include "Quad.hpp"

#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gfx
{
    class ZEN_API CScene : public CSubsystem
    {
    public:
        CScene(const uint16_t w, const uint16_t h, asset::CAssetManager& Mgr);
        ~CScene();

        /// Initializes internal graphical components.
        bool Init();
        bool Destroy();

        /**
         * Adds an unloaded, managed entity to the scene.
         *  If the entity is simply left as-returned, nothing will
         *  be drawn on-screen. This method is merely here to abstract
         *  away memory cleanup operations from the user and give them
         *  a valid reference to work with at all times.
         *
         * @return  An unloaded entity instance.
         **/
        obj::CEntity& AddEntity();

        /**
         * Adds a managed primitive to the scene as an entity.
         *  The default primitive initialization options are specified for
         *  this primitive. If you want to specify them on your own, the
         *  suggested approach is adding a raw managed entity via AddEntity(),
         *  and then loading it with primitives yourself.
         *
         * @return  A `CEntity&` instance loaded with the given primitive.
         **/
        template<typename T>
        obj::CEntity& AddPrimitive();

        /**
         * Creates and adds a managed light instance to the scene.
         *  The only setup performed on the light is the initial
         *  loading of the effects. You can check that status by
         *  just calling `IsInit()` on the returned instance.
         *
         *  Light render ordering is irrelevant, therefore there
         *  is no `InsertLight()` or `GetLightIndex()` method
         *  like there are for the other renderable objects.
         *
         * @param   Type    The light type you wish to create
         *
         * @return  A created light instance.
         *
         * @todo    Fix window size
         **/
        CLight& AddLight(const LightType& Type);

        /**
         * Adds an effect to be rendered on the whole scene.
         *  This is a post-processing effect shader that will be applied
         *  to the entire scene after lighting, geometry, etc is renderered.
         *
         *  You can check the creation status by checking the `GetError()`
         *  method of the returned instance.
         *
         *  Though the effect execution order may matter, just add them in
         *  the right order, because there are no `Insert()` or `GetIndex()`
         *  methods provided for this, just like for lights.
         *
         * @param   Type    The type of post-processing you wish to add
         *
         * @return  The effect instance
         *
         * @see     gfx::CEffect::GetError()
         **/
        CEffect& AddEffect(const EffectType& Type);

        /**
         * Inserts an entity at a point in the draw queue.
         *  Sometimes, you need to have something drawn in a different order
         *  than you had originally planned. Thus this method allows you to
         *  insert entities at any point in the draw queue. This operation
         *  is `O(index)`, meaning it's complexity varies in direct proportion
         *  to the `index` parameter.
         *
         * @param   index   The index to insert an entity at
         *
         * @return  A `CEntity&` instance with no data.
         *
         * @warning The index must be in the range of the current list, or
         *          an entity will just be inserted at the end of the internal
         *          list. The range cannot be checked because this method only
         *          returns a valid reference. Use `IsValidEntityIndex()` to
         *          check for a good index.
         *
         * @see     GetEntityIndex()
         * @see     IsValidEntityIndex()
         **/
        obj::CEntity& InsertEntity(const uint32_t index);

        /**
         * Removes an entity instance from the scene, if it exists.
         *
         * @param   Ent     The entity to remove from the manager
         *
         * @return  `true`  if the entity was removed from the internal manager,
         *          `false` if it wasn't found or the index is out of range
         *                  (for the indexed overload, of course).
         **/
        bool RemoveEntity(const obj::CEntity& Obj);
        bool RemoveEntity(const uint32_t index);        ///< @overload

        /// Renders the scene to the screen.
        bool Render();

        /// Verifies the given index is within the valid range.
        bool IsValidEntityIndex(int32_t i);

        /// Returns the queue index of a certain entity (or -1).
        int32_t GetEntityIndex(const obj::CEntity& D);

        /// Enables / disables scene lighting.
        void EnableLighting()  { m_lighting = true;  }
        void DisableLighting() { m_lighting = false; }
        bool ToggleLighting()  { return m_lighting = !m_lighting; }

        /// Enables / disables post processing effects.
        void EnablePostProcessing() { m_ppfx = true; } 
        void DisablePostProcessing(){ m_ppfx = false; }
        bool TogglePostProcessing() { return m_pffx = !m_ppfx; }
        
        /**
         * Allows for underlying rendered data to be seen where scene is empty.
         *  If, for example, geometry is rendered prior to a call to Render()
         *  on this particular scene, and there are portions of the scene
         *  without geometry rendered to it, this will allow for that previously
         *  rendered data to be seen in those parts.
         *
         * @param   flag    Turn it on, or off?
         *
         * @warning This does not work well when combined with lighting.
         *
         * @see     EnableLighting()
         **/
        inline void SetSeeThrough(bool flag) { m_through = flag; }

    private:
        util::CLog&             m_Log;
        asset::CAssetManager&   m_Assets;
        gfxcore::CVertexArray   m_Geometry;
        gfx::CRenderTarget      m_FBO1, m_FBO2;

        math::vector_t          m_Camera;

        // Lists of things that will be rendered.
        std::list<CLight*>          m_allLights;
        std::list<CEffect*>         m_allPPFX;
        std::list<obj::CEntity*>    m_allEntities;

        bool m_lighting, m_ppfx, m_through;
    };

    #include "Scene.inl"
}
}

#endif // ZENDERER__GRAPHICS__SCENE_HPP

/** @} **/
