/**
 * @file
 *  Zenderer/GUI/EntryField.hpp - A wrapper for easily creating user
 *  input fields.
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
 * @addtogroup GUI
 * @{
 **/

#ifndef ZENDERER__GUI__ENTRY_FIELD_HPP
#define ZENDERER__GUI__ENTRY_FIELD_HPP

#include <functional>
#include "Element.hpp"

namespace zen
{
namespace gui
{
    enum class InputStyle : int16_t
    {
        HIDDEN,
        BORDERED,
        FILLED
    };

    /// Simluates an entry box for user input.
    class ZEN_API zEntryField : public zElement
    {
    public:
        using zElement::IsOver;
        using zElement::Place;

        /// Loads 3 entities into the scene.
        zEntryField(gfx::zScene& MenuScene, asset::zAssetManager& Assets) :
            zElement(MenuScene, Assets),  m_Label(m_Scene.AddEntity()),
            m_Field(m_Scene.AddEntity()), m_Text(m_Scene.AddEntity()),
            mp_Font(nullptr), m_Style(InputStyle::FILLED), m_limit(0)
        {
            m_filter = [](char) { return true; };
        }

        ~zEntryField() {}

        void Place(const real_t x, const real_t y)
        {
            m_Label.Move(x, y);
            m_Field.Move(x +  m_Label.GetWidth()  + 5,
                         y - (m_Field.GetHeight() - m_Label.GetHeight()) / 2);
            m_Text.Move(m_Field.GetX() + 5, y + 3);
        }

        virtual bool Create(gui::zFont& Font)
        {
            this->Destroy();
            this->RenderText(m_Label, Font, m_label);

            uint16_t size = (m_limit > 0) ? m_limit : 16;

            switch(m_Style)
            {
            case InputStyle::BORDERED:
            case InputStyle::FILLED:
                m_Field.AddPrimitive(gfx::zQuad(
                    m_Assets, size * Font.GetTextWidth("_"),
                    Font.GetLineHeight() + 10).SetColor(m_outline).Create()
                );
                break;

            case InputStyle::HIDDEN:
                m_Field.Disable();
                break;
            }

            mp_Font = &Font;
            return true;
        }

        virtual bool IsOver(const math::rect_t& Rect)
        {
            return m_Field.Collides(Rect);
        }

        virtual void Focus()
        {
            m_focus = true;
            m_Field.Destroy();
            m_Field.AddPrimitive(gfx::zQuad(
                m_Assets, m_Field.GetWidth(),
                mp_Font->GetLineHeight() + 10).SetColor(m_inv_outline).Create()
            );
        }

        virtual void Unfocus()
        {
            m_focus = false;
            m_Field.Destroy();
            m_Field.AddPrimitive(gfx::zQuad(
                m_Assets, m_Field.GetWidth(),
                mp_Font->GetLineHeight() + 10).SetColor(m_outline).Create()
            );
        }

        void OnChange(std::function<void(const string_t&)> fun)
        {
            m_change = std::move(fun);
        }

        void HandleEvent(const evt::event_t& Event)
        {
            ZEN_ASSERT(mp_Font != nullptr);
            ZEN_ASSERT(m_filter.operator bool());

            if(!m_focus) return;

            else if(Event.type == evt::EventType::KEY_PRINTABLE &&
                   (m_input.length() < m_limit || m_limit == 0))
            {
                if(m_filter(Event.key.symbol))
                {
                    m_Text.Destroy();
                    m_Text.Enable();
                    m_input += Event.key.symbol;
                    this->RenderText(m_Text, *mp_Font, m_input, &m_incolor);
                    if(m_change) m_change(m_input);
                }
            }

            else if((Event.type == evt::EventType::KEY_DOWN  ||
                     Event.type == evt::EventType::KEY_HOLD) &&
                     Event.key.key == evt::Key::BACKSPACE)
            {
                if(m_input.length() <= 1)
                {
                    m_Text.Disable();
                    m_input.clear();
                }
                else
                {
                    m_Text.Destroy();
                    m_input = m_input.substr(0, m_input.length() - 1);
                    this->RenderText(m_Text, *mp_Font, m_input, &m_incolor);
                    if(m_change) m_change(m_input);
                }
            }
        }

        /// Sets entry field color(s).
        void SetInputColor(color4f_t normal, color4f_t focused)
        {
            m_outline = normal;
            m_inv_outline = focused;
        }

        void SetInputTextColor(color4f_t outline)
        {
            m_incolor = outline;
        }

        void SetLabel(string_t label)
        {
            m_label = label;
        }

        void SetInputStyle(const InputStyle& Style)
        {
            m_Style = Style;
        }

        /// Sets character limit for input field, 0 is unlimited.
        void SetMaxChars(const uint16_t char_limit)
        {
            m_limit = char_limit;
        }

        void SetFilterFunction(std::function<bool(char)> filter)
        {
            m_filter = std::move(filter);
        }

        const string_t& GetInput() const
        {
            return m_input;
        }

        uint16_t GetHeight() const
        {
            return math::max<uint16_t>(
                   math::max<uint16_t>(
                        m_Text.GetHeight(), m_Field.GetHeight()
                   ), m_Label.GetHeight());
        }

    private:
        void Destroy()
        {
            m_input.clear();
            m_Text.Destroy();
            m_Field.Destroy();
            m_Label.Destroy();
        }

        obj::zEntity&   m_Label;
        obj::zEntity&   m_Field;
        obj::zEntity&   m_Text;

        zFont*          mp_Font;
        InputStyle      m_Style;

        color4f_t       m_outline, m_inv_outline, m_incolor;
        string_t        m_input, m_label;
        uint32_t        m_limit;

        std::function<bool(char)> m_filter;
        std::function<void(const string_t&)> m_change;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__ENTRY_FIELD_HPP

/**
 * @class zen::gui::zEntryField
 * @details
 *  A wrapper to control user input.
 **/

/** @} **/
