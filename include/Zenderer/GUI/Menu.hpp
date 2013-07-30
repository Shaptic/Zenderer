/**
 * @file
 *  Zenderer/GUI/Menu.hpp - A high-level menu creating interface.
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

#ifndef ZENDERER__GUI__MENU_HPP
#define ZENDERER__GUI__MENU_HPP

#include "Zenderer/Events/EventHandler.hpp"
#include "Zenderer/Events/Mouse.hpp"
#include "Zenderer/Graphics/Scene.hpp"
#include "Font.hpp"
#include "Button.hpp"

namespace zen
{
namespace gui
{
    class ZEN_API CMenu
    {
    public:
        CMenu(gfx::CWindow& Window, asset::CAssetManager& Assets) :
            m_Scene(Window.GetWidth(), Window.GetHeight(), Assets),
            m_Background(m_Scene.AddEntity()), mp_Font(nullptr),
            m_Title(m_Scene.AddEntity()),
            mp_Bg(nullptr), m_spacing(0)
        {
            m_Scene.Init();
            mp_menuButtons.clear();
            m_Scene.DisableLighting();
        }

        virtual ~CMenu()
        {
            m_Scene.Destroy();
            for(auto& i : mp_menuButtons) delete i;
            mp_menuButtons.clear();
        }

        virtual int16_t HandleEvent(const evt::event_t& Evt)
        {
            if(Evt.type == evt::EventType::MOUSE_MOTION)
            {
                math::aabb_t MouseBox(Evt.mouse.position,
                                      math::Vector<uint32_t>(2, 2));

                auto i = mp_menuButtons.begin(),
                     j = mp_menuButtons.end();

                for( ; i != j; ++i)
                {
                    if((*i)->IsOver(MouseBox))
                    {
                        (*i)->SetActive();
                    }
                    else
                    {
                        (*i)->SetDefault();
                    }
                }
            }

            else if(Evt.type == evt::EventType::MOUSE_DOWN &&
                    Evt.mouse.button == evt::MouseButton::LEFT)
            {
                math::aabb_t MouseBox(Evt.mouse.position,
                                      math::Vector<uint32_t>(2, 2));

                for(size_t i = 0; i < mp_menuButtons.size(); ++i)
                {
                    if(mp_menuButtons[i]->IsOver(MouseBox)) return i;
                }
            }

            return -1;
        }

        virtual uint16_t AddButton(const string_t& text)
        {
            CButton* pNew = new CButton(m_Scene);
            pNew->SetFont(*mp_Font);
            pNew->SetActiveColor(m_acolor);
            pNew->SetNormalColor(m_ncolor);

            if(mp_Bg != nullptr) pNew->SetBackground(*mp_Bg);

            pNew->Prepare(text);
            pNew->Place(m_Position);
            pNew->SetDefault();

            m_Position.y += m_spacing;
            mp_menuButtons.push_back(pNew);
            return mp_menuButtons.size() - 1;
        }

        virtual void Update()
        {
            m_Scene.Render();
        }

        void SetFont(gui::CFont& Font)
        {
            mp_Font = &Font;
        }

        void SetButtonBackground(const obj::CEntity& Bg)
        {
            mp_Bg = &Bg;
        }

        void SetNormalButtonTextColor(const color4f_t& Color)
        {
            m_ncolor = Color;
        }

        void SetActiveButtonTextColor(const color4f_t& Color)
        {
            m_acolor = Color;
        }

        void SetInitialButtonPosition(const math::vector_t& Pos)
        {
            m_Position = Pos;
        }

        void SetSpacing(const uint16_t vertical_spacing)
        {
            m_spacing = vertical_spacing;
        }

    private:
        gfx::CScene m_Scene;
        obj::CEntity& m_Background;
        obj::CEntity& m_Title;
        const obj::CEntity* mp_Bg;

        gui::CFont* mp_Font;
        color4f_t m_acolor, m_ncolor;

        std::vector<CButton*> mp_menuButtons;

        math::vector_t m_Position;
        uint16_t m_spacing;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__MENU_HPP

/**
 * @class zen::gui::CMenu
 * @details
 *  A high-level wrapper that facilitates a simple method of creating
 *  fairly customizable and high-quality menus with custom buttons,
 *  fonts, backgrounds, and other sections.
 **/

/** @} **/
