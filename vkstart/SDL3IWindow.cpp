#include "SDL3IWindow.h"

namespace vkstart
{

SDL3IWindow::SDL3IWindow(SDL_Window *window) : m_window{window}
{
}

SDL3IWindow::SDL3IWindow(SDL3IWindow &&other) noexcept
{
    m_window = other.m_window;
    other.m_window = nullptr;
}

SDL3IWindow::~SDL3IWindow()
{
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
    }
}

vk::raii::SurfaceKHR SDL3IWindow::CreateSurface(const vk::raii::Instance &instance)
{
    VkSurfaceKHR surface = nullptr;

    bool success = SDL_Vulkan_CreateSurface(m_window, *instance, nullptr, &surface);
    HandleSDLError(!success, "SDL_Vulkan_CreateSurface");

    return vk::raii::SurfaceKHR{instance, surface};
}

void SDL3IWindow::GetPixelDimensions(int *width, int *height)
{
    sdl::GetWindowSizeInPixels(m_window, width, height);
}

std::vector<std::string> SDL3IWindow::RequiredInstanceExtensions()
{
    Uint32 numInstanceExtensions = 0;
    const char *const *const instanceExtensions =
        SDL_Vulkan_GetInstanceExtensions(&numInstanceExtensions);
    HandleSDLError(instanceExtensions == nullptr, "SDL_Vulkan_GetInstanceExtensions");

    std::vector<std::string> extensions{};

    for (Uint32 i = 0; i < numInstanceExtensions; ++i)
    {
        extensions.push_back(instanceExtensions[i]);
    }

    return extensions;
}

} // namespace vkstart