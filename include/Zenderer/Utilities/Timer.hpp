/**
 * @file
 *  Zenderer/Utilities/Timer.hpp - A native timer class for controlling engine
 *  frame rates and other high-precision timing operations.
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
 * @addtogroup Utilities
 * @{
 **/

#ifndef ZENDERER__UTILITIES__TIMER_HPP
#define ZENDERER__UTILITIES__TIMER_HPP

// Un/comment this line to show/hide delay info every frame (ZEN_DEBUG_BUILD only).
//#define ZEN_SHOW_DELAY

#include <thread>
#include <chrono>

#include "Zenderer/Core/Types.hpp"
#include "Log.hpp"

namespace zen
{
namespace util
{
    typedef uint32_t time_t;

    /// A high-precision timer class for controlling frame rates.
    class ZEN_API zTimer
    {
    public:
        /// Shortcut for the type of clock we are using
        typedef std::chrono::high_resolution_clock clock_t;

        /// Shortcut for time point structure.
        typedef clock_t::time_point timepoint_t;

        /// Shortcut for the precision we want (do not change if using delays).
        typedef std::chrono::milliseconds precision_t;

        /// Constructs a timer with a custom frame rate.
        zTimer(const uint16_t frames = 60);
        virtual ~zTimer();

        virtual time_t Start();     ///< Marks start time and returns it.
        virtual time_t Finish();    ///< Marks finish time and returns it.
        virtual time_t Elapsed();   ///< Returns finish time - start time.

        /// Sleeps the current thread (defaults to milliseconds).
        void Sleep(const time_t ticks);

        /**
         * Delays the current thread appropriately to maintain frame-rate.
         *  It is not necessary to call Finish() prior to this method, as
         *  it will automatically use the immediate `timepoint` that it
         *  was called at.
         *
         *  A solid game loop that utilizes the timer to regulate frame-rate
         *  would function something like this:
         *
         *  @code
         *  // Cap at 24 frames per second.
         *  zTimer m_Timer(24);
         *
         *  while(m_Window.IsOpen())
         *  {
         *      m_Timer.Start();
         *      // ...
         *      // Handle events, game logic, rendering.
         *      // ...
         *
         *      // Final update to draw everything.
         *      m_Window.Update();
         *
         *      // Regulate frame-rate.
         *      m_Timer.Delay();
         *  }
         *  @endcode
         *
         * @return  The amount of milliseconds the thread slept (if any).
         **/
        virtual time_t Delay();

        /// Returns the current clock time.
        virtual time_t GetTime() const;

        /// Sets a custom frame-rate, overriding the constructor.
        void SetFrameRate(const uint16_t fps);

    private:
        timepoint_t m_start, m_end;
        uint16_t m_fps;
        real_t m_delta;
    };
}
}

#endif // ZENDERER__UTILITIES__TIMER_HPP

/**
 * @class zen::util::zTimer
 * @details
 *  This class provides fairly generic timing functionality,
 *  with the ability to start, stop, measure, and delay for
 *  various periods of time.
 *  Most methods are virtual, allowing for a user to inherit
 *  and implement their own custom timer, but still be able to
 *  pass it around to various parts of the engine when necessary.
 **/

/** @} **/
