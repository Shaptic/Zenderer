#include "Zenderer/Zenderer.hpp"

using namespace zen;

int main()
{
    Init();

    asset::zAssetManager Assets1; Assets1.Init();
    gfx::zWindow Window1(300, 200, "Window 1", Assets1, false);
    gfx::zWindow Window2(300, 200, "Window 2", Assets1, false);

    Window1.Init();
    evt::event_t e;
    bool quit = false;
    while (!quit)
    {
        evt::zEventHandler::GetInstance().PollEvents();
        while (evt::zEventHandler::GetInstance().PopEvent(e))
            quit = e.type == evt::EventType::WINDOW_CLOSE;

        Window1.Clear(color4f_t(1.0, 0, 0));
        Window1.Update();
    }

    Window1.Destroy();
    Window2.Init();
    quit = false;
    while (!quit)
    {
        evt::zEventHandler::GetInstance().PollEvents();
        while (evt::zEventHandler::GetInstance().PopEvent(e))
            quit = e.type == evt::EventType::WINDOW_CLOSE;

        Window2.Clear(color4f_t(0, 1.0, 0));
        Window2.Update();
    }

    Window2.Destroy();
    Quit();

    return 0;
}
