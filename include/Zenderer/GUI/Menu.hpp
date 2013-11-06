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

#include <vector>
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
    /// A high-level menu-creation API.
    class ZEN_API zMenu
    {
    public:
        /**
         * Creates a menu instance.
         *  The given parameters are used in order to create a
         *  `gfx::zScene` object to represent the menu internally.
         *
         * @param   Window  The window you wish to contain the menu
         * @param   Assets  The asset manager to load data with
         **/
        zMenu(gfx::zWindow& Window, asset::zAssetManager& Assets);
        virtual ~zMenu();

        /**
         * Handles system events to perform menu actions.
         *  This method must be called from the object creator whenever
         *  there are events polled to the application in order to
         *  have a properly functioning menu.
         *
         * @param   Evt     The event to process.
         *
         * @return  `true`  if a menu button was clicked,
         *          `false` otherwise.
         **/
        virtual bool HandleEvent(const evt::event_t& Evt);

        /**
         * Adds a button to the menu.
         *  This will place a button below the current list of buttons,
         *  if any, based on the padding set or the font size. The given
         *  callback handler will be triggered whenever the button in
         *  question is pressed.
         *  This somewhat functional approach eliminates bloat in terms
         *  of tracking button states on behalf of the user.
         *
         * @param   text    The button shall contain this rendered text
         * @param   handler Callback to execute when button is pressed
         *
         * @return  Unique identifier for the button just added.
         *          This value also corresponds to the current number
         *          of buttons added to the menu.
         *
         * @see     SetFont()
         * @see     SetSpacing()
         **/
        virtual uint16_t AddButton(const string_t& text,
                                   std::function<void(size_t)> handler);

        /**
         * Adds an entity object directly to the menu.
         *  This method is in-place to provide access to the underlying
         *  `gfx::zScene` object. Sometimes menus require more functionality
         *  or "stuff" than is provided by this API. The accessibility is
         *  limited to adding objects and lighting.
         *
         *  Direct access to the underlying scene is intentionally not
         *  be permitted.
         *
         * @return  A new entity that was added to the menu.
         *
         * @see     gfx::zScene::AddEntity()
         * @see     AddLight()
         **/
        virtual obj::zEntity& AddEntity();

        /**
         * Adds a light object directly to the menu,
         *  Similarly to AddEntity(), this method provides access to the
         *  functionality of the underlying `gfx::zScene` object.
         *
         * @param   type    The type of light to add to the scene
         *
         * @return  An initialized light of the given type (if it could
         *          initialize it) that has been added to the scene.
         *
         * @see     gfx::zScene::AddLight()
         **/
        virtual gfx::zLight& AddLight(const gfx::LightType& type);

        /**
         * Renders data with the internal menu font.
         *  This allows for the user to use the font they loaded into the
         *  menu for other rendering operations. For example, it could be
         *  tied in with `AddEntity()` above to create a scrolling credits
         *  list.
         *
         * @param   Obj     The entity to render text into
         * @param   str     The string to render
         *
         * @return  `true`  if the text was rendered successfully,
         *          `false` otherwise, or if no font has been loaded yet.
         *
         * @see     SetFont()
         **/
        virtual bool RenderWithFont(obj::zEntity& Obj, const string_t& str);

        /**
         * Updates the menu state.
         *  This should be called every frame by the user, in order to
         *  ensure on-time hovering effects and event handling.
         **/
        virtual void Update();

        /**
         * Loads a font for the menu to use.
         * @param   filename    Path to the font
         * @param   size        Font size to load
         * @return  `true`  if it went swell, `false` otherwise.
         **/
        bool SetFont(const string_t& filename, const uint16_t size=18);

        /// Sets a background for buttons which text will overlay.
        void SetButtonBackground(const obj::zEntity& Bg);

        /// Sets "normal" button state text color.
        void SetNormalButtonTextColor(const color4f_t& Color);

        /// Sets "active" button state text color.
        void SetActiveButtonTextColor(const color4f_t& Color);

        /// Sets the place where buttons will begin to be added.
        void SetInitialButtonPosition(const math::vector_t& Pos);

        /// Renders a menu title in a certain position.
        void SetTitle(const string_t& Title, const math::vector_t& Pos);

        /// Sets spacing between buttons, defaults to font size.
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
 *  fonts, and backgrounds.
 **/

/** @} **/
