#include "Zenderer/Events/Mouse.hpp"

using namespace zen;

evt::mouse_t::mouse_t() :
    position(0, 0), button(MouseButton::UNKNOWN), down(false)
{
}

math::vector_t zen::evt::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(glfwGetCurrentContext(), &x, &y);
    return math::vector_t(x, y);
}

bool evt::GetMouseState(const evt::MouseButton& Btn)
{
    return glfwGetMouseButton(glfwGetCurrentContext(),
                              static_cast<int>(Btn)) == GLFW_PRESS;
}
