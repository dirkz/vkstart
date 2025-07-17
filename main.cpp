#include <vkstart.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>

using namespace vkstart;

struct ApplicationState
{
    ApplicationState(SDL3IWindow *window, Engine &engine)
        : m_window{window}, m_engine{std::move(engine)}
    {
    }

    ~ApplicationState()
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
    SDL3IWindow *m_window;
    Engine m_engine;
};

static ApplicationState *GetApplicationState(void *appstate)
{
    ApplicationState *appData = reinterpret_cast<ApplicationState *>(appstate);
    return appData;
}

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

    SDL3IWindow *window = new SDL3IWindow{sdlWindow};

    SDL_FunctionPointer sdlProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
    HandleSDLError(sdlProcAddr == nullptr, "SDL_Vulkan_GetVkGetInstanceProcAddr");

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(sdlProcAddr);

    assert(vkGetInstanceProcAddr != nullptr);

    Engine engine = Engine{vkGetInstanceProcAddr, window};

    ApplicationState *appState = new ApplicationState{window, engine};
    *appstate = appState;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        ApplicationState *appData = GetApplicationState(appstate);
        Sint32 width = event->display.data1;
        Sint32 height = event->display.data2;
        appData->GetEngine().PixelSizeChanged();
    }
        return SDL_APP_CONTINUE;
    default:
        return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    ApplicationState *appData = GetApplicationState(appstate);

    appData->GetEngine().DrawFrame();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    ApplicationState *appData = reinterpret_cast<ApplicationState *>(appstate);
    appData->GetEngine().WaitIdle();
    free(appData);
}
