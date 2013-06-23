#include "Zenderer/Core/ErrorHandling.hpp"

void zen::error_fallback(const char* message, const char* title)
{
#ifdef _WIN32
    MessageBoxA(NULL, message, title, NULL);
#else
    std::cerr << "[" << title << "] -- " << message << std::endl;
#endif // _WIN32
}
