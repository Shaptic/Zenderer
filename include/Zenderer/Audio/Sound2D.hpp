/**
 * @file
 *  Zenderer/Audio/Sound2D.hpp - Short `.wav` file sound effects used
 *  in-game.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.0
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

#ifndef ZENDERER__AUDIO__SOUND_2D_HPP
#define ZENDERER__AUDIO__SOUND_2D_HPP

#include "Audio2D.hpp"

namespace zen
{
namespace sfx
{
    /// Sound effects (`.wav` files)
    class ZEN_API zSound2D : public zAudio2D
    {
    public:
        virtual ~zSound2D();

        bool LoadFromFile(const string_t& filename);
        bool LoadFromExisting(const zAsset* const pCopy){ return true; }

        /// Plays the sound effect no matter what (if loaded).
        void Play();

        /// This only checks for looping.
        inline void Update()
        {
            if(this->IsLoaded() && this->GetAudioState() != AL_PLAYING)
            {
                this->Play();
            }
        }

        /// Always returns `true` if the file is loaded.
        inline bool Ready() const { return true; }

        friend class ZEN_API asset::zAssetManager;

    private:
        zSound2D(const void* const owner = nullptr,
                 const void* const settings = nullptr);
        zSound2D(const zSound2D&) = delete;
    };
}   // namespace sfx
}   // namespace zen

#endif // ZENDERER__AUDIO__SOUND_2D_HPP

/**
 * @class zen::sfx::zSound2D
 * @details
 *  This managed audio asset is designed primarily for short,
 *  repetitive sound effects stored in an uncompressed `.wav`
 *  file format.
 *  It's not recommended to store or load large `.wav` files
 *  in your application, because of the uneccessary consumption
 *  of memory and disk space. A better alternative for long audio
 *  is the OGG-Vorbis (`.ogg`) format, loadable through the
 *  zen::sfx::zMusic2D class.
 **/

/** @} **/
