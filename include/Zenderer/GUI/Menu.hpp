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

#include <functional>

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
        CMenu(gfx::CWindow& Window, asset::CAssetManager& Assets);

        virtual ~CMenu();

        virtual bool HandleEvent(const evt::event_t& Evt);

        virtual uint16_t AddButton(const string_t& text,
                                   std::function<void()> handler);

        virtual void Update();

        bool SetFont(const std::string& filename);
        void SetButtonBackground(const obj::CEntity& Bg);
        void SetNormalButtonTextColor(const color4f_t& Color);
        void SetActiveButtonTextColor(const color4f_t& Color);
        void SetInitialButtonPosition(const math::vector_t& Pos);
        void SetSpacing(const uint16_t vertical_spacing);

    private:
        gfx::CScene m_Scene;
        obj::CEntity& m_Background;
        obj::CEntity& m_Title;
        const obj::CEntity* mp_Bg;

        gui::CFont* mp_Font;
        color4f_t m_acolor, m_ncolor;

        std::map<CButton*, std::function<int()>> m_menuActions;

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
