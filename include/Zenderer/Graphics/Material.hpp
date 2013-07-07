/**
 * @file
 *  Zenderer/Graphics/Material.hpp - Defines a material structure for rendering different images and effects.
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

#ifndef ZENDERER__GRAPHICS__MATERIAL_HPP
#define ZENDERER__GRAPHICS__MATERIAL_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Utilities/INIParser.hpp"
#include "Zenderer/CoreGraphics/Texture.hpp"
#include "Effect.hpp"

namespace zen
{
namespace gfxcore { class ZEN_API CRenderer; }
namespace gfx
{
    /// Rendering data composed of a shader effect and a texture.
    class ZEN_API CMaterial
    {
    public:
        explicit CMaterial(asset::CAssetManager* Assets = nullptr);
        CMaterial(gfxcore::CTexture& Texture, gfx::CEffect& Effect,
                  asset::CAssetManager* Assets = nullptr);

        ~CMaterial();

        /**
         * Loads a material from a `.zfx` file.
         * @see /specs.html#zmat
         **/
        bool LoadFromFile(const string_t& filename);
        
        /// Loads a texture from disk.
        bool LoadTextureFromFile(const string_t& filename);
        
        /// Loads an effect from disk.
        bool LoadEffect(const EffectType Type);

        /// Attaches the given effect/texture to the material.
        bool Attach(CEffect& E, gfxcore::CTexture& T);
        
        /// Temporary `rvalue` references are not allowed.
        bool Attach(CEffect&& E, gfxcore::CTexture&& T);

        /// Enables the effect and texture for rendering.
        bool Enable() const;        
        bool EnableEffect() const;      ///< Enables only the effect.
        bool EnableTexture() const;     ///< Enables only the texture.

        /// Disables rendering with the effect/texture.
        bool Disable() const;
        bool DisableEffect() const;     ///< Disables only the effect.
        bool DisableTexture() const;    ///< Disables only the texture.

        CEffect* GetEffect();
        gfxcore::CTexture* GetTexture() const;

        friend class gfxcore::CRenderer;

    private:
        void Destroy();
        
        asset::CAssetManager&   m_Assets;
        util::CLog&             m_Log;
        gfxcore::CTexture*      mp_Texture;
        gfx::CEffect*           mp_Effect;

        bool m_egiven, m_tgiven;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__MATERIAL_HPP

/** @} **/
