/**
 * @file
 *  Zenderer/GUI/Button.hpp - A wrapper for easily creating roll-over button
 *  effects.
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
 * @addtogroup GUI
 * @{
 **/

#ifndef ZENDERER__GUI__BUTTON_HPP
#define ZENDERER__GUI__BUTTON_HPP

#include "Element.hpp"

namespace zen
{
namespace gui
{
    /// Simulates a hoverable GUI button element.
    class ZEN_API zButton : public zElement
    {
    public:
        using zElement::Place;
        using zElement::IsOver;

        /**
         * Creates a button instance.
         *  The given parameter is a scene object to which all
         *  button states will attach themselves onto.
         *
         * @param   MenuScene   Scene to attach buttons to
         *
         * @pre     The `MenuScene` object must have been initialized.
         * @post    The `MenuScene` object has 2 new entities within it.
         *
         * @see     gfx::zScene::Init()
         **/
        zButton(gfx::zScene& MenuScene, asset::zAssetManager& Assets);
        ~zButton();

        virtual void Place(const real_t x, const real_t y);
        virtual bool IsOver(const math::rect_t& Box);

        /**
         * Creates the button with the given text.
         *  This will use the previously configured parameters
         *  (via the various `Set*()` methods) to render and prepare
         *  a button for live on-screen use within the scene it's
         *  attached to (from the constructor).
         *
         * @return  `true`  if the button was rendered in both states
         *                  successfully,
         *          `false` if there is internal state data missing,
         *                  or if the button couldn't render.
         *
         * @see     SetText()
         * @see     SetActiveColor()
         * @see     SetNormalColor()
         * @see     SetBackground()
         **/
        virtual bool Create(zFont& Font);

        virtual void Focus();
        virtual void Unfocus();

        /// Sets the button text (before `Create()`).
        void SetText(const string_t& text);

        /// Sets the "focused" button state text color.
        void SetFocusColor(const color4f_t& active);

        /// Sets the button background to overlay text on.
        void SetBackground(const obj::zEntity& Bg);

    private:
        obj::zEntity&   m_Active;
        obj::zEntity&   m_Normal;
        obj::zEntity*   mp_Current;

        color4f_t m_acolor;
        string_t m_text;
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
 *  dynamic and clean menus, and is only really useful on its own
 *  to create GUI overlays to in-game worlds and such.
 **/

/** @} **/
