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
            gfx::zQuad* pQ = new gfx::zQuad(m_Assets, 0, 0);
            bool ret = m_Material.LoadTextureFromFile(filename);
            if(ret)
            {
                pQ->Resize(m_Material.GetTexture().GetWidth(),
                           m_Material.GetTexture().GetHeight());
                pQ->AttachMaterial(m_Material);
                pQ->Create();
                mp_allPrims.push_back(pQ);

                m_texc = 1.0 / (pQ->GetW() / m_Size.x);

                gfx::zEffect& e = m_Material.GetEffect();
                e.Enable();
                e.SetParameter("tc_offset", &m_texc, 1);
                e.Disable();
            }
            else
            {
                delete pQ;
            }

            return ret;
        }

        bool LoadFromFile(const string_t& filename)  { ZEN_ASSERTM(false, "no."); return false; }
        bool AddPrimitive(const gfx::zPolygon& Prim) { ZEN_ASSERTM(false, "no."); return false; }
        //bool Draw(bool is_bound = false);
        bool Update()
        {
            if(++m_now < m_rate) return false;

            real_t start[] = { m_current * m_texc, m_current };

            gfx::zEffect& e = const_cast<gfx::zMaterial&>(
                mp_allPrims.front()->GetMaterial()).GetEffect();
            e.Enable();
            e.SetParameter("tc_start", start, 2);
            e.Disable();

            return true;
        }

        void SetKeyframeCount(const uint16_t frames) { m_framecount = frames; }
        void SetKeyframeSize(const math::vectoru16_t& Size) { m_Size = Size; }
        void SetKeyframeRate(const uint16_t rate) { m_rate = rate; }

    private:
        gfx::zMaterial m_Material;
        math::vectoru16_t m_Size;
        real_t m_texc;
        uint16_t m_framecount, m_now, m_rate;
        uint16_t m_current;
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__ANIMATION_HPP

/** @} **/
