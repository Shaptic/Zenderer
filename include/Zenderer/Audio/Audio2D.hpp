/**
 * @file
 *  Zenderer/Audio/Audio2D.hpp - Generic base class for audio files.
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
 * @addtogroup Audio
 * @{
 **/

#ifndef ZENDERER__AUDIO__AUDIO_2D_HPP
#define ZENDERER__AUDIO__AUDIO_2D_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Core/Allocator.hpp"

#include "Zenderer/Assets/Asset.hpp"
#include "AudioManager.hpp"

namespace zen
{
namespace sfx
{
    /// An abstract base class for audio objects.
    class ZEN_API CAudio2D : public asset::CAsset
    {
    public:
        virtual ~CAudio2D();
        
        /// Loads an audio file from disk.
        virtual bool LoadFromFile(const string_t& filename) = 0;

        /// Called once.
        virtual void Play() = 0;

        /// Called every frame after Play()
        virtual void Update() = 0;

        /// Returns true if the stream is ready to Update()
        inline bool Ready() const;

        /// Unloads the OpenAL source handle.
        void UnloadSource();

        ALenum              GetAudioState() const;
        const void* const   GetData()       const;

    protected:
        CAudio2D();
        
        bool Destroy() { this->UnloadSource(); return true; }

        struct ALData
        {
            ALuint* buffers;
            size_t  buffer_count;

            ALuint   source;
            real_t  volume;
        } m_AL; // OpenAL data

        uint32_t    m_error_code;   // OpenAL error code
        bool        m_loop;         // Loop after finishing?
    };

}   // namespace sfx
}   // namespace zen

#endif // ZENDERER__AUDIO__AUDIO_2D_HPP

/** @} **/

/**
 * @class zen::sfx::CAudio2D
 * @description
 *  A managed audio asset object that acts as a unified base class
 *  to support playing a variety of audio formats.
 **/
