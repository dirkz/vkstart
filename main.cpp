#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <SDL.hpp>

static SDL_Window *window = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    sdl::SetAppMetadata("Vulkan Hpp SDL", "1.0", "com.dirkz.vulkan.sample");
    sdl::Init(SDL_INIT_VIDEO);

    window = sdl::CreateWindow("Vulkan Hpp SDL", 800, 600, SDL_WINDOW_VULKAN);

    Uint32 numInstanceExtensions = 0;
    const char *const *const instanceExtensions =
        SDL_Vulkan_GetInstanceExtensions(&numInstanceExtensions);

    if (!instanceExtensions)
    {
        const char *errorMsg = SDL_GetError();
        if (errorMsg && errorMsg[0])
        {

            SDL_Log("Error with SDL_Vulkan_GetInstanceExtensions: %s", errorMsg);
        }
    }

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
