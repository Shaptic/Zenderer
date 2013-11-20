#include "Zenderer/GUI/Menu.hpp"

using namespace zen;
using gui::zMenu;

#define LOG_ERROR(text)     \
    Log << Log.SetMode(util::LogMode::ZEN_FATAL)    \
        << Log.SetSystem("Menu") << text            \
        << util::zLog::endl;

gui::menucfg_t zMenu::DEFAULT_SETTINGS = {
    "", "", ZENDERER_FONT_PATH"default.ttf", 18,
    math::vector_t(),

    "", 0, math::vector_t(),
    color4f_t(), color4f_t(0, 0, 0),

    "", "", 0, 0, gui::InputStyle::FILLED,
    color4f_t(), color4f_t(0, 0, 0), color4f_t()
};

zMenu::zMenu(gfx::zWindow& Window, asset::zAssetManager& Assets,
             const menucfg_t& settings) :
    m_Scene(Window.GetWidth(), Window.GetHeight(), Assets), m_Assets(Assets),
    m_settings(settings), m_spacing(0)
{
    m_Scene.Init();
    m_menuActions.clear();
    m_menuInputs.clear();
    m_Scene.DisableLighting();
    m_Scene.DisablePostProcessing();

    util::zLog& Log = util::zLog::GetEngineLog();
    if(settings.font == "" || settings.font_size == 0)
    {
        LOG_ERROR("No menu font specified.");
        return;
    }

    gui::fontcfg_t f = { settings.font_size };
    mp_MenuFont = Assets.Create<gui::zFont>(settings.font, nullptr,
                                            static_cast<void*>(&f));

    if(mp_MenuFont == nullptr)
    {
        LOG_ERROR("Failed to load menu font.");
        return;
    }

    if(!this->LoadFont(settings.button_f, settings.button_fs, mp_ButtonFont, f.size) ||
        this->LoadFont(settings.label_f,  settings.label_fs,  mp_LabelFont,  f.size) ||
        this->LoadFont(settings.input_f,  settings.input_fs,  mp_InputFont,  f.size))
    {
        LOG_ERROR("Failed to load a font.");
        return;
    }

    m_Position = settings.button_pos;
}

zMenu::~zMenu()
{
    m_Scene.Destroy();
    for(auto& i : m_menuActions) delete i.first;
    for(auto& i : m_menuInputs)  delete i;
    m_menuActions.clear();
    m_menuInputs.clear();
}

bool zMenu::HandleEvent(const evt::event_t& Evt)
{
    if(Evt.type == evt::EventType::MOUSE_MOTION)
    {
        for(auto& i : m_menuActions)
        {
            if(i.first->IsOver(Evt.mouse.position))
            {
                i.first->Focus();
                break;
            }
            else
            {
                i.first->Unfocus();
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

        for(auto& i : m_menuInputs)
        {
            if(i->IsOver(Evt.mouse.position))
            {
                i->Focus();
            }
            else
            {
                i->Unfocus();
            }
        }
    }

    for(auto& i : m_menuInputs)
    {
        i->HandleEvent(Evt);
    }

    return false;
}

uint16_t zMenu::AddButton(const string_t& text,
                          std::function<void(size_t)> handler)
{
    zButton* pNew = new zButton(m_Scene, m_Assets);
    pNew->SetFocusColor(m_settings.button_foccol);
    pNew->SetColor(m_settings.button_normcol);
    pNew->SetText(text);
    pNew->Create(*mp_ButtonFont);
    pNew->Place(m_settings.button_pos);
    pNew->Unfocus();

    m_Position.y += m_spacing;

    // @Griwes for this optimization.
    m_menuActions.emplace_back(pNew, std::move(handler));

    return m_menuActions.size() - 1;
}

uint16_t zMenu::AddEntryField(const string_t& label_text,
                              const vector_t& Position,
                              std::function<bool(char)> filter,
                              const string_t& prefill)
{
    zEntryField* pNew = new zEntryField(m_Scene, m_Assets);
    pNew->SetInputColor(m_settings.input_boxcol);
    pNew->SetInputTextColor(m_settings.input_col);
    pNew->SetFilterFunction(filter);
    pNew->SetColor(m_settings.button_normcol);
    pNew->SetLabel(label_text);
    pNew->SetInputStyle(m_settings.input_style);
    pNew->Create(*mp_InputFont);
    pNew->Place(Position);
    pNew->Unfocus();

    m_menuInputs.push_back(pNew);
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
    color4f_t old = mp_MenuFont->GetColor();
    mp_MenuFont->SetColor(color);
    bool val = mp_MenuFont->Render(Obj, str);
    mp_MenuFont->SetColor(old);
    return val;
}

void zMenu::Update()
{
    m_Scene.Render();
}

void zMenu::SetSpacing(const uint16_t vertical_spacing)
{
    m_spacing = vertical_spacing;
}

void zMenu::SetOverlayMode(const bool flag)
{
    m_Scene.SetSeeThrough(flag);
}

bool zMenu::LoadFont(const string_t&    font_name,
                     const uint16_t     font_size,
                     gui::zFont*&       font_ptr,
                     const uint16_t     font_def_size)
{
    util::zLog& Log = util::zLog::GetEngineLog();

    if(font_name == "")
    {
        gui::fontcfg_t f = { (font_size != 0) ? font_size : font_def_size };
        font_ptr = m_Assets.Create<gui::zFont>(font_name, nullptr, &f);

        if(font_ptr == nullptr)
        {
            LOG_ERROR("Failed to load button font.");
            return false;
        }
    }

    else
    {
        font_ptr = mp_MenuFont;
        return false;
    }

    return true;
}

#undef LOG_ERROR

