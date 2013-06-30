#include <algorithm>

#include "Zenderer/Zenderer.hpp"

using namespace zen;

// Here's some perfect sample XML.
static const char* SAMPLE_XML[] = {
    "<entity type=\"QUAD\" name=\"generic\">",
        "<position x=\"100\" y=\"200\">",
        "</ >",
        "<texture>",
            "Tester.tga",
        "</ >",
        "<shader type=\"both\">",
            "<vshader>",
                "Tester.vs",
            "</ >",
            "<fshader>",
                "Tester.fs",
            "</ >",
        "</ >",
    "</ >"
};

#if defined(ZEN_API) && !defined(_DEBUG) && !defined(ZENDERER_EXPORTS)
  #pragma comment(lib, "Zenderer.lib")
#endif // ZEN_API

// Windowed version for release builds.
#ifndef _DEBUG
  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPInst,
                     LPSTR lpCmdLine, int nShowCmd)
#else
  int main(int argc, char* argv[])
#endif // _DEBUG
{
    Init();

    asset::CAssetManager Manager; Manager.Init();
    gfx::CWindow Window(800, 600, "Hello, Zenderer.");

    Window.AttachAssetManager(Manager);
    Window.Init();

    sfx::CSound2D Sound;
    Sound.LoadFromFile("Crackle.wav");
    Sound.Play();

    color4f_t Teal(0.0, 1.0, 1.0, 1.0);

    gfxcore::CVertexArray Vao;
    gfxcore::DrawBatch D;

    gfxcore::index_t i[] = {0, 1, 3, 3, 1, 2};

    D.Vertices  = new gfxcore::vertex_t[4];
    D.vcount    = 4;
    D.Indices   = i;
    D.icount    = 6;

    D.Vertices[0].position = math::vector_t(0, 0);
    D.Vertices[1].position = math::vector_t(400, 10);
    D.Vertices[2].position = math::vector_t(480, 200);
    D.Vertices[3].position = math::vector_t(230, 200);

    D.Vertices[0].tc = math::vector_t(0.0, 0.0);
    D.Vertices[1].tc = math::vector_t(1.0, 0.0);
    D.Vertices[2].tc = math::vector_t(1.0, 1.0);
    D.Vertices[3].tc = math::vector_t(0.0, 1.0);

    D.Vertices[0].color =
    D.Vertices[1].color =
    D.Vertices[2].color =
    D.Vertices[3].color = color4f_t(1, 1, 1, 1);

    Vao.Init();
    Vao.AddData(D);
    Vao.Offload();

    D.Indices = nullptr;
    delete[] D.Vertices;
    D.vcount = D.icount = 0;

    gfx::CQuad Q(32, 32); 
    Q.Create().Move(math::vector_t(100, 100));

    Window.ToggleVSYNC();
    util::CTimer Timer(60);

    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    gfx::CLight L(Manager, gfx::LightType::ZEN_SPOTLIGHT, Window.GetHeight());
    L.Init();
    L.Enable(); 
    L.SetBrightness(0.05);
    L.SetColor(0.0, 1.0, 0.0);
    L.SetPosition(200, 100);
    L.Disable();

    while(Window.IsOpen())
    {
        Timer.Start();

        // Handle events
        glfwPollEvents();

        // Game logic
        Sound.Update();

        // Rendering
        Window.Clear(Teal);

        L.Enable();
        Vao.Draw();
        L.Disable();

        Q.Draw();

        Window.Update();

        // Finalize
        Timer.Delay();
    }

    Vao.Destroy();

    Quit();

    return 0;
}
