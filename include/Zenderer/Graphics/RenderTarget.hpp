/**
 * @file
 *  Zenderer/Graphics/RenderTarget.hpp - An object wrapper for OpenGL
 *  frame buffer objects.
 *
 * @author      George (@_Shaptic)
 * @version     1.1
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

#ifndef ZENDERER__GRAPHICS__RENDER_TARGET_HPP
#define ZENDERER__GRAPHICS__RENDER_TARGET_HPP

#include "Zenderer/Utilities/Log.hpp"
#include "Zenderer/Math/Math.hpp"
#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"

namespace zen
{
namespace gfx
{
    /// Creates a target for all subsequent rendering operations.
    class ZEN_API zRenderTarget : gfxcore::zGLSubsystem
    {
    public:
        /// Creates a render target of the specified dimensions.
        explicit zRenderTarget(const math::rect_t& Dimensions);
        zRenderTarget(const uint16_t w, const uint16_t h);  ///< @overload

        ~zRenderTarget();

        /// Creates the render target with an attached texture.
        bool Init();

        /// Destroys the render target and everything attached to it.
        bool Destroy();

        /// All draws after this call will go to the texture, not the screen.
        bool Bind() const;

        /// Resume drawing to the screen.
        bool Unbind() const;

        /// Binds the render target's texture for drawing or modification.
        bool BindTexture() const;

        /// Clears the rendering target texture with a color (default black).
        bool Clear(const color4f_t C = color4f_t(0.0, 0.0, 0.0, 1.0));

        bool AttachDepthBuffer();   ///< Attach depth buffer to render target.
        bool AttachStencilBuffer(); ///< Attach stencil to the render target.

        /// Get the raw render target OpenGL handle.
        GLuint GetObjectHandle() const;

        /// Get the raw render target's texture handle.
        GLuint GetTexture() const;

        inline uint16_t GetHeight() const { return m_Viewport.y; }
        inline uint16_t GetWidth()  const { return m_Viewport.x; }

    private:
        util::zLog& m_Log;

        glm::u16vec2 m_OldViewport, m_Viewport;
        glm::mat4 m_Main;

        string_t m_error_str;

        GLuint m_texture, m_fbo;
        GLuint* m_rbos;
        GLuint m_rbo_count;
    };
}
}

#endif // ZENDERER__GRAPHICS__RENDER_TARGET_HPP

/**
 * @class zen::gfx::zRenderTarget
 * @details
 *  Render targets are useful for rendering things off-screen,
 *  multi-pass rendering, and other useful routines for rendering
 *  things to a texture, and then using that texture for later
 *  operations.
 *  It's used extensively in the gfx::zScene API in order to
 *  provide post-processing effects and additive lighting effects.
 **/

/** @} **/
