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
#include "Zenderer/Graphics/Material.hpp"
#include "OpenGL.hpp"
#include "VertexArray.hpp"

namespace zen
{
namespace gfx
{
    // Forward declarations.
    class ZEN_API zWindow;
    class ZEN_API zRenderTarget;
}

namespace gfxcore
{
    /// A variety of blending operations.
    enum class ZEN_API BlendFunc : uint16_t
    {
        DISABLE_BLEND,      ///< Disables blending
        ENABLE_BLEND,       ///< Just enables blending
        STANDARD_BLEND,     ///< Enables the standard blending function
        ADDITIVE_BLEND,     ///< Enables additive blending operation
        IS_ENABLED,         ///< Test if blending is enabled
    };

    /// Abstracts away API-specific rendering operations.
    /// @todo   Add support for depth test toggling.
    /// @todo   Document class in detail.
    class ZEN_API zRenderer
    {
    public:
        /// Sets the blending mode for subsequent draw calls.
        inline static bool BlendOperation(const BlendFunc& Func);

        /// Binds a texture handle.
        inline static bool EnableTexture(const GLuint handle);

        /// Enables / Disables wireframe mode.
        inline static bool ToggleWireframe();

        /// Resets shader and texture state to nothing bound.
        inline static bool ResetMaterialState();

        /// Disables blending (can also be done via BlendOperation()).
        inline static bool DisableBlending();

        /// Disables any bound texture (also via `EnableTexture(0)`).
        inline static bool DisableTexture();

        inline static const gfx::zMaterial&     GetDefaultMaterial();
        inline static const gfx::zEffect&       GetDefaultEffect();
        inline static const gfxcore::zTexture&  GetDefaultTexture();

        inline static const math::matrix4x4_t&  GetProjectionMatrix();
        inline static zVertexArray&             GetFullscreenVBO();

        friend class ZEN_API gfx::zWindow;          ///< Accesses material
        friend class ZEN_API gfx::zRenderTarget;    ///< Accesses matrices

    private:
        explicit zRenderer();
        ~zRenderer();

        /// Only to be called by `gfx::zWindow` ONCE.
        static bool Init(asset::zAssetManager& Assets,
                         const uint16_t w, const uint16_t h);

        static zVertexArray         s_FullscreenQuad;
        static gfx::zMaterial*      s_DefaultMaterial;
        static math::matrix4x4_t    s_ProjMatrix;
        static BlendFunc            s_LastBlend;
        static bool                 s_blend;
        static bool                 s_wf;
    };

    #include "Renderer.inl"
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__RENDERER_HPP

/**
 * @class gfxcore::zRenderer
 * @details
 *  This static class allows for cross-API (when I get there) manipulation
 *  of the current renderer state. From here, you can do useful things like
 *  change blending mode, bind default textures / shaders, and toggle
 *  wireframe mode, all without having to worry about direct, low-level
 *  OpenGL / DirectX API calls.
 **/

/** @} **/
