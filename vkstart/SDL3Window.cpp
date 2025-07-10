#include "SDL3Window.h"

namespace vkstart
{

SDL3Window::SDL3Window(SDL_Window *window) : m_window{window}
{
}

SDL3Window::SDL3Window(SDL3Window &&other)
{
    m_window = other.m_window;
    other.m_window = nullptr;
}

SDL3Window::~SDL3Window()
{
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
    }
}

vk::raii::SurfaceKHR SDL3Window::CreateSurface(const vk::raii::Instance &instance)
{
    VkSurfaceKHR surface = nullptr;

    bool success = SDL_Vulkan_CreateSurface(m_window, *instance, nullptr, &surface);
    HandleSDLError(!success, "SDL_Vulkan_CreateSurface");

    return vk::raii::SurfaceKHR{instance, surface};
}

void SDL3Window::GetPixelDimensions(int *width, int *height)
{
    sdl::GetWindowSize(m_window, width, height);
}

std::vector<std::string> SDL3Window::RequiredInstanceExtensions()
{
    Uint32 numInstanceExtensions = 0;
    const char *const *const instanceExtensions =
        SDL_Vulkan_GetInstanceExtensions(&numInstanceExtensions);
    HandleSDLError(instanceExtensions == nullptr, "SDL_Vulkan_GetInstanceExtensions");

    std::vector<std::string> extensions{};

    for (auto i = 0; i < numInstanceExtensions; ++i)
    {
        extensions.push_back(instanceExtensions[i]);
    }

    return extensions;
}

} // namespace vkstart