#ifndef ZENDERER__UTILITIES__TIMER_HPP
#define ZENDERER__UTILITIES__TIMER_HPP

#include <thread>
#include <chrono>

#include "Zenderer/Core/Types.hpp"

namespace zen
{
namespace util
{
    typedef std::chrono::high_resolution_clock clock_t;
    typedef clock_t::time_point timepoint_t;
    typedef std::chrono::milliseconds precision_t;
    typedef uint32_t time_t;
    
    using std::chrono::duration_cast;
 
    // A high-precision timer class for controlling frame rates.
    class CTimer
    {
    public:
        CTimer(const uint16_t frames) : m_fps(frames) {}
        virtual ~CTimer(){}
        
        virtual time_t Start()
        {
            m_start = clock_t::now();
            return duration_cast<precision_t>(m_start.time_since_epoch()).count();
        }
        
        virtual time_t Finish()
        {
            m_end = clock_t::now();
            return duration_cast<precision_t>(m_start.time_since_epoch()).count();
        }
        
        virtual time_t Elapsed()
        {
            return duration_cast<precision_t>(m_end - m_start).count();
        }
        
        virtual time_t Delay()
        {
            timepoint_t::duration pause = clock_t::now() - m_start;
            time_t ticks = duration_cast<precision_t>(pause).count();
            
            if(ticks < 1000.0 / m_fps)
                std::this_thread::sleep_for(pause);
                
            return ticks;
        }
        
    private:
        timepoint_t m_start, m_end;
        uint16_t m_fps;
    };
}
}

#endif // ZENDERER__UTILITIES__TIMER_HPP

/** @} **/
