#include "Zenderer/GUI/Menu.hpp"

using namespace zen;
using namespace gui;

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
    m_Assets(Assets),
    m_Scene(Window.GetWidth(), Window.GetHeight(), Assets),
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

    if(!this->LoadFont(settings.button_f, settings.button_fs,
                       mp_ButtonFont, settings.font, f.size) ||

       !this->LoadFont(settings.label_f,  settings.label_fs,
                       mp_LabelFont,  settings.font, f.size) ||

       !this->LoadFont(settings.input_f,  settings.input_fs,
                       mp_InputFont,  settings.font, f.size))
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
                     const string_t&    font_def_name,
                     const uint16_t     font_def_size)
{
    util::zLog& Log = util::zLog::GetEngineLog();

    if(font_name == "")
    {
        gui::fontcfg_t f = { (font_size != 0) ? font_size : font_def_size };
        font_ptr = m_Assets.Create<gui::zFont>(font_def_name, nullptr, &f);

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

menucfg_t zMenu::LoadThemeFromFile(const string_t& filename)
{
    util::zFileParser Parser;
    menucfg_t theme;
    theme.valid = false;

    if(!Parser.LoadFromFile(filename)) return theme;

    #define parse(str, f)       \
        if(Parser.Exists(str))  \
        {                       \
            theme.f = Parser.GetFirstResult(str); \
        }                       \

    parse("Background", background);
    parse("Title",      title);
    parse("MenuFont",   font);
    parse("ButtonFont", button_f);
    parse("LabelFont",  label_f);
    parse("InputFont",  input_f);

    #undef parse

    theme.valid = (!theme.background.empty() && !theme.font.empty());

    #define parse(str, f)       \
        if(Parser.Exists(str))  \
        {                       \
            int s = std::stoi(Parser.GetFirstResult(str));      \
            theme.f = (s > 0) ? s : zMenu::DEFAULT_SETTINGS.f;  \
        }

    parse("MenuFontSize",   font_size);
    parse("ButtonFontSize", button_fs);
    parse("InputFontSize",  input_fs);
    parse("LabelFontSize",  label_fs);

    #undef parse
    #define parse(str, f)       \
        if(Parser.Exists(str))  \
        {                       \
            std::vector<string_t> parts = util::split(  \
                    Parser.GetFirstResult(str), ',');   \
                                                        \
            if(parts.size() != 2)                       \
                ;                                       \
                                                        \
            else                                        \
                theme.f = math::vector_t(std::stoi(parts[0]),  \
                                         std::stoi(parts[1])); \
        }

    parse("TitlePosition",  title_pos);
    parse("ButtonPosition", button_pos);

    #undef parse
    #define parse(str, f)       \
        if(Parser.Exists(str))  \
        {                       \
            std::vector<string_t> parts = util::split(      \
                    Parser.GetFirstResult(str), ',');       \
                                                            \
            if(parts.size() == 1 && parts[0][0] == '#' &&   \
               parts[0].length() == 7)                      \
            {                                               \
                std::stringstream ss;                       \
                                                            \
                ss << parts[0][1] << parts[0][2];           \
                ss >> std::hex >> theme.f.r;                \
                                                            \
                ss << parts[0][3] << parts[0][4];           \
                ss >> std::hex >> theme.f.g;                \
                                                            \
                ss << parts[0][5] << parts[0][6];           \
                ss >> std::hex >> theme.f.b;                \
            }                                               \
            else if(parts.size() != 4)                      \
                ;                                           \
                                                            \
            else                                            \
                theme.f = color4f_t(std::stoi(parts[0]),    \
                                    std::stoi(parts[1]),    \
                                    std::stoi(parts[2]),    \
                                    std::stoi(parts[3]));   \
        }

    parse("ButtonFocusCol",     button_foccol);
    parse("ButtonNormalCol",    button_normcol);
    parse("InputBoxColor",      input_boxcol);
    parse("InputTextColor",     input_col);
    parse("LabelColor",         label_col);

    #undef parse

    string_t val = Parser.GetFirstResult("InputStyle", "FILLED");

    if(val == "FILLED")
    {
        theme.input_style = InputStyle::FILLED;
    }
    else if(val == "BORDERED")
    {
        theme.input_style = InputStyle::BORDERED;
    }
    else if(val == "HIDDEN")
    {
        theme.input_style = InputStyle::HIDDEN;
    }

    return theme;
}
