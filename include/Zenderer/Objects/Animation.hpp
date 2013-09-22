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

#include <functional>

#include "Entity.hpp"

namespace zen
{
namespace obj
{
    class ZEN_API zAnimation : public zEntity
    {
    public:
        zAnimation(asset::zAssetManager& Assets);
        ~zAnimation();

        bool LoadFromTexture(const string_t& filename);
        bool LoadFromFile(const string_t& filename)  { ZEN_ASSERTM(false, "no."); return false; }
        bool AddPrimitive(const gfx::zPolygon& Prim) { ZEN_ASSERTM(false, "no."); return false; }

        bool Update();

        void SetKeyframeCount(const uint16_t frames);
        void SetKeyframeSize(const math::vectoru16_t& Size);
        void SetKeyframeSize(const uint16_t w, const uint16_t h);
        void SetKeyframeRate(const uint16_t rate);

        void PlayAnimation(const uint32_t loops = 0);
        void StopAnimation(const int16_t frame  = -1);

        void OnFinish(std::function<void()>&& callback);

    private:
        void SwitchFrame(const uint16_t frame);

        // When animating is done, this is called.
        std::function<void()> m_callback;

        // A single frame's dimensions.
        math::vectoru16_t m_Size;

        // Texture coordinate scalar (normalize(frames * size))
        real_t m_texc;

        // Total number of frames for this animation,
        // and the frame we're currently on.
        uint16_t m_framecount, m_current;

        // The current tick, and the number of ticks before switching frames.
        uint16_t m_now, m_rate;

        // The number of loops we can execute, and the number we've done.
        uint16_t m_maxloops, m_loops;

        // Is animation enabled?
        bool m_stop;
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__ANIMATION_HPP

/** @} **/
