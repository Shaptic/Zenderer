#include "Zenderer/Events/Mouse.hpp"

evt::mouse_t::mouse_t() :
    position(0, 0), button(MouseButton::UNKNOWN), down(false)
{
}

zen::math::vector_t evt::GetMousePosition()
{
    double x, y;
    glfwGetMousePos(glfwGetCurrentContext(), &x, &y);
    return zen::math::vector_t(x, y);
}

bool CWindow::GetMouseState(const evt::MouseButton& Btn) const 
{
    return glfwGetMouseButton(glfwGetCurrentContext(),
                              static_cast<int>(Btn)) == GLFW_PRESS;
}
