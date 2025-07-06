#include <vkstart.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>

using namespace vkstart;

static SDL_Window *window = nullptr;
static Engine *engine = nullptr;

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

        SDL_Log("%s", errorMsg);
        throw std::runtime_error{errorMsg};
    }
}

struct SurfaceCreator
{
    vk::raii::SurfaceKHR operator()(const vk::raii::Instance &instance)
    {
        VkSurfaceKHR surface = nullptr;

        bool success = SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface);
        HandleSDLError(success, "SDL_Vulkan_CreateSurface");

        return vk::raii::SurfaceKHR{instance, surface};
    }
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    sdl::SetAppMetadata("Vulkan Hpp SDL", "1.0", "com.dirkz.vulkan.sample");
    sdl::Init(SDL_INIT_VIDEO);

    window = sdl::CreateWindow("Vulkan Hpp SDL", 800, 600, SDL_WINDOW_VULKAN);

    Uint32 numSdlInstanceExtensions = 0;
    const char *const *const sdlInstanceExtensions =
        SDL_Vulkan_GetInstanceExtensions(&numSdlInstanceExtensions);
    HandleSDLError(sdlInstanceExtensions == nullptr, "SDL_Vulkan_GetInstanceExtensions");

    std::vector<const char *> instanceExtensions(numSdlInstanceExtensions);
    instanceExtensions.assign(sdlInstanceExtensions,
                              sdlInstanceExtensions + numSdlInstanceExtensions);

    SDL_FunctionPointer sdlProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
    HandleSDLError(sdlProcAddr == nullptr, "SDL_Vulkan_GetVkGetInstanceProcAddr");

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(sdlProcAddr);

    assert(vkGetInstanceProcAddr != nullptr);

    engine = new Engine{vkGetInstanceProcAddr, std::span{instanceExtensions}};

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
    if (engine)
    {
        free(engine);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }
}
