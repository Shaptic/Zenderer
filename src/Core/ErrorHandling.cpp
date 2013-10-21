#include "Zenderer/Core/ErrorHandling.hpp"

void zen::error_fallback(const char* message, const char* title)
{
#ifdef _WIN32
    MessageBoxA(nullptr, message, title, 0);
#else
    std::cerr << "[" << title << "] -- " << message << std::endl;
#endif // _WIN32
}
