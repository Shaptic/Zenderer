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
    //Sound.LoadFromFile("Crackle.wav");

    color4f_t Teal(0.0, 1.0, 1.0, 1.0);

    gfxcore::CTexture* T = Manager.Create<gfxcore::CTexture>(
        string_t("sample.png"));

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

    gfxcore::CVertexArray FS;
    FS.Init();
    FS.AddData(D);
    FS.Offload();

    D.Indices = nullptr;
    delete[] D.Vertices;
    D.vcount = D.icount = 0;

    gfx::CQuad Q(32, 32);
    Q.Create();
    Q.SetColor(color4f_t(1));

    Window.ToggleVSYNC();
    util::CTimer Timer(60);

    gfxcore::CRenderer::EnableAlphaBlending();

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

    while(Window.IsOpen())
    {
        Timer.Start();

        // Handle events
        glfwPollEvents();
        glfwGetCursorPos(Window.GetWindow(), &x, &y);

        // Game logic
        Sound.Update();

        RT.Bind();
        RT.Clear();
        T->Bind();
        gfxcore::CRenderer::GetDefaultEffect().Enable();
        gfxcore::CRenderer::GetDefaultEffect().SetParameter(
            "proj", gfxcore::CRenderer::GetProjectionMatrix());
        gfxcore::CRenderer::GetDefaultEffect().SetParameter(
            "mv", math::matrix4x4_t::GetIdentityMatrix());
        Vao.Draw();
        gfxcore::CRenderer::GetDefaultEffect().Disable();
        T->Unbind();
        RT.Unbind();

        // Rendering
        Window.Clear(Teal);

        GL(glBindTexture(GL_TEXTURE_2D, RT.GetTexture()));
        gfxcore::CRenderer::GetDefaultEffect().Enable();
        FS.Draw();
        gfxcore::CRenderer::GetDefaultEffect().Disable();
        GL(glBindTexture(GL_TEXTURE_2D, 0));

        T->Bind();
        gfxcore::CRenderer::GetDefaultEffect().Enable();
        math::matrix4x4_t MV = math::matrix4x4_t::GetIdentityMatrix();
        MV[0][3] = 100.0;
        MV[1][3] = 100.0;
        gfxcore::CRenderer::GetDefaultEffect().SetParameter("mv", MV);
        Vao.Draw();
        gfxcore::CRenderer::GetDefaultEffect().Disable();
        T->Unbind();

        Q.Move(x, y);
        Q.Draw();

        Window.Update();

        // Finalize
        Timer.Delay();
    }

    Vao.Destroy();

    Quit();

    return 0;
}
