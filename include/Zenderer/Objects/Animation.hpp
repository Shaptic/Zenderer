/**
 * @file
 *  Zenderer/Objects/Animation.hpp - A wrapper for easily creating animated entities.
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
 * @addtogroup Objects
 * @{
 **/

#ifndef ZENDERER__OBJECTS__ANIMATION_HPP
#define ZENDERER__OBJECTS__ANIMATION_HPP

#include "Entity.hpp"

namespace zen
{
namespace obj
{
    class ZEN_API zAnimation : public zEntity
    {
    public:
        zAnimation(asset::zAssetManager& Assets) :
            zEntity(Assets), m_Material(Assets), m_current(0)
        {
            m_Material.LoadEffect(gfx::EffectType::SPRITESHEET);
        }

        ~zAnimation()
        {
        }

        bool LoadFromTexture(const string_t& filename)
        {
            return m_Material.LoadTextureFromFile(filename);
        }

        bool LoadFromFile(const string_t& filename)  { ZEN_ASSERTM(false, "no."); }
        bool AddPrimitive(const gfx::zPolygon& Prim) { ZEN_ASSERTM(false, "no."); }
        bool Draw(bool is_bound = false);
        bool Update()
        {
            if(++m_now < m_rate) return false;

            real_t one = 1.0 / m_Material.GetTexture().GetWidth();
            float offset[2] = { one * (m_current - 1), 0.0 };

            gfx::zEffect& e = m_Material.GetEffect();
            e.Enable();
            e.SetParameter("offset", offset, 2);
            e.Disable();
        }

        void SetKeyframeCount(const uint16_t frames);
        void SetKeyframeSize(const math::vectoru16_t& Size);
        void SetKeyframeRate(const uint16_t rate);

    private:
        gfx::zMaterial m_Material;
        uint16_t m_framecount, m_now, m_rate;
        uint16_t m_current;
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__ANIMATION_HPP

/** @} **/
