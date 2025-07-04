#include "Engine.h"

namespace vkstart
{

Engine::Engine(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
               std::span<const char *> instanceExtensions)
    : m_context{vkGetInstanceProcAddr}
{
    for (auto extension : instanceExtensions)
    {
        SDL_Log("requested instance extension: %s", extension);
    }

    CreateInstance(instanceExtensions);
}

void Engine::CreateInstance(std::span<const char *> instanceExtensions)
{
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangles",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "vkstart",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vk::ApiVersion14,
    };

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    m_instance = std::make_unique<vk::raii::Instance>(m_context, createInfo);
}

} // namespace vkstart