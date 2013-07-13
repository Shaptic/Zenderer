/**
 * @file
 *  Zenderer/Events/Event.hpp - Defines keyboard-related event data
 *  structures and convenience functionality.
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
 * @addtogroup Events
 * @{
 **/

#ifndef ZENDERER__EVENTS__EVENT_HPP
#define ZENDERER__EVENTS__EVENT_HPP

#include <stack>

#include "Keyboard.hpp"
#include "Mouse.hpp"

/// Forward declaration for callback functions.
struct GLFWWindow;

namespace zen
{
namespace evt
{
    /// Enumeration of the various event types.
    enum class EventType
    {
        NONE = -1,
        KEY_DOWN,
        KEY_UP,
        KEY_HOLD,
        PRINTABLE_KEY,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_MOTION,
        WINDOW_MINIMIZE,
        WINDOW_MAXIMIZE,
        WINDOW_CLOSE
    };

    /// The event structure storing information about system events.
    struct event_t
    {
        event_t() : type(EventType::NONE) {}
        event_t(const event_t& evt) : type(evt.type) 
        {
            *this = evt;
        }
        
        /// Copies data from an event to this one.
        event_t& operator=(const event_t& evt)
        {
            type            = evt.type;            
            mouse.position  = evt.mouse.position;
            mouse.button    = evt.mouse.button;
            mouse.mods      = evt.mouse.mods;
            mouse.down      = evt.mouse.down;
            key.mods        = evt.key.mods;
            key.symbol      = evt.key.symbol;
            key.scan        = evt.key.scan;
            key.key         = evt.key.key;
            
            return (*this);
        }
        
        event_t& Reset()
        {
            type            = EventType::NONE;
            mouse.position  = math::vector_t();
            mouse.button    = MouseButton::UNKNOWN;
            mouse.mods      = 0;
            mouse.down      = false;
            key.mods        = 0;
            key.key         = Key::UNKNOWN;
            key.symbol      = '\0';
            key.scan        = -1;

            return (*this);
        }
        
        EventType   type;
        mouse_t     mouse;
        key_t       key;
    };
}
}

#endif // ZENDERER__EVENTS__EVENT_HPP

/** @} **/
