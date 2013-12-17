// A C++ version of Praecursor.py

#include <ctime>
#include <cmath>

#include "Zenderer/Zenderer.hpp"
#define LEVEL_PATH      "assets/"
#define TEXTURE_PATH    "assets/textures/"
#define FONT_PATH       "assets/ttf/"

using namespace zen;

inline real_t rgb2f(const real_t rgb) { return rgb / 255.0; }

class gBullet
{
public:
    gBullet(gfx::zScene& Scene,  asset::zAssetManager& Assets,
            const real_t startx, const real_t starty,
            const real_t endx,   const real_t endy) :
        m_Scene(Scene), m_Entity(Scene.AddEntity())
    {
        m_Entity.AddPrimitive(gfx::zQuad(Assets, 4, 4).Create());
        m_Entity.Move(startx, starty);
        real_t angle = std::atan2(endy - starty, endx - startx);
        m_Rate = math::vector_t(std::cos(angle) * 5,
                                std::sin(angle) * 5,
                                0.0);
    }

    ~gBullet()
    {
        m_Scene.RemoveEntity(m_Entity);
    }

    void Update()
    {
        m_Entity.Adjust(m_Rate);
    }

    void SetRate(const real_t dx, const real_t dy)
    {
        m_Rate.x = dx;
        m_Rate.y = dy;
    }

    inline const math::vector_t& GetRate() const
    {
        return m_Rate;
    }

    inline real_t GetX() const { return m_Entity.GetX(); }
    inline real_t GetY() const { return m_Entity.GetY(); }

    template<typename T> inline
    bool Collides(const T& t, math::cquery_t* poi = nullptr) const
    { return m_Entity.Collides(t, poi); }

private:
    gfx::zScene&    m_Scene;
    obj::zEntity&   m_Entity;
    math::vector_t  m_Rate;
};

class gWorld
{
    static real_t s_GRAVITY;
    struct trail_t
    {
        trail_t(const evt::event_t& event, real_t delay = 0.5) :
            m_start(time(nullptr) + delay),
            m_evt(event) {}

        inline bool Ready() const
        { return m_start <= time(nullptr); }

        real_t       m_start;
        evt::event_t m_evt;
    };

public:
    gWorld(const gfx::zWindow& W, asset::zAssetManager& Assets,
           gfx::zScene& Scene) :

        m_Window(W), m_Assets(Assets), m_Scene(Scene),

        m_Trail(Scene.AddEntity()),
        m_Player(Scene.AddAnimation()),

        m_TDelta(0, 0, 0), m_PDelta(0, 0, 0),
        m_plat(-1.0, 0.0), m_tdelay(0.5), m_tshift(0.0)
    {
        m_Player.SetKeyframeSize(24, 52);
        m_Player.SetKeyframeCount(4);
        m_Player.SetKeyframeRate(10);
        m_Player.LoadFromTexture(TEXTURE_PATH"hero_walk.png");
        m_Player.StopAnimation(0);

        gfx::zQuad Q(Assets, 32, 32);
        Q.SetColor(0, 0, rgb2f(100));
        m_Trail.AddPrimitive(std::move(Q.Create()));

        gui::fontcfg_t f = { 16 };
        mp_Font = m_Assets.Create<gui::zFont>(FONT_PATH"game.ttf", nullptr, &f);

        if (mp_Font == nullptr)
        {
            util::zLog& Log = util::zLog::GetEngineLog();
            Log << Log.SetMode(util::LogMode::ZEN_FATAL)
                << Log.SetSystem("World") << "Failed to load '"
                << FONT_PATH"game.ttf" << "'." << util::zLog::endl;
        }
    }

    ~gWorld() {}

    bool LoadLevel(lvl::level_t level)
    {
        if (!level.valid) return false;

        m_Level = std::move(level);
        m_Player.Move(std::find_if(m_Level.spawnpoints.begin(),
                                   m_Level.spawnpoints.end(),
            [](const lvl::spawn_t& point) {
                return point.type == lvl::SpawnType::PLAYER_SPAWN;
            }
        )->position);

        m_Trail.Move(m_Player.GetPosition());
        return m_Level.valid;
    }

    void Update()
    {
        if (!math::compf(m_PDelta.x, 0.0) && !m_Player.Playing())
        {
            m_Player.PlayAnimation(0);
        }
        else if (math::compf(m_PDelta.x, 0.0) && m_Player.Playing())
        {
            m_Player.StopAnimation(0);
        }

        if (!m_allTrails.empty() && m_allTrails.front().Ready())
        {
            const trail_t latest = m_allTrails.front();
            this->HandleEvent_Default(latest.m_evt, m_Trail, m_TDelta);
            m_allTrails.erase(m_allTrails.begin());
        }

        this->HandleGravity(m_Player, m_PDelta);
        this->HandleGravity(m_Trail, m_TDelta);

        if (!math::compf(m_PDelta.x, 0.0))
        {
            for (auto& i : m_Level.physical)
            {
                math::cquery_t poi;
                i->Adjust(-m_PDelta.x, 5);
                i->Collides(m_Player, &poi);
                i->Adjust(m_PDelta.x, -5);

                if (poi.collision)
                {
                    real_t m = (poi.line1[0].y - poi.line1[1].y) /
                               (poi.line1[0].x - poi.line1[1].x);

                    if (m < -1 && m_PDelta.x > 0 ||
                        m >  1 && m_PDelta.x < 0)
                        m_PDelta.x = 0;

                    else
                    {
                        // Not going down a hill?
                        /*if(m_PDelta.x < 0 == m < 0)
                        {
                            // Hamper speed.
                            m_PDelta.x *= m;
                        }*/

                        m_PDelta.y = m_PDelta.x * m;
                        m_Player.Adjust(0, -1);
                    }
                    return;
                }
            }
        }

        m_Player.Adjust(m_PDelta);
        m_Trail .Adjust(m_TDelta);

        if (math::compf(m_PDelta.x, 0.0) && math::compf(m_PDelta.y, 0.0))
        {
            m_tdelay = 1.0;//math::min<real_t>(5.0, 100.0 / m_Light.GetPosition().distance(m_Player.GetPosition()));
        }

        if (math::compf(m_TDelta.x, 0.0) && m_allTrails.empty() &&
           !math::compf(m_Trail.GetX(), m_Player.GetX()))
        {
            bool match = m_Player.GetX() > m_Trail.GetX() && m_tshift < 0;

            if (match)
            {
                m_Trail.Move(m_Player.GetPosition());
                m_tshift = 0.0;
            }

            else
            {
                m_tshift += (m_Player.GetX() > m_Trail.GetX()) ? 0.02  : -0.02;
            }
        }

        else
        {
            m_tshift = 0.0;
        }

        /*
        if (m_Trail.Collides(m_LineOfSight))
        {
            gBullet* pBullet = new gBullet(m_Scene, m_Target.GetX(), m_Target.GetY(),
                                           m_Trail.GetX(), m_Trail.GetY());
            mp_allBullets.push_back(pBullet);
        }
        */
        m_Trail.Adjust(m_tshift, 0.0);

        obj::zEntity* bg = lvl::zLevelLoader::GetEntityByID(m_Level, "background");
        if (bg != nullptr) bg->Adjust(-1, 0);
    }

    void Render()
    {
        mp_allBullets.erase(
            std::remove_if(mp_allBullets.begin(), mp_allBullets.end(),
                           [this](const gBullet* const i) {
                return (i->GetX() > this->m_Scene.GetWidth()  || i->GetX() < 0.0 ||
                        i->GetY() > this->m_Scene.GetHeight() || i->GetY() < 0.0);
            }),
            mp_allBullets.end()
        );

        math::cquery_t q;
        for (auto& i : mp_allBullets)
        {
            bool g = true;
            for (auto& j : m_Level.physical)
            {
                if (i->Collides(*j, &q))
                {
                    math::vector_t rate = i->GetRate();
                    math::vector_t surf = q.line2[1] - q.line2[0];
                    real_t theta = std::acos(rate * surf /
                                            (rate.Magnitude() * surf.Magnitude()));
                    rate.Rotate(-2 * theta);
                    i->SetRate(rate.x, rate.y);
                    g = false;
                }
            }

            if (g && !math::compf(i->GetRate().x, 0.0))
            {
                i->SetRate(i->GetRate().x * 0.999,
                           i->GetRate().y + (gWorld::s_GRAVITY / 50));
            }

            i->Update();
        }

        m_Scene.Render();
        /*
        uint16_t y = m_Scene.GetHeight() - 200;//m_Ground.GetHeight();
        obj::zEntity Line(m_Assets);
        gui::zFont& Font = *mp_Font;

        for (auto& i : m_allTrails)
        {
            Font << static_cast<int>(i.m_evt.type)    << " ["
                 << static_cast<int>(i.m_evt.key.key) << "] -- "
                 << (i.m_start - time(nullptr));
            Font.Render(Line);
            Font.ClearString();

            Line.Move(0, y);
            Line.Draw();

            y += Line.GetHeight();
        }
        */
    }

    void HandleEvent(const evt::event_t& Evt)
    {
        this->HandleEvent_Default(Evt, m_Player, m_PDelta);
        if (Evt.type == evt::EventType::KEY_DOWN     ||
            Evt.type == evt::EventType::KEY_UP       ||
            Evt.type == evt::EventType::MOUSE_DOWN)
        {
            m_allTrails.emplace_back(Evt, m_tdelay);
        }

        if (Evt.type == evt::EventType::KEY_DOWN &&
            Evt.key.key == evt::Key::U)
        {
            m_TDelta = math::vector_t();
            m_Trail.Move(m_Player.GetPosition());
            m_allTrails.clear();
        }
    }

private:
    void HandleEvent_Default(const evt::event_t& Evt, const obj::zEntity& Object, math::vector_t& Rate)
    {
        switch (Evt.type)
        {
        case evt::EventType::KEY_DOWN:
            switch (Evt.key.key)
            {
            case evt::Key::W:
            case evt::Key::SPACE:
                Rate.y = (!math::compf(Rate.y, 0.0)) ? Rate.y : -8.0;
                break;

            case evt::Key::D:
            case evt::Key::RIGHT:
                Rate.x = 5;
                break;

            case evt::Key::A:
            case evt::Key::LEFT:
                Rate.x = -5;
                break;
            }

            break;

        case evt::EventType::KEY_UP:
            switch (Evt.key.key)
            {
            case evt::Key::D:
            case evt::Key::LEFT:
            case evt::Key::A:
            case evt::Key::RIGHT:
                Rate.x = 0;
                break;
            }

            break;

        case evt::EventType::MOUSE_DOWN:
        {
            gBullet* pBullet = new gBullet(m_Scene, m_Assets,
                                           Object.GetX(), Object.GetY(),
                                           Evt.mouse.position.x,
                                           Evt.mouse.position.y);
            mp_allBullets.push_back(pBullet);
            break;
        }
        }
    }

    void HandleGravity(const obj::zEntity& Object, math::vector_t& Rate)
    {
        bool ok = false;
        int tmp = static_cast<int>(Rate.y);

        for (auto& i : m_Level.physical)
        {
            i->Adjust(0, -tmp);
            ok = Object.Collides(*i);
            i->Adjust(0, tmp);
            if (ok) break;
        }

        if (ok) Rate.y = 0.0;
        else Rate.y += gWorld::s_GRAVITY;
    }

    asset::zAssetManager&   m_Assets;
    const gfx::zWindow&     m_Window;
    gfx::zScene&            m_Scene;

    obj::zAnimation&        m_Player;
    obj::zEntity&           m_Trail;

    lvl::level_t  m_Level;

    gui::zFont*   mp_Font;

    std::vector<trail_t>    m_allTrails;
    std::list<gBullet*>     mp_allBullets;

    math::vector_t  m_PDelta, m_TDelta;
    math::vector_t  m_plat;
    real_t          m_tdelay,
                    m_tshift;
};

real_t gWorld::s_GRAVITY = 0.5;

#define CHECK(f) { if (!f) return 1; }

int main_uitest()
{
    // Boilerplate setup code.
    // Error-checking omitted for brevity.
    Init();

    asset::zAssetManager GameAssets;
    GameAssets.Init();

    gfx::zWindow Window(800, 600, "Test Window", GameAssets, false);
    Window.Init();

    // We may want fancy effects later.
    gfx::zScene Pong(Window.GetWidth(), Window.GetHeight(), GameAssets);
    Pong.Init();

    // Create the empty in-game objects.
    obj::zEntity& Player = Pong.AddEntity();
    obj::zEntity& Opponent = Pong.AddEntity();
    obj::zEntity& Ball = Pong.AddEntity();

    // Create a 64x8 paddle primitive.
    gfx::zQuad Paddle(GameAssets, 8, 64);

    // The player will be on the left and have a blue paddle.
    Paddle.SetColor(color4f_t(0.0, 0.0, 1.0));
    Player.AddPrimitive(Paddle.Create());
    Player.Move(0, Window.GetHeight() / 2 - Player.GetHeight() / 2);

    // We can easily just re-use the old primitive, since it still
    // contains the relevant shape data. AddPrimitive() makes a copy
    // internally, and we never offloaded things to the GPU.

    // The opponent will be on the right and have a red paddle.
    // We don't need the primitive after this anymore, so we can
    // move it directly.
    Paddle.SetColor(color4f_t(1.0, 0.0, 0.0));
    Opponent.AddPrimitive(std::move(Paddle.Create()));
    Opponent.Move(Window.GetWidth() - Opponent.GetWidth(),
                  Window.GetHeight() / 2 - Opponent.GetHeight() / 2);

    // The ball will be a white hexagon in the center of the screen.
    gfx::zPolygon Hex(GameAssets, 6);
    math::vector_t Center(Window.GetWidth() / 2, Window.GetHeight() / 2);
    Hex.AddVertex(Center);
    for (uint8_t i = 0; i <= 6; ++i)
    {
        // Hexagon has 6 sides with an angle of 60 degrees (pi/3).
        // We can use the engine's built-in tools if we didn't know that,
        // which is shown for the y-value.
        math::vector_t point(15 * std::cos(i * math::rad(60)),
                             15 * std::sin(i * math::PI / 3));

        Hex.AddVertex(Center + point);
    }
    Ball.AddPrimitive(std::move(Hex.Create()));

    // Our pause flag.
    bool pause = false;

    // Create an overlay menu.
    gui::menucfg_t m;
    m.font = FONT_PATH"menu.ttf";
    m.button_foccol  = color4f_t(1.0, 0.0, 0.0);
    m.button_normcol = color4f_t(0.0, 1.0, 0.0);
    m.button_pos = math::vector_t(Window.GetWidth() - 80, 0);

    gui::zMenu UI(Window, GameAssets, m);

    UI.SetOverlayMode(true);
    UI.AddButton("Pause Game", [&pause](size_t){ pause = !pause; });

    // Frame rate regulator.
    util::zTimer Frames(60);

    // Within our game loop, we can do the following to catch button events.
    // Poll events and pass all to the menu.
    while (Window.IsOpen())
    {
        Frames.Start();

        // Poll events and handle mouse clicks on the button if needed.
        evt::event_t Evt;
        evt::zEventHandler::PollEvents();
        while(evt::zEventHandler::GetInstance().PopEvent(Evt))
        {
            if (Evt.type == evt::EventType::WINDOW_CLOSE)
                Window.Close();

            UI.HandleEvent(Evt);
        }

        // Remainder of game loop.
        Window.Clear();
        if (!pause)
        {
            Pong.Render(color4f_t(0.1, 0.1, 0.1));
        }
        UI.Update();
        Window.Update();

        // Regulate frame rate.
        Frames.Delay();
    }

    Quit();
    return 0;
}

static const uint8_t BRUSH_SIZE = 10;
int main_paint()
{
    Init();

    asset::zAssetManager PaintAssets;
    gfx::zWindow PaintWindow(800, 600, "Paint", PaintAssets, false);

    PaintWindow.Init();

    gfx::zScene Paint(PaintWindow.GetWidth(),
                      PaintWindow.GetHeight(),
                      PaintAssets);
    Paint.Init();

    const uint16_t FIDELITY = 45;
    gfx::zPolygon BrushPrim(PaintAssets, FIDELITY+1);
    for (uint16_t i = 0; i < FIDELITY; ++i)
    {
        BrushPrim.AddVertex(BRUSH_SIZE * std::sin(math::rad(i*360/FIDELITY)),
                            BRUSH_SIZE * std::cos(math::rad(i*360/FIDELITY)));
    }
    BrushPrim.SetColor(color4f_t(0, 0, 0));

    obj::zEntity& Brush = Paint.AddEntity();
    Brush.AddPrimitive(BrushPrim.Create());

    evt::zEventHandler& Evts = evt::zEventHandler::GetInstance();
    evt::event_t Evt;
    bool quit = false;

    std::vector<gfx::zPolygon*> allInstances;
    gfxcore::zVertexArray One(GL_DYNAMIC_DRAW); One.Init();

    while (!quit)
    {
        Evts.PollEvents();
        while (Evts.PopEvent(Evt))
        {
            if (Evt.type == evt::EventType::WINDOW_CLOSE)
                quit = true;

            if (GetMouseState(evt::MouseButton::LEFT) &&
                Evt.type == evt::EventType::MOUSE_MOTION)
            {
                if (std::find_if(allInstances.begin(), allInstances.end(),
                    [&Evt](const gfx::zPolygon* Obj) {
                        return Obj->GetPosition() == Evt.mouse.position;
                    }) == allInstances.end())
                {
                    gfx::zPolygon* tmp = new gfx::zPolygon(BrushPrim);
                    allInstances.push_back(tmp);
                    tmp->Move(Evt.mouse.position);
                    tmp->LoadIntoVAO(One, gfx::VAOState::EMBED_TRANSFORM |
                                          gfx::VAOState::NO_PRESERVE_DATA);
                    One.Offload();
                }
            }
        }

        Brush.Move(evt::GetMousePosition());

        PaintWindow.Clear();

        Paint.Render(color4f_t());

        const gfx::zEffect& E = gfxcore::zRenderer::GetDefaultEffect();
        E.Enable();
        E.SetParameter("proj", gfxcore::zRenderer::GetProjectionMatrix());
        E.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
        One.Bind();

        for (auto& i : allInstances)
        {
            i->Draw(gfx::RenderState::READY);
        }

        One.Unbind();
        E.Disable();

        PaintWindow.Update();
    }

    Quit();
    return 0;
}

// Shader toy.
#include "Zenderer/GUI/EntryField.hpp"

int main()
{
    enum class State
    {
        MAIN_VIEW,
        EDIT_BRT,
        EDIT_ATT,
        EDIT_ANG,
        QUIT
    } ToolState = State::MAIN_VIEW;

    Init();

    asset::zAssetManager Assets;
    gfx::zWindow Window(800, 600, "Lighting Toy", Assets, false);
    Window.Init();

    gfx::zScene Scene(Window.GetWidth(), Window.GetHeight(), Assets);
    Scene.Init(); Scene.EnableLighting();

    gfx::zLight& Light = Scene.AddLight(gfx::LightType::ZEN_SPOTLIGHT);
    Light.Enable();
    Light.SetPosition(Window.GetWidth() / 2, Window.GetHeight() / 2);
    Light.SetBrightness(1.0);
    Light.SetMaximumAngle(0);
    Light.SetMinimumAngle(359);
    Light.Disable();

    gui::menucfg_t cfg = gui::zMenu::DEFAULT_SETTINGS;/*
    cfg.font = "C:\\Windows\\Fonts\\Arial.ttf";
    cfg.font_size = 22;
    cfg.button_pos = math::vector_t(32, 256);
    cfg.button_foccol = color4f_t();
    cfg.button_normcol = color4f_t(.8, .8, .8);
    cfg.label_col = color4f_t();
    cfg.valid = true;*/
    cfg = gui::zMenu::LoadThemeFromFile("basetheme.gui");
    //cfg.button_normcol = color4f_t();
    ZEN_ASSERT(cfg.valid);
    gui::zMenu StartMenu(Window, Assets, cfg);
    StartMenu.SetOverlayMode(true);
    StartMenu.AddEntryField("Brightness: ", math::vector_t(10, 0),
                            [&Light](const string_t& t) {
        if (!t.empty() && std::stod(t) > 0)
        {
            Light.Enable();
            Light.SetBrightness(std::stod(t));
            Light.Disable();
        }
    }, [](char c) -> bool {
        std::stringstream ss;
        ss << c;
        real_t o = -1.0;
        return (ss >> o) || c == '.';
    }, "1.0");
    StartMenu.AddEntryField("Attenuation: ",   math::vector_t(10, 50),
                            [](const string_t&){});
    StartMenu.AddEntryField("Maximum Angle: ", math::vector_t(10, 100),
                            [&Light](const string_t& t) {
        if (!t.empty())
        {
            Light.Enable();
            Light.SetMaximumAngle(std::stod(t));
            Light.Disable();
        }
    });
    StartMenu.AddEntryField("Minumum Angle: ", math::vector_t(10, 150),
                            [&Light](const string_t& t) {
        if (!t.empty())
        {
            Light.Enable();
            Light.SetMinimumAngle(std::stod(t));
            Light.Disable();
        }
    });

    StartMenu.AddButton("I am a highlighted button AMA", [](size_t){});
    evt::zEventHandler& Evts = evt::zEventHandler::GetInstance();
    evt::event_t Evt;
    bool quit = false;

    while(ToolState != State::QUIT)
    {
        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::WINDOW_CLOSE)
                ToolState = State::QUIT;

            StartMenu.HandleEvent(Evt);
        }

        Window.Clear();
        Scene.Render(color4f_t(0.1, 0.1, 0.1));
        //UI.Render();
        StartMenu.Update();
        Window.Update();
    }

    Quit();
    return 0;
}

struct ray_t
{
    math::vector_t pos;
    bool edge;
};

int main_shadows()
{
    using namespace gfx;
    using gfxcore::zRenderer;
    using gfxcore::BlendFunc;

    Init();

    util::zLog::GetEngineLog().ToggleStdout();

    asset::zAssetManager Assets;
    gfx::zWindow Window(800, 600, "Shadow Test", Assets, false);
    Window.Init();

    evt::zEventHandler& Evts = evt::zEventHandler::GetInstance();
    evt::event_t Evt;
    bool quit = false;

    math::vector_t LPos(Window.GetWidth() / 2, Window.GetHeight() / 2);
    zQuad Light(Assets, 32, 32);
    Light.SetColor(color4f_t()).Create().Move(LPos);

    /*zConcavePolygon Caster(Assets, 16);
    Caster.AddVertex(0, 0).AddVertex(111, 39).AddVertex(161, 40)
        .AddVertex(217, 53).AddVertex(274, 53).AddVertex(323, 55)
        .AddVertex(497, -27).AddVertex(583, -23).AddVertex(589, -4)
        .AddVertex(581, 32).AddVertex(482, 104).AddVertex(450, 112)
        .AddVertex(446, 134).AddVertex(478, 144).AddVertex(596, 147)
        .AddVertex(728, 145).AddVertex(752, 121).AddVertex(757, 88)
        .AddVertex(779, 65).AddVertex(834, 73).AddVertex(880, 92)
        .AddVertex(927, 92).AddVertex(957, 84).AddVertex(974, 61)
        .AddVertex(892, 41).AddVertex(903, -7).AddVertex(917, -12)
        .AddVertex(1002, -16).AddVertex(1211, 46).AddVertex(1287, 49)
        .AddVertex(1332, 69).AddVertex(1505, 72).AddVertex(1547, 31)
        .AddVertex(1577, 28).AddVertex(1572, 164).AddVertex(959, 198)
        .AddVertex(325, 205).AddVertex(-25, 208).AddVertex(-21, 24)
        .AddVertex(-4, 3);
    Caster.Move(0, 350);
    Caster.SetColor(color4f_t(1, 0, 0));
    Caster.Create();*/
    zQuad Caster(Assets, Window.GetWidth() / 4, 64);
    Caster.SetColor(color4f_t(1, 0, 0, 1)).Create()
          .Move(Window.GetWidth() - 300,
                Light.GetPosition().y);

    zRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);

    gui::fontcfg_t s { 12 };
    gui::zFont& Font = *Assets.Create<gui::zFont>("assets/ttf/game.ttf",
                                                  nullptr, &s);
    Font.SetColor(1, 1, 0);

    gfx::zRenderTarget ShadowMap(Window.GetWidth(), Window.GetHeight());
    ZEN_ASSERT(ShadowMap.Init());
    ShadowMap.Unbind();

    while(!quit)
    {
        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::WINDOW_CLOSE)
                quit = true;

            else if(Evt.type == evt::EventType::KEY_DOWN &&
                    Evt.key.key == evt::Key::Z)
                zRenderer::ToggleWireframe();

            else if(Evt.type == evt::EventType::KEY_DOWN &&
                    Evt.key.key == evt::Key::W)
                Caster.Move(Caster.GetX(), Caster.GetY() + 1);

            else if(Evt.type == evt::EventType::KEY_DOWN &&
                    Evt.key.key == evt::Key::S)
                Caster.Move(Caster.GetX(), Caster.GetY() - 1);

            else if(Evt.type == evt::EventType::KEY_HOLD &&
                    Evt.key.key == evt::Key::W)
                    Caster.Move(Caster.GetX(), Caster.GetY() + 5);

            else if(Evt.type == evt::EventType::KEY_HOLD &&
                    Evt.key.key == evt::Key::S)
                    Caster.Move(Caster.GetX(), Caster.GetY() - 5);
        }

        Window.Clear();

        //Light.Move(evt::GetMousePosition());
        Caster.Move(evt::GetMousePosition());
        LPos = Light.GetPosition();
        Caster.Draw();
        Light.Draw();

        auto& tris = Caster.GetTriangulation();
        std::vector<math::vector_t> edges;

        for(const auto& i : tris)
        {
            if(std::find(edges.begin(), edges.end(), i) == edges.end())
            {
                const int LENGTH = 500;

                real_t m = math::slope(math::line_t { { LPos, i } });
                math::vector_t ext = i;

                if(math::compf(i.x, LPos.x) || std::isnan(m))
                {
                    ext.y += (ext.y > LPos.y) ? LENGTH * 2 : LENGTH * -2;
                }
                else if(i.x > LPos.x)
                {
                    ext.x += LENGTH;
                    ext.y += LENGTH * m;
                }
                else if(i.x < LPos.x)
                {
                    ext.x -= LENGTH;
                    ext.y -= LENGTH * m;
                }

                edges.push_back(i);
                edges.push_back(ext);
            }
        }

        std::function<bool(const math::vector_t&, const math::vector_t&)>
            alt_sort = [&LPos](const math::vector_t& a,
                                const math::vector_t& b) {
            const math::vector_t& l = LPos;
            real_t theta = std::atan2(l.y - a.y, l.x - a.x);
            real_t phi = std::atan2(l.y - b.y, l.x - b.x);

            if(math::compf(theta, phi))
            {
                return math::distance(l, a, false) <
                       math::distance(l, b, false);
            }

            else if(theta < 0 != phi < 0) return theta > 0;

            return theta < phi;
        };

        std::function<bool(const math::vector_t&, const math::vector_t&)>
            reg_sort = [&LPos](const math::vector_t& a,
                                const math::vector_t& b) {
            const math::vector_t& l = LPos;
            real_t theta = std::atan2(l.y - a.y, l.x - a.x);
            real_t phi = std::atan2(l.y - b.y, l.x - b.x);

            if(math::compf(theta, phi))
            {
                return math::distance(l, a, false) <
                       math::distance(l, b, false);
            }

            return theta < phi;
        };

        std::sort(edges.begin(), edges.end(),
                  (LPos.y > Caster.GetY() &&
                   LPos.y < Caster.GetY() + Caster.GetHeight() &&
                   LPos.x < Caster.GetX()) ?
                  alt_sort : reg_sort);

        for(size_t index = 1; index < edges.size(); index += 2)
        {
            auto& e = edges[index];

            std::vector<math::vector_t> collisions;
            math::line_t ray { { LPos, e } };

            for(size_t i = 0; i < tris.size(); i += 3)
            {
                math::line_t one { { tris[i],       tris[i + 1] } };
                math::line_t two { { tris[i + 1],   tris[i + 2] } };
                math::line_t thr { { tris[i],       tris[i + 2] } };

                math::cquery_t q1, q2, q3;
                if(math::collides(ray, one, &q1))
                {
                    collisions.push_back(std::move(q1.point));
                    gfx::zPolygon P(Assets, 5);
                    P.AddVertex(q1.point);
                    for(size_t i = 0; i < 36; ++i)
                    {
                        P.AddVertex(q1.point +
                                    math::vector_t(
                                        std::cos(math::rad(i / 360)),
                                        std::sin(math::rad(i / 360))
                                    ) * 10);
                    }

                    P.SetColor(0, 1, 0, 1).Create(false).Draw();
                }

                if(math::collides(ray, two, &q2))
                {
                    collisions.push_back(std::move(q2.point));
                    gfx::zPolygon P(Assets, 5);
                    P.AddVertex(q1.point);
                    for(size_t i = 0; i < 36; ++i)
                    {
                        P.AddVertex(q1.point +
                                    math::vector_t(
                                        std::cos(math::rad(i / 360)),
                                        std::sin(math::rad(i / 360))
                                    ) * 10);
                    }

                    P.SetColor(0, 1, 0, 1).Create(false).Draw();
                }

                if(math::collides(ray, thr, &q3))
                {
                    collisions.push_back(std::move(q3.point));
                    gfx::zPolygon P(Assets, 5);
                    P.AddVertex(q1.point);
                    for(size_t i = 0; i < 36; ++i)
                    {
                        P.AddVertex(q1.point +
                                    math::vector_t(
                                        std::cos(math::rad(i / 360)),
                                        std::sin(math::rad(i / 360))
                                    ) * 10);
                    }

                    P.SetColor(0, 1, 0, 1).Create(false).Draw();
                }
            }

            math::vector_t& comp = edges[index - 1];
            for(size_t i = 0; i < collisions.size(); ++i)
            {
                if(math::distance(LPos, collisions[i], false) >
                   math::distance(LPos, comp, false))
                {
                    comp = collisions[i];
                }
            }
        }
        /*
        std::map<real_t, int> angles;
        std::unique(edges.begin(), edges.end(),
                    [&LPos, &angles](const math::vector_t& a,
                                     const math::vector_t& b) {
            real_t ta = std::atan2(LPos.y - a.y, LPos.x - a.x);
            real_t tb = std::atan2(LPos.y - b.y, LPos.x - b.x);

            if(math::compf(ta, tb))
            {
                auto it = angles.find(ta);
                if(it == angles.end())
                {
                    angles[ta] = 1;
                    return false;
                }
                else
                {
                    ++(it->second);
                    return (it->second) >= 3;
                }
            }

            return false;
        });*/

        for(auto& i : edges)
        {
            std::cout << i << std::endl;
        }

        std::cout << std::endl;

        gfx::zConcavePolygon ShadowGeo(Assets, edges.size() * 3);
        for(size_t i = 0; i < edges.size() - 2; ++i)
        {
            ShadowGeo.AddVertex(edges[i]);
            ShadowGeo.AddVertex(edges[i + 1]);
            ShadowGeo.AddVertex(edges[i + 2]);
        }

        ShadowMap.Bind();
        ShadowGeo.SetColor(1, 1, 1, 1).Create(false).Draw();
        ShadowMap.Unbind();
        ShadowGeo.Draw();

        obj::zEntity MousePos(Assets);
        std::stringstream ss;
        ss << '(' << evt::GetMousePosition().x << ", "
                  << evt::GetMousePosition().y << ')';
        Font.Render(MousePos, ss.str());
        MousePos.Move(evt::GetMousePosition());
        MousePos.Draw();

        /*
        std::vector<ray_t> rays;
        zConcavePolygon ShadowMap(Assets);

        auto& tris = Caster.GetTriangulation();

        // Cast a ray from the light to each individual edge on the
        // collidable objects. If the ray collides with more than one edge,
        // discard it. Otherwise, extend it out for an arbitrary amount.
        /*
        for(auto& edge : tris)
        {
            std::vector<math::vector_t> collisions;
            math::line_t ray { { LPos, edge } };

            if(std::find(rays.begin(), rays.end(), edge) != rays.end())
                continue;

            for(size_t i = 0; i < tris.size(); i += 3)
            {
                math::line_t one { { tris[i],       tris[i + 1] } };
                math::line_t two { { tris[i + 1],   tris[i + 2] } };
                math::line_t thr { { tris[i],       tris[i + 2] } };

                math::cquery_t q1, q2, q3;
                if(math::collides(ray, one, &q1) &&
                   std::find(collisions.begin(), collisions.end(),
                             q1.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q1.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q1.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q1.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q1.point));
                }

                if(math::collides(ray, two, &q2) &&
                   std::find(collisions.begin(), collisions.end(),
                             q2.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q2.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q2.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q2.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q2.point));
                }

                if(math::collides(ray, thr, &q3) &&
                   std::find(collisions.begin(), collisions.end(),
                             q3.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q3.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q3.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q3.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q3.point));
                }
            }

            real_t m = math::slope(ray);
            if(collisions.size() <= 1 || std::isnan(m))
            {
                rays.push_back(edge);
                auto tmp = edge;

                int factor = LPos.x > edge.x ? -1000 : 1000;

                if(std::isnan(m))
                {
                    m = LPos.y > edge.y ? -1000 : 1000;;
                    factor = 1;
                }

                ray[1].x += factor;
                ray[1].y += factor * m;

                rays.push_back(ray[1]);
                //rays.push_back(tmp);
            }
        }
        *
        rays.reserve(tris.size() + 4);
        for(auto& i : tris)
        {
            bool exists = false;
            for(auto& j : rays)
            {
                if(j.pos == i)
                {
                    exists = true;
                    break;
                }
            }
            if(!exists) rays.push_back(ray_t { i, true });
        }

        rays.push_back(ray_t { math::vector_t(0, 0), false });
        rays.push_back(ray_t { math::vector_t(Window.GetWidth(), 0), false });
        rays.push_back(ray_t { math::vector_t(Window.GetWidth(), Window.GetHeight()), false });
        rays.push_back(ray_t { math::vector_t(0, Window.GetHeight()), false });

        // Cast a ray from the light to each individual edge on the
        // collidable objects. If the ray collides with more than one edge,
        // discard it. Otherwise, extend it out for an arbitrary amount.
        auto it = rays.begin();
        //std::advance(it, rays.size() - 4);
        for( ; it != rays.end(); /* no third /)
        {
            const auto& edge = (*it).pos;
            std::vector<math::vector_t> collisions;
            math::line_t ray { { LPos, edge } };

            for(size_t i = 0; i < tris.size(); i += 3)
            {
                math::line_t one { { tris[i],       tris[i + 1] } };
                math::line_t two { { tris[i + 1],   tris[i + 2] } };
                math::line_t thr { { tris[i],       tris[i + 2] } };

                math::cquery_t q1, q2, q3;
                if(math::collides(ray, one, &q1) &&
                   std::find(collisions.begin(), collisions.end(),
                             q1.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q1.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q1.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q1.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q1.point));
                }

                if(math::collides(ray, two, &q2) &&
                   std::find(collisions.begin(), collisions.end(),
                             q2.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q2.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q2.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q2.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q2.point));
                }

                if(math::collides(ray, thr, &q3) &&
                   std::find(collisions.begin(), collisions.end(),
                             q3.point) == collisions.end())
                {
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(q3.point);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(q3.point.x + 5 * std::cos(math::rad(i / 360)),
                                            q3.point.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    Collision.Create().Draw();
                    collisions.push_back(std::move(q3.point));
                }
            }

            if(collisions.size() > 1)
            {
                it = rays.erase(it);
            }
            else
            {
                ++it;
            }
        }
        std::unique(rays.begin(), rays.end(),
                    [](const ray_t& a, const ray_t& b) {
            return a.pos == b.pos;
        });
        std::sort(rays.begin(), rays.end(),
                  [&LPos](const ray_t& ra,
                          const ray_t& rb) {
            const auto& a = ra.pos;
            const auto& b = rb.pos;
            const math::vector_t& l = LPos;
            real_t theta = std::atan2(l.y - a.y, l.x - a.x);
            real_t phi   = std::atan2(l.y - b.y, l.x - b.x);

            if(math::compf(theta, phi))
            {
                return a.x < LPos.x ?
                    math::distance(l, a, false) >
                    math::distance(l, b, false) :
                    math::distance(l, a, false) <
                    math::distance(l, b, false);
            }
            else if(theta < 0 == phi < 0) return theta < phi;
            return theta < phi;
        });

        for(auto& i : rays)
        {
            std::cout << math::deg(std::atan2(LPos.y - i.pos.y, LPos.x - i.pos.x)) << std::endl;
            std::cout << "End: " << i.pos << std::endl;
        }
        std::cout << std::endl;

        for(size_t i = 0; i < rays.size() - 1; ++i)
        {
            ShadowMap.AddVertex(LPos);
            ShadowMap.AddVertex(rays[i].pos);
            ShadowMap.AddVertex(rays[i + 1].pos);
        }
        ShadowMap.AddVertex(LPos);
        ShadowMap.AddVertex(rays.front().pos);
        ShadowMap.AddVertex(rays.back().pos);

        ShadowMap.SetColor(0, 1, 0, 0.3).Create(false).Draw();

        ////////////////////////////////////////////////////////////////////////

        //LPos = Light.GetPosition();
        //Light.Move(evt::GetMousePosition());
        //Caster.Move(evt::GetMousePosition());
        Caster.Draw();
        Light.Draw();

        std::vector<real_t> angles;
        zPolygon ShadowMap(Assets);

        auto& tris = Caster.GetTriangulation();
        angles.reserve(4 + tris.size());

        for(auto& edge : tris)
        {
            angles.push_back(std::atan2(edge.y - LPos.y, edge.x - LPos.x));
        }

        angles.push_back(std::atan2(    - LPos.y,     - LPos.x));
        angles.push_back(std::atan2(600 - LPos.y,     - LPos.x));
        angles.push_back(std::atan2(600 - LPos.y, 800 - LPos.x));
        angles.push_back(std::atan2(    - LPos.y, 800 - LPos.x));

        for(auto& d : angles) d = math::deg(d);
        std::unique(angles.begin(), angles.end());
        std::sort(angles.begin(), angles.end(),
                  [](const real_t a, const real_t b) {
            if(a < 0 != b < 0) return a > b;
            return a < b;
        });

        real_t maxlen = 400;

        math::vector_t first;
        ShadowMap.AddVertex(LPos);
        for(auto& d : angles)
        {
            math::line_t line { { LPos,
                LPos + math::vector_t(maxlen * std::cos(math::rad(d)),
                                      maxlen * std::sin(math::rad(d)))
            } };

            math::vector_t end = line[1]; end.z = 1;
            for(size_t i = 0; i < tris.size(); i += 3)
            {
                math::cquery_t query;
                math::tri_t tri { { tris[i], tris[i + 1], tris[i + 2] } };
                query.collision = math::collides(line, tri, &query);
                if(query.collision &&
                   math::distance(LPos, query.point, true) <
                   math::distance(LPos, end, true))
                {
                    end = query.point;
                    zPolygon Collision(Assets, 5);
                    Collision.AddVertex(end);
                    for(size_t i = 0; i < 5; ++i)
                    {
                        Collision.AddVertex(end.x + 5 * std::cos(math::rad(i / 360)),
                                            end.y + 5 * std::sin(math::rad(i / 360)));
                    }
                    Collision.SetColor(color4f_t(0, 0, 1, 1));
                    if(query.edge_case) Collision.SetColor(color4f_t(0, 1, 1, 1));
                    Collision.Create().Draw();
                }
            }

            ShadowMap.AddVertex(end);
            if(d == angles.front()) first = end;
        }

        ShadowMap.AddVertex(first);
        ShadowMap.SetColor(color4f_t(0, 1, 0, 0.3)).Create(false).Draw();

        //////////////////////////////////////////////////////////////////////*/

        Window.Update();
    }

    Quit();
    return 0;
}
