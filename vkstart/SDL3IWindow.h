#pragma once

#include "stdafx.h"

#include "IWindow.h"

namespace vkstart
{

inline void HandleSDLError(bool errorCheck, const char *functionName)
{
    if (errorCheck)
    {
        constexpr size_t ErrorMessageSize = 256;
        char errorMsg[ErrorMessageSize];

        const char *sdlErrorMessage = SDL_GetError();
        if (sdlErrorMessage && sdlErrorMessage[0])
        {
            SDL_snprintf(errorMsg, ErrorMessageSize, "SDL error calling %s: %s", functionName,
                         sdlErrorMessage);
        }
        else
        {
            SDL_snprintf(errorMsg, ErrorMessageSize, "SDL error calling %s", functionName);
        }

        SDL_Log("%s", errorMsg);
        throw std::runtime_error{errorMsg};
    }
}

struct SDL3IWindow : public IWindow
{
    SDL3IWindow(SDL_Window *window);
    SDL3IWindow(SDL3IWindow &&other) noexcept;

    ~SDL3IWindow();

    vk::raii::SurfaceKHR CreateSurface(const vk::raii::Instance &instance) override;
    void GetPixelDimensions(int *width, int *height) override;
    std::vector<std::string> RequiredInstanceExtensions() override;

  private:
    SDL_Window *m_window;
};

} // namespace vkstart
