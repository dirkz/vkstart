#include <vkstart.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>

using namespace vkstart;

struct Application
{
    Application(SDL3Window *window, Engine &engine) : m_window{window}, m_engine{std::move(engine)}
    {
    }

    ~Application()
    {
        if (m_window)
        {
            delete m_window;
        }
    }

    Engine &GetEngine()
    {
        return m_engine;
    }

  private:
    SDL3Window *m_window;
    Engine m_engine;
};

struct SurfaceCreator
{
    SurfaceCreator(SDL_Window *window) : m_window{window}
    {
    }

    vk::raii::SurfaceKHR operator()(const vk::raii::Instance &instance)
    {
        VkSurfaceKHR surface = nullptr;

        bool success = SDL_Vulkan_CreateSurface(m_window, *instance, nullptr, &surface);
        HandleSDLError(!success, "SDL_Vulkan_CreateSurface");

        return vk::raii::SurfaceKHR{instance, surface};
    }

  private:
    SDL_Window *m_window;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    sdl::SetAppMetadata("Vulkan Hpp SDL", "1.0", "com.dirkz.vulkan.sample");
    sdl::Init(SDL_INIT_VIDEO);

    SDL_Window *sdlWindow =
        sdl::CreateWindow("Vulkan Hpp SDL", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    SDL3Window *window = new SDL3Window{sdlWindow};

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

    int pixelWidth, pixelHeight;
    sdl::GetWindowSize(sdlWindow, &pixelWidth, &pixelHeight);

    Engine engine = Engine{vkGetInstanceProcAddr, std::span{instanceExtensions}, window};

    Application *appData = new Application{window, engine};
    *appstate = appData;

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
    Application *appData = reinterpret_cast<Application *>(appstate);

    appData->GetEngine().DrawFrame();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    Application *appData = reinterpret_cast<Application *>(appstate);
    free(appData);
}
