/**
 * @file
 *  Zenderer/Graphics/Window.hpp - An OpenGL context wrapper that the
 *  engine performs operations on.
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
#include "Zenderer/Events/EventHandler.hpp"

#include "Effect.hpp"

namespace zen
{

/// Encompasses all user-level graphics APIs.
namespace gfx
{
    /// Minimum supported OpenGL API version.
    static const real_t ZENDERER_GL_VERSION = 3.3;

    /// Creates a renderable OpenGL context.
    class ZEN_API CWindow : public CSubsystem
    {
    public:
        /**
         * Constructs and sets up an OpenGL-enabled window for rendering.
         *  The asset manager is there to allow for assets to be properly
         *  reloaded when toggling fullscreen mode. When going into (or out of)
         *  fullscreen mode, the current OpenGL context is destroyed, thus
         *  invalidating any and all handles to textures, shaders, vertex
         *  buffers, and other critical OpenGL objects used during rendering.
         *  By attaching a proper asset manager to the window, these can be
         *  reloaded on-the-fly when switching contexts.
         *
         *  Since the asset base class (zen::asset::CAsset) stores a
         *  filename, this can be used to reload the asset when necessary.
         *  Keep in mind that assets that were generated programmatically
         *  (such as various primitives) that do not depend on any files cannot
         *  normally be successfully reloaded and attached properly to the new
         *  rendering context.
         *
         * @param   w       Window width
         * @param   h       Window height
         * @param   caption Caption to place in the title bar (windowed mode)
         * @param   Mgr     Manager to attach to window
         *
         * @note    The window will automatically be created fullscreen in
         *          release mode (@ref ZEN_DEBUG_BUILD).
         *
         * @see     Init()
         * @see     EnableFullscreen()
         * @see     DisableFullscreen()
         * @see     ToggleFullscreen()
         * @see     zen::asset::CAsset
         **/
        CWindow(const uint16_t w,
                const uint16_t h,
                const string_t& caption,
                asset::CAssetManager& Mgr);

        ~CWindow();

        bool Init();

        /**
         * Destroys the current OpenGL-enabled window.
         *  This should only be called if you are planning on calling Init()
         *  again, otherwise this will ruin any OpenGL object cleanup and cause
         *  fatal errors in debug builds. Leave the window management to 
         *  zen::Quit().
         *
         * @return  `true`, always.
         *
         * @post    All assets from attached manager are destroyed.
         *
         * @warning Invalidates all OpenGL objects.
         **/
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
         *
         * @todo    Implement this
         **/
        inline bool ToggleFullscreen(int* const loaded = nullptr);

        inline bool EnableFullscreen();     ///< Enables fullscreen mode.
        inline bool DisableFullscreen();    ///< Disables fullscreen mode.

        bool IsOpen() const;

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

        inline uint16_t GetWidth()  const { return m_Dimensions.x; }
        inline uint16_t GetHeight() const { return m_Dimensions.y; }

        /// @todo Remove when uneccessary.
        inline GLFWwindow* GetWindow() const { return mp_Window; }
        
        math::vector_t GetMousePosition() const;
        bool GetMouseState(const evt::MouseButton& Btn) const;

        static bool ToggleVSYNC();

    private:
        // Prevent copying.
        CWindow(const CWindow&);
        CWindow& operator=(const CWindow&);

        GLFWwindow*             mp_Window;
        util::CLog&             m_Log;
        asset::CAssetManager&   m_Assets;

        math::Vector<uint16_t>  m_Dimensions;
        math::matrix4x4_t       m_ProjMatrix;
        string_t                m_caption;

        uint32_t        m_clearbits;
        bool            m_fullscreen;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__WINDOW_HPP

/**
 * @class zen::gfx::CWindow
 * @details
 *  This is a wrapper for creating a core OpenGL context to draw to. There
 *  is no testing of multiple windows (and thus multiple engine instances)
 *  but this likely isn't supported, especially since there is no way to
 *  specify which window is active. This is supported by GLFW, the underlying
 *  context creation API, so it's a potential `todo`.
 *  This absolutely must be the first thing you create, prior to interaction
 *  with any part of the graphics API. If it isn't, your objects will likely
 *  be invalid, or will simply fail to load (and destroy).
 *
 * @todo    Test / Implement multiple-context control.
 **/

/** @} **/
