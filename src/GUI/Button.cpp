#include "Zenderer/GUI/Button.hpp"

using namespace zen;
using gui::zButton;

zButton::zButton(gfx::zScene& MenuScene,
                 asset::zAssetManager& Assets) :
    zElement(MenuScene, Assets),
    m_Active(m_Scene.AddEntity()), m_Normal(m_Scene.AddEntity()),
    mp_Current(&m_Normal)
{
}

zButton::~zButton()
{
    m_Scene.RemoveEntity(m_Active);
    m_Scene.RemoveEntity(m_Normal);
}

void zButton::Place(const real_t x, const real_t y)
{
    m_Active.Move(x, y);
    m_Normal.Move(x, y);
}

bool zButton::Create(gui::zFont& Font)
{
    if(mp_Current == nullptr || m_text.empty()) return false;

    this->RenderText(m_Normal, Font, m_text);
    this->RenderText(m_Active, Font, m_text, &m_acolor);
    return true;
}

bool zButton::IsOver(const math::rect_t& Box)
{
    return (mp_Current && mp_Current->Collides(Box));
}

void zButton::Focus()
{
    if(mp_Current == &m_Active) return;

    mp_Current->Disable();
    mp_Current = &m_Active;
    mp_Current->Enable();
}

void zButton::Unfocus()
{
    if(mp_Current == &m_Normal) return;

    mp_Current->Disable();
    mp_Current = &m_Normal;
    mp_Current->Enable();
}

void zButton::SetText(const string_t& text)
{
    m_text = text;
}

void zButton::SetFocusColor(const color4f_t& active)
{
    m_acolor = active;
}

void zButton::SetBackground(const obj::zEntity& Bg)
{
    for(const auto& i : Bg)
    {
        m_Active.AddPrimitive(*i);
        m_Normal.AddPrimitive(*i);
    }
}
