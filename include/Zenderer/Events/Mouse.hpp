/**
 * @file
 *  Zenderer/Events/Mouse.hpp - Defines mouse-related event data structures
 *  and convenience functionality.
 *
 * @author      George (@_Shaptic)
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

#ifndef ZENDERER__EVENTS__MOUSE_HPP
#define ZENDERER__EVENTS__MOUSE_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Math.hpp"
#include "Zenderer/CoreGraphics/OpenGL.hpp"

namespace zen
{
namespace evt
{
    /**
     * @enum MouseButton
     * Mouse button identifiers.
     **/
    enum class ZEN_API MouseButton
    {
        UNKNOWN = -1,
        LEFT    = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT   = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE  = GLFW_MOUSE_BUTTON_MIDDLE,
        BUTTON4 = 4,
        BUTTON5,
        BUTTON6,
        BUTTON7,
        BUTTON8
    };

    /// Mouse event structure.
    struct ZEN_API mouse_t
    {
        mouse_t();
        math::vector_t position;    /// Event location
        MouseButton button;         /// Event's mouse button
        bool down;                  /// Is the mouse button down?
        int mods;                   /// Any key modifiers held during event.
    };

    /// Retrieves the current mouse position on the active context.
    ZEN_API math::vector_t GetMousePosition();

    /// Returns `true` if the mouse button is pressed.
    ZEN_API bool GetMouseState(const evt::MouseButton& Btn);
}
}

#endif // ZENDERER__EVENTS__MOUSE_HPP

/**
 * @struct zen::evt::mouse_t
 * @details
 *  Contains information about the state of the mouse for a specific
 *  event instance. You can retrieve position data, button state,
 *  and keyboard modifiers.
 *
 * @fn zen::evt::GetMousePosition()
 * @details
 *  This function acts on the current graphical context, so whichever
 *  zen::gfx::zWindow is currently in an active state at that time.
 *  It will retrieve relative mouse positioning, so there is no need to
 *  regard for absolute window positioning within the operating system.
 *
 * @fn zen::evt::GetMouseState(const evt::MouseButton& Btn)
 * @details
 *  Retrieves the state of the mouse at that precise moment, indicating
 *  whether or not a certain button is being pressed at that time.
 **/

/** @} **/
