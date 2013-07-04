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
    class ZEN_API CWindow;
    class ZEN_API CRenderTarget;
    class ZEN_API CScene;
}

namespace gfxcore
{
    enum class BlendFunc : uint16_t
    {
        DISABLE_BLEND,
        ENABLE_BLEND,
        STANDARD_BLEND,
        ADDITIVE_BLEND
    };

    /// Abstracts away API-specific rendering operations.
    class ZEN_API CRenderer
    {
    public:
        virtual ~CRenderer();

        inline static bool BlendOperation(const BlendFunc& Func);

        inline static bool EnableTexture(const GLuint handle);
        inline static bool ResetMaterialState();
        inline static bool DisableBlending();
        inline static bool DisableTexture();

        inline static gfx::material_t&          GetDefaultMaterial();
        inline static gfx::CEffect&             GetDefaultEffect();
        inline static const gfxcore::CTexture&  GetDefaultTexture();

        inline static const math::matrix4x4_t&  GetProjectionMatrix();
        inline static CVertexArray&             GetFullscreenVBO();

        /// Only the scenes can modify graphical API state.
        friend class gfx::CScene;

        // Can modify and store static data
        friend class gfx::CWindow;
        friend class gfx::CRenderTarget;

    protected:
        explicit CRenderer();

        /// Only to be called by `gfx::CWindow` ONCE.
        static bool Init(const uint16_t w, const uint16_t h);

        static CVertexArray         s_FullscreenQuad;
        static gfx::CMaterial       s_DefaultMaterial;
        static gfx::CEffect         s_DefaultEffect;
        static gfxcore::CTexture*   s_DefaultTexture;
        static math::matrix4x4_t    s_ProjMatrix;
        static bool                 s_blend;
    };

    #include "Renderer.inl"
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__RENDERER_HPP

/** @} **/
