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

#include "Zenderer/Graphics/Scene.hpp"
#include "Font.hpp"

namespace zen
{
namespace gui
{
    class ZEN_API CButton
    {
    public:
        CButton(gfx::CScene& MenuScene) :
            m_Scene(MenuScene), m_Active(m_Scene.AddEntity()),
            m_Normal(m_Scene.AddEntity()), mp_Current(&m_Normal),
            mp_Font(nullptr) {}

        ~CButton()
        {
            m_Scene.RemoveEntity(m_Active);
            m_Scene.RemoveEntity(m_Normal);
        }

        void SetFont(CFont& Font)
        {
            Font.SetStacking(true);
            mp_Font = &Font;
        }

        void SetActiveColor(const color4f_t& active)
        {
            m_acolor = active;
        }

        void SetNormalColor(const color4f_t& normal)
        {
            m_ncolor = normal;
        }

        void SetBackground(const obj::CEntity& Bg)
        {
            auto i = Bg.cbegin(), j = Bg.cend();
            for( ; i != j; ++i)
            {
                m_Active.AddPrimitive(**i);
                m_Normal.AddPrimitive(**i);
            }
        }

        void Place(const math::vector_t& Pos)
        {
            m_Active.Move(Pos);
            m_Normal.Move(Pos);
        }

        void Place(const real_t x, const real_t y)
        {
            m_Active.Move(x, y);
            m_Normal.Move(x, y);
        }

        bool Prepare(const string_t& text)
        {
            if(!mp_Font || mp_Current == nullptr) return false;

            mp_Font->SetColor(m_ncolor);
            bool ret = mp_Font->Render(m_Normal, text);

            mp_Font->SetColor(m_acolor);
            return ret && mp_Font->Render(m_Active, text);
        }

        bool SetActive()
        {
            if(!mp_Font || mp_Current == &m_Active) return false;

            mp_Current->Disable();
            mp_Current = &m_Active;
            mp_Current->Enable();

            return true;
        }

        bool IsOver(const math::vector_t& Pos)
        {
            return this->IsOver(math::aabb_t(
                Pos, math::Vector<uint32_t>(2, 2))
            );
        }

        bool IsOver(const math::aabb_t& Box)
        {
            if(mp_Current == nullptr || !mp_Font) return false;
            return mp_Current->GetBox().collides(Box);
        }

        void SetDefault()
        {
            if(!mp_Font || mp_Current == &m_Normal) return;

            mp_Current->Disable();
            mp_Current = &m_Normal;
            mp_Current->Enable();
        }

    private:
        gfx::CScene&    m_Scene;
        obj::CEntity&   m_Active;
        obj::CEntity&   m_Normal;
        obj::CEntity*   mp_Current;
        gui::CFont*     mp_Font;

        color4f_t m_acolor, m_ncolor;
    };
}   // namespace gui
}   // namespace zen

#endif  // ZENDERER__GUI__BUTTON_HPP

/**
 * @class zen::gui::CButton
 * @details
 *  A wrapper for a menu button that allows for text to be easily
 *  rendered on top of a background, and to easily add effects when
 *  some sort of event occurs, like mouse-over.
 *  This is used internally by zen::gui::CMenu to easily create
 *  dynamic and clean menus, and likely shouldn't really be touched
 *  on its own.
 **/

/** @} **/
