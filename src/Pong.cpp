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
    string_t    data;
};

typedef std::pair<string_t, string_t> addr_t;

static const string_t PONG_HOST_PORT("2013");   // When hosting.
static const string_t PONG_JOIN_PORT("2014");   // When joining.
static const uint16_t MAX_PONG = sizeof(PongPacket)+254;
static const uint16_t MIN_PONG = 9;

util::zRandom<> RNG;
math::vector_t make_ball();
bool parse_msg(const string_t& data, PongPacket& P);
string_t build_packet(PacketType type, const string_t& data);

#if defined(ZEN_DEBUG_BUILD) || !defined(_WIN32)
int main()
#elif defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShowCmd)
#endif // ZEN_DEBUG_BUILD
{
    if(!Init()) return 1;

    asset::zAssetManager Assets; Assets.Init();
    gfx::zWindow Main(800, 600, "Networked Pong 2.0", Assets);

    Main.Init();

    gfx::zScene Field(Main.GetWidth(), Main.GetHeight(), Assets);

    Field.Init();
    Field.EnableLighting();
    Field.DisablePostProcessing();
    Field.SetSeeThrough(false);

    obj::zEntity& BG            = Field.AddEntity();
    obj::zEntity& LeftPaddle    = Field.AddEntity();
    obj::zEntity& RightPaddle   = Field.AddEntity();
    obj::zEntity& Ball          = Field.AddEntity();
    gfx::zLight& BallLight      = Field.AddLight(gfx::LightType::ZEN_POINT);

    gfx::zQuad Paddle(Assets, 8, 64); Paddle.Create();
    Paddle.SetColor(color4f_t(1.0, 1.0, 1.0));

    LeftPaddle.AddPrimitive(Paddle);
    RightPaddle.AddPrimitive(Paddle);
    Ball.LoadFromTexture("assets/textures/ball.png");

    LeftPaddle.Move(0, Main.GetHeight() / 2 - LeftPaddle.GetH() / 2);
    RightPaddle.Move(Main.GetWidth() - RightPaddle.GetW(),
                     Main.GetHeight() / 2 - RightPaddle.GetH() / 2);
    Ball.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);

    BallLight.Enable();
    BallLight.SetAttenuation(0.005, 0.01, 0.0);
    BallLight.SetBrightness(1.0);
    BallLight.SetColor(color3f_t(0.0, 1.0, 1.0));
    BallLight.SetPosition(Ball.GetPosition());
    BallLight.Disable();

    real_t dy = 0.0;
    math::vector_t ball_d = make_ball();

    bool host = false, join = false;

    // Are we starting? Or are we using the other player's
    // initial ball position / velocity?
    bool is_start = false;

    // Main menu.
    gui::zMenu MainMenu(Main, Assets);
    MainMenu.SetFont("assets/ttf/menu.ttf", 24);
    MainMenu.SetNormalButtonTextColor(color4f_t(1, 1, 1));
    MainMenu.SetActiveButtonTextColor(color4f_t(0, 1, 1));
    MainMenu.SetInitialButtonPosition(math::vector_t(64, 200));
    MainMenu.SetSpacing(32);

    MainMenu.AddButton("Host a Game", [&host](size_t) { host = true; });
    MainMenu.AddButton("Join a Game", [&join](size_t) { join = true; });
    MainMenu.AddButton("Quit", [&Main](size_t)        { Main.Close();});

    util::CTimer Timer(60);
    evt::zEventHandler& Evts = evt::zEventHandler::GetInstance();
    evt::event_t Evt;

    while(Main.IsOpen() && !(host || join))
    {
        Timer.Start();
        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::WINDOW_CLOSE)
            {
                Main.Close();
                break;
            }

            MainMenu.HandleEvent(Evt);
        }

        Main.Clear();
        MainMenu.Update();
        Main.Update();
        Timer.Delay();
    }

    addr_t Connection;
    net::zSocket Socket(net::SocketType::UDP);

    gui::zFont& Font = *Assets.Create<gui::zFont>();
    Font.AttachManager(Assets);
    Font.LoadFromFile("assets/ttf/menu.ttf");
    Font.SetColor(color4f_t(1.0, 1.0, 0.0));

    if(join)
    {
        if(!Socket.Init("", PONG_JOIN_PORT)) { return 1; }
        Socket.SetNonblocking(true);

        gui::zMenu HostList(Main, Assets);
        HostList.SetFont("assets/ttf/menu.ttf");
        HostList.SetNormalButtonTextColor(color4f_t(1, 1, 1));
        HostList.SetActiveButtonTextColor(color4f_t(0, 1, 1));
        HostList.SetInitialButtonPosition(math::vector_t(64, 200));
        HostList.SetSpacing(32);

        // Add a title to the menu.
        obj::zEntity& Title = HostList.AddEntity();
        HostList.RenderWithFont(Title, "Searching for hosts...");
        Title.Move(64, 10);

        std::vector<addr_t> allHosts;
        int16_t host = -1;

        string_t call = build_packet(PacketType::STATUS, "");
        PongPacket response;
        Socket.SendBroadcast(call, PONG_HOST_PORT);

        // Loop until the user chooses a host.
        evt::zEventHandler& Evts = evt::zEventHandler::GetInstance();
        while(Main.IsOpen() && host == -1)
        {
            Timer.Start();
            Evts.PollEvents();
            while(Evts.PopEvent(Evt))
            {
                HostList.HandleEvent(Evt);
            }

            // Receive client data.
            string_t addr, port;
            string_t data = Socket.RecvFrom(MAX_PONG, addr, port);

            // Parse if we can. If so, check if the packet type
            // indicates host availability. If so, make sure that
            // this host isn't already in the list of allHosts. If
            // so, add it to the list of allHosts and post a menu button
            // allowing for the user to choose it.
            if(parse_msg(data, response) &&
               response.type == PacketType::HOST_AVAIL &&
               std::find(allHosts.begin(), allHosts.end(),
                         addr_t(addr, port)) == allHosts.end())
            {
                allHosts.emplace_back(addr_t(addr, port));
                HostList.AddButton(addr, [&host](size_t i) {
                    host = i;
                });
            }

            Main.Clear();
            HostList.Update();
            Main.Update();
            Timer.Delay();
        }

        // User chose a host?
        if(host != -1)
        {
            gfx::zScene Prelim(Main.GetWidth(), Main.GetHeight(), Assets);
            Prelim.Init();
            Prelim.DisableLighting();

            obj::zEntity& Status = Prelim.AddEntity();

            Font.ClearString();
            Font << "Attempting to join " << allHosts[host].first << "...";
            Font.Render(Status);
            Status.Center(Main);

            // By the time the user clicked the host, the host may already
            // have lost its HOST_AVAIL status, so we need to check it again.
            Socket.SendTo(allHosts[host].first, allHosts[host].second,
                          build_packet(PacketType::STATUS, ""));

            while(Main.IsOpen())
            {
                Evts.PollEvents();

                // Receive any data.
                string_t addr, port;
                string_t r = Socket.RecvFrom(MAX_PONG, addr, port);

                // If we got something, and it was a valid Pong protocol
                // packet, check the response type.
                if(addr == allHosts[host].first && parse_msg(r, response))
                {
                    // Host is still available? Then, send him our player's
                    // name and move on to the next step. (temporary)
                    if(response.type == PacketType::HOST_AVAIL)
                    {
                        Socket.SendTo(allHosts[host].first, allHosts[host].second,
                            build_packet(PacketType::JOIN, "username"));
                        break;
                    }
                    else if(response.type == PacketType::HOST_BUSY ||
                            response.type == PacketType::HOST_INGAME)
                    {
                        Font.Render(Status, "This host is no longer available.");
                    }
                    else
                    {
                        Font.Render(Status, "Host failed to respond!");
                    }
                }

                Main.Clear();
                Prelim.Render();
                Main.Update();
            }

            // We've sent our status, now wait for response.
            while(Main.IsOpen())
            {
                Evts.PollEvents();

                // Receive data.
                string_t addr, port;
                string_t r = Socket.RecvFrom(MAX_PONG, addr, port);

                // If it's from our potential match and a valid Pong packet...
                if(addr == allHosts[host].first && parse_msg(r, response))
                {
                    // If the host wants to sync...
                    if(response.type == PacketType::SYNC)
                    {
                        // Parse the data on the semi-colon.
                        // Protocol states that the sync gives the ball position,
                        // and the ball forces in this format:
                        // bx;by;bdx;bdy
                        std::vector<string_t> parts = util::split(response.data, ';');

                        Ball.Move(stod(parts[0]), stod(parts[1]));
                        ball_d = math::vector_t(stod(parts[2]), stod(parts[3]), 0.0);
                        Connection = std::move(allHosts[host]);
                        allHosts.clear();

                        // We aren't "starting," so we mark ourselves as such.
                        // Thus later on in the game loop when points are scored,
                        // we need to wait for server sync on the new ball position.
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

    else if(host)
    {
        if(!Socket.Init("", PONG_HOST_PORT))
        {
            util::zLog& Log = util::zLog::GetEngineLog();
            Log << Log.SetMode(util::LogMode::ZEN_ERROR)
                << Log.SetSystem("Socket") << "Failed to initialize socket: "
                << Socket.GetError() << '.' << util::zLog::endl;
        }
        Socket.SetNonblocking(true);

        // We are starting, so mark ourselves as such for later in the game
        // loop to differentiate between who defers ball velocity creation.
        is_start = true;

        gfx::zScene Waiter(Main.GetWidth(), Main.GetHeight(), Assets);
        Waiter.Init();
        Waiter.DisableLighting();

        obj::zEntity& Status = Waiter.AddEntity();
        Font.Render(Status, "Awaiting player...");
        Status.Move(Main.GetWidth()  / 2 - Status.GetW() / 2,
                    Main.GetHeight() / 2 - Status.GetH() / 2);

        // Final match for who to actually play with.
        addr_t Match;

        // Do we have a potential host lined up?
        bool potential = false;

        while(Main.IsOpen())
        {
            evt::zEventHandler::PollEvents();

            // Temporary receiving data.
            PongPacket resp;
            string_t tmpaddr, tmpport;
            string_t data = Socket.RecvFrom(MAX_PONG, tmpaddr, tmpport);

            // Valid Pong packet?
            if(parse_msg(data, resp))
            {
                // Someone has asked us for our status...
                if(resp.type == PacketType::STATUS)
                {
                    // So we tell them that we are available for a game.
                    Socket.SendTo(tmpaddr, tmpport, build_packet(
                        PacketType::HOST_AVAIL, ""
                    ));

                    // Indicate to the user that someone wants to join us.
                    Font.ClearString();
                    Font << "Potential match: " << tmpaddr << "\nResolving...";
                    Font.Render(Status);
                    Status.Move(Main.GetWidth()  / 2 - Status.GetW() / 2,
                                Main.GetHeight() / 2 - Status.GetH() / 2);

                    potential = true;
                    Match = std::make_pair(tmpaddr, tmpport);
                }

                // Someone has asked to join us, and they are the potential
                // client we set up earlier.
                else if(resp.type == PacketType::JOIN &&
                        potential && tmpaddr == Match.first)
                {
                    // Send them syncing data.
                    // Pong protocol dictates that sync data is in the
                    // following format:
                    // ball_x;ball_y;ball_dx;ball_dy
                    std::stringstream ss;
                    ss << Main.GetWidth() - Ball.GetX() << ';' << Ball.GetY()
                       << ';' << -ball_d.x   << ';' << ball_d.y;

                    Socket.SendTo(tmpaddr, tmpport, build_packet(
                        PacketType::SYNC, ss.str()
                    ));

                    Connection = std::move(Match);
                    break;
                }
            }

            Main.Clear();
            Waiter.Render();
            Main.Update();
        }
    }

    obj::zEntity& NetStatus = Field.AddEntity();
    obj::zEntity& Score     = Field.AddEntity();
    Score.Move(Main.GetWidth() / 2 - Score.GetW() / 2, 0.0);

    // Now we have established a connection to another peer (host / client
    // is now irrelevant, except for creating the new ball velocity).
    // Every frame we check for socket data, pinging the client every second,
    // and send our paddle position if it has changed.
    uint32_t frame = 0;
    uint32_t last  = 0;

    // Track the scores of both players.
    math::zVector<uint16_t> Scores;
    Font.ClearString();
    Font << Scores.x << "    |    " << Scores.y;
    Font.Render(Score);

    // Create a near-black background for the light to properly render.
    gfx::zQuad* pQuad = new gfx::zQuad(Assets, Main.GetWidth(),
                                               Main.GetHeight());
    pQuad->Create().SetColor(color4f_t(0.1, 0.1, 0.1, 1.0));
    BG.AddPrimitive(*pQuad);
    delete pQuad;

    // Network status.
    bool losing = false, lost = false, kk = false;
    bool scored = false;

    while(Main.IsOpen())
    {
        Timer.Start();

        // Increase tick count since last ping.
        ++last;

        // Ping once a second, if we haven't lost connection already.
        if(++frame % 60 == 0 && !lost)
        {
            Socket.SendTo(Connection.first, Connection.second, build_packet(
                PacketType::PING, std::to_string(
                    util::string_hash(std::to_string(time(nullptr)))
                )
            ));

            // After pinging, also do a sync just to maintain consistent state.
            std::stringstream ss;
            ss << (is_start ? Main.GetWidth() - Ball.GetX() : Ball.GetX())
               << ';' << Ball.GetY() << ';'
               << (is_start ? -ball_d.x : ball_d.x) << ';' << ball_d.y;

            Socket.SendTo(Connection.first, Connection.second, build_packet(
                PacketType::SYNC, ss.str()
            ));
        }

        Evts.PollEvents();
        while(Evts.PopEvent(Evt))
        {
            // Control player movement.
            if(Evt.type == evt::EventType::KEY_DOWN)
            {
                switch(Evt.key.key)
                {
                case evt::Key::UP:
                case evt::Key::W:
                    dy = -8.0;
                    break;

                case evt::Key::DOWN:
                case evt::Key::S:
                    dy = 8.0;
                    break;
                }
            }

            else if(Evt.type == evt::EventType::KEY_UP)
            {
                if(Evt.key.key == evt::Key::UP      ||
                   Evt.key.key == evt::Key::DOWN    ||
                   Evt.key.key == evt::Key::W       ||
                   Evt.key.key == evt::Key::S)
                    dy = 0.0;

                // Below options are for debugging scene state.
#ifdef ZEN_DEBUG_BUILD
                else if(Evt.key.key == evt::Key::L)
                    Field.ToggleLighting();

                else if(Evt.key.key == evt::Key::M)
                    gfxcore::zRenderer::ToggleWireframe();

                else if(Evt.key.key == evt::Key::P)
                    Field.TogglePostProcessing();
#endif // ZEN_DEBUG_BUILD
            }
        }

        // Ball hit the left player's wall.
        if(Ball.GetX() <= -Ball.GetW() && !scored)
        {
            Font.ClearString();
            Font << Scores.x << "    |    " << ++Scores.y;
            Font.Render(Score);

            scored = true;
        }

        // Ball hit the right player's wall.
        else if(Ball.GetX() >= Main.GetWidth() && !scored)
        {
            Font.ClearString();
            Font << ++Scores.x << "    |    " << Scores.y;
            Font.Render(Score);

            scored = true;
        }

        // Ball hit the top / bottom walls.
        else if(Ball.GetY() <= 0.0 ||
                Ball.GetY() >= Main.GetHeight() - Ball.GetH())
        {
            ball_d.y = -ball_d.y;
        }

        // Bounce ball off of player paddles.
        if(LeftPaddle.GetBox().collides(Ball.GetBox()) ||
           RightPaddle.GetBox().collides(Ball.GetBox()))
        {
            ball_d.x = -ball_d.x;
        }

        // Player moved since last frame?
        if(!math::compf(dy, 0.0))
        {
            LeftPaddle.Adjust(0.0, dy);
            if(!lost)
            {
                // Sync up player position.
                std::stringstream ss;
                ss << LeftPaddle.GetX() << ';' << LeftPaddle.GetY();
                Socket.SendTo(Connection.first, Connection.second,
                    build_packet(PacketType::POS_PLAYER, ss.str()));
            }
        }

        // Handle a bad connection. ~8 seconds = lost connection.
        if(last > 60 * 8 && !lost)
        {
            Font.Render(NetStatus, "connection lost.");
            lost = true; kk = false;
        }

        // ~4 seconds = losing connection.
        else if(last > 60 * 4 && !losing)
        {
            Font.Render(NetStatus, "losing connection.");
            losing = true; kk = false;
        }

        // If there was a goal scored, we need to sync things up
        // with the other player. If we were originally the host,
        // we create the new ball position. Otherwise, we wait for
        // the other player to send us one.
        if(scored && is_start && !lost)
        {
            Ball.Move(Main.GetWidth() / 2, Main.GetHeight() / 2);
            ball_d = make_ball();
            std::stringstream ss;
            ss << Main.GetWidth() - Ball.GetX() << ';' << Ball.GetY() << ';'
               << -ball_d.x    << ';' << ball_d.y;

            Socket.SendTo(Connection.first, Connection.second,
                          build_packet(PacketType::SYNC, ss.str()));
            scored = false;
        }

        // Handle network operations.
        PongPacket resp;
        string_t addr, port;
        string_t data = Socket.RecvFrom(MAX_PONG, addr, port);
        if(!lost && addr == Connection.first && parse_msg(data, resp))
        {
            switch(resp.type)
            {
            case PacketType::PING:
                // Confirm ping.
                //if(resp.data != last_ping)
                //{
                //    Socket.SendTo(addr, port, resp.data);
                //}
                if(!kk) Font.Render(NetStatus, "ok.");
                last = 0;
                break;

            // Other player has moved.
            case PacketType::POS_PLAYER:
            {
                std::vector<string_t> parts = util::split(resp.data, ';');
                RightPaddle.Move(RightPaddle.GetX(), stod(parts[1]));
                break;
            }

            // Ball has moved? (temporary).
            case PacketType::POS_BALL:
            {
                // Ignore if we are in charge.
                if(is_start) break;
                std::vector<string_t> parts = util::split(resp.data, ';');

                // If we are inaccurate, we should probably think about
                // syncing with the other player.
                math::vector_t Pos(std::stod(parts[0]), std::stod(parts[1]));
                if(Pos != Ball.GetPosition())
                {
                    std::stringstream ss;
                    ss << Ball.GetX() << ';' << Ball.GetY() << ';'
                       << ball_d.x    << ';' << ball_d.y;

                    Socket.SendTo(Connection.first, Connection.second,
                                  build_packet(PacketType::SYNC,
                                               ss.str()));
                }

                break;
            }

            // Someone wants to know wassup.
            case PacketType::STATUS:
                Socket.SendTo(Connection.first, Connection.second,
                              build_packet(PacketType::HOST_INGAME,
                                           Connection.first));
                break;

            // Sync with player.
            case PacketType::SYNC:
            {
                // Parse new ball data.
                std::vector<string_t> parts = util::split(resp.data, ';');

                math::vector_t BallPosTmp(std::stod(parts[0]),
                                          std::stod(parts[1]));
                math::vector_t BallDTmp  (std::stod(parts[2]),
                                          std::stod(parts[3]));

                // Is the temporary velocity identical to ours?
                if(BallDTmp != ball_d)
                {
                    // If not, and we are the hoster, we don't care about the
                    // temporary, and we just send our true position.
                    if(is_start)
                    {
                        std::stringstream ss;
                        ss << Main.GetWidth() - Ball.GetX() << ';'
                           << Ball.GetY() << ';' << -ball_d.x
                           << ';' << ball_d.y;

                        Socket.SendTo(Connection.first, Connection.second,
                                      build_packet(PacketType::SYNC,
                                                   ss.str()));
                    }
                    else
                    {
                        ball_d = BallDTmp;
                        Ball.Move(BallPosTmp);
                    }
                }

                // "scored" is set when... well... someone scores. And another
                // point cannot be made until the flag is unset.
                // Thus we need to unset it when we sync up.
                scored = false;
                break;
            }
            }
        }

        Ball.Adjust(ball_d);

        // Light follows the ball position.
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
    math::zVector<int8_t> dirs(RNG.randint(-1, 1), RNG.randint(-1, 1));
    if(dirs.x == 0) dirs.x = -1;
    if(dirs.y == 0) dirs.y = -1;

    return math::vector_t(dirs.x * (RNG.randint(2, 7)),
                          dirs.y * (RNG.randint(2, 7)), 0.0);
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
