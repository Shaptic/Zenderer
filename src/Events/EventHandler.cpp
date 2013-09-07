#include "Zenderer/Events/EventHandler.hpp"

using namespace zen::evt;

std::stack<event_t> zEventHandler::s_evtList;
event_t zEventHandler::s_Active;

zEventHandler::~zEventHandler()
{
    while(!s_evtList.empty()) s_evtList.pop();
}

bool zEventHandler::PollEvents()
{
    glfwPollEvents();
    return !s_evtList.empty();
}

bool zEventHandler::PopEvent(event_t& Evt)
{
    Evt.Reset();
    if(s_evtList.empty()) return false;
    Evt = s_evtList.top();
    s_evtList.pop();
    return true;
}

zEventHandler& zEventHandler::GetInstance()
{
    static zEventHandler g_Events;
    return g_Events;
}

void zEventHandler::KeyboardCallback(GLFWwindow*, int key, int scancode,
                                     int action, int mods)
{
    s_Active.key.key    = static_cast<Key>(key);
    s_Active.key.symbol = static_cast<char>(key);
    s_Active.key.scan   = scancode;

    if(action == GLFW_PRESS)        s_Active.type = EventType::KEY_DOWN;
    else if(action == GLFW_RELEASE) s_Active.type = EventType::KEY_UP;
    else if(action == GLFW_REPEAT)  s_Active.type = EventType::KEY_HOLD;

    s_evtList.push(s_Active);
    s_Active.Reset();
}

void zEventHandler::MouseMotionCallback(GLFWwindow*, double x, double y)
{
    s_Active.mouse.position = math::vector_t(x, y);
    s_Active.mouse.button   = MouseButton::UNKNOWN;
    s_Active.mouse.down     = false;
    s_Active.type           = EventType::MOUSE_MOTION;

    s_evtList.push(s_Active);
    s_Active.Reset();
}

void zEventHandler::MouseCallback(GLFWwindow*, int button, int action, int mods)
{
    s_Active.mouse.position = evt::GetMousePosition();
    s_Active.mouse.button   = static_cast<MouseButton>(button);
    s_Active.mouse.down     = (action == GLFW_PRESS);
    s_Active.mouse.mods     = mods;

    if(action == GLFW_PRESS)
        s_Active.type = EventType::MOUSE_DOWN;
    else
        s_Active.type = EventType::MOUSE_UP;

    s_evtList.push(s_Active);
    s_Active.Reset();
}

void zEventHandler::CharacterCallback(GLFWwindow*, unsigned int c)
{
    s_Active.key.symbol = c;
    s_Active.key.key    = Key::UNKNOWN;
    s_Active.key.mods   = 0;
    s_Active.key.scan   = -1;
    s_Active.type       = EventType::PRINTABLE_KEY;

    s_evtList.push(s_Active);
    s_Active.Reset();
}

void zEventHandler::WindowCloseCallback(GLFWwindow* pWindow)
{
    s_Active.type = EventType::WINDOW_CLOSE;
    s_evtList.push(s_Active);
    s_Active.Reset();
}
