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
#include <sstream>

#include "Zenderer/Events/EventHandler.hpp"
#include "Zenderer/Events/Mouse.hpp"
#include "Zenderer/Graphics/Scene.hpp"

#include "Font.hpp"
#include "Button.hpp"
#include "EntryField.hpp"

namespace zen
{
namespace gui
{
    using math::vector_t;

    /**
     * Menu configuration structure.
     *  If certain values are not set, the following rules will be used:
     *
     *      If `button_f`, `label_f`, or `input_f` are not set,
     *      `font` will be used for all three.
     *
     *      If `button_foccol` or `button_normcol` aren't set, white
     *      and black are used, respectively.
     *
     *      If `input_style` isn't set, `InputStyle::FILLED` is assumed.
     *
     *      If `input_boxcol`, `input_col`, or `label_col` are not set,
     *      white, black, and white are used, respectively.
     *
     * Theme files should be structured like other Zenderer configuration
     * files, or simply following an INI-style format.
     *
     * Here are the values and their corresponding field in a menu
     * configuration structure:
     *
     * | Label           | Field Name       |
     * | --------------- | ---------------- |
     * | Background      | background       |
     * | Title           | title            |
     * | MenuFont        | font             |
     * | MenuFontSize    | font_size        |
     * | TitlePosition   | title_pos        |
     * |                 |                  |
     * | ButtonFont      | button_f         |
     * | ButtonFontSize  | button_fs        |
     * | ButtonPosition  | button_pos       |
     * | ButtonFocusCol  | button_foccol    |
     * | ButtonNormalCol | button_normcol   |
     * |                 |                  |
     * | LabelFont       | label_f          |
     * | InputFont       | input_f          |
     * | InputFontSize   | input_fs         |
     * | LabelFontSize   | label_fs         |
     * | InputStyle      | input_style      |
     * | InputFocusCol   | input_foccol     |
     * | InputNormalCol  | input_normcol    |
     * | InputTextColor  | input_col        |
     * | LabelColor      | input_col        |
     *
     **/
    struct menucfg_t
    {
        string_t    background;     ///< Menu background
        string_t    title;          ///< Menu title
        string_t    font;           ///< Menu font
        uint16_t    font_size;      ///< Menu font size
        vector_t    title_pos;      ///< Position of title on screen

        string_t    button_f;       ///< Button font
        uint16_t    button_fs;      ///< Button font size
        vector_t    button_pos;     ///< Starting position for buttons
        color4f_t   button_foccol;  ///< Button text focused color
        color4f_t   button_normcol; ///< Button text normal color

        string_t    label_f;        ///< Label font for inputs
        string_t    input_f;        ///< Input context font
        uint16_t    label_fs;       ///< Label font size
        uint16_t    input_fs;       ///< Input context font size
        InputStyle  input_style;    ///< Style for entry field boxes
        color4f_t   input_foccol;   ///< Input box color when focused  (based on style)
        color4f_t   input_normcol;  ///< Input box color when inactive (based on style)
        color4f_t   input_col;      ///< Input text color
        color4f_t   label_col;      ///< Input box label text color

        bool        valid;          ///< Configuration state
    };

    /// A high-level menu-creation API.
    class ZEN_API zMenu
    {
    public:
        static const menucfg_t DEFAULT_SETTINGS;

        /**
         * Creates a menu instance.
         *  The given parameters are used in order to create a
         *  `gfx::zScene` object to represent the menu internally.
         *
         * @param   Window  The window you wish to contain the menu
         * @param   Assets  The asset manager to load data with
         **/
        zMenu(gfx::zWindow& Window, asset::zAssetManager& Assets,
              const menucfg_t& settings = DEFAULT_SETTINGS);
        virtual ~zMenu();

        static menucfg_t LoadThemeFromFile(const string_t& path);

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

        /// @todo   Document this.
        virtual uint16_t AddEntryField(const string_t& label_text,
                                       const vector_t& Position,
                                       std::function<void(const string_t&)> action,
                                       std::function<bool(char)> filter =
                                            [](char) { return true; },
                                       const string_t& prefill = "");

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
         * @param   color   The color to render text with   (optional)
         *
         * @return  `true`  if the text was rendered successfully,
         *          `false` otherwise, or if no font has been loaded yet.
         *
         * @see     SetFont()
         **/
        virtual bool RenderWithFont(obj::zEntity& Obj, const string_t& str,
                                    const color4f_t& color = color4f_t(0, 0, 0));

        /**
         * Updates the menu state.
         *  This should be called every frame by the user, in order to
         *  ensure on-time hovering effects and event handling.
         **/
        virtual void Update();

        /// Sets a background for buttons which text will overlay.
        void SetButtonBackground(const obj::zEntity& Bg);

        /// Sets spacing between buttons, defaults to font size.
        void SetSpacing(const uint16_t vertical_spacing);

        /// Enables the menu for use as an "overlay" (think HUD).
        void SetOverlayMode(const bool flag);

    private:
        static bool IsColor(const string_t& c)
        {
            if(c.empty()) return false;

            auto parts = util::split(c, ',');
            if(parts.size() == 4)
            {
                return std::find_if(c.begin(), c.end(), [](const char i) {
                    return !(isdigit(i) || i == '.' || i == '-' || i == '+');
                }) == c.end();
            }

            if(c.length() == 7 && c[0] == '#')
            {
                return std::find_if(c.begin() + 1, c.end(), [](const char i) {
                    return !(isdigit(i) || (toupper(i) >= 65 && toupper(i) <= 70));
                }) == c.end();
            }

            return false;
        }

        /**
         * @todo Get your shit together with error handling.
         *       An exception is thrown using MSVC on a bad `std::stoi()` call.
         **/
        static color4f_t ParseColor(const string_t& s)
        {
            color4f_t result;

            auto parts = util::split(s, ',');
            if(parts.size() == 1 && s[0] == '#' && s.length() == 7)
            {
                int num =
#ifndef __GNUC__
                    std::stoi(s.substr(1), nullptr, 16);
#else
                    0;
                std::stringstream ss(s.substr(1));
                ss >> std::hex >> num;
#endif // __GNUC__
                result.r = ((num >> 16) & 0xFF) / 255.0;
                result.g = ((num >> 8) & 0xFF) / 255.0;
                result.b =  (num & 0xFF) / 255.0;
            }

            else if(parts.size() == 4)
                result = color4f_t(std::stod(parts[0]),
                                   std::stod(parts[1]),
                                   std::stod(parts[2]),
                                   std::stod(parts[3]));

            return result;
        }

        bool LoadFont(const string_t&    font_name,
                      const uint16_t     font_size,
                      gui::zFont*&       font_ptr,
                      const string_t&    font_def_name,
                      const uint16_t     font_def_size = 18);

        asset::zAssetManager& m_Assets;
        gfx::zScene m_Scene;

        gui::zFont* mp_MenuFont;
        gui::zFont* mp_ButtonFont;
        gui::zFont* mp_LabelFont;
        gui::zFont* mp_InputFont;

        std::vector<zEntryField*> m_menuInputs;
        std::vector<
            std::pair<
                zButton*,
                std::function<void(size_t)>
            >
        > m_menuActions;

        vector_t    m_Position;
        menucfg_t   m_settings;
        uint16_t    m_spacing;
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
 *
 * @todo    Fix wiki documentation after `zElement` re-structuring.
 **/

/** @} **/
