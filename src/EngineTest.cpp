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

    util::CXMLParser XML;
    XML.LoadFromString(SAMPLE_XML);
    XML.LoadFromFile("sample.xml");

    for(auto& i : XML)
    {
        std::cout << i << "[" << i->parent << "]: " << i->name << " {";
        for(auto& j : i->options)
            std::cout << j.first << " -> " << j.second << ", ";

        std::cout << "} --- " << i->content << std::endl;
    }

    util::CSettings& Settings = util::g_EngineSettings;

    Settings["PLAYER_NAME"] = "Zenderer Player";
    Settings["PLAYER_HEALTH"] = 100ULL;
    Settings["PLAYER_SPEED"] = 5.0;
    Settings["PLAYER_ALIVE"] = true;

    /*
    for(const auto& i : Settings)
        std::cout   << i.first << " => "
                    << i.second << std::endl;
    */

    Settings["PLAYER_ALIVE"] = false;
    std::cout << "Player: " << Settings["PLAYER_ALIVE"] << std::endl;

    size_t health = Settings["PLAYER_HEALTH"];
    std::cout << "Health: " << health << std::endl;

    int*    arr1 = g_Alloc.get<int>(2);
    char*   arr2 = g_Alloc.get<char>(45);
    double* arr3 = g_Alloc.get<double>(5);

    std::fill(arr1 , arr1 + 2, 69);

    asset::CAssetManager Manager;
    gfx::CWindow Window(800, 600, "Hello, Zenderer.");

    Window.Init();
    Window.AttachAssetManager(Manager);

    sfx::CSound2D Sound;
    Sound.LoadFromFile("Crackle.wav");
    Sound.Play();

    while(glfwGetWindowParam(GLFW_OPENED))
    {
        Window.Clear(color4f_t(0.f, 1.f, 1.f, 1.f));
        Window.Update();
        Sound.Update();
    }
    
    /*
    std::for_each(arr1 , arr1 + 2,
        [](const int i) {
            std::cout << i << std::endl;
        }
    );
    */

    //g_Alloc.Free(arr1);
    g_Alloc.Free(arr3);
    g_Alloc.Free(arr3);
    g_Alloc.Free(arr3);

    Quit();

    return 0;
}
