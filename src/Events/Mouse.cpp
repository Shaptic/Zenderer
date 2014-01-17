#include "Zenderer/Events/Mouse.hpp"

zen::evt::mouse_t::mouse_t() :
    position(0, 0), button(MouseButton::UNKNOWN), down(false)
{
}

glm::vec2 zen::evt::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(glfwGetCurrentContext(), &x, &y);
    return glm::vec2(x, y);
}

bool zen::evt::GetMouseState(const evt::MouseButton& Btn)
{
    return glfwGetMouseButton(glfwGetCurrentContext(),
                              static_cast<int>(Btn)) == GLFW_PRESS;
}
