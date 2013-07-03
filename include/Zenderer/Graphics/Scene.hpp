/**
 * @file
 *  Zenderer/Graphics/Scene.hpp - A high-level graphics rendering interface intended to abstract away all low-level rendering details.
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
#include "RenderTarget.hpp"

#include "Window.hpp"
#include "Quad.hpp"
#include "Effect.hpp"
#include "Light.hpp"

namespace zen
{
namespace gfx
{
    using gfxcore::CRenderer;
    using gfxcore::BlendFunc;

    class ZEN_API CScene
    {
    public:
        CScene();
        ~CScene();
        
        /**
         * Initializes internal graphical components.
         **/
        bool Init();
        
        //CEntity& AddEntity();

        /**
         * Adds a managed primitive to the scene.
         *  No initialization or setting of parameters is done on the
         *  returned instance, it's merely added to the internal list of
         *  primitives in order to clean them up appropriately and draw
         *  them together in the rendering sequence, and `Create()` is
         *  called, since it takes no parameters and is a default.
         *
         *  We return the value of `Create()` to avoid having the user work
         *  with pointers.
         *
         * @return  A `CDrawable&` instance of whatever type `T` you specified.
         **/
        template<typename T>
        T& AddPrimitive()
        {
            T* pNew = new T;
            m_allPrimitives.push_back(pNew);
            return m_allPrimitives.back()->Create();
        }
        
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
        CLight& AddLight(const LightType& Type)
        {
            CLight* pNew = new CLight(m_Assets, Type, 800);
            pNew->Init();
            m_allLights.push_back(pNew);
            return *m_allLights.back();
        }
        
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
         **/
        CEffect& AddEffect(const EffectType& Type)
        {
            CEffect* pNew = new CEffect(Type);
            pNew->Init();
            m_allPPFX.push_back(pNew);
            return *m_allPPFX.back();
        }
        
        /** 
         * Inserts a primitive at a point in the draw queue.
         *  Sometimes, you need to have something drawn in a different order
         *  than you had originally planned. Thus this method allows you to 
         *  insert primitives at any point in the draw queue. This operation
         *  is `O(1)` thanks to `std::list`, so performance worries are
         *  non-existant.
         *
         * @param   index   The index to insert a primitive at
         *
         * @return  A `CDrawable&` instance of whatever type `T` you specified.
         *
         * @warning The index MUST be in the range of the current list, or
         *          an exception will (likely) be thrown. The range cannot be
         *          checked because this method only returns a valid reference.
         *          Use `IsValidPrimitiveIndex()` to check for a good index.
         *
         * @see     GetPrimitiveIndex()
         * @see     IsValidPrimitiveIndex()
         **/
        template<typename T>
        T& InsertPrimitive(const uint32_t index)
        {
            T* pNew = new T;
            T& Ret = pNew->Create();
            m_allPrimitives.insert(pNew, index);
            return Ret;
        }
        
        /// Renders the scene to the current render target.
        bool Render()
        {
            // We keep only one matrix instance and just 
            // modify it for every object.
            math::matrix4x4_t MV = math::matrix4x4_t::CreateIdentityMatrix();
            
            // Called every frame because there is no more appropriate
            // time to call it. Things won't be offloaded multiple times.
            m_Geometry.Offload();
            
            // Clear our frame buffers from the last drawing.
            // We will be rendering to FBO1 at first.
            m_FBO2.Bind(); m_FBO2.Clear();
            m_FBO1.Bind(); m_FBO1.Clear();
            
            // Set the standard blending state.
            CRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);
            
            // All geometry is stored here.
            m_Geometry.Bind();
            
            // Prepare for primitive rendering.
            material_t& M = CRenderer::GetBlankMaterial();
            CEffect& E = CRenderer::GetDefaultEffect();
            M.Enable();
            
            // Commence individual primitive rendering.
            auto i = m_allPrimitives.begin(),
                 j = m_allPrimitives.end();
                 
            for( ; i != j; ++i) 
            {
                MV.Translate((*i)->GetPosition());
                MV.TranslateAdj(m_Camera);
                
                (*i)->Draw(true);
            }

            M.Disable();

            // Shortcut reference.
            gfxcore::CVertexArray& FS = CRenderer::GetFullscreenVBO();

            // Primitive rendering is complete.
            // Now, render lights with additive blending.
            GLuint final_texture = m_FBO1.GetTexture();
            
            if(m_lighting)
            {
                // We want to render to the entire scene.
                
                // Lighting renders ON the geometry, so we
                // bind the FBO1 texture to render onto. The
                // final result ends up on the FBO2 texture.
                m_FBO2.Enable();
                CRenderer::EnableTexture(final_texture);
                CRenderer::BlendOperation(BlendFunc::ADDITIVE_BLEND);
                
                auto i = m_allLights.begin(),
                     j = m_allLights.end();
                     
                for( ; i != j; ++i) 
                {
                    CLight& L = *(*i);
                    L.Enable();
                    
                    // Move with scene
                    L.Adjust(m_Camera.x, m_Camera.y);
                    
                    FS.Draw();
                    
                    // Restore state
                    L.Adjust(-m_Camera.x, -m_Camera.y);
                    L.Disable();
                }
                
                final_texture = m_FBO2.GetTexture();
                CRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);
            }
            
            // Ping-pong post-processing effects.
            // This means one is drawn to FBO1, then FBO1's result
            // is used to draw on FBO2, etc.
            if(m_ppfx)
            {
                // If there was lighting, the first texture is 
                // the second FBO.
                CRenderTarget& One = m_lighting ? m_FBO2 : m_FBO1;
                CRenderTarget& Two = m_lighting ? m_FBO1 : m_FBO2;
                
                auto i = m_allPPFX.begin(),
                     j = m_allPPFX.end();
                     
                for(size_t c = 0; i != j; ++i, ++c) 
                {
                    bool even = ((c & 0x1) == 0);
                    if(even)    Two.Enable();
                    else        One.Enable();
                    
                    (*i)->Enable();
                    CRenderer::EnableTexture(final_texture);
                    FS.Draw();
                    (*i)->Disable(); 
                  
                    if(even)    final_texture = Two.GetTexture();
                    else        final_texture = One.GetTexture();    
                }
            }
            
            // Now we have the final scene data in `final_texture`
            
            // Doesn't matter which we disable.
            m_FBO1.Unbind();
            
            E.Enable();
            CRenderer::EnableTexture(final_texture);
            
            // Make sure the right data is set.
            E.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
            E.SetParameter("proj", CRenderer::GetProjectionMatrix());
                        
            FS.Draw();
            
            CRenderer::EnableTexture(0);
            E.Disable();
            
            return true;
        }
        
        
        /// Returns the queue index of a certain primitive (or -1).
        int32_t GetPrimitiveIndex(const gfxcore::CDrawable& D)
        {
            auto i = m_allPrimitives.begin(),
                 j = m_allPrimitives.end();
            
            int32_t index = -1;
            for( ; i != j; ++i, ++index)
            {
                if((*i) == &D) return index;
            }
            
            return index;
        }
        
        /// Verifies the given index is within the valid range.
        bool IsValidPrimitiveIndex(int32_t i)
        {
            return (i > 0 && i < m_allPrimitives.size());
        }
        
    private:
        util::CLog&             m_Log;
        asset::CAssetManager    m_Assets;
        gfxcore::CVertexArray   m_Geometry;
        gfx::CRenderTarget      m_FBO1, m_FBO2;
        
        math::vector_t          m_Camera;
        
        // Lists of things that will be rendered.
        std::list<CLight*>              m_allLights;
        std::list<CEffect*>             m_allPPFX;
        std::list<gfxcore::CDrawable*>  m_allPrimitives;
        //std::list<CEntity*> m_allEntities;

        bool m_lighting, m_ppfx;
    };
}
}


#endif // ZENDERER__GRAPHICS__SCENE_HPP

/** @} **/
