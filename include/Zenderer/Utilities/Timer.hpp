#ifndef ZENDERER__UTILITIES__TIMER_HPP
#define ZENDERER__UTILITIES__TIMER_HPP

#include <thread>
#include <chrono>

#include "Zenderer/Core/Types.hpp"

namespace zen
{
namespace util
{
    using time_t = uint32_t;
 
    /**
     * A high-precision timer class for controlling frame rates.
     *  This class provides fairly generic timing functionality,
     *  with the ability to start, stop, measure, and delay for
     *  various periods of time. 
     *  Most methods are virtual, allowing for a user to inherit
     *  and implement their own custom timer, but still be able to
     *  pass it around to various parts of the engine when necessary.
     **/
    class CTimer
    {
    public:
        /// Shortcut for the type of clock we are using
        using clock_t = std::chrono::high_resolution_clock;
        
        /// Shortcut for time point structure.
        using timepoint_t = clock_t::time_point;
        
        /// Shortcut for the precision we want.
        using precision_t = std::chrono::milliseconds;
    
        /// Constructs a timer with a custom frame rate.
        CTimer(const uint16_t frames = 60);
        virtual ~CTimer();
        
        virtual time_t Start();     ///< Marks start time and returns it.
        virtual time_t Finish();    ///< Marks finish time and returns it.
        virtual time_t Elapsed();   ///< Returns finish time - start time.
        
        /// Sleeps the current thread (defaults to milliseconds). 
        template<typename T = precision_t>
        virtual void Sleep(const time_t ticks);
        
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
         *  while(!m_quit)
         *  {
         *      m_Timer.Start();
         *      // ...
         *      // Handle events, game logic, renderering.
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
    };
}
}

#endif // ZENDERER__UTILITIES__TIMER_HPP

/** @} **/
