/**
 * @file
 *  Zenderer/Events/Mouse.hpp - Defines mouse-related event data structures
 *  and convenience functionality.
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

#ifndef ZENDERER__EVENTS__MOUSE_HPP
#define ZENDERER__EVENTS__MOUSE_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Vector.hpp"

namespace zen
{
namespace evt
{
    /// Mouse button identifiers.
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

    struct mouse_t
    {
        mouse_t() :
            position(0, 0),
            button(MouseButton::UNKNOWN),
            down(false) {}

        math::vector_t position;
        MouseButton button;
        bool down;
        int mods;
    };
}
}

#endif // ZENDERER__EVENTS__MOUSE_HPP

/** @} **/
