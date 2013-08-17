#include "Zenderer/GUI/Button.hpp"

using namespace zen;
using gui::CButton;

CButton::CButton(gfx::CScene& MenuScene) :
    m_Scene(MenuScene), m_Active(m_Scene.AddEntity()),
    m_Normal(m_Scene.AddEntity()), mp_Current(&m_Normal),
    mp_Font(nullptr)
{
}

CButton::~CButton()
{
    m_Scene.RemoveEntity(m_Active);
    m_Scene.RemoveEntity(m_Normal);
}

void CButton::Place(const math::vector_t& Pos)
{
    m_Active.Move(Pos);
    m_Normal.Move(Pos);
}

void CButton::Place(const real_t x, const real_t y)
{
    m_Active.Move(x, y);
    m_Normal.Move(x, y);
}

bool CButton::Prepare(const string_t& text)
{
    if(mp_Font == nullptr && mp_Current == nullptr || text.empty()) return false;

    mp_Font->SetColor(m_ncolor);
    bool ret = mp_Font->Render(m_Normal, text);

    mp_Font->SetColor(m_acolor);
    return ret && mp_Font->Render(m_Active, text);
}

bool CButton::IsOver(const math::vector_t& Pos)
{
    return this->IsOver(math::aabb_t(
        Pos, math::Vector<uint32_t>(2, 2))
    );
}

bool CButton::IsOver(const math::aabb_t& Box)
{
    if(mp_Current == nullptr || !mp_Font) return false;
    return mp_Current->GetBox().collides(Box);
}

bool CButton::SetActive()
{
    if(!mp_Font || mp_Current == &m_Active) return false;

    mp_Current->Disable();
    mp_Current = &m_Active;
    mp_Current->Enable();

    return true;
}

void CButton::SetDefault()
{
    if(!mp_Font || mp_Current == &m_Normal) return;

    mp_Current->Disable();
    mp_Current = &m_Normal;
    mp_Current->Enable();
}

void CButton::SetFont(gui::CFont& Font)
{
    Font.SetStacking(true);
    mp_Font = &Font;
}

void CButton::SetActiveColor(const color4f_t& active)
{
    m_acolor = active;
}

void CButton::SetNormalColor(const color4f_t& normal)
{
    m_ncolor = normal;
}

void CButton::SetBackground(const obj::CEntity& Bg)
{
    auto i = Bg.cbegin(), j = Bg.cend();
    for( ; i != j; ++i)
    {
        m_Active.AddPrimitive(**i);
        m_Normal.AddPrimitive(**i);
    }
}
