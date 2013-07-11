/**
 * @file
 *  Zenderer/Events/Keyboard.hpp - Defines keyboard-related event data
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

#ifndef ZENDERER__EVENTS__KEYBOARD_HPP
#define ZENDERER__EVENTS__KEYBOARD_HPP

namespace zen
{
namespace evt
{
    /// Keyboard key enumerations.
    enum class Key
    {
        UNKNOWN     = -1,
        SPACE       = 32,
        APOSTROPHE  = 39,
        SEMICOLON   = 59,
        EQUAL       = 61,
        COMMA       = 44,
        MINUS, PERIOD, SLASH,
        
        // Top row of numbers.
        ZERO,   ONE,    TWO,    THREE, FOUR,
        FIVE,   SIX,    SEVEN,  EIGHT, NINE,
        
        A = 65,
        B, C, D, E, F, G, H, I,
        J, K, L, M, N, O, P, Q,
        R, S, T, U, V, W, X, Y, Z,
        
        LEFT_BRACKET, BACKSLASH, RIGHT_BRACKET, GRAVE,
        
        ESCAPE = 256,
        ENTER,      TAB,        BACKSPACE,
        INSERT,     DELETE,
        RIGHT,      LEFT,       DOWN, UP,        
        PAGE_UP,    PAGE_DOWN,  HOME, END,
        
        CAPS_LOCK = 280,
        SCOLL_LOCK, NUM_LOCK,
        PRINT_SCR,  PAUSE,
        
        F1 = 290,
        F2, F3, F4,  F5,  F6, F7
        F8, F9, F10, F11, F12,
        
        LEFT_SHIFT = 340,
        LEFT_CONTROL,   LEFT_ALT,       LEFT_SUPER,
        RIGHT_SHIFT,    RIGHT_CONTROL,  RIGHT_ALT,
        RIGHT_SUPER,
        
        MENU = 348
    };

    /// Keyboard event structure.
    struct key_t
    {
        key_t() : 
            mods(0),
            keycode(Key::UNKNOWN),
            key('\0') {}

        int     mods;
        Key     keycode;
        char    key;
    };
}
}

#endif // ZENDERER__EVENTS__KEYBOARD_HPP

/** @} **/
