#include <random>
#include "Zenderer/Zenderer.hpp"

using namespace zen;

std::mt19937 rng;
math::vector_t make_ball();

int main()
{
    if(!Init()) return 1;
    rng.seed(time(nullptr));
    
    util::CSettings Settings;
    Settings.LoadFromFile("Pong.ini");
    
    asset::CAssetManager Assets; Assets.Init();
    gfx::CWindow Main(Settings["WINDOW_WIDTH"], Settings["WINDOW_HEIGHT"],
                      "Networked Pong 2.0", Assets);

    Main.Init();
    
    gfx::CScene Field(Main.GetWidth(), Main.GetHeight(), Assets);
    
    Field.DisablePostProcessing();
    Field.SetSeeThrough(false);
    
    obj::CEntity& LeftPaddle    = Field.AddEntity();
    obj::CEntity& RightPaddle   = Field.AddEntity();
    obj::CEntity& Ball          = Field.AddEntity();
    
    gfx::CLight& BallLight      = Field.AddLight(gfx::LightType::POINT_LIGHT);
    
    gfx::CQuad Paddle(8, 32);
    Paddle.Create();
    Paddle.SetColor(color4f_t(1.0, 0.0, 0.0));
    
    LeftPaddle.AddPrimitive(Paddle);
    RightPaddle.AddPrimitive(Paddle);
    Ball.LoadFromTexture("assets/textures/ball.png");
    
    BallLight.Enable();
    BallLight.SetBrightness(1.0);
    BallLight.SetColor(color4f_t(0.0, 1.0, 1.0));
    BallLight.SetPosition(Ball.GetPosition());
    BallLight.Disable();
    
    util::CTimer Timer(60);
    evt::event_t Evt;
    
    real_t dy = 0.0;
    math::vector_t ball_d = make_ball();
    Ball.Move(Window.GetWidth() / 2, Window.GetHeight() / 2);
    
    // Main menu.
    bool play = false;
    gui::CMenu MainMenu(Main, Assets);
    MainMenu.SetFont("assets/ttf/menu.ttf", 24);
    MainMenu.SetNormalButtonTextColor(color4f_t(1, 1, 1));
    MainMenu.SetActiveButtonTextColor(color4f_t(0, 1, 1));
    MainMenu.SetInitialButtonPosition(math::vector_t(64, 200));
    
    MainMenu.AddButton("Play Pong!", [&play]{
        play = true;
    });

    MainMenu.AddButton("Quit", [&Window] {
        Window.Close();
    });
    
    while(Window.IsOpen() && !play)
    {
        while(evt::CEventHandler::PopEvent(Evt)
            MainMenu.HandleEvent(Evt);
        
        Main.Clear();
        MainMenu.Update();
        Main.Update();
    }
    
    while(Window.IsOpen())
    {
        Timer.Start();
        
        while(evt::CEventHandler::PopEvent(Evt))
        {
            if(Evt.type == evt::EventType::KEY_DOWN)
            {
                switch(Evt.key.key)
                {
                case evt::Key::UP:
                    dy = -5.0;
                    break;
                    
                case evt::Key::DOWN:
                    dy = 5.0;
                    break;
                }
            }
            
            else if(Evt.type == evt::EventType::KEY_UP)
            {
                if(Evt.key.key == evt::Key::UP || 
                   Evt.key.key == evt::Key::DOWN)
                    dy = 0.0;
            }
        }
        
        if(Ball.GetPosition().x <= 0.0 ||
           Ball.GetPosition().x >= Window.GetWidth())
        {
            Ball.Move(Window.GetWidth() / 2, Window.GetHeight() / 2);
            ball_d = make_ball();
        }
        
        else if(Ball.GetPosition().y <= 0.0 ||
                Ball.GetPosition().y >= Window.GetHeight())
        {
            ball_d.y = -ball_d.y;
        }
        
        LeftPaddle.Move(LeftPaddle.GetPosition() + math::vector_t(0.0, dy));
        Ball.Move(Ball.GetPosition() + ball_d);
        
        BallLight.Enable();
        BallLight.Move(Ball.GetPosition());
        BallLight.Disable();
        
        Main.Clear();
        Field.Update();
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
                          dirs.y * (randint(2, 7)));
}

int randint(const int low, const int hi)
{
    std::uniform_int_distribution<int> dist(low, hi);
    return dist(rng);
}
