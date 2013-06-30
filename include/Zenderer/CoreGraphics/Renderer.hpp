/**
 * @file
 *  Zenderer/CoreGraphics/Renderer.hpp - A low-level class to take care of
 *  rendering state and other graphical management abstractions.
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
 * @addtogroup CoreGraphics
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__RENDERER_HPP
#define ZENDERER__CORE_GRAPHICS__RENDERER_HPP

#include "Zenderer/Utilities/Settings.hpp"
#include "Zenderer/Math/Matrix.hpp"
#include "Zenderer/Graphics/Effect.hpp"
#include "OpenGL.hpp"
#include "VertexArray.hpp"

namespace zen
{
namespace gfx { class ZEN_API CWindow; }

namespace gfxcore
{
    enum class ZEN_API RenderState : uint16_t
    {
        ZEN_NORMAL_RENDER,
        ZEN_OFFSCREEN_RENDER,
        ZEN_ALPHA_RENDER,
        ZEN_LIGHTING_RENDER,
        ZEN_POSTPROCESS_RENDER
    };

    class ZEN_API CRenderer
    {
    public:
        virtual ~CRenderer();

        static gfx::CEffect& GetDefaultEffect()
        {
            return s_DefaultShader;
        }

        static const math::matrix4x4_t& GetProjectionMatrix()
        {
            return s_ProjMatrix;
        }

        static void ResetMaterialState()
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);
        }

        static void EnableAlphaBlending()
        {
            GL(glEnable(GL_BLEND));
            GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }

        static void DisableAlphaBlending()
        {
            GL(glDisable(GL_BLEND));
        }

        /// Only the scenes can modify graphical API state.
        //friend class gfx::CScene;
        friend class gfx::CWindow;

    protected:
        CRenderer(const util::CSettings& Settings);

        virtual void SaveRenderState();
        virtual void PrepareRenderState(const RenderState& Type)
        {
            switch(Type)
            {
            case RenderState::ZEN_NORMAL_RENDER:
                // @todo: set identity matrix, proj matrix from window.
                s_DefaultShader.Enable();
                //m_FullscreenQuad.Prepare();
                //m_FinalTarget.GetTexture().Bind();

                m_FullscreenQuad.Draw();

                // Return to previous state.
                //m_FinalTarget.GetTexture().Unbind();
                //m_FullscreenQuad.ResetState();
                s_DefaultShader.Disable();
                break;

            case RenderState::ZEN_OFFSCREEN_RENDER:
                break;
            }

            m_LastState = Type;
        }

        // Uses last render state.
        virtual void ResetRenderState();

        // CScene::Render() :
        // ...
        // gfxcore::CTexture* pWrapper =
        //      m_Assets.Create<gfxcore::CTexture*>(this);
        // pWrapper->LoadFromTexture(final_texture);
        // m_Renderer.SetFinalTexture(pWrapper);
        // ...

        void SetFinalTexture(const void* pTexture) //gfxcore::CTexture* pTexture)
        {
            ZEN_ASSERT(pTexture != nullptr);

            //m_FBOTexture = *pTexture;
        }

        void UpdateConfiguration();

        virtual inline bool Enable(const int flag);
        virtual inline bool Disable(const int flag);

        virtual void PrepareAlphaRenderState();
        virtual void PreparePostFXRenderState();
        virtual void PrepareFBORenderState();
        virtual void PrepareLightingRenderState();

        CVertexArray    m_FullscreenQuad;
        static gfx::CEffect s_DefaultShader;
        //gfx::CRenderTarget  m_FinalScene;
        static math::matrix4x4_t s_ProjMatrix;

        RenderState         m_LastState;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__RENDERER_HPP

/** @} **/
