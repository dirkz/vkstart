#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <cassert>
#include <exception>

#include <SDL.hpp>

#include "Engine.h"

static SDL_Window *window = NULL;

static void HandleSDLError(bool errorCheck, const char *functionName)
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

		SDL_Log(errorMsg);
		throw std::runtime_error{errorMsg};
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    sdl::SetAppMetadata("Vulkan Hpp SDL", "1.0", "com.dirkz.vulkan.sample");
    sdl::Init(SDL_INIT_VIDEO);

    window = sdl::CreateWindow("Vulkan Hpp SDL", 800, 600, SDL_WINDOW_VULKAN);

    Uint32 numInstanceExtensions = 0;
    const char *const *const instanceExtensions =
        SDL_Vulkan_GetInstanceExtensions(&numInstanceExtensions);
    HandleSDLError(instanceExtensions == nullptr, "SDL_Vulkan_GetInstanceExtensions");

    SDL_FunctionPointer sdlProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
    HandleSDLError(sdlProcAddr == nullptr, "SDL_Vulkan_GetVkGetInstanceProcAddr");

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(sdlProcAddr);

    assert(vkGetInstanceProcAddr != nullptr, "expecting non-null vkGetInstanceProcAddr");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
