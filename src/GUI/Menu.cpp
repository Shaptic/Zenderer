#include "Zenderer/GUI/Menu.hpp"

using namespace zen;
using gui::CMenu;

CMenu::CMenu(gfx::CWindow& Window, asset::CAssetManager& Assets) :
    m_Scene(Window.GetWidth(), Window.GetHeight(), Assets),
    m_Background(m_Scene.AddEntity()), mp_Font(Assets.Create<gui::CFont>()),
    m_Title(m_Scene.AddEntity()),
    mp_Bg(nullptr), m_spacing(0)
{
    m_Scene.Init();
    mp_menuButtons.clear();
    m_Scene.DisableLighting();
    mp_Font->AttachManager(Assets);
}

CMenu::~CMenu()
{
    m_Scene.Destroy();
    for(auto& i : mp_menuButtons) delete i;
    mp_menuButtons.clear();
}

int16_t CMenu::HandleEvent(const evt::event_t& Evt)
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

uint16_t CMenu::AddButton(const string_t& text)
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

void CMenu::Update()
{
    m_Scene.Render();
}

bool CMenu::SetFont(const std::string& filename)
{
    return mp_Font->LoadFromFile(filename);
}

void CMenu::SetButtonBackground(const obj::CEntity& Bg)
{
    mp_Bg = &Bg;
}

void CMenu::SetNormalButtonTextColor(const color4f_t& Color)
{
    m_ncolor = Color;
}

void CMenu::SetActiveButtonTextColor(const color4f_t& Color)
{
    m_acolor = Color;
}

void CMenu::SetInitialButtonPosition(const math::vector_t& Pos)
{
    m_Position = Pos;
}

void CMenu::SetSpacing(const uint16_t vertical_spacing)
{
    m_spacing = vertical_spacing;
}
