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
#include "Zenderer/Assets/Asset.hpp"
#include "AudioManager.hpp"

namespace zen
{
namespace asset { class zAssetManager; }
namespace sfx
{
    /// An abstract base class for audio objects.
    class ZEN_API zAudio2D : public asset::zAsset
    {
    public:
        virtual ~zAudio2D();

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

        friend class asset::zAssetManager;

    protected:
        zAudio2D(const void* const owner = nullptr,
                 const void* const settings = nullptr);
        zAudio2D(const zAudio2D&) = delete;

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

/**
 * @class zen::sfx::zAudio2D
 *
 * @details
 *  A managed audio asset object that acts as a unified base class
 *  to support playing a variety of audio formats.
 **/

/** @} **/
