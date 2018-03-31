/**
 * @file
 *  Zenderer/GUI/Button.hpp - A wrapper for easily creating roll-over button
 *  effects.
 *
 * @author      george (halcyon)
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
 * @addtogroup GUI
 * @{
 **/

#ifndef ZENDERER__GUI__BUTTON_HPP
#define ZENDERER__GUI__BUTTON_HPP

#include "Zenderer/Graphics/Scene.hpp"
#include "Font.hpp"

namespace zen
{
namespace gui
{
    class ZEN_API zButton
    {
    public:
        zButton(gfx::zScene& MenuScene);

        ~zButton();

        void Place(const math::vector_t& Pos);
        void Place(const real_t x, const real_t y);

        bool Prepare(const string_t& text);

        bool IsOver(const math::vector_t& Pos);
        bool IsOver(const math::rect_t& Box);

        void SetActive();
        void SetDefault();

        void SetFont(zFont& Font);
        void SetActiveColor(const color4f_t& active);
        void SetNormalColor(const color4f_t& normal);
        void SetBackground(const obj::zEntity& Bg);

    private:
        gfx::zScene&    m_Scene;
        obj::zEntity&   m_Active;
        obj::zEntity&   m_Normal;
        obj::zEntity*   mp_Current;
        gui::zFont*     mp_Font;

        color4f_t m_acolor, m_ncolor;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__BUTTON_HPP

/**
 * @class zen::gui::zButton
 * @details
 *  A wrapper for a menu button that allows for text to be easily
 *  rendered on top of a background, and to easily add effects when
 *  some sort of event occurs, like mouse-over.
 *  This is used internally by zen::gui::zMenu to easily create
 *  dynamic and clean menus, and likely shouldn't really be touched
 *  on its own.
 **/

/** @} **/
