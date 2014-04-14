#include "Zenderer/Zenderer.hpp"

#define ZEN_BOILERPLATE(AssetManagerName,   \
    EventStructName, LoopControllerName)    \
        using namespace zen;                \
        Init();                             \
        asset::zAssetManager                \
            AssetManagerName;               \
        AssetManagerName.Init();            \
        evt::event_t EventStructName;       \
        bool LoopControllerName = false;

int main69()
{
    ZEN_BOILERPLATE(Assets, e, quit);

    gfx::zWindow Window1(300, 200, "Window 1", Assets, false);
    gfx::zWindow Window2(300, 200, "Window 2", Assets, false);
    Window1.Init();

    gfx::zScene Scene1(Window1.GetWidth(), Window1.GetHeight(), Assets);
    Scene1.Init();

    gui::zFont* pFont = Assets.Create<gui::zFont>(
        ZENDERER_FONT_PATH"errors.ttf", nullptr, 18
    );
    pFont->SetColor(0, 1, 0);

    //gfx::zQuad Q(Assets, 32, 32);
    //Q.SetColor(0, 1, 0);
    //Q.Create();
    obj::zEntity& String = Scene1.AddEntity();//(Assets);
    pFont->Render(String, "Hello");
    //String.AddPrimitive(std::move(Q));

    while (!quit)
    {
        evt::zEventHandler::GetInstance().PollEvents();
        while (evt::zEventHandler::GetInstance().PopEvent(e))
            quit = e.type == evt::EventType::WINDOW_CLOSE;

        Window1.Clear(color4f_t(1.0, 0, 0));
        String.Draw();
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

   // Window2.Destroy();
    Quit();

    return 0;
}
