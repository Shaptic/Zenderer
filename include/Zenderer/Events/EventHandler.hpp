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

#include "Zenderer/CoreGraphics/OpenGL.hpp"

#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Event.hpp"

/// Forward declaration for callback functions.
struct GLFWwindow;

namespace zen
{
namespace evt
{
    class ZEN_API zEventHandler
    {
    public:
        ~zEventHandler();

        /// Registers all pending events into the interal stack.
        static bool PollEvents();

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
        bool PopEvent(event_t& Evt);

        /// Retrieves the singleton instance of the event handler.
        static zEventHandler& GetInstance();

        /// OS-callback for printable characters.
        static void CharacterCallback(GLFWwindow*, unsigned int c);

        /// OS-callback for any keyboard events.
        static void KeyboardCallback(GLFWwindow*, int key,    int scancode,
                                                  int action, int mods);

        /// OS-callback for mouse movements.
        static void MouseMotionCallback(GLFWwindow*, double x, double y);

        /// OS-callback for mouse-click events.
        static void MouseCallback(GLFWwindow*, int button, int action, int mods);

    private:
        zEventHandler() { s_Active.Reset(); }
        zEventHandler(const zEventHandler&);
        zEventHandler& operator=(const zEventHandler&);

        static std::stack<event_t> s_evtList;
        static event_t s_Active;
    };
}
}

#endif // ZENDERER__EVENTS__EVENT_HANDLER_HPP

/**
 * @class zen::evt::zEventHandler
 * @details
 *  This singleton class instance handles system-wide event occurences during
 *  the application's run-time. It does not *do* anything with these events,
 *  merely providing the user with an interface to poll them and handle them
 *  as they see fit.
 *
 * @todo   Window events / callback.
 *
 * @example Events
 *  This is a very simple event loop that would output every printable
 *  character that a user hits during the duration of the program.
 *
 *  @code
 *  zen::evt::zEventHandler& Events = zen::evt::zEventHandler::GetInstance();
 *  zen::evt::event_t Evt;
 *
 *  while(Window.IsOpen())
 *  {
 *      Events.PollEvents();    // or zen::evt::zEventHandler::PollEvents();
 *      while(Events.PopEvent(Evt))
 *      {
 *          if(Evt.type == zen::evt::EventType::PRINTABLE_KEY)
 *          {
 *              std::cout << "User pressed " << Evt.key.key.symbol << ".\n";
 *          }
 *      }
 *  }
 *  @endcode
 **/

/** @} **/
