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
    m_menuActions.clear();
    m_Scene.DisableLighting();
    m_Scene.DisablePostProcessing();
    mp_Font->AttachManager(Assets);
}

CMenu::~CMenu()
{
    m_Scene.Destroy();
    for(auto& i : m_menuActions) delete i.first;
    m_menuActions.clear();
}

bool CMenu::HandleEvent(const evt::event_t& Evt)
{
    if(Evt.type == evt::EventType::MOUSE_MOTION)
    {
        math::aabb_t MouseBox(Evt.mouse.position,
            math::Vector<uint32_t>(2, 2));

        for(auto& i : m_menuActions)
        {
            if(i.first->IsOver(MouseBox))
            {
                i.first->SetActive();
            }
            else
            {
                i.first->SetDefault();
            }
        }
    }

    else if(Evt.type == evt::EventType::MOUSE_DOWN &&
            Evt.mouse.button == evt::MouseButton::LEFT)
    {
        math::aabb_t MouseBox(Evt.mouse.position,
            math::Vector<uint32_t>(2, 2));

        size_t index = 0;
        for(auto& i : m_menuActions)
        {
            if(i.first->IsOver(MouseBox))
            {
                i.second(index);
                return true;
            }
            
            ++index;
        }
    }

    return false;
}

uint16_t CMenu::AddButton(const string_t& text, std::function<void(size_t)> handler)
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
    
    // Credit where credit is due:
    // @Griwes for this optimization.
    m_menuActions.emplace(std::make_pair(pNew, std::move(handler)));

    return m_menuActions.size() - 1;
}

void CMenu::Update()
{
    m_Scene.Render();
}

bool CMenu::SetFont(const std::string& filename, const uint16_t size)
{
    mp_Font->SetSize(size);
    bool ret = mp_Font->LoadFromFile(filename);
    m_spacing = mp_Font->GetLineHeight();
    return ret;
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
