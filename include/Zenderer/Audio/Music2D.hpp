 /**
 * @file
 *  Zenderer/Audio/Music2D.hpp - Long `.ogg` files used for music,
 *  dialogue, etc.
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

#ifndef ZENDERER__AUDIO__MUSIC_2D_HPP
#define ZENDERER__AUDIO__MUSIC_2D_HPP

#include "Audio2D.hpp"

namespace zen
{
namespace sfx
{
    /// Music files (`.ogg` files)
    class ZEN_API CMusic2D : public CAudio2D
    {
    public:
        virtual ~CMusic2D();

        /// Loads an `.ogg` file from disk.
        bool LoadFromFile(const string_t& filename);

        /**
         * Plays the file (if loaded).
         *  This should only be called a single time, and
         *  then subsequent calls should be to Update().
         **/
        void Play();

        /// Updates the internal file stream for continuous play.
        void Update();

        /// Is the stream ready to call Update()?
        bool Ready() const;

    private:
        CMusic2D();

        bool FillChunk(const uint32_t buffer);

        // Read in 32-byte chunks.
        static const uint16_t READ_SIZE = (1 << 15);

        // How many buffers for streaming?
        static const uint8_t BUFFER_COUNT = 3;

        // Which AL buffer is in use?
        uint8_t     m_active;

        // ogg data
        OggVorbis_File  m_ogg;          // Vorbis file handle
        char*       m_ogg_buffer;       // Raw OGG data buffer
        int         m_format, m_freq;   // Audio format / frequency
    };
}   // namespace sfx
}   // namespace zen

#endif // ZENDERER__AUDIO__MUSIC_2D_HPP

/**
 * @class zen::sfx::CMusic2D
 * @details
 *  A managed asset that will stream large files in the Ogg-Vorbis (`.ogg`)
 *  file format. This format is not recommended for files smaller than 96kB,
 *  due to the streaming nature that stores 96kB of data at a time. A better
 *  alternative would be to use the zen::sfx::CSound2D class in combination
 *  with an uncompressed `.wav` file.
 **/

/** @} **/
