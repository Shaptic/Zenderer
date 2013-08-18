#include <random>
#include <sstream>

#include "Zenderer/Zenderer.hpp"

using namespace zen;

enum class PacketType : uint16_t
{
    UNKNOWN,
    HOST_AVAIL,
    HOST_BUSY,
    HOST_INGAME,
    JOIN,
    PING,
    POS_PLAYER,
    POS_BALL,
    STATUS,
    SYNC
};

struct PongPacket
{
    uint32_t    seq;
    uint32_t    stamp;
    PacketType  type;
    uint16_t    size;
    std::string data;
};

static const string_t PONG_PORT("2013");
static const uint16_t MAX_PONG = sizeof(PongPacket) + 255;
static const uint16_t MIN_PONG = 1;

std::mt19937 rng;
math::vector_t make_ball();
int randint(const int low, const int hi);
bool parse_msg(const string_t& data, PongPacket& P);
string_t build_packet(PacketType type, const string_t& data);

int main()
{
    if(!Init()) return 1;
    rng.seed(time(nullptr));

    util::CSettings Settings;
    Settings.Init();

    asset::CAssetManager Assets; Assets.Init();
    gfx::CWindow Main(800, 600, "Networked Pong 2.0", Assets);

    Main.Init();

    gfx::CScene Field(Main.GetWidth(), Main.GetHeight(), Assets);

    Field.Init();
    Field.EnableLighting();
    Field.DisablePostProcessing();
    Field.SetSeeThrough(false);

    obj::CEntity& BG            = Field.AddEntity();
    obj::CEntity& LeftPaddle    = Field.AddEntity();
    obj::CEntity& RightPaddle   = Field.AddEntity();
    obj::CEntity& Ball          = Field.AddEntity();
    gfx::CLight& BallLight      = Field.AddLight(gfx::LightType::ZEN_POINT);

    gfx::CQuad Paddle(Assets, 8, 64);
    Paddle.Create();
    Paddle.SetColor(color4f_t(1.0, 1.0, 1.0));

    LeftPaddle.AddPrimitive(Paddle);
    RightPaddle.AddPrimitive(Paddle);
    Ball.LoadFromTexture("assets/textures/ball.png");

    LeftPaddle.Move(0, Main.GetHeight() / 2 - LeftPaddle.GetH() / 2);
    RightPaddle.Move(Main.GetWidth() - RightPaddle.GetW(),
                     Main.GetHeight() / 2 - RightPaddle.GetH() / 2);

    BallLight.Enable();
    BallLight.SetAttenuation(0.005, 0.01, 0.0);
    BallLight.SetBrightness(1.0);
    BallLight.SetColor(color3f_t(0.0, 1.0, 1.0));
    BallLight.SetPosition(Ball.GetPosition());
    BallLight.Disable();

    util::CTimer Timer(60);
    evt::event_t Evt;

    real_t dy = 0.0;
    math::vector_t ball_d = make_ball();
    Ball.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);

    bool play = false, host = false, join = false;
    bool is_start = false;

    // Main menu.
    gui::CMenu MainMenu(Main, Assets);
    MainMenu.SetFont("assets/ttf/menu.ttf", 24);
    MainMenu.SetNormalButtonTextColor(color4f_t(1, 1, 1));
    MainMenu.SetActiveButtonTextColor(color4f_t(0, 1, 1));
    MainMenu.SetInitialButtonPosition(math::vector_t(64, 200));
    MainMenu.SetSpacing(32);

    MainMenu.AddButton("Host a Game", [&host, &play](size_t) {
        play = host = true;
    });

    MainMenu.AddButton("Join a Game", [&join, &play](size_t) {
        play = join = true;
    });

    MainMenu.AddButton("Quit", [&Main](size_t) {
        Main.Close();
    });

    evt::CEventHandler& Evts = evt::CEventHandler::GetInstance();
    while(Main.IsOpen() && !play)
    {
        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            Timer.Start();
            MainMenu.HandleEvent(Evt);
            Timer.Delay();
        }

        Main.Clear();
        MainMenu.Update();
        Main.Update();
    }

    obj::CEntity& NetStatus = Field.AddEntity();
    obj::CEntity& Score     = Field.AddEntity();

    std::pair<string_t, string_t> conn;
    net::CSocket Socket(net::SocketType::UDP);

    gui::CFont& Font = *Assets.Create<gui::CFont>();
    Font.AttachManager(Assets);
    Font.LoadFromFile("assets/ttf/menu.ttf");
    Font.SetColor(color4f_t(1.0, 1.0, 0.0));
    Font.Render(NetStatus, "ok.");

    if(play && join)
    {
        if(!Socket.Init("", "2014"))
        {
            util::CLog& Log = util::CLog::GetEngineLog();
            Log << Log.SetMode(util::LogMode::ZEN_ERROR)
                << Log.SetSystem("Socket") << "Failed to initialize socket: "
                << Socket.GetError() << '.' << util::CLog::endl;
        }
        Socket.SetNonblocking(true);

        gui::CMenu HostList(Main, Assets);
        HostList.SetFont("assets/ttf/menu.ttf");
        HostList.SetNormalButtonTextColor(color4f_t(1, 1, 1));
        HostList.SetActiveButtonTextColor(color4f_t(0, 1, 1));
        HostList.SetInitialButtonPosition(math::vector_t(64, 200));
        HostList.SetSpacing(32);

        HostList.AddButton("Searching for hosts...");

        std::vector<std::pair<string_t, string_t>> hosts;
        int16_t host = -1;

        string_t packet = build_packet(PacketType::STATUS, "");

        Socket.SendBroadcast(packet, "2013");

        PongPacket response;
        while(Main.IsOpen() && host == -1)
        {
            evt::CEventHandler::PollEvents();
            while(evt::CEventHandler::GetInstance().PopEvent(Evt))
            {
                HostList.HandleEvent(Evt);
            }

            string_t addr, port, data;
            data = Socket.RecvFrom(MAX_PONG, addr, port);

            parse_msg(data, response);

            size_t b4 = hosts.size();

            if(!data.empty() && response.type == PacketType::HOST_AVAIL &&
                std::find(hosts.begin(), hosts.end(),
                std::make_pair(addr, port)) == hosts.end())
            {
                hosts.emplace_back(std::make_pair(addr, port));
                HostList.AddButton(addr, [&host](size_t i) {
                    host = i - 1;   // Offset for the "Searching" button.
                });
            }

            Main.Clear();
            HostList.Update();
            Main.Update();
        }

        // User chose a host?
        if(host != -1)
        {
            gfx::CScene Prelim(Main.GetWidth(), Main.GetHeight(), Assets);
            Prelim.Init();
            Prelim.DisableLighting();

            obj::CEntity& Status = Prelim.AddEntity();
            Font.ClearString();
            Font << "Attempting to join " << hosts[host].first << "...";
            Font.Render(Status);
            Status.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);

            Socket.SendTo(hosts[host].first, hosts[host].second,
                          build_packet(PacketType::STATUS, ""));

            while(Main.IsOpen())
            {
                evt::CEventHandler::PollEvents();
                string_t addr, port;
                string_t r = Socket.RecvFrom(MAX_PONG, addr, port);
                if(!r.empty() && addr == hosts[host].first)
                {
                    parse_msg(r, response);
                    if(response.type == PacketType::HOST_AVAIL)
                    {
                        std::stringstream ss;
                        Socket.SendTo(hosts[host].first, hosts[host].second,
                            build_packet(PacketType::JOIN, "username"));
                        break;
                    }
                    else
                    {
                        Font.ClearString();
                        Font << "Host failed to respond!";
                        Font.Render(Status);
                    }
                }

                Main.Clear();
                Prelim.Render();
                Main.Update();
            }

            // We've sent our status, now wait for response.
            while(Main.IsOpen())
            {
                evt::CEventHandler::PollEvents();
                string_t addr, port;
                string_t r = Socket.RecvFrom(MAX_PONG, addr, port);

                if(!r.empty() && addr == hosts[host].first)
                {
                    parse_msg(r, response);
                    if(response.type == PacketType::SYNC)
                    {
                        std::vector<string_t> parts = util::split(response.data, ';');

                        // Protocol states that the sync gives the ball position,
                        // and the ball forces in this format:
                        // bx;by;bdx;bdy
                        Ball.Move(stod(parts[0]), stod(parts[1]));
                        ball_d = math::vector_t(stod(parts[2]), stod(parts[3]), 0.0);
                        conn = hosts[host];
                        is_start = false;
                        break;
                    }
                }

                Main.Clear();
                Prelim.Render();
                Main.Update();
            }
        }
    }

    else if(play && host)
    {
        if(!Socket.Init("", PONG_PORT))
        {
            util::CLog& Log = util::CLog::GetEngineLog();
            Log << Log.SetMode(util::LogMode::ZEN_ERROR)
                << Log.SetSystem("Socket") << "Failed to initialize socket: "
                << Socket.GetError() << '.' << util::CLog::endl;
        }
        Socket.SetNonblocking(true);

        is_start = true;

        gfx::CScene Waiter(Main.GetWidth(), Main.GetHeight(), Assets);
        Waiter.Init();
        Waiter.DisableLighting();

        obj::CEntity& Status = Waiter.AddEntity();
        Font.Render(Status, "Awaiting player...");
        Status.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);

        string_t addr, port;
        bool potential = false;

        while(Main.IsOpen())
        {
            evt::CEventHandler::PollEvents();

            string_t tmpaddr;
            string_t data = Socket.RecvFrom(MAX_PONG, tmpaddr, port);

            PongPacket resp;
            if(parse_msg(data, resp))
            {
                if(resp.type == PacketType::STATUS)
                {
                    Socket.SendTo(tmpaddr, port, build_packet(
                        PacketType::HOST_AVAIL, ""
                    ));

                    Font.ClearString();
                    Font << "Potential match: " << tmpaddr << "\nResolving...";
                    Font.Render(Status);
                    potential = true;
                    addr = tmpaddr;
                }

                else if(resp.type == PacketType::JOIN && potential && tmpaddr == addr)
                {
                    std::stringstream ss;
                    ss << Ball.GetX() << ';' << Ball.GetY() << ';'
                       << ball_d.x << ';' << ball_d.y;
                    Socket.SendTo(addr, port, build_packet(
                        PacketType::SYNC, ss.str()
                    ));

                    conn.first = addr;
                    conn.second = port;
                    break;
                }
            }

            Main.Clear();
            Waiter.Render();
            Main.Update();
        }
    }

    // Now we have established a connection to another peer (host / client
    // is now irrelevant). Every frame we check for socket data, pinging
    // the client every second, and send our paddle position if it has changed.
    uint32_t frame = 0;
    uint32_t last  = 0;

    gfx::CQuad* pQuad = new gfx::CQuad(Assets, Main.GetWidth(), Main.GetHeight());
    pQuad->Create().SetColor(color4f_t(0.1, 0.1, 0.1, 1.0));
    BG.AddPrimitive(*pQuad);
    delete pQuad;

    uint16_t mescore = 0, theyscore = 0;
    Font.ClearString();
    Font << mescore << "    |    " << theyscore;
    Font.Render(Score);

    Score.Move(Main.GetWidth() / 2 - Score.GetW() / 2, 0.0);
    bool losing = false, lost = false, kk = true;

    while(Main.IsOpen())
    {
        ++last;
        if(++frame % 60 == 0 && !lost)
        {
            std::stringstream ss;
            ss << time(nullptr);

            Socket.SendTo(conn.first, conn.second, build_packet(
                PacketType::PING, ss.str()
            ));
        }

        Timer.Start();

        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::KEY_DOWN)
            {
                switch(Evt.key.key)
                {
                case evt::Key::UP:
                    dy = -8.0;
                    break;

                case evt::Key::DOWN:
                    dy = 8.0;
                    break;
                }
            }

            else if(Evt.type == evt::EventType::KEY_UP)
            {
                if(Evt.key.key == evt::Key::UP || Evt.key.key == evt::Key::DOWN)
                    dy = 0.0;

                else if(Evt.key.key == evt::Key::L)
                    Field.ToggleLighting();

                else if(Evt.key.key == evt::Key::M)
                    gfxcore::CRenderer::ToggleWireframe();

                else if(Evt.key.key == evt::Key::P)
                    Field.TogglePostProcessing();
            }
        }

        if(Ball.GetX() <= -Ball.GetW())
        {
            Font.ClearString();
            Font << mescore << "    |    " << ++theyscore;
            Font.Render(Score);

            Ball.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);
            ball_d = make_ball();
        }
        else if(Ball.GetX() >= Main.GetWidth())
        {
            Font.ClearString();
            Font << ++mescore << "    |    " << theyscore;
            Font.Render(Score);

            Ball.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);
            ball_d = make_ball();
        }
        else if(Ball.GetY() <= 0.0 ||
                Ball.GetY() >= Main.GetHeight() - Ball.GetH())
        {
            ball_d.y = -ball_d.y;
        }

        if(LeftPaddle.GetBox().collides(Ball.GetBox()))
        {
            ball_d.x = -ball_d.x;
        }

        if(!math::compf(dy, 0.0))
        {
            std::stringstream ss;
            LeftPaddle.Adjust(0.0, dy);
            if(!lost)
            {
                ss << LeftPaddle.GetX() << ';' << LeftPaddle.GetY();
                Socket.SendTo(conn.first, conn.second, build_packet(
                    PacketType::POS_PLAYER, ss.str()
                ));
            }
        }

        if(last > 60 * 8 && !lost)
        {
            Font.Render(NetStatus, "connection lost.");
            lost = true; kk = false;
        }
        else if(last > 60 * 4 && !losing)
        {
            Font.Render(NetStatus, "losing connection.");
            losing = true; kk = false;
        }

        string_t addr, port;
        string_t data = Socket.RecvFrom(MAX_PONG, addr, port);
        if(!lost && addr == conn.first && data.size() > 0)
        {
            PongPacket resp;
            parse_msg(data, resp);
            switch(resp.type)
            {
            case PacketType::PING:
                // Confirm ping.
                //if(resp.data != last_ping)
                //{
                //    Socket.SendTo(addr, resp.data);
                //}
                if(!kk) Font.Render(NetStatus, "ok.");
                last = 0;
                break;

            case PacketType::POS_PLAYER:
            {
                std::vector<string_t> parts = util::split(resp.data, ';');
                RightPaddle.Move(RightPaddle.GetX(), stod(parts[1]));
                break;
            }

            case PacketType::POS_BALL:
            {
                std::vector<string_t> parts = util::split(resp.data, ';');
                Ball.Move(std::stod(parts[0]), std::stod(parts[1]));
                break;
            }

            case PacketType::SYNC:
            {
                std::vector<string_t> parts = util::split(resp.data, ';');
                math::vector_t ball_d_tmp(std::stod(parts[0]), std::stod(parts[1]));
                if(ball_d_tmp != ball_d)
                {
                    if(is_start)
                    {
                        std::stringstream ss;
                        ss << Ball.GetX() << ';' << Ball.GetY() << ';'
                           << ball_d.x << ';' << ball_d.y;
                        Socket.SendTo(conn.first, conn.second, build_packet(
                            PacketType::SYNC, ss.str()
                        ));
                    }
                    else
                    {
                        ball_d = ball_d_tmp;
                    }
                }

                break;
            }

            case PacketType::UNKNOWN:
            {
                util::CLog& L = util::CLog::GetEngineLog();
                L << L.SetMode(util::LogMode::ZEN_ERROR) << L.SetSystem("Pong")
                  << "Unknown packet received from peer." << util::CLog::endl;
              break;
            }
            }
        }

        Ball.Adjust(ball_d);

        BallLight.Enable();
        BallLight.SetPosition(Ball.GetPosition() +
                              math::vector_t(Ball.GetW() / 2,
                                             Ball.GetH() / 2));
        BallLight.Disable();

        Main.Clear();
        Field.Render();
        Main.Update();

        Timer.Delay();
    }

    Quit();
    return 0;
}

math::vector_t make_ball()
{
    math::Vector<int8_t> dirs(randint(-1, 1), randint(-1, 1));
    if(dirs.x == 0) dirs.x = -1;
    if(dirs.y == 0) dirs.y = -1;

    return math::vector_t(dirs.x * (randint(2, 7)),
                          dirs.y * (randint(2, 7)), 0.0);
}

int randint(const int low, const int hi)
{
    std::uniform_int_distribution<int> dist(low, hi);
    return dist(rng);
}

bool parse_msg(const string_t& data, PongPacket& P)
{
    P.seq = P.stamp = P.size = 0;
    P.type = PacketType::UNKNOWN;
    P.data = "";

    if(data.empty() || data.size() < MIN_PONG) return false;

    std::vector<string_t> parts = util::split(data, ':');
    if(parts.size() < 5) return false;

    P.seq  = stoi(parts[0]);
    P.stamp= stoi(parts[1]);
    P.type = static_cast<PacketType>(stoi(parts[2]));
    P.size = stoi(parts[3]);
    P.data.resize(data.size(), '\0');

    // Compensate for the fact that the data itself may have
    // contained the ':' character.
    string_t final_data;
    std::for_each(parts.begin() + 4, parts.end(),
                 [&final_data](const string_t& s) {
        final_data.insert(final_data.end(), s.begin(), s.end());
    });
    P.data = final_data;

    return P.type != PacketType::UNKNOWN;
}

string_t build_packet(PacketType type, const string_t& data)
{
    static std::stringstream ss;
    static uint32_t seq_no = 0;

    ss.str(std::string());

    ss << ++seq_no << ':' << time(nullptr) << ':'
       << static_cast<uint16_t>(type) << ':' << data.size()
       << ':' << data;

    return ss.str();
}
