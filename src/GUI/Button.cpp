#include "Zenderer/GUI/Button.hpp"

using namespace zen;
using gui::zButton;

zButton::zButton(gfx::zScene& MenuScene) :
    m_Scene(MenuScene), m_Active(m_Scene.AddEntity()),
    m_Normal(m_Scene.AddEntity()), mp_Current(&m_Normal),
    mp_Font(nullptr)
{
}

zButton::~zButton()
{
    m_Scene.RemoveEntity(m_Active);
    m_Scene.RemoveEntity(m_Normal);
}

void zButton::Place(const math::vector_t& Pos)
{
    m_Active.Move(Pos);
    m_Normal.Move(Pos);
}

void zButton::Place(const real_t x, const real_t y)
{
    m_Active.Move(x, y);
    m_Normal.Move(x, y);
}

bool zButton::Prepare(const string_t& text)
{
    if(mp_Font == nullptr && mp_Current == nullptr || text.empty()) return false;

    mp_Font->SetColor(m_ncolor);
    bool ret = mp_Font->Render(m_Normal, text);

    mp_Font->SetColor(m_acolor);
    return ret && mp_Font->Render(m_Active, text);
}

bool zButton::IsOver(const math::vector_t& Pos)
{
    return this->IsOver(math::rect_t(Pos.x, Pos.y, 1, 1));
}

bool zButton::IsOver(const math::rect_t& Box)
{
    return (mp_Current && mp_Font && mp_Current->Collides(Box));
}

void zButton::SetActive()
{
    if(!mp_Font || mp_Current == &m_Active) return;

    mp_Current->Disable();
    mp_Current = &m_Active;
    mp_Current->Enable();
}

void zButton::SetDefault()
{
    if(!mp_Font || mp_Current == &m_Normal) return;

    mp_Current->Disable();
    mp_Current = &m_Normal;
    mp_Current->Enable();
}

void zButton::SetFont(gui::zFont& Font)
{
    Font.SetStacking(true);
    mp_Font = &Font;
}

void zButton::SetActiveColor(const color4f_t& active)
{
    m_acolor = active;
}

void zButton::SetNormalColor(const color4f_t& normal)
{
    m_ncolor = normal;
}

void zButton::SetBackground(const obj::zEntity& Bg)
{
    for(const auto& i : Bg)
    {
        m_Active.AddPrimitive(*i);
        m_Normal.AddPrimitive(*i);
    }
}
