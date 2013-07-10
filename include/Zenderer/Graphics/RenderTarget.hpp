/**
 * @file
 *  Zenderer/Graphics/RenderTarger.hpp - An object wrapper for OpenGL
 *  frame buffer objects.
 *
 * @author      George Kudrayvtsev (halcyon)
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
    class ZEN_API CRenderTarget : gfxcore::CGLSubsystem
    {
    public:
        explicit CRenderTarget(const math::rect_t& Dimensions);
        CRenderTarget(const uint16_t w, const uint16_t h);

        ~CRenderTarget();

        bool Init();
        bool Destroy();

        bool Bind() const;
        bool BindTexture() const;        
        bool Unbind() const;
        
        bool Clear();

        bool AttachDepthBuffer();
        bool AttachStencilBuffer();

        GLuint GetObjectHandle() const;
        GLuint GetTexture() const;

    private:
        util::CLog& m_Log;

        math::vector_t m_OldViewport, m_Viewport;
        math::matrix4x4_t m_ProjMatrix, m_Main;

        string_t m_error_str;

        GLuint m_texture, m_fbo;
        GLuint* m_rbos;
        GLuint m_rbo_count;
    };
}
}

#endif // ZENDERER__GRAPHICS__RENDER_TARGET_HPP

/**
 * @class gfx::CRenderTarger
 *
 * @details
 *  Render targets are useful for rendering things off-screen,
 *  multi-pass rendering, and other useful routines for rendering
 *  things to a texture, and then using that texture for later
 *  operations.
 **/

/** @} **/
