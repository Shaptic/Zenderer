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
         * @see @ref specs.html#ZMat
         **/
        bool LoadFromFile(const string_t& filename);
        
        /** 
         * Loads a material from an existing file stream.
         *  It must follow the same specifications as LoadFromFile()
         *  The default behavior is parsing from the current stream
         *  location until EOF is reached. If `start` or `end` are specified,
         *  the stream will seek to the `start`, parse till `end` (inclusive)
         *  and then reset the stream to `start` (regardless of errors).
         *
         * @param   stream  File stream to load from
         * @param   start   File position to start loading from (optional)
         * @param   end     File position to end loading from   (optional)
         *
         * @return  `true` if a valid material was loaded, `false` otherwise.
         *
         * @see @ref specs.html#ZMat
         * @see zen::util::CINIParser::LoadFromStream()
         **/
        bool LoadFromStream(std::ifstream& stream,
                            const std::streampos& start = 0,
                            const std::streampos& end   = -1);
        
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

/**
 * @class zen::gfx::CMaterial
 * @details
 *  A material is a combination of an effect (shaders) and a texture.
 *  It's used to specify what picture should be drawn on the screen, and
 *  how it should be drawn. There is a default instance of it in
 *  zen::gfxcore::CRenderer, and it is used for standard rendering to the
 *  screen, primarily for primitives.
 **/

/** @} **/
