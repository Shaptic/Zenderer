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

#include <unordered_map>
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
    class ZEN_API zMenu
    {
    public:
        zMenu(gfx::zWindow& Window, asset::zAssetManager& Assets);

        virtual ~zMenu();

        virtual bool HandleEvent(const evt::event_t& Evt);
        virtual uint16_t AddButton(const string_t& text,
                                   std::function<void(size_t)> handler);
        virtual obj::zEntity& AddEntity();
        virtual bool RenderWithFont(obj::zEntity& Obj, const string_t& str);
        virtual void Update();

        bool SetFont(const string_t& filename, const uint16_t size=18);
        void SetButtonBackground(const obj::zEntity& Bg);
        void SetNormalButtonTextColor(const color4f_t& Color);
        void SetActiveButtonTextColor(const color4f_t& Color);
        void SetInitialButtonPosition(const math::vector_t& Pos);
        void SetTitle(const string_t& Title, const math::vector_t& Pos);
        void SetSpacing(const uint16_t vertical_spacing);

    private:
        gfx::zScene m_Scene;
        const obj::zEntity* mp_Bg;
        gui::zFont* mp_Font;
        color4f_t m_acolor, m_ncolor;

        std::vector<
            std::pair<
                zButton*,
                std::function<void(size_t)>
            >
        > m_menuActions;

        math::vector_t m_Position;
        uint16_t m_spacing;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__MENU_HPP

/**
 * @class zen::gui::zMenu
 * @details
 *  A high-level wrapper that facilitates a simple method of creating
 *  fairly customizable and high-quality menus with custom buttons,
 *  fonts, backgrounds, and other sections.
 *
 * @todo    Document the API.
 **/

/** @} **/
