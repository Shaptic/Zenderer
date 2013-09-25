#include "Zenderer/Zenderer.hpp"

using namespace zen;

int main()
{
    Init();

    asset::zAssetManager Mgr; Mgr.Init();
    gfx::zWindow Win(800, 600, "Collision Test", Mgr, false);
    Win.Init();

    gfx::zPolygon P1(Mgr);
    P1.AddVertex(math::vector_t(0, 0));
    P1.AddVertex(math::vector_t(100, 0));
    P1.AddVertex(math::vector_t(0, 100));
    P1.SetColor(color4f_t(1.0, 0, 0));
    P1.Create();

    gfx::zPolygon P2(P1);
    P2.SetColor(color4f_t(0, 1, 0));
    P2.Create();
    P2.Move(100, 100);

    evt::event_t E;
    while(Win.IsOpen())
    {
        evt::zEventHandler::PollEvents();
        while(evt::zEventHandler::GetInstance().PopEvent(E))
        {
            if(E.type == evt::EventType::MOUSE_MOTION)
            {
                math::vector_t d = P1.GetPosition() - E.mouse.position;
                P1.Move(E.mouse.position);
            }
        }

        std::cout << P1.Collides(P2) << std::endl;

        Win.Clear();
        P2.Draw();
        P1.Draw();
        Win.Update();
    }

    Quit();
    return 0;
}
