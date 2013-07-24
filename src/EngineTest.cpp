#include "Zenderer/Zenderer.hpp"

using namespace zen;
using gfxcore::CRenderer;

#if defined(ZEN_API) && !defined(_DEBUG) && !defined(ZENDERER_EXPORTS)
  #pragma comment(lib, "Zenderer.lib")
#endif // ZEN_API

// Windowed version for release builds.
#if !defined(_DEBUG) && defined(_WIN32)
  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPInst,
                     LPSTR lpCmdLine, int nShowCmd)
#else
  int main(int argc, char* argv[])
#endif // _DEBUG
{
    Init();
    asset::CAssetManager Manager; Manager.Init();
    gfx::CWindow Window(800, 600, "Hello, Zenderer.", Manager);

    Window.Init();

    sfx::CSound2D* Sound = Manager.Create<sfx::CSound2D>("Crackle.wav");

    color4f_t Teal(0.0, 1.0, 1.0, 1.0);

    gfx::CMaterial Sample(Manager);
    gfx::CMaterial Grass(Manager);

    Sample.LoadTextureFromFile("sample.png");
    Grass.LoadTextureFromFile("grass.png");

    // Create the vertex buffers we will be using (inefficient).
    // Recommended practice would be to combine into one buffer
    // to minimize state change.
    gfxcore::CVertexArray Vao, FS, Gr;
    Vao.Init(); FS.Init(); Gr.Init();

    // Quad to easily store vertex data
    gfx::CQuad Default(Manager,
                       Sample.GetTexture().GetWidth(),
                       Sample.GetTexture().GetHeight());

    // Regular quad
    Default.Create();
    Default.SetColor(color4f_t(1, 1, 1, 1));
    Default.LoadIntoVAO(Vao);

    // Full screen
    Default.Resize(800, 600);
    Default.Create();
    Default.LoadIntoVAO(FS);

    // Stretched grass quad
    Default.SetInverted(true);
    Default.SetRepeating(true);
    Default.AttachMaterial(Grass);
    Default.Resize(15 * Grass.GetTexture().GetWidth(),
                   Grass.GetTexture().GetHeight());
    Default.Create();
    Default.LoadIntoVAO(Gr);

    Vao.Offload();
    FS.Offload();
    Gr.Offload();

    Default.SetInverted(false);
    Default.Resize(32, 32);
    Default.RemoveMaterial();
    Default.SetColor(color4f_t(1, 0, 0, 1));
    Default.Create();

    Window.ToggleVSYNC();
    util::CTimer Timer(60);

    CRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);

    gfx::CLight L(Manager, gfx::LightType::ZEN_POINT, Window.GetHeight());
    L.Init();
    L.Enable();
    L.SetBrightness(0.5);
    L.SetColor(1.0, 1.0, 1.0);
    L.SetPosition(200, 100);
    L.Disable();

    gfx::CRenderTarget RT(800, 600);
    RT.Init();

    gfx::CEffect& DEffect = CRenderer::GetDefaultEffect();

    real_t angle = 45.0, d = -5.5;

    evt::CEventHandler& Evt = evt::CEventHandler::GetInstance();
    evt::event_t event;

    gfx::CQuad EntPrim(Manager, 111, 64);
    EntPrim.AttachMaterial(Sample);
    EntPrim.SetInverted(true);
    EntPrim.Create();

    gfx::CScene Scene(800, 600, Manager);
    Scene.Init(); Scene.EnableLighting();

    obj::CEntity& Ent = Scene.AddEntity();
    gui::CFont* Font = Manager.Create<gui::CFont>();
    Font->AttachManager(Manager);
    Font->SetSize(36);
    Font->SetColor(color4f_t(1.0, 1.0, 0.0));
    Font->LoadFromFile("C:\\Windows\\Fonts\\segoeuil.ttf");
    (*Font) << "Zenderer!" << 6606 << " lines!";
    Font->Render(Ent);
    Ent.Move(100, 69);

    obj::CEntity& Bg = Scene.AddEntity();
    Bg.LoadFromTexture("MockMenu.png");

    obj::CEntity& Ent3 = Scene.AddEntity();
    obj::CEntity& Ent2 = Scene.AddEntity();
    gfx::CLight& L2 = Scene.AddLight(gfx::LightType::ZEN_SPOTLIGHT);
    gfx::CLight& L3 = Scene.AddLight(gfx::LightType::ZEN_POINT);
    Font->Render(Ent2, "Hi");
    Ent2.Move(200, 200);

    Ent3.LoadFromTexture("sample.png");

    L2.Enable();
    L2.SetBrightness(0.8);
    L2.SetMaximumAngle(270.0);
    L2.SetMinimumAngle(0.0);
    L2.SetColor(color3f_t(1.0, 0.0, 0.0));
    L2.Disable();

    L3.Enable();
    L3.SetBrightness(1.8);
    L3.SetColor(color3f_t(0.0, 1.0, 0.0));
    L3.Disable();

    obj::CEntity& Grass1 = Scene.AddEntity();
    obj::CEntity& Grass2 = Scene.AddEntity();

    Grass1.LoadFromTexture("Zenderer/textures/grass.png");
    Grass2.LoadFromTexture("Zenderer/textures/grass2.png");

    math::vector_t mouse;

    while(Window.IsOpen())
    {
        Timer.Start();

        // Handle events.
        Evt.PollEvents();

        while(Evt.PopEvent(event))
        {
            switch(event.type)
            {
            case evt::EventType::KEY_DOWN:
            case evt::EventType::KEY_UP:
            case evt::EventType::KEY_HOLD:
                std::cout << "Key: " << (int16_t)event.key.key << "\n";
                break;

            case evt::EventType::PRINTABLE_KEY:
                std::cout << "Printable: " << event.key.symbol << "\n";
                break;
            }
        }

        mouse = evt::GetMousePosition();

        // Game logic
        Sound->Update();

        // Rendering
        Window.Clear(Teal);

        RT.Bind();
        Sample.EnableTexture();
        DEffect.Enable();

        DEffect.SetParameter("proj", CRenderer::GetProjectionMatrix());
        DEffect.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());

        RT.Clear();
        Vao.Draw();
        RT.Unbind();

        RT.BindTexture();
        DEffect.Enable();
        DEffect.SetParameter("proj", CRenderer::GetProjectionMatrix());
        FS.Draw();

        Sample.Enable();
        Sample.GetEffect().SetParameter("proj", CRenderer::GetProjectionMatrix());
        Sample.GetEffect().SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
        Vao.Draw();
        Sample.Disable();

        Default.Move(mouse);
        Default.Draw();

        Ent.Draw();
        Ent2.Move(mouse.x, mouse.y);
        Scene.Render();

        {
            Grass.Enable();

            math::matrix4x4_t MV = math::matrix4x4_t::GetIdentityMatrix();

            if((d > 0 && angle > 45.0) || (d < 0 && angle < -45.0))
                d = -d;

            MV.Shear(math::vector_t(angle += d, 0.0));
            MV.Translate(math::vector_t(300, 300));

            DEffect.SetParameter("proj", CRenderer::GetProjectionMatrix());
            DEffect.SetParameter("mv", MV);

            Gr.Draw();
            Grass.Disable();
        }

        Sound->Update();
        Window.Update();

        // Regulate frame rate.
        Timer.Delay();
    }

    // This will be taken care of by Quit() automatically,
    // but it's still a good practice.
    Vao.Destroy();
    Gr.Destroy();
    FS.Destroy();

    Quit();

    return 0;
}
