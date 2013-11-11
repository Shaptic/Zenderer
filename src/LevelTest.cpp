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

        mp_Font = m_Assets.Create<gui::zFont>();
        mp_Font->SetSize(16);
        mp_Font->SetColor(1.0, 1.0, 1.0);

        if (!mp_Font->LoadFromFile(FONT_PATH"game.ttf"))
        {
            m_Assets.Delete(mp_Font);
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

                std::cout << poi.collision << " ... ";
                if (poi.collision)
                {
                    real_t m = (poi.line1[0].y - poi.line1[1].y) /
                               (poi.line1[0].x - poi.line1[1].x);

                    std::cout << m << ',' << poi.tri1[0] << std::endl;

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

                    std::cout << m_PDelta << std::endl;
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
            mp_allBullets.emplace_back(pBullet);
        }
        */
        m_Trail.Adjust(m_tshift, 0.0);
    }

    void Render()
    {
        math::cquery_t q;
        for (auto& i : mp_allBullets)
        {
            for (auto& j : m_Level.physical)
            {
                if (i->Collides(*j, &q))
                {
                    math::vector_t rate = i->GetRate();
                    math::vector_t surf = q.line2[1] - q.line2[0];
                    real_t dot   = rate * surf;
                    real_t theta = std::acos(dot / (rate.Magnitude() * surf.Magnitude()));
                    rate.Rotate(-2 * theta);
                    i->SetRate(rate.x, rate.y);
                }
            }

            if (i->GetX() > m_Scene.GetWidth()  || i->GetX() < 0.0 ||
                i->GetY() > m_Scene.GetHeight() || i->GetY() < 0.0)
            {
                // I'm not sure if this is even valid lol.
                mp_allBullets.erase(std::find(mp_allBullets.begin(),
                                              mp_allBullets.end(), i));
                break;
            }

            i->Update();
        }

        m_Scene.Render();
        /*
        uint16_t y = m_Scene.GetHeight() - 200;//m_Ground.GetH();
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

            y += Line.GetH();
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
                Rate.y = (!math::compf(Rate.y, 0.0)) ? Rate.y : -15.0;
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
            mp_allBullets.emplace_back(pBullet);
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

int main65(int argc, char* argv[])
{
    if (!Init()) return 1;

    asset::zAssetManager Assets;
    CHECK(Assets.Init());

    gfx::zWindow Window(800, 600, "Praecursor", Assets);
    CHECK(Window.Init());

    gfx::zScene GameWorld(800, 600, Assets);
    CHECK(GameWorld.Init());
    GameWorld.EnableLighting();

    string_t level_name(LEVEL_PATH"level2.zlv");
    if (argc > 1) level_name = string_t(argv[1]);

    lvl::level_t FirstLevel;
    lvl::zLevelLoader LevelLoader(GameWorld, Assets);
    CHECK(LevelLoader.LoadFromFile(level_name));
    CHECK(LevelLoader.PopLevel(FirstLevel));

    gWorld World(Window, Assets, GameWorld);
    CHECK(World.LoadLevel(std::move(FirstLevel)));

    using evt::EventType;
    util::zTimer Timer(60);
    evt::event_t Evt;

    std::cout << "Entering main loop...\n";
    bool quit = false;
    while (!quit && Window.IsOpen())
    {
        Timer.Start();
        evt::zEventHandler::PollEvents();
        while(evt::zEventHandler::GetInstance().PopEvent(Evt))
        {
            quit = (Evt.type    == EventType::WINDOW_CLOSE ||
                    Evt.key.key == evt::Key::ESCAPE);
            World.HandleEvent(Evt);
        }

        World.Update();
        Window.Clear();
        World.Render();
        Window.Update();
        Timer.Delay();
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
int main_stoy()
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

    gfx::zScene UI(Window.GetWidth(), Window.GetHeight(), Assets);
    UI.Init(); UI.SetSeeThrough(true);

    gfx::zLight& Light = Scene.AddLight(gfx::LightType::ZEN_POINT);
    Light.Enable();
    Light.SetPosition(Window.GetWidth() / 2, Window.GetHeight() / 2);
    Light.SetBrightness(1.0);
    Light.Disable();

    gui::zButton Brightness(UI);
    gui::zButton Attenuation(UI);
    gui::zButton Angles(UI);

    std::vector<gui::zButton*> Buttons { &Brightness, &Attenuation, &Angles };

    gui::zFont& Font = *Assets.Create<gui::zFont>("C:\\Windows\\Fonts\\Arial.ttf");

    for(auto& i : Buttons) i->SetFont(Font);

    color4f_t black(0.2, 0.2, 0.2, 1);
    for(auto& i : Buttons)
    {
        i->SetActiveColor(black);
        i->SetNormalColor(color4f_t());
    }

    Brightness.Prepare  ("Set Brightness");
    Attenuation.Prepare ("Set Attenuation");
    Angles.Prepare      ("Set Angles [Spotlight Only]");

    uint16_t y = 0;
    for(auto& i : Buttons)
    {
        i->Place(0, y);
        y += Font.GetLineHeight() + 10;
    }

    obj::zEntity& InputLabel = Scene.AddEntity();

    Font.SetColor(color4f_t());
    Font << "Brightness: " << Light.GetBrightness();
    Font.Render(InputLabel);
    InputLabel.Move(Window.GetWidth()  / 2 - Font.GetTextWidth("Brightness: "),
                    Window.GetHeight() / 3 * 2);
    InputLabel.Disable();

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

            if(Evt.type == evt::EventType::MOUSE_DOWN)
            {
                if(Brightness.IsOver(Evt.mouse.position))
                {
                    ToolState = State::EDIT_BRT;
                }
            }

            if(ToolState == State::EDIT_BRT &&
               Evt.type == evt::EventType::KEY_PRINTABLE)
            {
                InputLabel.Scale(math::vector_t(-1, 1, 1));
            }
        }

        for(auto& i : Buttons)
        {
            i->SetDefault();
            if(i->IsOver(evt::GetMousePosition()))
                i->SetActive();
        }

        Window.Clear();

        if(ToolState == State::EDIT_BRT)
        {
            InputLabel.Enable();
        }

        Scene.Render(color4f_t(0.1, 0.1, 0.1));

        UI.Render();
        Window.Update();
    }

    Scene.Destroy();

    Quit();
    return 0;
}

#include <set>

int main()
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

    math::vector_t LPos(400, 300);
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
    Caster.Move(0, 450);
    Caster.SetColor(color4f_t(1, 0, 0));
    Caster.Create();*/
    zQuad Caster(Assets, 256, 32);
    Caster.SetColor(color4f_t(1, 0, 0, 0.2)).Create().Move(250, 150);

    zRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);

    while(!quit)
    {
        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::WINDOW_CLOSE)
                quit = true;

            else if(Evt.type == evt::EventType::KEY_DOWN && 
                    Evt.key.key == evt::Key::W)
                zRenderer::ToggleWireframe();
        }

        Window.Clear();

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

        angles.push_back(std::atan2(-LPos.y, -LPos.x));
        angles.push_back(std::atan2(600 - LPos.y, -LPos.x));
        angles.push_back(std::atan2(600 - LPos.y, 800 - LPos.x));
        angles.push_back(std::atan2(-LPos.y, 800 - LPos.x));

        for(auto& d : angles) d = math::deg(d);
        std::unique(angles.begin(), angles.end());
        std::sort(angles.begin(), angles.end(), std::less<real_t>());

        real_t maxlen = 400;

        math::vector_t first;
        ShadowMap.AddVertex(LPos);
        for(auto& d : angles)
        {
            math::line_t line { {
                LPos,
                LPos + math::vector_t(maxlen * std::cos(math::rad(d)),
                                      maxlen * std::sin(math::rad(d)))
            } };

            math::vector_t end = line[1]; end.z = 1;
            for(size_t i = 0; i < tris.size(); i += 3)
            {
                math::cquery_t query;
                math::tri_t tri { { tris[i], tris[i + 1], tris[i + 2] } };

                if(math::collides(line, tri, &query) &&
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
                    Collision.SetColor(color4f_t(0, 0, 1, 1)).Create().Draw();
                }
            }

            ShadowMap.AddVertex(end);
            if(d == angles.front()) first = end;
        }

        ShadowMap.AddVertex(first);
        ShadowMap.SetColor(color4f_t(0, 1, 0, 0.1)).Create(false).Draw();

        ////////////////////////////////////////////////////////////////////////

        Window.Update();
    }

    Quit();
}
