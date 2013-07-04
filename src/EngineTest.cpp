#include <algorithm>

#include "Zenderer/Zenderer.hpp"

using namespace zen;
using gfxcore::CRenderer;

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
    //Sound.LoadFromFile("Crackle.wav");

    color4f_t Teal(0.0, 1.0, 1.0, 1.0);

    gfxcore::CTexture* T = Manager.Create<gfxcore::CTexture>(
        string_t("sample.png"));

    gfxcore::CTexture* Grass = Manager.Create<gfxcore::CTexture>(
        string_t("grass.png"));

    gfxcore::CVertexArray Vao;
    gfxcore::DrawBatch D;

    gfxcore::index_t i[] = {0, 1, 3, 3, 1, 2};

    D.Vertices  = new gfxcore::vertex_t[4];
    D.vcount    = 4;
    D.Indices   = i;
    D.icount    = 6;

    D.Vertices[0].position = math::vector_t(0, 0);
    D.Vertices[1].position = math::vector_t(T->GetWidth(), 0);
    D.Vertices[2].position = math::vector_t(T->GetWidth(), T->GetHeight());
    D.Vertices[3].position = math::vector_t(0, T->GetHeight());

    D.Vertices[0].tc = math::vector_t(0.0, 1.0);
    D.Vertices[1].tc = math::vector_t(1.0, 1.0);
    D.Vertices[2].tc = math::vector_t(1.0, 0.0);
    D.Vertices[3].tc = math::vector_t(0.0, 0.0);

    D.Vertices[0].color =
    D.Vertices[1].color =
    D.Vertices[2].color =
    D.Vertices[3].color = color4f_t(1, 1, 1, 1);

    Vao.Init();
    Vao.AddData(D);
    Vao.Offload();

    D.Vertices[0].position = math::vector_t(0, 0);
    D.Vertices[1].position = math::vector_t(800, 0);
    D.Vertices[2].position = math::vector_t(800, 600);
    D.Vertices[3].position = math::vector_t(0, 600);

    gfxcore::CVertexArray FS, G;
    FS.Init(); G.Init();
    FS.AddData(D);
    FS.Offload();

    D.Vertices[0].position = math::vector_t(0, 0);
    D.Vertices[1].position = math::vector_t(Grass->GetWidth(), 0);
    D.Vertices[2].position = math::vector_t(Grass->GetWidth(), Grass->GetWidth());
    D.Vertices[3].position = math::vector_t(0, Grass->GetWidth());

    G.AddData(D);
    G.Offload();

    D.Indices = nullptr;
    delete[] D.Vertices;
    D.vcount = D.icount = 0;

    gfx::CQuad Q(32, 32);
    Q.Create();
    Q.SetColor(color4f_t(1));

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

    double x, y;

    gfx::CEffect& Default = CRenderer::GetDefaultEffect();

    real_t angle = 45.0, d = -5.5;

    while(Window.IsOpen())
    {
        Timer.Start();

        // Handle events
        glfwPollEvents();
        glfwGetCursorPos(Window.GetWindow(), &x, &y);

        // Game logic
        Sound.Update();

        {
            RT.Bind();
            T->Bind();
            Default.Enable();

            Default.SetParameter("proj", CRenderer::GetProjectionMatrix());
            Default.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());

            RT.Clear();
            Vao.Draw();

            CRenderer::ResetMaterialState();
            RT.Unbind();
        }

        // Rendering
        Window.Clear(Teal);

        {
            CRenderer::EnableTexture(RT.GetTexture());
            Default.Enable();

            FS.Draw();

            CRenderer::ResetMaterialState();
        }

        {
            T->Bind();
            Default.Enable();

            Vao.Draw();

            CRenderer::ResetMaterialState();
        }

        Q.Move(x, y);
        Q.Draw();

        {
            Default.Enable();
            Grass->Bind();

            math::matrix4x4_t MV = math::matrix4x4_t::GetIdentityMatrix();

            if((d > 0 && angle > 45.0) || (d < 0 && angle < -45.0))
                d = -d;

            MV.Translate(math::vector_t(300, 300));
            MV.Shear(math::vector_t(angle += d, 0.0));

            Default.SetParameter("proj", CRenderer::GetProjectionMatrix());
            Default.SetParameter("mv", MV);

            G.Draw();

            CRenderer::ResetMaterialState();
        }

        Window.Update();

        // Finalize
        Timer.Delay();
    }

    Vao.Destroy();

    Quit();

    return 0;
}
