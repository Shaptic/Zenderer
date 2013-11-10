#include "Zenderer/GUI/Menu.hpp"

using namespace zen;
using gui::zMenu;

zMenu::zMenu(gfx::zWindow& Window, asset::zAssetManager& Assets) :
    m_Scene(Window.GetWidth(), Window.GetHeight(), Assets),
    mp_Bg(nullptr), mp_Font(Assets.Create<gui::zFont>()),
    m_spacing(0)
{
    m_Scene.Init();
    m_menuActions.clear();
    m_Scene.DisableLighting();
    m_Scene.DisablePostProcessing();
}

zMenu::~zMenu()
{
    m_Scene.Destroy();
    for(auto& i : m_menuActions) delete i.first;
    m_menuActions.clear();
}

bool zMenu::HandleEvent(const evt::event_t& Evt)
{
    if(Evt.type == evt::EventType::MOUSE_MOTION)
    {
        for(auto& i : m_menuActions)
        {
            if(i.first->IsOver(Evt.mouse.position))
            {
                i.first->SetActive();
                break;
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
        size_t index = 0;
        for(auto& i : m_menuActions)
        {
            if(i.first->IsOver(Evt.mouse.position))
            {
                i.second(index);
                return true;
            }

            ++index;
        }
    }

    return false;
}

uint16_t zMenu::AddButton(const string_t& text, std::function<void(size_t)> handler)
{
    zButton* pNew = new zButton(m_Scene);
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
    m_menuActions.emplace_back(pNew, std::move(handler));

    return m_menuActions.size() - 1;
}

obj::zEntity& zMenu::AddEntity()
{
    return m_Scene.AddEntity();
}

gfx::zLight& zMenu::AddLight(const gfx::LightType& type)
{
    m_Scene.EnableLighting();
    return m_Scene.AddLight(type);
}

bool zMenu::RenderWithFont(obj::zEntity& Obj, const string_t& str,
                           const color4f_t& color)
{
    mp_Font->SetColor(color);
    return mp_Font->Render(Obj, str);
}

void zMenu::Update()
{
    m_Scene.Render();
}

bool zMenu::SetFont(const std::string& filename, const uint16_t size)
{
    mp_Font->SetSize(size);
    bool ret = mp_Font->LoadFromFile(filename);
    m_spacing = mp_Font->GetLineHeight() + 5;
    return ret;
}

void zMenu::SetButtonBackground(const obj::zEntity& Bg)
{
    mp_Bg = &Bg;
}

void zMenu::SetNormalButtonTextColor(const color4f_t& Color)
{
    m_ncolor = Color;
}

void zMenu::SetActiveButtonTextColor(const color4f_t& Color)
{
    m_acolor = Color;
}

void zMenu::SetInitialButtonPosition(const math::vector_t& Pos)
{
    m_Position = Pos;
}

void zMenu::SetSpacing(const uint16_t vertical_spacing)
{
    m_spacing = vertical_spacing;
}
