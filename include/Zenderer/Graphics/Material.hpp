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
        CMaterial(asset::CAssetManager* Assets = nullptr) :
            m_Assets(Assets == nullptr ? CEffect::s_DefaultManager : *Assets),
            mp_Texture(nullptr), mp_Effect(nullptr) {}

        CMaterial(gfxcore::CTexture& Texture,
                  gfx::CEffect& Effect,
                  asset::CAssetManager* Assets = nullptr) :
            m_Assets(Assets == nullptr ? CEffect::s_DefaultManager : *Assets),
            mp_Texture(&Texture), mp_Effect(&Effect) {}

        ~CMaterial()
        {
            if(!m_given)
            {
                delete mp_Effect;
                m_Assets.Delete(mp_Texture);
            }
        }

        bool LoadFromFile(const string_t& filename);
        bool LoadTextureFromFile(const string_t& filename);
        bool LoadEffect(EffectType& Type);

        bool Attach(CEffect& E, gfxcore::CTexture& T)
        {
            mp_Texture = &T;
            mp_Effect  = &E;
            return (m_given = true);
        }

        bool Enable() const
        {
            bool ret = true;
            if(mp_Effect)   ret = mp_Effect->Enable();
            if(mp_Texture)  ret = ret && mp_Texture->Bind();

            return ret;
        }

        bool EnableEffect() const;
        bool EnableTexture() const;

        bool Disable() const { if(mp_Effect) mp_Effect->Disable(); if(mp_Texture) mp_Texture->Unbind(); return true; }
        bool DisableEffect() const;
        bool DisableTexture() const;

        CEffect* GetEffect()                    { return mp_Effect; }
        gfxcore::CTexture* GetTexture() const   { return mp_Texture; }

        friend class ZEN_API gfxcore::CRenderer;

    private:
        asset::CAssetManager& m_Assets;
        gfxcore::CTexture* mp_Texture;
        gfx::CEffect* mp_Effect;

        bool m_given;
    };

    typedef CMaterial material_t;
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__MATERIAL_HPP

/** @} **/
