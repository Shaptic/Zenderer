/**
 * @file
 *  Zenderer/Events/EventHandler.hpp - Defines an event-handling singleton
 *  object that stores and processes system events from a stack.
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
 * @addtogroup Events
 * @{
 **/

#ifndef ZENDERER__EVENTS__EVENT_HANDLER_HPP
#define ZENDERER__EVENTS__EVENT_HANDLER_HPP

#include <stack>

#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Event.hpp"

/// Forward declaration for callback functions.
struct GLFWwindow;

namespace zen
{
namespace evt
{
    /// @todo   Window events / callback.
    class ZEN_API CEventHandler
    {
    public:
        ~CEventHandler() { while(!s_evtList.empty()) s_evtList.pop(); }
        
        /**
         * Removes the latest event from the stack and stores it in the parameter.
         *  This will pop an event from the internal event stack and store
         *  it in the parameter that you passed, tossing out any existing event
         *  data that may have been stored in it.
         *
         * @param   Evt     Event structure to store latest even in
         *
         * @return  `true`  if an event was loaded,
         *          `false` if there are none remaining.
         **/
        bool PopEvent(event_t& Evt)
        {
            Evt.Reset();
            if(s_evtList.empty()) return false;
            Evt = s_evtList.top();
            s_evtList.pop();
            return true;
        };
        
        /// Retrieves the singleton instance of the event handler.
        static CEventHandler& GetInstance() 
        {
            static CEventHandler g_Events;
            return g_Events;
        }
        
        static void KeyboardCallback(
            GLFWwindow*, int key, int scancode,
            int action, int mods)
        {
            s_Active.key.keycode= static_cast<Key>(key);
            s_Active.key.key    = static_cast<char>(scancode);
            
            if(action == GLFW_PRESS)        s_Active.type = EventType::KEY_DOWN;
            else if(action == GLFW_RELEASE) s_Active.type = EventType::KEY_UP;
            else if(action == GLFW_REPEAT)  s_Active.type = EventType::KEY_HOLD;

            s_evtList.push(s_Active);
            s_Active.Reset();
        }
        
        static void MouseMotionCallback(
            GLFWwindow*, double x, double y)
        {
            s_Active.mouse.position = math::vector_t(x, y);
            s_Active.mouse.button   = MouseButton::UNKNOWN;
            s_Active.mouse.down     = false;
            s_Active.type           = EventType::MOUSE_MOTION;
            
            s_evtList.push(s_Active);
            s_Active.Reset();
        }
        
        static void MouseCallback(
            GLFWwindow*, int button, int action, int mods)
        {
            //GetMousePosition(s_Active.mouse.position);
            s_Active.mouse.button   = static_cast<MouseButton>(button);
            s_Active.mouse.down     = (action == GLFW_PRESS);
            s_Active.mouse.mods     = mods;
            
            if(action == GLFW_PRESS)
                s_Active.type = EventType::MOUSE_DOWN;
            else
                s_Active.type = EventType::MOUSE_UP;
            
            s_evtList.push(s_Active);
            s_Active.Reset();
        }
        
    private:
        CEventHandler() {}
        CEventHandler(const CEventHandler&);
        CEventHandler& operator=(const CEventHandler&);
       
        static std::stack<event_t> s_evtList;
        static event_t s_Active;
    };
}
}

#endif // ZENDERER__EVENTS__EVENT_HANDLER_HPP

/** @} **/
