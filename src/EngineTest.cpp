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

    sfx::CSound2D* Sound =
        Manager.Create<sfx::CSound2D*>(string_t("Crackle.wav"));

    color4f_t Teal(0.0, 1.0, 1.0, 1.0);

    gfx::CMaterial Sample(&Manager);
    gfx::CMaterial Grass(&Manager);
    
    Sample.LoadTextureFromFile("sample.png");
    Sample.LoadEffect(gfx::EffectType::NO_EFFECT);
    
    Grass.LoadTextureFromFile("grass.png");
    Grass.LoadEffect(gfx::EffectType::NO_EFFECT);

    // Create the vertex buffers we will be using (inefficient).
    gfxcore::CVertexArray Vao, FS, Gr;
    Vao.Init(); FS.Init(); Gr.Init();
        
    // Quad to easily store vertex data
    gfx::CQuad Default(T->GetWidth(), T->GetHeight());
    Default.SetColor(color4f_t(1, 1, 1, 1));
    Default.Create();
    
    // Regular quad
    Default.LoadIntoVAO(Default);

    // Full screen
    Default.Resize(800, 600);
    Default.Create();
    Default.LoadIntoVAO(FS);
    
    // Stretched grass quad
    Default.SetInverted(true);
    Default.SetRepeating(true);
    Default.AttachMaterial(CMaterial(nullptr, Grass));
    Default.Resize(15 * Grass->GetWidth(), Grass->GetHeight());
    Default.Create();
    Default.LoadIntoVAO(Gr);
    
    Vao.Offload();
    FS.Offload();
    Gr.Offload();

    Default.Resize(32, 32);
    Default.AttachMaterial(nullptr);
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

    double x, y;
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
            DEffect.Enable();

            DEffect.SetParameter("proj", CRenderer::GetProjectionMatrix());
            DEffect.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());

            RT.Clear();
            Vao.Draw();

            CRenderer::ResetMaterialState();
            RT.Unbind();
        }

        // Rendering
        Window.Clear(Teal);

        {
            CRenderer::EnableTexture(RT.GetTexture());
            DEffect.Enable();

            FS.Draw();

            CRenderer::ResetMaterialState();
        }

        {
            T->Bind();
            DEffect.Enable();

            Vao.Draw();

            CRenderer::ResetMaterialState();
        }

        Default.Move(x, y);
        Default.Draw();

        {
            DEffect.Enable();
            Grass->Bind();

            math::matrix4x4_t MV = math::matrix4x4_t::GetIdentityMatrix();

            if((d > 0 && angle > 45.0) || (d < 0 && angle < -45.0))
                d = -d;

            MV.Shear(math::vector_t(angle += d, 0.0));
            MV.Translate(math::vector_t(300, 300));

            DEffect.SetParameter("proj", CRenderer::GetProjectionMatrix());
            DEffect.SetParameter("mv", MV);

            Gr.Draw();

            CRenderer::ResetMaterialState();
        }

        Window.Update();
        
        //Sound.Update();

        // Finalize
        Timer.Delay();
    }

    Vao.Destroy();

    Quit();

    return 0;
}
