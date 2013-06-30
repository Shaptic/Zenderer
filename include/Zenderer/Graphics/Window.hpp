/**
 * @file
 *  Zenderer/Graphics/Window.hpp - An OpenGL context wrapper that the
 *  engine performs operations on.
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
 *  This engine component contains a high-level graphics API that allows
 *  for manipulation of effects, lights, and other features without
 *  worrying about low-level details.
 *
 * @{
 */

#ifndef ZENDERER__GRAPHICS__WINDOW_HPP
#define ZENDERER__GRAPHICS__WINDOW_HPP

#include <cmath>

#include "Zenderer/Core/Subsystem.hpp"
#include "Zenderer/Utilities/Log.hpp"
#include "Zenderer/Math/Vector.hpp"
#include "Zenderer/Math/Matrix.hpp"

#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/Assets/AssetManager.hpp"

#include "Effect.hpp"

namespace zen
{
    typedef bool (APIENTRY* PFNWGLSWAPINTERVALFARPROC)(int);
    static PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

/// Encompasses all user-level graphics APIs.
namespace gfx
{
    /// Minimum supported OpenGL API version.
    static const real_t ZENDERER_GL_VERSION = 3.3;

    /// Creates a renderable OpenGL context.
    class ZEN_API CWindow : public CSubsystem
    {
    public:
        CWindow(const uint16_t w,
                const uint16_t h,
                const string_t& window_caption);

        ~CWindow();

        bool Init();
        bool Destroy();

        /// Clears the framebuffer to black.
        bool Clear();

        /// Clears the framebuffer to a specified color.
        bool Clear(const color4f_t& Color);

        /// Performs a context update.
        void Update() const;

        /**
         * Toggles the OpenGL context into/out of fullscreen mode.
         *  Switching in and out of fullscreen mode requires a complete
         *  destruction and re-creation of the OpenGL context, invalidating
         *  any and all handles to OpenGL objects like textures, vertex
         *  buffers, and shaders. Thus, it is necessary for the user to
         *  attempt to keep track reloading assets and OpenGL data as
         *  needed.
         *  If a manager was given via AttachAssetManager(), an attempt
         *  will be made to reload all of the assets it contains via the
         *  zen::asset::CAsset::Reload method. The optional `loaded`
         *  parameter will be set to the number of assets that were
         *  reloaded successfully, based on the return value of the method.
         *
         * @param   loaded  Track number of assets reloaded (optional=`nullptr`)
         *
         * @return  `true`  if the window is now in fullscreen mode, and
         *          `false` if it was changed to windowed mode.
         **/
        inline bool ToggleFullscreen(int* const loaded = nullptr);

        inline bool EnableFullscreen();     ///< Enables fullscreen mode.
        inline bool DisableFullscreen();    ///< Disables fullscreen mode.

        inline bool IsOpen() const 
        {
            return !glfwWindowShouldClose(mp_Window);
        }

        /**
         * Allows for assets to be properly reloaded when toggling fs mode.
         *  When going into (or out of) fullscreen mode, the current OpenGL
         *  context is destroyed, thus invalidating any and all handles to
         *  textures, shaders, vertex buffers, and other critical
         *  properties used during rendering. By attaching a proper asset
         *  manager to the window, these can be reloaded on-the-fly when
         *  switching contexts.
         *  Since the asset base class (zen::asset::CAsset) stores a
         *  filename, this can be used to reload the asset when necessary.
         *  Keep in mind that assets that were generated programmatically
         *  (such as various shapes) that do not depend on any files cannot
         *  be successfully reloaded and attached properly to the new
         *  rendering context.
         *
         * @param   Mgr     Manager to attach to window
         *
         * @see     zen::asset::CAsset
         * @see     EnableFullscreen()
         * @see     DisableFullscreen()
         * @see     ToggleFullscreen()
         **/
        void AttachAssetManager(asset::CAssetManager& Mgr);

        /**
         * Sets OpenGL clearing bits.
         *  By default, calling Clear() will clear the color buffer. This
         *  method allows the user to specify exactly which bits get
         *  cleared.
         *
         * @param   bits    OpenGL bits to clear (`GL_*_BIT`)
         *
         * @see     GL/gl.h
         **/
        void SetClearBits(const uint32_t bits);

        inline const math::matrix4x4_t& GetProjectionMatrix() const
        { return m_ProjMatrix; }

        static inline bool ToggleVSYNC()
        {
            static bool on = true;

            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)
                wglGetProcAddress("wglSwapIntervalEXT");
            wglSwapIntervalEXT(on = !on);

            return on;
        }

    private:
        GLFWwindow*             mp_Window;
        util::CLog&             m_Log;
        asset::CAssetManager*   mp_Assets;

        math::vector_t      m_Dimensions;
        math::matrix4x4_t   m_ProjMatrix;
        string_t            m_caption;

        uint32_t        m_clearbits;
        bool            m_fullscreen;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__WINDOW_HPP

/** @} **/
