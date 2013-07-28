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

    // Tiled grass quad
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
    Scene.Init(); Scene.SetSeeThrough(true); //Scene.EnableLighting();

    obj::CEntity& Ent = Scene.AddEntity();
    gui::CFont* Font = Manager.Create<gui::CFont>();
    Font->AttachManager(Manager);
    Font->SetSize(18);
    Font->SetColor(color4f_t(0.0, 0.0, 0.0));
    Font->LoadFromFile("C:\\Windows\\Fonts\\segoeuil.ttf");
    (*Font) << "Zenderer! " << 6606 << " lines and\neven More qomments.";
    Font->Render(Ent);
    Ent.Move(100, 69);

    obj::CEntity& Bg = Scene.AddEntity();
    //Bg.LoadFromTexture("MockMenu.png");

    obj::CEntity& Ent3 = Scene.AddEntity();
    obj::CEntity& Ent2 = Scene.AddEntity();
    gfx::CLight& L2 = Scene.AddLight(gfx::LightType::ZEN_SPOTLIGHT);
    gfx::CLight& L3 = Scene.AddLight(gfx::LightType::ZEN_POINT);
    Font->Render(Ent2, "Hi");
    Ent2.Move(200, 200);

    Ent3.LoadFromTexture("sample.png");
    Ent3.Move(500, 100);

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
    obj::CEntity& Grass3 = Scene.AddEntity();

    Grass1.Invert();
    Grass2.Invert();
    Grass3.Invert();

    Grass1.LoadFromTexture(ZENDERER_TEXTURE_PATH"grass.png");
    Grass2.LoadFromTexture(ZENDERER_TEXTURE_PATH"grass2.png");
    Grass3.LoadFromTexture(ZENDERER_TEXTURE_PATH"grass.png");

    Grass1.Move(100, 400);
    Grass2.Move(100, 400);
    Grass3.Move(100, 400);

    math::vector_t GrassT(45.0, 45.0, -45.0);
    math::vector_t GrassDT(-1.2, 0.9, -0.5);
    math::vector_t mouse;

    obj::CPhysicsWorld World(math::vector_t(0.0, 9.81), 60);
    World.Init();

    obj::CBody Ground(World, Manager);
    Ground.SetType(obj::BodyType::STATIC_BODY);
    Ground.Move(0, 800);
    Ground.Create();
    //Body.SetSize_px(800, 10);
    //or
    //World.SetScale(0.01);
    //Body.SetSize(8, 0.1);

    obj::CBody Falling(World, Manager);
    Falling.SetType(obj::BodyType::DYNAMIC_BODY);
    Falling.Move(10, 0);
    Falling.Create();
    //Falling.SetSize_px(32, 32);
    //obj::fixture_t Fixture;
    //Fixture.density = 1.0;
    //Fixture.friction = 0.3;
    //Falling.AddFixture(Fixture);

    while(Window.IsOpen())
    {
        Timer.Start();

        World.Update();

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
                if(event.key.symbol == 'm') CRenderer::ToggleWireframe();
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
        math::matrix4x4_t MV = math::matrix4x4_t::CreateIdentityMatrix();
        MV.Translate(math::vector_t(400, 100));
        Sample.GetEffect().SetParameter("proj", CRenderer::GetProjectionMatrix());
        Sample.GetEffect().SetParameter("mv", MV);
        Vao.Draw();
        Sample.Disable();

        Default.Move(mouse);
        Default.Draw();

        Ent.Draw();
        Ent2.Move(mouse.x, mouse.y);

        if((GrassDT.x > 0 && GrassT.x > 45.0) ||
           (GrassDT.x < 0 && GrassT.x < -45.0)) GrassDT.x = -GrassDT.x;

        if((GrassDT.y > 0 && GrassT.y > 45.0) ||
           (GrassDT.y < 0 && GrassT.y < -45.0)) GrassDT.y = -GrassDT.y;

        if((GrassDT.z > 0 && GrassT.z > 45.0) ||
           (GrassDT.z < 0 && GrassT.z < -45.0)) GrassDT.z = -GrassDT.z;

        Grass1.Shear(math::vector_t(GrassT.x += GrassDT.x, 0.0));
        Grass2.Shear(math::vector_t(GrassT.y += GrassDT.y, 0.0));
        Grass3.Shear(math::vector_t(GrassT.z += GrassDT.z, 0.0));

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

        PHYZ.Draw();

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
