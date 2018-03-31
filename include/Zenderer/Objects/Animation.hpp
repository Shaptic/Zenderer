/**
 * @file
 *  Zenderer/Objects/Animation.hpp - A wrapper for easily creating animated
 *  entities.
 *
 * @author      george (halcyon)
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
    /// An animated in-game object.
    class ZEN_API zAnimation : public zEntity
    {
    public:
        zAnimation(asset::zAssetManager& Assets);
        ~zAnimation();

        /**
         * Loads an animation from a texture file.
         *  All settings should be set prior to this loading sequence, because
         *  it relies on the sprite size setting to properly generate the
         *  animation.
         *
         *  If no frame size has been given, the entire texture is assumed to
         *  be the first and only sprite in the sprite sheet.
         *
         * @param   filename    The path to the sprite sheet to load
         *
         * @pre     SetKeyframeSize() must have been called.
         *
         * @return  `true`  if the texture and animation shader loaded successfully,
         *          `false` otherwise.
         **/
        bool LoadFromTexture(const string_t& filename);
        bool LoadFromFile(const string_t& filename)  { ZEN_ASSERTM(false, "no."); return false; }
        bool AddPrimitive(const gfx::zPolygon& Prim) { ZEN_ASSERTM(false, "no."); return false; }

        /**
         * Plays an animation for a certain number of loops.
         *  This is not necessary to call if you wish to loop forever. By default,
         *  the animation will play indefinitely.
         *
         * @param   loops   Number of loops to execute, 0 means infinite.
         *
         * @see OnFinish()
         **/
        void PlayAnimation(const uint32_t loops = 0);

        /**
         * Freeze the animation at a certain frame.
         *  This will stop animating the object, and will stop on the current
         *  frame by default. If you provide a parameter >= 0, the object will
         *  use that keyframe as its static sprite.
         *  This *will not* reset any internal ticker, so resuming the animation
         *  will likely immediately switch sprites.
         *
         * @param   frame   Frame to stop on, -1 means stop on current.
         **/
        void StopAnimation(const int16_t frame = -1);

        /**
         * Update the current animation frame if the set frame time has passed.
         *  This method relies on a per-frame rate being set for the object.
         *  This method will call the given callback function if a maximum loop count
         *  was specified, and has been reached.
         *
         *  This method should be called every frame to ensure that the ticker is
         *  updated accordingly.
         *
         * @bool    `true`  if a frame was changed, `false` otherwise.
         *
         * @see     SetKeyframeRate()
         * @see     PlayAnimation()
         * @see     OnFinish()
         **/
        bool Update();

        /**
         * Executes a callback function when all animation loops are complete.
         *  This does not apply in infinitely looping animations, and is
         *  executed right after the given number of maximum loops has been
         *  reached.
         *  Currently the callback must be move-constructable.
         *
         * @param   callback    The callback function you wish to execute.
         *
         * @see     PlayAnimation()
         **/
        void OnFinish(std::function<void()>&& callback);

        /// Configures the number of sprites in a given spritesheet.
        void SetKeyframeCount(const uint16_t frames);

        /// Configures the dimensions of a single frame.
        void SetKeyframeSize(const uint16_t w, const uint16_t h);
        void SetKeyframeSize(const math::vectoru16_t& Size);    /// @overload

        /// Configures the time, in frames, before showing the next keyframe.
        void SetKeyframeRate(const uint16_t rate);

        /// Checks to see if animation is currently enabled.
        inline bool Playing() const { return !m_stop; }

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

/**
 * @class   zen::obj::zAnimation
 * @details
 *  This is a high-level interface designed to facilitate efficient animation
 *  routines. It has several limitations, such as its reliance on constant calls
 *  to zAnimation::Update() in your main loop, and the requirement of uniform
 *  size of keyframes within a sprite sheet.
 **/

/** @} **/
