/**
 * @file
 *  Zenderer/Audio/AudioManager.hpp - Convenience functions for the audio
 *  library.
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
 *  This contains the audio subset of the @a Zenderer engine. Not used in the
 *  engine itself, this subsystem provides the ability to play sound effects,
 *  music, and other audio files both in WAV and OGG formats. It's recommended
 *  that you only use OGG for longer files (greater than 96kB), because they
 *  have the possibility to fail otherwise. This is because the OGG streaming
 *  facility cycles through 3 32kB buffers, so they should all at least be
 *  able to be filled once.
 * @{
 **/

#ifndef ZENDERER__AUDIO__AUDIO_MANAGER_HPP
#define ZENDERER__AUDIO__AUDIO_MANAGER_HPP

#include "vorbis/vorbisfile.h"
#include "AL/al.h"
#include "AL/alut.h"

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Utilities/Assert.hpp"
#include "Zenderer/Utilities/Log.hpp"

#ifdef AL
  #undef AL
#endif // AL

/// Automatic OpenAL call checking in debug builds.
#ifdef ZEN_DEBUG_BUILD
  #define AL(f) f; zen::sfx::CAudioManager::alCheck(#f, __LINE__, __FILE__);
#else
  #define AL(f) f;
#endif // _DEBUG

namespace zen
{
/// Encompasses the audio API in @a Zenderer.
namespace sfx
{
    /// A collection of static helper functions for the audio API.
    class ZEN_API CAudioManager
    {
    public:
        /// Initializes OpenAL.
        static bool Init();

        /**
         * Finds an available audio source to use for a buffer.
         *  If all of the sources are completely used up, this function
         *  will return `-1`, and someone needs to call `FreeSource()` on
         *  an audio file to free up some sources to use.
         *
         * @return  An index to the next available OpenAL source buffer.
         **/
        static int GetAvailableSourceIndex();

        /// Actually returns the OpenAL source handle.
        static int GetAvailableSource(const uint16_t index);

        /// Creates an OpenAL source handle and returns its handle.
        static ALuint CreateSource();

        /// Frees an OpenAL source.
        static bool FreeSource(ALuint index);

        /**
         * Checks the validity of an OpenAL call.
         *  Mostly used internally for debugging, this will check valid
         *  execution of an OpenAL call. It should be used in congruence with
         *  the macro defined at the top of this file (AL(f)).
         *  This function won't really work well without the macro, as too
         *  many things have to be passed as parameters for accuracy; it'd be
         *  unwieldy to pass the string of the OpenAL function call by hand
         *  every time.

         * @param   expr    OpenAL call expression
         * @param   line    Line number of function call
         * @param   file    File name of function call
         *
         * @return  `true`     if there was no error, and
         *           `false`    otherwise, though the assertion will exit.
         **/
        static bool alCheck(const char* expr,
                            const uint32_t line,
                            const char* file);

        /// Logs and outputs a readable error from an OGG error code.
        static void OGGError(const int error_code);

        /// Is OpenAL initialized?
        static bool IsInit();

    private:
        static const uint16_t AVAILABLE_BUFFERS = 256;
        static uint32_t s_available[256];
        static bool s_init;
    };
}   // namespace sfx
}   // namespace zen

#endif // ZENDERER__AUDIO__AUDIO_MANAGER_HPP

/**
 * @class zen::sfx::CAudioManager
 * @details
 *  This class provides a variety of `static` convenience functions for
 *  the various audio asset instance classes to use in order to find
 *  available OpenAL sources.
 * 
 *  OpenAL is limited to 256 buffers, and despite the fact that they likely
 *  will never all be in use simultaneously, there is still no sense in being
 *  greedy. This class should be the one and only way that external audio
 *  classes get an available OpenAL source.
 *
 *  The architecture for controlling sources is as follows:
 *
 *  There exists a static array of 256 potential sources, zeroed out. As
 *  audio is played, the array fills up with source data, starting from 
 *  `[0]`. So, the first call to `GetAvailableSource()` would return `0`
 *  and after a call to `alGenSources` it would be filled with some arbitrary
 *  data. So thus
 *
 *      CAudioManager::s_sources[0] = 12345
 *
 *  Then, a subsequent call to `GetAvailableSource()` would return `1` and
 *  the process repeats. Now, let's say the first source is done playing an
 *  `.ogg` file and CMusic2D::FreeSource() is called. Now the first available
 *  source is `s_sources[0]`, but due to the nature of the source-finding
 *  algorithm, `GetAvailableSource()` will return `2`, because it first
 *  searches the index immediately following the last-used source before
 *  starting from the beginning. Hence (assuming `2` is then used):
 *
 *      CAudioManager::s_sources[0] = 0
 *      CAudioManager::s_sources[1] = 223344
 *      CAudioManager::s_sources[2] = 696969
 *
 **/

/** @} **/
