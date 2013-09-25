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

    auto tri1 = P1.Triangulate();
    auto tri2 = P2.Triangulate();

    for(auto& i : tri2) i = i + math::vector_t(100, 100);

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
                for(auto& i : tri1) i = i - d;
            }
        }

        math::tri_t a = { tri1[0], tri1[2], tri1[2] };
        math::tri_t b = { tri2[0], tri2[2], tri2[2] };

        for(auto i : a) std::cout << i << ' ';
        for(auto i : b) std::cout << i << ' ';

        std::cout << math::collides(a, b) << std::endl;

        Win.Clear();
        P2.Draw();
        P1.Draw();
        Win.Update();
    }

    Quit();
    return 0;
}
