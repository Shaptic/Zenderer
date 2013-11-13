/**
 * @file
 *  Zenderer/GUI/Element.hpp - An abstract base class for GUI elements.
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

#ifndef ZENDERER__GUI__ELEMENT_HPP
#define ZENDERER__GUI__ELEMENT_HPP

#include "Zenderer/Graphics/Scene.hpp"
#include "Font.hpp"

namespace zen
{
namespace gui
{
    /// An abstraction of a GUI element.
    class ZEN_API zElement
    {
    public:
        /**
         * Creates the GUI element.
         *
         * @param   GUI     The scene to attach this element to
         * @param   Assets  Asset manager to create element from
         *
         * @pre     `MenuScene` must have been initialized.
         * @post    `MenuScene` contains a number of new entities.
         **/
        zElement(gfx::zScene& GUI, asset::zAssetManager& Assets) :
            m_Scene(GUI), m_Assets(Assets), m_focus(false) {}

        /**
         * Places the GUI element somewhere in the scene.
         *
         * @param   x   X-placement coordinate
         * @param   y   X-placement coordinate
         **/
        virtual void Place(const real_t x, const real_t y) = 0;
        inline void Place(const math::vector_t& Pos)    ///< @overload
        { Place(Pos.x, Pos.y); }

        /**
         * Creates the GUI element.
         *  This should be called *after* specifying all of the
         *  customization parameters for the element.
         *
         * @param   Font    Loaded font to use for rendering text
         *
         * @return  `true`  if the element was created successfully,
         *          `false` otherwise.
         *
         * @note    This modifies the given font, but old font
         *          state is preserved and restored after this
         *          method call.
         **/
        virtual bool Create(gui::zFont& Font) = 0;

        /**
         * Checks if the given position is over the element.
         *
         * @param   Box     Collision area to check
         *
         * @return  `true`  if the given area collides w/ the element,
         *          `false` otherwise.
         **/
        virtual bool IsOver(const math::rect_t& Box) = 0;
        inline bool IsOver(const math::vector_t& Pos)   ///< @overload
        { return this->IsOver(math::rect_t(Pos.x, Pos.y, 1, 1)); }
        inline bool IsOver(real_t x, real_t y)          ///< @overload
        { return this->IsOver(math::rect_t(x, y, 1, 1)); }

        /**
         * Activates the element's "focused" state.
         *  This varies from element to element, and may sometimes
         *  have no effect whatsoever.
         *
         * @see Unfocus()
         **/
        virtual inline void Focus()   { m_focus = true; }

        /// Removes the element's "focused" state.
        virtual inline void Unfocus() { m_focus = false; }

        /// Is the element currently in a focused state?
        bool IsFocused() const { return m_focus; }

        /**
         * Sets the text color for rendering the GUI element.
         *  This has varying effects on different elements. Some elements
         *  have multiple textual parts to render, and this will activate
         *  a specific one of those. This should be noted in that element's
         *  respective documentation.
         *
         *  By default, white text will be used.
         *
         * @param   text_color  Color to render element's text
         **/
        inline void SetColor(color4f_t text_color)
        { m_tcolor = text_color; }

    protected:
        virtual void RenderText(obj::zEntity& E, gui::zFont& Font,
                                const string_t& text,
                                const color4f_t* const c = nullptr)
        {
            color4f_t old = Font.GetColor();
            Font.SetColor((c == nullptr) ? m_tcolor : *c);
            Font.Render(E, text);
            Font.SetColor(old);
        }

        gfx::zScene&            m_Scene;
        asset::zAssetManager&   m_Assets;

        math::vector_t  m_position; // element position
        color4f_t       m_tcolor;   // text color
        bool            m_focus;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__ELEMENT_HPP

/**
 * @class zen::gui::zElement
 * @details
 *  Every GUI element abstraction (buttons, entry fields, etc.) must inherit
 *  from this class. It provides some useful abstractions that come in
 *  handy regardless of what element it is, such as detecting mouse-overs
 *  (for element animations, focusing, etc). It also specifies exactly what
 *  functionality a GUI element must be able to perform.
 **/

/** @} **/
