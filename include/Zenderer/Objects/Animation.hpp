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
            zEntity(Assets), m_current(0),
            m_texc(0.0), m_rate(0), m_now(0), m_stop(false)
        {
        }

        ~zAnimation()
        {
        }

        bool LoadFromTexture(const string_t& filename)
        {
            gfx::zMaterial TmpMaterial(m_Assets);
            gfx::zQuad* pQ = new gfx::zQuad(m_Assets, 0, 0);

            bool ret = TmpMaterial.LoadEffect(gfx::EffectType::SPRITESHEET) &&
                       TmpMaterial.LoadTextureFromFile(filename);

            if(ret)
            {
                pQ->Resize(m_Size.x, TmpMaterial.GetTexture().GetHeight());
                pQ->AttachMaterial(TmpMaterial);
                pQ->Create();
                mp_allPrims.push_back(pQ);

                m_texc = 1.0 / (TmpMaterial.GetTexture().GetWidth() / m_Size.x);

                gfx::zEffect& e = const_cast<gfx::zEffect&>(pQ->GetMaterial().GetEffect());
                e.Enable();
                e.SetParameter("tc_offset", &m_texc, 1);
                e.SetParameter("proj", gfxcore::zRenderer::GetProjectionMatrix());
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

        bool Update()
        {
            if(m_stop || ++m_now < m_rate) return false;
            if(++m_current >= m_framecount) m_current = 0;
            this->SwitchFrame(m_current);
            m_now = 0;
            return true;
        }

        void SetKeyframeCount(const uint16_t frames) { m_framecount = frames; }
        void SetKeyframeSize(const math::vectoru16_t& Size) { m_Size = Size; }
        void SetKeyframeRate(const uint16_t rate) { m_rate = rate; }

        void StopAnimation(const int16_t frame = -1)
        {
            this->SwitchFrame(frame < 0 ? m_current : frame);
            m_stop = true;
        }

        void StartAnimation(const int16_t frame = -1)
        {
            this->SwitchFrame(frame < 0 ? m_current : frame);
            m_stop = false;
        }

    private:
        void SwitchFrame(const uint16_t frame)
        {
            m_current = frame;
            real_t start[] = { m_current * m_texc, 0.0 };

            gfx::zEffect& e = const_cast<gfx::zMaterial&>(
                mp_allPrims.front()->GetMaterial()).GetEffect();
            e.Enable();
            e.SetParameter("tc_start", start, 2);
            e.Disable();
        }

        math::vectoru16_t m_Size;
        real_t m_texc;
        uint16_t m_framecount, m_now, m_rate;
        uint16_t m_current;
        bool m_stop;
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__ANIMATION_HPP

/** @} **/
